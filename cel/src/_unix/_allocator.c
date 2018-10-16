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
#include "cel/allocator.h"
#include "cel/log.h"
#ifdef _CEL_UNIX

// On systems (like freebsd) that don't define MAP_ANONYMOUS, use the old
// form of the name instead.
#ifndef MAP_ANONYMOUS
# define MAP_ANONYMOUS MAP_ANON
#endif

// MADV_FREE is specifically designed for use by malloc(), but only
// FreeBSD supports it; in linux we fall back to the somewhat inferior
// MADV_DONTNEED.
#if !defined(MADV_FREE) && defined(MADV_DONTNEED)
# define MADV_FREE  MADV_DONTNEED
#endif

// Solaris has a bug where it doesn't declare madvise() for C++.
//    http://www.opensolaris.org/jive/thread.jspa?threadID=21035&tstart=0
#if defined(__sun) && defined(__SVR4)
# include <sys/types.h>    // for caddr_t
  extern "C" { extern int madvise(caddr_t, size_t, int); }
#endif

void *os_system_alloc(size_t size, size_t *actual_size, size_t alignment)
{
    // Align on the pagesize boundary
    const size_t pagesize = getpagesize();
    size_t extra;
    void *result;
    uintptr_t ptr;
    size_t adjust;

    if (alignment < pagesize) 
        alignment = pagesize;
    size = CEL_ALIGNED(size, alignment);

    // Safest is to make actual_size same as input-size.
    if (actual_size != NULL)
    {
        *actual_size = size;
        //_tprintf(_T("actual_size %d, size %d")CEL_CRLF, *actual_size, size);
    }
    // Ask for extra memory if alignment > pagesize
    extra = 0;
    if (alignment > pagesize) 
        extra = alignment - pagesize;
    if ((result = mmap(0, size + extra, 
        PROT_WRITE | PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)) == NULL)
        return NULL;
    // Adjust the return memory so it is aligned
    ptr = (uintptr_t)(result);
    adjust = 0;
    if ((ptr & (alignment - 1)) != 0) 
        adjust = alignment - (ptr & (alignment - 1));
    ptr += adjust;

    return (void *)ptr;
}

BOOL os_system_release(void *start, size_t length)
{
    const size_t pagesize = getpagesize();
    const size_t pagemask = pagesize - 1;
    size_t new_start = (size_t)start;
    size_t end = new_start + length;
    size_t new_end = end;
    int result;

    // Round up the starting address and round down the ending address
    // to be page aligned:
    new_start = (new_start + pagesize - 1) & ~pagemask;
    new_end = new_end & ~pagemask;

    CEL_ASSERT((new_start & pagemask) == 0);
    CEL_ASSERT((new_end & pagemask) == 0);
    CEL_ASSERT(new_start >= (size_t)start);
    CEL_ASSERT(new_end <= end);

    if (new_end > new_start) 
    {
        do 
        {
            result = madvise((char*)new_start, new_end - new_start, MADV_FREE);
        } while (result == -1 && errno == EAGAIN);

        return (result != -1);
    }
    return FALSE;
}

#endif
