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
 *	@FILE: 	serializer.h
 *	@AUTOR: Facundo Viale
 *	@VERSION:	1.0	(11/05/2011)
 *					- Initial Implementation
 */

#ifndef OLD_SERIALIZER_H_
#define OLD_SERIALIZER_H_

	#define _XOPEN_SOURCE 500

	#include <stdlib.h>
	#include <stdarg.h>
	#include "collections/list.h"

	#define MAX_SERIALIZER_BUFFER 4096
	#define SERIALIZER_END_ARGS NULL

	typedef enum{
		SFIELD_TYPE_END 			= 0x0,
		SFIELD_TYPE_INT32 			= 0x01,
		SFIELD_TYPE_INT64 			= 0x02,
		SFIELD_TYPE_CHAR 			= 0x03,
		SFIELD_TYPE_LONG32 			= 0x04,
		SFIELD_TYPE_LONG64 			= 0x05,
		SFIELD_TYPE_FLOAT 			= 0x06,
		SFIELD_TYPE_DOUBLE 			= 0x07,
		SFIELD_TYPE_ARRAY 			= 0x08,
		SFIELD_TYPE_CHAR_POINTER 	= 0x09,
		SFIELD_TYPE_VOID_POINTE 	= 0x0A
	}e_sfield_type;

	typedef enum{
		SFIELD_INFO_SIZE 			= 0x0,
	}e_sfield_info;


	/* The First column is the length of the field */
	static const int SFIELD_INFO [][1] = {
									{0},
									{4},		/* SFIELD_TYPE_INT32 */
									{8},		/* SFIELD_TYPE_INT64 */
									{1},		/* SFIELD_TYPE_CHAR */
									{4},		/* SFIELD_TYPE_LONG32 */
									{8},		/* SFIELD_TYPE_LONG64 */
									{4},		/* SFIELD_TYPE_FLOAT */
									{8},		/* SFIELD_TYPE_DOUBLE */
									{0},		/* SFIELD_TYPE_ARRAY */
									{4},		/* SFIELD_TYPE_CHAR_POINTER */
									{4}			/* SFIELD_TYPE_VOID_POINTE */
	};


	typedef struct{
		e_sfield_type type;
		unsigned int firstTypeSize;
		e_sfield_type secondType;
		unsigned int secondTypeSize;
		void *serializer;
	}t_serializer_field;

	typedef struct{
		char *name;
		t_list *fields;
		int mainStructureSize;
	}t_serializer;

	typedef struct{
		int lenght;
		char *data;
	}t_stream;

	t_serializer	*serializer_create(char *name, ...);
	t_stream		*serializer_build(t_serializer*, void *obj);
	void			*serializer_rebuild(t_serializer*, t_stream *strem);
	void			 serializer_delete(t_serializer*, t_stream**);
	void			 serializer_destroy(t_serializer*);

#endif /*OLD_SERIALIZER_H_*/
