/*
 * threadRaid.h
 *
 *  Created on: 24/10/2011
 *      Author: pablo
 */

#ifndef THREADRAID_H_
#define THREADRAID_H_






void *ppdThread(char *id);		/*Thread x PPD*/

int asignarThreadPpd(char *id);
int respuestaPPD(int socketPfs, int socketPpd);

void *raidProcess();	// Thread Distribuidor de pedidos y Sincronizador de PPDs
int raidProcessInit();
#endif /* THREADRAID_H_ */
