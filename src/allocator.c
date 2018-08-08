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
#include "cel/thread.h"
#include "cel/multithread.h"
#include "cel/error.h"
#undef _CEL_DEBUG
//#define _CEL_DEBUG
#include "cel/log.h"
#include "cel/list.h"
#include "cel/freelist.h"
#include "cel/radixtree.h"

size_t cel_capacity_get_min(size_t capacity)
{
    const size_t capacity_table[] =
    {
        11, 19, 37, 73, 109, 163, 251, 367, 557, 823, 1237, 1861, 2777, 4177,
        6247, 9371, 14057, 21089, 31627, 47431, 71143, 106721, 160073,  240101,
        360163, 540217, 810343, 1215497, 1823231, 2734867, 4102283, 6153409, 
        9230113, 13845163
    };
    int i, tb_size = sizeof(capacity_table) / sizeof(capacity_table[0]);

    for (i = 0; i < tb_size; i++)
    {
        if (capacity_table[i] > capacity)
        {
            return capacity_table[i];
        }
    }

    return capacity_table[tb_size - 1];
}

/* 
 * Cel allocator extern functions.
 */
CelMallocFunc _cel_sys_malloc = malloc;
CelFreeFunc _cel_sys_free = free;
CelReallocFunc _cel_sys_realloc = realloc;

CelMallocFunc cel_malloc = malloc;
CelFreeFunc cel_free = free;
CelReallocFunc cel_realloc = realloc;

//#define ALIGN_8BYTES
//#define PAGES_32K
//#define PAGES_64K
#define SMALL_BUT_SLOW

/* 8 or 16 */
#ifdef ALIGN_8BYTES
#define MIN_ALIGN                           8
#define BLOCKS_BASE_NUM                     16
#else
#define MIN_ALIGN                           16 
#define BLOCKS_BASE_NUM                     9
#endif

/* 15(32k pages), 16(64k pages) or 13 (8k pages) */
#ifdef PAGES_32K
#define PAGE_SHIFT                          15 
#define BLOCKS_NUM                          BLOCKS_BASE_NUM + 69
#elif defined PAGES_64K
#define PAGE_SHIFT                          16
#define BLOCKS_NUM                          BLOCKS_BASE_NUM + 73
#else
#define PAGE_SHIFT                          13
#define BLOCKS_NUM                          BLOCKS_BASE_NUM + 77
#endif

#define PAGE_SIZE                           (1 << PAGE_SHIFT)
#define MAX_PAGES                           (1 << (20 - PAGE_SHIFT))
#define MAX_BLOCK_SIZE                      256 * 1024
#define ALIGNMENT                           8
#define BLOCK_ARRAY_NUM  \
    (((MAX_BLOCK_SIZE + 127 + (120 << 7)) >> 7) + 1)
#define SLOTS_NUM                           64

#define MIN_SYSTEM_ALLOC_PAGES              MAX_PAGES

#ifdef SMALL_BUT_SLOW
#define CENTRAL_CACHE_MAX_SLOTS_NUM         0
#else
#define CENTRAL_CACHE_MAX_SLOTS_NUM         64
#endif

#define THREAD_CACHE_MIN_SIZE               (MAX_BLOCK_SIZE * 2)
#define THREAD_CACHE_MAX_SIZE               (4 << 20)
#ifdef SMALL_BUT_SLOW
#define THREAD_CACHE_DEFAULT_OVERALL_SIZE   THREAD_CACHE_MAX_SIZE
#else
#define THREAD_CACHE_DEFAULT_OVERALL_SIZE   (8u * THREAD_CACHE_MAX_SIZE)
#endif
#define THREAD_CACHE_STEAL_AMOUNT           (1<<16)

#define FREE_LIST_MAX_OVERAGES              3
#define FREE_LIST_MAX_DYNAMIC_SIZE          8192

/* With 4K pages, this comes to 4GB of deallocation.*/
#define HEAP_MAX_RELEASE_DELAY              (1 << 20)
/* With 4K pages, this comes to 1GB of memory. */
#define HEAP_DEFAULT_RELEASE_DELAY          (1 << 18)

enum { SPAN_IN_USE, SPAN_ON_NORMAL_FREELIST, SPAN_ON_RETURNED_FREELIST };

typedef struct _CelSpan
{
    union {
        CelListItem list_item;
        struct {
            struct _CelSpan *next, *prev;
        };
    };
    uintptr_t start;                /* Starting page number */
    unsigned int n_pages;           /* Number of pages in span */
    unsigned int ref_count:16;      /* Number of non-free objects */
    unsigned int index:8;           /* Block-index for small objects (or 0) */
    unsigned int location:2;        /* Is the span on a freelist, and if so, which? */
    //unsigned int sample:1;        /* Sampled object ? */
    CelBlock *blocks;               /* Linked list of free blocks */
}CelSpan;

typedef struct _CelSpanList 
{
    CelList normal_spans;
    CelList returned_spans;
}CelSpanList;

typedef struct _CelPageHeap
{
    CelSpinLock lock;
    int realeased_index;
    S64 scavenge_counter;
    U64 system_bytes;
    U64 free_bytes;
    U64 unmapped_bytes;
    U64 committed_bytes;
    CelSpanList large;
    CelSpanList free[MAX_PAGES];
    CelRadixTree page_map;
}CelPageHeap;

typedef struct _CelBlockSlot
{
    CelBlock *head;
    CelBlock *tail;
}CelBlockSlot;

typedef struct _CelCentralCache
{
    CelSpinLock lock;
    size_t index;                 /* Block size of map index */
    CelList empty_spans;          /* List of empty spans */
    CelList none_empty_spans;     /* List of non-empty spans*/
    size_t n_free_blocks;         /* Number of free blocks in cache */
    size_t n_allocated_blocks;    /* Number of allocated blocks from pageheap */
    size_t used_slots;            /* Number of currently used slots */
    CelBlockSlot slots[SLOTS_NUM];/* Cache slots */
    int cache_size;               /* The current number of slots for this size class.*/
    int max_cache_size;           /* Maximum size of the cache for a given size class. */
}CelCentralCache;

typedef struct _CelThreadCache
{
    union {
        CelListItem list_item;
        struct {
            struct _CelThreadCache *next, *prev;
        };
    };
    CelThreadId thread_id;
    size_t size, max_size;       /* Bytes size */
    CelFreeList free_list[BLOCKS_NUM];
}CelThreadCache;

static int s_block_array[BLOCK_ARRAY_NUM];
static int s_block_grow[BLOCKS_NUM];
static int s_block_to_size[BLOCKS_NUM];
static int s_block_to_pages[BLOCKS_NUM];
static CelPageHeap s_pageheap;
static CelCentralCache s_centralcache[BLOCKS_NUM];

static CelList s_threadcache_list;
static CelThreadCache *s_next_memory_steal = NULL;
static volatile size_t s_per_thread_cache_size = THREAD_CACHE_MAX_SIZE;
static int s_unclaimed_cache_space = THREAD_CACHE_DEFAULT_OVERALL_SIZE;
static BOOL s_init = FALSE;

static __inline size_t cel_sizemap_index(int s) 
{
    if (s < 1024)
        return (((s) + 7) >> 3);
    else
        return (((s) + 127 + (120 << 7)) >> 7);
}

/* Mapping from size cache to max size storable in that caches */
static __inline size_t cel_sizemap_size_to_block(size_t size)
{
    return s_block_array[cel_sizemap_index(size)];
}

static __inline size_t cel_sizemap_block_to_size(size_t index)
{
    return s_block_to_size[index];
}

static __inline size_t cel_sizemap_block_to_pages(size_t index)
{
    return s_block_to_pages[index];
}

static __inline size_t cel_sizemap_block_grow(size_t index)
{
    return s_block_grow[index];
}

static __inline int cel_sizemap_log_floor(size_t n) 
{
    int log = 0;
    int i;
    int shift;
    size_t x;

    for (i = 4; i >= 0; --i) 
    {
        shift = (1 << i);
        x = n >> shift;
        if (x != 0) 
        {
            n = x;
            log += shift;
        }
    }
    CEL_ASSERT(n == 1);
    return log;
}

int cel_sizemap_alignment_for_size(size_t size) 
{
  int alignment = ALIGNMENT;

  if (size > MAX_BLOCK_SIZE) 
  {
      /* Cap alignment at PageSize for large sizes. */
      alignment = PAGE_SIZE;
  } 
  else if (size >= 128) 
  {
      /* Space wasted due to alignment is at most 1/8, i.e., 12.5%. */
      alignment = (1 << cel_sizemap_log_floor(size)) / 8;
  }
  else if (size >= MIN_ALIGN) 
  {
      /* We need an alignment of at least 16 bytes to satisfy
         requirements for some SSE types. */
      alignment = MIN_ALIGN;
  }
  /* Maximum alignment allowed is page size alignment. */
  if (alignment > PAGE_SIZE) 
  {
      alignment = PAGE_SIZE;
  }
  CEL_ASSERT(size < MIN_ALIGN || alignment >= MIN_ALIGN);
  CEL_ASSERT((alignment & (alignment - 1)) == 0);
  return alignment;
}

int cel_sizemap_grow(size_t size) 
{
    int num;

    if (size == 0) 
        return 0;
    /* Use approx 64k transfers between thread and central caches. */
    num = (64 * 1024 / size);
    if (num < 2) 
        num = 2;
    if (num > 32) 
        num = 32;

    return num;
}

int cel_sizemap_init()
{
    /* Compute the size classes we want to use */
    int index = 1;   /* Next size class to assign */
    int alignment = ALIGNMENT;
    int grow;
    size_t psize;
    size_t my_pages;
    size_t my_objects;
    size_t prev_objects;
    size_t max_size_in_class;
    size_t next_size;
    size_t size;

    for (size = ALIGNMENT; size <= MAX_BLOCK_SIZE; size += alignment) 
    {
        alignment = cel_sizemap_alignment_for_size(size);
        CEL_ASSERT((size % alignment) == 0);
        grow = cel_sizemap_grow(size) / 4;
        psize = 0;
        do 
        {
            psize += PAGE_SIZE;
            /*
             * Allocate enough pages so leftover is less than 1/8 of total.
             * This bounds wasted space to at most 12.5%.
             */
            while ((psize % size) > (psize >> 3))
            {
                psize += PAGE_SIZE;
            }
            /*
             * Continue to add pages until there are at least as many objects 
             * in the span as are needed when moving objects from the central
             * freelists and spans to the thread caches.
             */
        }while ((int)(psize / size) < (grow));
        my_pages = psize >> PAGE_SHIFT;

        if (index > 1 && my_pages == (size_t)s_block_to_pages[index - 1]) 
        {
            /*
             * See if we can merge this into the previous class without
             * increasing the fragmentation of the previous class.
             */
            my_objects = (my_pages << PAGE_SHIFT) / size;
            prev_objects = (s_block_to_pages[index - 1] << PAGE_SHIFT)
                / s_block_to_size[index - 1];
            if (my_objects == prev_objects) 
            {
                /* Adjust last class to include this size */
                s_block_to_size[index - 1] = size;
                continue;
            }
        }
        /* Add new class */
        s_block_to_pages[index] = my_pages;
        s_block_to_size[index] = size;
        //_tprintf(_T("index %d, pages %d, size %d\r\n"), index, my_pages, size);
        index++;
    }
    CEL_ASSERT(index == BLOCKS_NUM);
    /* Initialize the mapping arrays */
    next_size = 0;
    for (index = 1; index < BLOCKS_NUM; index++) 
    {
        max_size_in_class = s_block_to_size[index];
        for (size = next_size; size <= max_size_in_class; size += ALIGNMENT) 
        {
            s_block_array[cel_sizemap_index(size)] = index;
            //_tprintf(_T("size %d, index %d\r\n"), size, index);
        }
        next_size = max_size_in_class + ALIGNMENT;
    }
    /* Initialize the num_objects_to_move array. */
    for (index = 1; index  < BLOCKS_NUM; ++index) 
    {
        s_block_grow[index] = cel_sizemap_grow(s_block_to_size[index]);
        //_tprintf(_T("index %d, grow %d\r\n"), index, s_block_grow[index]);
    }
    return 0;
}

static int cel_pageheap_init(CelPageHeap *page_heap)
{
    int i;

    cel_spinlock_init(&(page_heap->lock), 0);
    page_heap->realeased_index = MAX_PAGES;
    page_heap->scavenge_counter = 0;
    page_heap->free_bytes = page_heap->system_bytes = 0;
    page_heap->committed_bytes = page_heap->unmapped_bytes = 0;
    cel_list_init(&(page_heap->large.normal_spans), NULL);
    cel_list_init(&(page_heap->large.returned_spans), NULL);
    i = MAX_PAGES;
    while (--i >= 0)
    {
        cel_list_init(&(page_heap->free[i].normal_spans), NULL);
        cel_list_init(&(page_heap->free[i].returned_spans), NULL);
    }
    cel_radixtree_init(&(page_heap->page_map), 
        ADDRESS_BITS - PAGE_SHIFT, (ADDRESS_BITS - PAGE_SHIFT)/3, NULL);
    cel_radixtree_set_node_allocator(&(page_heap->page_map), 
        _cel_sys_malloc, _cel_sys_free);

    return 0;
}

void cel_pageheap_push_span(CelPageHeap *page_heap, CelSpan *span)
{
    CelSpanList *span_list = (span->n_pages < MAX_PAGES 
        ? &(page_heap->free[span->n_pages]) : &(page_heap->large));
    if (span->location == SPAN_ON_NORMAL_FREELIST)
    {
        (page_heap->free_bytes) += (span->n_pages << PAGE_SHIFT);
        cel_list_push_front(&(span_list->normal_spans), (CelListItem *)span);
    }
    else
    {
        (page_heap->unmapped_bytes) += (span->n_pages << PAGE_SHIFT);
        cel_list_push_front(&(span_list->returned_spans), (CelListItem *)span);
    }
}

void cel_pageheap_remove_span(CelPageHeap *page_heap, CelSpan *span)
{
    CelSpanList *span_list = (span->n_pages < MAX_PAGES 
        ? &(page_heap->free[span->n_pages]) : &(page_heap->large));

    if (span->location == SPAN_ON_NORMAL_FREELIST)
    {
        (page_heap->free_bytes) -= (span->n_pages << PAGE_SHIFT);
        cel_list_remove(&(span_list->normal_spans), (CelListItem *)span);
    }
    else
    {
        (page_heap->unmapped_bytes) -= (span->n_pages << PAGE_SHIFT);
        cel_list_remove(&(span_list->returned_spans), (CelListItem *)span);
    }
}

static void cel_pageheap_merge_into_freelist(CelPageHeap *page_heap, 
                                             CelSpan *span)
{
    uintptr_t page_id;
    unsigned int n_pages;
    CelSpan *prev, *next;

    page_id = span->start;
    n_pages = span->n_pages;
    if ((prev = (CelSpan *)cel_radixtree_get(
        &(page_heap->page_map), page_id - 1)) != NULL
        && prev->location == span->location)
    {
        CEL_DEBUG((_T("Merge span %p-0x%x(%d), prev %p-0x%x(%d)"), 
            span, span->start, span->n_pages, prev, 
            prev->start, prev->n_pages));
        /* Remove from free list */
        cel_pageheap_remove_span(page_heap, prev);

        span->start -= prev->n_pages;
        span->n_pages += prev->n_pages;
        cel_radixtree_set(&(page_heap->page_map), span->start, span);
        _cel_sys_free(prev);
    }
    if ((next = (CelSpan *)cel_radixtree_get(
        &(page_heap->page_map), page_id + n_pages)) != NULL
        && next->location == span->location)
    {
        CEL_DEBUG((_T("Merge span %p-0x%x(%d), next %p-0x%x(%d)"), 
            span, span->start, span->n_pages, next, 
            next->start, next->n_pages));
        /* Remove from free list */
        cel_pageheap_remove_span(page_heap, next);

        span->n_pages += next->n_pages;
        cel_radixtree_set(&(page_heap->page_map), 
            span->start + span->n_pages - 1, span);
        _cel_sys_free(next);
    }
    CEL_DEBUG((_T("Add span %p-0x%x(%d) to freelist"), span, 
        span->start, span->n_pages));
    cel_pageheap_push_span(page_heap, span);
}

void cel_pageheap_destroy(CelPageHeap *page_heap)
{
    int i;
    CelSpan *span;

    while ((span = (CelSpan *)cel_list_pop_front(
        &(page_heap->large.normal_spans))) != NULL)
    {
        if (cel_system_release((void *)(span->start << PAGE_SHIFT),
            span->n_pages << PAGE_SHIFT))
        {
            page_heap->committed_bytes -= (span->n_pages << PAGE_SHIFT);
            span->location = SPAN_ON_RETURNED_FREELIST;
            cel_pageheap_merge_into_freelist(page_heap, span);
        }
    }
    i = MAX_PAGES;
    while (--i >= 0)
    {
        while ((span = (CelSpan *)cel_list_pop_front(
            &(page_heap->free[i].normal_spans))) != NULL)
        {
            if (cel_system_release((void *)(span->start << PAGE_SHIFT),
                span->n_pages << PAGE_SHIFT))
            {
                page_heap->committed_bytes -= (span->n_pages << PAGE_SHIFT);
                span->location = SPAN_ON_RETURNED_FREELIST;
                cel_pageheap_merge_into_freelist(page_heap, span);
            }
        }
    }
    cel_radixtree_destroy(&(page_heap->page_map));
}

CelSpan *cel_pageheap_search_free_and_large_list(CelPageHeap *page_heap,
                                                 size_t n_pages)
{
    int i;
    CelSpan *span, *tail, *best = NULL;
    CelSpanList *span_list;

    /* Find first size >= n that has a non-empty list */
    for (i = n_pages; i < MAX_PAGES; i++)
    {
        span_list = &(page_heap->free[i]);
        if (!cel_list_is_empty(&(span_list->normal_spans)))
        {
            span = (CelSpan *)span_list->normal_spans.head.next;
            CEL_ASSERT(span->location == SPAN_ON_NORMAL_FREELIST);
            return (CelSpan *)span_list->normal_spans.head.next;
        }
        if (!cel_list_is_empty(&(span_list->returned_spans)))
        {
            span = (CelSpan *)span_list->returned_spans.head.next;
            CEL_ASSERT(span->location == SPAN_ON_RETURNED_FREELIST);
            return (CelSpan *)span_list->returned_spans.head.next;
        }
    }
    /* Search through normal_spans list */
    span = (CelSpan *)cel_list_get_front(&(page_heap->large.normal_spans));
    tail = (CelSpan *)cel_list_get_tail(&(page_heap->large.normal_spans));
    for (; span != tail; span = span->next)
    {
        if (span->n_pages >= n_pages)
        {
            if ((best == NULL)
                || span->n_pages < best->n_pages
                || (span->n_pages == best->n_pages 
                && span->start < best->start))
            {
                best = span;
                CEL_ASSERT(best->location == SPAN_ON_NORMAL_FREELIST);
            }
        }
    }
    /* Search through released list in case it has a better fit */
    span = (CelSpan *)cel_list_get_front(&(page_heap->large.returned_spans));
    tail = (CelSpan *)cel_list_get_tail(&(page_heap->large.returned_spans));
    for (; span != tail; span = span->next)
    {
        if (span->n_pages >= n_pages)
        {
            if ((best == NULL)
                || span->n_pages < best->n_pages
                || (span->n_pages == best->n_pages 
                && span->start < best->start))
            {
                best = span;
                CEL_ASSERT(best->location == SPAN_ON_RETURNED_FREELIST);
            }
        }
    }

    return best;
}

int cel_pageheap_grow(CelPageHeap *page_heap, size_t n_pages)
{
    unsigned int ask;
    size_t actual_size;
    char *ptr;
    CelSpan *span;

    //_tprintf(_T("num %d")CEL_CRLF, num);
    ask = (n_pages > MIN_SYSTEM_ALLOC_PAGES 
        ? n_pages : MIN_SYSTEM_ALLOC_PAGES);
    if ((ptr = (char *)cel_system_alloc(
        ask << PAGE_SHIFT, &actual_size, PAGE_SIZE)) == NULL)
    {
        CEL_ERR((_T("New span memory failed(%s)."),
            cel_geterrstr(cel_sys_geterrno())));
        return -1;
    }
    ask = actual_size >> PAGE_SHIFT;
    page_heap->committed_bytes += (ask << PAGE_SHIFT);
    page_heap->system_bytes += (ask << PAGE_SHIFT);
    if ((span = (CelSpan *)_cel_sys_malloc(sizeof(CelSpan))) == NULL)
    {
        CEL_ERR((_T("New span failed(%s)."), 
            cel_geterrstr(cel_sys_geterrno())));
        munmap(ptr, n_pages << PAGE_SHIFT);
        return -1;
    }
    span->next = span->prev = NULL;
    span->start = ((uintptr_t)ptr >> PAGE_SHIFT);
    span->n_pages = ask;
    span->ref_count = 0;
    span->index = 0;
    span->blocks = NULL;
    span->location = SPAN_ON_NORMAL_FREELIST;
    cel_radixtree_set(&(page_heap->page_map), span->start, span);
    if (span->n_pages > 1)
        cel_radixtree_set(&(page_heap->page_map), 
        span->start + span->n_pages - 1, span);
    CEL_DEBUG((_T("New span %p-0x%x(%d x %d = %d)"),
        span, span->start, (int)ask, (int)PAGE_SIZE, (int)actual_size));
    cel_pageheap_push_span(page_heap, span);

    return 0;
}

CelSpan *cel_pageheap_carve(CelPageHeap *page_heap, 
                            CelSpan *span, size_t n_pages)
{
    int old_location, extra;
    CelSpan *leftover;

    old_location = span->location;
    cel_pageheap_remove_span(page_heap, span);
    span->location = SPAN_IN_USE;
    if ((extra = span->n_pages - n_pages) > 0)
    {
        leftover = (CelSpan *)_cel_sys_malloc(sizeof(CelSpan));
        leftover->next = leftover->prev = NULL;
        leftover->start = span->start + n_pages;
        leftover->n_pages = extra;
        leftover->ref_count = 0;
        leftover->index = 0;
        leftover->blocks = NULL;
        leftover->location = old_location;
        cel_radixtree_set(&(page_heap->page_map), leftover->start, leftover);
        if (leftover->n_pages > 1)
            cel_radixtree_set(&(page_heap->page_map), 
            leftover->start + leftover->n_pages - 1, leftover);
        /* Prepend to free list */
        cel_pageheap_push_span(page_heap, leftover);

        /* Update return span */
        span->n_pages = n_pages;
        span->ref_count = 0;
        span->index = 0;
        cel_radixtree_set(&(page_heap->page_map), 
            span->start + span->n_pages - 1, span);
        CEL_DEBUG((_T("Carve 2 spans %p-0x%x(%d) %p-0x%x(%d)."),
            span, span->start, (int)n_pages, 
            leftover, leftover->start, (int)extra));
    }
    if (old_location == SPAN_ON_RETURNED_FREELIST)
    {
        CEL_DEBUG((_T("Commit span %p-0x%x"),
            span, span->start, (int)span->n_pages));
        cel_system_commit((void *)(span->start << PAGE_SHIFT),
            span->n_pages << PAGE_SHIFT);
        page_heap->committed_bytes += (span->n_pages << PAGE_SHIFT);
    }

    return span;
}

void cel_pageheap_register_block(CelPageHeap *page_heap, 
                                 CelSpan *span, int block_index)
{
    unsigned int i;

    span->index = block_index;
    for (i = 1; i < span->n_pages - 1; i++)
        cel_radixtree_set(&(page_heap->page_map), span->start + i, span);
}

CelSpan *cel_pageheap_allocate(CelPageHeap *page_heap, int n_pages)
{
    CelSpan *span;

    cel_spinlock_lock(&(page_heap->lock));
    if ((span = cel_pageheap_search_free_and_large_list(
        page_heap, n_pages)) == NULL)
    {
        if (cel_pageheap_grow(page_heap, n_pages) == -1)
        {
            cel_spinlock_unlock(&(page_heap->lock));
            return NULL;
        }
        if ((span = cel_pageheap_search_free_and_large_list(
            page_heap, n_pages)) == NULL)
        {
            cel_spinlock_unlock(&(page_heap->lock));
            CEL_ERR((_T("Page heap search free[%d] and large list return null."),
                n_pages));
            return NULL;
        }
    }
    span = cel_pageheap_carve(page_heap, span, n_pages);
    CEL_DEBUG((_T("Allocate span %p-0x%x(%d), n_pages %d"),
        span, span->start, span->n_pages, n_pages));
    cel_spinlock_unlock(&(page_heap->lock));

    return span;
}

int cel_pageheap_release_pages(CelPageHeap *page_heap, int n_pages)
{
    int released_pages = 0, released_length;
    int i;
    CelSpanList *span_list;
    CelSpan *span;

    while (released_pages < n_pages && page_heap->free_bytes > 0)
    {
        for (i = 0; 
            i < MAX_PAGES + 1 && released_pages < n_pages;
            i++, (page_heap->realeased_index)++ )
        {
            if (page_heap->realeased_index > MAX_PAGES)
                page_heap->realeased_index = 0;
            span_list = ((page_heap->realeased_index == MAX_PAGES)
                ? &(page_heap->large) 
                : &(page_heap->free[page_heap->realeased_index]));
            if (!cel_list_is_empty(&(span_list->normal_spans)))
            {
                span = (CelSpan *)span_list->normal_spans.head.next;
                if (cel_system_release((void *)(span->start << PAGE_SHIFT),
                    span->n_pages << PAGE_SHIFT))
                {
                    page_heap->committed_bytes -= 
                        (span->n_pages << PAGE_SHIFT);
                    cel_pageheap_remove_span(page_heap, span);
                    released_length = span->n_pages;
                    span->location = SPAN_ON_RETURNED_FREELIST;
                    cel_pageheap_merge_into_freelist(page_heap, span);
                }
                if (released_length == 0)
                    return released_pages;
                released_pages += released_length;
            }
        }
    }
    return released_pages;
}

void cel_pageheap_incremental_scavenge(CelPageHeap *page_heap, int n_pages)
{
    double rate = 1, mult, wait;
    int released_pages;

    (page_heap->scavenge_counter) -= n_pages;
    if (page_heap->scavenge_counter > 0)
        return;
    released_pages = cel_pageheap_release_pages(page_heap, 1);
    if (released_pages == 0)
    {
        page_heap->scavenge_counter = HEAP_DEFAULT_RELEASE_DELAY;
    }
    else
    {
        mult = 1000.0 / rate;
        wait = mult * released_pages;
        if (wait > HEAP_MAX_RELEASE_DELAY)
            wait = HEAP_MAX_RELEASE_DELAY;
        page_heap->scavenge_counter = (S64)wait;
    }
}

void cel_pageheap_deallocate(CelPageHeap *page_heap, CelSpan *span)
{
    int n;
    
    n = span->n_pages;
    cel_spinlock_lock(&(page_heap->lock));
    span->location = SPAN_ON_NORMAL_FREELIST;
    cel_pageheap_merge_into_freelist(page_heap, span);
    cel_pageheap_incremental_scavenge(page_heap, n);
    cel_spinlock_unlock(&(page_heap->lock));
}

static int cel_centralcache_init(CelCentralCache *central_cache, size_t index)
{
    size_t bytes, grow, new_size;

    cel_spinlock_init(&(central_cache->lock), 0);
    central_cache->index = index;
    cel_list_init(&(central_cache->none_empty_spans), NULL);
    cel_list_init(&(central_cache->empty_spans), NULL);
    central_cache->used_slots = 0;
    central_cache->n_free_blocks = 0;
    central_cache->n_allocated_blocks = 0;
    central_cache->max_cache_size = CENTRAL_CACHE_MAX_SLOTS_NUM;
#ifdef SMALL_BUT_SLOW
    /* Disable the transfer cache for the small footprint case. */
    central_cache->cache_size = 0;
#else
    central_cache->cache_size = 16;
#endif
    if (index > 0)
    {
        bytes = cel_sizemap_block_to_size(index);
        grow = cel_sizemap_block_grow(index);
        new_size = 1024 * 1024 / (bytes * grow);
        if (new_size < 1)
            new_size = 1;
        if ((int)new_size < central_cache->max_cache_size)
            central_cache->max_cache_size = new_size;
        if (central_cache->max_cache_size < central_cache->cache_size)
            central_cache->cache_size = central_cache->max_cache_size;
    }

    return 0;
}

void cel_centralcache_release_to_spans(CelCentralCache *central_cache, 
                                       CelBlock *block)
{
    CelSpan *span;
    size_t num;

    if ((span = (CelSpan *)cel_radixtree_get(
        &(s_pageheap.page_map), (uintptr_t)block >> PAGE_SHIFT)) == NULL)
    {
        CEL_ERR((_T("Error cel_centralcache_release_to_spans")));
        return;
    }
    if (span->blocks == NULL)
    {
        cel_list_remove(&(central_cache->empty_spans), (CelListItem *)span);
        cel_list_push_front(&(central_cache->none_empty_spans), (CelListItem *)span);
    }
    (central_cache->n_free_blocks)++;
    (span->ref_count)--;
    if (span->ref_count == 0)
    {
        CEL_DEBUG((_T("span %p ref count %d"), span, span->ref_count));
        num = (span->n_pages << PAGE_SHIFT) 
            / cel_sizemap_block_to_size(central_cache->index);
        (central_cache->n_free_blocks) -= num;
        (central_cache->n_allocated_blocks) -= num;
        cel_list_remove(&(central_cache->none_empty_spans), (CelListItem *)span);
        /* Release central list lock while operating on pageheap */
        cel_spinlock_unlock(&central_cache->lock);
        cel_pageheap_deallocate(&s_pageheap, span);
        cel_spinlock_lock(&central_cache->lock);
    }
    else
    {
        block->next = span->blocks;
        span->blocks = block;
    }
}

void cel_centralcache_destroy(CelCentralCache *central_cache)
{
    CelBlock *start, *next;
    
    while (central_cache->used_slots > 0)
    {
        (central_cache->used_slots)--;
        (central_cache->cache_size)--;
        start = central_cache->slots[central_cache->used_slots].head;
        while (start != NULL)
        {
            next = start->next;
            cel_centralcache_release_to_spans(central_cache, start);
            start = next;
        }
    }
    CEL_DEBUG((_T("Central cache[%d] free, slots num %d, blocks num %d, "
        "emty size %d none_empty_spans size %d."), 
        central_cache->index, 
        central_cache->used_slots,
        central_cache->n_free_blocks, 
        central_cache->empty_spans.size, 
        central_cache->none_empty_spans.size));
    CEL_ASSERT(cel_list_is_empty(&(central_cache->empty_spans)) 
        && cel_list_is_empty(&(central_cache->none_empty_spans)));
    cel_list_destroy(&(central_cache->none_empty_spans));
    cel_list_destroy(&(central_cache->empty_spans));
}

CelSpan *cel_centralcache_fetch_from_pageheap(CelCentralCache *central_cache)
{
    size_t n_pages, block_size, num_block;
    CelSpan *span;
    CelBlock *new_block;
    char *limit;

    cel_spinlock_unlock(&(central_cache->lock));
    if ((n_pages = cel_sizemap_block_to_pages(central_cache->index)) <= 0
        || (span = cel_pageheap_allocate(&s_pageheap, n_pages)) == NULL)
    {
        cel_spinlock_lock(&(central_cache->lock));
        return NULL;
    }
    CEL_ASSERT(span->n_pages == n_pages);
    cel_pageheap_register_block(&s_pageheap, span, central_cache->index);
    /* Split the block into pieces and add to the free-list */
    span->blocks = NULL;
    block_size = cel_sizemap_block_to_size(central_cache->index);
    num_block = 0;
    new_block = (CelBlock *)(span->start << PAGE_SHIFT);
    limit = (char *)new_block + (span->n_pages << PAGE_SHIFT);
    //_tprintf(_T("%p %p")CEL_CRLF, new_block, limit);
    while ((char *)new_block + block_size <= limit)
    {
        new_block->next = span->blocks;
        span->blocks = new_block;
        new_block = (CelBlock *)((char *)new_block + block_size);
        num_block++;
    }
    CEL_ASSERT((char *)new_block <= limit);
    span->ref_count = 0;
    CEL_DEBUG((_T("Span 0x%x split %d blocks(%d)"),
        span->start, (int)num_block, block_size));
    cel_spinlock_lock(&(central_cache->lock));
    central_cache->n_free_blocks += num_block;
    central_cache->n_allocated_blocks += num_block;

    return span;
}

CelBlock *cel_centralcache_fetch_from_span(CelCentralCache *central_cache)
{
    CelSpan *span;
    CelBlock *free_block;

    if (cel_list_is_empty(&(central_cache->none_empty_spans)))
    {
        if ((span = cel_centralcache_fetch_from_pageheap(
            central_cache)) == NULL)
            return NULL;
        cel_list_push_front(&(central_cache->none_empty_spans), (CelListItem *)span);
    }
    span = (CelSpan *)cel_list_get_front(&(central_cache->none_empty_spans));
    (span->ref_count)++;
    CEL_ASSERT(span->blocks != NULL);
    free_block = span->blocks;
    if ((span->blocks = (free_block->next)) == NULL)
    {
        /* Move to empty list */
        cel_list_remove(&(central_cache->none_empty_spans),(CelListItem *)span);
        cel_list_push_front(&(central_cache->empty_spans), (CelListItem *)span);
    }
    return free_block;
}

int cel_centralcache_allocate(CelCentralCache *central_cache, 
                              CelBlock **start, CelBlock **end, int num)
{
    int ret;
    CelBlock *fetch_block;

    cel_spinlock_lock(&(central_cache->lock));
    if (central_cache->used_slots > 0)
    {
        --(central_cache->used_slots);
        *start = central_cache->slots[central_cache->used_slots].head;
        *end = central_cache->slots[central_cache->used_slots].tail;
        cel_spinlock_unlock(&central_cache->lock);
        return num;
    }
    ret = 0;
    *start = *end = NULL;
    if (((*end) = cel_centralcache_fetch_from_span(central_cache)) != NULL)
    {
        ret = 1;
        (*end)->next = NULL;
        (*start) = (*end);
        while (ret < num)
        {
            if ((fetch_block = 
                cel_centralcache_fetch_from_span(central_cache)) == NULL)
                break;
            fetch_block->next = (*start);
            (*start) = fetch_block;
            ret++;
            //printf("start %p next %p\r\n", *start, (*start)->next);
        }
    }
    central_cache->n_free_blocks -= ret;
    cel_spinlock_unlock(&(central_cache->lock));

    return ret;
}

BOOL cel_centralcache_shrink_cache(CelCentralCache *central_cache, 
                                   int locked_index, BOOL force)
{
    BOOL b_ret;
    CelBlock *start, *next;

    if (central_cache->cache_size == 0)
        return FALSE;
    if (force == FALSE 
        && central_cache->used_slots == (size_t)central_cache->cache_size)
        return FALSE;
    if (central_cache->cache_size == 0)
        return FALSE;
    // lock inverter
    //cel_spinlock_unlock(&(s_centralcache[locked_index].lock));
    //cel_spinlock_lock(&(central_cache->lock));
    if (central_cache->used_slots == (size_t)central_cache->cache_size)
    {
        if (force == FALSE)
        {
            b_ret = FALSE;
        }
        else
        {
            (central_cache->cache_size)--;
            (central_cache->used_slots)--;
            start = central_cache->slots[central_cache->used_slots].head;
            while (start != NULL)
            {
                next = start->next;
                cel_centralcache_release_to_spans(central_cache, start);
                start = next;
            }
            b_ret = TRUE;
        }
    }
    else
    {
        (central_cache->cache_size)--;
        b_ret = TRUE;
    }
    //cel_spinlock_unlock(&(central_cache->lock));
    //cel_spinlock_lock(&(s_centralcache[locked_index].lock));

    return b_ret;
}

BOOL cel_centralcache_evict_random(CelCentralCache *central_cache, 
                                   int locked_index, BOOL force)
{
    static int race_counter = 0;
    int t = race_counter++;
    if (t > BLOCKS_NUM)
    {
        while (t >= BLOCKS_NUM)
            t -= BLOCKS_NUM;
        race_counter = t;
    }
    if (t == locked_index) 
        return FALSE;
    return cel_centralcache_shrink_cache(central_cache, locked_index, force);
}

BOOL cel_centralcache_make_cache_space(CelCentralCache *central_cache)
{
    if ((int)central_cache->used_slots < central_cache->cache_size)
        return TRUE;
    if (central_cache->cache_size == central_cache->max_cache_size)
        return FALSE;
    if (cel_centralcache_evict_random(
        central_cache, central_cache->index, FALSE)
        || cel_centralcache_evict_random(
        central_cache, central_cache->index, TRUE))
    {
        if (central_cache->cache_size < central_cache->max_cache_size)
        {
            central_cache->cache_size++;
            return TRUE;
        }
    }
    return FALSE;
}

int cel_centralcache_deallocate(CelCentralCache *central_cache, 
                                CelBlock *start, CelBlock *end, int num)
{
    CelBlock *next;

    CEL_DEBUG((_T("Centralcache %d deallocate start %p, end %p, num %d"), 
        central_cache->index, start, end, num));
    cel_spinlock_lock(&(central_cache->lock));
    if ((size_t)num == cel_sizemap_block_grow(central_cache->index)
        && cel_centralcache_make_cache_space(central_cache))
    {
        central_cache->slots[central_cache->used_slots].head = start;
        central_cache->slots[central_cache->used_slots].tail = end;
        central_cache->used_slots++;
        cel_spinlock_unlock(&(central_cache->lock));
        return num;
    }
    while (start != NULL)
    {
        next = start->next;
        cel_centralcache_release_to_spans(central_cache, start);
        start = next;
        num--;
    }
    CEL_ASSERT(num == 0);
    cel_spinlock_unlock(&central_cache->lock);

    return 0;
}

void cel_threadcache_moudle_init(void)
{
    cel_list_init(&s_threadcache_list, NULL);
    s_init = TRUE;
}

void cel_threadcache_increase_cache_limit(CelThreadCache *thread_cache)
{
    int i;

    if (s_unclaimed_cache_space > 0) 
    {
        // Possibly make unclaimed_cache_space_ negative.
        s_unclaimed_cache_space -= THREAD_CACHE_STEAL_AMOUNT;
        thread_cache->max_size += THREAD_CACHE_STEAL_AMOUNT;
        return;
    }
    for (i = 0; i < 10; ++i, s_next_memory_steal = s_next_memory_steal->next)
    {
        // Reached the end of the linked list.  Start at the beginning.
        if (s_next_memory_steal == NULL) 
            s_next_memory_steal = 
            (CelThreadCache *)cel_list_get_front(&s_threadcache_list);
        if (s_next_memory_steal == thread_cache 
            || s_next_memory_steal->max_size <= THREAD_CACHE_MIN_SIZE)
                continue;
        s_next_memory_steal->max_size -= THREAD_CACHE_STEAL_AMOUNT;
        thread_cache->max_size += THREAD_CACHE_STEAL_AMOUNT;
        s_next_memory_steal = s_next_memory_steal->next;
        return;
    }
}

static CelThreadCache *cel_threadcache_new(void)
{
    int i;
    CelThreadCache *thread_cache;

    if ((thread_cache = 
        (CelThreadCache *)_cel_sys_malloc(sizeof(CelThreadCache))) != NULL)
    {
        cel_spinlock_lock(&(s_pageheap.lock));
        thread_cache->size = 0;
        thread_cache->max_size = 0;
        cel_threadcache_increase_cache_limit(thread_cache);
        if (thread_cache->max_size == 0)
        {
            thread_cache->max_size = THREAD_CACHE_MIN_SIZE;
            s_unclaimed_cache_space = THREAD_CACHE_MIN_SIZE;
        }
        i = BLOCKS_NUM;
        while ((--i) >= 0)
        {
            cel_freelist_init(&(thread_cache->free_list[i]));
        }
        if (cel_list_is_empty(&s_threadcache_list))
            s_next_memory_steal = thread_cache;
        cel_list_push_front(&s_threadcache_list, (CelListItem *)thread_cache);
        cel_spinlock_unlock(&(s_pageheap.lock));
        CEL_DEBUG((_T("New thread(tid:%ld) caches %p."), 
            cel_thread_getid(), thread_cache));
        return thread_cache;
    }
    CEL_DEBUG((_T("New thread(tid:%ld) caches retun null."),
        cel_thread_getid()));

    return NULL;
}

void cel_threadcache_release_blocks(CelThreadCache *thread_cache, 
                                    CelFreeList *free_list, int index, int num)
{
    int grow_size, delta_bytes;
    CelBlock *start, *end;

    if (num > (int)cel_freelist_get_size(free_list))
        num = cel_freelist_get_size(free_list);
    delta_bytes = num * cel_sizemap_block_to_size(index);
    grow_size = cel_sizemap_block_grow(index);
    while (num > grow_size)
    {
        cel_freelist_pop_range(free_list, &start, &end, grow_size);
        cel_centralcache_deallocate(&s_centralcache[index],
            start, end, grow_size);
        num -= grow_size;
    }
    if (num > 0)
    {
        cel_freelist_pop_range(free_list, &start, &end, num);
        cel_centralcache_deallocate(&s_centralcache[index], start, end, num);
    }
    thread_cache->size -= delta_bytes;
    CEL_DEBUG((_T("Thread(tid:%ld) releas blocks %d * %d, size %d, max_size %d"), 
        cel_thread_getid(), num, (int)cel_sizemap_block_to_size(index),
        thread_cache->size, thread_cache->max_size));
}

static void cel_threadcache_free(CelThreadCache *thread_cache)
{
    int i = BLOCKS_NUM;;

    while ((--i) >= 0)
    {
        if (cel_freelist_get_size(&(thread_cache->free_list[i])) > 0)
        {
            //printf("i = %d\r\n", i);
            cel_threadcache_release_blocks(thread_cache,
                &(thread_cache->free_list[i]), i,
                cel_freelist_get_size(&(thread_cache->free_list[i])));
        }
        cel_freelist_destroy(&(thread_cache->free_list[i]));
    }
    CEL_DEBUG((_T("Thread(tid:%ld) caches free, size %d."), 
        cel_thread_getid(), thread_cache->size));
    cel_spinlock_lock(&(s_pageheap.lock));
    cel_list_remove(&s_threadcache_list, (CelListItem *)thread_cache);
    if (s_next_memory_steal == thread_cache)
        s_next_memory_steal = thread_cache->next;
    if (s_next_memory_steal == NULL)
        s_next_memory_steal = 
        (CelThreadCache *)cel_list_get_front(&s_threadcache_list);
    s_unclaimed_cache_space += thread_cache->max_size;
    cel_spinlock_unlock(&(s_pageheap.lock));
    _cel_sys_free(thread_cache);
}

static CelThreadCache *cel_threadcache_get(void)
{
    CelThreadCache *thread_cache;

    if ((thread_cache = (CelThreadCache *)
        cel_multithread_get_keyvalue(CEL_MT_KEY_ALLOCATOR)) == NULL)
    {
        if ((thread_cache = cel_threadcache_new()) != NULL)
        {
            if (cel_multithread_set_keyvalue(
                CEL_MT_KEY_ALLOCATOR, thread_cache) != -1
                && cel_multithread_set_keydestructor(
                CEL_MT_KEY_ALLOCATOR, 
                (CelDestroyFunc)cel_threadcache_free) != -1)
            {
                return thread_cache;
            }
            cel_threadcache_free(thread_cache);
        }
        CEL_DEBUG((_T("Get thread(tid:%ld) caches return null."), 
            cel_thread_getid()));
        return NULL;
    }

    return thread_cache;
}

CelBlock *cel_threadcache_allocate(CelThreadCache *thread_cache, size_t index)
{
    size_t grow_size, allocate_num, block_size, new_size;
    CelFreeList *free_list;
    CelBlock *start, *end;

    free_list = &(thread_cache->free_list[index]);
    block_size = cel_sizemap_block_to_size(index);
    if (cel_freelist_is_empty(free_list))
    {
        grow_size = cel_sizemap_block_grow(index);
        if (grow_size > free_list->max_size)
            grow_size = free_list->max_size;
        if ((allocate_num = cel_centralcache_allocate(
            &s_centralcache[index], &start, &end, grow_size)) <= 0)
        {
            return NULL;
        }
        CEL_DEBUG((_T("Thread(tid:%ld) fetch %d[%p-%p] blocks from central cache."), 
            cel_thread_getid(), (int)allocate_num, start, end));
        if ((--allocate_num) >= 0)
        {
            cel_freelist_push_range(free_list, start->next, end, allocate_num);
            thread_cache->size += (allocate_num * block_size);
        }
        if (cel_freelist_get_max_size(free_list) < grow_size)
        {
            cel_freelist_set_max_size(free_list,
            cel_freelist_get_max_size(free_list) + 1);
        }
        else
        {
            new_size = cel_freelist_get_max_size(free_list) + grow_size;
            if (new_size > FREE_LIST_MAX_DYNAMIC_SIZE)
                new_size = FREE_LIST_MAX_DYNAMIC_SIZE;
            new_size -= new_size % grow_size;
            cel_freelist_set_max_size(free_list, new_size);
        }
    }
    else
    {
        //CEL_DEBUG((_T("Thread(tid:%ld) fetch block size %d from free list."), 
        //cel_thread_getid(), (int)block_size));
        start = cel_freelist_pop(free_list);
        (thread_cache->size) -= block_size;
    }

    return start;
}

void cel_threadcache_freelist_too_long(CelThreadCache *thread_cache,
                                       CelFreeList *free_list, int index)
{
    size_t grow_size;

    //puts("cel_threadcache_freelist_too_long");
    grow_size = cel_sizemap_block_grow(index);
    cel_threadcache_release_blocks(thread_cache, free_list, index, grow_size);
    if (cel_freelist_get_size(free_list) < grow_size)
        cel_freelist_set_max_size(free_list, 
        cel_freelist_get_max_size(free_list) + 1);
    else if (cel_freelist_get_max_size(free_list) > grow_size)
    {
        cel_freelist_set_overages(
            free_list, cel_freelist_get_overages(free_list) + 1);
        if (cel_freelist_get_overages(free_list) > FREE_LIST_MAX_OVERAGES)
        {
            cel_freelist_set_max_size(free_list,
                cel_freelist_get_max_size(free_list) - grow_size);
            cel_freelist_set_overages(free_list, 0);
        }
    }
}

void cel_threadcache_sacvenge(CelThreadCache *thread_cache)
{
    int index, lowmark, drop;
    size_t grow_size, new_size;
    CelFreeList *free_list;

    //puts("cel_threadcache_sacvenge");
    for (index = 0; index < BLOCKS_NUM; index++)
    {
        free_list = &(thread_cache->free_list[index]);
        if ((lowmark = cel_freelist_get_lowater(free_list)) > 0)
        {
            drop = (lowmark > 1 ? (lowmark / 2) : 1);
            cel_threadcache_release_blocks(
                thread_cache, free_list, index, drop);
            grow_size = cel_sizemap_block_grow(index);
            if (cel_freelist_get_size(free_list) > grow_size)
            {
                new_size = cel_freelist_get_max_size(free_list) - grow_size;
                if (new_size < grow_size)
                    new_size = grow_size;
                cel_freelist_set_max_size(free_list, new_size);
            }
        }
        cel_freelist_clear_lowater(free_list);
    }
    cel_spinlock_lock(&(s_pageheap.lock));
    cel_threadcache_increase_cache_limit(thread_cache);
    cel_spinlock_unlock(&(s_pageheap.lock));
}

void cel_threadcache_deallocate(CelThreadCache *thread_cache, 
                                CelBlock *free_block, size_t index)
{
    size_t block_size;
    int size_head_room, list_head_room;
    CelFreeList *free_list;

    block_size = cel_sizemap_block_to_size(index);
    free_list = &(thread_cache->free_list[index]);
    thread_cache->size += block_size;
    size_head_room = thread_cache->max_size - thread_cache->size - 1;
    cel_freelist_push(free_list, free_block);
    list_head_room = cel_freelist_get_max_size(free_list) 
        - cel_freelist_get_size(free_list);
    if ((size_head_room | list_head_room) < 0)
    {
        if (list_head_room < 0)
            cel_threadcache_freelist_too_long(thread_cache, free_list, index);
        if (thread_cache->size > thread_cache->max_size)
            cel_threadcache_sacvenge(thread_cache);
    }
}

int cel_allocator_dump(char *buf, size_t size)
{
    int i;
    size_t cursor;
    size_t num1, num2;
    CelThreadCache *thread_cache, *thread_cache_tail;

    if (cel_malloc != cel_allocate)
        return 0;
    cursor = 0;
    cursor += snprintf(buf + cursor, size - cursor, "{\"allocator\":");
    /* central_caches */
    cursor += snprintf(buf + cursor, size - cursor, CEL_CRLF"{\"central_caches\":[");
    for (i = 0; i < BLOCKS_NUM; i++)
    {
        if (s_centralcache[i].n_allocated_blocks == 0)
            continue;
        cel_spinlock_lock(&(s_centralcache[i].lock));
        cursor += snprintf(buf + cursor, size - cursor, 
            CEL_CRLF"{ \"id\":%d, \"size\":%d, \"allocated\":%d, \"free\":%d, "
            "\"thread_caches\":[",
            (int)i, (int)cel_sizemap_block_to_size(i), 
            (int)s_centralcache[i].n_allocated_blocks, 
            (int)s_centralcache[i].n_free_blocks);
        num1 = 0;
        thread_cache =  
            (CelThreadCache *)cel_list_get_head(&s_threadcache_list);
        thread_cache_tail = 
            (CelThreadCache *)cel_list_get_tail(&s_threadcache_list);
        while ((thread_cache = thread_cache->next) != thread_cache_tail)
        {
            cursor += snprintf(buf + cursor, size - cursor, 
                "%d,", (int)(thread_cache->free_list[i].size));
            num1 += thread_cache->free_list[i].size;
        }
        cursor -= 1;
        cursor += snprintf(buf + cursor, size - cursor, "], \"used\":%d },", 
            (int)(s_centralcache[i].n_allocated_blocks 
            - s_centralcache[i].n_free_blocks - num1));
        cel_spinlock_unlock(&(s_centralcache[i].lock));
    }
    cursor -= 1;
    cursor += snprintf(buf + cursor, size - cursor, "],");
    /* page_heaps */
    cel_spinlock_lock(&(s_pageheap.lock));
    cursor += snprintf(buf + cursor, size - cursor, CEL_CRLF"{\"page_heaps\":[");
    for (i = 0; i < MAX_PAGES; i++)
    {
        num1 = cel_list_get_size(&(s_pageheap.free[i].normal_spans));
        num2 = cel_list_get_size(&(s_pageheap.free[i].returned_spans));
        if (num1 + num2 <= 0)
            continue;
        cursor += snprintf(buf + cursor, size - cursor, 
            CEL_CRLF"{ \"id\":%d, \"size\":%d, \"normal\":%d, \"retunred\":%d },",
            (int)i, (int)((i + 1) * PAGE_SIZE), (int)num1, (int)num2);
    }
    cursor -= 1;
    cursor += snprintf(buf + cursor, size - cursor, "],");
    /* lagre_pages */
    cursor += snprintf(buf + cursor, size - cursor, CEL_CRLF"{\"lagre_pages\":[");
    cursor -= 1;
    cursor += snprintf(buf + cursor, size - cursor, "]");
    cel_spinlock_unlock(&(s_pageheap.lock));
    cursor += snprintf(buf + cursor, size - cursor, "}");
    //puts(buf);

    return (int)cursor;
}

void *cel_allocate(size_t size)
{
    size_t n;
    CelThreadCache *thread_cache;
    CelBlock *new_block;
    CelSpan *span;

    if (size <= MAX_BLOCK_SIZE)
    {
        n = cel_sizemap_size_to_block(size);
        if ((thread_cache = cel_threadcache_get()) == NULL
            || (new_block = cel_threadcache_allocate(thread_cache, n)) == NULL)
            return NULL;
    }
    else
    {
        n = (size >> PAGE_SHIFT) + ((size & (PAGE_SIZE - 1)) > 0 ? 1 : 0);
        if ((span = cel_pageheap_allocate(&s_pageheap, n)) == NULL)
            return NULL;
        span->index = 0;
        span->blocks = NULL;
        new_block = (CelBlock *)(span->start << PAGE_SHIFT);
        //_tprintf(_T("size %d, span size %d\r\n"), 
        //    size, span->n_pages * (1 << PAGE_SHIFT));
    }
    //_tprintf(_T("new block %p")CEL_CRLF, new_block);
    return new_block;
}

void cel_deallocate(void *buf)
{
    CelSpan *span;
    size_t index;
    CelThreadCache *thread_cache;

    span = (CelSpan *)cel_radixtree_get(
        &(s_pageheap.page_map), ((uintptr_t)buf) >> PAGE_SHIFT);
    CEL_ASSERT(span != NULL);
   /*CEL_DEBUG((_T("Deallocate %p(0x%x), span %p index %d."), 
        buf, ((uintptr_t)buf) >> PAGE_SHIFT, span->start, span->index));*/
    if ((index = span->index) != 0)
    {
        if ((thread_cache = cel_threadcache_get()) != NULL)
            cel_threadcache_deallocate(thread_cache, (CelBlock *)buf, index);
    }
    else
    {
        cel_pageheap_deallocate(&s_pageheap, span);
    }
}

void *cel_reallocate(void *memory, size_t new_size)
{
    CelSpan *span;
    size_t index, old_size;
    void *new_mem;

    if (memory == NULL)
        return cel_allocate(new_size);

    span = (CelSpan *)cel_radixtree_get(
        &(s_pageheap.page_map), ((uintptr_t)memory) >> PAGE_SHIFT);
    CEL_ASSERT(span != NULL);
    if (((index = span->index) != 0 
        && (old_size = cel_sizemap_block_to_size(index)) < new_size)
        || (size_t)(span->n_pages >> PAGE_SHIFT) < new_size)
    {
        if ((new_mem = cel_allocate(new_size)) != NULL)
        {
            memcpy(new_mem, memory, old_size);
            cel_deallocate(memory);
            return new_mem;
        }
    }

    return NULL;
}

int cel_allocator_init()
{
    int i;

    cel_sizemap_init();
    cel_pageheap_init(&s_pageheap);
    i = BLOCKS_NUM;
    while ((--i) >= 0)
        cel_centralcache_init(&s_centralcache[i], i);
    cel_threadcache_moudle_init();
    return 0;
}

void cel_allocator_destroy()
{
    int i = BLOCKS_NUM;
    while ((--i) >= 0)
        cel_centralcache_destroy(&s_centralcache[i]);
    cel_pageheap_destroy(&s_pageheap);
}

int cel_allocator_hook_register(CelMallocFunc malloc_func, 
                                CelFreeFunc free_func,
                                CelReallocFunc realloc_func)
{
    if (malloc_func == NULL || malloc_func == cel_allocate)
    {
        if (cel_allocator_init() == 0)
        {
            cel_malloc = cel_allocate;
            cel_free = cel_deallocate;
            cel_realloc = cel_reallocate;
            return 0;
        }
        return -1;
    }
    else
    {
        cel_malloc = malloc_func;
        cel_free = free_func;
        cel_realloc = realloc_func;
    }
    return 0;
}

void cel_allocator_hook_unregister(void)
{
    if (cel_malloc == cel_allocate)
    {
        cel_allocator_destroy();
    }
    cel_malloc = malloc;
    cel_free = free;
    cel_realloc = realloc;
}
