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
 *	@FILE: config_loader.c
 *	@AUTOR: Facundo Viale
 *	@VERSION:	1.0	(10/09/2008)
 *					- First Release
 */

#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "config_loader.h"
#include "fileio.h"
#include "scanner.h"


char *config_loader_open(char* path){
	return fileio_openfile(path);
}

char *config_loader_getString(char *configBuff, char *key){
	char *retValue = NULL, *line;
	int keysize = strlen(key);

	int closure(char *line){ return scanner_starWith(line, key) == 1; }

	line = scanner_findLine(configBuff, &closure);

	if( line != NULL ){
		int tmpIndex = keysize;
		for(; line[tmpIndex]!='\n' && line[tmpIndex]!='\0'; tmpIndex++);

		retValue = malloc( tmpIndex + 1 );
		memcpy(retValue, &line[keysize], tmpIndex);
		retValue[tmpIndex]='\0';

		free(line);
	}

	return retValue;
}

int config_loader_getInt(char *configBuff, char *key){
	char *data = config_loader_getString(configBuff, key);

	if( data != NULL){
		int x = atoi(data);
		free(data);
		return x;
	}
	return 0;
}

double config_loader_getDouble(char *configBuff, char *key){
	char *data = config_loader_getString(configBuff, key);

	if( data != NULL){
		double x = atof(data);
		free(data);
		return x;
	}
	return 0;
}
