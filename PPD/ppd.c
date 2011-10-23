
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>
#include "Comunes/sockets.h"
#include "funcionesDisco.h"
#include "Comunes/ProtocoloDeMensajes.h"
#include "Comunes/librerias/config_loader.h"
#include "Comunes/librerias/collections/list.h"
#include "Comunes/NIPC.h"
#include "Comunes/librerias/log.h"
#include "threadPPD.h"
#include "listaPPD.h"
#include "mapeoArchivos.h"
#include "ppd.h"

/*lPeticiones *lped = NULL;*/ /*Defino variable global para los pedidos, de esta manera como es global puede ser usada tanto
						   *por el productor como el consumidor.*/
t_list *lped;
t_list *lped2;
pthread_mutex_t sem;
uint32_t cantPedidosEscritura = 0;

char *sectorString;
uint32_t pistaActual = 0, sectActual = 0;
int algoritmo, logActivado;
int sentido = ASCENDENTE, starv = 0;
t_log *archLog;

chs cilCabSec;



int main(){

	int  listenSocket, acceptSocket, puerto, ret=0, n=0;
	NIPC_Header *head;
	NIPC_PKG *pkg;


	int sockEscuchaConsola, sockConsola;
	void *lala;
	char *bufConfig = config_loader_open("ppd.config");
	char *pathConsola = config_loader_getString(bufConfig, "PathConsola");
	char *pathConfig = config_loader_getString(bufConfig, "PathConfig");
	char *msg;
	char *arg[] = {pathConsola, pathConfig, NULL};
	struct stat *sbuf;
	char* dataSector = malloc(512), *buf;

	t_list *lPasiva;
	uint32_t pista, sector, i, pos;
	pid_t childPid;
	lPeticiones *pedido =NULL;
	lPeticiones *ped;
	int32_t trace[5];
	respuestaClean clean;
	handshakeConsola *hdk;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Pruebas~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	lped = collection_list_create();
	lped2 = collection_list_create();
	lped->tipoLista = ACTIVA;
	lped2->tipoLista = PASIVA;

	pthread_mutex_init(&sem, NULL);

	if (operacionesDeInicio()){
		return 1;
	}

	inicializaSocketUnix(&sockEscuchaConsola, pathUnix);

	switch (fork()){

			case -1:
					/* Error*/
					break;
			case 0:
					/* Proceso Hijo*/
					if (execv(pathConsola, arg) < 0) {
							/* Error*/
							return EXIT_FAILURE;
					}
					break;
			default:


					/* Proceso Padre*/

//					sockConsola = aceptarUnix(sockEscuchaConsola);
//					printf("Consola Conectada!\n");
//					if (sockConsola < 0) {
//							return -1;
//					}
//
//					close(sockEscuchaConsola);
//					sockEscuchaConsola = -1;

					break;

	}
	sockConsola = aceptarUnix(sockEscuchaConsola);
	printf("Consola Conectada!\n");
	if (sockConsola < 0) {
			return -1;
	}
	close(sockEscuchaConsola);
	sockEscuchaConsola = -1;

//	sockConsola = aceptarUnix(sockEscuchaConsola);
//	printf("Consola Conectada!\n");

	//system("clear");
	printf("-------------------------------------\n");
	printf("Proceso Planificador de Disco\n");
	printf("-------------------------------------\n");
	printf("\n");
	printf("Consola Conectada!\n");
	if (threadProductorInit() == EXIT_FAILURE) {
			printf("Error threadProductorInit");
			return EXIT_FAILURE;
	}


	if (threadConsumidorInit() == EXIT_FAILURE) {
		printf("Error threadConsumidorInit");
		return EXIT_FAILURE;
	}
	if ( algoritmo == FSCAN ){
		if (lped->tipoLista == PASIVA){
			lPasiva = lped;
		}
		else lPasiva = lped2;

	}
	else lPasiva = lped;
	while (1){
		head = (NIPC_Header *)malloc(sizeof(NIPC_Header));
		ret = recv(sockConsola, head, sizeof(NIPC_Header), MSG_WAITALL);
		if (ret > 0){
			switch(head->type){

				case (uint8_t)HANDSHAKE:
				hdk = (handshakeConsola *)malloc(sizeof(handshakeConsola));
				hdk->cantPistas = cilCabSec.pistas;
				hdk->sectsxPista = cilCabSec.sectores;
				hdk->tpoEntrePistas = tpoEntrePistas;
				hdk->tpoEntreSectores = tpoEntreSectores;

				buf = (char *) malloc(sizeof(handshakeConsola));
				memcpy(buf, hdk, sizeof(handshakeConsola));
				msg = serializarNipc(PAYOK, sizeof(handshakeConsola), buf);
				send(sockConsola, msg, sizeof(NIPC_Header) + sizeof(handshakeConsola), 0);
				free(buf);
				free(msg);
				break;

				case (uint8_t)CAMBIO:
				pkg =(NIPC_PKG *) malloc(sizeof(NIPC_PKG));
				pkg->cabecera = head;
				pkg->payload = (char *)malloc(head->payloadlength);
				recv(sockConsola, pkg->payload, head->payloadlength, 0);

				if (algoritmo == SCAN){
					algoritmo = FSCAN;

				}else algoritmo = SCAN;

				break;

				case (uint8_t)CLEAN:

				recv(sockConsola, &clean, sizeof(respuestaClean), MSG_WAITALL);
				i = clean.sectorInicial;
				for(i; i<=clean.sectorFinal; i++){
					pedido = (lPeticiones *)malloc(sizeof(lPeticiones));
					obtenerDirFisicaSector(i, &pista, &sector);

					pedido->fd = sockConsola;
					pedido->pista = pista;
					pedido->sectorEnPista = sector;
					pedido->sectorLogico = i;
					pedido->tipo = (uint8_t)ESCRITURA;
					bzero(&pedido->data, 512);


					pthread_mutex_lock(&sem);
					pos = insertarSegunAlgoritmo(pista);
					if (pos != -1){
						collection_list_put(lPasiva, pos, pedido);
					}else collection_list_add(lPasiva, pedido);
					pthread_mutex_unlock(&sem);

				}
				break;

				case (uint8_t)INFO:

				buf = malloc(100);
				sprintf(buf, "Pista Actual:%u----SectorActual%u\n", pistaActual, sectActual);//MALLOC A BUF???
				msg = serializarNipc(INFO, strlen(buf) + 1, buf);
				send(sockConsola, msg, sizeof(NIPC_Header) + strlen(buf) + 1, 0);

				break;

				case (uint8_t)STARV:

				if (starv == 0){
					starv = 1;
					buf = malloc(strlen("Time Starvation Activado\n"));
					sprintf(buf, "Time Starvation Activado\n");
					//tpoLectura = 1 * 1000000;


				}else{
					starv = 0;
					buf = malloc(strlen("Time Starvation Desactivado\n"));
					sprintf(buf, "Time Starvation Desactivado\n");
					tpoLectura = config_loader_getDouble(bufConfig, "tpoLectura");
				}

				msg = serializarNipc(STARV, strlen(buf) + 1, buf);
				send(sockConsola, msg, sizeof(NIPC_Header) + strlen(buf) + 1, 0);

				break;
				case (uint8_t)TRACE:
				bzero(&trace, sizeof(uint32_t)*5);
				recv(sockConsola, &trace, head->payloadlength, 0);
				i = 0;
				while (trace[i] != -1 && i<5){
					pedido = (lPeticiones *)malloc(sizeof(lPeticiones));
					obtenerDirFisicaSector(trace[i], &pista, &sector);

					pedido->fd = sockConsola;
					pedido->pista = pista;
					pedido->sectorEnPista = sector;
					pedido->sectorLogico = trace[i];
					pedido->tipo = (uint8_t)TRACE;
					bzero(&pedido->data, 512);


					pthread_mutex_lock(&sem);
					pos = insertarSegunAlgoritmo(pista);
					if (pos != -1){
						collection_list_put(lPasiva, pos, pedido);
					}else collection_list_add(lPasiva, pedido);

					pthread_mutex_unlock(&sem);
					i++;

				}

				break;
				default:
					break;
			}
		}else{

			if (ret < 0) {
				perror("recv socket %d failed");

			}

			if (ret  == 0){
				printf("Cierre de conexion.\n ");
				close(sockConsola);
			}
		}
	}

	//while (1) sleep(1);

	return EXIT_SUCCESS;
}
