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
 *	@FILE:	scanner.h
 *	@AUTOR: Facundo Viale
 *	@VERSION:	1.6	(16/09/2009)
 *					- Add Split Function
 *				1.5	(9/09/2009)
 *					- Full Revision
 */
#ifndef OLD_SCANNER_H_
#define OLD_SCANNER_H_

	#define _XOPEN_SOURCE 500

	#include "array.h"

	char 	*scanner_getLine(char buff[], int num);
	int   	 scanner_getNumberOfLines(char buff[]);
	void  	 scanner_iterateLines(char buff[], void (*closure)(char*));
	char 	*scanner_findLine(char buff[], int (*closure)(char*));
	int	  	 scanner_starWith( char *str, char *prefix );

	t_array *scanner_split1( char *str, const char sep, char startTokenDelimitor, char endTokenDelimitor);
	t_array *scanner_split2( char *str, const char sep);

	char 	*scanner_trim( char str[] );
	char	*scanner_trimInBuffer( char str[], char trim_str[], int max_size );

	char	*scanner_getIpFromAddress( char str_adress[], char str_ip[] );
	int		 scanner_getPortFromAddress( char str_adress[] );
	char 	*scanner_getPortFromAddress2( char str_adress[], char str_port[] );

#endif /*OLD_SCANNER_H_*/
