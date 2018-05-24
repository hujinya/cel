/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2016 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/rbtree.h"
#include "cel/error.h"
#include "cel/log.h"
#include "cel/allocator.h"

/* Debug defines */
#define Debug(args)   /* cel_log_debug args */ 
#define Warning(args) CEL_SETERRSTR(args)/* cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args)/* cel_log_err args */

int cel_rbtree_init(CelRbTree *rbtree, CelCompareFunc key_comparator, 
                    CelFreeFunc key_freefunc, CelFreeFunc value_freefunc)
{
    rbtree->key_comparator = key_comparator;
    rbtree->key_freefunc = key_freefunc;
    rbtree->value_freefunc = value_freefunc;
    rbtree->size = 0;
    rbtree->root_node = NULL;

    return 0;
}

void cel_rbtree_destroy(CelRbTree *rbtree)
{
    cel_rbtree_clear(rbtree);
    rbtree->key_comparator = NULL;
    rbtree->key_freefunc = NULL;
    rbtree->value_freefunc = NULL;
    rbtree->size = 0;
    rbtree->root_node = NULL;
}

CelRbTree *cel_rbtree_new(CelCompareFunc key_comparator, 
                          CelFreeFunc key_freefunc, CelFreeFunc value_freefunc)
{
    CelRbTree *rbtree;

    if ((rbtree = (CelRbTree *)cel_malloc(sizeof(CelRbTree))) != NULL)
    {
        if (cel_rbtree_init(
            rbtree, key_comparator, key_freefunc, value_freefunc) == 0)
            return rbtree;
        cel_free(rbtree);
    }
    return NULL;
}

void cel_rbtree_free(CelRbTree *rbtree)
{
    cel_rbtree_destroy(rbtree);
    cel_free(rbtree);
}

#define cel_rbtree_get_parent(node)  \
    ((CelRbTreeNode *)((node)->parent_color & ~3))
#define cel_rbtree_get_color(node)   ((node)->parent_color & 1)
#define cel_rbtree_is_red(node)      (cel_rbtree_get_color(node) == RBTREE_RED)
#define cel_rbtree_is_black(node)    \
    (cel_rbtree_get_color(node) == RBTREE_BLACK)
#define cel_rbtree_set_red(node)     (node)->parent_color &= ~1
#define cel_rbtree_set_black(node)   (node)->parent_color |= 1

static __inline void cel_rbtree_set_parent(CelRbTreeNode *rbtree, 
                                           CelRbTreeNode *p)
{
    rbtree->parent_color = (rbtree->parent_color & 3) | (unsigned long)p;
}

static __inline void cel_rbtree_set_color(CelRbTreeNode *rbtree, int color)
{
    rbtree->parent_color = (rbtree->parent_color & ~1) | color;
}

static __inline void cel_rbtree_link_node(CelRbTreeNode *node, 
                                          CelRbTreeNode *parent, 
                                          CelRbTreeNode **node_link)
{
    node->parent_color = (unsigned long)parent;
    node->left_node = node->right_node = NULL;

    *node_link = node;
}

static void cel_rbtree_rotate_left(CelRbTree *rbtree, CelRbTreeNode *node)
{
    CelRbTreeNode *right = node->right_node;
    CelRbTreeNode *parent = cel_rbtree_get_parent(node);

    if ((node->right_node = right->left_node) != NULL)
        cel_rbtree_set_parent(right->left_node, node);
    right->left_node = node;

    cel_rbtree_set_parent(right, parent);
    if (parent != NULL)
    {
        if (node == parent->left_node)
            parent->left_node = right;
        else
            parent->right_node = right;
    }
    else
        rbtree->root_node = right;
    cel_rbtree_set_parent(node, right);
}

static void cel_rbtree_rotate_right(CelRbTree *rbtree, CelRbTreeNode *node)
{
    CelRbTreeNode *left = node->left_node;
    CelRbTreeNode *parent = cel_rbtree_get_parent(node);

    if ((node->left_node = left->right_node) != NULL)
        cel_rbtree_set_parent(left->right_node, node);
    left->right_node = node;

    cel_rbtree_set_parent(left, parent);
    if (parent != NULL)
    {
        if (node == parent->right_node)
            parent->right_node = left;
        else
            parent->left_node = left;
    }
    else
        rbtree->root_node = left;
    cel_rbtree_set_parent(node, left);
}

static void cel_rbtree_insert_color(CelRbTree *rbtree, CelRbTreeNode *node)
{
    CelRbTreeNode *parent, *gparent;

    while ((parent = cel_rbtree_get_parent(node)) != NULL 
        && cel_rbtree_is_red(parent))
    {
        gparent = cel_rbtree_get_parent(parent);
        if (parent == gparent->left_node)
        {
            {
                register CelRbTreeNode *uncle = gparent->right_node;
                if (uncle != NULL && cel_rbtree_is_red(uncle))
                {
                    cel_rbtree_set_black(uncle);
                    cel_rbtree_set_black(parent);
                    cel_rbtree_set_red(gparent);
                    node = gparent;
                    continue;
                }
            }
            if (parent->right_node == node)
            {
                register CelRbTreeNode *tmp;
                cel_rbtree_rotate_left(rbtree, parent);
                tmp = parent;
                parent = node;
                node = tmp;
            }
            cel_rbtree_set_black(parent);
            cel_rbtree_set_red(gparent);
            cel_rbtree_rotate_right(rbtree, gparent);
        } 
        else 
        {
            {
                register CelRbTreeNode *uncle = gparent->left_node;
                if (uncle != NULL && cel_rbtree_is_red(uncle))
                {
                    cel_rbtree_set_black(uncle);
                    cel_rbtree_set_black(parent);
                    cel_rbtree_set_red(gparent);
                    node = gparent;
                    continue;
                }
            }
            if (parent->left_node == node)
            {
                register CelRbTreeNode *tmp;
                cel_rbtree_rotate_right(rbtree, parent);
                tmp = parent;
                parent = node;
                node = tmp;
            }
            cel_rbtree_set_black(parent);
            cel_rbtree_set_red(gparent);
            cel_rbtree_rotate_left(rbtree, gparent);
        }
    }
    cel_rbtree_set_black(rbtree->root_node);
}

void cel_rbtree_insert(CelRbTree *rbtree, void *key, void *value)
{
    int ret;
    CelRbTreeNode **tmp = &(rbtree->root_node), *parent = NULL, *new_node;

    /* Figure out where to put new node */  
    while ((*tmp) != NULL)
    {
        parent = *tmp; 
        //printf("Parent value %d\r\n", *((int *)(*tmp)->key));
        if ((ret= rbtree->key_comparator(key, (*tmp)->key)) < 0)
            tmp = &((*tmp)->left_node);  
        else if (ret > 0) 
            tmp = &((*tmp)->right_node);  
        else   
            return ;  
    }  
    if ((new_node = 
        (CelRbTreeNode *)cel_malloc(sizeof(CelRbTreeNode))) == NULL)
        return;
    new_node->key = key;
    new_node->value = value;
    /* Add new node and rebalance tree. */  
    cel_rbtree_link_node(new_node, parent, tmp);  
    cel_rbtree_insert_color(rbtree, new_node);
    rbtree->size++;
}

static void cel_rbtree_remove_color(CelRbTree *rbtree, 
                                    CelRbTreeNode *node, CelRbTreeNode *parent)
{
    CelRbTreeNode *other;

    while ((node == NULL || cel_rbtree_is_black(node))
        && node != rbtree->root_node)
    {
        if (parent->left_node == node)
        {
            other = parent->right_node;
            if (cel_rbtree_is_red(other))
            {
                cel_rbtree_set_black(other);
                cel_rbtree_set_red(parent);
                cel_rbtree_rotate_left(rbtree, parent);
                other = parent->right_node;
            }
            if ((other->left_node == NULL 
                || cel_rbtree_is_black(other->left_node))
                && (other->right_node == NULL 
                || cel_rbtree_is_black(other->right_node)))
            {
                cel_rbtree_set_red(other);
                node = parent;
                parent = cel_rbtree_get_parent(node);
            }
            else
            {
                if (other->right_node == NULL 
                    || cel_rbtree_is_black(other->right_node))
                {
                    cel_rbtree_set_black(other->left_node);
                    cel_rbtree_set_red(other);
                    cel_rbtree_rotate_right(rbtree, other);
                    other = parent->right_node;
                }
                cel_rbtree_set_color(other, cel_rbtree_get_color(parent));
                cel_rbtree_set_black(parent);
                cel_rbtree_set_black(other->right_node);
                cel_rbtree_rotate_left(rbtree, parent);
                node = rbtree->root_node;
                break;
            }
        }
        else
        {
            other = parent->left_node;
            if (cel_rbtree_is_red(other))
            {
                cel_rbtree_set_black(other);
                cel_rbtree_set_red(parent);
                cel_rbtree_rotate_right(rbtree, parent);
                other = parent->left_node;
            }
            if ((other->left_node == NULL 
                || cel_rbtree_is_black(other->left_node))
                && (other->right_node == NULL 
                || cel_rbtree_is_black(other->right_node)))
            {
                cel_rbtree_set_red(other);
                node = parent;
                parent = cel_rbtree_get_parent(node);
            }
            else
            {
                if (other->left_node == NULL 
                    || cel_rbtree_is_black(other->left_node))
                {
                    cel_rbtree_set_black(other->right_node);
                    cel_rbtree_set_red(other);
                    cel_rbtree_rotate_left(rbtree, other);
                    other = parent->left_node;
                }
                cel_rbtree_set_color(other, cel_rbtree_get_color(parent));
                cel_rbtree_set_black(parent);
                cel_rbtree_set_black(other->left_node);
                cel_rbtree_rotate_right(rbtree, parent);
                node = rbtree->root_node;
                break;
            }
        }
    }
    if (node != NULL)
        cel_rbtree_set_black(node);
}

static void cel_rbtree_remove_node(CelRbTree *rbtree, CelRbTreeNode *node)
{
    int color;
    CelRbTreeNode *child, *parent;

    if (node->left_node == NULL)
        child = node->right_node;
    else if (node->right_node == NULL)
        child = node->left_node;
    else
    {
        CelRbTreeNode *old = node, *left;

        node = node->right_node;
        while ((left = node->left_node) != NULL)
            node = left;
        child = node->right_node;
        parent = cel_rbtree_get_parent(node);
        color = cel_rbtree_get_color(node);
        if (child != NULL)
            cel_rbtree_set_parent(child, parent);
        if (parent == old)
        {
            parent->right_node = child;
            parent = node;
        } 
        else
        {
            parent->left_node = child;
        }
        node->parent_color = old->parent_color;
        node->right_node = old->right_node;
        node->left_node = old->left_node;
        if (cel_rbtree_get_parent(old) != NULL)
        {
            if (cel_rbtree_get_parent(old)->left_node == old)
                cel_rbtree_get_parent(old)->left_node = node;
            else
                cel_rbtree_get_parent(old)->right_node = node;
        } 
        else
            rbtree->root_node = node;

        cel_rbtree_set_parent(old->left_node, node);
        if (old->right_node != NULL)
            cel_rbtree_set_parent(old->right_node, node);
        goto color;
    }

    parent = cel_rbtree_get_parent(node);
    color = cel_rbtree_get_color(node);

    if (child != NULL)
        cel_rbtree_set_parent(child, parent);
    if (parent != NULL)
    {
        if (parent->left_node == node)
            parent->left_node = child;
        else
            parent->right_node = child;
    }
    else
        rbtree->root_node = child;

color:
    if (color == RBTREE_BLACK)
        cel_rbtree_remove_color(rbtree, child, parent);
}

static CelRbTreeNode *cel_rbtree_lookup_node(CelRbTree *rbtree, 
                                             const void *key)
{
    int ret;
    CelRbTreeNode *node = rbtree->root_node; 

    while (node != NULL)
    {  
        if ((ret = rbtree->key_comparator(key, node->key)) < 0)
            node = node->left_node;
        else if (ret > 0) 
            node = node->right_node;
        else   
            return node;
    }
    return NULL;
}

void *cel_rbtree_lookup(CelRbTree *rbtree, const void *key)
{
    CelRbTreeNode *node;

    return ((node = cel_rbtree_lookup_node(rbtree, key)) == NULL)
        ? NULL : node->value;
}

void cel_rbtree_remove(CelRbTree *rbtree, const void *key)
{
    CelRbTreeNode *node;

    if ((node = cel_rbtree_lookup_node(rbtree, key)) == NULL)
        return ;
    //printf("node %p\r\n", node);
    cel_rbtree_remove_node(rbtree, node);
    rbtree->size--;
    cel_free(node);
}

/*
 * This function returns the first node (in sort order)of the tree.
 */
CelRbTreeNode *cel_rbtree_first(CelRbTree *rbtree)
{
    CelRbTreeNode *node;

    if ((node = rbtree->root_node) == NULL)
        return NULL;
    while (node->left_node != NULL)
        node = node->left_node;
    return node;
}

CelRbTreeNode *cel_rbtree_last(CelRbTree *rbtree)
{
    CelRbTreeNode *node;

    if ((node = rbtree->root_node) == NULL)
        return NULL;
    while (node->right_node)
        node = node->right_node;
    return node;
}

CelRbTreeNode *cel_rbtree_next(CelRbTreeNode *node)
{
    CelRbTreeNode *parent;

    if (cel_rbtree_get_parent(node) == node)
        return NULL;

    /* If we have a right-hand child, go down and then left as far as we can. */
    if (node->right_node != NULL)
    {
        node = node->right_node; 
        while (node->left_node != NULL)
            node = node->left_node;
        return node;
    }
    /* 
     * No right-hand children. Everything down and left is smaller than us, 
     * so any 'next' node must be in the general direction of our parent. 
     * Go up the tree; any time the ancestor is a right-hand child of its parent,
     * keep going up. First time it's a left-hand child of its parent, said parent 
     * is our 'next' node. 
     */
    while ((parent = cel_rbtree_get_parent(node)) != NULL
        && node == parent->right_node)
        node = parent;

    return parent;
}

CelRbTreeNode *cel_rbtree_prev(CelRbTreeNode *node)
{
    CelRbTreeNode *parent;

    if (cel_rbtree_get_parent(node) == node)
        return NULL;

    /* If we have a left-hand child, go down and then right as far as we can. */
    if ((node->left_node) != NULL)
    {
        node = node->left_node; 
        while (node->right_node != NULL)
            node = node->right_node;
        return node;
    }

    /* 
     * No left-hand children. Go up till we find an ancestor which
     * is a right-hand child of its parent 
     */
    while ((parent = cel_rbtree_get_parent(node)) != NULL 
        && node == parent->left_node)
        node = parent;

    return parent;
}

int cel_rbtree_foreach(CelRbTree *rbtree, 
                       CelKeyValuePairEachFunc each_func, void *user_data)
{
    int ret;
    CelRbTreeNode *node;

    node = cel_rbtree_first(rbtree);
    while (node != NULL)
    {
        if ((ret = each_func(node->key, node->value, user_data)) != 0)
            return ret;
        node = cel_rbtree_next(node);
    }
    return 0;
}

CelRbTreeNode *cel_rbtree_clear_next(CelRbTreeNode *node)
{
    CelRbTreeNode *parent;

    if ((parent = cel_rbtree_get_parent(node)) == NULL)
        return NULL;
    if (parent->right_node == NULL 
        || parent->right_node == node)
    {
        node = parent;
        return node;
    }
    node = parent->right_node;
    while (node->left_node != NULL)
        node = node->left_node;
    return node;
}

void cel_rbtree_clear(CelRbTree *rbtree)
{
    CelRbTreeNode *node, *next_node;

    node = cel_rbtree_first(rbtree);
    while (node != NULL)
    {
        next_node = cel_rbtree_clear_next(node);
        if (rbtree->key_freefunc != NULL)
            rbtree->key_freefunc(node->key);
        if (rbtree->value_freefunc != NULL)
            rbtree->value_freefunc(node->value);
        /*printf("Remove node %d, parent %p, size %d.\r\n", 
            *((int *)node->value), cel_rbtree_get_parent(node), rbtree->size);*/
        cel_free(node);
        rbtree->size--;
        node = next_node;
    }
    rbtree->root_node = NULL;
}
