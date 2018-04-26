/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_RBTREE_H__
#define __CEL_RBTREE_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CelRbTreeNode
{
#define    RBTREE_RED        0
#define    RBTREE_BLACK      1
    unsigned long  parent_color;
    struct _CelRbTreeNode *right_node;
    struct _CelRbTreeNode *left_node;
    void *key;
    void *value;
}CelRbTreeNode;

typedef struct _CelRbTree
{
    int size;
    CelRbTreeNode *root_node;
    CelCompareFunc key_comparator;
    CelFreeFunc key_freefunc;
    CelFreeFunc value_freefunc;
}CelRbTree;

int cel_rbtree_init(CelRbTree *rbtree, CelCompareFunc key_comparator,
                    CelFreeFunc key_freefunc, CelFreeFunc value_freefunc);
void cel_rbtree_destroy(CelRbTree *rbtree);
CelRbTree *cel_rbtree_new(CelCompareFunc key_comparator, 
                          CelFreeFunc key_freefunc, 
                          CelFreeFunc value_freefunc);
void cel_rbtree_free(CelRbTree *rbtree);

void cel_rbtree_insert(CelRbTree *rbtree, void *key, void *value);
static __inline int cel_rbtree_get_size(CelRbTree *rbtree)
{
    return rbtree->size;
}

void *cel_rbtree_lookup(CelRbTree *rbtree, const void *key);
int cel_rbtree_foreach(CelRbTree *rbtree, 
                       CelKeyValuePairEachFunc each_func, void *user_data);

void cel_rbtree_remove(CelRbTree *rbtree, const void *key);
void cel_rbtree_clear(CelRbTree *rbtree);

#ifdef __cplusplus
}
#endif

#endif
