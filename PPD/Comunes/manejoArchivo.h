/*
 * manejoArchivos.h
 *
 *  Created on: 07/11/2011
 *      Author: pablo
 */

#ifndef MANEJOARCHIVO_H_
#define MANEJOARCHIVO_H_

char *levantarConfig(char *nomConfig, char *identificador);
int OpenArchivo(char *path, int flag, struct stat *buf);


#endif /* MANEJOARCHIVO_H_ */
