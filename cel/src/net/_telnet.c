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
#include "cel/net/_telnet.h"

    //BYTE iac, iac_sb; 
    //                  /**< IAC(Interpret as Command) Refer to Telnet RFC 854. */
    //size_t iac_len;
    //BYTE iac_buf[9];
 //if ((ch = in_buf[i]) == IAC)
        //{
        //    if (rl->iac_len != 0 && rl->iac_sb != 1)
        //    {
        //        cel_readline_terminal_option_recv(rl);
        //        rl->iac_len = 0;
        //    }
        //    rl->iac = 1;
        //}
        ///* Telnet command */
        //if (rl->iac == 1)
        //{
        //    if (ch == SB)
        //        rl->iac_sb = 1;
        //    if (rl->iac_len < sizeof(rl->iac_buf))
        //        rl->iac_buf[rl->iac_len++] = ch;
        //    if (ch == SE)
        //    {
        //        cel_readline_terminal_option_recv(rl);
        //        rl->iac = 0;
        //        rl->iac_sb = 0;
        //        rl->iac_len = 0;
        //    }
        //    continue;
        //}

//static void cel_readline_terminal_option_send(CelReadLine *rl)
//{
//    BYTE cmd1[] = { IAC, WILL, TELOPT_ECHO };
//    BYTE cmd2[] = { IAC, WILL, TELOPT_SGA };
//    BYTE cmd3[] = { IAC, DONT, TELOPT_LINEMODE };
//    BYTE cmd4[] = { IAC, DO, TELOPT_NAWS };
//    //BYTE cmd5[] = { IAC, DONT, TELOPT_LFLOW };
//
//    /*
//    printf("IAC = %x WILL = %x DONT = %x DO = %x "
//        "TELOPT_ECHO = %x TELOPT_SGA = %x TELOPT_LINEMODE = %x"
//        "TELOPT_NAWS = %x TELOPT_LFLOW = %x SE = %x SB = %x\r\n",
//        IAC, WILL, DONT, DO, TELOPT_ECHO, TELOPT_SGA, TELOPT_LINEMODE,
//        TELOPT_NAWS, TELOPT_LFLOW, SE, SB);
//        */
//
//    cel_readline_write(rl, cmd1, sizeof(cmd1));
//    cel_readline_write(rl, cmd2, sizeof(cmd2));
//    cel_readline_write(rl, cmd3, sizeof(cmd3));
//    cel_readline_write(rl, cmd4, sizeof(cmd4));
//    //cel_readline_write(rl, cmd5, sizeof(cmd5));
//}
//
//static void cel_readline_terminal_option_recv(CelReadLine *rl)
//{
//    switch (rl->iac_buf[1])
//    {
//    /* Telnet window size option RFC 1703 */
//    case SB:
//        if (rl->iac_len == 9
//            && rl->iac_buf[2] == TELOPT_NAWS)
//        {
//            rl->rows = ((rl->iac_buf[3] << 8) | rl->iac_buf[4]);
//            rl->cols = ((rl->iac_buf[5] << 8) | rl->iac_buf[6]);
//            //printf("NAWS %d x %d.\r\n", rl->rows, rl->cols);
//        }
//        break;
//    default:
//        //printf("IAC 0x%x 0x%x\r\n", rl->iac_buf[1], rl->iac_buf[2]);
//        break;
//    }
//}
//
