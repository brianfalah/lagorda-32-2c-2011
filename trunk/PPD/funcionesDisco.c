#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "Comunes/librerias/collections/list.h"
#include "Comunes/librerias/collections/collections.h"
#include "Comunes/ProtocoloDeMensajes.h"
#include "Comunes/librerias/log.h"
#include "Comunes/librerias/config_loader.h"
#include "Comunes/NIPC.h"
#include "ppd.h"
#include "threadPPD.h"
#include "funcionesDisco.h"
#include "listaPPD.h"




extern t_list *lped;
extern t_list *lped2;
extern int sentido;
extern int algoritmo;
extern t_log *archLog;
extern uint32_t pistaActual;
extern uint32_t sectActual;
extern chs cilCabSec;
extern uint32_t cantPedidosEscritura;
extern int logActivado;
extern int starv;

int operacionesDeInicio(){

	char *bufConfig = NULL;
	int ret;


	bufConfig = config_loader_open("ppd.config");



	cilCabSec.pistas = config_loader_getInt(bufConfig, "Pistas");
	cilCabSec.cabezas = 1;
	cilCabSec.sectores = config_loader_getInt(bufConfig, "Sectores");
	sectsxPista = cilCabSec.sectores;


	if(cilCabSec.pistas <= 0 || cilCabSec.sectores <= 0 ){
		printf("El disco debe tener un numero de cilindros >= a 0, y un numero de sectores mayor a 0.");
		return 1;
	}
	else{
		strcpy(cilCabSec.id, config_loader_getString(bufConfig, "Nombre"));
		pathUnix = config_loader_getString(bufConfig, "ArchConsola");
		pathConsola = config_loader_getString(bufConfig, "PathConsola");
		pathDisco = config_loader_getString(bufConfig, "PathDisco");
		puerto = config_loader_getInt(bufConfig,"Puerto");
		modo = config_loader_getString(bufConfig, "Modo");
		rpm = config_loader_getInt(bufConfig, "RPM");
		retardoRotEnMs=(60/rpm)*1000;
		tpoEntrePistas = config_loader_getDouble(bufConfig, "tpoEntrePistas");
		tpoEntreSectores = retardoRotEnMs / cilCabSec.sectores;
		logActivado = config_loader_getInt(bufConfig, "flagLogs");
		algoritmo = config_loader_getInt(bufConfig, "algoritmo");

		pistaActual = config_loader_getInt(bufConfig, "pistaInicial");
		sectActual = config_loader_getInt(bufConfig, "sectorInicial");

		tpoLectura = config_loader_getDouble(bufConfig, "tpoLectura") * 1000000;
		tpoEscritura = config_loader_getDouble(bufConfig, "tpoEscritura") * 1000000;
		mapearDisco(pathDisco);
		if (logActivado){
				archLog = log_create("PPD", "ppd.log", INFOLOG, M_CONSOLE_DISABLE);
		}
		return 0;
	}
}
int cargarInfoInet(char **ip, int *puerto){
	char *bufConfig = NULL;
	bufConfig = config_loader_open("ppd.config");

	*ip = config_loader_getString(bufConfig, "RaidIP");
	*puerto = config_loader_getInt(bufConfig, "PuertoRaid");

	return 0;

}
uint32_t devolverDirLogicaActual(void){
	return pistaActual * sectsxPista + sectActual;
}

int obtenerDirFisicaSector(uint32_t direccionLogica , uint32_t *pista , uint32_t *sector ){
	*pista = direccionLogica / sectsxPista;
	*sector = direccionLogica % sectsxPista;
	return 0;
}

double MoverASector(uint32_t direccionLogica , t_list *sectsAtravesados){
	double tiempoTotal = 0;
	double seekTime = 0;
	double rotationDelay = 0;
	int32_t pistaObjetivo = 0;
	int32_t sectorObjetivo = 0;
	int ret;
	int32_t i = 0;
	PisSec *pistaSector;

	ret = obtenerDirFisicaSector(direccionLogica,&pistaObjetivo,&sectorObjetivo);

	//sentido solamente ascendente, caso N1---------------------------------------------------
	if((pistaObjetivo > pistaActual) && (sentido == ASCENDENTE)){

//		for( i = pistaActual; i <= pistaObjetivo; i++){
//			if(logActivado){
//				pistaSector = malloc(sizeof(PisSec));
//				pistaSector -> pista = i;
//				pistaSector -> sector = sectActual;
//				collection_list_add(sectsAtravesados, pistaSector);
//				seekTime = seekTime + tpoEntrePistas;
//			}
		i = pistaActual;
		while(i<= pistaObjetivo){

			if(logActivado){
				pistaSector = malloc(sizeof(PisSec));
				pistaSector -> pista = i;
				pistaSector -> sector = sectActual;
				collection_list_add(sectsAtravesados, pistaSector);
				seekTime = seekTime + tpoEntrePistas;
			}
			i++;
		}
		pistaActual = i - 1;
	}

	//esta ascendiendo y tiene que rebotar , caso N2------------------------------------------------

	if((pistaObjetivo < pistaActual) && (sentido == ASCENDENTE)){

		//		for( i = pistaActual; i <= cilCabSec.pistas - 1; i++){
		//			if(logActivado){
		//				pistaSector = malloc(sizeof(PisSec));
		//				pistaSector->pista = i;
		//				pistaSector->sector = sectActual;
		//				collection_list_add(sectsAtravesados, pistaSector);
		//				seekTime = seekTime + tpoEntrePistas;
		//			}

		i = pistaActual;
		while (i <= (cilCabSec.pistas - 1)){
			if(logActivado){
				pistaSector = malloc(sizeof(PisSec));
				pistaSector->pista = i;
				pistaSector->sector = sectActual;
				collection_list_add(sectsAtravesados, pistaSector);
				seekTime = seekTime + tpoEntrePistas;
			}
			i++;
		}

		sentido = DESCENDENTE;

		//		for(i = (cilCabSec.pistas - 2); i >= pistaObjetivo; i--){
		//			if(logActivado){
		//				pistaSector = malloc(sizeof(PisSec));
		//				pistaSector -> pista = i;
		//				pistaSector -> sector = sectActual;
		//				collection_list_add(sectsAtravesados, pistaSector);
		//				seekTime = seekTime + tpoEntrePistas;

		i = cilCabSec.pistas - 2;

		while(i >= pistaObjetivo){
			if(logActivado){
				pistaSector = malloc(sizeof(PisSec));
				pistaSector -> pista = i;
				pistaSector -> sector = sectActual;
				collection_list_add(sectsAtravesados, pistaSector);
				seekTime = seekTime + tpoEntrePistas;
			}
			i--;
		}
		pistaActual = i + 1;
	}

	// desciende solamente, caso N3-------------------------------------------------------------------
	if((pistaObjetivo < pistaActual) && (sentido == DESCENDENTE)){


//		for(i = pistaActual; i >= pistaObjetivo; i--){
//			if(logActivado){
//				pistaSector = malloc(sizeof(PisSec));
//				pistaSector -> pista = i;
//				pistaSector -> sector = sectActual;
//				collection_list_add(sectsAtravesados, pistaSector);
//				seekTime = seekTime + tpoEntrePistas;
//			}

		i = pistaActual;

		while(i >= pistaObjetivo){
			if(logActivado){
				pistaSector = malloc(sizeof(PisSec));
				pistaSector -> pista = i;
				pistaSector -> sector = sectActual;
				collection_list_add(sectsAtravesados, pistaSector);
				seekTime = seekTime + tpoEntrePistas;
			}
			i--;
		}
		pistaActual = i + 1;
	}

	//caso N4, desciende y rebota en 0------------------------------------------------------

	if((pistaObjetivo > pistaActual) && (sentido == DESCENDENTE)){

//		for(i = pistaActual; i >= 0; i--){
//			if(logActivado){
//				pistaSector = malloc(sizeof(PisSec));
//				pistaSector -> pista = i;
//				pistaSector -> sector = sectActual;
//				collection_list_add(sectsAtravesados, pistaSector);
//				seekTime = seekTime + tpoEntrePistas;
//			}

		i = pistaActual;
		while(i >= 0){
			if(logActivado){
				pistaSector = malloc(sizeof(PisSec));
				pistaSector -> pista = i;
				pistaSector -> sector = sectActual;
				collection_list_add(sectsAtravesados, pistaSector);
				seekTime = seekTime + tpoEntrePistas;
			}
			i--;
		}

		sentido = ASCENDENTE;

//		for(i = 1; i <= pistaObjetivo; i++){
//			if(logActivado){
//				pistaSector = malloc(sizeof(PisSec));
//				pistaSector -> pista = i;
//				pistaSector -> sector = sectActual;
//				collection_list_add(sectsAtravesados, pistaSector);
//				seekTime = seekTime + tpoEntrePistas;
//			}
		i = 1;
		while(i <= pistaObjetivo){
			if(logActivado){
				pistaSector = malloc(sizeof(PisSec));
				pistaSector -> pista = i;
				pistaSector -> sector = sectActual;
				collection_list_add(sectsAtravesados, pistaSector);
				seekTime = seekTime + tpoEntrePistas;
			}
			i++;
		}
		pistaActual = i - 1;

	}
	posicionarseEnSector(sectsAtravesados, &rotationDelay, sectorObjetivo);
	tiempoTotal = seekTime + rotationDelay;

return tiempoTotal;
}

void posicionarseEnSector(t_list *sectsAtravesados, double *rotationDelay , unsigned int sectorObjetivo){
	PisSec *pistaSector;

	if( sectActual == sectorObjetivo ){//Caimos justo!!,lo agregamos a sectsAtravesados y lo leemos

		if(logActivado){
			pistaSector = malloc(sizeof(PisSec));
			pistaSector -> pista = pistaActual;
			pistaSector -> sector = sectActual;
			collection_list_add(sectsAtravesados, pistaSector);
			*rotationDelay = *rotationDelay + tpoEntreSectores;
		}
		sectActual = sectorObjetivo + 1;
		sectActual = sectActual % sectsxPista; //por si la cuenta da mayor a la cant de sectores
	}
	else{
		sectActual++ ;
		*rotationDelay = *rotationDelay + tpoEntreSectores;

		if( sectActual <= sectorObjetivo ) {
			while( sectActual <= sectorObjetivo ) {//sectores atravesados,cada iteracion representa una movida de un sector a otro(una vez q ya estamos en la pista deseada)
				if(logActivado){
					pistaSector = malloc(sizeof(PisSec));
					pistaSector -> pista = pistaActual;
					pistaSector -> sector = sectActual;
					collection_list_add(sectsAtravesados, pistaSector);
					*rotationDelay = *rotationDelay + tpoEntreSectores;
				}
				sectActual++;
			}
			sectActual = sectActual % sectsxPista; //por si la cuenta da mayor a la cant de sectores
		}
		else{
			while( sectActual < sectsxPista ){//sectores atravesados,cada iteracion representa una movida de un sector a otro(una vez q ya estamos en la pista deseada)
				if(logActivado){
					pistaSector = malloc(sizeof(PisSec));
					pistaSector -> pista = pistaActual;
					pistaSector -> sector = sectActual;
					collection_list_add(sectsAtravesados, pistaSector);
					*rotationDelay = *rotationDelay + tpoEntreSectores;
				}
				sectActual++;
			}
			sectActual=0;
			while( sectActual <= sectorObjetivo ){
				if(logActivado){
					pistaSector = malloc(sizeof(PisSec));
					pistaSector -> pista = pistaActual;
					pistaSector -> sector = sectActual;
					collection_list_add(sectsAtravesados, pistaSector);
					*rotationDelay=*rotationDelay+tpoEntreSectores;
				}
				sectActual++;
			}
			sectActual = sectActual%sectsxPista;//Lee el sector y queda parado en el siguiente
		}

	}
}

int insertarSegunAlgoritmo(uint32_t pista){
	lPeticiones *pedido = NULL;
	uint32_t pistaAnt = pistaActual;
	t_list *lPasiva;
	int i=0;


	switch (algoritmo){
	case SCAN:

		if (lped->elements_count == 0){
			return 0;
		}

		if (sentido == ASCENDENTE){

			if(pista >= pistaActual){
				//caso N1 solo un tramo ascendente
				for(i = 0 ; i < lped -> elements_count; i++){
					//pedido = malloc(sizeof (lPeticiones));
					pedido = collection_list_get(lped, i);
					if((pedido->pista > pista) || ((pistaAnt < pista) && (pedido->pista < pista))){
						return i - 1;
					}
					pistaAnt = pedido->pista;
				}
				//if(i == lped -> elements_count) return i - 1;
			}
			else {
				//caso N2 un tramo ascendente rebota y descience
				for(i = 0 ; i < lped -> elements_count; i++){
					//pedido = malloc(sizeof (lPeticiones));
					pedido = collection_list_get(lped, i);

					if(pedido -> pista < pista){
						return i - 1;
					}

				}
				//if(i == lped -> elements_count) return i - 1;
			}
		}
		else {
			//sentido descendente
			if(pista <= pistaActual){
				//caso N3 solo desciende
				for(i = 0 ; i < lped -> elements_count; i++){
					//pedido = malloc(sizeof (lPeticiones));
					pedido = collection_list_get(lped, i);
					if((pedido->pista < pista) || ((pistaAnt > pista) && (pedido->pista > pista))){
						return i - 1;
					}
					pistaAnt = pedido->pista;
				}
				//if(i == lped -> elements_count) return i - 1;
			}
			else {
				//caso N4, pista > pistaAnt tiene que rebotar al principio de pista y subir
				for(i = 0 ; i < lped -> elements_count; i++){
					//pedido = malloc(sizeof (lPeticiones));
					pedido = collection_list_get(lped, i);
					if(pedido -> pista > pista){
						return i - 1;
					}

				}
				//if(i == lped -> elements_count) return i - 1;
			}

		}
		break;

	case FSCAN:
		//selecciona lPasiva
		if (lped->tipoLista == PASIVA){
			lPasiva = lped;
		}
		else lPasiva = lped2;


		if (lPasiva->elements_count == 0){
			return 0;
		}

		if (sentido == ASCENDENTE){

			if(pista >= pistaActual){
				//caso N1 solo un tramo ascendente
				for(i = 0 ; i < lPasiva -> elements_count; i++){
					//pedido = malloc(sizeof (lPeticiones));
					pedido = collection_list_get(lPasiva, i);
					if((pedido->pista > pista) || ((pistaAnt < pista) && (pedido->pista < pista))){
						return i - 1;
					}
					pistaAnt = pedido->pista;
				}
				//if(i == lPasiva -> elements_count) return i - 1;
			}
			else {
				//caso N2 un tramo ascendente rebota y descience
				for(i = 0 ; i < lPasiva -> elements_count; i++){
					//pedido = malloc(sizeof (lPeticiones));
					pedido = collection_list_get(lPasiva, i);

					if(pedido -> pista < pista){
						return i - 1;
					}

				}
				//if(i == lPasiva -> elements_count) return i - 1;
			}
		}
		else {
			//sentido descendente
			if(pista <= pistaActual){
				//caso N3 solo desciende
				for(i = 0 ; i < lPasiva -> elements_count; i++){
					//pedido = malloc(sizeof (lPeticiones));
					pedido = collection_list_get(lPasiva, i);
					if((pedido->pista < pista) || ((pistaAnt > pista) && (pedido->pista > pista))){
						return i - 1;
					}
					pistaAnt = pedido->pista;
				}
				//if(i == lPasiva -> elements_count) return i - 1;
			}
			else {
				//caso N4, pista > pistaAnt tiene que rebotar al principio de pista y subir
				for(i = 0 ; i < lPasiva -> elements_count; i++){
					//pedido = malloc(sizeof (lPeticiones));
					pedido = collection_list_get(lPasiva, i);
					if(pedido -> pista > pista){
						return i - 1;
					}

				}

			}

		}
		break;
	default:
		break;
	}

	return -1;
}


	int sentDes(uint32_t pista, uint32_t pAnterior){
		if((pista - pAnterior) <= 0){
			return 1;
		}
	return 0;
}


int sentAs(uint32_t pista, uint32_t pAnterior){
	if((pista - pAnterior) >= 0){
		return 1;
	}
	return 0;
}

int reordenaComoScan(void){
	lPeticiones *pedido;


	int i;
	int pos;

	lped-> tipoLista = ACTIVA;


	for(i=0; i < lped -> elements_count; i++){

		pedido = malloc(sizeof(lPeticiones));
		pedido = collection_list_get(lped2, i);
		pos = insertarSegunAlgoritmo(pedido->pista);
		collection_list_put(lped, pos, pedido);

	}
	return 0;
}

int reordenaComoFScan(void){

	lped2->tipoLista = PASIVA;
	return 0;
}

int handshake(int socket){

	diskData disco;
	char *buf, *msg;
	NIPC_Header *head = malloc(sizeof(NIPC_Header));
	int ret;

	strcpy(disco.id, cilCabSec.id);
	disco.pista = cilCabSec.pistas;
	disco.sector = cilCabSec.sectores;
	buf = malloc(sizeof(diskData));

	memcpy(buf, &disco, sizeof(diskData));
	msg = serializarNipc(HANDSHAKE, sizeof(diskData), buf);
	send(socket, msg, sizeof(NIPC_Header) + sizeof(diskData), MSG_NOSIGNAL);

	ret = recv(socket, head, sizeof(NIPC_Header), MSG_WAITALL);

	if (head->type == PAYOK){
		printf("Se establecio conexion con el PRaid \n");
		return 0;
	}
	else {

		printf("Conexion denegada %d\n", ret);
		return 1;
	}
}



