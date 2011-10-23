/*
 * NIPC.c
 *
 *  Created on: 05/11/2011
 *      Author: pablo
 */


#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "NIPC.h"




char *serializarNipc(uint8_t type, int32_t payloadLength, char *msg){
	NIPC_Header h;
	h.type = type;
	h.payloadlength = payloadLength;
	char *buf;
	int offset = 0;

	if (payloadLength == 0) {
		buf = malloc(sizeof(NIPC_Header));
		memcpy(buf, &h, sizeof(NIPC_Header));

		return buf;
	}
	else {
		buf = malloc(sizeof(NIPC_Header) + payloadLength);
		memcpy(buf, &h, sizeof(NIPC_Header));
		offset = sizeof(NIPC_Header);
		memcpy(buf + offset, msg, payloadLength);

		return buf;
	}
}



NIPC_PKG *deserializarNipc(char *buf, int32_t length){

	NIPC_PKG *pkg = malloc(sizeof(NIPC_PKG));
	NIPC_Header *head = malloc(sizeof(NIPC_Header));
	int offset = 0;

	memcpy(&head->type, buf, sizeof(uint8_t));
	offset += sizeof(uint8_t);

	memcpy(&head->payloadlength, buf + offset, sizeof(int32_t));
	offset += sizeof(int32_t);

	pkg->cabecera = head;
	pkg->payload = malloc(length);
	memcpy(pkg->payload, buf + offset, length);

	return pkg;
}

NIPC_Header *deserializarHeader(char *buf){

	NIPC_Header *head = malloc(sizeof(NIPC_Header));
	int offset = 0;

	memcpy(&head->type, buf, sizeof(uint8_t));
	offset += sizeof(uint8_t);

	memcpy(&head->payloadlength, buf + offset, sizeof(int32_t));

	return head;
}

char *cargarPedido(uint32_t sector, char *data){
	pedidoStr *pedido = malloc(sizeof(pedidoStr));
	int offset;
	char *buf;
	pedido->sector = sector;


	if (data == NULL) {
		buf = malloc(sizeof(pedidoStr));
		memcpy(buf, &pedido->sector, sizeof(uint32_t));
		offset = sizeof(pedidoStr);
		buf [offset] = NULL;
	}
	else{
		buf = malloc(sizeof(pedidoStr) + 512);
		memcpy(buf, &pedido->sector, sizeof(uint32_t));
		offset = sizeof(uint32_t);
		memcpy(buf + offset, data, 512);
	}
	return buf;

}
