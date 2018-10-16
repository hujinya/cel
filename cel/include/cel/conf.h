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
#ifndef __CEL_CONF_H__
#define __CEL_CONF_H__

#include "cel/config.h"
#include "cel/json.h"

typedef CelJsonNode CelConfItem;
typedef CelJson CelConf;

#ifdef __cplusplus
extern "C" {
#endif

int cel_conf_init(CelConf *conf);
void cel_conf_destroy(CelConf *conf);

CelConf *cel_conf_new();
void cel_conf_free();

int cel_conf_get(CelConf *conf, const char *keys);
int cel_conf_set(CelConf *conf, const char *keys);

#ifdef __cplusplus
}
#endif

#endif
