#ifndef CACHE_H_
#define CACHE_H_

#include <stdint.h>
#include <pthread.h>
#include "librerias/collections/list.h"
#include "librerias/collections/collections.h"

#define LECTURACACHE 0
#define ESCRITURACACHE 1

typedef struct {

	char *nombre;
	int users;
	pthread_mutex_t fileSemaphore;
	t_list *listaBloques;

} entradaCacheArchivos;

typedef struct {

	int cantidadDeAccesos;
	int tipo;
	uint32_t numeroDeCluster;
	char *data;

} entradaBloqueArchivos;

t_list *cacheArchivos;

pthread_mutex_t cacheSemaphore;

int inicializarCacheArchivos(void);
int leerCacheArchivos(char *nombre, uint32_t numeroDeCluster, char *data);
int reemplazarEntradaBloque(uint32_t numeroDeCluster, char *data, entradaCacheArchivos *entradaArchivo, int tipo);
int agregarEntradaBloque(uint32_t numeroDeCluster, char *data, entradaCacheArchivos *entradaArchivo, int tipo);
int abrirArchivo(char *nombre);
int escribirCacheArchivos(char *nombre, uint32_t numeroDeCluster, char *data);
int configSignal(void);
void cacheDump(int signum);
void freeEntradaBloque(entradaBloqueArchivos *entradaBloque);
void freeEntradaArchivo(entradaCacheArchivos *entradaArchivo);
int liberarArchivo(char *nombre);
int flushear(entradaCacheArchivos *entradaArchivo);
int flushearArchivo(char *nombre);
int fechaActual(char *timeStamp);

#endif /* CACHE_H_ */
