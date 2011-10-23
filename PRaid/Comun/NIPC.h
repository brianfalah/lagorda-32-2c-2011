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

NIPC_PKG *deserializarNipc(char *buf, int32_t length);
NIPC_Header *deserializarHeader(char *buf);
char *serializarNipc(uint8_t type, int32_t payloadLength, char *msg);
char *cargarPedido(uint32_t sector, char *data);

#endif /* NIPC_H_ */
