

#ifndef FUNCIONESDISCO_H_
#define FUNC

#define TAM_SECTOR 512
#define ASCENDENTE 1
#define DESCENDENTE 0

#include <stdint.h>
#include "Comunes/librerias/collections/list.h"

int obtenerDirFisicaSector(uint32_t direccionLogica,unsigned int *pista,unsigned int *sector);

double MoverASector(uint32_t direccionLogica , t_list *sectsAtravesados, uint32_t pistaActual, uint32_t sectActual, int sentido, double *tpo);
void posicionarseEnSector(t_list *sectsAtravesados, double *rotationDelay , unsigned int sectorObjetivo,uint32_t pistaActual, uint32_t sectActual);

typedef struct {
	uint32_t pista;
	uint32_t sector;
} __attribute__((__packed__))PisSec;

#endif /* FUNCIONESDISCO_H_ */
