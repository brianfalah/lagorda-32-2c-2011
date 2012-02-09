/*
 * ProtocoloDeMensajes.h
 *
 *  Created on: 08/11/2011
 *      Author: pablo
 */

#ifndef PROTOCOLODEMENSAJES_H_
#define PROTOCOLODEMENSAJES_H_

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Mensajes NIPC~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define HANDSHAKE 0
#define LECTURA 1
#define ESCRITURA 2
#define ERROR 3
#define PAYOK 4
#define SINCRO 5

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Estados Conexiones~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define PRIMERO 'P'
#define A_SINCRONIZAR  'S'
#define OCUPADO 'B'


#endif /* PROTOCOLODEMENSAJES_H_ */
