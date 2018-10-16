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
#ifndef __CEL_COMMAND_H__
#define __CEL_COMMAND_H__

#include "cel/types.h"
#include "cel/arraylist.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _CelCommandElement;
struct _CelCommandNode;
struct _CelCommand;
struct _CelCommandSession;

#define CEL_COMMAND_ARGC                128
/* Return value of the commands. */
#define CEL_COMMAND_SUCCESS               0
#define CEL_COMMAND_ERROR                -1
#define CEL_COMMAND_ERR_NO_MATCH         -2
#define CEL_COMMAND_ERR_AMBIGUOUS        -3
#define CEL_COMMAND_ERR_INCOMPLETE       -4
#define CEL_COMMAND_ERR_EXEED_ARGC_MAX   -5
#define CEL_COMMAND_ERR_NOTHING_TODO     -6
#define CEL_COMMAND_COMPLETE_FULL_MATCH  -7
#define CEL_COMMAND_COMPLETE_MATCH       -8
#define CEL_COMMAND_COMPLETE_LIST_MATCH  -9
#define CEL_COMMAND_SUCCESS_DAEMON      -10

/**
 * TOKEN: | () {} [] <> = .
 * PARAMETER: 
 * KEYWORD:
 */
typedef enum _CelCommandTokenType
{
    CEL_COMMAND_ROOT = 0x000,
    CEL_COMMAND_LITERAL,
    CEL_COMMAND_OPTION,
    CEL_COMMAND_VARIABLE,
    CEL_COMMAND_VARARG,
    CEL_COMMAND_RANGE,
    CEL_COMMAND_IPV4,
    CEL_COMMAND_IPV4_PREFIX,
    CEL_COMMAND_IPV6,
    CEL_COMMAND_IPV6_PREFIX,
    CEL_COMMAND_MULTIPLE = 0x100,
    CEL_COMMAND_KEYWORD = 0x200
}CelCommandTokenType;

typedef struct _CelCommandToken
{
    CelCommandTokenType type;
    const TCHAR *str;
}CelCommandToken;

typedef struct _CelCommandArgs
{
    union {
        U8 token8[4];
        U32 token32;
    };
    int argc;
    TCHAR *argv[CEL_COMMAND_ARGC];
}CelCommandArgs;

typedef struct _CelCommandElement
{
    const TCHAR *str;                    /**< Command line strings */
    const TCHAR *help;                   /**< Help strings */
    int (*exec_func) (struct _CelCommandSession *cli_si, int argc, TCHAR *argv[]);
                                         /**< Function to execute this command */
}CelCommandElement;

typedef struct _CelCommandNodeList
{
    struct _CelCommandNode *head;
}CelCommandNodeList;

typedef struct _CelCommandNode
{
    struct _CelCommandNode *preious, *next;  /**< Node list pointer */
    struct _CelCommandNode *parent;          /**< Node parent */
    CelCommandTokenType type;                /**< Command node token types */
    TCHAR *str;                              /**< String to be matched */
    int depth;                               /**< Node depth */
    int ref_counted;                             /**< Reference count */
    struct _CelCommandElement *element;      /**< Pointer to cmd element */
    struct _CelCommandNodeList token_list;   /**< Command token(multiple, keyword) list */
    struct _CelCommandNodeList child_list;   /**< Command tree child node list */
}CelCommandNode;

typedef struct _CelCommand
{
    const TCHAR *prompt;                     /**< Prompt string */
    struct _CelCommandNodeList child_list;   /**< Child node list */
}CelCommand;

typedef enum _CelCommandStatus
{
    CEL_CLISTATUS_NORMAL,
    CEL_CLISTATUS_CLOSE,
    CEL_CLISTATUS_MORE,
    CEL_CLISTATUS_CONTINUE,
    CEL_CLISTATUS_MORE_CONTINUE
}CelCommandStatus;

typedef struct _CelCommandSession
{
    struct _CelCommand *cmd;
    int (*out_func) (void *buf, size_t size);
    CelCommandStatus status;
    CelCommandNode *cur_node;
}CelCommandSession;

int cel_command_init(CelCommand *cmd);
void cel_command_destroy(CelCommand *cmd);

CelCommand *cel_command_new(void);
void cel_command_free(CelCommand *cmd);

/* Define command */
#define CEL_CLI(function, element_struct, cmd_str, help_str) \
    static int function(CelCommandSession *cmd, int argc, TCHAR *argv[]); \
    CelCommandElement element_struct = { cmd_str, help_str, function };\
    static int function(CelCommandSession *cli_si, int argc, TCHAR *argv[])
#define CEL_ALI(function, element_struct, cmd_str, help_str) \
    CelCommandElement element_struct = { cmd_str, help_str, function }

int cel_command_install_element(CelCommand *cmd, CelCommandElement *element);

int cel_clisession_init(CelCommandSession *cli_si, 
                        CelCommand *cmd, 
                        int (*out_func) (void *buf, size_t size));
void cel_clisession_destroy(CelCommandSession *cli_si);

CelCommandSession *cel_clisession_new(CelCommand *cmd, 
                                      int (*out_func) (void *buf, size_t size));
void cel_clisession_free(CelCommandSession *cli_si);

int cel_clisession_out(CelCommandSession *cli_si, const TCHAR *fmt, ...);

TCHAR *cel_clisession_prompt(CelCommandSession *cli_si);
int cel_clisession_describe(CelCommandSession *cli_si, CelCommandNodeList *node_list);
int cel_clisession_complete(CelCommandSession *cli_si, const TCHAR *cmd_str);
int cel_clisession_execute(CelCommandSession *cli_si, const TCHAR *cmd_str);

#ifdef __cplusplus
}
#endif

#endif
