#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "manejoArchivo.h"

char *levantarConfig(char *nomConfig, char *identificador){
	FILE *handle;
	char *bufArch, *valorBuscado, *token;
	struct stat dataArch;
	int ret = 0;
	char separadores[] = "<> \n\r";

	ret = stat(nomConfig, &dataArch);
	if (ret == -1){
		printf("error al cargar data Archivo");
		return NULL;
	}

	handle = fopen(nomConfig, "r+");
	if (handle == NULL){
		printf("error no se pudo levantar de %s  %s\n", nomConfig, identificador);
		return NULL;
	}

	bufArch = malloc(dataArch.st_size + 1);


	fread(bufArch, dataArch.st_size, dataArch.st_size, handle);

	token = strtok(bufArch, separadores);

	while (token != NULL) {
		if (strcmp(token, identificador) == 0) {
			valorBuscado = strtok(NULL, separadores);
			fclose(handle);
			free (bufArch);
			return valorBuscado;
		}
		token = strtok(NULL, separadores);
	}
	return "error";
}
