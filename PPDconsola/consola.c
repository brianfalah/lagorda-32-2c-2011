/*
 * consola.c
 *
 *  Created on: 09/11/2011
 *      Author: todos
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>
#include "Comunes/sockets.h"
#include "Comunes/manejoArchivo.h"
#include "Comunes/ProtocoloDeMensajes.h"
#include "Comunes/librerias/collections/list.h"
#include "consola.h"
#include "funcionesConsola.h"


uint32_t cantPistas;
uint32_t sectsxPista;
double tpoEntrePistas;
double tpoEntreSectores;

int main(int argc, char *argv[]){

	int sockDisco, i=0, j=0, args=0;
	int nbytes;
	int offset;
	char *msg, *buf ,*posActual;
	char *bufConfig, *pathDisco;
	int msglength, ret;
	char param[30];
	char comando [20];
	char dir[100];
	NIPC_Header *head;
	int32_t array[5];
	respuestaClean clean;
	t_list *listaSectsAtravesados = collection_list_create();
	respuestaTrace respTrace;
	double tpoTotal=0;
	uint32_t pista=0, sector=0;
	PisSec *pistaSector;

	printf("Consola Iniciada\n");
	printf("%s   %s\n", argv[0], argv[1]);
	bufConfig = config_loader_open(argv[1]);
	pathDisco = config_loader_getString(bufConfig, "ArchUnix");
	sleep(1);
	if (conectarSocketUnix(&sockDisco, pathDisco) == 1){ /*	CAMBIAR SE RECIBE POR CONFIGURACION*/
		printf("NO SE PUDO CONECTAR\n");
		return EXIT_FAILURE;
	}
	printf("OK\n");

	handshakePpd(sockDisco);
	head = (NIPC_Header *) malloc(sizeof(NIPC_Header));
	printf("Ingrese un comando.(Help - 'help', para ayuda)\n");
	fgets(dir,200,stdin);

	while (strcmp(dir,"exit") != 0){

		ret = sscanf(dir, "%s %s" , comando, param);

		if (strcmp(comando, "help") == 0 ){
			printf("- cambio (Cambia algoritmo actual)\r\n- clean parametros:2 (Setea los sectores desde el primer parametro al segundo en cero)\r\n- info	(Informa pista y sector Actual)\n- trace parametros: (maximo 5, informa planificacion de pedidos en el PPD)\r\n- starv \n");
		}
		if (strcmp ("info", comando) == 0){
			msg = serializarNipc(INFO, 0, NULL);
			msglength = sizeof( NIPC_Header);
			send(sockDisco, msg, msglength, 0);
			free(msg);

			nbytes = recv(sockDisco, head, sizeof(NIPC_Header), MSG_WAITALL);
			if (nbytes > 0){
				posActual= (char*)malloc( head -> payloadlength );
				nbytes = recv( sockDisco, posActual, head->payloadlength, MSG_WAITALL);

				if (nbytes>0){
					printf("La posicíon actual es %s \n", posActual);
					free(posActual);
				}
				else printf("Error en recv...\n");
			}

		}
		if (strcmp ("starv", comando) == 0){
			msg = serializarNipc(STARV, 0, NULL);
			msglength = sizeof(NIPC_Header);
			send(sockDisco, msg, msglength, 0);
			free(msg);
			nbytes = recv(sockDisco, head, sizeof(NIPC_Header), MSG_WAITALL);
			if (nbytes > 0){
				posActual= (char*)malloc( head -> payloadlength );
				nbytes = recv( sockDisco, posActual, head->payloadlength, MSG_WAITALL);

				if (nbytes>0){
					printf("%s \n", posActual);
					free(posActual);
				}
				else printf("Error en recv...\n");
			}else printf("Error en recv...\n");
		}


		if (strcmp ("clean", comando) == 0){

			clean.sectorInicial = atoi(strtok(param, ","));
			clean.sectorFinal = atoi(strtok(NULL," "));

			buf = malloc(sizeof(respuestaClean));
			memcpy(buf, &clean, sizeof(respuestaClean));
			msg = serializarNipc(CLEAN, sizeof(respuestaClean),buf);
			send(sockDisco, msg, sizeof(respuestaClean)+sizeof(NIPC_Header), MSG_NOSIGNAL);

			printf("Cantidad de sectores a escribir: %d\n", (clean.sectorFinal - clean.sectorInicial) + 1);
			for(i = 1; i <= ((clean.sectorFinal - clean.sectorInicial)+1); i++){
				nbytes = recv(sockDisco, head, sizeof(NIPC_Header), MSG_WAITALL);
				if (head->type == ESCRITURA){
					printf("Se escribieron %d sector/es...\n", i);
				}

			}

			printf("Sectores Seteados a 0\n");
			free(msg);
			free(buf);
		}

		if (strcmp ("trace", comando) == 0){
			inicializaArray(&array);
			i = 0;
			args = 0;
			if (param == NULL){
				printf("Introduzca una lista de sectores correcta.\n");
			}

			array[i] = atoi(strtok(param,","));
			while (i<5){
				i++;
				buf = malloc(sizeof(uint32_t));
				bzero(buf, sizeof(uint32_t));
				buf = strtok(NULL, ",");
				if (buf != NULL){
					array[i] = atoi(buf);
					args++;
				}
			}

			buf = malloc(sizeof(uint32_t)*5);
			memcpy(buf, &array, sizeof(uint32_t)*5);
			msg = (char *)serializarNipc(TRACE, sizeof(uint32_t)*5, buf);
			msglength = (sizeof(NIPC_Header) + sizeof(uint32_t)*5);
			send(sockDisco, msg, msglength, 0);
			free(msg);
			free(buf);


			for (j=0; j <= args; j++){
				nbytes = recv(sockDisco, head, sizeof(NIPC_Header),0);
				if (nbytes > 0){
					posActual= (char*)malloc( head -> payloadlength );
					nbytes = recv( sockDisco, &respTrace, head->payloadlength, 0);
					free(posActual);
					if (nbytes>0){

						nbytes = obtenerDirFisicaSector(respTrace.sectLogActual , &pista , &sector);
						printf("Posición Actual: %u:%u\n",pista,sector);

						MoverASector(respTrace.sectLogSolic,listaSectsAtravesados,pista,sector,respTrace.sentido, &tpoTotal);
						nbytes = obtenerDirFisicaSector(respTrace.sectLogSolic , &pista , &sector);
						printf("Sector Solicitado: %u:%u\n",pista,sector);
						printf("Sectores Recorridos:");
						while(collection_list_size(listaSectsAtravesados)>0){
							pistaSector = collection_list_remove(listaSectsAtravesados,0);
							printf("%u:%u,	",pistaSector->pista, pistaSector->sector);
						}
						printf("\nTiempo Consumido: %f ms\n",tpoTotal);
						sector++;
						if (sector == sectsxPista){
							sector = 0;

						}
						printf("Próximo Sector: %u:%u\n",pista, sector);
						//sectActual = sectActual%sectsxPista;
					}
					else printf("Error en recv...\n");
				}
			}

		}

		if(strcmp ("cambio", comando) == 0){
			msg = serializarNipc(CAMBIO, 0, NULL);
			msglength = sizeof( NIPC_Header);
			send(sockDisco, msg, msglength, 0);
			free(msg);
		}
		printf("\n");
		printf("Ingrese un comando.(Help - 'help', para ayuda)\n");
		fgets(dir, 200, stdin);
	}

	recv(sockDisco,buf, 20, NULL);
	printf("%s\n",buf);
	return EXIT_SUCCESS;
}

int handshakePpd(int sockDisco){

	handshakeConsola *hdk;
	NIPC_Header *head;
	char *msg;

	msg = (char *)serializarNipc(HANDSHAKE, 0, NULL);
	send(sockDisco, msg, sizeof(NIPC_Header), 0);

	head = (NIPC_Header *)malloc(sizeof(NIPC_Header));
	recv(sockDisco, head, sizeof(NIPC_Header), 0);

	if (head->type == PAYOK){
		hdk = (handshakeConsola *)malloc(sizeof(handshakeConsola));
		recv(sockDisco, hdk, sizeof(handshakeConsola), 0);

		cantPistas = hdk->cantPistas;
		sectsxPista = hdk->sectsxPista;
		tpoEntrePistas = hdk->tpoEntrePistas;
		tpoEntreSectores = hdk->tpoEntreSectores;

		free(hdk);

		printf("\n");printf("\n");printf("\n");printf("\n");printf("\n");printf("\n");printf("\n");printf("\n");
		printf("-----------------------------------------------\n");
		printf("Handshake realizado, Consola conectada!!!\n");
		printf("-----------------------------------------------\n");
		return 0;
	}else{
		printf("Handshake no puedo realizarse\n");
		return 1;
	}
	return 1;
}

