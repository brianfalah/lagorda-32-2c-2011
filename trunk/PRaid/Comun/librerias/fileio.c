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
 *	@FILE: fileio.c
 *	@AUTOR: Facundo Viale
 *	@VERSION:	1.0	(10/09/2008)
 *					- First Release
 */



#include "fileio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * @DESC = Abre un archivo y lo devuelve en forma de cadena de caracteres
 * @PARAM = Path hacia el archivo a abrir
 * @RET = NULL en dac
 *
 */
char* fileio_openfile(char* path){
	FILE* file;
	int file_size = fileio_getfilesize(path);
	char* buff = malloc( file_size + 1 );
	int index;

	if( ( file=fopen(path,"r") ) == NULL )return NULL;

	for(index=0; index < file_size ;index++){
		buff[index]=fgetc(file);
	}
	buff[index]='\0';
	fclose(file);
	return buff;

}

/*
 *
 * @DESC = Abre un archivo, lo lee por partes y lo devuelve en forma de cadena de caracteres
 * @PARAM = Path hacia el archivo a abrir
 * @PARAM = Tamaño maximo del buffer de lectura
 * @RET= Cantidad de caracteres leidos. -1 si la ruta es invalida
 *
 */

int fileio_openfileinbuff(char* path,char* buff ,int bufflenght){
	FILE* file;
	int character,index;

	if( ( file=fopen(path,"r") ) ==NULL ) return -1;

	for(index=0; ( character=fgetc(file) ) != EOF &&  index < bufflenght - 1;index++){
		buff[index]=character;
	}
	buff[--index]='\0';
	fclose(file);
	return index;
}


t_file_block* fileio_readfileblock(char* path, int blocksize , int blocknum){
	FILE* f;
	t_file_block* file_block = malloc( sizeof(t_file_block) );
	int size = fileio_getfilesize(path);
	int index;

	if( (size - blocksize * blocknum) > 0){
		file_block->data_size = size - blocksize * blocknum;
		if( file_block->data_size > 0 ){
			if( file_block->data_size > blocksize ){
				file_block->data_size = blocksize;
			}

			if( ( f=fopen(path,"r") ) !=NULL && size > 0 ){
				if( fseek(f,blocksize*blocknum,SEEK_SET) == 0 ){
					file_block->data = malloc( file_block->data_size );
					for( index=0; index < file_block->data_size ;index++){
						file_block->data[index]=fgetc(f);
					}
					fclose(f);
					return file_block;
				}
			}
		}
	}
	free(file_block);
	return NULL;
}

int fileio_appendtofile(char* path, char block[], int blocksize){
	FILE* f;
	int index,aux;

	if( ( f=fopen(path,"ab+") ) !=NULL){
		for(index=0; index < blocksize ;index++){
			aux=fputc(block[index],f);
		}
		fflush(f);
		fclose(f);
		return 1;
	}
	return 0;
}

/*
 * @DESC= Fucion que libera la memoria de un tipo file_block
 */
void fileio_destroyfileblock(t_file_block* file_block){
	free(file_block->data);
	free(file_block);
}

/*
 * @DESC= Borra un archivo del disco a partir de un path
 * @RET= 1 si lo pudo borrar, 0 en cualquier otro caso
 */
int fileio_deletefile(char* path){
	if( remove( path ) == -1 )
		return 0;
	else
		return 1;
}

/*
 * @DESC= Se fija si existe un archivo a partir de un path
 * @RET= 1 en caso que el archivo exista, 0 en cualquier otro caso.
 */
int fileio_fileexist(char* path){
	struct stat st;
	if(stat(path,&st) == 0)	return 1;
	return 0;
}

int fileio_isdirectory(char* path) {
	int status;
	struct stat  *astatBuf = NULL;
	status = stat(path, astatBuf);
	status = (status == 0 && S_ISDIR(astatBuf->st_mode));
	return status;
}

/*
 * @DESC= Devuelve el tamalo de un archivo en bytes a partir de un path
 */
long fileio_getfilesize(char* path){
	struct stat file;
	if( stat(path,&file) ) return _FILE_NOFOUND;
	return file.st_size;
}

/*
 * @DESC= Crea un archivo vacio a partir de un path
 * @RET= 0 si no se pudo crear, 1 en cualquier otro caso
 */
int fileio_createfile(char* path, FILE* file){
	file = fopen(path,"w+");
	if (file == NULL) return 0;
	return 1;
}





