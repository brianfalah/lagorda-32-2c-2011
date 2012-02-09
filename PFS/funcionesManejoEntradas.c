#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include "utils.h"
#include "funciones.h"
#include "funcionesManejoEntradas.h"

int tipoDeEntradaFat(unsigned int valorEntrada){
	int tipoEntrada=0;

	if (valorEntrada == 0x00000000)tipoEntrada = 2; //cluster libre
	else{
		if (valorEntrada == 0x00000001 || (valorEntrada >= 0x0FFFFFF0 && valorEntrada <= 0x0FFFFFF5) || valorEntrada == 0x0FFFFFF6 || valorEntrada == 0x0FFFFFF7) tipoEntrada = 3; //cluster reservado o usado o bad sector in cluster
		else{
			if (valorEntrada >= 0x00000002 && valorEntrada <= 0x0FFFFFEF){
				tipoEntrada = 4;//cluster usado, el valor apunta al proximo en la cadena
			}
			else{
				if (valorEntrada >= 0x0FFFFFF8 && valorEntrada <= 0x0FFFFFFF){
					tipoEntrada = 5; //Last cluster in file (EOC)
				}
			}

		}
	}

	return tipoEntrada;
}

int nombrarEntradas(char *nuevoNombre, tipoLFN *lfnEntry, tipoDirectorio *dirEntry){
	uint32_t tamNombreUtf16 = 0;
	uint16_t *nombreLfnEntry = NULL;
	int it=0, it2=0, restantes=0,largoNombreSinExt=0, largoExtension=0, ret=0;
	char *nombreSinExtension = malloc(strlen(nuevoNombre)+1);
	char *nombreDos = malloc(14), *extConPunto = NULL, *extConPuntoDOS = malloc(4);
	 //reservo mas por las dudas;char *nombreSinExtension = malloc(9);
	unsigned char *stringParaCheckSum = malloc(11);

	tamNombreUtf16 = strlen(nuevoNombre) * 2;
    nombreLfnEntry = malloc(100);
    unicode_utf8_to_utf16_inbuffer(nuevoNombre, strlen(nuevoNombre), nombreLfnEntry, &tamNombreUtf16);

//    memcpy(lfnEntry->nombre1, nombreLfnEntry, 10); //REVISAR ESTO, PODRIA SER MENOR QUE 10(5 chars)
//    for(it = 0;it < 6;it++){
//        lfnEntry->nombre2[it] = nombreLfnEntry[it + 5];
//    }
//    //RENOMBRANDO
//    for(it2 = 0;it2 < 2;it2++){
//        lfnEntry->nombre3[it2] = nombreLfnEntry[it2 + 11];
//    }

    if(strlen(nuevoNombre) < 5){
    	memcpy(lfnEntry->nombre1, nombreLfnEntry, strlen(nuevoNombre)*2);
        restantes = 5 - strlen(nuevoNombre);
        if(restantes != 0){
            lfnEntry->nombre1[strlen(nuevoNombre)] = 0x0000; //despues del ultimo caracter, el primer UTF16 es 0x00 0x00
            for(it = 0;it < restantes - 1;it++){
                lfnEntry->nombre1[strlen(nuevoNombre) + 1 + it] = 0xFFFF; ////los demas se rellenan con 0xFF 0xFF
            }
        }

        memset(lfnEntry->nombre2, 0xFF, 6 * 2);
        memset(lfnEntry->nombre3, 0xFF, 2 * 2);
    }
    else{
        if(strlen(nuevoNombre) < 11){
        	memcpy(lfnEntry->nombre1, nombreLfnEntry, 10);

            for(it = 0;it < strlen(nuevoNombre) - 5;it++){
                lfnEntry->nombre2[it] = nombreLfnEntry[it + 5];
            }

            restantes = 11 - strlen(nuevoNombre);
            if(restantes != 0){
                lfnEntry->nombre2[strlen(nuevoNombre) - 5] = 0x0000;
                for(it = 0;it < restantes - 1;it++){
                    lfnEntry->nombre2[strlen(nuevoNombre) - 5 + it + 1 ] = 0xFFFF; ////los demas se rellenan con 0xFF 0xFF
                }
            }

            memset(lfnEntry->nombre3, 0xFF, 2 * 2);
        }
        else{
        	memcpy(lfnEntry->nombre1, nombreLfnEntry, 10);

            for(it = 0;it < 6;it++){
                lfnEntry->nombre2[it] = nombreLfnEntry[it + 5];
            }

            for(it2 = 0;it2 < strlen(nuevoNombre) - 11; it2++){
                lfnEntry->nombre3[it2] = nombreLfnEntry[it2 + 11];
            }

            restantes = 13 - strlen(nuevoNombre);
            if(restantes != 0){
                lfnEntry->nombre3[strlen(nuevoNombre) - 11] = 0x0000;
                for(it = 0;it < restantes - 1;it++){
                    lfnEntry->nombre3[strlen(nuevoNombre) - 11 + it + 1] = 0xFFFF; ////los demas se rellenan con 0xFF 0xFF
                }
            }

        }

    }

    //convertir a DOS
    strcpy(nombreSinExtension, nuevoNombre);
    largoNombreSinExt = strlen(nombreSinExtension);
    if((extConPunto = strrchr(nuevoNombre, '.')) != NULL){//habria que buscar el ultimo punto que aparezca
		largoExtension = strlen(extConPunto+1);
		largoNombreSinExt = strcspn(nuevoNombre,".");//Regresa el numero de caracteres al principio de nombreLfn que no coinciden con '.'.
		strncpy(nombreSinExtension,nuevoNombre,largoNombreSinExt);
		nombreSinExtension[largoNombreSinExt]='\0';
	}
    ret = convertirANombreDos(nombreSinExtension, nombreDos);
    memcpy(dirEntry->nombreUTF8, nombreDos, strlen(nombreDos));
    memset(dirEntry->nombreUTF8 + strlen(nombreDos), ' ', 8 - strlen(nombreDos)); //Any unused space in the filename is padded with space characters
    if(extConPunto != NULL){
		for(it=0;it<(largoExtension>3? 3 :largoExtension);it++){
			extConPuntoDOS[it]=toupper(extConPunto[it+1]);
		}
		memcpy(dirEntry->extensionUTF8,extConPuntoDOS,it );
		memset(dirEntry->extensionUTF8+it,' ',3-it);//se rellena extension con espacios
	}
	else{
		memset(dirEntry->extensionUTF8,' ',3);//se rellena extension con espacios
	}
    //FIN convertir a DOS
    memcpy(stringParaCheckSum, dirEntry->nombreUTF8, sizeof (dirEntry->nombreUTF8));
    memcpy(stringParaCheckSum + 8, dirEntry->extensionUTF8, sizeof (dirEntry->extensionUTF8));
    lfnEntry->checkSum = lfn_checksum(stringParaCheckSum); //usamos la funcion de wikipedia para calcularle el checksum!

    free(nombreDos);
    free(extConPuntoDOS);
	free(nombreLfnEntry);
	free(nombreSinExtension);
	free(stringParaCheckSum);

    return EXIT_SUCCESS;
}

// A SFN filename(es lo mismo que DOS) can have at most 8 characters before the dot. If it has more than that, you should write the first 6, then put a tilde ~ as the seventh character and a number (usually 1) as the eight. The number distinguishes it from other files with both the same first six letters and the same extension.
int convertirANombreDos(char *nombreSinExtension,char *nombreDos){
	int i=0;
	char* nombreSinEspacios;
	char* nombreConEspacios = nombreSinExtension;

	nombreSinEspacios = removerEspaciosYPuntos(nombreConEspacios);

	if(strlen(nombreSinEspacios) > 8){
		for(i = 0; i < 6; i++)
		{
			nombreDos[i] = toupper(nombreSinEspacios[i]); //convertirmos a mayusculas
		}
		nombreDos[6] = '~';
		nombreDos[7]= '1';  //el numero depende de si existen otros archivos que empiezan con los mismos primeros 6 caracteres y la misma extension, si llego lo hago, por ahora queda asi
		nombreDos[8]= '\0';
	}
	else{
		for(i = 0; i < strlen(nombreSinEspacios); i++){
			nombreDos[i] = toupper(nombreSinEspacios[i]); //convertirmos a mayusculas
		}
		nombreDos[i]='\0';
	}

	//free(nombreSinEspacios);
	//free(nombreConEspacios);
	//nombreConEspacios = NULL;
	return 0;
}

char* removerEspaciosYPuntos(char* nombre){
	char *p1 = nombre;
	char *p2 = nombre;
	p1 = nombre;
	while(*p1 != 0) {
	if(ispunct(*p1) || isspace(*p1)) {
	++p1;
	}
	else
	*p2++ = *p1++;
	}
	*p2 = 0;
	return nombre;
}

unsigned char lfn_checksum(unsigned char *pFcbName){
        int i;
        unsigned char sum=0;

        for (i=11; i; i--)
                sum = ((sum & 1) << 7) + (sum >> 1) + *pFcbName++;
        return sum;
}

int esUnDirectorio(uint32_t numCluster,uint32_t posBytesEntrada){
	int ret=0, numBloque = 0, posEntrada=0;
	char *bloque = NULL;
	tipoDirectorio *dirEntry = malloc(32);

	numBloque = bloqueDeUnSector((int)posBytesEntrada/512);
	posEntrada = posicionEnBloque(posBytesEntrada);
	bloque = malloc(bs->bytesXsector * 4);
	ret = pedirBloque(bloque, numBloque);
	memcpy(dirEntry,bloque + posEntrada + 32, 32 ); //y dps las normales

	free(bloque);
	if(dirEntry->atributos == 0x10){
		free(dirEntry);
		return 0;
	}
	else{
		free(dirEntry);
		return 1;
	}

}

int esUnArchivo(uint32_t numCluster,uint32_t posBytesEntrada){
	int ret=0,numBloque = 0, posEntrada=0;
	char *bloque = NULL;
	tipoDirectorio *dirEntry = malloc(32);

	numBloque = bloqueDeUnSector((int)posBytesEntrada/512);
	posEntrada = posicionEnBloque(posBytesEntrada);
	bloque = malloc(bs->bytesXsector * 4);
	ret = pedirBloque(bloque, numBloque);
	memcpy(dirEntry,bloque + posEntrada + 32, 32 ); //y dps las normales

	if(dirEntry->atributos == 0x20){
		free(bloque);
		free(dirEntry);
		return EXIT_SUCCESS;
	}


	else{
		free(bloque);
		free(dirEntry);
		return 1;
	}
}

uint32_t buscarEntradasLibres(char *dataCluster,uint32_t *posBytesEntrada){
	int i=0, cantEntradasDobles = bs->sectorXcluster * bs->bytesXsector / 64;
	int encontradas = 0;
	tipoDirectorio *dirEntry = malloc(32);

	while(i <= cantEntradasDobles && !encontradas){
		memcpy(dirEntry,dataCluster + (i*64) + 32, 32 );
		if( dirEntry->nombreUTF8[0] == 0x00 || dirEntry->nombreUTF8[0] == 0xE5 || dirEntry->nombreUTF8[0] == -27 ){ //si encontramos 2 entradas vacias o borradas
			encontradas = 1;
			*posBytesEntrada = i*64;
			free(dirEntry);
			return encontradas;
		}
		i++;
	}

	free(dirEntry);
	return encontradas;
}

int asignarClusterAEntrada(tipoDirectorio *dirEntry , uint32_t numCluster) {

	char *bytesDelCluster = (char *) &numCluster;

	dirEntry->bytesBajosPrimerClust[0] = bytesDelCluster[0];
	dirEntry->bytesBajosPrimerClust[1] = bytesDelCluster[1];

	dirEntry->bytesAltosPrimerClust[0] = bytesDelCluster[2];
	dirEntry->bytesAltosPrimerClust[1] = bytesDelCluster[3];

	return 0;
}

