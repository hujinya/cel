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
#include "cel/_command.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"
#include <stdarg.h>

/* Debug defines */
#define Debug(args)   cel_log_debug args 
#define Warning(args) /*CEL_SETERRSTR(args)*/ cel_log_warning args 
#define Err(args)   /*CEL_SETERRSTR(args)*/ cel_log_err args 

int cel_command_init(CelCommand *cmd)
{
    cmd->child_list.head = NULL;
    return 0;
}

static int cel_command_remove(CelCommandNodeList *node_list)
{
    CelCommandNode *node;

    while ((node = node_list->head) != NULL)
    {
        _tprintf(_T("Uninstall %s.\r\n"), node->str);
        if (node->str != NULL)
            cel_free(node->str);
        cel_command_remove(&(node->child_list));
        cel_command_remove(&(node->token_list));
        node_list->head = node->next;
        cel_free(node);
    }
    return 0;   
}

void cel_command_destroy(CelCommand *cmd)
{
    cel_command_remove(&(cmd->child_list));
    cmd->child_list.head = NULL;
}

CelCommand *cel_command_new(void)
{
    CelCommand *cmd;

    if ((cmd = (CelCommand *)cel_malloc(sizeof(CelCommand))) != NULL)
    {
        if (cel_command_init(cmd) == 0)
            return cmd;
        cel_free(cmd);
    }

    return NULL;
}

void cel_command_free(CelCommand *cmd)
{
    cel_command_destroy(cmd);
    cel_free(cmd);
}

#define cel_command_args_destroy(args) \
    while ((--((args)->argc)) >= 0) \
        if ((args)->argv[(args)->argc] != NULL) \
            cel_free((args)->argv[(args)->argc]) 

int cel_command_args_init(CelCommandArgs *args, const TCHAR *cmd_line)
{
    size_t len;
    const TCHAR *start, *end = cmd_line;
    
    args->argc = 0;
next_argument:
    args->token32 = UL(0); 
    start = end;
    //_putts(end);
    while (TRUE) 
    {
        switch (*end)
        {
        case _T('('):
            args->token8[0]++;
            break;
        case _T('{'):
            args->token8[1]++;
            break;
        case _T('['):
            args->token8[2]++;
            break;
        case _T('<'):
            args->token8[3]++;
            break;
        case _T(')'):
            args->token8[0]--;
            break;
        case _T('}'):
            args->token8[1]--;
            break;
        case _T(']'):
            args->token8[2]--;
            break;
        case _T('>'):
            args->token8[3]--;
            break;
        case _T(' '):
        case _T('\n'):
        case _T('\r'):
        case _T('\0'):
            //_tprintf(_T("token = %d size = %d %s\r\n"), args->token32, end - start, start);
            if (args->token32 == UL(0))
            {
                len = end - start;
                args->argv[args->argc] = 
                    (TCHAR *)cel_malloc((len + 1) * sizeof(TCHAR));
                memcpy(args->argv[args->argc], start, len * sizeof(TCHAR));
                args->argv[args->argc][len] = _T('\0');
                if (++(args->argc) >= CEL_COMMAND_ARGC)
                    return 0;
                /* Get next argument */
                while (TRUE)
                {
                    if (*end == _T('\0'))
                        return 0;
                    if (!(CEL_ISSPACE(*end) 
                        || *end == _T('\n') || *end == _T('\r')))
                        break;
                    end++;
                }
                goto next_argument;
            }
            else if (*end != _T(' '))
            {
                Err((_T("Command line '%s' arguments init failed, token 0x%x."), 
                    cmd_line, args->token32));
                cel_command_args_destroy(args);
                return -1;
            }
            break;
        default:
            break;
        }
        end++;
    }
}

static CelCommandNode *cel_command_lookup(CelCommandNodeList *node_list, 
                                          const TCHAR *str)
{
    int ret;
    CelCommandNode *node = node_list->head;

    //_tprintf(_T("look up '%s' %p\r\n"), str, node_list->head);
    while (node != NULL)
    {
        if ((ret = _tcscmp(node->str, str)) == 0)
            return node;
        if (ret > 0)
            return NULL;
        node = node->next;
    }
    return NULL;
}

static void cel_command_insert(CelCommandNodeList *node_list, 
                               CelCommandNode *child_node)
{
    CelCommandNode *node;

    //_tprintf(_T("insert '%s'\r\n"), child__node->str);
    if ((node =  node_list->head) == NULL)
    {
        node_list->head = child_node;
        return ;
    }
    while (TRUE)
    {
        /*
        _tprintf(_T("_tcscmp(%s, %s) = %d\r\n"), 
            node->str, child_node->str, _tcscmp(node->str, child_node->str));
            */
        if (_tcscmp(node->str, child_node->str) > 0)
        {
            if (node->preious != NULL)
            {
                node->preious->next = child_node;
                child_node->preious = node->preious;
            }
            else
            {
                node_list->head = child_node;
                child_node->preious = NULL;
            }
            node->preious = child_node;
            child_node->next = node;
            return;
        }
        if (node->next == NULL)
        {
            node->next = child_node;
            child_node->preious = node;
            return ;
        }
        node = node->next;
    }
}

static CelCommandNode *cel_command_install(CelCommandNodeList *node_list,
                                           const TCHAR *cmd_str)
{
    CelCommandTokenType type;
    int depth = 0;
    size_t len;
    TCHAR buf[1024];
    const TCHAR *start;
    CelCommandNode *node = NULL;
    union {
        U8 uint8[4];
        U32 uint32;
    }token_cnt;

    token_cnt.uint32 = UL(0);
next_argument: 
    type = CEL_COMMAND_ROOT;
    while (TRUE)
    {
        if (type == 0)
        {
            if (*cmd_str == _T(' ') 
                || *cmd_str == _T('\n') || *cmd_str == _T('\r'))
                break;
            start = cmd_str;
            switch (*cmd_str)
            {
            case _T('{'):
                type = CEL_COMMAND_KEYWORD;
                token_cnt.uint8[0]++;
                break;
            case _T('('):
                type = CEL_COMMAND_MULTIPLE;
                token_cnt.uint8[1]++;
                break;
            case _T('['):
                type = CEL_COMMAND_OPTION;
                token_cnt.uint8[2]++;
                break;
            case _T('<'):
                type = CEL_COMMAND_RANGE;
                token_cnt.uint8[3]++;
                break;
            default:
                if (_tcscmp(cmd_str, _T("A.B.C.D")) == 0)
                {
                    type = CEL_COMMAND_IPV4;
                    (cmd_str) += 7;
                }
                else if (_tcscmp(cmd_str, _T("A.B.C.D/M")) == 0)
                {
                    type = CEL_COMMAND_IPV4_PREFIX;
                    (cmd_str) += 9;
                }
                else if (_tcscmp(cmd_str, _T("X:X::X:X")) == 0)
                {
                    type = CEL_COMMAND_IPV6;
                    (cmd_str) += 8;
                }
                else if (_tcscmp(cmd_str, _T("X:X::X:X/M")) == 0)
                {
                    type = CEL_COMMAND_IPV6_PREFIX;
                    (cmd_str) += 10;
                }
                else if (*cmd_str == _T('.'))
                    type = CEL_COMMAND_VARARG;
                else if (*cmd_str >= _T('A') && *cmd_str <= _T('Z'))
                    type = CEL_COMMAND_VARIABLE;
                else
                    type = CEL_COMMAND_LITERAL;
                break;
            }
            if (*cmd_str == _T('\0'))
            {
                return node;
            }
        }
        else
        {
            switch (*cmd_str)
            {
            case _T('{'):
                token_cnt.uint8[0]++;
                break;
            case _T('('):
                token_cnt.uint8[1]++;
                break;
            case _T('['):
                token_cnt.uint8[2]++;
                break;
            case _T('<'):
                token_cnt.uint8[3]++;
                break;
            case _T('}'):
                token_cnt.uint8[0]--;
                break;
            case _T(')'):
                token_cnt.uint8[1]--;
                break;
            case _T(']'):
                token_cnt.uint8[2]--;
                break;
            case _T('>'):
                token_cnt.uint8[3]--;
                break;
            //case _T('|'):
            //case _T('-'):
            case _T(' '):
            case _T('\n'):
            case _T('\r'):
            case _T('\0'):
                if (token_cnt.uint32 == UL(0))
                {
                    if ((len = cmd_str - start) >= 0)
                    {
                        memcpy(buf, start, len * sizeof(TCHAR));
                        buf[len] = _T('\0');
                        _tprintf(_T("Type: %d, %s\r\n"), type, buf);
                        if ((node = cel_command_lookup(node_list, buf)) == NULL)
                        {
                            if ((node = (CelCommandNode *)
                                cel_malloc(sizeof(CelCommandNode))) == NULL)
                            {
                                return NULL;
                            }
                            node->preious = node->next = NULL;
                            node->type = type;
                            node->str = cel_tcsdup(buf);
                            node->depth = depth;
                            node->ref_counted = 0;
                            node->element = NULL;
                            node->token_list.head = NULL;
                            node->child_list.head = NULL;
                            cel_command_insert(node_list, node);
                            if (node->type == CEL_COMMAND_MULTIPLE
                                || node->type == CEL_COMMAND_KEYWORD)
                            {
                                buf[len - 1] = _T('\0');
                                cel_command_install(&(node->token_list), buf + 1);
                            }
                        }
                        node->ref_counted++;
                        depth++;
                        node_list = &(node->child_list);
                    }
                    if (*cmd_str == _T('\0'))
                        return node;
                    (cmd_str)++;
                    goto next_argument;
                }
                else if (*cmd_str != _T(' '))
                {
                    Err((_T("Command line '%s' arguments init failed, token 0x%x."), 
                        cmd_str, token_cnt.uint32));
                    return NULL;
                }
                break;
            default:
                break;
            }
        }
        (cmd_str)++;
    }
    return node;
}


int cel_command_install_element(CelCommand *cmd, CelCommandElement *element)
{
    CelCommandNode *node;

    if ((node = cel_command_install(&(cmd->child_list), element->str)) == NULL
        || node->element != NULL)
    {
         Err((_T("Element '%s' already exists."), element->str));
        return -1;
    }
    node->element = element;
    //_tprintf(_T("Install %s\r\n"), node->str);

    return 0;
}

int cel_cli_install_default(CelCommand *cmd)
{
    return 0;
}

int cel_clisession_init(CelCommandSession *cli_si, 
                        CelCommand *cmd, 
                        int (*out_func) (void *buf, size_t size))
{
    cli_si->cmd = cmd;
    cli_si->out_func = out_func;
    cli_si->cur_node = NULL;

    return 0;
}

void cel_clisession_destroy(CelCommandSession *cli_si)
{
    ;
}

int cel_clisession_out(CelCommandSession *cli_si, const TCHAR *fmt, ...)
{
    size_t out_size;
    va_list args;
    TCHAR out_buf[CEL_LINELEN];

    va_start(args, fmt);
    out_size = _vsntprintf(out_buf, CEL_LINELEN, fmt, args);
    va_end(args);

    return cli_si->out_func(out_buf, out_size);
}

static void cel_clisession_nodelist_out(CelCommandSession *cli_si, 
                                        CelCommandNodeList *node_list)
{
    int i;
    CelCommandNode *node = node_list->head;

    while (node != NULL)
    {
        for (i = 0; i < node->depth; i++)
            cel_clisession_out(cli_si, _T("| "));
        if (node->element != NULL)
            cel_clisession_out(cli_si, 
            _T("|-%s %s")CEL_CRLF, node->str, node->element->help);
        else
            cel_clisession_out(cli_si, _T("|-%s")CEL_CRLF, node->str);
        cel_clisession_nodelist_out(cli_si, &(node->child_list));
        node = node->next;
    }
}

int cel_clisession_describe(CelCommandSession *cli_si, 
                            CelCommandNodeList *node_list)
{
    //CelCommandNode *node;
    cel_clisession_nodelist_out(cli_si, &(cli_si->cmd->child_list));

    return 0;
}

int cel_clisession_complete(CelCommandSession *cli_si, const TCHAR *cmd_str)
{
    //CelCommandArgs args;

    //if (cel_command_args_init(&args, cmd_str) == -1)
    //    return -1;
    //cel_command_args_destroy(&args);

    return 0;
}

int cel_clisession_execute(CelCommandSession *cli_si, const TCHAR *cmd_str)
{
    int depth, ret;
    CelCommandNodeList *node_list;
    CelCommandNode *cur_node;
    CelCommandArgs args;

    if (cel_command_args_init(&args, cmd_str) == -1)
        return -1;
    if ((cur_node = cli_si->cur_node) == NULL)
        node_list = &(cli_si->cmd->child_list);
    else
        node_list = &(cur_node->child_list);
    for (depth = 0; depth < args.argc; depth++)
    {
        //_tprintf(_T("%s####\r\n"), args.argv[depth]);
        if ((cur_node = cel_command_lookup(
            node_list, args.argv[depth])) == NULL)
        {
            Err((_T("Command %s invalid."), cmd_str));
            return -1;
        }
        node_list = &(cur_node->child_list); 
    }  
    if (cur_node->element == NULL)
    {
        Err((_T("Element '%s' not exists."), cmd_str));
        return -1;
    }
    ret = cur_node->element->exec_func(cli_si, args.argc, args.argv);
    cel_command_args_destroy(&args);

    return ret;
}
