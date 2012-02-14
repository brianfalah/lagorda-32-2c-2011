#ifndef MAPEOARCHIVOS_H_
#define MAPEOARCHIVOS_H_

#include <stdint.h>
#include <sys/mman.h>

int writeSector(char *info,uint32_t numSector);
int readSector(char *sector,uint32_t numSector);
int mapearDisco(char *pathDisco);
int persistirDisco();
#endif /* MAPEOARCHIVOS_H_ */
