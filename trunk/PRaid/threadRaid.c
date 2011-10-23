#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "Comun/librerias/collections/list.h"
#include "Comun/ProtocoloDeMensajes.h"
#include "Comun/NIPC.h"
#include "threadRaid.h"
#include "funcionesRaid.h"


extern t_list *dc;
extern pthread_mutex_t sem;
extern uint32_t pista;
extern uint32_t sector;


void *ppdThread(char *id){

	lDiscos *disco, *discoAux;
	lPeticiones  *ped = NULL;
	NIPC_Header *head;
	pedidoStr pedRespuesta;
	pedidoStr pedEnvio;
	time_t t;
	struct tm* now;
	uint32_t holaSoyVariableLoca = 0;
	int sockPpd;
	char *msg, *bufEnvio;
	int pos, val, ret = 0, retSnd;

	//Desencolo pedido
	pthread_mutex_lock(&sem);
	disco = BuscarDiscoPorId(id);
	//disco->threadId = (pthread_t)getpid();
	sockPpd = disco->descriptor;
	ped = pedidoSiguiente(disco->p);
	pthread_mutex_unlock(&sem);

	while (1){

		if (ped!=NULL) {
			//enviar pedido PPD

			msg =(char*) malloc(sizeof(pedidoStr));
			bzero(&pedEnvio, sizeof(pedidoStr));
			switch (ped->tipoPeticion){

				case (uint8_t)LECTURA:
				pedEnvio.sector = ped->sector;
				memcpy(pedEnvio.data, ped->data, 512);
				memcpy(msg, &pedEnvio, sizeof(pedidoStr));
				bufEnvio = serializarNipc(LECTURA, sizeof(pedidoStr), msg);
				retSnd = send(sockPpd, bufEnvio, sizeof(NIPC_Header) + sizeof(pedidoStr), MSG_NOSIGNAL);
				if (retSnd == EPIPE){
					caidaDeDisco(id, ped);
					pthread_exit(NULL);
				}
				free(bufEnvio);
				break;

				case (uint8_t)ESCRITURA:
				pedEnvio.sector = ped->sector;
				memcpy(pedEnvio.data, ped->data, 512);
				memcpy(msg, &pedEnvio, sizeof(pedidoStr));
				bufEnvio = serializarNipc(ESCRITURA, sizeof(pedidoStr), msg);
				retSnd = send(sockPpd, bufEnvio, sizeof(NIPC_Header) + sizeof(pedidoStr), MSG_NOSIGNAL);
				if (retSnd == EPIPE){
					caidaDeDisco(id, ped);
					pthread_exit(NULL);
				}

				free(bufEnvio);
				break;

				case (uint8_t) SINCRO:
				pedEnvio.sector = ped->sector;
				memcpy(pedEnvio.data, ped->data, 512);
				memcpy(msg, &pedEnvio, sizeof(pedidoStr));
				bufEnvio = serializarNipc(SINCRO, sizeof(pedidoStr), msg);
				retSnd = send(sockPpd, bufEnvio, sizeof(NIPC_Header) + sizeof(pedidoStr), MSG_NOSIGNAL);
				if (retSnd == EPIPE){
					caidaDeDisco(id, ped);
					pthread_exit(NULL);
				}

				free(bufEnvio);
				break;



			case (uint8_t)SINCRO_ESCRIBIR:

				pedEnvio.sector = ped->sector;
				memcpy(pedEnvio.data, ped->data, 512);
				memcpy(msg, &pedEnvio, sizeof(pedidoStr));
				bufEnvio = serializarNipc(SINCRO_ESCRIBIR, sizeof(pedidoStr), msg);
				retSnd = send(sockPpd, bufEnvio, sizeof(NIPC_Header) + sizeof(pedidoStr), MSG_NOSIGNAL);
				if (retSnd == EPIPE){
					caidaDeDisco(id, ped);
					pthread_exit(NULL);
				}

				free(bufEnvio);
				break;

			default:
				printf("Cayo en default NO SE MANDO MENSAJE CORRECTO\n");
				break;
			}
			free(msg);

			//RECIBO RESPUESTA PPD
			head = (NIPC_Header *)malloc(sizeof(NIPC_Header));
			ret = recv(sockPpd, head, sizeof(NIPC_Header), MSG_WAITALL);

			if (ret>0) {

				switch (head->type) {
				case (uint8_t) LECTURA: //me llega lectura con data sector

				//pedRespuesta = (pedidoStr *)malloc(sizeof(pedidoStr));
				recv(sockPpd, &pedRespuesta, sizeof(pedidoStr), MSG_WAITALL);
				msg = (char *)malloc(sizeof(pedidoStr));
				memcpy(msg, &pedRespuesta, sizeof(pedidoStr));
				bufEnvio = serializarNipc(LECTURA, sizeof(pedidoStr), msg);
				retSnd = send(ped->descriptor, bufEnvio, sizeof(NIPC_Header) + sizeof(pedidoStr), MSG_NOSIGNAL);
				if (retSnd == EPIPE){
					caidaDeDisco(id, ped);
					pthread_exit(NULL);
				}

				printf("Completo Lectura en disco: %s sector: %d ,Lectura Ok\n", disco->id, ped->sector);
				//free(pedRespuesta);
				free(bufEnvio);
				free(msg);
				break;

				case (uint8_t) ESCRITURA: //llega escritura ok
				if (head->payloadlength == 0){
					holaSoyVariableLoca++;
					if (holaSoyVariableLoca == 30000){
						printf("Completo Escritura en disco: %s sector: %d ,Escritura Ok\n", disco->id, ped->sector);
					}
				}
				else{
					// ???
				}
				break;
				case (uint8_t)SINCRO_ESCRIBIR:
				if (head->payloadlength == 0){
					holaSoyVariableLoca++;
					if (holaSoyVariableLoca == 30000){//imprime cada 30000 sectores escritos
						printf("Completo Escritura en disco: %s sector: %d ,Escritura Ok\n", disco->id, ped->sector);
						holaSoyVariableLoca = 0;
					}
					disco->cantPetSincro++;
					if (disco->cantPetSincro == (pista * sector)){
						t = time(NULL);
						now = localtime(&t);

						printf("[%d:%d:%d]Se termino Sincronizacion de Disco: '%s'\n", now->tm_hour, now->tm_min, now->tm_sec, id);

						bufEnvio = serializarNipc(FINSINCRO, 0, NULL);
						retSnd = send(sockPpd, bufEnvio, sizeof(NIPC_Header), MSG_NOSIGNAL);
						if (retSnd == EPIPE){
							caidaDeDisco(id, ped);
							pthread_exit(NULL);
						}

						pthread_mutex_lock(&sem);
						disco->estado = HABILITADO;
						pos = posDisco(id);
						//collection_list_set(dc, pos, disco, free);
						pthread_mutex_unlock(&sem);
						free(bufEnvio);
					}
				}
				break;
				case (uint8_t) SINCRO:
				//pedRespuesta = (pedidoStr *)malloc(sizeof(pedidoStr));
				recv(sockPpd, &pedRespuesta, sizeof(pedidoStr), MSG_WAITALL);
				pthread_mutex_lock(&sem);
				discoAux = buscarDiscoPorDesc(ped->descriptor);
				if (discoAux != NULL) {
					Agregar(discoAux->p, ped->descriptor, pedRespuesta.sector, pedRespuesta.data, sizeof(pedidoStr), SINCRO_ESCRIBIR);
				}
				else{
					//Cayo disco a sincronizar, borrar pedidos de ese socket?
				}
				pthread_mutex_unlock(&sem);
				//free(pedRespuesta);
				break;

				default:
					printf("Si cae aca tamo al horno error en el NIPC_header \n");
					break;
				}

			}
			else{
				if (ret == 0) {
				//Desconexion
					val = caidaDeDisco(id, ped);
					if (val == -1){
						printf("-------------------------------\n");
						printf("el Raid Debe Cerrarse.\n");
						printf("-------------------------------\n");
					}
					//Terminar Thread
					pthread_exit(NULL);
				}
				if (ret < 0) {
				//Error
				}
			}
		//libero pedido y head
		free(head);
		free(ped);
		}

		pthread_mutex_lock(&sem);
		ped = pedidoSiguiente(disco->p);
		pthread_mutex_unlock(&sem);

	}
	pthread_exit(NULL);;

}



int asignarThreadPpd(char *id){
	int rc;
	pthread_t thread;
	char *disk = malloc(strlen(id) + 1);
	strcpy(disk , id);

	rc = pthread_create(&thread, NULL,(void *) &ppdThread, disk);
	if (rc){
		printf("ERROR; return code from pthread_create() is %d\n", rc);
		return EXIT_FAILURE;
	}
	printf("Se conecto un nuevo disco: %s \n", disk);
	return EXIT_SUCCESS;
}


int raidProcessInit(){


	dc = collection_list_create();
	pthread_mutex_init(&sem, NULL);

	return EXIT_SUCCESS;
}






