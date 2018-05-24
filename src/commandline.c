/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2016 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/commandline.h"
#include "cel/error.h"
#include "cel/log.h"

/* Debug defines */
#define Debug(args)   /* cel_log_debug args */
#define Warning(args) CEL_SETERRSTR(args)/* cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args)/* cel_log_err args */

int cel_commandline_help(CelCommandTableEntry *cmd_table)
{
    return 0;
}

int cel_commandline_parse(CelArgumentsTableEntry *args_table,
                          int argc, TCHAR **argv)
{
    return 0;
}

int cel_commandline_dispatch(CelCommandTableEntry *cmd_table, 
                             int argc, TCHAR **argv)
{
    int i = 0 ,j, k;

    while (cmd_table[i].key != NULL)
    {
        if (argc < 2) 
        {
            if (_tcscmp(cmd_table[i].key, _T("-s")) != 0 
                &&  _tcscmp(cmd_table[i].full_key, _T("--start")) != 0)
            {
                i++;
                continue;
            }
        }
        else if (_tcscmp(argv[1], cmd_table[i].key) != 0 
            && _tcscmp(argv[1], cmd_table[i].full_key) != 0)
        {
            i++;
            continue;
        }
        if (cmd_table[i].proc != NULL)
        {
            /* Check args count */
            j = 0;
            while (cmd_table[i].args[j] != NULL)
                j++;
            if (argc + 2 <= j)
            {
                break;
            }
            return cmd_table[i].proc(argc, argv);
        }
        /* Print help message */
        else if (_tcscmp(cmd_table[i].key, _T("-h")) == 0)
        {
            _tprintf(_T("%sUsage:%s "), CEL_CRLF, argv[0]);
            i = 0;
            while (cmd_table[i].key != NULL)
            {
                if (i == 0)_tprintf(_T(" -["));
                _puttchar(cmd_table[i++].key[1]);
            }
            if  (i != 0)_puttchar(_T(']'));
            _tprintf(_T("%sCommands:%s"), CEL_CRLF, CEL_CRLF);
            i = 0;
            while (cmd_table[i].key != NULL)
            {
                k = _tprintf(_T("  %s, %s"), 
                    cmd_table[i].key, cmd_table[i].full_key);
                j = 0;
                while (cmd_table[i].args[j] != NULL)
                {
                    k +=_tprintf(_T(" <%s>"), cmd_table[i].args[j]);
                    j++;
                }
                if (k > 28)
                {
                    _tprintf(_T("%s"), CEL_CRLF);
                    k = 0;
                }
                k = 28 - k;
                while ((--k) > 0)_puttchar(_T(' '));
                _tprintf(_T(" %s.%s"),cmd_table[i].help, CEL_CRLF);
                i++;
            }
            _tprintf(_T("%s"), CEL_CRLF);
            return 0;
        }
        /* Not define process function */
        else
        {
            return 0;
        }
    }
    /* Print error information */
    _tprintf(_T("%sInvalid command -- '"), CEL_CRLF);
    i = 1;
    if (i < argc)_tprintf(_T("%s"), argv[i++]);
    while (i < argc)_tprintf(_T(" %s"), argv[i++]);
    _tprintf(_T("'%sTry '%s -h' or '%s --help' for more information.%s%s"), 
        CEL_CRLF, argv[0], argv[0], CEL_CRLF, CEL_CRLF);

    return -1;
}
