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
#include "cel/radixtree.h"
#include "cel/allocator.h"

#define DIV_ROUND_UP(x,y) (((x) + ((y) - 1)) / (y))

int cel_radixtree_init(CelRadixTree *radix_tree, 
                       int key_bits, int slot_bits, CelFreeFunc free_func)
{
    radix_tree->node_malloc = cel_malloc;
    radix_tree->node_free = cel_free;
    radix_tree->free_func = free_func;
    radix_tree->slot_bits = slot_bits;
    radix_tree->level_mask = ((1UL << radix_tree->slot_bits) - 1);
    radix_tree->height = DIV_ROUND_UP(key_bits, slot_bits);
    radix_tree->root = NULL;

    return 0;
}

void cel_radixtree_node_free(CelRadixTree *radix_tree, 
                             CelRadixNode *node, int height)
{
    int sub_index;
    CelRadixNode *childs;

    if (height > 1)
    {
        for (sub_index = 0; 
            sub_index < (int)radix_tree->level_mask; 
            sub_index++)
        {
            if ((childs = node[sub_index].childs) != NULL)
            {
               /* _tprintf(_T("height %d, sub %d free %p")CEL_CRLF,
                    height - 1, sub_index, childs);*/
                cel_radixtree_node_free(radix_tree, childs, height - 1);
                node[sub_index].childs = NULL;
            }
        }
    }
    else
    {
        if (node->value != NULL && radix_tree->free_func != NULL)
            radix_tree->free_func(node->value);
    }
    radix_tree->node_free(node);
}

void cel_radixtree_destroy(CelRadixTree *radix_tree)
{
    CelRadixNode *childs;

    if ((childs = radix_tree->root) != NULL)
    {
        //_tprintf(_T("free root %p")CEL_CRLF, childs);
        cel_radixtree_node_free(radix_tree, childs, radix_tree->height);
        radix_tree->root = NULL;
    }
}

CelRadixTree *cel_radixtree_new(int key_bits, 
                                int slot_bits, CelFreeFunc free_func)
{
    CelRadixTree *radix_tree;

    if ((radix_tree = 
        (CelRadixTree *)cel_malloc(sizeof(CelRadixTree))) != NULL)
    {
        if (cel_radixtree_init(
            radix_tree, key_bits, slot_bits, free_func) == 0)
            return radix_tree;
        cel_free(radix_tree);
    }
    return NULL;
}

void cel_radixtree_free(CelRadixTree *radix_tree)
{
    cel_radixtree_destroy(radix_tree);
    cel_free(radix_tree);
}

void *cel_radixtree_get(CelRadixTree *radix_tree, uintptr_t key)
{
    int height, slot_bits, sub_index;
    CelRadixNode *node, *slot;

    slot = radix_tree->root;
    height = radix_tree->height;
    slot_bits = (height - 1) * radix_tree->slot_bits;
    while (height > 0)
    {
        if (slot == NULL)
            return NULL;
        sub_index = ((key >> slot_bits) & radix_tree->level_mask);
        //_tprintf(_T("[%d]"), sub_index);
        node = &(slot[sub_index]);
        slot = node->childs;
        slot_bits -= radix_tree->slot_bits;
        height--;
    }
    //_tprintf(CEL_CRLF);
    return node->value;
}

int cel_radixtree_set(CelRadixTree *radix_tree, uintptr_t key, void *value)
{
    int height, slot_bits, sub_index;
    CelRadixNode *node = NULL, *slot;

    slot = radix_tree->root;
    height = radix_tree->height;
    slot_bits = (height - 1) * radix_tree->slot_bits;
    while (height > 0)
    {
        if (slot == NULL)
        {
            if ((slot = (CelRadixNode *)radix_tree->node_malloc(
                sizeof(CelRadixNode)
                * (1UL << radix_tree->slot_bits))) == NULL)
                return -1;
            memset(slot, 0, 
                sizeof(CelRadixNode) * (1UL << radix_tree->slot_bits));
            if (node != NULL)
                node->childs = slot;
            else
                radix_tree->root = slot;
            /*_tprintf(_T("height %d sub %d new %p,size %d")CEL_CRLF, 
                height, sub_index, slot, 
                sizeof(CelRadixNode) * (1UL << radix_tree->slot_bits));*/
        }
        //_tprintf(_T("[%d]"), sub_index);
        sub_index = ((key >> slot_bits) & radix_tree->level_mask);
        node = &(slot[sub_index]);
        slot = node->childs;
        slot_bits -= radix_tree->slot_bits;
        height--;
    }
    //_tprintf(CEL_CRLF);
    node->value = value;

    return 0;
}
