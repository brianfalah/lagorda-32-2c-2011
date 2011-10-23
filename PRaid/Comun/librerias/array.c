/*
 * Copyright (c) 2011, C-Talks and/or its owners. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact C-Talks owners, Matias Dumrauf or Facundo Viale
 * or visit https://sites.google.com/site/utnfrbactalks/ if you need additional
 * information or have any questions.
 */

/*
 *	@FILE: 	array.c
 *	@AUTOR: Facundo Viale
 *	@VERSION:	1.0	(16/09/2009)
 *					- First Release
 */

#include <stdlib.h>
#include <semaphore.h>

#include "array.h"

/*
 * @NAME: array_create
 * @DESC: Crea una array
 */
t_array *array_create(int size){
	t_array *array = malloc( sizeof(t_array) );
	array->data = malloc( sizeof( void* ) * size );
	array->size = size;
	if( sem_init( &array->semaforo, 0, 1) == -1 ){
		free(array);
		return NULL;
	}
	{
		int cont;
		for(cont=0; cont<size; cont++) array_set(array, cont, NULL, NULL);
	}
	return array;
}

/*
 * @NAME: array_get
 * @DESC: Retorna una posicion del array
 */
void* array_get( t_array *array, int index ){
	void* data = NULL;

	sem_wait( &array->semaforo );
	if( (array_size(array) > index) && (index >= 0) ){
		data = array->data[index];
	}
	sem_post( &array->semaforo );
	return data;
}

/*
 * @NAME: array_switch
 * @DESC: Coloca un valor en una de la posiciones del array retornando el valor anterior
 */
void *array_switch( t_array *array, int index, void *data ){
	void *old_data = NULL;
	sem_wait( &array->semaforo );
	if( (array_size(array) > index ) && (index >= 0) ){
		old_data = array->data[index];
		array->data[index] = data;
	}
	sem_post( &array->semaforo );
	return old_data;
}

/*
 * @NAME: array_set
 * @DESC: Coloca un valor en una de la posiciones del array destruyendo el valor anterior
 */
void array_set( t_array *array, int index, void *data, void (*data_destroyer)(void*)){
	void *old_data = array_switch( array, index, data );
	if( (old_data != NULL) && (data_destroyer != NULL) ){
		data_destroyer(old_data);
	}
}

/*
 * @NAME: array_find
 * @DESC: Retorna el primer valor encontrado, el cual haga que el closure devuelva != 0
 */
void* array_find( t_array *array, int (*closure)(void*) ){
	void *data = NULL;
	int cont;
	sem_wait( &array->semaforo );
	for(cont=0; cont<array_size(array); cont++){
		if( closure( array->data[cont] ) ){
			data = array->data[cont];
			break;
		}
	}
	sem_post( &array->semaforo );
	return data;
}

/*
 * @NAME: array_iterator
 * @DESC: Itera el array llamando al closure por cada elemento.
 */
void array_iterator( t_array* array, void (*closure)(void*) ){
	int cont;
	sem_wait( &array->semaforo );
	for(cont=0; cont<array_size(array); cont++){
		closure( array->data[cont] );
	}
	sem_post( &array->semaforo );
}


/*
 * @NAME: array_remove
 * @DESC: Setea NULL una posicion determiana del array retornando el valor anterior.
 */
void *array_remove( t_array *array, int index ){
	return array_switch(array, index, NULL);
}

/*
 * @NAME: array_removeAndDestroy
 * @DESC: Destruye el valor de una determinada posicion del array y setea NULL esa posicion.
 */
void array_removeAndDestroy( t_array *array, int index, void (*data_destroyer)(void*) ){
	void *data = array_remove(array, index);
	data_destroyer(data);
}

/*
 * @NAME: array_removeByClosure
 * @DESC: Destruye todas las posicion del array hagan que el closure devuelva != 0
 */
void array_removeByClosure( t_array *array, int (*closure)(void*), void (*data_destroyer)(void*) ){

}

/*
 * @NAME: array_size
 * @DESC: Retorna el tamaño del array
 */
int array_size( t_array *array ){
	return array->size;
}

/*
 * @NAME: array_clean
 * @DESC: Destruye todas las posiciones del array y las setea a NULL.
 */
void array_clean( t_array *array, void (*data_destroyer)(void*) ){
	int cont;
	sem_wait( &array->semaforo );
	for(cont=0; cont<array_size(array); cont++){
		data_destroyer(array->data[cont]);
		array->data[cont] = NULL;
	}
	sem_post( &array->semaforo );
}

/*
 * @NAME: array_destroy
 * @DESC: Destruye el array, reciviendo como argumento el metodo encargado de destruir cada
 * 			posicion de este.
 */
void array_destroy( t_array *array, void (*data_destroyer)(void*) ){
	if( data_destroyer != NULL ){
		array_clean(array, data_destroyer);
	}
	sem_destroy( &array->semaforo );
	free(array->data);
	free(array);
}

