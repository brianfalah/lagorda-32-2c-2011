/*
 * threadPPD.h
 *
 *  Created on: 11/11/2011
 *      Author: pablo
 */

#ifndef THREADPPD_H_
#define THREADPPD_H_

#include <stdlib.h>
#include <stdint.h>
#include "Comunes/librerias/config_loader.h"
#include "Comunes/librerias/collections/list.h"

#define MAX_EVENTS 100

typedef struct{
	uint32_t pistas;
	uint32_t cabezas;
	uint32_t sectores;
	char id[50];
}chs;

extern chs cilCabSec;

char *pathConsola, *pathUnix, *pathDisco;

unsigned int sectsxPista;
double retardoRotEnMs,rpm,tpoEntrePistas,tpoEntreSectores;
unsigned int pistaActual, sectActual;
double tpoLectura, tpoEscritura;
int puerto;
char *modo;

void *threadConsumidor();
void *threadProductor();
int threadProductorInit();
int threadConsumidorInit();
int loguearPeticionProcesada(lPeticiones *pedido,double t,t_list *lActiva, t_list *sectsAtravesados, uint32_t pistaAnt, uint32_t sectAnt);



#endif /* THREADPPD_H_ */
