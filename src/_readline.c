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
#include "cel/_readline.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"
#include "cel/net/sockaddr.h"
#include "cel/net/_telnet.h"
#include <stdarg.h>

static void cel_readline_clear(CelReadLine *rl);
static void cel_readline_redisplay(CelReadLine *rl);
static void cel_readline_forward_char(CelReadLine *rl, int nbytes);
static void cel_readline_backward_char(CelReadLine *rl, int nbytes);
static void cel_readline_beginning_of_line(CelReadLine *rl);
static void cel_readline_end_of_line(CelReadLine *rl);
static void cel_readline_delete_char(CelReadLine *rl, int nbytes);
static void cel_readline_backward_delete_char(CelReadLine *rl, int nbytes);
static void cel_readline_backward_delete_word(CelReadLine *rl);
static void cel_readline_self_insert(CelReadLine *rl, int nbytes, CHAR c);
//static void cel_readline_self_insert_overwrite(CelReadLine *rl, int nbytes, CHAR c);
static void cel_readline_transpose(CelReadLine *rl);
static void cel_readline_kill_line(CelReadLine *rl);
static void cel_readline_kill_whole_line(CelReadLine *rl);
static void cel_readline_stop_input(CelReadLine *rl);
static void cel_readline_end_config(CelReadLine *rl);
static void cel_readline_history_print(CelReadLine *rl);
static void cel_readline_history_next(CelReadLine *rl);
static void cel_readline_history_previous(CelReadLine *rl);
static void cel_readline_complete(CelReadLine *rl);
static void cel_readline_describe(CelReadLine *rl);
static void cel_readline_escape(CelReadLine *rl);

static void cel_readline_keymaps_init(CelReadLine *rl)
{
    int i;

    memset(rl->key_maps, 0, sizeof(rl->key_maps));
    rl->key_maps[RL_CTL('A')] = (CelReadLineCommandFunc)cel_readline_beginning_of_line;
    rl->key_maps[RL_CTL('B')] = (CelReadLineCommandFunc)cel_readline_backward_char;
    rl->key_maps[RL_CTL('C')] = (CelReadLineCommandFunc)cel_readline_stop_input;
    rl->key_maps[RL_CTL('D')] = (CelReadLineCommandFunc)cel_readline_delete_char;
    rl->key_maps[RL_CTL('E')] = (CelReadLineCommandFunc)cel_readline_end_of_line;
    rl->key_maps[RL_CTL('F')] = (CelReadLineCommandFunc)cel_readline_forward_char;
    rl->key_maps[RL_CTL('H')] = (CelReadLineCommandFunc)cel_readline_backward_delete_char; 
    rl->key_maps[RL_CTL('I')] = (CelReadLineCommandFunc)cel_readline_complete; 
    rl->key_maps[RL_CTL('J')] = (CelReadLineCommandFunc)cel_readline_newline;  
    rl->key_maps[RL_CTL('K')] = (CelReadLineCommandFunc)cel_readline_kill_line;
    rl->key_maps[RL_CTL('M')] = (CelReadLineCommandFunc)cel_readline_newline; 
    rl->key_maps[RL_CTL('N')] = (CelReadLineCommandFunc)cel_readline_history_next;
    rl->key_maps[RL_CTL('P')] = (CelReadLineCommandFunc)cel_readline_history_previous;
    rl->key_maps[RL_CTL('T')] = (CelReadLineCommandFunc)cel_readline_transpose;
    rl->key_maps[RL_CTL('U')] = (CelReadLineCommandFunc)cel_readline_kill_whole_line;
    rl->key_maps[RL_CTL('W')] = (CelReadLineCommandFunc)cel_readline_backward_delete_word;
    rl->key_maps[RL_CTL('Z')] = (CelReadLineCommandFunc)cel_readline_end_config;
    rl->key_maps[RL_CTL('[')] = (CelReadLineCommandFunc)cel_readline_escape;
    rl->key_maps['?'] = (CelReadLineCommandFunc)cel_readline_describe;
    for (i = (BYTE)'\x1f'; i < (TBYTE)'\x7f'; i++)
        rl->key_maps[i] = (CelReadLineCommandFunc)cel_readline_self_insert;
    rl->key_maps[(TBYTE)'\x7f'] = (CelReadLineCommandFunc)cel_readline_backward_delete_char;
    for (i = (BYTE)'\x80'; i < (TBYTE)'\xff'; i++)
        rl->key_maps[i] = (CelReadLineCommandFunc)cel_readline_self_insert;
}

int cel_readline_init(CelReadLine *rl, 
                      CelReadLineInCallbackFunc in_func, 
                      CelReadLineOutCallbackFunc out_func)
{
    rl->state = CEL_RLST_NORMAL;

    rl->cols = rl->rows = 0;
    rl->in_func = in_func;
    rl->out_func = out_func;

    rl->escape = CEL_RLESC_NORMAL;
    rl->cmd_cursor = 0;
    rl->cmd_len = 0;
    rl->cmd_size = 512;
    cel_readline_keymaps_init(rl);

    rl->history_cursor = 0;
    rl->history_index = 0;
    memset(rl->history, 0, sizeof(rl->history));

    return 0;
}

void cel_readline_destroy(CelReadLine *rl)
{
    memset(rl, 0, sizeof(CelReadLine));
}

CelReadLine *cel_readline_new(CelReadLineInCallbackFunc in_func, 
                              CelReadLineOutCallbackFunc out_func)
{
    CelReadLine *rl;

    if ((rl = (CelReadLine *)cel_malloc(sizeof(CelReadLine))) != NULL)
    {
        if (cel_readline_init(rl, in_func, out_func) == 0)
            return rl;
        cel_free(rl);
    }
    return NULL;
}

void cel_readline_free(CelReadLine *rl)
{
    cel_readline_destroy(rl);
    cel_free(rl);
}

int cel_readline_printf(CelReadLine *rl, const CHAR *fmt, ...)
{
    int size = 4095;
    char *buf;

    va_list args;
    va_start(args, fmt);
#ifdef _CEL_WIN
    size = vsnprintf(NULL, 0, fmt, args);
#endif
    buf = (char* )malloc(size + 1);
    size = vsnprintf(buf, size  + 1, fmt, args);
    va_end(args);
    size = rl->out_func(buf, size);
    free(buf);

    return size;
}

static void cel_readline_prompt(CelReadLine *rl)
{
    cel_readline_printf(rl, "%s>", cel_gethostname_a());
}

void cel_readline_clear(CelReadLine *rl)
{
    rl->cmd_cursor = 0;
    rl->cmd_len = 0;
}

/* 
 * Change what's displayed on the screen to reflect the cu,ent contents of cmd_buffer.
 */
void cel_readline_redisplay(CelReadLine *rl)
{
    cel_readline_write(rl, rl->cmd_buf, rl->cmd_len);
    rl->cmd_cursor = rl->cmd_len;
}

void cel_readline_forward_char(CelReadLine *rl, int nbytes)
{
    while ((--nbytes) >= 0 && (rl->cmd_cursor) < (rl->cmd_len))
    {
        cel_readline_putc(rl, rl->cmd_buf[(rl->cmd_cursor)]);
        (rl->cmd_cursor)++;
    }
}

void cel_readline_backward_char(CelReadLine *rl, int nbytes)
{
    while ((--nbytes) >= 0 && (rl->cmd_cursor) > 0)
    {
        cel_readline_putc(rl, RL_BS);
        (rl->cmd_cursor)--;
    }
}

void cel_readline_beginning_of_line(CelReadLine *rl)
{
    while ((rl->cmd_cursor) > 0)
    {
        cel_readline_putc(rl, RL_BS);
        (rl->cmd_cursor)--;
    }
}

void cel_readline_end_of_line(CelReadLine *rl)
{
    while ((rl->cmd_cursor) < (rl->cmd_len))
    {
        cel_readline_putc(rl, rl->cmd_buf[(rl->cmd_cursor)]);
        (rl->cmd_cursor)++;
    }
}

void cel_readline_delete_char(CelReadLine *rl, int nbytes)
{
    size_t len;

    while ((--nbytes) >= 0 && (len = rl->cmd_len - (rl->cmd_cursor)) > 0)
    {
        memmove(&(rl->cmd_buf[(rl->cmd_cursor)]),
            &(rl->cmd_buf[(rl->cmd_cursor) + 1]), len - 1);
        rl->cmd_buf[--(rl->cmd_len)] = '\0';
        /*
        if (rl->node == AUTH_NODE || rl->node == AUTH_ENABLE_NODE)
            return;
        */
        cel_readline_write(rl, &(rl->cmd_buf[(rl->cmd_cursor)]), len - 1);
        cel_readline_putc(rl, RL_SPACE);
        while (len-- > 0)
            cel_readline_putc(rl, RL_BS);
    }
}

void cel_readline_backward_delete_char(CelReadLine *rl, int nbytes)
{
    while ((--nbytes) >= 0 && (rl->cmd_cursor) > 0)
    {
        cel_readline_backward_char(rl, 1);
        cel_readline_delete_char(rl, 1);
    }
}

void cel_readline_backward_delete_word(CelReadLine *rl)
{
    cel_readline_backward_delete_char(rl, 2);
}

void cel_readline_self_insert(CelReadLine *rl, int nbytes, CHAR c)
{
    size_t len;

    if ((len = rl->cmd_len - (rl->cmd_cursor)) > 0)
    {
        memmove(&(rl->cmd_buf[(rl->cmd_cursor) + 1]),
            &(rl->cmd_buf[(rl->cmd_cursor)]), len);
    }
    rl->cmd_buf[(rl->cmd_cursor)] = c;
    cel_readline_write(rl, &(rl->cmd_buf[(rl->cmd_cursor)]), len + 1);
    (rl->cmd_cursor)++;
    (rl->cmd_len)++;
    while (len-- > 0)
    {
        cel_readline_putc(rl, RL_BS);
    }
}

//void cel_readline_self_insert_overwrite(CelReadLine *rl, int nbytes, CHAR c)
//{
//    rl->cmd_buf[(rl->cmd_cursor)++] = c;
//    rl->cmd_len++;
//    cel_readline_putc(rl, c);
//}

void cel_readline_transpose(CelReadLine *rl)
{
    ;
}

void cel_readline_kill_line(CelReadLine *rl)
{
    size_t i, len;

    if ((len = rl->cmd_len - (rl->cmd_cursor)) == 0)
        return;
    for (i = 0; i < len; i++)
        cel_readline_putc(rl, RL_SPACE);
    for (i = 0; i < len; i++)
        cel_readline_putc(rl, RL_BS);
    memset(&rl->cmd_buf[(rl->cmd_cursor)], 0, len);
    rl->cmd_len = (rl->cmd_cursor);
}

void cel_readline_kill_whole_line(CelReadLine *rl)
{
    cel_readline_beginning_of_line(rl);
    cel_readline_kill_line(rl);
}

void cel_readline_stop_input(CelReadLine *rl)
{
    cel_readline_newline(rl);
    cel_readline_puts(rl, "Stop input.\r\n");
    /* Clear command line buffer. */
    cel_readline_clear(rl);
    if (rl->state != CEL_RLST_CLOSE)
        cel_readline_prompt(rl);
}

void cel_readline_end_config(CelReadLine *rl)
{
    cel_readline_newline(rl);
    cel_readline_puts(rl, "End config.\r\n");
    /* Clear command line buffer. */
    cel_readline_clear(rl);
    if (rl->state != CEL_RLST_CLOSE)
        cel_readline_prompt(rl);
}

void cel_readline_history_add(CelReadLine *rl)
{
    int index, last_index;

    if (rl->cmd_len > 0)
    {
        index = rl->history_index;
        last_index = ((index == 0 ) ? CEL_RL_HISTORY_NUM - 1: index - 1);
        if (rl->history[last_index] != NULL
            && strcmp(rl->history[last_index], rl->cmd_buf) == 0)
        {
            rl->history_cursor = rl->history_index;
            return ;
        }
        if (rl->history[index] != NULL)
            cel_free(rl->history[index]);
        rl->cmd_buf[rl->cmd_len] = '\0';
        if ((rl->history[index] = cel_strdup(rl->cmd_buf)) != NULL)
        {
            if ((++rl->history_index) >= CEL_RL_HISTORY_NUM)
                rl->history_index = 0;
            //printf("histroy add = %d\r\n", rl->history_cursor);
        }
    }
    rl->history_cursor = rl->history_index;
}

void cel_readline_history_print(CelReadLine *rl)
{
  size_t len;

  cel_readline_kill_whole_line(rl);
  /* Get previous line from history buffer */
  len = strlen(rl->history[rl->history_cursor]);
  memcpy(rl->cmd_buf, rl->history[rl->history_cursor], len);
  (rl->cmd_cursor) = rl->cmd_len = len;
  /* Redraw current line */
  cel_readline_redisplay(rl);
  //printf("histroy print = %d\r\n", rl->history_cursor);
}

void cel_readline_history_next(CelReadLine *rl)
{
    int try_index;

    /* Try is there history exist or not. */
    try_index = ((rl->history_cursor == (CEL_RL_HISTORY_NUM - 1)) 
        ? 0 : (rl->history_cursor + 1));
    if (try_index != rl->history_index 
        && rl->history[try_index] != NULL)
    {
        /* If there is not history return. */
        rl->history_cursor = try_index;
        cel_readline_history_print(rl);
    }
}

void cel_readline_history_previous(CelReadLine *rl)
{
    int try_index;

    try_index = (rl->history_cursor == 0 
        ? CEL_RL_HISTORY_NUM - 1 : rl->history_cursor - 1);
    if (try_index != rl->history_index 
        && rl->history[try_index] != NULL)
    {
        //printf("histroy print = %d\r\n", try_index);
        rl->history_cursor = try_index;
        cel_readline_history_print(rl);
    }
}

void cel_readline_complete(CelReadLine *rl)
{
    ;
}

void cel_readline_describe(CelReadLine *rl)
{
    ;
}

void cel_readline_escape(CelReadLine *rl)
{
    rl->escape = CEL_RLESC_PRE;
}

static int cel_readline_in(CelReadLine *rl, void *buf, size_t size)
{
    BYTE ch, *in_buf = (BYTE *)buf;
    size_t i, nbytes = size;

    for (i = 0; i < nbytes; i++)
    {
       /* printf("0x%x\r\n", in_buf[i]);
        continue;*/
        ch = in_buf[i];
        if (rl->state == CEL_RLST_MORE)
        {
            switch (ch)
            {
            case RL_CTL('C'):
            case 'q':
            case 'Q':
                //cel_readline_outbuffer_reset(rl);
                break;
            case '\n':
            case '\r':
                rl->state = CEL_RLST_MORELINE;
                break;
            default:
                break;
            }
            continue;
        }
        /* Pre-escape status. */
        if (rl->escape == CEL_RLESC_PRE)
        {
            switch (ch)
            {
            case '[':
                rl->escape = CEL_RLESC_ESCAPE;
                break;
            case 'b': /* Backward word */
                cel_readline_backward_char(rl, 2);
                rl->escape = CEL_RLESC_NORMAL;
                break;
            case 'f': /* Forward word */
                cel_readline_forward_char(rl, 2);
                rl->escape = CEL_RLESC_NORMAL;
                break;
            case 'd': /* Forward delete word */
                cel_readline_delete_char(rl, 2);
                rl->escape = CEL_RLESC_NORMAL;
                break;
            case RL_CTL('H'): /* Backward delete word */
            case (BYTE)'\x7f':
                cel_readline_backward_delete_char(rl, 2);
                rl->escape = CEL_RLESC_NORMAL;
                break;
            default:
                rl->escape = CEL_RLESC_NORMAL;
                break;
            }
            continue;
        }
        /* Escape character. */
        if (rl->escape == CEL_RLESC_ESCAPE)
        {
            switch (ch)
            {
            //case '1': /* VK_HOME */
            //    cel_readline_beginning_of_line(rl);
            //    rl->escape = CEL_RLESC_PRE;
            //    break;
            //case '2': /* VK_INSERT */
            //    rl->escape = CEL_RLESC_PRE;
            //    break;
            //case '3': /* VK_DELETE */
            //    rl->escape = CEL_RLESC_PRE;
            //    break;
            //case '4': /* VK_END */
            //    cel_readline_end_of_line(rl);
            //    rl->escape = CEL_RLESC_PRE;
            //    break;
            //case '5': /* VK_PRIOR */
            //    cel_readline_history_previous(rl);
            //    rl->escape = CEL_RLESC_PRE;
            //    break;
            //case '6': /* VK_NEXT */
            //    cel_readline_history_next(rl);
            //    rl->escape = CEL_RLESC_PRE;
            //    break;
            case 'A': /* VK_UP */
                cel_readline_history_previous(rl);
                rl->escape = CEL_RLESC_NORMAL;
                break;
            case 'B': /* VK_DOWN */
                cel_readline_history_next(rl);
                rl->escape = CEL_RLESC_NORMAL;
                break;
            case 'C': /* VK_LEFT */
                cel_readline_forward_char(rl, 1);
                rl->escape = CEL_RLESC_NORMAL;
                break;
            case 'D': /* VK_RIGHT */
                cel_readline_backward_char(rl, 1);
                rl->escape = CEL_RLESC_NORMAL;
                break;
            default:
                rl->escape = CEL_RLESC_NORMAL;
                break;
            }
            continue;
        }
        if (rl->key_maps[ch] != NULL)
            rl->key_maps[ch](rl, 1, ch);
    }

    return 0;
}

const char *cel_readline_gets(CelReadLine *rl, const char *prompt)
{
    char ch;

    /* Clear command line buffer. */
    cel_readline_clear(rl);
    if (rl->state != CEL_RLST_CLOSE)
        cel_readline_prompt(rl);
    while (rl->in_func(&ch, 1) == 1)
    {
        if (ch == '\r')
        {
            cel_readline_newline(rl);
            cel_readline_history_add(rl);
            break;
        }
        cel_readline_in(rl, &ch, 1);
    }
    rl->cmd_buf[(rl->cmd_cursor)] = '\0';
    return rl->cmd_buf;
}
