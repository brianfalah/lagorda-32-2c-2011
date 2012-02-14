#define FUSE_USE_VERSION 26//ver bien

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include "Comun/ProtocoloDeMensajes.h"
#include "librerias/config_loader.h"
#include "funciones.h"
#include "funcionesManejoEntradas.h"
#include "Comun/sockets.h"
#include "Comun/NIPC.h"
#include "consola.h"
#include "cache.h"

pool poolConexiones;

static int pfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {

	int ret;
	char* punteroUltimaBarra = NULL;
	char *nombreArch;// = malloc(14);
	uint32_t posBytesEntrada = 0, clusterInfo = 0, clusterEntrada = 0;
	char* pathParaTokenizar=strdup(path);//porq no se puede modificar un const

	//abrirArchivo(path);

	punteroUltimaBarra = strrchr(pathParaTokenizar, '/');
	nombreArch = punteroUltimaBarra+1;

	if(strlen(nombreArch) > 13){
		free(pathParaTokenizar);
		return -ENAMETOOLONG;;//ver que devolver,el nombre no puede superar los 13 caracteres
	}

	if(pathParaTokenizar == punteroUltimaBarra){// si se crea dentro del root(si el punt q apunta a la ultima barra es igual al q apunta al primer caracter del path)
		clusterInfo = 2;
		punteroUltimaBarra[0]='\0';
	}
	else{
		punteroUltimaBarra[0]='\0';
		ret = recorrerEntradasPath(pathParaTokenizar, &clusterInfo,&clusterEntrada, &posBytesEntrada);
		if(ret!=0){
			free(pathParaTokenizar);
			return -ENOENT; //no existe el dir en donde se crea
		}

		//pedir entradas y verificar que sea un directorio
		ret = esUnDirectorio(clusterEntrada,posBytesEntrada);
		if (ret){free(pathParaTokenizar); return -ENOTDIR;}
	}


	//ret = buscarEntradaEnDir(&clusterInfo,nombreArch,&posBytesEntrada);//ahora vemos si ya existe alguno igual
	//		if (ret)return -1; //ver qué devolver,ya existe un archivo/dir con mismo nombre

	ret = crearArchivo(clusterInfo, nombreArch);
	if(ret){free(pathParaTokenizar); return ret;}
	abrirArchivo(path);
	ret = persistirFat();

	free(pathParaTokenizar);
	return ret;

}

static int pfs_open(const char *path, struct fuse_file_info *fi) {
	int ret = 0;
	uint32_t posBytesEntrada = 0, clusterInfo = 0, clusterEntrada = 0,*unCluster = NULL;
	t_list* listaClusters = collection_list_create();

	ret = recorrerEntradasPath(path, &clusterInfo,&clusterEntrada, &posBytesEntrada);
	if(ret!=0)return -ENOENT;
	ret = esUnArchivo(clusterEntrada,posBytesEntrada);
	if(ret)return -EISDIR;
//	if((fi->flags & 3) != O_RDONLY)
//	return -EACCES;//por ahora solo puede leer

	ret = listaClustersEncadenados(clusterInfo, listaClusters, 0);

	log_info(archLog,"PFS","%s abierto.",path);
	log_write_without_extra_info(archLog,"lista de clusters asociados:");
	while(collection_list_size(listaClusters)){
		unCluster = collection_list_remove(listaClusters,0);
		log_write_without_extra_info(archLog," %u",*unCluster);
		free(unCluster);
	}

	log_write_without_extra_info(archLog,"\n");

	abrirArchivo(path);

	free(listaClusters);
	return 0;
}

static int pfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	uint32_t posBytesEntrada = 0, clusterInfo = 0, clusterEntrada = 0, tamanoArch = 0;
	struct stat *stbuf = malloc(sizeof(struct stat));
	int ret=0;

	ret = recorrerEntradasPath(path, &clusterInfo,&clusterEntrada, &posBytesEntrada);
	if(ret!=0){free(stbuf); return -ENOENT;}

	ret=getAttributes(path,stbuf);
	tamanoArch = stbuf->st_size;
	free(stbuf);

	if(offset < tamanoArch){
		if(offset + size > tamanoArch){
			size = tamanoArch - offset;
		}
		ret = leerYCargar(clusterInfo, buf, size, offset, path);
	}
	else{
		ret = 0;
	}

	return ret;
}

static int pfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	uint32_t posBytesEntrada = 0, clusterInfo = 0, clusterEntrada = 0, tamanoArch = 0;
	struct stat *stbuf = malloc(sizeof(struct stat));
	static int ret=0;

	//al parecer FUSE hace solo estas comprobaciones
	ret = recorrerEntradasPath(path, &clusterInfo,&clusterEntrada, &posBytesEntrada);
	if(ret!=0) {free(stbuf); return -ENOENT;}

	if(size == 0){return 0; free(stbuf);}

	ret=getAttributes(path,stbuf);
	tamanoArch = stbuf->st_size;

	//agrandar archivo, para mi va a venir una llamada al truncate antes
	if(offset + size > tamanoArch){
		//ret = truncarArchivo(clusterEntrada,posBytesEntrada, clusterInfo,offset + size, path);
		ret = agrandarArchivo(clusterEntrada,posBytesEntrada, clusterInfo,offset + size, path);
	}
//	else{
//
//	}

	ret = escribirArchivo(clusterInfo, buf, size, offset, path);
	free(stbuf);

	//persistirFat();

	return ret;
}

static int pfs_flush(const char *path, struct fuse_file_info *fi) {

	flushearArchivo(path);

	return 0;
}

static int pfs_release(const char *path, struct fuse_file_info *fi) {
	int ret=0;

	ret = persistirFat();
	liberarArchivo(path);

	return 0;
}

static int pfs_truncate(const char *path, off_t nuevoTamano) {
	uint32_t posBytesEntrada = 0, clusterInfo = 0, clusterEntrada = 0, tamanoArch = 0;
	struct stat *stbuf = malloc(sizeof(struct stat));
	int ret=0;

	ret = recorrerEntradasPath(path, &clusterInfo,&clusterEntrada, &posBytesEntrada);
	if(ret!=0){
		free(stbuf);
		return -ENOENT;
	}

	ret=getAttributes(path,stbuf);
	tamanoArch = stbuf->st_size;

	//if (nuevoTamano == tamanoArch)return EXIT_SUCCESS;//tiene sentido, puede querer vaciarlo

	ret = truncarArchivo(clusterEntrada,posBytesEntrada, clusterInfo,nuevoTamano, path);
	if(ret){
		free(stbuf);
		return ret;
	}

	//ret = persistirFat();
	free(stbuf);
	return ret;
}

//static int pfs_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi) {
//}

static int pfs_unlink(const char *path) {
	uint32_t posBytesEntrada = 0, clusterInfo = 0, clusterEntrada = 0, tamanoArch = 0;
	int ret=0;

	ret = recorrerEntradasPath(path, &clusterInfo,&clusterEntrada, &posBytesEntrada);
	if(ret!=0)return -ENOENT;

	ret = borrarArchivo(clusterEntrada,posBytesEntrada, clusterInfo);
	if(ret)return ret;
	ret = persistirFat();
	return ret;
}

static int pfs_mkdir(const char *path, mode_t mode) {
	int ret;
	char* punteroUltimaBarra = NULL;
	char *nombreDir = malloc(50);
	uint32_t posBytesEntrada = 0, clusterInfo = 0, clusterEntrada = 0, tamanoArch = 0;
	char* pathParaTokenizar=strdup(path);//porq no se puede modificar un const

	punteroUltimaBarra = strrchr(pathParaTokenizar, '/');
	strcpy(nombreDir,punteroUltimaBarra+1);

	if(strlen(nombreDir) > 13){
		free(nombreDir);
		free(pathParaTokenizar);
		return -ENAMETOOLONG;//ver que devolver,el nombre no puede superar los 13 caracteres
	}

	if(pathParaTokenizar == punteroUltimaBarra){// si se crea dentro del root(si el punt q apunta a la ultima barra es igual al q apunta al primer caracter del path)
		clusterInfo = 2;
		punteroUltimaBarra[0]='\0';
	}
	else{
		punteroUltimaBarra[0]='\0';
		ret = recorrerEntradasPath(pathParaTokenizar, &clusterInfo,&clusterEntrada, &posBytesEntrada);
		if(ret!=0){
			free(nombreDir);
			free(pathParaTokenizar);
			return -ENOENT;//ver qué devolver, no existe el dir en donde se crea
		}

		//pedir entradas y verificar que sea un directorio
		ret = esUnDirectorio(clusterEntrada,posBytesEntrada);
		if (ret){
			free(nombreDir);
			free(pathParaTokenizar);
			return -ENOTDIR;//ver que devolver
		}
	}


	//ret = buscarEntradaEnDir(&clusterInfo,nombreDir,&posBytesEntrada);//ahora vemos si ya existe alguno igual
	//		if (ret)return -1; //ver qué devolver,ya existe un archivo/dir con mismo nombre

	ret = crearDirectorio(clusterInfo, nombreDir);
	free(nombreDir);
	if(ret){
		free(pathParaTokenizar);
		return ret;
	}
	ret = persistirFat();

	free(pathParaTokenizar);
	return ret;
}

static int pfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	int ret;
	t_list* listaEntradas = collection_list_create();
	uint32_t posBytesEntrada = 0, clusterInfo = 0, clusterEntrada = 0;
	char *nombreEntrada = NULL;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	ret = recorrerEntradasPath(path, &clusterInfo,&clusterEntrada, &posBytesEntrada);
	if(ret!=0){	free(listaEntradas); return -ENOENT; }
	//ret = esUnDirectorio(clusterInfo,posBytesEntrada); //ver si hace falta
	//if (ret){ free(listaEntradas); return -ENOTDIR; }
	ret = listarEntradasDeCluster(clusterInfo, listaEntradas);

	while(collection_list_size(listaEntradas)!=0){
		nombreEntrada = (char *)collection_list_remove(listaEntradas,0);
		filler(buf, nombreEntrada, NULL, 0);
		free(nombreEntrada);
	}


	//collection_list_destroy(listaEntradas, &free);
	free(listaEntradas);

	return 0;
}

static int pfs_rmdir(const char *path) {
	uint32_t posBytesEntrada = 0, clusterInfo = 0, clusterEntrada = 0, tamanoArch = 0;
	t_list* listaEntradas = NULL;
	int ret=0;

	ret = recorrerEntradasPath(path, &clusterInfo,&clusterEntrada, &posBytesEntrada);
	if(ret!=0)return -ENOENT;

	ret = esUnDirectorio(clusterEntrada,posBytesEntrada);
	if (ret) return -ENOTDIR;

	listaEntradas = collection_list_create();
	ret = listarEntradasDeCluster(clusterInfo, listaEntradas);
	if(collection_list_size(listaEntradas) > 0){
		collection_list_destroy(listaEntradas, &free);
		return -ENOTEMPTY;
	}
	collection_list_destroy(listaEntradas, &free);

	ret = borrarDirectorio(clusterEntrada,posBytesEntrada, clusterInfo);//borrar . y .., marcar como borrado y liberar clusters
	if(ret)return ret;
	ret = persistirFat();
	return ret;
}

//static int pfs_fgetattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
//}

static int pfs_getattr(const char *path, struct stat *stbuf) {
	int ret =0;
	char *punteroUltimaBarra=NULL, *nombre;// = malloc(20);
	punteroUltimaBarra = strrchr(path, '/');

	//strcpy(nombre,punteroUltimaBarra+1);
	nombre = punteroUltimaBarra + 1;
	if(strlen(nombre)>13){
		//free(nombre);
		return -ENAMETOOLONG;
	}
	ret=getAttributes(path,stbuf);

	//free(nombre);
	return ret;
}

static int pfs_rename(const char *path, const char *newname) {
	int ret=0;
	uint32_t clusterInfo = 0, clusterEntrada = 0, posBytesEntrada = 0;
	char *punteroUltimaBarra = NULL;
	char *nuevoNombre;
	char* pathParaTokenizar=strdup(newname);

	punteroUltimaBarra = strrchr(pathParaTokenizar, '/');
	if(punteroUltimaBarra != NULL) nuevoNombre = punteroUltimaBarra + 1;

	if (strlen(nuevoNombre) > 13){//el nombre.ext no puede ser mayor a 13
		ret = -ENAMETOOLONG;
	}
	else{
		if(pathParaTokenizar == punteroUltimaBarra){// si se crea dentro del root(si el punt q apunta a la ultima barra es igual al q apunta al primer caracter del path)
			clusterInfo = 2;
			punteroUltimaBarra[0]='\0';
		}
		else{
			punteroUltimaBarra[0]='\0';
			ret = recorrerEntradasPath(pathParaTokenizar, &clusterInfo,&clusterEntrada, &posBytesEntrada);
			if(ret!=0){
				free(pathParaTokenizar);
				return -ENOENT; //no existe el dir destino
			}
		}
		ret = renombrarArch(path, nuevoNombre, clusterInfo); //tener cuidado con los tipos de datos, porq adentro se hace un strtok por ejemplo, y es const
	}

	free(pathParaTokenizar);
	return ret;
}

static struct fuse_operations pfs_oper = {

		.create = pfs_create,
		.open = pfs_open,
		.read = pfs_read,
		.write = pfs_write,
		.flush = pfs_flush,
		.release = pfs_release,
		.truncate = pfs_truncate,
		.unlink = pfs_unlink,
		.mkdir = pfs_mkdir,
		.readdir = pfs_readdir, //deberia andar
		.rmdir = pfs_rmdir,
		.getattr = pfs_getattr, //deberia andar
		.rename = pfs_rename,

};

int handshake(int *socket, char*superip, int puerto){
	char *msg = NULL;
	NIPC_Header *head = malloc(sizeof(NIPC_Header));
	char* ip = malloc(strlen(superip)+1);
	strcpy(ip,superip);

	conectarSocketInet(socket, ip, puerto);
	msg = serializarNipc(HANDSHAKE, 0, NULL);
	send(*socket, msg, sizeof(NIPC_Header), MSG_NOSIGNAL);
	recv(*socket,head ,sizeof(NIPC_Header), MSG_WAITALL);
	free(head);
	free(ip);
	free(msg);
	//IMPRIMIR HANDSHAKE CORRECTO
}

int inicializarPool() {
	char *bufConfig = config_loader_open("pfs.config");
	//char *ip = levantarConfig("pfs.config", "IP");
	char *ip = config_loader_getString(bufConfig,"IP");
	//int puerto = atoi(levantarConfig("pfs.config", "Puerto"));
	int puerto = config_loader_getInt(bufConfig,"Puerto");
	int n = config_loader_getInt(bufConfig,"CantidadMaximaDeConexiones");
	int i=0;
	pthread_mutex_init(&poolConexiones.poolSemaphore, NULL);
	poolConexiones.conexiones = malloc(n * sizeof(conexion));
	poolConexiones.cantidadConexiones = n;

	for (i = 0; i < n; i++) {
		pthread_mutex_init(&poolConexiones.conexiones[i].socketSemaphore, NULL);
		handshake(&poolConexiones.conexiones[i].socket, ip, puerto);
	}

	return 0;

}


//int fd;

int main(int argc, char *argv[]) {

	bs = malloc(sizeof(bootSector));
	//char *path;
	int ret;
	cantClusLibres=0;
	pthread_t threadConsola;
	pthread_mutex_init(&semFat, NULL);
	//int argc_pfs = 3;//5
	//char *argv_pfs[] = {NULL , NULL,"-f"}; //"-d","-s","-f"}

	//char *argv_pfs[] = { "/home/utn_so/trunk/PFS/Debug/PFS" , "/home/utn_so/Desarrollo/MONTFUSE" ,"-f"};
	//argv_pfs[0] = argv[0];
	//argv_pfs[1] = argv[1];

	//path = levantarConfig("pfs.config", "Path del archivo de montaje");
	//path="/home/pablo/Desktop/small_fat32.disk";
	//fd = open(path, O_RDWR);

	inicializarPool();
	archLog = log_create("PFS","pfs.log",INFO,M_CONSOLE_DISABLE);
	inicializarCacheArchivos();
	configSignal();

	ret=readBootSector();
	fat_table = malloc((int) bs->sectoresXfat * bs->bytesXsector);
	ret=levantarFAT();
	clustersFatActualizados = collection_list_create();

	ret = pthread_create(&threadConsola, NULL,(void *) &consolaThread, (NULL));
	if (ret){
		printf("ERROR; return code from pthread_create() is %d\n", ret);
		return EXIT_FAILURE;
	}


//	free(fat_table);
//	free(bs);
//	close(fd);
//	return 0;

	return fuse_main(argc, argv, &pfs_oper, NULL);

}
