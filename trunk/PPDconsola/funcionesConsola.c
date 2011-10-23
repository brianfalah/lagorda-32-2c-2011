#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "Comunes/ProtocoloDeMensajes.h"
#include "Comunes/NIPC.h"
#include "funcionesConsola.h"


extern uint32_t cantPistas;
extern uint32_t sectsxPista;
extern double tpoEntrePistas;
extern double tpoEntreSectores;

int obtenerDirFisicaSector(uint32_t direccionLogica , unsigned int *pista , unsigned int *sector ){
	*pista = direccionLogica / sectsxPista;

	*sector = direccionLogica % sectsxPista;
	return 0;
}

double MoverASector(uint32_t direccionLogica , t_list *sectsAtravesados, uint32_t pistaActual, uint32_t sectActual, int sentido , double *tiempoTotal){

	double seekTime = 0;
	double rotationDelay = 0;
	int32_t pistaObjetivo = 0;
	int32_t sectorObjetivo = 0;
	int ret;
	int32_t i = 0;
	PisSec *pistaSector;

	//rpm = atoi(levantar("vda.config","RPM"));
	//retardoRotEnMs = (60/rpm)*1000;
	//tpoEntrePistas = atoi(levantar("vda.config","tpoEntrePistas"));
	//tpoEntreSectores = retardoRotEnMs/sectsxPista;
	//posInicial = levantar("vda.config","PosInicial");

	ret = obtenerDirFisicaSector(direccionLogica,&pistaObjetivo,&sectorObjetivo);

	if((pistaObjetivo > pistaActual) && (sentido == ASCENDENTE)){
		//sentido solamente ascendente, caso N1

		for( i = pistaActual; i <= pistaObjetivo; i++){

			pistaSector = malloc(sizeof(PisSec));
			pistaSector -> pista = i;
			pistaSector -> sector = sectActual;
			collection_list_add(sectsAtravesados, pistaSector);
			seekTime = seekTime + tpoEntrePistas;
		}
		pistaActual = i - 1;

	}
	if((pistaObjetivo < pistaActual) && (sentido == ASCENDENTE)){
		//esta ascendiendo y tiene que rebotar , caso N2

		for( i = pistaActual; i <= cantPistas - 1; i++){
			pistaSector = malloc(sizeof(PisSec));
			pistaSector->pista = i;
			pistaSector->sector = sectActual;
			collection_list_add(sectsAtravesados, pistaSector);
			seekTime = seekTime + tpoEntrePistas;
		}
		pistaActual = i;

		sentido = DESCENDENTE;

		for(i = (cantPistas - 2); i >= pistaObjetivo; i--){
			pistaSector = malloc(sizeof(PisSec));
			pistaSector -> pista = i;
			pistaSector -> sector = sectActual;
			collection_list_add(sectsAtravesados, pistaSector);
			seekTime = seekTime + tpoEntrePistas;
		}
		pistaActual = i + 1;

	}

	if((pistaObjetivo < pistaActual) && (sentido == DESCENDENTE)){
		// desciende solamente, caso N3

		for(i = pistaActual; i >= pistaObjetivo; i--){
			pistaSector = malloc(sizeof(PisSec));
			pistaSector -> pista = i;
			pistaSector -> sector = sectActual;
			collection_list_add(sectsAtravesados, pistaSector);
			seekTime = seekTime + tpoEntrePistas;
		}
		pistaActual = i + 1;

	}

	if((pistaObjetivo > pistaActual) && (sentido == DESCENDENTE)){
		//caso N4, desciende y rebota en 0

		for(i = pistaActual; i >= 0; i--){
			pistaSector = malloc(sizeof(PisSec));
			pistaSector -> pista = i;
			pistaSector -> sector = sectActual;
			collection_list_add(sectsAtravesados, pistaSector);
			seekTime = seekTime + tpoEntrePistas;
		}
		pistaActual = i;

		sentido = ASCENDENTE;

		for(i = 1; i <= pistaObjetivo; i++){
			pistaSector = malloc(sizeof(PisSec));
			pistaSector -> pista = i;
			pistaSector -> sector = sectActual;
			collection_list_add(sectsAtravesados, pistaSector);
			seekTime = seekTime + tpoEntrePistas;
		}
		pistaActual = i - 1;


	}
	if(pistaActual >= cantPistas){
		pistaActual = pistaActual % cantPistas;
	}

	posicionarseEnSector(sectsAtravesados, &rotationDelay, sectorObjetivo, pistaActual, sectActual);
	*tiempoTotal = seekTime + rotationDelay;


	return 0;

}


void posicionarseEnSector(t_list *sectsAtravesados, double *rotationDelay , unsigned int sectorObjetivo, uint32_t pistaActual, uint32_t sectActual){
	PisSec *pistaSector;

	if( sectActual == sectorObjetivo ){
		//Caimos justo!!

		pistaSector = malloc(sizeof(PisSec));
		pistaSector -> pista = pistaActual;
		pistaSector -> sector = sectActual;
		collection_list_add(sectsAtravesados, pistaSector);
		*rotationDelay = *rotationDelay + tpoEntreSectores;

		sectActual = sectorObjetivo + 1;

		if(sectActual == sectsxPista){
			sectActual = 0;
		}

	}
	else{
		sectActual++ ;
		*rotationDelay = *rotationDelay + tpoEntreSectores;

		if( sectActual <= sectorObjetivo ) {//EL SECTOR ACTUAL ES MENOR AL OBJETIVO

			while( sectActual <= sectorObjetivo ) {

				pistaSector = malloc(sizeof(PisSec));
				pistaSector -> pista = pistaActual;
				pistaSector -> sector = sectActual;
				collection_list_add(sectsAtravesados, pistaSector);
				*rotationDelay = *rotationDelay + tpoEntreSectores;

				sectActual++;
			}

			if(sectActual == sectsxPista){
				sectActual = 0;
			}
		}

		else{//SECTOR ACTUAL MAYOR AL SECTOR OBJETIVO
			while( sectActual < sectsxPista ){

				pistaSector = malloc(sizeof(PisSec));
				pistaSector -> pista = pistaActual;
				pistaSector -> sector = sectActual;
				collection_list_add(sectsAtravesados, pistaSector);
				*rotationDelay = *rotationDelay + tpoEntreSectores;

				sectActual++;
			}
			//TERMINA WHILE POR LLEGAR A 1024 (SECTSXPISTA)
			sectActual=0;


			if(pistaActual == cantPistas){
				pistaActual = 0;
			}

			//EMPIEZA EL CICLO DESDE 0 CON SENTIDO ASCENDENTE
			while( sectActual <= sectorObjetivo ){

				pistaSector = malloc(sizeof(PisSec));
				pistaSector -> pista = pistaActual;
				pistaSector -> sector = sectActual;
				collection_list_add(sectsAtravesados, pistaSector);
				*rotationDelay=*rotationDelay+tpoEntreSectores;

				sectActual++;
			}
			if(sectActual == sectsxPista){
				sectActual = 0;
			}

		}

	}
}

void inicializaArray(int32_t *array){
	int i;
	for (i=0; i<=4; i++ ){
		array[i] = -1 ;
	}
	return ;
}
