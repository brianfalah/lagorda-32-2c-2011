

#ifndef FUNCIONESDISCO_H_
#define FUNC

#define TAM_SECTOR 512
#define ASCENDENTE 1
#define DESCENDENTE 0

#define  ACTIVA 0
#define  PASIVA 1

#include <stdint.h>
#include "Comunes/librerias/collections/list.h"

typedef struct{
	uint32_t pista;
	uint32_t sector;
	char id[50];
} diskData;

int operacionesDeInicio(void);
int obtenerDirFisicaSector(uint32_t direccionLogica,unsigned int *pista,unsigned int *sector);
uint32_t devolverDirLogicaActual(void);
int cargarInfoInet(char **ip, int *puerto);
int handshake(int socket);

double MoverASector(uint32_t direccionLogica, t_list *sectsAtravesados);

void posicionarseEnSector( t_list *sectsAtravesados, double *rotationDelay, unsigned int sectorObjetivo);
int insertarSegunAlgoritmo(uint32_t pista);

typedef struct {
	uint32_t pista;
	uint32_t sector;
} __attribute__((__packed__))PisSec;

#endif /* FUNCIONESDISCO_H_ */
