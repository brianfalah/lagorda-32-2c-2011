/*
 * NIPC.h
 *
 *  Created on: 05/11/2011
 *      Author: pablo
 */

#ifndef NIPC_H_
#define NIPC_H_
#include <stdint.h>

typedef struct  {
        uint8_t type;
        int32_t payloadlength;
} __attribute__((__packed__)) NIPC_Header;


typedef struct  {
	NIPC_Header *cabecera;
    char *payload;
}__attribute__((__packed__)) NIPC_PKG;


typedef struct {
	uint32_t sector;
	char data[512];
}__attribute__((__packed__)) pedidoStr;

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

NIPC_PKG *deserializarNipc(char *buf, int32_t length);
NIPC_Header *deserializarHeader(char *buf);
char *serializarNipc(uint8_t type, int32_t payloadLength, char *msg);
char *cargarPedido(uint32_t sector, char *data);

#endif /* NIPC_H_ */
