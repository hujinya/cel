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
#include "cel/crypto/checksum.h"
#include "cel/error.h"
#include "cel/log.h"


/*
 * The checksum is the 16-bit one's complement of the one's complement
 * sum of the entire VRRP message starting with the version field.  For
 * computing the checksum, the checksum field is set to zero. 
 * See RFC1071 for more detail.
 */
U16 cel_checksum(U16 *buf, size_t len)
{
    int sum;
    
    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    /* Mop up an odd byte, if necessary */
    if (len == 1)
        sum += (*(BYTE *)buf << 8);

    sum = (sum >> 16) + (sum & 0xFFFF);     /* add hi 16 to low 16 */
    sum += (sum >> 16);                     /* add carry */    

    return  (U16)(~sum);  
}
