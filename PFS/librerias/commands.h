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
 *	@FILE: commands.h
 *	@AUTOR: Facundo Viale
 *	@VERSION:	1.1	(25/09/2009)
 *					- First Release
 */

#ifndef OLD_COMMANDS_H_
#define OLD_COMMANDS_H_

	#define _XOPEN_SOURCE 500

	#include "array.h"
	#include "collections/list.h"

	typedef struct{
		char *text;
		void *context;
		void (*command_handler)(void*, t_array*);
	}t_command;

	typedef struct{
		t_list *commands;
		void *globalcontext;
		char startArgsToken;
		char endArgsToken;
		char startArgToken;
		char endArgToken;
		char separatorArgsToken;
	}t_commands;



	t_commands *commands_create(char startArgsToken, char endArgsToken, char separatorArgsToken);

	t_commands *commands_createWithConext(void *context, char startArgsToken, char endArgsToken, char separatorArgsToken);

	void commands_setStartEndArgTokens(t_commands *commandsmanager, char startArgToken, char endArgToken);

	void commands_setStartEndArgsTokens(t_commands *commandsmanager, char startArgsToken, char endArgsToken);

	char commands_getStartArgsTokens(t_commands *commandsmanager);

	char commands_getEndArgsTokens(t_commands *commandsmanager);

	void commands_setSeparatorArgsToken(t_commands *commandsmanager, char separatorArgsToken);

	char commands_getSeparatorArgsToken(t_commands *commandsmanager);

	void commands_add(t_commands *commands, const char *strcommand, void (*command_handler)(void*, t_array*));

	void commands_addWithContext(t_commands *commands, void *context,const char *strcommand, void (*command_handler)(void*, t_array*));

	int  commands_parser(t_commands *commands, char *strcommand);

	void commands_destroy(t_commands *commands);


#endif /* OLD_COMMANDS_H_ */
