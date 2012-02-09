#include <stdio.h>
#include <string.h>
#include "consola.h"
#include "funciones.h"


void *consolaThread(void){
	char comando[200]={0};
	char *encontroEspacio = NULL;
	char *path = malloc(200), *funcion = malloc(10);
	int tamanoFat = 0,ret=0, hayEspacio=0;
	uint32_t cantClusters = bs->sectoresXfat * bs->bytesXsector / sizeof(uint32_t);// 131072
	t_list* listaClusters = collection_list_create();
	uint32_t posBytesEntrada = 0, clusterInfo = 0, clusterEntrada = 0, *unCluster=NULL, i=0;
	uint32_t cantClusOcupados = 0;

	while(1){
		//hayEspacio = 0;
		printf("Pedime lo que quieras\n");
		fgets(comando, 200, stdin);
		sscanf(comando, "%s %s", funcion, path);
		//encontroEspacio=strchr(comando,' ');
//		if (encontroEspacio != NULL){
//			hayEspacio = 1;
//			path = encontroEspacio +1;
//			encontroEspacio = '\0';
//		}

		if (strcmp(funcion, "fsinfo") && strcmp(funcion, "finfo")){
			printf("%s no es un comando válido\n", funcion);
			//flush(stdin) creo q era
		}
		else{
			if (strcmp(funcion, "fsinfo") == 0) {
				tamanoFat = bs->sectoresXfat * bs->bytesXsector / 1024;
				cantClusOcupados = cantClusters - cantClusLibres;

				printf("Clusters ocupados: %u\nClusters libres: %u\ntamaño de un sector:%dB\ntamaño de un cluster:%dB\ntamaño de la FAT:%dKB\n",cantClusOcupados,cantClusLibres,bs->bytesXsector,bs->sectorXcluster * bs->bytesXsector, tamanoFat);
			}

			if (strcmp(funcion, "finfo") == 0) {
				if (path == NULL){
					printf("Por favor ingrese un path\n");
				}
				else{
					if(strcmp(path,"/")==0)printf("La ruta especificada no corresponde a un archivo\n");
					else{
						ret = recorrerEntradasPath(path, &clusterInfo,&clusterEntrada, &posBytesEntrada);
						if(ret)printf("La ruta especificada no existe\n");
						else{
							ret = esUnArchivo(clusterInfo,posBytesEntrada);
							if (ret)printf("La ruta especificada no corresponde a un archivo\n");
							else{
								ret = listaClustersEncadenados(clusterInfo,listaClusters, 0);
								i=0;
								while(collection_list_size(listaClusters) && i<20){
									unCluster = collection_list_remove(listaClusters,0);
									printf("%u\n",*unCluster);
									i++;
								}
							}
						}
					}

				}

			}
		}
		bzero(comando,200);
	}
	return;
}
