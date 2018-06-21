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
#ifndef __CEL_ALLOCATOR_UNIX_H__
#define __CEL_ALLOCATOR_UNIX_H__

#include "cel/types.h"
#include <sys/mman.h> /* mmap(),munmap()*/

#ifdef __cplusplus
extern "C" {
#endif

void *os_system_alloc(size_t size, size_t *actual_size, size_t alignment);
BOOL os_system_release(void *start, size_t length);
#define os_system_commit(start, length)

#ifdef __cplusplus
}
#endif

#endif