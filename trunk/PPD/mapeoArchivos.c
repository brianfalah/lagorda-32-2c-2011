#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include "mapeoArchivos.h"

char *mapeo = NULL;
extern double tpoLectura;
extern double tpoEscritura;
extern int starv;

int mapearDisco(char *pathDisco) {

	int ret = 0;
	struct stat statbuf;
	uint32_t fd = 0;

	fd = open(pathDisco, O_RDWR);

	if (fstat(fd, &statbuf) < 0){
		printf("Error fstat()");
		return EXIT_FAILURE;
	}

	mapeo = mmap(NULL, statbuf.st_size, PROT_WRITE , MAP_SHARED, fd, 0);
	//VER BIEN COMO LEER, POR LO QUE ESCUCHE TODOS USAN MMAP HAY QUE ENCONTRAR UNA RAZON
	ret = posix_madvise (mapeo, statbuf.st_size, POSIX_MADV_SEQUENTIAL);
	if (ret < 0)
	perror ("madvise");

	close(fd);

}

int readSector(char *sector,uint32_t numSector){
//	int ret=0, corrimiento=0;
	uint32_t offset = numSector * 512;

	memcpy(sector,mapeo + offset,512);
//	if (munmap(mapeo, 4096) == -1) {//LIBERAMOS MEMORIA
//			perror("Error haciendo el mmap");
//	}
	if (starv) {
		sleep(1);
	}else{
		if(tpoLectura != 0)	sleep(tpoLectura);
	}
	return EXIT_SUCCESS;
}

int writeSector(char *info,uint32_t numSector){
//	int ret=0, corrimiento = 0, tamanoPagina;
	uint32_t offset = numSector * 512;

	//tamanoPagina = getpagesize(); //QUIZAS SIRVA PARA EL LEN

	//levantamos en memoria solo la pagina del sector a escribir
	//es MAP_PRIVATE porque no es necesario compartir la memoria con otros procesos
	//el offset a partir del cual leer debe ser un multiplo del tamaño de pagina

	//VER BIEN COMO LEER Y ESCRIBIR, POR LO QUE ESCUCHE TODOS USAN MMAP HAY QUE ENCONTRAR UNA RAZON

	memcpy(mapeo + offset, info, 512);


//	munmap(mapeo, 4096); //LIBERAMOS MEMORIA
	if (starv) {
		sleep(1);
	}else {
		if(tpoEscritura != 0) sleep(tpoEscritura);
	}

	return EXIT_SUCCESS;
}

int persistirDisco() {

	int ret = 0;

	//the segment of the file is copied into RAM and periodically flushed to disk, synchronization can be forced with the msync system call.
	ret = msync(mapeo, 4096,MS_SYNC);
	if (ret < 0)
	perror ("msync");

}

