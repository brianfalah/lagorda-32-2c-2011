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
 *	@FILE: config_loader.h
 *	@AUTOR: Facundo Viale
 *	@VERSION:	1.0	(10/09/2008)
 *					- First Release
 */


#ifndef OLD_CONFIG_LOADER_H
#define OLD_CONFIG_LOADER_H

	#define _XOPEN_SOURCE 500

	char   *config_loader_open(char *path);
	char   *config_loader_getString(char *configBuff, char *key);
	int     config_loader_getInt(char *configBuff,char *key);
	double  config_loader_getDouble(char *configBuff, char *key);

#endif /* OLD_CONFIG_LOADER_H */
