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
#ifndef __CEL_READLINE_H__
#define __CEL_READLINE_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RL_BS                '\b'
#define RL_SPACE             '\x20'
#define RL_NEWLINE           "\r\n"
#define RL_CTL(X)            ((X) - '@')
#define CEL_RL_HISTORY_NUM   20

typedef struct _CelReadLine CelReadLine;

typedef int (* CelReadLineInCallbackFunc)(void *buf, size_t len);
typedef int (* CelReadLineOutCallbackFunc)(const void *buf, size_t len);
typedef int (* CelReadLineCommandFunc)(CelReadLine *rl, int nbytes, int key);
typedef char **(* CelReadLineDescribeFunc)(const char *text); 
typedef char **(* CelReadLineCompleteFunc)(const char *text);

typedef enum _CelReadLineState
{
    CEL_RLST_NORMAL, 
    CEL_RLST_CLOSE, 
    CEL_RLST_MORE, 
    CEL_RLST_MORELINE
}CelReadLineState;

typedef enum _CelReadLineEscape
{
    CEL_RLESC_NORMAL,
    CEL_RLESC_PRE,
    CEL_RLESC_ESCAPE
}CelReadLineEscape;

struct _CelReadLine
{
    CelReadLineState state;
    int cols, rows; /**< Window cols, rows */
    CelReadLineInCallbackFunc in_func;
    CelReadLineOutCallbackFunc out_func;
    /**< For escape character, 0 = normal, 1 = pre, 2 = escape. */
    CelReadLineEscape escape; 
    size_t cmd_cursor, cmd_len, cmd_size;
    CHAR cmd_buf[512]; /**< Command lime input buffer */ 
    CelReadLineCommandFunc key_maps[255];
    CelReadLineDescribeFunc describe_func;
    CelReadLineCompleteFunc complete_func;
    int history_cursor, history_index;
    CHAR *history[CEL_RL_HISTORY_NUM]; /**< History of command */
};


int cel_readline_init(CelReadLine *rl, 
                      CelReadLineInCallbackFunc in_func, 
                      CelReadLineOutCallbackFunc out_func);
void cel_readline_destroy(CelReadLine *rl);

CelReadLine *cel_readline_new(CelReadLineInCallbackFunc in_func, 
                              CelReadLineOutCallbackFunc out_func);
void cel_readline_free(CelReadLine *rl);

//int cel_readline_bind_key(CelReadLine *rl, int key, 
//                          CelReadLineCommandFunc func);
//int cel_readline_unbind_key(CelReadLine *rl, int key);

static __inline 
int cel_readline_write(CelReadLine *rl, const void *buf, size_t size)
{
    return rl->out_func(buf, size);
}

static __inline int cel_readline_putc(CelReadLine *rl, CHAR c)
{
    return rl->out_func(&c, 1);
}
#define cel_readline_puts(rl, str) cel_readline_write(rl, str, strlen(str))
static __inline int cel_readline_newline(CelReadLine *rl)
{
    return cel_readline_puts(rl, RL_NEWLINE);
}
int cel_readline_printf(CelReadLine *rl, const char *fmt, ...);
//void cel_readline_clear(CelReadLine *rl);

const char *cel_readline_gets(CelReadLine *rl, const char *prompt);
//int cel_readline_getc(CelReadLine *rl);

void cel_readline_history_add(CelReadLine *rl);

//void cel_readline_get_screen_size(int *rows, int *cols);
//void cel_readline_set_screen_size(int rows, int cols);

#ifdef __cplusplus
}
#endif

#endif
