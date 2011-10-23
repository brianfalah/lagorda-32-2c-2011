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
 *	@FILE: commands.c
 *	@AUTOR: Facundo Viale
 *	@VERSION:	1.1	(25/09/2009)
 *					- First Release
 */

#include <stdlib.h>
#include <string.h>

#include "collections/list.h"
#include "scanner.h"

#include "commands.h"

t_commands *commands_create(char startArgsToken, char endArgsToken, char separatorArgsToken){
	return commands_createWithConext(NULL, startArgsToken, endArgsToken, separatorArgsToken);
}

t_commands *commands_createWithConext(void *context, char startArgsToken, char endArgsToken, char separatorArgsToken){
	t_commands * commandsmanager = malloc( sizeof(t_commands) );
	commandsmanager->commands = collection_list_create();
	commandsmanager->globalcontext = context;
	commandsmanager->startArgsToken = startArgsToken;
	commandsmanager->endArgsToken = endArgsToken;
	commandsmanager->startArgToken = '\0';
	commandsmanager->endArgToken = '\0';
	commandsmanager->separatorArgsToken = separatorArgsToken;
	return commandsmanager;
}

void commands_setStartEndArgTokens(t_commands *commandsmanager, char startArgToken, char endArgToken){
	commandsmanager->startArgToken = startArgToken;
	commandsmanager->endArgToken = endArgToken;
}

void commands_setStartEndArgsTokens(t_commands *commandsmanager, char startArgsToken, char endArgsToken){
	commandsmanager->startArgsToken = startArgsToken;
	commandsmanager->endArgsToken = endArgsToken;
}

char commands_getStartArgsTokens(t_commands *commandsmanager){
	return commandsmanager->startArgsToken;
}

char commands_getEndArgsTokens(t_commands *commandsmanager){
	return commandsmanager->endArgsToken;
}

void commands_setSeparatorArgsToken(t_commands *commandsmanager, char separatorArgsToken){
	commandsmanager->separatorArgsToken = separatorArgsToken;
}

char commands_getSeparatorArgsToken(t_commands *commandsmanager){
	return commandsmanager->separatorArgsToken;
}

void commands_add(t_commands *commandsmanager, const char *strcommand, void (*command_handler)(void*, t_array*)){
	commands_addWithContext(commandsmanager, NULL, strcommand, command_handler);
}

void commands_addWithContext(t_commands *commandsmanager, void *context, const char *strcommand, void (*command_handler)(void*, t_array*)){
	t_command *command = malloc( sizeof(t_command) );
	command->text = strcommand;
	command->context = context;
	command->command_handler = command_handler;
	collection_list_add(commandsmanager->commands, command);
}

int commands_parser(t_commands *commandsmanager, char *strcommand){
	t_command *command = NULL;
	t_array *args = NULL;
	int strcmdlen = strlen(strcommand);

	int find_command(t_command *cmd){ return scanner_starWith(strcommand, cmd->text); }
	if( (command = collection_list_find(commandsmanager->commands, (void*)&find_command)) != NULL ){
		if( strcommand[ strlen(command->text) ] == commandsmanager->startArgsToken  &&
				(commandsmanager->endArgsToken == '\0' ||  strcommand[ strcmdlen - 1 ] == commandsmanager->endArgsToken ) ){
			char subargsbuffer[1024];

			int tokendiff = 2;
			if( commandsmanager->endArgsToken == '\0') tokendiff = 1;

			memcpy(subargsbuffer, &strcommand[ strlen(command->text) + 1 ], strcmdlen - strlen(command->text) - tokendiff );
			subargsbuffer[strcmdlen - strlen(command->text) - tokendiff] = '\0';
			if( strlen(subargsbuffer) > 0 ){
				args = scanner_split1(subargsbuffer, commandsmanager->separatorArgsToken, commandsmanager->startArgToken, commandsmanager->endArgToken );
			}
		}

		if(command->context != NULL){
			command->command_handler(command->context, args);
		}else{
			command->command_handler(commandsmanager->globalcontext, args);
		}
		return 1;
	}

	return 0;
}

void commands_destroy(t_commands *commandsmanager){
	collection_list_destroy(commandsmanager->commands, &free);
	free(commandsmanager);
}


