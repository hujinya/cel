/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __OS_ALLOCATOR_UNIX_H__
#define __OS_ALLOCATOR_UNIX_H__

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