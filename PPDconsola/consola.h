/*
 * consola.h
 *
 *  Created on: 01/12/2011
 *      Author: magBejar
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

typedef struct  {
	        uint8_t type;
	        int32_t payloadlength;
	} __attribute__((__packed__)) NIPC_Header;


typedef struct {
	uint32_t sectLogActual;
	uint32_t sectLogSolic;
	int sentido;
	//double tpo;
} __attribute__((__packed__))respuestaTrace; //usar las mismas funciones q el ppd, pero pasando por parametros estos datos,dps imprimir todo

typedef struct {
	uint32_t sectorInicial;
	uint32_t sectorFinal;
} __attribute__((__packed__))respuestaClean;

typedef struct {
	uint32_t cantPistas;
	uint32_t sectsxPista;
	double tpoEntrePistas;
	double tpoEntreSectores;
} __attribute__((__packed__))handshakeConsola;

#define STARV 40

int handshakePpd(int sockDisco);

#endif /* CONSOLA_H_ */
