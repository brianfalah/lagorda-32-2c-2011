
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include "listaPPD.h"
#include "Comunes/ProtocoloDeMensajes.h"



extern lped;
/*
lPeticiones *lAgregar(lPeticiones **p, int desc,uint32_t sectorLogico,uint32_t pista, uint32_t sectorEnPista, char* buf, int buflen) {
    lPeticiones *n = (lPeticiones *)malloc(sizeof(lPeticiones));
    if (n == NULL)
        return NULL;
    n->next = *p;
    *p = n;



    n->fd = desc;

    n->sectorLogico = sectorLogico;
    n->pista = pista;
    n->sectorEnPista = sectorEnPista;
    n->data = malloc(buflen);
    n->data = buf;




    return n;
}

void lSacar(lPeticiones **p) {
    if (*p != NULL) {
        lPeticiones *n = *p;
        *p = (*p)->next;
        free(n);
    }
}

void lSacarAll(lPeticiones **p){
	while (*p != NULL) {
		lSacar(&*p);
	}
	return;
}

lPeticiones **lBuscarPorFd(lPeticiones **n, int desc) {
		while (*n != NULL) {
			if ((*n)->fd == desc) {
				return n;
			}
			n = &(*n)->next;
		}
		return NULL;
}

lPeticiones **lBuscarPorSector(lPeticiones **n, uint32_t sect) {
		while (*n != NULL) {
			if ((*n)->sectorLogico == sect) {
				return n;
			}
			n = &(*n)->next;
		}
		return NULL;
}

void lImprimir(lPeticiones *n) {
    if (n == NULL) {
        printf("lista esta vacía\n");
    }
    while (n != NULL) {
        printf("print Apuntado:%p Apunta a:%p  Descriptor:%d Sector:%d Data:%s \n", n, n->next, n->fd, n->sectorLogico,n->data);
        n = n->next;
    }
}


int pruebaLista(lPeticiones **p){

	testingCargarPedidos(&*p);
	lImprimir(*p);
	lSacar(lBuscarPorSector(&*p, 7));
	lImprimir(*p);

	return EXIT_SUCCESS;
}
*/
