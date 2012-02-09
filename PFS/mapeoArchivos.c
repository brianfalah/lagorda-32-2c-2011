#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mapeoArchivos.h"

extern fd;

int readSector(char *sector,uint32_t numSector){
	int ret=0, corrimiento=0;
	uint32_t offset = numSector * 512;
	//char *mapeo=malloc(4096);
	//char *pathDisco = "/home/pablo/Desktop/fat32v2.bin";
	//struct stat statbuf;

	//fd = open(pathDisco, O_RDONLY);
//	if (fstat(fd, &statbuf) < 0){
//		printf("Error fstat()");
//		return EXIT_FAILURE;
//	}

	//mapeo = mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, fd, offset);
	uint32_t page = floor(offset / 4096);
	char* mapeo = mmap(NULL, 4096, PROT_WRITE , MAP_SHARED, fd, page*4096);
	corrimiento = offset % 4096;
	//VER BIEN COMO LEER, POR LO QUE ESCUCHE TODOS USAN MMAP HAY QUE ENCONTRAR UNA RAZON
	ret = posix_madvise (mapeo + corrimiento, 512, POSIX_MADV_SEQUENTIAL);
	if (ret < 0)
	perror ("madvise");

	//memcpy(buf, mapeo, 36);
	memcpy(sector,mapeo + corrimiento,512);
	if (munmap(mapeo, 4096) == -1) {//LIBERAMOS MEMORIA
			perror("Error haciendo el mmap");
	}

	//sleep(tpoLectura);
	return EXIT_SUCCESS;
}

int writeSector(char *info,uint32_t numSector){
	int ret=0, corrimiento = 0, tamanoPagina;
	//char *mapeo=malloc(4096);
	uint32_t offset = numSector * 512;
	//char *pathDisco = "/home/pablo/Desktop/fat32v2.bin";

	struct stat statbuf;

	//fd = open(pathDisco, O_RDWR);
//	if (fstat(fd, &statbuf) < 0){
//		printf("Error fstat()");
//		return EXIT_FAILURE;
//	}

	//tamanoPagina = getpagesize(); //QUIZAS SIRVA PARA EL LEN

	//levantamos en memoria solo la pagina del sector a escribir
	//es MAP_PRIVATE porque no es necesario compartir la memoria con otros procesos
	//el offset a partir del cual leer debe ser un multiplo del tamaño de pagina
	uint32_t page = floor(offset / 4096);
	char* mapeo = mmap(NULL, 4096, PROT_WRITE , MAP_SHARED, fd, page*4096);
	corrimiento = offset % 4096;
	//VER BIEN COMO LEER Y ESCRIBIR, POR LO QUE ESCUCHE TODOS USAN MMAP HAY QUE ENCONTRAR UNA RAZON
	ret = posix_madvise (mapeo + corrimiento, 512, POSIX_MADV_SEQUENTIAL);
	if (ret < 0)
	perror ("madvise");

	memcpy(mapeo + corrimiento, info, 512);
	//the segment of the file is copied into RAM and periodically flushed to disk, synchronization can be forced with the msync system call.
	ret = msync(mapeo, 4096,MS_SYNC);
	if (ret < 0)
	perror ("msync");

	munmap(mapeo, 4096); //LIBERAMOS MEMORIA

	//sleep(tpoEscritura);
	return EXIT_SUCCESS;
}

