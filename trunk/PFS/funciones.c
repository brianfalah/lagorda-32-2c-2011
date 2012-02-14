#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fuse.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include "utils.h"
#include "funciones.h"
#include "mapeoArchivos.h"

extern int fd;
extern pool poolConexiones;

int readBootSector(void){
	int ret=0;
	char *mapeo = NULL;

	mapeo = malloc (512 * 4);

//	if (fstat(fd, &statbuf) < 0){
//		printf("Error fstat()");
//		return EXIT_FAILURE;
//	}

	//char* mapeo = mmap(NULL, 4096, PROT_READ, MAP_SHARED, fd, 0);

	//ret = posix_madvise (mapeo, 1024, POSIX_MADV_SEQUENTIAL);

	//pedir bloque 0, y utilizar solo el sector 1

	ret = pedirBootSector(mapeo);
	if (ret) return ret;
	//memcpy(buf, mapeo, 36);
	//memcpy(bs->jumpInstruction,mapeo,3);
	//memcpy(bs->OEM,mapeo+3,8);
	bs ->bytesXsector = swap_uint16((mapeo[11] << 8) + mapeo[12]);
	bs->sectorXcluster=mapeo[13];
	bs->cantReservedSector= swap_uint16((mapeo[14] << 8) + mapeo[15]);

	bs->noFat=mapeo[16];
	memcpy(bs->rootDirectoryEntries,mapeo+17,2);
	memcpy(bs->totalSectors,mapeo+19,2);
	bs->mediaDescriptor=mapeo[21];
	//memcpy(bs->sectoresXpista,mapeo+24,2);
	bs->sectoresXpista=(mapeo[25]<<8)+mapeo[24];
	//memcpy(bs->noCabezal,mapeo+26,2);
	bs->noCabezas = (mapeo[27]<<8)+mapeo[26];
	bs->hiddenSectors = ((mapeo[31]<<24)+(mapeo[30]<<16)+(mapeo[29]<<8)+(mapeo[28]));
	bs->totalSectors2 = swap_uint32((mapeo[32]<<24)+(mapeo[33]<<16)+(mapeo[34]<<8)+(mapeo[35]));

	//Extended BIOS Parameter Block
	bs->sectoresXfat = swap_uint32((mapeo[36]<<24)+(mapeo[37]<<16)+(mapeo[38]<<8)+(mapeo[39]));
	bs->rootDirectoryStart = ((mapeo[47]<<24)+(mapeo[46]<<16)+(mapeo[45]<<8)+(mapeo[44]));
	bs->FSInfoSector = (mapeo[49]<<8)+mapeo[48]; //numero de sector del Fyle System Information Sector, donde tenemos q setear Number of free clusters on the drive en -1 y Number of the most recently allocated cluster en 0xFFFFFFFF
	//finaliza el boot sector(lo q nos interesa)

	//comienza File System Information Sector en el 512
	//mapeo[1003]='1'; quiero setear en -1 los free sectors no puedo!
	//if (msync (mapeo+1003, 4, MS_ASYNC) == -1)
	//perror ("msync");

	bs->freeClusters = ((mapeo[1003]<<24)+(mapeo[1002]<<16)+(mapeo[1001]<<8)+(mapeo[1000]));

	free(mapeo);
	return EXIT_SUCCESS;
}


int levantarFAT(void){
	int ret=0,offset=0,cluster=0,i=0;
	unsigned int entrada = 0;
	uint32_t cantClusters = 0;
	uint32_t tamDeCluster = bs->bytesXsector * bs->sectorXcluster;
	char *dataCluster = malloc(tamDeCluster);

	char *temp=malloc(4);

	offset=bs->cantReservedSector*512; //ubicacion de la fat, empieza en cluster 4
	cantClusters = bs->sectoresXfat/bs->sectorXcluster;
	//cantBloques = calcularCantBloques(sectoresXFat);

	// juntar e ir leyendo de a 4 bytes

	for (cluster=0;cluster < cantClusters;cluster++){
		//ACA ADENTRO pedirCluster(4 + cluster) (y empezar cluster desde 0)
		ret = pedirClusterDeReservedOFatRegion(4 + cluster ,dataCluster);

		for(i=0; i<1024; i++){
			memcpy(temp,dataCluster+(i*4),4);
			fat_table[entrada] = swap_int32( (uint32_t)traducirSectorAUnsignedInt(temp) );
			if (fat_table[entrada] == 0x00000000){
				cantClusLibres++;
			}

			entrada++;
		}
		offset=offset+4096;
	}

	free(temp);
	free(dataCluster);

	return 0;
}

//t_list *listaClustersLibres(int sectoresXFat,int cantReservedSectors,int *cantClustersLibres){
//	int ret=0,offset,cantClusters,cluster,i,cantBloques;
//	unsigned int entrada = 0;
//	t_list * lista = collection_list_create();
//	//tipoCluster* clusterFat = malloc(sizeof(tipoCluster));
//
//	offset=cantReservedSectors*512; //ubicacion de la fat, empieza en cluster 4
//	cantClusters = sectoresXFat/8;
//	cantBloques = calcularCantBloques(sectoresXFat);// para dps
//	//pedirBloques()...
//
//	for (cluster=1;cluster <= cantClusters;cluster++){
//
//		for(i=0; i<=4088; i=i+4){
//			if (fat_table[entrada] == 0x00000000){
//				(*cantClustersLibres)++;
//		        //clusterFat-> numero = entrada;
//		        collection_list_add(lista,entrada);
//			}
//			entrada++;
//		}
//		offset=offset+4096;
//	}
//
//	//free(clusterFat);
//
//	return 0;
//}

int recorrerEntradasPath(const char* path, uint32_t* numeroClusterInfo, uint32_t* numeroClusterEntrada, uint32_t *posBytesEntrada) {
	char *delimitadores = "/";
	//char *token = malloc(13 + 1);
	char *token; //por lo q vi, no hay q reservar memoria para los strtok
	char *pathParaTokenizar = NULL;
	uint32_t numCluster = bs->rootDirectoryStart, clusterAnterior = 0; //inicializa en el cluster del direcorio raiz que es 2
	int entradaEncontrada = 0;
	char *resto;

//	pathParaTokenizar=strdup(path);
	pathParaTokenizar = malloc(strlen(path) + 1);
	strcpy(pathParaTokenizar,path);
	token = strtok_r(pathParaTokenizar, delimitadores, &resto);

	if (token == NULL) {
		*numeroClusterInfo = numCluster;
		//ret = pedirCluster(cluster, numCluster);

	}

	else {

		do {
			entradaEncontrada = 0;
			while(!entradaEncontrada && clusterAnterior != numCluster){
				clusterAnterior = numCluster;
				entradaEncontrada = buscarEntradaEnDir(&numCluster, token, posBytesEntrada);
			}

			token = strtok_r(NULL,delimitadores, &resto);
		} while (token != NULL && entradaEncontrada != 0);

		if (entradaEncontrada != 1){
			free(pathParaTokenizar);
			return -ENOENT;
		}
		else{
			*numeroClusterEntrada = clusterAnterior;
			*numeroClusterInfo = numCluster;
			//pedir clusters encadenados del archivo/carpeta y hacer lo que corresponda(por ej:listardir o devolver el archivo)
		}

	}

	//free(token);
	free(pathParaTokenizar);
	return EXIT_SUCCESS;
}

int buscarEntradaEnDir(uint32_t *numCluster, char * nombreEntrada, uint32_t *posBytesEntrada){
	char *cluster = malloc((bs->bytesXsector * bs->sectorXcluster) );
	uint16_t *nombreLfnEntry = malloc(26);
	uint32_t tamNombreUtf16 = 0;
	char *nombreEnUtf8 = NULL, *strProxCluster = malloc(4);
	tipoLFN *lfnEntry = malloc(32);
	tipoDirectorio *dirEntry = malloc(32);
	int i = 0,it = 0,it2=0, comienzoLectura = 0, encontrada = 0, ret = 0;
	size_t tamanoNombreEnUtf8=0;
	unsigned int cantEntradasDobles = 0;
	t_list* listaClustersDelDir = collection_list_create();
	uint32_t *clusterSacado = NULL;

	ret = listaClustersEncadenados(*numCluster, listaClustersDelDir, 0);
	if(ret) return -ENOENT;

	while(collection_list_size(listaClustersDelDir)!=0 && !encontrada){

		clusterSacado = collection_list_remove(listaClustersDelDir,0);
		cantEntradasDobles = bs->sectorXcluster * bs->bytesXsector / 64 ; //en c/ cluster se almacenarán como maximo 64 arch/dir - 2(. y ..) porq siempre les crearemos una LFNENTRY
		ret = pedirCluster(*clusterSacado, cluster);
		if (*numCluster!=2){
			cantEntradasDobles = cantEntradasDobles -2; //nos salteamos las 2 entradas . y .. q NO TIENEN LFNEntries, por lo q ocupan 64 bytes
			comienzoLectura = 64;
		}
		while(i < cantEntradasDobles && !encontrada){
			memcpy(lfnEntry,(cluster + comienzoLectura)+ i*64, 32 ); //primero estan las LFNEntries
			memcpy(dirEntry,(cluster + comienzoLectura)+ i*64 + 32, 32 ); //y dps las normales

			//comparar nombreEntrada con el de la lfnEntry, si coinciden, encontrada, armar dir cluster
			memcpy(nombreLfnEntry,lfnEntry->nombre1,5 * sizeof(uint16_t));
			//memcpy(nombreLfnEntry+ 5 * sizeof(uint16_t),lfnEntry->nombre2,6 * sizeof(uint16_t));
			for (it=0;it<6;it++){
				nombreLfnEntry[it + 5]= lfnEntry->nombre2[it];
			}

			for (it2=0;it2<2;it2++){
				nombreLfnEntry[it2 + 11]= lfnEntry->nombre3[it2];
			}
			tamNombreUtf16 = unicode_strlen(nombreLfnEntry);
			nombreEnUtf8 = malloc(100); //TODO VER BIEN ESTO, VALGRIND TIRA MUCHO MEMORY LEAK
			unicode_utf16_to_utf8_inbuffer(nombreLfnEntry,tamNombreUtf16 * sizeof(uint16_t),nombreEnUtf8,&tamanoNombreEnUtf8);

			nombreEnUtf8[13]='\0';

			if(strcmp(nombreEntrada,nombreEnUtf8) == 0){
				encontrada=1; //entrada encontrada!
				*posBytesEntrada = primerSectorDeCluster(*clusterSacado)*bs->bytesXsector + comienzoLectura + i*64;//ver si esta bien el valor de esto
				memcpy(strProxCluster, dirEntry->bytesBajosPrimerClust,2);
				memcpy(strProxCluster+2,dirEntry->bytesAltosPrimerClust,2);
				*numCluster = swap_int32( (uint32_t)traducirSectorAUnsignedInt(strProxCluster));
				//tamano =  swap_int32( (uint32_t)traducirSectorAUnsignedInt(dirEntry->tamano) );
			}
			i++;
			free(nombreEnUtf8);
		}
		free(clusterSacado);
	}

	//collection_list_destroy(listaClustersDelDir,free);

	free(lfnEntry);
	free(dirEntry);
	free(cluster);
	free(strProxCluster);
	free(nombreLfnEntry);
	//free(nombreEnUtf8);
	free(listaClustersDelDir);
	return encontrada;
}

int listarEntradasDeCluster(uint32_t numCluster, t_list* listaEntradas){
	char *cluster = malloc((bs->bytesXsector * bs->sectorXcluster) );
	uint16_t *nombreLfnEntry = malloc(26);
	uint32_t tamNombreUtf16 = 0;
	char *nombreEnUtf8 = NULL;
	tipoLFN *lfnEntry = malloc(32);
	tipoDirectorio *dirEntry = malloc(32);
	int i = 0,it = 0,it2=0, comienzoLectura = 0, encontrada = 0, ret = 0;
	size_t tamanoNombreEnUtf8=0;
	unsigned int cantEntradasDobles = 0;
	t_list* listaClustersDelDir = collection_list_create();
	uint32_t* numDeCluster = NULL;

	ret = listaClustersEncadenados(numCluster, listaClustersDelDir, 0);

	while(collection_list_size(listaClustersDelDir)!=0){
		numDeCluster = collection_list_remove(listaClustersDelDir,0);

		cantEntradasDobles = bs->sectorXcluster * bs->bytesXsector / 64 ; //en c/ cluster se almacenarán como maximo 64 arch/dir - 2(. y ..) porq siempre les crearemos una LFNENTRY
		ret = pedirCluster(*numDeCluster, cluster);
		if (*numDeCluster!=2){
			cantEntradasDobles = cantEntradasDobles -2; //nos salteamos las 2 entradas . y .. q NO TIENEN LFNEntries, por lo q ocupan 64 bytes
			comienzoLectura = 64;
		}
		while(i < cantEntradasDobles){
			memcpy(lfnEntry,(cluster + comienzoLectura)+ i*64, 32 ); //primero estan las LFNEntries
			memcpy(dirEntry,(cluster + comienzoLectura)+ i*64 + 32, 32 ); //y dps las normales
			nombreEnUtf8 = malloc(100);
			if( dirEntry->nombreUTF8[0] != 0x00 && dirEntry->nombreUTF8[0]!=0xE5 && dirEntry->nombreUTF8[0]!=(-27) ){ //si la entrada no esta vacia ni borrada
				memcpy(nombreLfnEntry,lfnEntry->nombre1,5 * sizeof(uint16_t));

				for (it=0;it<6;it++){
					nombreLfnEntry[it + 5]= lfnEntry->nombre2[it];
				}

				for (it2=0;it2<2;it2++){
					nombreLfnEntry[it2 + 11]= lfnEntry->nombre3[it2];
				}

				tamNombreUtf16 = unicode_strlen(nombreLfnEntry);

				//tamanoNombreEnUtf8 = tamNombreUtf16;
				unicode_utf16_to_utf8_inbuffer(nombreLfnEntry,tamNombreUtf16 * sizeof(uint16_t),nombreEnUtf8,&tamanoNombreEnUtf8);

				nombreEnUtf8[13] = '\0';
				collection_list_add(listaEntradas,nombreEnUtf8);
			}
			else{
				free(nombreEnUtf8);
			}
			i++;

		}
		free(numDeCluster);
	}

	free(lfnEntry);
	free(dirEntry);
	free(cluster);
	free(nombreLfnEntry);
	//nombreEnUtf8 = NULL;
	//free(nombreEnUtf8);
	free(listaClustersDelDir);
	return encontrada;

}

int listaClustersEncadenados(uint32_t numCluster,t_list* listaClusters, int orden){
	int finCadena = 0, tipoEntrada = 0;
	uint32_t *cluster = NULL;
	//tipoCluster* clusterFat = malloc(sizeof(tipoCluster));

	//listaClusters = collection_list_create();
	pthread_mutex_lock(&semFat);
	if(orden == 0){
		while(!finCadena){
			tipoEntrada = tipoDeEntradaFat( fat_table[numCluster] );
			if (tipoEntrada == 5 ){
				finCadena = 1;
				cluster = malloc(sizeof(uint32_t));
				*cluster = numCluster;
				collection_list_add(listaClusters,cluster);
			}
			else{
				if(tipoEntrada == 4){
					cluster = malloc(sizeof(uint32_t));
					*cluster = numCluster;
					collection_list_add(listaClusters,cluster);
					numCluster = fat_table[numCluster]; //paso al proximo en la cadena
				}
				else{
					pthread_mutex_unlock(&semFat);
					return -ENOENT;
				}
			}
		}
	}
	else{
		while(!finCadena){
			tipoEntrada = tipoDeEntradaFat( fat_table[numCluster] );
			if (tipoEntrada == 5 ){
				finCadena = 1;
				cluster = malloc(sizeof(uint32_t));
				*cluster = numCluster;
				collection_list_put(listaClusters, 0, cluster);
			}
			else{
				if(tipoEntrada == 4){
					cluster = malloc(sizeof(uint32_t));
					*cluster = numCluster;
					collection_list_put(listaClusters, 0, cluster);
					numCluster = fat_table[numCluster]; //paso al proximo en la cadena
				}
				else{
					pthread_mutex_unlock(&semFat);
					return -ENOENT;
				}
			}
		}
	}
	pthread_mutex_unlock(&semFat);

	return EXIT_SUCCESS;
}

int renombrarArch(char *path,char *nuevoNombre, uint32_t clusterDirDestino){
	int ret=0, numBloque = 0, posEntrada;
	uint32_t posBytesEntrada = 0, clusterInfo = 0, clusterEntrada = 0;
	uint32_t nuevoCluster=0;
	//char* ultimaEntradaPath = malloc(13 + 1);
	char *bloque = NULL;
    tipoLFN *lfnEntry = malloc(32);
    tipoDirectorio *dirEntry = malloc(32);
    char primerCaracter;
    uint32_t* clusterSacado = NULL;

    uint32_t tamDeCluster = bs->bytesXsector * bs->sectorXcluster;
	t_list * listaClusters = collection_list_create();
	char *dataCluster = malloc(tamDeCluster);


    ret = recorrerEntradasPath(path, &clusterInfo, &clusterEntrada, &posBytesEntrada);
    if(ret != 0)
        return ret;

    numBloque = bloqueDeUnSector((int)posBytesEntrada / 512);
    posEntrada = posicionEnBloque(posBytesEntrada);
    bloque = malloc(bs->bytesXsector * 4);
    ret = pedirBloque(bloque, numBloque);
    memcpy(lfnEntry, bloque + posEntrada, 32); //primero estan las LFNEntries
    memcpy(dirEntry, bloque + posEntrada + 32, 32); //y dps las normales
    ret = nombrarEntradas(nuevoNombre, lfnEntry, dirEntry);

    //primero borramos la entrada, tal vez se reuse si es un rename normal, sino no, pero las entradas seran las mismas
    primerCaracter = dirEntry->nombreUTF8[0]; //guardamos el primer caracter para dps
    dirEntry->nombreUTF8[0] = 0xE5;
    //actualizamos el bloque
    memcpy(bloque + posEntrada, lfnEntry, 32 );
	memcpy(bloque + posEntrada + 32, dirEntry, 32 );
	ret = escribirBloque(bloque,numBloque);

	ret = listaClustersEncadenados(clusterDirDestino,listaClusters, 0);
	clusterSacado = collection_list_remove(listaClusters,collection_list_size(listaClusters)-1 );
	ret = pedirCluster(*clusterSacado,dataCluster);
	ret = buscarEntradasLibres(dataCluster, &posBytesEntrada);
	if (!ret){ //si no encontro entradas libres,asignar nuevo cluster

		nuevoCluster = asignarClusterLibreA(*clusterSacado);
		if (nuevoCluster == 0) return -1; //buscar error
		//fat_table[nuevoCluster]= 0x0FFFFFFF; //EOC
		*clusterSacado = nuevoCluster;
		ret = pedirCluster(*clusterSacado, dataCluster);
		posBytesEntrada=0;
	}

	dirEntry->nombreUTF8[0]= primerCaracter;

	memcpy(dataCluster + posBytesEntrada,lfnEntry,32);
	memcpy(dataCluster + posBytesEntrada + 32,dirEntry,32);

	ret = escribirCluster(*clusterSacado, dataCluster);

	free(clusterSacado);
	free(bloque);
	free(lfnEntry);
	free(dirEntry);
	free(dataCluster);
	collection_list_destroy(listaClusters, &free);
	//free(listaClusters);
	return ret;
}

int getAttributes(const char *path, struct stat *stbuf){
	int ret=0, numBloque = 0, posEntrada=0, entradaEncontrada=0;
	uint32_t posBytesEntrada = 0, clusterInfo = 0,tamanoEnBytes=0;
	int  clusterEntrada = 0;
	char *bloque = NULL;
	tipoLFN *lfnEntry = NULL;
	tipoDirectorio *dirEntry = NULL;

	memset(stbuf, 0, sizeof(struct stat));
	if(strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}
	else{
		entradaEncontrada = recorrerEntradasPath(path, &clusterInfo,&clusterEntrada, &posBytesEntrada);
		if(entradaEncontrada != 0){
			return ret = -ENOENT; //entrada no encontrada
		}
		else{
			lfnEntry=malloc(32);
			dirEntry=malloc(32);
			numBloque = bloqueDeUnSector((int)posBytesEntrada/512);
			posEntrada = posicionEnBloque(posBytesEntrada);
			bloque = malloc(bs->bytesXsector * 4);
			ret = pedirBloque(bloque, numBloque);
			//memcpy(lfnEntry,bloque + posEntrada, 32 ); //primero estan las LFNEntries
			memcpy(dirEntry,bloque + posEntrada + 32, 32 ); //y dps las normales
			if (dirEntry->atributos == 0x10) { //no puedo compararlo con el macro SUBDIRECTORIO??
				stbuf->st_mode =  S_IFDIR | 0755;
			}
			else{
				stbuf->st_mode =  S_IFREG | 0755;//sino, es un arhivo normal
			}

			stbuf->st_nlink = 1;
			tamanoEnBytes = dirEntry->tamano;
			stbuf->st_size = tamanoEnBytes;
			//stbuf->st_blksize = 2048;//tamaño del bloque, para que se necesita esto?
			//stbuf->st_blocks = (tamanoEnBytes / 2048);


			free(bloque);
			free(lfnEntry);
			free(dirEntry);
		}
	}

	return 0;
}

//usada para el read, cargamos el buffer con el size pedido, a partir de offset
int leerYCargar(uint32_t numCluster,char *buf,size_t size,off_t offset, char *path){
	int ret=0, primerClusterAPedir=0, i=0;
	uint32_t offsetEnCluster = 0, tamanoTemporal=0;
	uint32_t *unNumDeCluster = NULL;
	uint32_t restantes=0, tamDeCluster = bs->bytesXsector * bs->sectorXcluster;
	t_list * listaClusters = collection_list_create();
	char *dataCluster = malloc(tamDeCluster);

	primerClusterAPedir = offset/ tamDeCluster;
	offsetEnCluster = offset % tamDeCluster;
	ret = listaClustersEncadenados(numCluster,listaClusters,1);

//	while(i != primerClusterAPedir){ //sacamos los clusters q no nos sirvan de la lista
//		unNumDeCluster = collection_list_remove(listaClusters,0);
//		i++;
//		free(unNumDeCluster);
//	}
	while(tamanoTemporal < size){ //ir pidiendo a partir de offset hasta llenar buf con el size deseado
		restantes = size - tamanoTemporal;
		//ret = pedirCluster((uint32_t)collection_list_remove(listaClusters,0),dataCluster);
		unNumDeCluster = collection_list_remove(listaClusters,collection_list_size(listaClusters) -1 - primerClusterAPedir);
		ret = leerCacheArchivos(path, *unNumDeCluster, dataCluster);
		free(unNumDeCluster);
		memcpy(buf + tamanoTemporal, dataCluster + offsetEnCluster,restantes >= tamDeCluster-offsetEnCluster? tamDeCluster-offsetEnCluster:restantes);
		tamanoTemporal = tamanoTemporal + (restantes >= tamDeCluster-offsetEnCluster? tamDeCluster-offsetEnCluster:restantes);
		offsetEnCluster = 0;
	}

	//free(listaClusters);
	collection_list_destroy(listaClusters, &free);
	free(dataCluster);
	return tamanoTemporal;
}

int truncarArchivo(uint32_t clusterEntrada,uint32_t posBytesEntrada,uint32_t clusterInfo,off_t nuevoTamano, char *path){
	int ret=0,primClus=0, numBloque = 0, posEntrada=0, nuevoTotalClusters=0, i=0;
	uint32_t unNumDeCluster=0, nuevoCluster=0;
	int primerClusterAPedir=0, offsetEnCluster = 0;
	char *bloque = NULL;// = malloc((int)bs->bytesXsector * 4);
	tipoDirectorio *dirEntry = malloc(32);
	uint32_t tamDeCluster = bs->bytesXsector * bs->sectorXcluster;
	uint32_t tamanoViejo = 0;
	t_list * listaClusters = collection_list_create();
	char *dataCluster = malloc(tamDeCluster);
	uint32_t *clusterSacado = NULL;

	numBloque = bloqueDeUnSector((int)posBytesEntrada/512);
	posEntrada = posicionEnBloque(posBytesEntrada);
	bloque = malloc(bs->bytesXsector * 4);
	ret = pedirBloque(bloque, numBloque);
	memcpy(dirEntry,bloque + posEntrada + 32, 32 ); //y dps las normales

	//tamanoViejo = swap_int32( (uint32_t)traducirSectorAUnsignedInt(dirEntry->tamano));
	tamanoViejo = dirEntry->tamano;
	//memset(dirEntry->tamano,'\0',4);
	dirEntry->tamano = nuevoTamano; //VER si asi se guarda bien

	//actualizamos el bloque
	memcpy(bloque + posEntrada + 32, dirEntry, 32 );
	ret = escribirBloque(bloque,numBloque);

	if(nuevoTamano > tamanoViejo ){ //hay que agrandar, seteando en /0 y pedir mas clusters si es necesario
		nuevoTotalClusters = (nuevoTamano - 1) / tamDeCluster + 1;
		offsetEnCluster = tamanoViejo % tamDeCluster;
		ret = listaClustersEncadenados(clusterInfo,listaClusters, 0);
		clusterSacado = collection_list_get(listaClusters,collection_list_size(listaClusters)-1); //el ultimo del arch
		//ret = pedirCluster(clusterInfo,dataCluster);
		ret = leerCacheArchivos(path, *clusterSacado, dataCluster);

		if( nuevoTotalClusters == collection_list_size(listaClusters) ){//no necesitamos mas clusters
			memset(dataCluster + offsetEnCluster,'\0', nuevoTamano - tamanoViejo);
			//ret = escribirCluster(clusterInfo,dataCluster);
			clusterSacado = collection_list_remove(listaClusters,collection_list_size(listaClusters)-1);
			ret = escribirCacheArchivos(path, *clusterSacado, dataCluster);
			free(clusterSacado);
		}
		else{// hay que asignarle mas, seteamos el ultimo actual hasta el final en /0 salvo que este lleno, y todos los nuevos
			if( (offsetEnCluster != 0) || (tamanoViejo == 0) ){
				memset(dataCluster + offsetEnCluster,'\0', tamDeCluster - offsetEnCluster);
				//ret = escribirCluster(clusterInfo,dataCluster);
				ret = escribirCacheArchivos(path, *clusterSacado, dataCluster);
			}

			i=0;
			while(i < (nuevoTotalClusters - collection_list_size(listaClusters)) ) {
				bzero(dataCluster,tamDeCluster);
				nuevoCluster = asignarClusterLibreA(*clusterSacado);
				//fat_table[clusterInfo] = nuevoCluster;
				ret = escribirCacheArchivos(path, nuevoCluster, dataCluster);
				*clusterSacado = nuevoCluster;
//				if (i == nuevoTotalClusters - collection_list_size(listaClusters) - 1){
//					pthread_mutex_lock(&semFat);
//					fat_table[clusterInfo] = 0x0FFFFFFF; //EOC
//					pthread_mutex_unlock(&semFat);
//				}
				i++;
			}
		}
	}
	else{
		//hacer esto si el nuevo tamaño es menor que antes
		//PARA MI NO HACE FALTA SETEAR EN \0 EL RESTO DEL ARCHIVO
		//HAY QUE LIBERAR LOS CLUSTERS QUE YA NO SE USAN
		if (nuevoTamano == 0){
			primerClusterAPedir = 0;
		}
		else{
			primerClusterAPedir = (nuevoTamano-1) / tamDeCluster;//porq si por ej el nuevo es 4096, necesitaria un solo cluster para el archivo
		}

		offsetEnCluster = tamanoViejo % tamDeCluster;
		ret = listaClustersEncadenados(clusterInfo,listaClusters, 0);

		while(primClus != primerClusterAPedir){ //sacamos los clusters q no nos sirvan de la lista
			clusterSacado = collection_list_remove(listaClusters,0);
			free(clusterSacado);
			primClus++;
		}
		clusterSacado = collection_list_remove(listaClusters,0);
		//if(nuevoTamano >= 0){
			pthread_mutex_lock(&semFat);
			fat_table[*clusterSacado] = 0x0FFFFFFF;
			pthread_mutex_unlock(&semFat);
			free(clusterSacado);
		//}
//		else{
//			fat_table[unNumDeCluster] = 0x00000000; //sino, le liberamos todos los clusters que tenga
//		}

		//ret = pedirCluster(collection_list_remove(listaClusters,0),dataCluster);
		//memset(dataCluster + offsetEnCluster,'\0', tamDeCluster - offsetEnCluster);//seteamos en 0 la parte que no nos sirve mas
		//ret = escribirCluster(unNumDeCluster, dataCluster);

		while(collection_list_size(listaClusters) != 0){ //liberamos el resto de los clusters, sin setearlos en 0, se trunca con \0 solo cuando aumenta
			clusterSacado = collection_list_remove(listaClusters,0);
			ret = liberarCluster(*clusterSacado);
			free(clusterSacado);
			//ret = pedirCluster(collection_list_remove(listaClusters,0),dataCluster);
			//memset(dataCluster,'\0', tamDeCluster);
			//ret = escribirCluster(unNumDeCluster, dataCluster);
		}
	}

	free(bloque);
	free(dirEntry);
	collection_list_destroy(listaClusters, &free); //solo si tengo punteros en la estructura, buscar ejemplo en google
	//free(listaClusters);
	free(dataCluster);
	return EXIT_SUCCESS;
}

int agrandarArchivo(uint32_t clusterEntrada,uint32_t posBytesEntrada,uint32_t clusterInfo,off_t nuevoTamano, char *path){
	int ret=0, numBloque = 0, posEntrada=0, nuevoTotalClusters=0, i=0;
	uint32_t nuevoCluster=0;
	int offsetEnCluster = 0;
	char *bloque = NULL;// = malloc((int)bs->bytesXsector * 4);
	tipoDirectorio *dirEntry = malloc(32);
	uint32_t tamDeCluster = bs->bytesXsector * bs->sectorXcluster;
	uint32_t tamanoViejo = 0;
	t_list * listaClusters = collection_list_create();
	uint32_t* clusterSacado = NULL;

	numBloque = bloqueDeUnSector((int)posBytesEntrada/512);
	posEntrada = posicionEnBloque(posBytesEntrada);
	bloque = malloc(bs->bytesXsector * 4);
	ret = pedirBloque(bloque, numBloque);
	memcpy(dirEntry,bloque + posEntrada + 32, 32 ); //y dps las normales

	//tamanoViejo = swap_int32( (uint32_t)traducirSectorAUnsignedInt(dirEntry->tamano));
	tamanoViejo = dirEntry->tamano;
	//memset(dirEntry->tamano,'\0',4);
	dirEntry->tamano = nuevoTamano; //VER si asi se guarda bien

	//actualizamos el bloque
	memcpy(bloque + posEntrada + 32, dirEntry, 32 );
	ret = escribirBloque(bloque,numBloque);

	nuevoTotalClusters = (nuevoTamano - 1) / tamDeCluster + 1;
	offsetEnCluster = tamanoViejo % tamDeCluster;
	ret = listaClustersEncadenados(clusterInfo,listaClusters, 0);
	clusterSacado = collection_list_get(listaClusters,collection_list_size(listaClusters)-1); //el ultimo del arch
	//ret = pedirCluster(clusterInfo,dataCluster);
	//ret = leerCacheArchivos(path, clusterInfo, dataCluster);

	if( nuevoTotalClusters == collection_list_size(listaClusters) ){//no necesitamos mas clusters

	}
	else{// hay que asignarle mas
		i=0;
		clusterInfo = *clusterSacado;
		while(i < (nuevoTotalClusters - collection_list_size(listaClusters)) ) {
			nuevoCluster = asignarClusterLibreA(clusterInfo);
			clusterInfo = nuevoCluster;
			i++;
		}

	}


	free(bloque);
	free(dirEntry);
	collection_list_destroy(listaClusters, &free); //solo si tengo punteros en la estructura, buscar ejemplo en google
	//free(listaClusters);
	return EXIT_SUCCESS;
}

int escribirArchivo(uint32_t clusterInfo,char* buf,uint32_t size,uint32_t offset, char *path){
	int ret=0,primClus=0;
	uint32_t unNumDeCluster=0, tamanoTemporal=0;
	int restantes=0, primerClusterAPedir, offsetEnCluster = 0;
	uint32_t tamDeCluster = bs->bytesXsector * bs->sectorXcluster;
	t_list * listaClusters = collection_list_create();
	char *dataCluster = malloc(tamDeCluster);
	uint32_t *clusterSacado = NULL;

	primerClusterAPedir = offset / tamDeCluster;
	offsetEnCluster = offset % tamDeCluster;
	ret = listaClustersEncadenados(clusterInfo,listaClusters, 0);

	while(primClus != primerClusterAPedir){ //sacamos los clusters q no nos sirvan de la lista
		clusterSacado = collection_list_remove(listaClusters,0);
		primClus++;
		free(clusterSacado);
	}

	while(tamanoTemporal < size){ //ir escribiendo a partir de offset hasta vaciar buf con el size deseado
		restantes = size - tamanoTemporal;
		clusterSacado = collection_list_remove(listaClusters,0);
		//ret = pedirCluster(unNumDeCluster, dataCluster);
		ret = leerCacheArchivos(path, *clusterSacado, dataCluster);
		memcpy(dataCluster + offsetEnCluster, buf, restantes >= tamDeCluster-offsetEnCluster? tamDeCluster-offsetEnCluster:restantes);
		tamanoTemporal = tamanoTemporal + (restantes >= tamDeCluster-offsetEnCluster? tamDeCluster-offsetEnCluster:restantes);
		//ret = escribirCluster(unNumDeCluster, dataCluster);
		ret = escribirCacheArchivos(path, *clusterSacado, dataCluster);
		offsetEnCluster = 0;
		free(clusterSacado);
	}

	//free(listaClusters);
	collection_list_destroy(listaClusters, &free);
	free(dataCluster);
	return tamanoTemporal;
}

int crearDirectorio(uint32_t clusterInfo, char* nombreDir){
	int ret=0,i=0;
	uint32_t tamDeCluster = bs->bytesXsector * bs->sectorXcluster, posBytesEntrada=0;
	t_list * listaClusters = collection_list_create();
	char *dataCluster = malloc(tamDeCluster);
	tipoDirectorio *dirEntry = malloc(32);
	tipoLFN *lfnEntry = malloc(32);
	tipoDirectorio *entradaPunto = malloc(32);
	tipoDirectorio *entradaDoblePunto = malloc(32);
	uint32_t nuevoCluster=0;
	uint32_t clusterDoblePunto = clusterInfo, clusterPunto = 0;
	uint32_t* clusterSacado = NULL;

	ret = listaClustersEncadenados(clusterInfo,listaClusters, 0);
	clusterSacado = collection_list_remove(listaClusters,collection_list_size(listaClusters)-1 );
	ret = pedirCluster(*clusterSacado,dataCluster);
	ret = buscarEntradasLibres(dataCluster, &posBytesEntrada);
	if (!ret){ //si no encontro entradas libres,asignar nuevo cluster
		//nuevoCluster = collection_list_remove(listaClustersLibres, 0);
		nuevoCluster = asignarClusterLibreA(*clusterSacado); //getClusterLibre();
		if (nuevoCluster == 0) return -1; //buscar error
		//fat_table[nuevoCluster]= 0x0FFFFFFF; //EOC
		*clusterSacado = nuevoCluster;
		ret = pedirCluster(nuevoCluster,dataCluster);
		bzero(dataCluster, tamDeCluster); //por si quedo algo borrado de antes
		posBytesEntrada=0;
	}

	memset(lfnEntry,'\0',32);
	memset(dirEntry,'\0',32);
	lfnEntry->sequenceNumber = 0x41;//en el WInhex se crean asi
	ret = nombrarEntradas(nombreDir,lfnEntry,dirEntry);
	lfnEntry->atributos = 0x0F;//siempre para LFNs
	dirEntry->atributos = 0x10;//subdirectory
	dirEntry->tamano = 0;

	nuevoCluster = getClusterLibre();
	if (nuevoCluster == 0) return -1; //buscar error
	ret = asignarClusterAEntrada(dirEntry,nuevoCluster);
	clusterPunto = nuevoCluster;

	memcpy(dataCluster + posBytesEntrada,lfnEntry,32);
	memcpy(dataCluster + posBytesEntrada + 32,dirEntry,32);
	//fat_table[nuevoCluster]= 0x0FFFFFF8; //EOC
	ret = escribirCluster(*clusterSacado, dataCluster);

	//agregar las entradas . y ..
	bzero(dataCluster,4096);
	ret = pedirCluster(nuevoCluster,dataCluster);
	memset(entradaPunto,'\0',32);
	memset(entradaDoblePunto,'\0',32);

	ret = asignarClusterAEntrada(entradaPunto,clusterPunto);
	entradaPunto->tamano = 0;
	entradaPunto->atributos = 0x10;
	entradaPunto->nombreUTF8[0] = '.';
	for( i = 1 ; i <= 10 ; i++ ){
		entradaPunto->nombreUTF8[i] = 0x20;
	}

	ret = asignarClusterAEntrada(entradaDoblePunto,clusterDoblePunto);
	entradaDoblePunto->tamano = 0;
	entradaDoblePunto->atributos = 0x10;
	entradaDoblePunto->nombreUTF8[0] = '.';
	entradaDoblePunto->nombreUTF8[1] = '.';
	for( i = 2 ; i <= 10 ; i++ ){
		entradaDoblePunto->nombreUTF8[i] = 0x20;
	}

	memcpy(dataCluster, entradaPunto,32);
	memcpy(dataCluster + 32, entradaDoblePunto,32);

	ret = escribirCluster(nuevoCluster, dataCluster);

	//free(listaClusters);
	free(clusterSacado);
	collection_list_destroy(listaClusters, &free);
	free(dirEntry);
	free(lfnEntry);
	free(entradaPunto);
	free(entradaDoblePunto);
	free(dataCluster);

	return EXIT_SUCCESS;
}

int crearArchivo(uint32_t clusterInfo, char* nombreArch){
	int ret=0;
	uint32_t tamDeCluster = bs->bytesXsector * bs->sectorXcluster, posBytesEntrada=0;
	t_list * listaClusters = collection_list_create();
	char *dataCluster = malloc(tamDeCluster);
	tipoDirectorio *dirEntry = malloc(32);
	tipoLFN *lfnEntry = malloc(32);
	uint32_t nuevoCluster=0;
	uint32_t* clusterSacado = NULL;

	ret = listaClustersEncadenados(clusterInfo,listaClusters, 0);
	clusterSacado = collection_list_remove(listaClusters,collection_list_size(listaClusters)-1 );
	ret = pedirCluster(*clusterSacado,dataCluster);
	ret = buscarEntradasLibres(dataCluster, &posBytesEntrada);
	if (!ret){ //si no encontro entradas libres,asignar nuevo cluster
		//nuevoCluster = collection_list_remove(listaClustersLibres, 0);
		nuevoCluster = asignarClusterLibreA(*clusterSacado);
		if (nuevoCluster == 0) return -1; //buscar error
		//fat_table[nuevoCluster]= 0x0FFFFFFF; //EOC
		*clusterSacado = nuevoCluster;
		ret = pedirCluster(nuevoCluster,dataCluster);
		posBytesEntrada=0;
	}

	memset(lfnEntry,'\0',32);
	memset(dirEntry,'\0',32);
	lfnEntry->sequenceNumber = 0x41;//en el WInhex se crean asi
	ret = nombrarEntradas(nombreArch,lfnEntry,dirEntry);
	lfnEntry->atributos = 0x0F;//siempre para LFNs
	dirEntry->atributos = 0x20;//archivo
	dirEntry->tamano = 0;

	nuevoCluster = getClusterLibre();
	if (nuevoCluster == 0) return -1; //buscar error
	//fat_table[nuevoCluster]= 0x0FFFFFF8; //EOC
	ret = asignarClusterAEntrada(dirEntry,nuevoCluster);

	memcpy(dataCluster + posBytesEntrada,lfnEntry,32);
	memcpy(dataCluster + posBytesEntrada + 32,dirEntry,32);

	ret = escribirCluster(*clusterSacado, dataCluster);

	free(clusterSacado);
	free(dirEntry);
	free(lfnEntry);
	free(dataCluster);
	//free(listaClusters);
	collection_list_destroy(listaClusters, &free);

	return EXIT_SUCCESS;
}

int borrarArchivo(uint32_t clusterEntrada,uint32_t posBytesEntrada,uint32_t clusterInfo){
	int ret=0, numBloque = 0, posEntrada=0;
	uint32_t unNumDeCluster=0;
	char *bloque = NULL;// = malloc((int)bs->bytesXsector * 4);
	tipoDirectorio *dirEntry = malloc(32);
	t_list * listaClusters = collection_list_create();
	uint32_t *clusterSacado = NULL;

	numBloque = bloqueDeUnSector((int)posBytesEntrada/512);
	posEntrada = posicionEnBloque(posBytesEntrada);
	bloque = malloc(bs->bytesXsector * 4);
	ret = pedirBloque(bloque, numBloque);
	memcpy(dirEntry,bloque + posEntrada + 32, 32 ); //y dps las normales

	dirEntry->nombreUTF8[0] = 0xE5;

	//actualizamos el bloque
	memcpy(bloque + posEntrada + 32, dirEntry, 32 );
	ret = escribirBloque(bloque,numBloque);

	ret = listaClustersEncadenados(clusterInfo,listaClusters, 0);

	while(collection_list_size(listaClusters) != 0){ //liberamos todos los clusters
		clusterSacado = collection_list_remove(listaClusters,0);
		ret = liberarCluster(*clusterSacado);
		free(clusterSacado);
	}


	free(bloque);
	free(dirEntry);
	//collection_list_destroy(listaClusters, &free); //solo si tengo punteros en la estructura, buscar ejemplo en google
	free(listaClusters);
	return EXIT_SUCCESS;
}

int borrarDirectorio(uint32_t clusterEntrada,uint32_t posBytesEntrada,uint32_t clusterInfo){
	int ret=0, numBloque = 0, posEntrada=0;
	uint32_t unNumDeCluster=0, primerSector = 0;
	char *bloque = NULL;// = malloc((int)bs->bytesXsector * 4);
	tipoDirectorio *dirEntry = malloc(32);
	tipoDirectorio *entradaPunto = malloc(32);
	tipoDirectorio *entradaDoblePunto = malloc(32);
	t_list * listaClusters = collection_list_create();
	uint32_t *clusterSacado = NULL;

	numBloque = bloqueDeUnSector((int)posBytesEntrada/512);
	posEntrada = posicionEnBloque(posBytesEntrada);
	bloque = malloc(bs->bytesXsector * 4);
	ret = pedirBloque(bloque, numBloque);
	memcpy(dirEntry,bloque + posEntrada + 32, 32 ); //y dps las normales

	dirEntry->nombreUTF8[0] = 0xE5;

	//actualizamos el bloque
	memcpy(bloque + posEntrada + 32, dirEntry, 32 );
	ret = escribirBloque(bloque,numBloque);
	free(dirEntry);

	bzero(bloque, bs->bytesXsector * 4);

	primerSector = primerSectorDeCluster(clusterInfo);
	numBloque = bloqueDeUnSector(primerSector);
	ret = pedirBloque(bloque, numBloque);
	memcpy(entradaPunto,bloque, 32);
	memcpy(entradaDoblePunto,bloque + 32, 32);

	entradaPunto->nombreUTF8[0] = 0xE5;
	entradaDoblePunto->nombreUTF8[0] = 0xE5;
	memcpy(bloque, entradaPunto,32);
	memcpy(bloque + 32, entradaDoblePunto, 32);
	ret = escribirBloque(bloque, numBloque);

	ret = listaClustersEncadenados(clusterInfo,listaClusters, 0);

	while(collection_list_size(listaClusters) != 0){ //liberamos todos los clusters
		clusterSacado = collection_list_remove(listaClusters,0);
		ret = liberarCluster(*clusterSacado);
		free(clusterSacado);
	}


	free(entradaPunto);
	free(entradaDoblePunto);
	free(bloque);
	//collection_list_destroy(listaClusters, &free); //solo si tengo punteros en la estructura, buscar ejemplo en google
	free(listaClusters);
	return EXIT_SUCCESS;
}


uint32_t asignarClusterLibreA(uint32_t unCluster){
	uint32_t i=3, cantClusters=0, clusterDeFat=0;
	cantClusters = bs->sectoresXfat/8;
	int tamDeCluster = bs->bytesXsector * bs->sectorXcluster;
	int cantEntradas = cantClusters * tamDeCluster / 4;

	pthread_mutex_lock(&semFat);

	while( (fat_table[i] != 0x00000000) && (i<cantEntradas) ){
		i++;
	}
	if(i==cantEntradas){
		pthread_mutex_unlock(&semFat);
		return 0;
	}
	else{
		fat_table[unCluster] = i;
		fat_table[i] = 0x0FFFFFF8; //le ponemos EOC, tal vez cambie, tal vez no
		cantClusLibres--;

		clusterDeFat = calcularClusterFat(unCluster);
		agregarAClustersAActualizar(clusterDeFat);
		clusterDeFat = calcularClusterFat(i);
		agregarAClustersAActualizar(clusterDeFat);

		pthread_mutex_unlock(&semFat);
		return i;
	}
}

uint32_t getClusterLibre(void){
	uint32_t i=3, cantClusters=0, clusterDeFat=0;
	cantClusters = bs->sectoresXfat/8;
	int tamDeCluster = bs->bytesXsector * bs->sectorXcluster;
	int cantEntradas = cantClusters * tamDeCluster / 4;

	pthread_mutex_lock(&semFat);

	while( (fat_table[i] != 0x00000000) && (i<cantEntradas) ){
		i++;
	}
	if(i==cantEntradas){
		pthread_mutex_unlock(&semFat);
		return 0;
	}
	else{
		cantClusLibres--;
		fat_table[i] = 0x0FFFFFF8; //ahora lo ocupamos

		clusterDeFat = calcularClusterFat(i);
		agregarAClustersAActualizar(clusterDeFat);
		pthread_mutex_unlock(&semFat);
		return i;
	}
}

int liberarCluster(uint32_t unNumDeCluster){
	uint32_t clusterDeFat = 0;

	pthread_mutex_lock(&semFat);
	fat_table[unNumDeCluster] = 0x00000000;
	cantClusLibres++;
	clusterDeFat = calcularClusterFat(unNumDeCluster);
	agregarAClustersAActualizar(clusterDeFat);
	pthread_mutex_unlock(&semFat);

	return EXIT_SUCCESS;
}

//calcular cluster de la fat y agregar a la lista clustersFatActualizados si no esta
int calcularClusterFat(uint32_t unNumDeCluster){
	return (uint32_t)(unNumDeCluster * sizeof(uint32_t)) / (bs->sectorXcluster * bs->bytesXsector);
}

void agregarAClustersAActualizar(uint32_t clusterDeFat){
	uint32_t* clusEnLista = NULL;
	uint32_t* cluster = NULL;
	int i=0, encontrado=0;

	for(i = 0; i < clustersFatActualizados->elements_count; i++) {
		clusEnLista = collection_list_get(clustersFatActualizados, i);
		if(*clusEnLista == clusterDeFat)encontrado=1;
	}
	if(!encontrado){
		cluster = malloc(sizeof(uint32_t));
		*cluster = clusterDeFat;
		collection_list_add(clustersFatActualizados,cluster);
	}
}

//-----------------------CAPA DE DIRECCIONAMIENTO-------------------------


//devuelve el numero del primer sector de un cluster de la data region
int primerSectorDeCluster(int numeroDeCluster){
	int sectorComienzoCluster = 0;
	sectorComienzoCluster = bs->cantReservedSector + bs-> noFat * bs->sectoresXfat + ((numeroDeCluster - 2) * bs->sectorXcluster);

	return sectorComienzoCluster;
}

int primerSectorDeBloque(int numBloque){
	int sectorComienzoBloque = 0;
	sectorComienzoBloque = numBloque * 4;

	return sectorComienzoBloque;
}

//Devuelve el bloque en el que se encuentra un determinado sector
int bloqueDeUnSector(int numSector){
	int bloque = 0, tamanoBloque = bs->bytesXsector * 4;
	unsigned int offset = 0;

	offset = numSector * bs->bytesXsector;

	bloque = offset / tamanoBloque;

	return bloque;
}

//dado los bytes, retorna el offset dentro de un bloque
int posicionEnBloque(uint32_t bytes){
	int tamanoBloque = bs->bytesXsector * 4;
	unsigned int offset = 0;

	offset = bytes % tamanoBloque;

	return offset;
}

//un bloque estará compuesto por 4 sectores
int pedirBloque(char* bloque, int numBloque){
	//char *bloque = (char *)malloc(bs->bytesXsector * 4);
	char *sector = NULL;// = malloc(512);
	int i=0, j=0, ret=0, numSec=0, primerSector=0;
	int posiciones[4];
	//int sectores[4];
	int pos=0;
	int mandadas = 0;
	int recibidas = 0;

	sector = (char *)malloc (bs->bytesXsector);
	primerSector = primerSectorDeBloque(numBloque);

//	if(poolConexiones.cantidadConexiones < 4){

		while(mandadas < 4) {

			//for (i = 0; i < poolConexiones.cantidadConexiones; i++) { //aca habria que hacer todos los send y dps todos los rcv y juntarlos,
			while(pos != -1 && mandadas < 4){
//				if(mandadas < 4) {

					pthread_mutex_lock(&poolConexiones.poolSemaphore);
					pos = buscarSocketLibre();
					pthread_mutex_unlock(&poolConexiones.poolSemaphore);
					if (pos != -1){
						posiciones[mandadas] = pos;
						ret = pedirSector(primerSector + mandadas, pos); //LEER SECTOR CON PROTOCOLO NIPC
						if(ret)return ret;

						mandadas++;
					}
//				}
			}
//			}
			pos = 0;
//			for (i = 0; i < poolConexiones.cantidadConexiones; i++) { //aca habria que hacer todos los send y dps todos los rcv y juntarlos,


			for (i = recibidas; i < mandadas; i++) {
//				if(recibidas < 4) {

					ret = recibirSector(sector, posiciones[i], &numSec); //LEER SECTOR CON PROTOCOLO NIPC
					pthread_mutex_unlock(&poolConexiones.conexiones[posiciones[i]].socketSemaphore);
					if(ret)return ret;
					memcpy(bloque + (bs->bytesXsector * recibidas), sector, bs->bytesXsector);
					recibidas++;

//				}
			}
		}

//			}


//	}
//
//	else {
//		//
//		//printf("Lock Semaforo %p",&poolConexiones.poolSemaphore);
//
//		//printf("Unlock Semaforo %p\n",&poolConexiones.poolSemaphore);
//
//		for (i = 0; i < 4; i++) {
//			pthread_mutex_lock(&poolConexiones.poolSemaphore);
//			pos = buscarSocketLibre();
//			pthread_mutex_unlock(&poolConexiones.poolSemaphore);
//			posiciones[i] = pos;
//			ret = pedirSector(primerSector + i, pos);
//			if(ret)return ret;
////			sectores[i] = primerSector + i;
//		}
//
//		for (i = 0; i < 4; i++) {
//			//readSector(sector, primerSector + i);
//			ret = recibirSector(sector, posiciones[i], &numSec);
//			pthread_mutex_unlock(&poolConexiones.conexiones[posiciones[i]].socketSemaphore);
//			if(ret)return ret;
//			//for(j = 0; j < 4; j++){
//			//	if(sectores[j] == numSec) {
//			memcpy(bloque + (bs->bytesXsector * i), sector, bs->bytesXsector);
//			//	}
//			//}
//		}
//
//		//printf("Unlock Semaforo %p \n",&poolConexiones.conexiones[pos].socketSemaphore);
//
//		//
//	}

	free(sector);
	return 0;
}

int escribirBloque(char *bloque,int numBloque){
	char *sector = NULL;// = malloc(512);
	int primerSector=0, i=0, j=0, ret=0;
	int posiciones[4];
	//int sectores[4];
	int pos=0;
	int mandadas = 0;
	int recibidas = 0;

	sector = (char *)malloc (bs->bytesXsector);
	primerSector = primerSectorDeBloque(numBloque);

//	if(poolConexiones.cantidadConexiones < 4){

		while(mandadas < 4) {

			//for (i = 0; i < poolConexiones.cantidadConexiones; i++) { //aca habria que hacer todos los send y dps todos los rcv y juntarlos,
			while(pos != -1 && mandadas < 4){
				//if(mandadas < 4 && pos != -1) {
					pthread_mutex_lock(&poolConexiones.poolSemaphore);
					pos = buscarSocketLibre();
					pthread_mutex_unlock(&poolConexiones.poolSemaphore);
					if (pos != -1){
						//posMandadas = i;
						posiciones[mandadas] = pos;
						memcpy(sector, bloque + (512 * mandadas), 512);
						ret = mandarSector(sector, primerSector + mandadas, pos); //ESCRIBIRSECTOR CON NIPC
						if (ret)return ret;
						mandadas++;
					}
				//}
			}
			//}
			pos = 0;

			for (i = recibidas; i < mandadas; i++) { //aca habria que hacer todos los send y dps todos los rcv y juntarlos,

//				if(recibidas < 4) {

					ret = recibirConfirmacion(posiciones[i]);
					pthread_mutex_unlock(&poolConexiones.conexiones[posiciones[i]].socketSemaphore);
					if (ret)return ret;
					recibidas++;
					//posRecibidas++;
//				}

			}

		}

//	}
//
//	else{
//		//
//
//		//printf("Lock Semaforo %p",&poolConexiones.poolSemaphore);
//
//		//printf("Unlock Semaforo %p\n",&poolConexiones.poolSemaphore);
//
//		for(i = 0; i < 4; i++){ //habria que hacer todos los send y dps esperar las confirmaciones de cada uno con los rcv
//			pthread_mutex_lock(&poolConexiones.poolSemaphore);
//			pos = buscarSocketLibre();
//			pthread_mutex_unlock(&poolConexiones.poolSemaphore);
//			posiciones[i] = pos;
//			memcpy(sector, bloque + (512 * i), 512);
//			//writeSector(sector, primerSector+i);
//			ret = mandarSector(sector, primerSector + i, pos); //ESCRIBIRSECTOR CON NIPC
//			if (ret)return ret;
//		}
//
//		for(i = 0; i < 4; i++){ //habria que hacer todos los send y dps esperar las confirmaciones de cada uno con los rcv
//			ret = recibirConfirmacion(posiciones[i]);
//			pthread_mutex_unlock(&poolConexiones.conexiones[posiciones[i]].socketSemaphore);
//			if (ret)return ret;
//		}
//
////		printf("Unlock Semaforo %p \n",&poolConexiones.conexiones[pos].socketSemaphore);
//
//
//	}

	free(sector);
	return 0;
}

int pedirSector(uint32_t numSector, int pos){
	char *msg = NULL;
	char *pedido = NULL;
	int ret = 0;
//	int i=0;
	pedido = cargarPedido(numSector, NULL);
	msg = serializarNipc(LECTURA, sizeof(pedidoStr), pedido);

	send(poolConexiones.conexiones[pos].socket, msg, sizeof(NIPC_Header)+ sizeof(pedidoStr),MSG_NOSIGNAL);

	free(pedido);
	free(msg);

	return 0;

}

int recibirSector(char *sector, int pos, int *numSec){
	pedidoStr pedido;// = malloc(sizeof(pedidoStr));
	int ret=0;
	NIPC_Header *head = malloc(sizeof(NIPC_Header));
	//printf("1° Recv\n");
	ret = recv(poolConexiones.conexiones[pos].socket, head, sizeof(NIPC_Header), MSG_WAITALL);
	if(ret <= 0)return -1;//=0 cerro conex <0 error
	//pedido = cargarPedido(sector, NULL);
	//msg = serializarNipc(LECTURA, sizeof(pedidoStr), pedido);

	//pedido->data = malloc(512);
	//printf("2° Recv\n");
	ret =recv(poolConexiones.conexiones[pos].socket, &pedido, head->payloadlength, MSG_WAITALL);
	//printf("Recibiendo por socket posicion %d\n",pos);

	if(ret <= 0)return -1;//=0 cerro conex <0 error

	memcpy(sector, &pedido.data, 512);

	*numSec = (int)pedido.sector;

	free(head);
	//send(poolConexiones.conexiones[i],msg,sizeof(NIPC_PKG)+ sizeof(pedidoStr));

	return 0;
}

int mandarSector(char *dataSector, uint32_t numSector, int pos){
	char *msg = NULL;
	char *pedido = NULL;
	int ret=0;
	pedido = cargarPedido(numSector, dataSector);
	msg = serializarNipc(ESCRITURA, sizeof(pedidoStr), pedido);

	ret = send(poolConexiones.conexiones[pos].socket, msg, sizeof(NIPC_Header) + sizeof(pedidoStr), MSG_NOSIGNAL);
	//printf("Enviado por socket posicion %d\n",pos);
	if(ret <= 0){
		free(pedido);
		free(msg);
		return ret;
	}

	free(pedido);
	free(msg);

	return 0;

}

int recibirConfirmacion(int pos){
	int ret = 0;
	NIPC_Header *head = malloc(sizeof(NIPC_Header));

	ret = recv(poolConexiones.conexiones[pos].socket, head, sizeof(NIPC_Header), MSG_WAITALL);
	if(ret <= 0)return ret;
	//pedido = cargarPedido(sector, NULL);
	//msg = serializarNipc(LECTURA, sizeof(pedidoStr), pedido);

	//pedido->data = malloc(512);

	if(head->type == ERROR){
		free(head);
		return -1;
	}
	else{
		free(head);
		return EXIT_SUCCESS;
	}

	//send(poolConexiones.conexiones[i],msg,sizeof(NIPC_PKG)+ sizeof(pedidoStr));
}

//devuelve todos los bloques juntos a partir de bloque inicial y una determinada cantidad de bloques
int pedirBloques(char *bloques, int bloqueInicial, int cantBloques){
	//char *bloques = malloc((int)bs->bytesXsector * 4 * cantBloques);// esta reserva deberia estar afuera de la funcion
	char *bloque = NULL;
	int i, ret=0;

	bloque = malloc (bs->bytesXsector * 4);
	for(i = 0; i < cantBloques; i++){
		ret = pedirBloque(bloque, bloqueInicial + i);
		if(ret)return ret;
		memcpy(bloques + (bs->bytesXsector * 4 * i),bloque, bs->bytesXsector * 4);
	}

	free(bloque);

	return 0;
}

int pedirCluster(uint32_t numCluster, char *cluster){
	char *bloques = NULL;
	int primerSector=0, primerBloque = 0, cantBloques = 0, ret = 0;

	primerSector = primerSectorDeCluster(numCluster);
	primerBloque = bloqueDeUnSector(primerSector);

	cantBloques = calcularCantBloques(bs->sectorXcluster);

	bloques = malloc(bs->bytesXsector * 4 * cantBloques);
	ret = pedirBloques(bloques, primerBloque, cantBloques);
	if(ret)return ret;
	memcpy(cluster, bloques, bs->bytesXsector * 4 * cantBloques);

	free(bloques);

	return 0;
}

int pedirClusterDeReservedOFatRegion(int numCluster, char *cluster){
	char *bloques = NULL;
	int primerSector=0, primerBloque = 0, cantBloques = 0, ret = 0;

	primerSector = numCluster * bs->sectorXcluster;
	primerBloque = bloqueDeUnSector(primerSector);

	cantBloques = calcularCantBloques(bs->sectorXcluster);

	bloques = malloc(bs->bytesXsector * 4 * cantBloques);
	ret = pedirBloques(bloques, primerBloque, cantBloques);
	if(ret)return ret;
	memcpy(cluster, bloques, bs->bytesXsector * 4 * cantBloques);

	free(bloques);

	return 0;
}

int escribirCluster(uint32_t numCluster, char* dataCluster){
	char *bloque = NULL;
	int primerSector=0, primerBloque = 0, cantBloques = 0, ret = 0,i=0;

	primerSector = primerSectorDeCluster(numCluster);
	primerBloque = bloqueDeUnSector(primerSector);

	cantBloques = calcularCantBloques(bs->sectorXcluster);

	bloque = malloc((uint32_t)bs->bytesXsector * 4);
	for (i=0;i<cantBloques;i++){
		memcpy(bloque, dataCluster+(i * bs->bytesXsector * 4), bs->bytesXsector * 4);
		ret = escribirBloque(bloque,primerBloque+i);
		if(ret)return ret;
	}

	free(bloque);
	bloque = NULL;
	return EXIT_SUCCESS;
}

int pedirBootSector(char* bloque) {
	char *sector = NULL;// = malloc(512);
	int primerSector=0, i=0, j=0, ret=0;
	int posiciones[4];
	int sectores[4];
	int pos=0;
	int numSec=0;
	int mandadas = 0;
	int recibidas = 0;

	sector = (char *)malloc (512);
	primerSector = primerSectorDeBloque(0);

//	if(poolConexiones.cantidadConexiones < 4){

		while(mandadas < 4) {

//			for (i = 0; i < poolConexiones.cantidadConexiones; i++) { //aca habria que hacer todos los send y dps todos los rcv y juntarlos,
			while(pos != -1 && mandadas < 4){
//				if(mandadas < 4) {

					pthread_mutex_lock(&poolConexiones.poolSemaphore);
					pos = buscarSocketLibre();
					pthread_mutex_unlock(&poolConexiones.poolSemaphore);
					if (pos != -1){
						posiciones[mandadas] = pos;
						ret = pedirSector(primerSector + mandadas, pos); //LEER SECTOR CON PROTOCOLO NIPC
						if(ret)return ret;
						mandadas++;
					}
//				}
			}
//			}
			pos = 0;
//			for (i = 0; i < poolConexiones.cantidadConexiones; i++) { //aca habria que hacer todos los send y dps todos los rcv y juntarlos,


			for (i = recibidas; i < mandadas; i++) {
//				if(recibidas < 4) {

					ret = recibirSector(sector, posiciones[i],&numSec); //LEER SECTOR CON PROTOCOLO NIPC
					pthread_mutex_unlock(&poolConexiones.conexiones[posiciones[i]].socketSemaphore);
					if(ret)return ret;
					memcpy(bloque + (512 * recibidas), sector, 512);
					recibidas++;

//				}
			}
//			}

		}

//	}
//
//	else {
//		//
//
//		//printf("Lock Semaforo %p\n",&poolConexiones.poolSemaphore);
//
//		//printf("Unlock Semaforo %p\n",&poolConexiones.poolSemaphore);
//
//		for (i = 0; i < 4; i++) { //aca habria que hacer todos los send y dps todos los rcv y juntarlos,
//			pthread_mutex_lock(&poolConexiones.poolSemaphore);
//			pos = buscarSocketLibre();
//			pthread_mutex_unlock(&poolConexiones.poolSemaphore);
//
//			ret = pedirSector(primerSector + i, pos); //LEER SECTOR CON PROTOCOLO NIPC
//			if(ret)return ret;
//			posiciones[i] = pos;
//			//sectores[i] = primerSector + i;
//		}
//
//		for (i = 0; i < 4; i++) { //aca habria que hacer todos los send y dps todos los rcv y juntarlos,
//			//readSector(sector,primerSector + i);
//			ret = recibirSector(sector, posiciones[i], &numSec); //LEER SECTOR CON PROTOCOLO NIPC
//			pthread_mutex_unlock(&poolConexiones.conexiones[posiciones[i]].socketSemaphore);
//			if(ret)return ret;
//			//for(j = 0; j < 4; j++){
//			//if(sectores[j] == numSec) {
//			memcpy(bloque + (512 * i), sector, 512);
//			//}
//			//}
//		}
//
//		//printf("Unlock Semaforo %p \n",&poolConexiones.conexiones[pos].socketSemaphore);
//
//
//		//
//	}

	free(sector);
	return 0;
}

int calcularCantBloques(int cantSectores){
	int cantBloques=0;
	cantBloques = ceil( (cantSectores+0.0)/4); //no pude usar el define
	return cantBloques;
}

int persistirFat(void) {

	int ret=0, cantClusters=0, i=0;
	unsigned int entrada = 0;
	uint32_t tamDeCluster = bs->bytesXsector * bs->sectorXcluster;
	char *dataCluster = malloc(tamDeCluster);
	uint32_t temp=0;
	char *bytesDelCluster;
	cantClusters = bs->sectoresXfat / 8;
	uint32_t *cluster = NULL;

	bzero(dataCluster,4096);

	//for (cluster = 0; cluster < cantClusters; cluster++) { //escribir de a clusters hasta encontrar una entrada libre, en tal caso no tiene sentido seguir persistiendola

	while(collection_list_size(clustersFatActualizados)){
		cluster = collection_list_remove(clustersFatActualizados, 0);
		entrada = ((*cluster) * tamDeCluster) / sizeof(uint32_t);

		for(i = 0; i <= 4088; i = i + 4) {

			temp = fat_table[entrada];
			bytesDelCluster = (char *) &temp;
			memcpy(dataCluster + i, bytesDelCluster, 4);
			entrada++;


		}

		ret = escribirClusterDeReservedOFatRegion(4 + (*cluster) ,dataCluster);//ubicacion de la fat, empieza en cluster 4
		if(ret)return ret;
		bzero(dataCluster,4096);
		free(cluster);
	}

	//free(bytesDelCluster);
	free(dataCluster);

	return 0;

}

int escribirClusterDeReservedOFatRegion(int numCluster, char *dataCluster){
	int primerSector=0, primerBloque = 0, cantBloques = 0, ret = 0, i = 0;
	char* bloque = NULL;

	primerSector = numCluster * bs->sectorXcluster;
	primerBloque = bloqueDeUnSector(primerSector);

	cantBloques = calcularCantBloques(bs->sectorXcluster);

	bloque = malloc(bs->bytesXsector * 4);
	for (i=0;i<cantBloques;i++){
		memcpy(bloque, dataCluster+(i * bs->bytesXsector * 4), bs->bytesXsector * 4);
		ret = escribirBloque(bloque,primerBloque+i);
		if(ret)return ret;
	}

	free(bloque);

	return 0;

}


int buscarSocketLibre(void) {

	int socketUsado;
	int i;

	for(i = 0; i < poolConexiones.cantidadConexiones; i++) {
		//printf("Try Semaforo %p\n",&poolConexiones.conexiones[i].socketSemaphore);
		socketUsado = pthread_mutex_trylock(&poolConexiones.conexiones[i].socketSemaphore);

		if(!socketUsado) {
//			printf("mandando por socket posicion: %d\n", i);
			return i;

		}

		if(socketUsado && i == poolConexiones.cantidadConexiones-1) {
			return -1;
//			i = -1;

		}

	}

}
