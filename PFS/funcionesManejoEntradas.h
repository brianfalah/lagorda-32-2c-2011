/*
 * funcionesManejoEntradas.h
 *
 *  Created on: Nov 28, 2011
 *      Author: pablo
 */

#ifndef FUNCIONESMANEJOENTRADAS_H_
#define FUNCIONESMANEJOENTRADAS_H_

int esUnDirectorio(uint32_t numCluster,uint32_t posBytesEntrada);
int convertirANombreDos(char *nombreSinExtension,char *nombreDos);
unsigned char lfn_checksum(unsigned char *pFcbName);
char* removerEspaciosYPuntos(char* nombre);

#endif /* FUNCIONESMANEJOENTRADAS_H_ */
