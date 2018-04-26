/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __OS_ALLOCATOR_WIN_H__
#define __OS_ALLOCATOR_WIN_H__

#include "cel/types.h"
#include <sys/types.h>       /* for _off_t */

#ifdef __cplusplus
extern "C" {
#endif

#define alloca _alloca

#define MAP_FAILED             0
#define MREMAP_FIXED           2     /* the value in linux, though it doesn't really matter */

#define PROT_READ              PAGE_READWRITE
#define PROT_WRITE             PAGE_READWRITE
#define MAP_ANONYMOUS          MEM_RESERVE
#define MAP_PRIVATE            MEM_COMMIT
#define MAP_SHARED             MEM_RESERVE   /* value of this #define is 100% arbitrary */

/* VirtualAlloc only replaces for mmap when certain invariants are kept. */
#define mmap(addr, length, prot, flags, fd, offset) \
    (((addr) == NULL && (fd) == -1 && (offset) == 0 \
    && (prot) == (PROT_READ | PROT_WRITE) \
    && (flags) == (MAP_PRIVATE | MAP_ANONYMOUS)) \
    ? VirtualAlloc(0, length, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE) : NULL)
#define munmap(addr, length) (VirtualFree(addr, 0, MEM_RELEASE) ? 0 : -1)

static __inline int getpagesize(void)
{
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    return (int)system_info.dwPageSize;
}

void *os_system_alloc(size_t size, size_t *actual_size, size_t alignment);
BOOL os_system_release(void *start, size_t length);
void os_system_commit(void *start, size_t length);

#ifdef __cplusplus
}
#endif

#endif
