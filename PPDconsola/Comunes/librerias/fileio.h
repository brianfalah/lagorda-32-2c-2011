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
 *	@FILE: fileio.h
 *	@AUTOR: Facundo Viale
 *	@VERSION:	1.0	(10/09/2008)
 *					- First Release
 */

#ifndef OLD_FILEIO_H_
#define OLD_FILEIO_H_
	 #include <stdio.h>

	#define _FILE_NOFOUND -1
	#define _XOPEN_SOURCE 500


	typedef struct{
		char* data;
		int data_size;
		int num_block;
		int block_size;
	} t_file_block;


	char 		 *fileio_openfile(char* path);

	int 		  fileio_openfileinbuff(char* path, char buff[] , int maxlenght);

	t_file_block *fileio_readfileblock(char* path, int blocksize , int blocknum);

	int 		  fileio_appendtofile(char* path, char block[], int blocksize);

	void 		  fileio_destroyfileblock(t_file_block* file_block);

	int 		  fileio_deletefile(char* path);

	int 		  fileio_fileexist(char* path);

	int 		  fileio_isdirectory(char* path);

	long 		  fileio_getfilesize(char *path);

	int           fileio_createfile(char* path, FILE* file);


#endif /*OLD_FILEIO_H_*/
