#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "Comun/ProtocoloDeMensajes.h"
#include "Comun/librerias/collections/list.h"
#include "funcionesRaid.h"




extern t_list *dc;
extern uint32_t sector;
extern uint32_t pista;
extern pthread_mutex_t sem;


lDiscos *BuscarDiscoPorId(char *id) {
	lDiscos *disco ;//= malloc(sizeof(lDiscos));;
	int i;
	for (i = 0; (i < dc->elements_count); i++){
		disco = collection_list_get(dc, i);
		if (strcmp(disco->id, id) == 0){
			return disco;
		}
	}
	return NULL;
}

lDiscos *buscarDiscoPorDesc(int socket){
	lDiscos *disco;
	int i;
	for (i = 0; (i < dc->elements_count); i++){
		disco = collection_list_get(dc, i);
		if (disco->descriptor == socket){
			return disco;
		}
	}
	return NULL;
}
int posDisco( char *id){
	lDiscos *disco = malloc(sizeof(lDiscos));;
		int i;
		for (i = 0; (i < dc->elements_count); i++){
			disco = collection_list_get(dc, i);
			if (strcmp(disco->id, id) == 0){
				return i;
			}
		}
	return -1;
}
int removerDiscoPorDesc(int socket){
	lDiscos *disco;
	int i;

	for (i = 0; (i < dc->elements_count); i++){
		disco = collection_list_get(dc, i);
		if (disco->descriptor == socket){
			collection_list_removeAndDestroy(dc, i, free);
			return 0;
		}
	}
	return -1;
}

int removerDisco(char *id){
	int pos;

	pos = posDisco(id);
	if (pos == -1){
		printf("No se Pudo remover disco, no existe\n");
		return -1;
	}
	collection_list_removeAndDestroy(dc, pos, free);
	return 0;
}


lDiscos *AgregarDisco(int desc, char* id, int idlen, char est){
	lDiscos *disco = malloc(sizeof(lDiscos));

	disco->descriptor = desc;
	disco->id = malloc(idlen + 1);
	strcpy(disco->id ,id);
	strncat(disco->id,"\0",idlen);
	disco->cantPetSincro = 0;
	disco->estado = est;
	disco->p = collection_list_create();

	collection_list_add(dc, disco);

	return disco;
}
int hayDiscosHabilitados(){
	lDiscos *disco;
	int i, cantDiscos = 0;
	if (dc->elements_count == 0){
		return 0;
	}
	for(i = 0; i < dc->elements_count; i++){
		disco = collection_list_get(dc, i);
		if (disco->estado == HABILITADO){
			cantDiscos++;
		}
	}
	return cantDiscos;
}
int imprimirDiscoHabilitado(){
	lDiscos *disco;
	int i;

	if (hayDiscosHabilitados() == 0){
		return 0;
	}
	for(i = 0; i < dc->elements_count; i++){
		disco = collection_list_get(dc, i);
		if (disco->estado == HABILITADO){
			printf(">Disco Habilitado: %s \n", disco->id);
		}
	}
	return 0;

}


lDiscos *discoHabilitado(){
	int minimo = 0;
	lDiscos *disco;// = malloc(sizeof(lDiscos));
	disco = collection_list_get(dc, 0);
	minimo = collection_list_size(disco->p);
	int i;

	for(i = 0; (i < dc->elements_count); i++){
		disco = collection_list_get(dc, i);
		if (disco->estado == HABILITADO){
			if (collection_list_size(disco->p) == 0){
				return disco;
			}
			else {
				if (minimo > collection_list_size(disco->p)) {
					minimo = collection_list_size(disco->p);
				}
			}
		}
	}
	for(i = 0; (i < dc->elements_count); i++){
		disco = collection_list_get(dc, i);
		if (collection_list_size(disco->p) == minimo && disco->estado == HABILITADO){
			return disco;
		}
	}
	return NULL;
}
int sincronizarDisco(int sock){

	lDiscos *disco;// = malloc(sizeof(lDiscos));
	uint32_t numeroSectores = (pista * sector) -1;
	uint32_t i = 0;
	int j;
	char data[512] = {0};
	//pedidoStr *ped;
	time_t t = time(NULL);
	struct tm* now = localtime(&t);

	printf("[%d:%d:%d] - Se inicia Sincronizacion del disco\n", now->tm_hour, now->tm_min, now->tm_sec);

	while (i <= numeroSectores){

		for(j = 0; j < dc->elements_count; j++){
			disco = collection_list_get(dc, j);
			if (disco->estado == HABILITADO){
				Agregar(disco->p, sock, i, data, 512, SINCRO);
				i++;
			}

		}
	}
	printf("Pedidos a Sincronizar cargados!!!\n");
	return 0;
}
int listarPedidosDe(char *id){
	lDiscos *disco = BuscarDiscoPorId(id);
	lPeticiones *pet;
	int i;

	for (i=0; i<disco->p->elements_count; i++){
		pet = collection_list_get(disco->p, i);
		printf("Para: %d, Tipo %u, Sector: %u\n", pet->descriptor, pet->tipoPeticion, pet->sector);
	}

	return 0;
}







/*PEDIDOS*/

/*
 * Agrega pedido a la lista, con los parametros indicados
 * */
lPeticiones *Agregar(t_list *p, int desc, uint32_t sector, char *data, int tamPed, uint8_t tPet){

	lPeticiones *pedido = malloc(sizeof(lPeticiones));

	pedido->descriptor = desc;
	pedido->tipoPeticion = tPet;
	pedido->sector = sector;
	memcpy(pedido->data, data, 512);

	collection_list_put(p, 0,pedido);

	return pedido;

}

/*
 * Distribuye pedido de lectura entre los discos HABILITADOS y con menos carga en sus colas
 * */
void distribuirPedidoLectura(int desc,pedidoStr *ped, int tamPed){

	lDiscos *disk;

	disk = discoHabilitado();
	if (disk!= NULL) {
		Agregar(disk->p, desc, ped->sector, ped->data, tamPed, (uint8_t)LECTURA);
		return;
	}
	printf("No se agrego Ningun pedido, no hay discos? distribuirPedidoLectura()\n");
	return;

}

/*
 * Carga pedido de escritura en todos los discos HABILITADOS
 * */
void distribuirPedidoEscritura(int desc,pedidoStr *ped, int pedlen){

	lDiscos *disk;
	int i;

	for (i = 0; i < dc->elements_count; i++){
		disk = collection_list_get(dc, i);
		if (disk->estado == HABILITADO){
			Agregar(disk->p, desc, ped->sector, ped->data, pedlen, ESCRITURA);
		}
	}
	return;
}

/*
 * Devuelve el siguiente pedido a procesar de acuerdo a la Escritura por demanda
 * */
lPeticiones *pedidoSiguiente(t_list* p){
	lPeticiones *ped;
	lPeticiones *pedCmp;
	int j,i = 0;
	int encontroLectura = 1, encontroEscritura = 1;

	if (collection_list_size(p) == 0){
		return NULL;
	}

	//ped = collection_list_get(p, 0);
	//Busco pedidos de SINCRO O SINCRO ESCRITURA (doy prioridad)
	while (i < p->elements_count){
		//ped = (lPeticiones *)malloc(sizeof(lPeticiones));
		ped = collection_list_get(p, i);
		if (ped->tipoPeticion == SINCRO || ped->tipoPeticion == SINCRO_ESCRIBIR){
			return collection_list_remove(p, i);
		}
		i++;
	}

	//Busco pedido siguiente Escritura por demanda
	i = 0;
	while (encontroLectura){

		ped = collection_list_get(p, i);
		if (i < p->elements_count) {
			if (ped->tipoPeticion == LECTURA){
				encontroLectura = 0;
			}else{
				i++;
			}
		}else{
			// Llego al final, que devuelva el ultimo
			return collection_list_remove(p, i-1);
		}
	}
	//encontre lectura busco escritura
	if (ped->tipoPeticion == LECTURA){
		j = 0;
		while (encontroEscritura){
			pedCmp = collection_list_get(p, j);
			if (j < p->elements_count) {
				if ((ped->sector == pedCmp->sector) && (pedCmp->tipoPeticion == ESCRITURA)){
					encontroEscritura = 0;
				}else{
					j++;
				}
			}else{
				//llego al final sin encontrar, devuelvo Lectura
				return collection_list_remove(p, i);
			}
		}
		//En este caso encontro una Escritura del mismo sector
		return collection_list_remove(p, j);
	}else{
		return collection_list_remove(p, i - 1);
	}
}

/*
 * Transfiere pedidos de la lista entre los discos HABILITADOS, tambien de acuerdo a la cantidad de elementos en sus colas
 * */
int transferirPedidos(t_list *p){
	lDiscos *disco;
	lPeticiones *pedido;
	int i;

	if (hayDiscosHabilitados() == 0){
		return -1;
	}
	while (p->elements_count !=0){

		for (i = 0; i < dc->elements_count; i++){
			disco = collection_list_get(dc, i);
			if (disco ->estado == HABILITADO){

				pedido = collection_list_remove(p, 0);
				if (pedido != NULL){
					if (pedido->tipoPeticion == ESCRITURA){
						free(pedido);
					}
					else{
						collection_list_put(disco->p, 0, pedido);
					}
				}

			}
		}
	}
	return 0;
}

/*
 * Procedimiento para Caida de Disco mientras se atiende un pedido(Aqui se llama a la funcion transferirPedidos\1 )
 * */
int caidaDeDisco(char *id, lPeticiones *pedidoActual){

	lDiscos *disco;
	int val = 0, pos;

	pthread_mutex_lock(&sem);
	//Transferir Pedidos de Disco Actual
	printf("---------------------------------------------------------------------------------\n");
	printf("Se detecto Desconexion de disco %s, deshabilitando...\n", id);
	pos = posDisco(id);
	disco = collection_list_remove(dc, pos);


	printf("Transfiriendo pedidos de disco caido...\n");
	val = transferirPedidos(disco->p);
	pthread_mutex_unlock(&sem);
	if (val == -1){
		printf("No hay Discos Habilitados para transferir pedidos, el Raid debe cerrarse.\n");
		printf("---------------------------------------------------------------------------------\n");
		//Cerrar Raid
		return -1;
	}
	printf("Removiendo disco...\n");



	//Close connection
	printf("Cerrando Conexion socket: %d \n", disco->descriptor);
	close(disco->descriptor);
	free(disco);
	printf("Extraccion Disco Completa.\n");

	//Pedido Actual
	disco = discoHabilitado();
	if (disco!= NULL) {
		Agregar(disco->p, pedidoActual->descriptor, pedidoActual->sector, pedidoActual->data, 512, pedidoActual->tipoPeticion);
	}
	else{
		printf("No se Encontro Lugar para el pedido Actual, Cerrar Raid.\n");
		return -1;
	}
	printf("---------------------------------------------------------------------------------\n");
	return 0;
}


