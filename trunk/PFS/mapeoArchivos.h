#ifndef MAPEOARCHIVOS_H_
#define MAPEOARCHIVOS_H_

#include <stdint.h>
#include <sys/mman.h>

int readSector(char *sector,uint32_t numSector);
int writeSector(char *info,uint32_t numSector);

#endif /* MAPEOARCHIVOS_H_ */
