#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include "cache.h"
#include "Comun/manejoArchivo.h"
#include "funciones.h"
#include "utils.h"
#include "librerias/config_loader.h"

int tamanoCache;

int inicializarCacheArchivos(void) {

	char *bufConfig;

	bufConfig = config_loader_open("pfs.config");

	tamanoCache = pow(2, config_loader_getInt(bufConfig, "TamanoDeLaCache"));
	//exponente = 12, 13, 14 o 15
	//tamanoCache = 4096, 8192, 16384 o 32768

	if(tamanoCache != 1) {

		cacheArchivos = collection_list_create();

	}

	free(bufConfig);
	return 0;

}

int leerCacheArchivos(char *nombre, uint32_t numeroDeCluster, char *data) {

	int i;
	int bloquesPorCache;
	int clusterEncontrado = 0;
	entradaCacheArchivos *entradaArchivo = NULL; // = malloc(sizeof(entradaCacheArchivos));
	entradaBloqueArchivos *entradaBloque = NULL; //malloc(sizeof(entradaBloqueArchivos));

	if(tamanoCache != 1) {

		bloquesPorCache = tamanoCache / 4096;

		for(i = 0; i < cacheArchivos->elements_count; i++) {

			entradaArchivo = collection_list_get(cacheArchivos, i);

			if(strcmp(nombre, entradaArchivo->nombre) == 0) {

				i = cacheArchivos->elements_count;

			}

		}

		pthread_mutex_lock(&entradaArchivo->fileSemaphore);

		for(i = 0; i < entradaArchivo->listaBloques->elements_count; i++) {

			entradaBloque = collection_list_get(entradaArchivo->listaBloques, i);

			if(numeroDeCluster == entradaBloque->numeroDeCluster) {

				clusterEncontrado = 1;
				i = entradaArchivo->listaBloques->elements_count;

			}

		}

		if(clusterEncontrado) {

			entradaBloque->cantidadDeAccesos++;
			memcpy(data, entradaBloque->data, 4096);
			//data = entradaBloque->data; //memcpy?
			pthread_mutex_unlock(&entradaArchivo->fileSemaphore);
			return 0;

		}

		else {

			pedirCluster(numeroDeCluster, data);

			//data = "wololo";

			if(bloquesPorCache == entradaArchivo->listaBloques->elements_count) {

				reemplazarEntradaBloque(numeroDeCluster, data, entradaArchivo, LECTURACACHE);

			}

			else {

				agregarEntradaBloque(numeroDeCluster, data, entradaArchivo, LECTURACACHE);

			}

		}

		pthread_mutex_unlock(&entradaArchivo->fileSemaphore);

	}

	else {

		pedirCluster(numeroDeCluster, data);

		//data = "wololo";

	}

	return 0;

}

int reemplazarEntradaBloque(uint32_t numeroDeCluster, char *data, entradaCacheArchivos *entradaArchivo, int tipo) {

	int i;
	entradaBloqueArchivos *entradaBloque = NULL;//= malloc(sizeof(entradaBloqueArchivos));
	int menor;
	int posMenor = 0;

	entradaBloque = collection_list_get(entradaArchivo->listaBloques, 0);
	menor = entradaBloque->cantidadDeAccesos;

	for(i = 1; i < entradaArchivo->listaBloques->elements_count; i++) {

		entradaBloque = collection_list_get(entradaArchivo->listaBloques, i);

		if(menor > entradaBloque->cantidadDeAccesos) {

			menor = entradaBloque->cantidadDeAccesos;
			posMenor = i;

		}

	}

	entradaBloque = collection_list_get(entradaArchivo->listaBloques, posMenor);

	if(entradaBloque->tipo == ESCRITURACACHE) {

		escribirCluster(entradaBloque->numeroDeCluster, entradaBloque->data);
		printf("aca hay que escribir.. wololo");
	}

	entradaBloque->cantidadDeAccesos = 1;
	memcpy(entradaBloque->data, data, 4096); //entradaBloque->data = data;
	entradaBloque->numeroDeCluster = numeroDeCluster;
	entradaBloque->tipo = tipo;
	//collection_list_set(entradaArchivo->listaBloques, posMenor, entradaBloque, (void *)freeEntradaBloque);

	return 0;

}

int agregarEntradaBloque(uint32_t numeroDeCluster, char *data, entradaCacheArchivos *entradaArchivo, int tipo) {

	entradaBloqueArchivos *entradaBloque = malloc(sizeof(entradaBloqueArchivos));

	entradaBloque->data = malloc(4096); //malloc(strlen(data) + 1)

	entradaBloque->cantidadDeAccesos = 1;
	memcpy(entradaBloque->data, data, 4096);
	entradaBloque->numeroDeCluster = numeroDeCluster;
	entradaBloque->tipo = tipo;

	//free(data);
	collection_list_add(entradaArchivo->listaBloques, entradaBloque);

	return 0;

}

int abrirArchivo(char *nombre) {

	int i;
	int archivoEncontrado = 0;
	entradaCacheArchivos *entradaArchivo = NULL; //= malloc(sizeof(entradaCacheArchivos));
	entradaCacheArchivos *entradaArchivoNueva = malloc(sizeof(entradaCacheArchivos));

	if(tamanoCache != 1) {

		pthread_mutex_lock(&cacheSemaphore);

		for(i = 0; i < cacheArchivos->elements_count; i++) {

			entradaArchivo = collection_list_get(cacheArchivos, i);

			if(strcmp(nombre, entradaArchivo->nombre) == 0) {

				entradaArchivo->users++;

				//collection_list_set(cacheArchivos, i, entradaArchivo, (void *)freeEntradaArchivo);

				i = cacheArchivos->elements_count;
				archivoEncontrado = 1;

			}

		}

		if(!archivoEncontrado) {

			entradaArchivoNueva->nombre = malloc(strlen(nombre) + 1);

			entradaArchivoNueva->listaBloques = collection_list_create();
			memcpy(entradaArchivoNueva->nombre, nombre, strlen(nombre) + 1);
			entradaArchivoNueva->users = 1;
			pthread_mutex_init(&entradaArchivoNueva->fileSemaphore, NULL);

			collection_list_add(cacheArchivos, entradaArchivoNueva);

		}
		else{
			free(entradaArchivoNueva);
		}

		pthread_mutex_unlock(&cacheSemaphore);

	}
	else{
		free(entradaArchivoNueva);
	}

	return 0;

}

int escribirCacheArchivos(char *nombre, uint32_t numeroDeCluster, char *data) {

	int i;
	int bloquesPorCache;
	int clusterEncontrado = 0;
	entradaCacheArchivos *entradaArchivo = NULL;//malloc(sizeof(entradaCacheArchivos));
	entradaBloqueArchivos *entradaBloque = NULL; // malloc(sizeof(entradaBloqueArchivos));

	if(tamanoCache != 1) {

		bloquesPorCache = tamanoCache / 4096;

		for(i = 0; i < cacheArchivos->elements_count; i++) {

			entradaArchivo = collection_list_get(cacheArchivos, i);

			if(strcmp(nombre, entradaArchivo->nombre) == 0) {

				i = cacheArchivos->elements_count;

			}

		}

		pthread_mutex_lock(&entradaArchivo->fileSemaphore);

		for(i = 0; i < entradaArchivo->listaBloques->elements_count; i++) {

			entradaBloque = collection_list_get(entradaArchivo->listaBloques, i);

			if(numeroDeCluster == entradaBloque->numeroDeCluster) {

				clusterEncontrado = 1;
				i = entradaArchivo->listaBloques->elements_count;

			}

		}

		if(clusterEncontrado) {

			entradaBloque->cantidadDeAccesos++;
			entradaBloque->tipo = ESCRITURACACHE;
			memcpy(entradaBloque->data, data, 4096); //entradaBloque->data = data;
			//collection_list_set(entradaArchivo->listaBloques, i, entradaBloque, (void *)freeEntradaBloque);

		}

		else {

			if(bloquesPorCache == entradaArchivo->listaBloques->elements_count) {

				reemplazarEntradaBloque(numeroDeCluster, data, entradaArchivo, ESCRITURACACHE);

			}

			else {

				agregarEntradaBloque(numeroDeCluster, data, entradaArchivo, ESCRITURACACHE);

			}

		}

		pthread_mutex_unlock(&entradaArchivo->fileSemaphore);

	}

	else {


		escribirCluster(numeroDeCluster, data);
		//printf("aca hay que escribir.. wololo");

	}

	return 0;

}

int configSignal(void) {

	struct sigaction new_action, old_action;
	pid_t pid;

	pid = getpid();

	printf("%d\r\n", pid);

	sleep(5);

    new_action.sa_handler = cacheDump;
	new_action.sa_flags = 0;
	sigemptyset(&new_action.sa_mask);

    sigaction(SIGUSR1, NULL, &old_action);

    if (old_action.sa_handler != SIG_IGN) {

    	sigaction(SIGUSR1, &new_action, NULL);

    }

    return 0;

}

void cacheDump(int signum) {

	int fd;
	ssize_t bytesEscritos;
	int i;
	int j;
	char *buffer = malloc(4096);
	int bloquesPorCache;
	char *bloquesPorCacheString = malloc(1);
	char *n = malloc(1);
	char *timeStamp = malloc(20);
	entradaCacheArchivos *entradaArchivo = NULL;
	entradaBloqueArchivos *entradaBloque = NULL;

	printf("llego al proceso tamanoCache: %d\r\n", tamanoCache);

	if(tamanoCache != 1) {

		bloquesPorCache = tamanoCache / 4096;
		itoa(bloquesPorCache, bloquesPorCacheString, 10);

		fd = open("cache_dump.txt", O_CREAT | O_RDWR | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO);

		if(fd == -1) {

			perror("error al abrir cache dump");
			return;

		}

		printf("abrio/creo archivo\r\n");

		pthread_mutex_lock(&cacheSemaphore);

		for(i = 0; i < cacheArchivos->elements_count; i++) {

			entradaArchivo = collection_list_get(cacheArchivos, i);

			bytesEscritos = write(fd, "-------------------------------------------------------------------------------\n\r", 80);
			fechaActual(timeStamp);
			write(fd, timeStamp, strlen(timeStamp));
			bytesEscritos = write(fd, "\n\r", 2);
			bytesEscritos = write(fd, "Nombre de Archivo: ", 19);
			bytesEscritos = write(fd, entradaArchivo->nombre, strlen(entradaArchivo->nombre));
			bytesEscritos = write(fd, "\n\r", 2);
			bytesEscritos = write(fd, "Tamaño de Bloque de Cache: 4096", 32);
			bytesEscritos = write(fd, "\n\r", 2);
			bytesEscritos = write(fd, "Bloques por Cache: ", 19);
			bytesEscritos = write(fd, bloquesPorCacheString, strlen(bloquesPorCacheString));
			bytesEscritos = write(fd, "\n\r", 2);
			bytesEscritos = write(fd, "\n\r", 2);

			printf("escribiendo por archivo");

			for(j = 0; j < entradaArchivo->listaBloques->elements_count; j++) {

				bytesEscritos = write(fd, "Contenido de Bloque de Cache ", 29);
				itoa(j + 1, n, 10);
				bytesEscritos = write(fd, n, 1);
				bytesEscritos = write(fd, ":", 1);
				bytesEscritos = write(fd, "\n\r", 2);
				entradaBloque = collection_list_get(entradaArchivo->listaBloques, j);
				memset(buffer, ' ', 4096);
				memcpy(buffer, entradaBloque->data, 4096); //strlen(entradaBloque->data) ??
				bytesEscritos = write(fd, buffer, 4096);
				bytesEscritos = write(fd, "\n\r", 2);

				printf("escribiendo por bloque\r\n");

			}

		}

		pthread_mutex_unlock(&cacheSemaphore);

		close(fd);
	}

	free(buffer);
	free(n);
	free(bloquesPorCacheString);
	free(timeStamp);

	return;

}

void freeEntradaBloque(entradaBloqueArchivos *entradaBloque) {

	free(entradaBloque->data);

	free(entradaBloque);

}

void freeEntradaArchivo(entradaCacheArchivos *entradaArchivo) {

	while(entradaArchivo->listaBloques->elements_count) {

		collection_list_removeAndDestroy(entradaArchivo->listaBloques, 0, (void *)freeEntradaBloque);

	}

	free(entradaArchivo->nombre);
	pthread_mutex_destroy(&entradaArchivo->fileSemaphore);

}

int liberarArchivo(char *nombre) {

	int i;
	entradaCacheArchivos *entradaArchivo = NULL;// = malloc(sizeof(entradaCacheArchivos));

	if(tamanoCache != 1) {

		pthread_mutex_lock(&cacheSemaphore);

		for(i = 0; i < cacheArchivos->elements_count; i++) {

			entradaArchivo = collection_list_get(cacheArchivos, i);

			if(strcmp(nombre, entradaArchivo->nombre) == 0) {

				entradaArchivo->users--;

				if(entradaArchivo->users == 0) {

					flushear(entradaArchivo);

					collection_list_removeAndDestroy(cacheArchivos, i, (void *)freeEntradaArchivo);

				}

				i = cacheArchivos->elements_count;

			}

		}

		pthread_mutex_unlock(&cacheSemaphore);

	}

	return 0;

}

int flushear(entradaCacheArchivos *entradaArchivo) {

	int i;
	entradaBloqueArchivos *entradaBloque = malloc(sizeof(entradaBloqueArchivos));

	for(i = 0; i < entradaArchivo->listaBloques->elements_count; i++) {

		entradaBloque = collection_list_get(entradaArchivo->listaBloques, i);

		if(entradaBloque->tipo == ESCRITURACACHE) {

			escribirCluster(entradaBloque->numeroDeCluster, entradaBloque->data);

		}

	}

	return 0;

}

int flushearArchivo(char *nombre) {

	int i;
	entradaCacheArchivos *entradaArchivo = NULL;// = malloc(sizeof(entradaCacheArchivos));

	if(tamanoCache != 1) {

		pthread_mutex_lock(&cacheSemaphore);

		for(i = 0; i < cacheArchivos->elements_count; i++) {

			entradaArchivo = collection_list_get(cacheArchivos, i);

			if(strcmp(nombre, entradaArchivo->nombre) == 0) {

				flushear(entradaArchivo);
				i = cacheArchivos->elements_count;

			}

		}

		pthread_mutex_unlock(&cacheSemaphore);

	}

	return 0;

}

int fechaActual(char *timeStamp) {

	time_t *tiempo = malloc(sizeof(time_t));
	struct tm *estructuraTiempo = malloc(sizeof(struct tm));

	time(tiempo);

	estructuraTiempo = localtime(tiempo);

	sprintf(timeStamp, "%d.%02d.%02d %02d:%02d:%02d", estructuraTiempo->tm_year + 1900, estructuraTiempo->tm_mon + 1, estructuraTiempo->tm_mday, estructuraTiempo->tm_hour, estructuraTiempo->tm_min, estructuraTiempo->tm_sec);


	//free(estructuraTiempo);
	free(tiempo);

	return 0;

}
