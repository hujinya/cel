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
#include "cel/ringlist.h"
#include "cel/error.h"
#include "cel/allocator.h"
#include "cel/convert.h"

int cel_ringlist_init(CelRingList *ring_list, size_t size, 
                      CelFreeFunc value_free)
{
    size = cel_power2min((int)size);
    //printf("size = %d\r\n", size);
    if ((ring_list->ring = (void **)cel_malloc(sizeof(void *) * size)) == NULL)
        return -1;
    ring_list->size = size;
    ring_list->value_free = value_free;
    ring_list->mask = size -1;
    ring_list->p_head = 0;
    ring_list->p_tail = 0;
    ring_list->c_head = 0;
    ring_list->c_tail = 0;

    return 0;
}

void cel_ringlist_destroy(CelRingList *ring_list)
{
    cel_ringlist_clear(ring_list);
    cel_free(ring_list->ring);
}

CelRingList *cel_ringlist_new(size_t size, CelFreeFunc value_free)
{
    CelRingList *ring_list;

    if ((ring_list = (CelRingList *)cel_malloc(
        sizeof(CelRingList))) != NULL)
    {
        if (cel_ringlist_init(ring_list, size, value_free) == 0)
            return ring_list;
        cel_free(ring_list);
    }

    return NULL;
}

void cel_ringlist_free(CelRingList *ring_list)
{
    cel_ringlist_destroy(ring_list);
    cel_free(ring_list);
}

void cel_ringlist_clear(CelRingList *ring_list)
{
    puts("cel_ringlist_clear is null");
}

static __inline 
void cel_ringlist_push_values(CelRingList *ring_list, 
                              unsigned long prod_head, size_t n, void **values)
{ 
    unsigned int i, idx;

    idx = (prod_head & (ring_list->mask));
    if (idx + n < ring_list->size)
    { 
        for (i = 0; i < (n & ((~(unsigned)0x3))); i += 4, idx += 4)
        { 
            ring_list->ring[idx] = values[i];
            ring_list->ring[idx + 1] = values[i + 1];
            ring_list->ring[idx + 2] = values[i + 2];
            ring_list->ring[idx + 3] = values[i + 3];
        } 
        switch (n & 0x3) 
        { 
        case 3: ring_list->ring[idx++] = values[i++];
        case 2: ring_list->ring[idx++] = values[i++];
        case 1: ring_list->ring[idx++] = values[i++];
        } 
    }
    else 
    { 
        for (i = 0; idx < ring_list->size; i++, idx++)
            ring_list->ring[idx] = values[i]; 
        for (idx = 0; i < n; i++, idx++) 
            ring_list->ring[idx] = values[i]; 
    } 
}

int _cel_ringlist_push_do(CelRingList *ring_list, BOOL is_sp, size_t n,
                          CelRingListConsFunc handle, void *user_data)
{
    U32 prod_head, prod_next;
    U32 cons_tail, free_entries;
    U32 mask = ring_list->mask;
    unsigned rep = 0;

    /* 
    * Avoid the unnecessary cmpset operation below, which is also
    * potentially harmful when n equals 0. 
    */
    if (n == 0)
        return 0;
    /* move p_head atomically */
    do 
    {
        /* Reset n to the initial burst count */
        prod_head = ring_list->p_head;
        cons_tail = ring_list->c_tail;
        free_entries = (mask + cons_tail - prod_head);
        //printf("cons_tail = %d, prod_head = %d\r\n", cons_tail, prod_head);
        /* check that we have enough room in ring */
        if (n > free_entries)
        {
            if (free_entries == 0)
                return 0;
            n = free_entries;
        }
        prod_next = prod_head + n;
        if (is_sp)
        {
            ring_list->p_head = prod_next;
            break;
        }
    }while (cel_atomic_cmp_and_swap(
        &(ring_list->p_head), prod_head, prod_next, 0) != prod_head);
    /* write entries in ring */
    if (handle == NULL)
        cel_ringlist_push_values(ring_list, prod_head, n, user_data);
    else
        handle(ring_list, prod_head, n, user_data);
    cel_compiler_barrier();
    if (!is_sp)
    {
        /*
        * If there are other enqueues in progress that preceded us,
        * we need to wait for them to complete
        */
        while (ring_list->p_tail != prod_head)
        {
            yield(rep++);
            /*if (rep > 4)
            printf("push yield, rep = %d, "
            "size %d, p_tail %d, prod_head %d\r\n",
            rep, ring_list->size, ring_list->p_tail, prod_head);*/
        }
    }
    ring_list->p_tail = prod_next;
    
    return n;
}

static __inline 
void cel_ringlist_pop_values(CelRingList *ring_list, 
                             unsigned long cons_head, size_t n, void **values)
{
    unsigned int i, idx;

    idx = cons_head & (ring_list->mask); 
    if (idx + n < ring_list->size) 
    { 
        for (i = 0; i < (n & ((~(unsigned)0x3))); i+=4, idx+=4) 
        {
            values[i] = ring_list->ring[idx]; 
            values[i + 1] = ring_list->ring[idx + 1]; 
            values[i + 2] = ring_list->ring[idx + 2]; 
            values[i + 3] = ring_list->ring[idx + 3]; 
        }
        switch (n & 0x3) 
        {
        case 3: values[i++] = ring_list->ring[idx++];
        case 2: values[i++] = ring_list->ring[idx++];
        case 1: values[i++] = ring_list->ring[idx++];
        }
    } 
    else 
    {
        for (i = 0; idx < ring_list->size; i++, idx++)
            values[i] = ring_list->ring[idx];
        for (idx = 0; i < n; i++, idx++)
            values[i] = ring_list->ring[idx];
    }
}

int _cel_ringlist_pop_do(CelRingList *ring_list, BOOL is_sp, size_t n, 
                         CelRingListProdFunc handle, void *user_data)
{
    U32 cons_head, prod_tail;
    U32 cons_next, entries;
    unsigned rep = 0;

    /* 
    * Avoid the unnecessary cmpset operation below, which is also
    * potentially harmful when n equals 0.
    */
    if (n == 0)
        return 0;
    /* move c_head atomically */
    do 
    {
        /* Restore n as it may change every loop */
        cons_head = ring_list->c_head;
        prod_tail = ring_list->p_tail;
        /* 
        * The subtraction is done between two unsigned 32bits value
        * (the result is always modulo 32 bits even if we have
        * cons_head > prod_tail). So 'entries' is always between 0
        * and size(ring)-1. 
        */
        entries = (prod_tail - cons_head);
        //printf("prod_tail = %d, cons_head = %d\r\n", prod_tail, cons_head);
        /* Set the actual entries for dequeue */
        if (n > entries) 
        {
            if (entries == 0)
                return 0;
            n = entries;
        }
        cons_next = cons_head + n;
        if (is_sp)
        {
            ring_list->c_head = cons_next;
            break;
        }
    }while(cel_atomic_cmp_and_swap(
        &(ring_list->c_head), cons_head, cons_next, 0) != cons_head);
    /* copy in table */
    if (handle == NULL)
        cel_ringlist_pop_values(ring_list, cons_head, n, user_data);
    else
        handle(ring_list, cons_head, n, user_data);
    cel_compiler_barrier();
    /*
    * If there are other dequeues in progress that preceded us,
    * we need to wait for them to complete
    */
    if (!is_sp)
    {
        while (ring_list->c_tail != cons_head)
        {
            yield(rep++);
            /*if (rep > 4)
            printf("pop yield, rep = %d c_tail %d c_head %d\r\n", 
            rep, ring_list->c_tail, cons_head);*/
        }
    }
    ring_list->c_tail = cons_next;

    return n;
}
