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
#include "cel/conf.h"

int cel_conf_init(CelConf *conf)
{
    return 0;
}

void cel_conf_destroy(CelConf *conf)
{
}

CelConf *cel_conf_new(void)
{
    return NULL;
}

void cel_conf_free(CelConf *conf)
{

}

int cel_conf_read_file(CelConf *conf, const TCHAR *path)
{
    return 0;
}

int cel_conf_write_file(CelConf *conf, const TCHAR *path)
{
    return 0;
}
