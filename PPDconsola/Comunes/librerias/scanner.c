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
 *	@FILE:	scanner.c
 *	@AUTOR: Facundo Viale
 *	@VERSION:	1.5	(9/09/2009)
 *					- Full Revision
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "scanner.h"
#include "array.h"

/*
 * @NAME: scanner_getLine
 * @DESC: Devuelve una linea especifica de un String
 */
char *scanner_getLine(char buff[], int num){
	char* line;
	int index=0, count=0, len=0;

	/* posiciona en la linea indicada */
	for(; buff[index] != '\0' && count != num; index++){
		if( buff[index] == '\n'){
			count++;
		}
	}

	/* busca la longuitud de esa linea */
	for(; buff[index+len] != '\0' &&  buff[index+len] != '\n'; len++);

	line = malloc(len+1);
	memcpy(line, &buff[index], len);
	line[len] = '\0';

	return line;
}

/*
 * @NAME: scanner_iterateLines
 * @DESC: Itera un String por cada una de sus lineas
 * @NOTA: NO hay que hacer free de estas lineas y si hay que usarlas
 * 		  hay que hacer un copy
 */
void scanner_iterateLines(char buff[], void (*closure)(char*)){
	int index;
	for(index=0; index<scanner_getNumberOfLines(buff); index++){
		char *line = scanner_getLine(buff, index);
		closure(line);
		free(line);
	}
}

char *scanner_findLine(char buff[], int (*closure)(char*)){
	int index;
	for(index=0; index<scanner_getNumberOfLines(buff); index++){
		char *line = scanner_getLine(buff, index);
		if( closure(line) ) return line;
		free(line);
	}
	return NULL;
}

/*
 * @NAME: scanner_getNumberOfLines
 * @DESC: Devuelve el numero de lineas de un String
 */
int scanner_getNumberOfLines(char buff[]){
	int count, index;
	for(index=0, count=1; buff[index] != '\0' ; index++){
		if( buff[index] == '\n'){
			count++;
		}
	}
	return count;
}

/*
 * @NAME: scanner_starWith
 * @DESC: Devuelve un String empieza con determino prefijo
 */
int scanner_starWith( char *str, char *prefix ){
	return strncmp(str, prefix, strlen(prefix)) == 0;
}

/*
 * @NAME: scanner_split1
 * @DESC: Realiza un split sobre un cadena usando un separador, adicionalmente se pueden
 * 		  usar delimitadores para evitar que se haga un split sobre un tramo de cadena.
 * 		  Ej: [12],[Hola, Mundo] o "12","Hola, Mundo" o 12,"Hola, Mundo"
 */
t_array *scanner_split1( char *str, const char sep, char startTokenDelimitor, char endTokenDelimitor){
	t_array *str_split = NULL;
	int cont, arg_pibot, arg_size, arg_end;
	int delimitorBlock = 0;

	for(arg_size=1, cont=0;  str[cont] != '\0'; cont++ ){
		if( str[cont] == startTokenDelimitor && startTokenDelimitor != '\0' && delimitorBlock == 0 ){
			delimitorBlock = 1;
		}else if( str[cont] == endTokenDelimitor && endTokenDelimitor != '\0' && delimitorBlock == 1 ){
			delimitorBlock = 0;
		}else if( str[cont] == sep && delimitorBlock == 0 ){
			arg_size++;
		}
	}

	str_split = array_create(arg_size);

	for(arg_pibot=0, cont=0; cont<array_size(str_split); cont++ ){
		char *substr;

		delimitorBlock = 0;
		for(arg_size=-1, arg_end=0; ; arg_end++ ){
			if( str[arg_end+arg_pibot] == startTokenDelimitor && startTokenDelimitor != '\0' && delimitorBlock == 0 ){
				delimitorBlock = 1;
				arg_pibot++;
			}else if( str[arg_end+arg_pibot] == endTokenDelimitor && endTokenDelimitor != '\0' && delimitorBlock == 1 ){
				delimitorBlock = 0;
				arg_size = arg_end;
			}else if( (str[arg_end+arg_pibot] == sep && delimitorBlock == 0) || str[arg_end+arg_pibot] == '\0' ){
				if( arg_size == -1 ) arg_size = arg_end;
				break;
			}
		}

		substr = malloc( arg_size + 1 );
		strncpy(substr, &str[arg_pibot], arg_size);
		substr[arg_size] = '\0';

		array_set(str_split, cont, substr, NULL);

		arg_pibot = arg_pibot + arg_end + 1;
	}

	return str_split;
}

/*
 * @NAME: scanner_split2
 * @DESC: Realiza un split sobre un cadena usando un separador
 */
t_array *scanner_split2( char *str, const char sep ){
	return scanner_split1( str, sep, '\0', '\0');
}

/*
 * @NAME: scanner_trim
 * @DESC: Genera un nuevo String a partir de otro
 * 		  pero eliminando los espacio de adelante y
 * 		  de atras del original
 * @NOTA: No olvidar de hacer free al nuevo String
 */
char *scanner_trim( char str[] ){
	char *newStr;
	int beginIndex,endIndex,index;
	int newStrLen,strLen=strlen(str);

	if( strLen <= 0 ) return "";

	for(index=0 ;isspace(str[index]) || str[index] == '\n'; index++);		beginIndex=index;
	for(index=strLen-1 ;isspace(str[index]) || str[index] == '\n'; index--);	endIndex=index;

	/*	Se le suma +1 porque endIndex y beginIndex
		son indices y el len es por cantidad de elementos	*/
	newStrLen = endIndex - beginIndex + 1;

	/*	Se le suma +1 por el \0	*/
	newStr = malloc( newStrLen + 1);

	memcpy(newStr, &str[beginIndex], newStrLen);

	newStr[newStrLen]='\0';

	return newStr;
}

/*
 * @NAME: scanner_trimInBuffer
 * @DESC: Genera un nuevo String a partir de otro
 * 		  pero eliminando los espacio de adelante y
 * 		  de atras del original
 */
char *scanner_trimInBuffer( char str[], char trim_str[], int max_size ){
	int beginIndex,endIndex,index;
	int newStrLen,strLen=strlen(str);

	if( strLen <= 0 ) return "";

	for(index=0 ;isspace(str[index]) || str[index] == '\n' ;index++);		beginIndex=index;
	for(index=strLen-1 ;isspace(str[index]) || str[index] == '\n' ;index--);endIndex=index;

	/*	Se le suma +1 porque endIndex y beginIndex
		son indices y el len es por cantidad de elementos	*/
	newStrLen = endIndex - beginIndex + 1;

	if( newStrLen >= max_size) return NULL;

	memcpy(trim_str, &str[beginIndex], newStrLen);

	trim_str[newStrLen]='\0';

	return trim_str;
}

char *scanner_getIpFromAddress( char str_adress[], char str_ip[] ){
	int index;
	for(index=0; str_adress[index] != ':' && index < strlen(str_adress)  ; index++){
		str_ip[index] = str_adress[index];
	}
	str_ip[index] = '\0';
	return str_ip;
}

int	scanner_getPortFromAddress( char str_adress[] ){
	char buffer[8] = "0";
	int  str_adress_len = strlen(str_adress);
	int  index;

	for(index=0; str_adress[index] != ':' && index < str_adress_len  ; index++);

	if( index < str_adress_len ){
		int bindex;
		for(bindex=0; (bindex + index  + 1) < str_adress_len && bindex < 8 ; bindex++){
			buffer[bindex] = str_adress[bindex + index  + 1];
		}
	}
	return atoi(buffer);
}

char *scanner_getPortFromAddress2( char str_adress[], char str_port[] ){
	int str_adress_len = strlen(str_adress);
	int index, bindex  = 0;

	for(index=0; str_adress[index] != ':' && index < str_adress_len  ; index++);

	if( index < str_adress_len ){
		for(bindex=0; (bindex + index  + 1) < str_adress_len && bindex < 8 ; bindex++){
			str_port[bindex] = str_adress[bindex + index  + 1];
		}
	}
	str_port[bindex] = '\0';
	return str_port;
}

