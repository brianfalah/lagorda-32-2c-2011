#ifndef FUNCIONES_H_
#define FUNCIONES_H_

#include <stdint.h>
#include "librerias/collections/collections.h"
#include "librerias/collections/list.h"
#include "Comun/ProtocoloDeMensajes.h"
#include "Comun/NIPC.h"
#include "librerias/log.h"

#define SECTSXBLOQUE 4;
#define SUBDIRECTORIO 0x10;
#define ARCHIVO 0x20;

int fd;

t_log *archLog;

typedef struct {

        pthread_mutex_t socketSemaphore;
        int socket;

} conexion;

typedef struct {

        pthread_mutex_t poolSemaphore;
        int cantidadConexiones;
        conexion *conexiones;

} pool;

typedef struct {
	char jumpInstruction[3];
	char OEM[8];
	int32_t bytesXsector;
	char sectorXcluster;
	//char cantReservedSector[2];
	int cantReservedSector;
	char noFat;
	char rootDirectoryEntries[2];
	char totalSectors[2];
	char mediaDescriptor;

	//char sectoresXpista[2];
	int  sectoresXpista;
	//char noCabezal[2];
	int noCabezas;
	int hiddenSectors;
	int totalSectors2; //este es el que sirve
	int sectoresXfat;
	int rootDirectoryStart; //numero de cluster donde empieza el root directory(deberia ser 2)
	int FSInfoSector; //numero de sector Fyle System Information Sector(deberia ser 1)
	int freeClusters;
}bootSector;

bootSector * bs;

typedef uint32_t* fat;
t_list *clustersFatActualizados;

fat fat_table;

typedef struct{
        int numero;

}tipoCluster;

uint32_t cantClusLibres;
pthread_mutex_t semFat;

//tipo entrada de archivo o directorio (32 bytes)
typedef struct {
        char nombreUTF8[8];
        char extensionUTF8[3];
        char atributos;
        char reservados; //no se usa
        char horaCreacion; //no se usa
        char horaCreacion2[2];//no se usan
        char fechaCreacion[2]; //no se usa
        char fechaUltAcceso[2]; //no se usa
        char bytesAltosPrimerClust[2]; //ocupa 2 bytes
        char horaUltModif[2]; //no se usa
        char fechaUltModif[2]; //no se usa
        char bytesBajosPrimerClust[2];
        uint32_t tamano;
} __attribute__((__packed__)) tipoDirectorio;

//tipo entrada LFN (32 bytes)
typedef struct {
        unsigned char sequenceNumber; //siempre en 0x40(en el otro endian) porque es la ultima y unica LFN
        uint16_t nombre1[5];
        unsigned char atributos; //siempre 0x0F para LFN
        unsigned char reservados; //no se usa, siempre 0x00
        unsigned char checkSum; // no se usa
        uint16_t nombre2[6];
        uint16_t firstCluster; //siempre 0x0000
        uint16_t nombre3[2];  //The remaining unused characters are filled with 0xFF 0xFF.
} __attribute__((__packed__)) tipoLFN;

//FUNCIONES
int readBootSector(void);
int pedirBootSector(char* bloque);
int levantarFAT(void);
int persistirFat(void);
//t_list *listaClustersLibres(int sectoresXFat,int cantReservedSectors,int *cantClustersLibres);
int recorrerEntradasPath(const char* path, uint32_t* numeroClusterInfo, uint32_t* numeroClusterEntrada, uint32_t *posBytesEntrada);
int buscarEntradaEnDir(uint32_t *numCluster, char * nombreEntrada, uint32_t *posBytesEntrada);
int listarEntradasDeCluster(uint32_t numCluster, t_list* listaEntradas);
int listaClustersEncadenados(uint32_t numCluster,t_list* listaClusters, int orden);
int renombrarArch(char *path,char *nuevoNombre, uint32_t clusterDirDestino);
int leerYCargar(uint32_t numCluster,char *buf,size_t size,off_t offset, char *path);
int escribirArchivo(uint32_t clusterInfo,char* buf, uint32_t size, uint32_t offset, char *path);
int truncarArchivo(uint32_t clusterEntrada,uint32_t posBytesEntrada,uint32_t clusterInfo,off_t nuevoTamano, char *path);

int primerSectorDeCluster(int numeroDeCluster);
int primerSectorDeBloque(int numBloque);
int bloqueDeUnSector(int numSector);
int posicionEnBloque(uint32_t bytes);
int pedirBloque(char * bloque, int numBloque);
int pedirBloques(char *bloques, int bloqueInicial, int cantBloques);
int pedirCluster(uint32_t numCluster, char *cluster);
int escribirCluster(uint32_t numCluster, char* dataCluster);
int calcularCantBloques(int cantSectores);
int escribirBloque(char *bloque,int numBloque);

uint32_t getClusterLibre(void);
uint32_t asignarClusterLibreA(uint32_t unCluster);

#endif /* FUNCIONES_H_ */
