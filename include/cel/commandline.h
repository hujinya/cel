/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com) 
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either version 2 
 * of the License, or (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 */
#ifndef __CEL_COMMANDLINE_H__
#define __CEL_COMMANDLINE_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CEL_ARGVNUM            16

typedef struct _CelArgumentsTableEntry
{
    BOOL must;
    TCHAR *key;
    TCHAR *full_key;
    TCHAR *help;
    TCHAR *argv;
}CelArgumentsTableEntry;

typedef struct _CelCommandTableEntry
{
    const TCHAR *key;
    const TCHAR *full_key;
    const TCHAR *help;
    CelMainFunc proc;
    TCHAR *args[CEL_ARGVNUM];
    //CelCommandArgEnty args[CEL_ARGVNUM];
}CelCommandTableEntry;

int cel_commandline_help(CelCommandTableEntry *cmd_table);
int cel_commandline_parse(CelArgumentsTableEntry *args_table,
                          int argc, TCHAR **argv);
int cel_commandline_dispatch(CelCommandTableEntry *cmd_table, 
                             int argc, TCHAR **argv);

#ifdef __cplusplus
}
#endif

#endif
