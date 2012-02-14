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
 * utils.c
 *
 *  Created on: 02/08/2009
 *      Author: Matias Dumrauf
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "utils.h"
#include "collections/list.h"


char **utils_convertList_intoDoublePointerList(t_list *list){
	int    i;
	char **wordlist = malloc((collection_list_size(list) + 1) * sizeof(char*));

	for(i = 0; i < collection_list_size(list); i++)
		wordlist[i] = collection_list_get(list, i);

	wordlist[i] = NULL;
	return wordlist;
}

/* --------------------------------------------------------------- */

int utils_numdigitsOfInt(int x){
	int t = x, n = 1;
	while(t>= 10){
		n++;
		t = t/10;
	}
	return n;
}

int utils_numdigitsOfLong(long x){
	long t = x, n = 1;
	while(t>= 10){
		n++;
		t = t/10;
	}
	return n;
}

int utils_numdigitsOfFloat(float x){
	float t = x, n = 1;
	while(t>= 10){
		n++;
		t = t/10;
	}
	return (unsigned int)n;
}

/* --------------------------------------------------------------- */

char *utils_convertIntToString(int x){
	char *cad = malloc(utils_numdigitsOfInt(x) + 1);
	sprintf(cad, "%d", x);
	return cad;
}

char *utils_convertIntToStringBuffer(int x, char buffer[]){
	sprintf(buffer, "%d", x);
	return buffer;
}


char *utils_convertLongToString(long x){
	char *cad = malloc(utils_numdigitsOfLong(x) + 1);
	sprintf(cad, "%ld", x);
	return cad;
}

char *utils_convertFloatToString(float f){
	char *cad = malloc(utils_numdigitsOfFloat(f) + 1);
	sprintf(cad, "%u", (unsigned int)f);
	return cad;
}

/* --------------------------------------------------------------- */

int utils_decodeFileExtension(char *extension){
    if( !strcmp(extension, "html") )
        return HTML_FILE;
    if( !strcmp(extension, "jpeg") || !strcmp(extension, "jpg"))
        return JPG_FILE;
    if( !strcmp(extension, "bmp") )
        return BMP_FILE;
    if( !strcmp(extension, "png") )
        return PNG_FILE;
    if( !strcmp(extension, "gif") )
		return GIF_FILE;
    /* else */
    return ANY_FILE;
}

char *utils_string_clone(char *s){
	char *str = malloc(strlen(s)+1);
	strcpy(str, s);
	return str;
}

/**
 * Returns: son iguales ? 0 : 1
 */
int utils_string_compare(char *s1, char *s2){
	int i;

	if( strlen(s1) != strlen(s2) )
		return 0;

	for(i = 0; i < strlen(s1); i++)
		if( s1[i] != s2[i] )
			return 1;

	return 0;
}

/* Crea una nueva cadena reemplazando por s3 las ocurrencias de s2 en s1, o NULL si no hay ninguna ocurrencia */
char  *utils_string_replace(const char *s1, const char *s2, const char *s3){
	int i, i_word, i_sep, i_cpy, i_cpy_ant, j;
	char *str_replaced;      /* guarda la cadena a devolver */
	char *str_replaced_word; /* guarda la cadena entre la ocurrencia de s2 actual y la anterior */
	char *str_replaced_aux;  /* variable auxiliar para poder reservar espacio para str_replaced + str_replaced_word */
	char *s1_iterated;

	s1_iterated      = malloc(strlen(s1) + 1 + 1);
	strcpy(s1_iterated, s1);
	s1_iterated[strlen(s1)]   = '|';
	s1_iterated[strlen(s1)+1] = '\0';

	str_replaced = NULL;

	i_cpy_ant = 0;

	for(i = 0; s1_iterated[i]; i++){
		if( s1_iterated[i] == s2[0] || s1_iterated[i] == '|' ){
			/* se itera al separador s2 en s1 para saber si hay ocurrencia */
			for(i_sep = i, j = 0; (j < strlen(s2)) && (s1_iterated[i_sep] == s2[j]); i_sep++, j++);

			if( j == strlen(s2) || s1_iterated[i] == '|' ){
				/* se encontro un separador s2 en s1 */
				str_replaced_word = malloc(i - i_cpy_ant + 1);

				/* se guarda la palabra contenida desde la ultima ocurrencia hasta la actual */
				for(i_cpy = 0, i_word = i_cpy_ant; i_cpy < (i - i_cpy_ant); i_cpy++, i_word++)
					str_replaced_word[i_cpy] = s1[i_word];
				str_replaced_word[i_cpy] = '\0';

				if( str_replaced != NULL ){
					str_replaced_aux = malloc(strlen(str_replaced) + 1);
					strcpy(str_replaced_aux, str_replaced);

					free(str_replaced);

					str_replaced = malloc(strlen(str_replaced_aux) + strlen(s3) + strlen(str_replaced_word)  + 1);
					strcpy(str_replaced, str_replaced_aux);
					strcat(str_replaced, s3);
					strcat(str_replaced, str_replaced_word);
					free(str_replaced_aux);
				}else{
					str_replaced = malloc(strlen(str_replaced_word)  + 1);
					strcpy(str_replaced, str_replaced_word);
				}
				free(str_replaced_word);
				i = i_cpy_ant = i_word + strlen(s2);
			}
			j = 0;
		}
	}
	free(s1_iterated);

	if( str_replaced == NULL )
		return NULL;

	return str_replaced;
}

char *utils_string_buildFromTemplate(const char *template, ...){
	char *s;
	char  buffer[DEFAULT_SPRINTF_BUFFER];
	va_list args_list;

	va_start(args_list, template);
	memset(buffer, 0, DEFAULT_SPRINTF_BUFFER);
	vsprintf(buffer, template, args_list);

	s = malloc(strlen(buffer) + 1);
	strcpy(s, buffer);
	return s;
}

/* --------------------------------------------------------------- */

void utils_decodeAnUrlData(char *url, char **ip, int *port){
    int i,j, posPort, posIp, posDash;
    char *portStr, *ipStr;

    posIp = strlen("http://");

    for(posPort = posIp; url[posPort] != ':'; posPort++);

    for(posDash = posPort; url[posDash] != '/'; posDash++);

    ipStr = malloc(posPort - posIp + 1);
    portStr = malloc(posDash - posPort + 1);

    for(j = 0, i = posIp; i < posPort; i++)
    	ipStr[j++] = url[i];

    ipStr[j] = '\0';

    for(j = 0, i = posPort+1; i < posDash; i++)
    	portStr[j++] = url[i];

    portStr[j] = '\0';

    *ip = malloc(strlen(ipStr));
    strcpy(*ip, ipStr);
    *port = atoi(portStr);
    free(portStr);
    free(ipStr);
}

char *utils_getFileName_fromAnUrl(char *url){
	int i, j;
	char *name, *aux;

	aux = malloc(strlen(url)+1);
	strcpy(aux, url);
	aux = aux + strlen("http://");

	for(i = 0; aux[i] != '/'; i++);

	name = malloc(strlen(url) - (i+1) + 1);
	i++;
	for(j = 0; aux[i]; j++)
		name[j] = aux[i++];

	name[j] = '\0';

	return name;
}

int utils_isEven(int num){
	return num % 2 == 0;
}

int utils_isOdd(int num){
	return !utils_isEven(num);
}

int utils_isMemSettedWith(char *mem, int memsize, int byte){
	int index;
	for(index=0; index<memsize; index++){
		if( mem[index] != byte ){
			return 0;
		}
	}
	return 1;
}
