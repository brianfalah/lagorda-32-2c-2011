/*
 * threadPPD.c
 *
 *  Created on: 11/11/2011
 *      Author: pablo
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <pthread.h>
#include "listaPPD.h"
#include "Comunes/sockets.h"
#include "Comunes/librerias/config_loader.h"
#include "Comunes/librerias/collections/list.h"
#include "Comunes/librerias/collections/collections.h"
#include "Comunes/librerias/log.h"
#include "mapeoArchivos.h"
#include "threadPPD.h"
#include "funcionesDisco.h"
#include "Comunes/NIPC.h"
#include "Comunes/ProtocoloDeMensajes.h"
#include "ppd.h"


extern t_list *lped;
extern t_list *lped2;
extern int algoritmo;
extern int sentido;
extern t_log *archLog;
extern uint32_t cantPedidosEscritura;
extern int logActivado;
extern pthread_mutex_t sem;
extern uint32_t pistaActual;
extern uint32_t sectActual;
extern int starv;

void *threadProductor() {

	struct epoll_event ev, events[MAX_EVENTS];
	struct sockaddr_in clienteAddr;
	int nfds, epollFd, listenSocket, acceptSocket, ret, n, sz, offset, socketRaid, i=0;
	char *buffer = malloc(sizeof(NIPC_Header));
	char *buf, *aux = NULL, *ip, *msg, *aux2;
	int addrLen = sizeof(clienteAddr);

	t_list *lPasiva;
	NIPC_Header *head;
	NIPC_PKG *pkg;
	int32_t numSector;
	int32_t dirLogica;
	uint32_t pista = 0, sector = 0;
	pedidoStr *p;

	lPeticiones *pedido;
	int pos=0;

	if(strcmp(modo, "LISTEN") == 0) {

		//buf = config_loader_open("ppd.config");
		/*Creo epoll*/
		epollFd = epoll_create(1000);
		if (epollFd == -1) {
			perror("epoll_create");
			exit(EXIT_FAILURE);
		}



		//lo defino global tambien --> puerto = config_loader_getInt("ppd.config","Puerto");
		if (inicializaSocketInet(&listenSocket, puerto) == 1){/*Inicializo listenSocket para escuchar conexiones*/
			perror("fallo inicializa socket");
			return EXIT_FAILURE;
		}


		ev.events = EPOLLIN;			/*Agrego el primer socket de Escucha(listenSocket) a la lista epoll*/
		ev.data.fd = listenSocket;
		if (epoll_ctl(epollFd, EPOLL_CTL_ADD, listenSocket, &ev) == -1) {
			perror("epoll_ctl: listenSocket");
			exit(EXIT_FAILURE);
		}



		printf("----------------------------------------------------------------------\n");
		printf("Modo Listen, esperando conexiones...\n");
		printf("----------------------------------------------------------------------\n");
		for (;;) {			/*Bucle Principal infinito, Esperando conexiones y peticiones de PFS en el wait*/
			nfds = epoll_wait(epollFd, events, MAX_EVENTS, -1);
			if (nfds == -1) {
				if (errno != EINTR){
					printf("error: %s , number :%d", strerror(errno), errno);
					exit(EXIT_FAILURE);
				}
			}

			for (n = 0; n < nfds; ++n) {

				if (events[n].data.fd == listenSocket) {			/*si es socketEscucha, agrego conexion*/
					acceptSocket = accept(listenSocket, (struct sockaddr *) &clienteAddr, &addrLen);
					if (acceptSocket == -1) {
						perror("accept");
						exit(EXIT_FAILURE);
					}

					ev.events = EPOLLIN | EPOLLET; 				/*agregamos evento-fd*/
					ev.data.fd = acceptSocket;
					if (epoll_ctl(epollFd, EPOLL_CTL_ADD, acceptSocket, &ev) == -1) {
						perror("epoll_ctl: acceptSocket");
						exit(EXIT_FAILURE);
					}

				} else {

					if ( algoritmo == FSCAN ){
						if (lped->tipoLista == PASIVA){
							lPasiva = lped;
						}
						else lPasiva = lped2;

					}
					else lPasiva = lped;

					head = (NIPC_Header*) malloc(sizeof(NIPC_Header));
					p =  (pedidoStr *)malloc(sizeof(pedidoStr));
					ret = recv(events[n].data.fd, head, sizeof(NIPC_Header), MSG_WAITALL);
					//printf("headerRet: %d \n", ret);

					if (ret > 0){

						switch (head->type) {
						case (uint8_t)HANDSHAKE:
						/* HANDSHAKE PFS*/
						aux = serializarNipc(PAYOK, 0, NULL);//que hago con los pfs conectados???
						send(events[n].data.fd, aux, sizeof(NIPC_Header), MSG_NOSIGNAL);
						printf("Handshake realizado con socket %d\n", events[n].data.fd);
						free(aux);

						break;

						case (uint8_t)(LECTURA):
						ret = recv(events[n].data.fd, p, sizeof(pedidoStr), MSG_WAITALL);
						//printf("payloadLength %d y ret %d \n", head->payloadlength, ret);
						/*aux =  malloc(512);
						readSector(aux, p->sector);
						buf =  malloc(sizeof(pedidoStr));
						memcpy(p->data, aux, 512);
						memcpy(buf, p, sizeof(pedidoStr));
						msg = serializarNipc(LECTURA, sizeof(pedidoStr), buf);
						send(events[n].data.fd, msg, sizeof(NIPC_Header) + sizeof(pedidoStr), 0);
						 */

						//convertir direccion a fisica, insertar segun algoritmo

						obtenerDirFisicaSector(p->sector, &pista, &sector);
						pedido = malloc(sizeof(lPeticiones));
						pedido->fd = events[n].data.fd;
						pedido->pista = pista;
						pedido->sectorEnPista = sector;
						pedido->sectorLogico = p->sector;
						pedido->tipo = (uint8_t)LECTURA;
						bzero(&pedido->data, 512);

						if (logActivado == 1){
							printf("Llego pedido Lectura sector:%d socket:%d  \n", p->sector, pedido->fd);
							log_info(archLog,"PPD","Llegada de pedido de lectura del sector %u.",pedido->sectorLogico);
						}


						pthread_mutex_lock(&sem);
						pos = insertarSegunAlgoritmo(pista);
						if (pos != -1){
							collection_list_put(lPasiva, pos, pedido);
						}else collection_list_add(lPasiva, pedido);
						pthread_mutex_unlock(&sem);

						break; 

						case (uint8_t)ESCRITURA:

						recv(events[n].data.fd, p, sizeof(pedidoStr), MSG_WAITALL);

						/*writeSector(p->data, p->sector);
						msg = serializarNipc(ESCRITURA, 0, NULL);
						send(events[n].data.fd, msg, sizeof(NIPC_Header), 0);
						 */


						obtenerDirFisicaSector(p->sector, &pista, &sector);
						pedido = malloc(sizeof(lPeticiones));
						pedido->fd = events[n].data.fd;
						pedido->pista = pista;
						pedido->sectorEnPista = sector;
						pedido->sectorLogico = p->sector;
						pedido->tipo = (uint8_t)ESCRITURA;
						memcpy(pedido->data, p->data, 512);


						if (logActivado == 1){
							printf("Llego pedido Escritura sector:%d socket:%d  \n", p->sector, pedido->fd);
							log_info(archLog,"PPD","Llegada de pedido de escritura del sector %u.\n",pedido->sectorLogico);
						}


						pthread_mutex_lock(&sem);
						pos = insertarSegunAlgoritmo(pista);
						if (pos != -1){
							collection_list_put(lPasiva, pos, pedido);
						}else collection_list_add(lPasiva, pedido);
						pthread_mutex_unlock(&sem);

						break;


						default:
							printf("nada por ahora...\n");
							break;

						}



					}
					if (ret < 0) {
						perror("recv socket %d failed");

					}
					if (ret  == 0){	/*cierre de conexion y remover de lista de eventos*/
						printf("close connection of %d\n", events[n].data.fd);
						if (epoll_ctl(epollFd, EPOLL_CTL_DEL, events[n].data.fd, &ev) == -1) {
							perror("epoll_ctl: acceptSocket");
							exit(EXIT_FAILURE);
						}
						printf("Cierre conexion socket:%d\n", events[n].data.fd);
						close(events[n].data.fd);
					}
					free(head);
					free(p);

				}
			}
		}

	}
	else{
		//Modo Connect

		printf("----------------------------------------------------------------------\n");
		printf("Modo Connect iniciado\n");
		printf("----------------------------------------------------------------------\n");
		cargarInfoInet(&ip, &puerto);
		conectarSocketInet(&socketRaid, ip, puerto);
		handshake(socketRaid);
		while (1){
			if ( algoritmo == FSCAN ){
				if (lped->tipoLista == PASIVA){
					lPasiva = lped;
				}
				else lPasiva = lped2;

			}
			else lPasiva = lped;

			p =(pedidoStr *)malloc(sizeof(pedidoStr));
			head = (NIPC_Header*)malloc(sizeof(NIPC_Header));
			ret = recv(socketRaid, head, sizeof(NIPC_Header), MSG_WAITALL);
			if (ret > 0){

				switch (head->type) {
				case (uint8_t)(LECTURA):

				recv(socketRaid, p, sizeof(pedidoStr), MSG_WAITALL);
				/*aux =  malloc(512);
					readSector(aux, p->sector);
					buf =  malloc(sizeof(pedidoStr));
					memcpy(p->data, aux, 512);
					memcpy(buf, p, sizeof(pedidoStr));
					msg = serializarNipc(LECTURA, sizeof(uint32_t) + 512, buf);
					send(socketRaid, msg, sizeof(NIPC_Header) + sizeof(pedidoStr), 0);
					free(p);*/

				//convertir direccion a fisica, insertar segun algoritmo
				obtenerDirFisicaSector(p->sector, &pista, &sector);
				pedido = malloc(sizeof(lPeticiones));
				pedido->fd = socketRaid;
				pedido->pista = pista;
				pedido->sectorEnPista = sector;
				pedido->sectorLogico = p->sector;
				pedido->tipo = (uint8_t)LECTURA;
				bzero(&pedido->data, 512);

				if (logActivado == 1){
					log_info(archLog,"PPD","Llegada de pedido de lectura del sector %u.\n",pedido->sectorLogico);
				}

				pthread_mutex_lock(&sem);
				pos = insertarSegunAlgoritmo(pista);
				if (pos != -1){
					collection_list_put(lPasiva, pos, pedido);
				}else collection_list_add(lPasiva, pedido);
				pthread_mutex_unlock(&sem);

				break;

				case (uint8_t) SINCRO:

				recv(socketRaid, p, sizeof(pedidoStr), MSG_WAITALL);

				obtenerDirFisicaSector(p->sector, &pista, &sector);
				pedido = malloc(sizeof(lPeticiones));
				pedido->fd = socketRaid;
				pedido->pista = pista;
				pedido->sectorEnPista = sector;
				pedido->sectorLogico = p->sector;
				pedido->tipo = (uint8_t)SINCRO;
				bzero(&pedido->data, 512);

				if (logActivado == 1){
					log_info(archLog,"PPD","Llegada de pedido de lectura del sector %u.\n",pedido->sectorLogico);
				}

				pthread_mutex_lock(&sem);
				pos = insertarSegunAlgoritmo(pista);
				if (pos != -1){
					collection_list_put(lPasiva, pos, pedido);
				}else collection_list_add(lPasiva, pedido);
				pthread_mutex_unlock(&sem);
				break;

				case (uint8_t)ESCRITURA:

				recv(socketRaid, p, head->payloadlength, MSG_WAITALL);

				/*writeSector(p->data, p->sector);
					//buf = cargarPedido(p.sector, NULL);
					msg = serializarNipc(ESCRITURA, 0, NULL);
					send(socketRaid, msg, sizeof(NIPC_Header), 0);
					free(p);
				 */

				obtenerDirFisicaSector(p->sector, &pista, &sector);
				pedido = malloc(sizeof(lPeticiones));
				pedido->fd = socketRaid;
				pedido->pista = pista;
				pedido->sectorEnPista = sector;
				pedido->sectorLogico = p->sector;
				pedido->tipo = (uint8_t)ESCRITURA;
				memcpy(pedido->data, p->data, 512);

				if (logActivado == 1){
					log_info(archLog,"PPD","Llegada de pedido de escritura del sector %u.\n",pedido->sectorLogico);
				}

				pthread_mutex_lock(&sem);
				pos = insertarSegunAlgoritmo(pista);
				if (pos != -1){
					collection_list_put(lPasiva, pos, pedido);
				}else collection_list_add(lPasiva, pedido);
				pthread_mutex_unlock(&sem);

				break;

				case (uint8_t)SINCRO_ESCRIBIR:

				recv(socketRaid, p, head->payloadlength, MSG_WAITALL);

				/*writeSector(p->data, p->sector);
										//buf = cargarPedido(p.sector, NULL);
										msg = serializarNipc(ESCRITURA, 0, NULL);
										send(socketRaid, msg, sizeof(NIPC_Header), 0);
										free(p);
				 */

				obtenerDirFisicaSector(p->sector, &pista, &sector);
				pedido = malloc(sizeof(lPeticiones));
				pedido->fd = socketRaid;
				pedido->pista = pista;
				pedido->sectorEnPista = sector;
				pedido->sectorLogico = p->sector;
				pedido->tipo = (uint8_t)SINCRO_ESCRIBIR;
				memcpy(pedido->data, p->data, 512);

				if (logActivado == 1){
					log_info(archLog,"PPD","Llegada de pedido de escritura del sector %u.\n",pedido->sectorLogico);
				}

				pthread_mutex_lock(&sem);
				pos = insertarSegunAlgoritmo(pista);
				if (pos != -1){
					collection_list_put(lPasiva, pos, pedido);
				}else collection_list_add(lPasiva, pedido);
				pthread_mutex_unlock(&sem);

				break;

				case(uint8_t)FINSINCRO:
				persistirDisco();
				break;

				default:
					//printf("nada por ahora...\n");
					break;

				}
			}
			free(p);
			free(head);
		}

	}

}


int threadProductorInit(){

	pthread_t thread;
	int rc;

	rc = pthread_create(&thread, NULL,(void *) &threadProductor, (NULL));
	if (rc){
		printf("ERROR; return code from pthread_create() is %d\n", rc);
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;
}

void *threadConsumidor(){

	int paylength, i=0;
	char *buf, *msg;
	char *dataSector;
	uint32_t direccionLogica = 0, pistaAnterior = 0, sectorAnterior = 0;
	double t = 0;

	t_list *sectsAtravesados = collection_list_create();
	t_list *lActiva;
	lPeticiones *pedido;
	pedidoStr *pedEnvio;
	respuestaTrace rtaTrace;


	//Preguntar por lista ACTIVA y se lo asignas a lActiva. si es scan lactiva recibe lped
	if ( algoritmo == FSCAN ){
		if (lped->tipoLista == ACTIVA){
			lActiva = lped;
		}
		else lActiva = lped2;

	}
	else lActiva = lped;

	//desencolar
	while (1){

		do {

			pthread_mutex_lock(&sem);
			if (lActiva ->elements_count >= 9) {
				printf("%d\n", lActiva->elements_count);
			}
			pedido =  collection_list_remove(lActiva,0);
			pthread_mutex_unlock(&sem);
			if (pedido != NULL) {

				//leer o escribir
				if ((pedido->tipo == (uint8_t)LECTURA) || (pedido->tipo == (uint8_t)SINCRO)) {
					pthread_mutex_lock(&sem);
					//					obtenerDirFisicaSector(pedido->sectorLogico, &pistaActual, &sectActual);
					//					sectActual++;
					//					if (sectActual == 1024){
					//						pistaActual++;
					//						sectActual = 0;
					//						if (pistaActual == 1024){
					//							pistaActual = 0;
					//						}
					//					}
					pistaAnterior = pistaActual;
					sectorAnterior = sectActual;
					t = MoverASector(pedido->sectorLogico, sectsAtravesados);//todo PISTA ACTUAL, EN LOG
					pthread_mutex_unlock(&sem);
					//LEER
					dataSector = malloc(512);
					readSector(dataSector, pedido->sectorLogico);

					if (logActivado == 1){
						loguearPeticionProcesada(pedido, t,lActiva, sectsAtravesados, pistaAnterior, sectorAnterior);
						printf("Envio pedido Lectura sector:%d a socket:%d  \n", pedido->sectorLogico, pedido->fd);
					}

					//ENVIAR
					pedEnvio = (pedidoStr *) malloc(sizeof(pedidoStr));
					pedEnvio->sector = pedido->sectorLogico;
					memcpy(pedEnvio->data, dataSector, 512);
					buf = (char *)malloc(sizeof(pedidoStr));
					memcpy(buf, pedEnvio, sizeof(pedidoStr));
					msg = serializarNipc(pedido->tipo, sizeof(pedidoStr), buf);
					send(pedido -> fd, msg, sizeof(NIPC_Header) + sizeof(pedidoStr), MSG_NOSIGNAL);

					//libero memoria dataSector y pedido

					free(msg);
					free(buf);
					free(pedEnvio);
					free(dataSector);
				}

				if ( (pedido->tipo == (uint8_t)ESCRITURA) || (pedido->tipo == (uint8_t)SINCRO_ESCRIBIR) ){
					// ESCRITURA
					pthread_mutex_lock(&sem);
					//					obtenerDirFisicaSector(pedido->sectorLogico, &pistaActual, &sectActual);
					//					sectActual++;
					//					if (sectActual == 1024){
					//						pistaActual++;
					//						sectActual = 0;
					//						if (pistaActual == 1024){
					//							pistaActual = 0;
					//						}
					//					}
					pistaAnterior = pistaActual;
					sectorAnterior = sectActual;
					t = MoverASector(pedido->sectorLogico, sectsAtravesados);
					pthread_mutex_unlock(&sem);
					// ESCRIBIR
					writeSector(pedido->data, pedido->sectorLogico);
					cantPedidosEscritura++;
					if(cantPedidosEscritura == 32000){
						persistirDisco();
						cantPedidosEscritura = 0;
					}

					if (logActivado == 1){
						loguearPeticionProcesada(pedido, t,lActiva, sectsAtravesados, pistaAnterior, sectorAnterior);
						printf("Envio pedido Escritura a sector:%d socket:%d  \n", pedEnvio->sector, pedido->fd);
					}
					//ENVIAR CONFIRMACION
					msg = serializarNipc(pedido->tipo, 0, NULL);
					send(pedido -> fd, msg, sizeof(NIPC_Header), MSG_NOSIGNAL);


					//libero memoria dataSector y pedido
					free(msg);
				}
				if (pedido->tipo == (uint8_t)TRACE){

					rtaTrace.sectLogActual = devolverDirLogicaActual();
					rtaTrace.sectLogSolic = pedido->sectorLogico;
					rtaTrace.sentido = sentido;

					pthread_mutex_lock(&sem);
					pistaAnterior = pistaActual;
					sectorAnterior = sectActual;
					t = MoverASector(pedido->sectorLogico, sectsAtravesados);
					//					obtenerDirFisicaSector(pedido->sectorLogico, &pistaActual, &sectActual);
					//					sectActual++;
					//					if (sectActual == 1024){
					//						pistaActual++;
					//						sectActual = 0;
					//						if (pistaActual == 1024){
					//							pistaActual = 0;
					//						}
					//					}
					pthread_mutex_unlock(&sem);

					if (logActivado == 1){
						loguearPeticionProcesada(pedido, t,lActiva, sectsAtravesados, pistaAnterior, sectorAnterior);
					}

					buf = (char *)malloc(sizeof(respuestaTrace));
					memcpy(buf, &rtaTrace, sizeof(respuestaTrace));
					msg = serializarNipc(TRACE, sizeof(respuestaTrace), buf);
					send(pedido->fd, msg, sizeof(NIPC_Header) + sizeof(respuestaTrace), MSG_NOSIGNAL);
					//Liberar Memoria

					free(buf);
					free(msg);
				}
				free(pedido);
			}
		} while (lActiva->elements_count != 0);
		pthread_mutex_lock(&sem);
		if (algoritmo == FSCAN ) {
			if (lActiva == lped) {
				lActiva = lped2;
				lped2->tipoLista = ACTIVA;
				lped->tipoLista = PASIVA;
			}
			else {
				lActiva = lped;
				lped->tipoLista = ACTIVA;
				lped2->tipoLista = PASIVA;
			}
		}
		pthread_mutex_unlock(&sem);

	}

	free(sectsAtravesados);
	return EXIT_SUCCESS;
}
int threadConsumidorInit(){
	int rc;
	pthread_t thread;


	rc = pthread_create(&thread, NULL,(void *) &threadConsumidor, (NULL));
	if (rc){
		printf("ERROR; return code from pthread_create() is %d\n", rc);
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;
}

int loguearPeticionProcesada(lPeticiones *pedido,double t,t_list *lActiva, t_list *sectsAtravesados, uint32_t pistaAnt, uint32_t sectorAnt){
	int i = 0;
	lPeticiones *pedidoTemporal = NULL;
	PisSec *pistaSector=NULL;

	log_info(archLog,"PPD","Petición de lectura del sector %u procesada correctamente.",pedido->sectorLogico);

	log_write_without_extra_info(archLog,"Cola de Pedidos:");
	for(i=0;i < collection_list_size(lActiva); i++){
		pedidoTemporal = collection_list_get(lActiva,i);
		log_write_without_extra_info(archLog," %u:%u,",pedidoTemporal->pista, pedidoTemporal->sectorEnPista);
	}
	log_write_without_extra_info(archLog,"\n");
	log_write_without_extra_info(archLog,"Tamaño: %d\n", collection_list_size(lActiva));
	log_write_without_extra_info(archLog,"Posición Actual: %u:%u\n", pistaAnt, sectorAnt);
	log_write_without_extra_info(archLog,"Sector Solicitado: %u:%u\n", pedido->pista, pedido->sectorEnPista);
	log_write_without_extra_info(archLog,"Sectores Recorridos:");
	while(collection_list_size(sectsAtravesados)){
		pistaSector = collection_list_remove(sectsAtravesados,0);
		log_write_without_extra_info(archLog," %u:%u,",pistaSector->pista, pistaSector->sector);
	}
	log_write_without_extra_info(archLog,"\n");
	log_write_without_extra_info(archLog,"Tiempo Consumido: %f ms\n",t);
	log_write_without_extra_info(archLog,"Próximo Sector: %u:%u\n\n", pedido->pista, (pedido->sectorEnPista) + 1 > (sectsxPista - 1)?0:(pedido->sectorEnPista) + 1);

	return EXIT_SUCCESS;
}



