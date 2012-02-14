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
 *	@FILE: 	array.h
 *	@AUTOR: Facundo Viale
 *	@VERSION:	1.0	(16/09/2009)
 *					- First Release
 */

#ifndef OLD_ARRAY_H_
#define OLD_ARRAY_H_

	#define _XOPEN_SOURCE 500

	#include <semaphore.h>

	typedef struct{
		void **data;
		int size;
		sem_t semaforo;
	}t_array;

	t_array    *array_create(int size);
	void       *array_get( t_array *list, int num );
	void	   *array_switch( t_array* list, int num, void* data );
	void 		array_set( t_array* list, int num, void* data, void (*data_destroyer)(void*));
	void*		array_find( t_array *list, int (*closure)(void*) );
	void		array_iterator( t_array *list, void (*closure)(void*) );
	void   	   *array_remove( t_array *list, int num );
	void        array_removeAndDestroy( t_array *list, int num, void (*data_destroyer)(void*) );
	void    	array_removeByClosure( t_array *list, int (*closure)(void*), void (*data_destroyer)(void*) );
	int     	array_size( t_array *list );
	void 		array_clean( t_array *list, void (*data_destroyer)(void*) );
	void 		array_destroy( t_array *list, void (*data_destroyer)(void*) );

#endif /*OLD_ARRAY_H_*/

