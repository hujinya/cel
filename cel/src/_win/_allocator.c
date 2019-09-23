/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2019 Hu Jinya(hu_jinya@163.com) 
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

#ifdef _CEL_WIN
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
    if ((result = VirtualAlloc(0, size, 
        MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE)) == NULL)
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
    char *ptr, *end;
    size_t resultSize, decommitSize;
    BOOL success;
    MEMORY_BASIC_INFORMATION info;

    if (VirtualFree(start, length, MEM_DECOMMIT))
        return TRUE;
    // The decommit may fail if the memory region consists of allocations
    // from more than one call to VirtualAlloc.  In this case, fall back to
    // using VirtualQuery to retrieve the allocation boundaries and decommit
    // them each individually.
    ptr = (char*)start;
    end = ptr + length;
    while (ptr < end) 
    {
        resultSize = VirtualQuery(ptr, &info, sizeof(info));
        CEL_ASSERT(resultSize == sizeof(info));
        decommitSize = (info.RegionSize < (size_t)(end - ptr) 
            ? info.RegionSize : end - ptr);
        success = VirtualFree(ptr, decommitSize, MEM_DECOMMIT);
        CEL_ASSERT(success == TRUE);
        ptr += decommitSize;
    }
    return TRUE;
}

void os_system_commit(void *start, size_t length)
{
    char *ptr = (char *)start;
    char *end = ptr + length;
    MEMORY_BASIC_INFORMATION info;
    size_t resultSize, commitSize;
    void* newAddress;

    if (VirtualAlloc(start, length, MEM_COMMIT, PAGE_READWRITE) == start)
        return;
    // The commit may fail if the memory region consists of allocations
    // from more than one call to VirtualAlloc.  In this case, fall back to
    // using VirtualQuery to retrieve the allocation boundaries and commit them
    // each individually.
    while (ptr < end) 
    {
        resultSize = VirtualQuery(ptr, &info, sizeof(info));
        CEL_ASSERT(resultSize == sizeof(info));
        commitSize = (info.RegionSize < (size_t)(end - ptr) 
            ? info.RegionSize : end - ptr);
        newAddress = VirtualAlloc(ptr, commitSize, MEM_COMMIT,  PAGE_READWRITE);
        CEL_ASSERT(newAddress == ptr);
        ptr += commitSize;
    }
}
#endif
