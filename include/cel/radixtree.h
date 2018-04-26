/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_RADIXTREE_H__
#define __CEL_RADIXTREE_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef union _CelRadixNode
{
    union _CelRadixNode *childs;
    void *value;
}CelRadixNode;

typedef struct _CelRadixTree
{
    CelMallocFunc node_malloc;
    CelFreeFunc node_free;
    CelFreeFunc free_func;
    int height;
    int slot_bits;
    uintptr_t level_mask;
    CelRadixNode *root;
}CelRadixTree;

/* 
 * MAP_SIZE = (1 << key_bits)
 * SLOT_SIZE = (1 << slot_bits)
 */

int cel_radixtree_init(CelRadixTree *radix_tree, 
                       int key_bits, int slot_bits, CelFreeFunc free_func);
void cel_radixtree_destroy(CelRadixTree *radix_tree);

CelRadixTree *cel_radixtree_new(int key_bits, int slot_bits, 
                                CelFreeFunc free_func);
void cel_radixtree_free(CelRadixTree *radix_tree);

static __inline 
void cel_radixtree_set_node_allocator(CelRadixTree *radix_tree, 
                                      CelMallocFunc node_malloc,
                                      CelFreeFunc node_free)
{
    radix_tree->node_malloc = node_malloc;
    radix_tree->node_free = node_free;
}

void *cel_radixtree_get(CelRadixTree *radix_tree, uintptr_t key);
int cel_radixtree_set(CelRadixTree *radix_tree, uintptr_t key, void *value);

#ifdef __cplusplus
}
#endif
#endif
