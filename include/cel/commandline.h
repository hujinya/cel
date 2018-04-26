/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
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
    TCHAR *key;
    TCHAR *full_key;
    TCHAR *help;
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
