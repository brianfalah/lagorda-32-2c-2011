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
 *	@FILE: 	serializer.c
 *	@AUTOR: Facundo Viale
 *	@VERSION:	1.0	(11/05/2011)
 *					- Initial Implementation
 */

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "collections/list.h"

#include "serializer.h"

static int serializer_getFieldSize(e_sfield_type type);
static t_serializer_field	*serializer_makeFieldInt32(t_serializer*, va_list args);
static t_serializer_field	*serializer_makeFieldInt64(t_serializer*, va_list args);
static t_serializer_field	*serializer_makeFieldChar(t_serializer*, va_list args);
static t_serializer_field	*serializer_makeFieldLong32(t_serializer*, va_list args);
static t_serializer_field	*serializer_makeFieldLong64(t_serializer*, va_list args);
static t_serializer_field	*serializer_makeFieldFloat(t_serializer*, va_list args);
static t_serializer_field	*serializer_makeFieldDouble(t_serializer*, va_list args);
static t_serializer_field	*serializer_makeFieldArray(t_serializer*, va_list args);
static t_serializer_field	*serializer_makeFieldCharPointer(t_serializer*, va_list args);
static t_serializer_field	*serializer_makeFieldVoidPointer(t_serializer*, va_list args);


t_serializer	*serializer_create(char *name, ... ){
	t_serializer *self = malloc( sizeof(t_serializer) );
	va_list args;
	int lookForArgs = 1;

	self->name = name;
	self->fields = collection_list_create();

	va_start(args, name);

	while( lookForArgs ){

		switch( va_arg(args, e_sfield_type) ){

			case SFIELD_TYPE_INT32:
				collection_list_add( self->fields, serializer_makeFieldInt32(self, args ) );
				break;
			case SFIELD_TYPE_INT64:
				collection_list_add( self->fields, serializer_makeFieldInt64(self, args ) );
				break;
			case SFIELD_TYPE_CHAR:
				collection_list_add( self->fields, serializer_makeFieldChar(self, args ) );
				break;
			case SFIELD_TYPE_LONG32:
				collection_list_add( self->fields, serializer_makeFieldLong32(self, args ) );
				break;
			case SFIELD_TYPE_LONG64:
				collection_list_add( self->fields, serializer_makeFieldLong64(self, args ) );
				break;
			case SFIELD_TYPE_FLOAT:
				collection_list_add( self->fields,serializer_makeFieldFloat(self, args ) );
				break;
			case SFIELD_TYPE_DOUBLE:
				collection_list_add( self->fields, serializer_makeFieldDouble(self, args ) );
				break;
			case SFIELD_TYPE_ARRAY:
				collection_list_add( self->fields, serializer_makeFieldArray(self, args ) );
				break;
			case SFIELD_TYPE_CHAR_POINTER:
				collection_list_add( self->fields, serializer_makeFieldCharPointer(self, args ) );
				break;
			case SFIELD_TYPE_VOID_POINTE:
				collection_list_add( self->fields, serializer_makeFieldVoidPointer(self, args ) );
				break;
			case 0:
				lookForArgs = 0;
				break;
		}
	}

	va_end(args);

	return self;
}

static int serializer_getFieldSize(e_sfield_type type){
	return SFIELD_INFO[type][SFIELD_INFO_SIZE];
}

static t_serializer_field	*serializer_makeFieldInt32(t_serializer *self, va_list args){
	t_serializer_field *field = malloc( sizeof(t_serializer_field) );

	field->type = SFIELD_TYPE_INT32;
	field->firstTypeSize = serializer_getFieldSize(SFIELD_TYPE_INT32);

	self->mainStructureSize += field->firstTypeSize;

	return field;
}

static t_serializer_field	*serializer_makeFieldInt64(t_serializer *self, va_list args){
	t_serializer_field *field = malloc( sizeof(t_serializer_field) );

	field->type = SFIELD_TYPE_INT64;
	field->firstTypeSize = serializer_getFieldSize(SFIELD_TYPE_INT64);

	self->mainStructureSize += field->firstTypeSize;

	return field;
}

static t_serializer_field	*serializer_makeFieldChar(t_serializer *self, va_list args){
	t_serializer_field *field = malloc( sizeof(t_serializer_field) );

	field->type = SFIELD_TYPE_CHAR;
	field->firstTypeSize = serializer_getFieldSize(SFIELD_TYPE_CHAR);

	self->mainStructureSize += field->firstTypeSize;

	return field;
}

static t_serializer_field	*serializer_makeFieldLong32(t_serializer *self, va_list args){
	t_serializer_field *field = malloc( sizeof(t_serializer_field) );

	field->type = SFIELD_TYPE_LONG32;
	field->firstTypeSize = serializer_getFieldSize(SFIELD_TYPE_LONG32);

	self->mainStructureSize += field->firstTypeSize;

	return field;
}

static t_serializer_field	*serializer_makeFieldLong64(t_serializer *self, va_list args){
	t_serializer_field *field = malloc( sizeof(t_serializer_field) );

	field->type = SFIELD_TYPE_LONG64;
	field->firstTypeSize = serializer_getFieldSize(SFIELD_TYPE_LONG64);

	self->mainStructureSize += field->firstTypeSize;

	return field;
}

static t_serializer_field	*serializer_makeFieldFloat(t_serializer *self, va_list args){
	t_serializer_field *field = malloc( sizeof(t_serializer_field) );

	field->type = SFIELD_TYPE_FLOAT;
	field->firstTypeSize = serializer_getFieldSize(SFIELD_TYPE_FLOAT);

	self->mainStructureSize += field->firstTypeSize;

	return field;
}

static t_serializer_field	*serializer_makeFieldDouble(t_serializer *self, va_list args){
	t_serializer_field *field = malloc( sizeof(t_serializer_field) );

	field->type = SFIELD_TYPE_DOUBLE;
	field->firstTypeSize = serializer_getFieldSize(SFIELD_TYPE_DOUBLE);

	self->mainStructureSize += field->firstTypeSize;

	return field;
}

static t_serializer_field	*serializer_makeFieldArray(t_serializer *self, va_list args){
	t_serializer_field *field = malloc( sizeof(t_serializer_field) );

	field->type = SFIELD_TYPE_ARRAY;
	field->firstTypeSize = va_arg(args, int);
	field->secondType = va_arg(args, e_sfield_type);
	field->secondTypeSize= serializer_getFieldSize(field->secondType);

	self->mainStructureSize += field->firstTypeSize * field->secondTypeSize;

	return field;
}

static t_serializer_field	*serializer_makeFieldCharPointer(t_serializer *self, va_list args){
	t_serializer_field *field = malloc( sizeof(t_serializer_field) );

	field->type = SFIELD_TYPE_CHAR_POINTER;
	field->firstTypeSize = serializer_getFieldSize(SFIELD_TYPE_CHAR_POINTER);

	self->mainStructureSize += field->firstTypeSize;

	return field;
}

static t_serializer_field	*serializer_makeFieldVoidPointer(t_serializer *self, va_list args){
	t_serializer_field *field = malloc( sizeof(t_serializer_field) );

	field->type = SFIELD_TYPE_VOID_POINTE;
	field->firstTypeSize = serializer_getFieldSize(SFIELD_TYPE_VOID_POINTE);
	field->serializer = va_arg(args, void*);

	self->mainStructureSize += field->firstTypeSize;

	return field;
}


t_stream		*serializer_build(t_serializer *self, void *obj){
	char tmp_buff[MAX_SERIALIZER_BUFFER];
	t_stream *stream = malloc( sizeof(t_stream) );
	int obj_offset = 0, stream_offset = 0, tmp_size = 0;

	collection_list_iterator(self->fields, (void*)({
			void $(t_serializer_field *field){
				char *bytes_obj = obj;

				if( field->type == SFIELD_TYPE_ARRAY ){

					tmp_size = field->firstTypeSize * field->secondTypeSize;
					memcpy(tmp_buff + stream_offset, bytes_obj + obj_offset, tmp_size);

					obj_offset += tmp_size;
					stream_offset += tmp_size;

				} else if( field->type == SFIELD_TYPE_CHAR_POINTER ){

					char **aux_str = (void*)(bytes_obj + obj_offset);
					int aux_str_len = strlen(*aux_str) + 1;

					memcpy(tmp_buff + stream_offset, *aux_str, aux_str_len);

					obj_offset += sizeof(char*);
					stream_offset += aux_str_len;

				} else if( field->type == SFIELD_TYPE_VOID_POINTE ){

					t_stream *aux_strem = serializer_build(field->serializer, (void*)(bytes_obj + obj_offset));

					memcpy(tmp_buff + stream_offset, aux_strem->data, aux_strem->lenght);

					obj_offset += sizeof(void*);
					stream_offset += aux_strem->lenght;

					free(aux_strem);

				} else {

					tmp_size = field->firstTypeSize;
					memcpy(tmp_buff + stream_offset, bytes_obj + obj_offset, tmp_size);

					obj_offset += tmp_size;
					stream_offset += tmp_size;
				}

			}
		$;}));

	stream->lenght = stream_offset;

	if( stream_offset > 0 ){
		stream->data = malloc( stream_offset );
		memcpy(stream->data, tmp_buff, stream_offset);
	}

	return stream;
}

void			*serializer_rebuild(t_serializer *self, t_stream *stream){
	void *obj = malloc( self->mainStructureSize );
	int obj_offset = 0, stream_offset = 0, tmp_size = 0;

	collection_list_iterator(self->fields, (void*)({
			void $(t_serializer_field *field){

				if( field->type == SFIELD_TYPE_ARRAY ){

					tmp_size = field->firstTypeSize * field->secondTypeSize;
					memcpy(obj + obj_offset, stream->data + stream_offset, tmp_size);

					obj_offset += tmp_size;
					stream_offset += tmp_size;

				} else if( field->type == SFIELD_TYPE_CHAR_POINTER ){

					char *aux_str = NULL;
					int aux_str_len = 1;

					for(; (stream->data + stream_offset)[aux_str_len-1] != '\0' ;aux_str_len++);

					aux_str = malloc( aux_str_len );

					memcpy(aux_str, stream->data + stream_offset, aux_str_len);

					memcpy(obj + obj_offset, &aux_str, sizeof(char*));

					obj_offset += sizeof(char*);
					stream_offset += aux_str_len;

				} else if( field->type == SFIELD_TYPE_VOID_POINTE ){

					t_stream aux_strem;
					void *aux_obj = NULL;

					aux_strem.lenght = -1;
					aux_strem.data = stream->data + stream_offset;

					aux_obj = serializer_rebuild(field->serializer, &aux_strem);

					memcpy(obj + obj_offset, aux_obj, sizeof(void*));

					obj_offset += sizeof(void*);
					stream_offset += aux_strem.lenght;

				} else {

					tmp_size = field->firstTypeSize;
					memcpy(obj + obj_offset, stream->data + stream_offset, tmp_size);

					obj_offset += tmp_size;
					stream_offset += tmp_size;
				}

			}
		$;}));

	if( stream->lenght == -1 ){
		stream->lenght = stream_offset;
	}

	return obj;
}

void			 serializer_delete(t_serializer *self, t_stream **stream){
	free( (*stream)->data );
	free( (*stream) );
	stream = NULL;
}

void			 serializer_destroy(t_serializer *self){
	collection_list_destroy(self->fields, free);
	free(self);
}
