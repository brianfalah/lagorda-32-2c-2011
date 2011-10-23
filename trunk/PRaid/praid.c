
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <pthread.h>
#include "praid.h"
#include "threadRaid.h"
#include "Comun/librerias/collections/list.h"
#include "./Comun/NIPC.h"
#include "funcionesRaid.h"
#include "./Comun/ProtocoloDeMensajes.h"
#include "./Comun/manejoArchivo.h"
#include "./Comun/sockets.h"



#define MAX_EVENTS 100
#define BUF_LEN 512




t_list *dc = NULL;
pthread_mutex_t sem;
uint32_t pista;
uint32_t sector;
diskData *datosDisco;

int main(){

	struct epoll_event ev, events[MAX_EVENTS];
	struct sockaddr_in clienteAddr;
	int nfds, epollFd, listenSocket, acceptSocket, puerto, ret, n, paylength;
	char *aux = NULL, *errorMsg, *msg;
	int addrLen = sizeof(clienteAddr);
	int error = 0;

	pedidoStr ped;
	diskData *datosDisco;
	NIPC_Header *head;



	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Pruebas~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	/*
	dc = collection_list_create();
	AgregarDisco(5,"disk1",strlen("disk1"), HABILITADO);
	AgregarDisco(6,"disk2",strlen("disk1"), HABILITADO);
	AgregarDisco(5,"disk3",strlen("disk1"), A_SINCRONIZAR);

	while (i <= 99){

			for(j = 0; j < dc->elements_count; j++){

				disco = (lDiscos *)malloc(sizeof(lDiscos));
				bzero(disco, sizeof(lDiscos));
				disco = collection_list_get(dc, j);

				if (disco->estado == HABILITADO){
					ped = (pedidoStr*)malloc(sizeof(pedidoStr));
					ped->sector = i;
					ped->data = NULL;
					Agregar(disco->p, 10, ped, sizeof(pedidoStr), SINCRO);
					i++;
				}
			}
	}
	disco = (lDiscos *)malloc(sizeof(lDiscos));
	disco = BuscarDiscoPorId("disk1");
	for(j = 0; j < disco->p->elements_count; j++){
		pedido = (lPeticiones *) malloc(sizeof(lPeticiones));
		pedido = collection_list_get(disco->p, j);
		printf("Descriptor:%d 	Sector:%u \n", pedido->descriptor, pedido->data->sector);
	}

	disco = (lDiscos *)malloc(sizeof(lDiscos));
	disco = BuscarDiscoPorId("disk2");
	for(j = 0; j < disco->p->elements_count; j++){
		pedido = (lPeticiones *) malloc(sizeof(lPeticiones));
		pedido = collection_list_get(disco->p, j);
		printf("Descriptor:%d 	Sector:%u \n", pedido->descriptor, pedido->data->sector);
	}

	 */
	/*
	pista = 100;
	sector = 100;

	dc = collection_list_create();

	AgregarDisco(8, "disk1", strlen("disk1"), HABILITADO);
	AgregarDisco(4, "disk2", strlen("disk1"), A_SINCRONIZAR);
	sincronizarDisco(4);
	//listarPedidosDe("disk1");

	disco = BuscarDiscoPorId("disk1");
	for(i = 0; i<20; i++){
		ped = (pedidoStr *)malloc(sizeof(pedidoStr));
		ped->sector = i;
		Agregar(disco->p, (i+20), ped, sizeof(pedidoStr), ESCRITURA);
		Agregar(disco->p, (i+20), ped, sizeof(pedidoStr), LECTURA);
	}
	printf("Pedidos:\n");
	while (1 && termino){
		pedido = pedidoSiguiente(disco->p);
		if (pedido != NULL){
			printf("Socket: %d --- Sector: %u --- Tipo: %u \n",pedido->descriptor, pedido->contenido->sector,pedido->tipoPeticion);
			printf("------------------------------------------\n");
		}
		else{
			termino = 0;
		}
	}
	printf("---END---\n");
	 */

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


	raidProcessInit();


	//Creo epoll
	epollFd = epoll_create(1000);
	if (epollFd == -1) {
		perror("epoll_create");
		exit(EXIT_FAILURE);
	}



	puerto = atoi(levantarConfig("praid.config", "Puerto"));
	if (inicializaSocketInet(&listenSocket, puerto) == 1){//Inicializo listenSocket para escuchar conexiones
		perror("fallo inicializa socket");
		return EXIT_FAILURE;
	}


	ev.events = EPOLLIN;			//Agrego el primer socket de Escucha(listenSocket) a la lista epoll
	ev.data.fd = listenSocket;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, listenSocket, &ev) == -1) {
		perror("epoll_ctl: listenSocket");
		exit(EXIT_FAILURE);
	}


	system("clear");
	printf("-------------------------------\n");
	printf("Esperando conexiones...\n");
	printf("-------------------------------\n");

	for (;;) {			//Bucle Principal infinito, Esperando conexiones y peticiones de PFS en el wait

		nfds = epoll_wait(epollFd, events, MAX_EVENTS, -1);
		imprimirDiscoHabilitado();
		if (nfds == -1 && errno != EINTR){
			printf("error: %s , number :%d\n", strerror(errno), errno);
			exit(EXIT_FAILURE);
		}

		for (n = 0; n < nfds; ++n) {

			if (events[n].data.fd == listenSocket) {			//si es socketEscucha, agrego conexion
				acceptSocket = accept(listenSocket, (struct sockaddr *) &clienteAddr, &addrLen);
				if (acceptSocket == -1) {
					perror("accept");
					exit(EXIT_FAILURE);
				}
				printf("--------------------------------------------------------------------\n");
				printf("Se Abrio Conexion en Socket %d\n",acceptSocket);
				ev.events = EPOLLIN | EPOLLET; 				//agregamos evento-fd
				ev.data.fd = acceptSocket;
				if (epoll_ctl(epollFd, EPOLL_CTL_ADD, acceptSocket, &ev) == -1) {
					perror("epoll_ctl: acceptSocket");
					exit(EXIT_FAILURE);
				}
			}
			else {
				head = (NIPC_Header *)malloc(sizeof(NIPC_Header));
				ret = recv(events[n].data.fd, head, sizeof(NIPC_Header), MSG_WAITALL);	//recibir info  por ahora no recibe mas de 512 bytes

				if (ret > 0){

					switch (head->type) {
					case (uint8_t)HANDSHAKE:
					if (head->payloadlength == (int32_t )0){

						if (hayDiscosHabilitados() == 0){
							paylength = strlen("No hay Discos conectados") + 1;
							errorMsg = malloc(paylength);
							strcpy(errorMsg, "No hay Discos conectados");
							strcat(errorMsg,"\0");
							aux = serializarNipc(ERROR, paylength, errorMsg);
							send(events[n].data.fd, aux, sizeof(NIPC_Header) + paylength, MSG_NOSIGNAL);
							free(errorMsg);

						}else{
							aux = serializarNipc(PAYOK, 0, NULL);//que hago con los pfs conectados???
							send(events[n].data.fd, aux, sizeof(NIPC_Header), MSG_NOSIGNAL);

						}
						free(aux);

					}
					else{
						// HANDSHAKE PPD
						datosDisco = (diskData *) malloc(sizeof(diskData));
						ret = recv(events[n].data.fd, datosDisco, head->payloadlength, MSG_WAITALL);
						if (ret > 0){

							if (BuscarDiscoPorId(datosDisco->id) == NULL){
								if (collection_list_size(dc) == 0){
									pthread_mutex_lock(&sem);
									AgregarDisco(events[n].data.fd, datosDisco->id, head->payloadlength, HABILITADO);
									pista = datosDisco->pista;
									sector = datosDisco->sector;
									error = 0;
									pthread_mutex_unlock(&sem);
								}
								else{
									if (datosDisco->pista == pista && datosDisco->sector == sector){
										pthread_mutex_lock(&sem);
										AgregarDisco(events[n].data.fd, datosDisco->id, head->payloadlength, A_SINCRONIZAR);
										sincronizarDisco(events[n].data.fd);
										error = 0;
										pthread_mutex_unlock(&sem);
									}
									else{
										error = 1;
										printf("Error Se intento conectar disco con CHS no compatible: C:%d, H:1, S:%d\n", datosDisco->pista, datosDisco->sector);
									}
								}
							}
							else{
								error = 1;
								printf("Error Se intento conectar disco con nombre ya existente... %s\n", datosDisco->id);
							}
							if (error){
								paylength = strlen("Ya existe un Disco conectado con el mismo ID") + 1;
								errorMsg = malloc(paylength);
								strcpy(errorMsg, "Ya existe un Disco conectado con el mismo ID");
								strcat(errorMsg,"\0");
								aux = (char *) serializarNipc(ERROR, paylength, errorMsg);

								send(events[n].data.fd, aux, sizeof(NIPC_Header) + paylength, MSG_NOSIGNAL);
								free(errorMsg);
								printf("Cerrar Conexion socket : %d",events[n].data.fd);
								close(events[n].data.fd);
							}
							else{

								aux = (char *)serializarNipc(PAYOK, 0, NULL);
								send(events[n].data.fd, aux, sizeof(NIPC_Header), MSG_NOSIGNAL);
								if (asignarThreadPpd(datosDisco->id) == EXIT_FAILURE) {
									printf("Error pthreadInit");
								}
								/*SACAR DEL EPOLL*/
								if (epoll_ctl(epollFd, EPOLL_CTL_DEL, events[n].data.fd, &ev) == -1) {
									perror("epoll_ctl: sacar del Epoll");
									exit(EXIT_FAILURE);
								}

							}
							free(aux);
						}
						else{
							//error closeConection?
						}
						free(datosDisco);
					}
					break;

					case (uint8_t)LECTURA:
					bzero(&ped, sizeof(pedidoStr));
					ret = recv(events[n].data.fd, &ped, head->payloadlength, MSG_WAITALL);

					if (ret > 0){
						pthread_mutex_lock(&sem);
						distribuirPedidoLectura(events[n].data.fd, &ped, head->payloadlength);
						pthread_mutex_unlock(&sem);

					}else{
						//error No mando nada
					}
					break;

					case (uint8_t)ESCRITURA:
					bzero(&ped, sizeof(pedidoStr));
					ret = recv(events[n].data.fd, &ped, head->payloadlength, MSG_WAITALL);

					if (ret > 0){
						pthread_mutex_lock(&sem);
						distribuirPedidoEscritura(events[n].data.fd, &ped, head->payloadlength);
						pthread_mutex_unlock(&sem);
					}else{
						//error closeConection?
					}
					msg = serializarNipc(ESCRITURA, 0, NULL);
					send(events[n].data.fd, msg, sizeof(NIPC_Header), MSG_NOSIGNAL);


					//libero memoria dataSector y pedido
					free(msg);
					break;

					default:
						printf("nada por ahora...\n");
						break;
					}

				}
				if (ret < 0) {
					perror("recv socket %d failed");

				}
				if (ret  == 0){	//cierre de conexion y remover de lista de eventos
					if (epoll_ctl(epollFd, EPOLL_CTL_DEL, events[n].data.fd, &ev) == -1) {
						perror("epoll_ctl: sacar del Epoll");
						exit(EXIT_FAILURE);
					}
					if (removerDiscoPorDesc(events[n].data.fd) != -1){
						printf("Desconexion Disco\n");
					}else{
						printf("Desconexion File System\n");
					}
					printf("Cerro Conexion Socket: %d \n",events[n].data.fd);
					close(events[n].data.fd);
				}

				free(head);
			}

		}

	}
	return 0;
}



