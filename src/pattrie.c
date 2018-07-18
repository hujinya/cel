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
#include "cel/pattrie.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

int cel_pattrie_init(CelPatTrie *pat_trie, CelFreeFunc value_free_func)
{
    pat_trie->root = NULL;
    pat_trie->value_free_func = value_free_func;
    return 0;
}

void cel_pattrie_destroy(CelPatTrie *pat_trie)
{
    cel_pattrie_clear(pat_trie);
}

CelPatTrie *cel_pattrie_new(CelFreeFunc value_free_func)
{
    CelPatTrie *pat_trie;

    if ((pat_trie = 
        (CelPatTrie *)cel_malloc(sizeof(CelPatTrie))) != NULL)
    {
        if (cel_pattrie_init(pat_trie, value_free_func) == 0)
            return pat_trie;
        cel_free(pat_trie);
    }
    return NULL;
}

void cel_pattrie_free(CelPatTrie *pat_trie)
{
    cel_pattrie_destroy(pat_trie);
    cel_free(pat_trie);
}

CelPatTrieNode *cel_pattrie_node_new(CelPatTrieNodeType type, 
                                     char *key, size_t key_len, void *value)
{
    CelPatTrieNode *new_node;

    new_node = (CelPatTrieNode *)cel_malloc(sizeof(CelPatTrieNode));
    CEL_ASSERT(new_node == NULL);

    if (key != NULL)
    {
        new_node->key = cel_malloc(key_len + 1);
        memcpy(new_node->key, key, key_len);
        new_node->key[key_len] = '\0';
        new_node->key_len = key_len;
    }
    else
    {
        new_node->key = NULL;
        new_node->key_len = 0;
    }
    CEL_ASSERT(new_node->key == NULL);

    new_node->type = type;
    new_node->value = value;
    new_node->static_children = NULL;
    new_node->param_children = NULL;
    new_node->terminal = FALSE;

    printf("new key %s, len %d\r\n", 
        new_node->key, (int)new_node->key_len);

    return new_node;
}

void cel_pattrie_node_free(CelPatTrieNode *node, CelFreeFunc value_free_func)
{
    CelList *list;
    CelPatTrieNode *child, *next;

    if ((list = node->static_children) != NULL)
    {
        child = (CelPatTrieNode *)(list->head.next);
        while (child != (CelPatTrieNode *)cel_list_get_tail(list))
        {
            //printf("child key %s\r\n", child->key);
            next = (CelPatTrieNode *)child->item.next;
            cel_pattrie_node_free(child, value_free_func);
            child = next;
        }
    }
    if ((list = node->param_children) != NULL)
    {
        child = (CelPatTrieNode *)(list->head.next);
        while (child != (CelPatTrieNode *)cel_list_get_tail(list))
        {
            //printf("child key %s\r\n", child->key);
            next = (CelPatTrieNode *)child->item.next;
            cel_pattrie_node_free(child, value_free_func);
            child = next;
        }
    }
    printf("free key %s\r\n", node->key);
    if (node->key != NULL)
        cel_free(node->key);
    if (node->value != NULL && value_free_func != NULL)
        value_free_func(node->value);
    cel_free(node);
}

int _cel_pattrie_child_node_insert(CelPatTrieNode *node, 
                                   char *key, size_t key_len, void *value)
{
    int i, p0 = -1, p1 = -1;
    CelPatTrieNode *new_node;

    printf("child node %s %d, key %s %d\r\n", 
        node->key, (int)node->key_len, key, (int)key_len);
    for (i = 0; i < (int)key_len; i++)
    {
        if (p0 < 0 && key[i] == '<')
            p0 = i;
        if (p0 >= 0 && key[i] == '>')
        {
            p1 = i;
            break;
        }
    }
    if ((p0 > 0 && p1 > 0) || p1 < 0)
    {
        if (p1 > 0)
        {
            new_node = cel_pattrie_node_new(
                CEL_PATTRIE_NODE_STATIC, key, p0, NULL);
            if (node->static_children == NULL)
                node->static_children = cel_list_new(NULL);
            cel_list_push_back(node->static_children, (CelListItem *)new_node);
            node = new_node;
        }
        else
        {
            new_node = cel_pattrie_node_new(
                CEL_PATTRIE_NODE_STATIC, key, key_len, value);
            if (node->static_children == NULL)
                node->static_children = cel_list_new(NULL);
            cel_list_push_back(node->static_children, (CelListItem *)new_node);
            return 0;
        }
    }
    /* Add param node */
    if (p1 == key_len - 1)
    {
        new_node = cel_pattrie_node_new(
            CEL_PATTRIE_NODE_PARAM, &key[p0], key_len - p0, value);
        if (node->param_children == NULL)
            node->param_children = cel_list_new(NULL);
        cel_list_push_back(node->param_children, (CelListItem *)new_node);
        return 0;
    }
    else
    {
        new_node = cel_pattrie_node_new(
            CEL_PATTRIE_NODE_PARAM, &key[p0], p1 - p0 + 1, NULL);
        if (node->param_children == NULL)
            node->param_children = cel_list_new(NULL);
        cel_list_push_back(node->param_children, (CelListItem *)new_node);
        return _cel_pattrie_child_node_insert(
            new_node, &key[p1 + 1], key_len - p1, value);
    }
}

int _cel_pattrie_node_insert(CelPatTrieNode *node, 
                             char *key, size_t key_len, void *value)
{
    size_t matched, node_key_len, sub_key_len;
    char *sub_key;
    CelPatTrieNode *child, *tail, *new_node;

    printf("node %s %d, key %s %d\r\n", 
        node->key, (int)node->key_len, key, (int)key_len);
    node_key_len = (node->key_len < key_len ? node->key_len : key_len);
    for (matched = 0 ;matched < node_key_len; matched++)
    {
        if (node->key[matched] != key[matched])
            break;
    }
    if (matched == node->key_len)
    {
        if (matched == key_len)
        {
            if (!(node->terminal))
            {
                node->terminal = TRUE;
                node->value = value;
            }
            return 0;
        }
        sub_key = &key[matched];
        sub_key_len = key_len - matched;
        /* Trying adding to a static child */
        if (node->static_children != NULL)
        {
            child = (CelPatTrieNode *)cel_list_get_head(node->static_children);
            tail = (CelPatTrieNode *)cel_list_get_tail(node->static_children);
            while ((child = (CelPatTrieNode *)child->item.next) != tail)
            {
                if (_cel_pattrie_node_insert(
                    child, sub_key, sub_key_len, value) == 0)
                    return 0;
            }
        }
        /* Trying adding to a param child */
        if (node->param_children != NULL)
        {
            child = (CelPatTrieNode *)cel_list_get_head(node->param_children);
            tail = (CelPatTrieNode *)cel_list_get_tail(node->param_children);
            while ((child = (CelPatTrieNode *)child->item.next) != tail)
            {
                if (_cel_pattrie_node_insert(
                    child, sub_key, sub_key_len, value) == 0)
                    return 0;
            }
        }
        return _cel_pattrie_child_node_insert(
            node, sub_key, sub_key_len, value);
    }
    if (matched == 0 
        || node->type != CEL_PATTRIE_NODE_STATIC) 
    {
        /*
         * no common prefix, 
         * or partial common prefix with a non-static node: should skip this node
         */
        return -1;
    }
    /* The node key shares a partial prefix with the key: split the node key */
    new_node = cel_pattrie_node_new(CEL_PATTRIE_NODE_STATIC, 
        &(node->key[matched]), node->key_len - matched, node->value);
    new_node->static_children = node->static_children;
    new_node->param_children = node->param_children;
    new_node->terminal = node->terminal;

    node->key[matched] = '\0';
    sub_key = cel_strdup(node->key);
    if (node->key != NULL)
        cel_free(node->key);
    node->key = sub_key;
    node->key_len = matched;
    node->value = NULL;
    node->static_children = cel_list_new(NULL);
    node->param_children = NULL;
    node->terminal = FALSE;
    cel_list_push_back(node->static_children, (CelListItem *)new_node);

    return _cel_pattrie_node_insert(node, key, key_len, value);
}

//void _cel_pattrie_node_insert(CelPatTrieNode *node, 
//                              char *key, size_t key_len, void *value)
//{
//    size_t i, child_key_len;
//    char *sub_key, *child_sub_key;
//    CelList *list;
//    CelPatTrieNode *child, *new_node, *new_node1;
//
//    if ((list = node->static_children) == NULL)
//        list = node->static_children = cel_list_new(NULL);
//    child = (CelPatTrieNode *)&(list->head);
//    while ((child = (CelPatTrieNode *)child->item.next) 
//        != (CelPatTrieNode *)cel_list_get_tail(list))
//    {
//        /* Use min(child.key.length, key.length) */
//        child_key_len = (child->key_len < key_len ? child->key_len : key_len);
//       /* printf("child %s %s, child len %d, %d\r\n", 
//            child->key, key, child->key_len, key_len);*/
//        for (i = 0 ;i < child_key_len; i++)
//        {
//            if (child->key[i] != key[i])
//                break;
//        }
//        if (i == 0)
//        {
//            if (key[0] < child->key[0])
//            {
//                new_node = cel_pattrie_node_new(key, key_len, value);
//                new_node->terminal = TRUE;
//
//                new_node->item.prev = (CelListItem *)child;
//                new_node->item.next = child->item.next;
//                child->item.next->prev = (CelListItem *)new_node;
//                child->item.next = (CelListItem *)new_node;
//                return ;
//            }
//            else
//                continue;
//        }
//        else
//        {
//            //printf("i %d child_key_len %d\r\n", i, child_key_len);
//            if (i == child_key_len)
//            {
//                if (key_len == child->key_len)
//                {
//                    if (child->terminal)
//                    {
//                        puts("Duplicate Key is found when insertion!");
//                        return ;
//                    }
//                    else
//                    {
//                        child->terminal = TRUE;
//                        child->value = value;
//                        return;
//                    }
//                }
//                else if (key_len > child->key_len)
//                {
//                    sub_key = &key[i];
//                    _cel_pattrie_node_insert(
//                        child, sub_key, key_len - i, value);
//                    return ;
//                }
//                else
//                {
//                    child_sub_key = &(child->key[i]);
//                    new_node = cel_pattrie_node_new(
//                        child_sub_key, child->key_len - i, child->value);
//                    new_node->static_children = child->static_children;
//                    new_node->terminal = child->terminal;
//                    if (child->key != NULL)
//                        cel_free(child->key);
//                    child->key = cel_strdup(key);
//                    child->key_len = key_len;
//                    child->value = value;
//                    child->static_children = cel_list_new(NULL);
//                    child->terminal = TRUE;
//                    cel_list_push_back(
//                        child->static_children, (CelListItem *)new_node);
//                    return ;
//                }
//            }
//            else //0 < j < child_key_len
//            {
//                child_sub_key = &(child->key[i]);
//                sub_key = &key[i];
//
//                new_node = cel_pattrie_node_new(
//                    child_sub_key, child->key_len - i, child->value);
//                new_node->static_children = child->static_children;
//                new_node->terminal = child->terminal;
//
//                new_node1 = cel_pattrie_node_new(
//                    sub_key, key_len - i, value);
//                new_node1->terminal = TRUE;
//
//                child->key[i] = '\0';
//                child_sub_key = cel_strdup(child->key);
//                if (child->key != NULL)
//                    cel_free(child->key);
//                child->key = child_sub_key;
//                child->key_len = i;
//                child->value = NULL;
//                child->static_children = cel_list_new(NULL);
//                child->terminal = FALSE;
//                if (new_node->key[0] < new_node1->key[0])
//                {
//                    cel_list_push_back(
//                        child->static_children, (CelListItem *)new_node);
//                    cel_list_push_back(
//                        child->static_children, (CelListItem *)new_node1);
//                }
//                else
//                {
//                    cel_list_push_back(
//                        child->static_children, (CelListItem *)new_node1);
//                    cel_list_push_back(
//                        child->static_children, (CelListItem *)new_node);
//                }
//                return ;
//            }
//        }
//    }
//    new_node = cel_pattrie_node_new(key, key_len, value);
//    new_node->terminal = TRUE;
//    cel_list_push_back(node->static_children, (CelListItem *)new_node);
//
//    return ;
//}

void *_cel_pattrie_node_lookup(CelPatTrieNode *node, char *key, size_t key_len)
{
    size_t child_key_len, i;
    char *sub_key;
    CelList *list;
    CelPatTrieNode *child;

    list = node->static_children;
    child = (CelPatTrieNode *)&(list->head);
    while ((child = (CelPatTrieNode *)child->item.next)
        != (CelPatTrieNode *)&(list->tail))
    {
        /* Use min(child.key.length, key.length) */
        child_key_len = child->key_len;
        if (child_key_len >= key_len)
            child_key_len = key_len;
        for (i = 0 ;i < child_key_len; i++)
        {
            if (child->key[i] != key[i])
                break;
        }
        if(i == 0)
        {
            /*
             * This child doesn't match any character with the new key
             * order keys by lexi-order
             */
            if(key[0] < child->key[i])
                return NULL;
            else
                continue;
        }
        else if (i == child_key_len)
        {  
            if (key_len == child_key_len)
            {
                if (child->terminal)
                    return child->value;
                else
                    return NULL;
            }
            else if(key_len > child_key_len)
            {
                sub_key = &key[i];
                return _cel_pattrie_node_lookup(
                    child, sub_key, key_len - i);
            }
            return NULL;
        }
        else
        {
            return NULL;
        }
    }
    return NULL;
}

void _cel_pattrie_node_remove(CelPatTrieNode *node, 
                              char *key, size_t key_len, 
                              CelFreeFunc value_free_func)
{
    size_t child_key_len, i;
    char *sub_key;
    CelList *list;
    CelPatTrieNode *child, *single_node;

    list = node->static_children;
    child = (CelPatTrieNode *)&(list->head);
    while ((child = (CelPatTrieNode *)child->item.next) 
        != (CelPatTrieNode *)cel_list_get_tail(list))
    {
        /* Use min(child.key.length, key.length) */
        child_key_len = child->key_len;
        if (child_key_len >= key_len)
            child_key_len = key_len;
        for (i = 0 ;i < child_key_len; i++)
        {
            if (child->key[i] != key[i])
                break;
        }
        if (i == 0)
        {
            if (key[0] < child->key[0])
            {
                puts("No such key is found for removal!");
                return ;
            }
            else
                continue;
        }
        else
        {
            if (i == child_key_len)
            {
                if (key_len == child_key_len)
                {
                    if (child->terminal)
                    {
                        if (child->static_children == NULL
                            || cel_list_get_size(child->static_children) == 0)
                        {
                            cel_list_remove(list, (CelListItem *)child);
                            if (node->static_children == NULL
                                || cel_list_get_size(node->static_children) == 1)
                            {
                                single_node = (CelPatTrieNode *)
                                    cel_list_get_front(node->static_children);
                                node->key = cel_realloc(node->key, 
                                    node->key_len + single_node->key_len + 1);
                                memcpy(node->key + node->key_len, 
                                    single_node->key, single_node->key_len);
                                node->key_len += single_node->key_len;
                                node->key[node->key_len] = '\0';
                                node->value = single_node->value;
                                if (node->static_children != NULL)
                                    cel_list_free(node->static_children);
                                node->static_children = single_node->static_children;
                                node->terminal = single_node->terminal;
                                single_node->static_children = NULL;
                                single_node->value = NULL;
                                cel_pattrie_node_free(single_node, value_free_func);
                            }
                        }
                        else
                        {
                            child->terminal = FALSE;
                            if(child->static_children != NULL
                                && cel_list_get_size(child->static_children) == 1)
                            {  
                                single_node = (CelPatTrieNode *)
                                    cel_list_get_front(node->static_children);
                                 node->key = cel_realloc(node->key, 
                                    node->key_len + single_node->key_len + 1);
                                memcpy(node->key + node->key_len, 
                                    single_node->key, single_node->key_len);
                                node->key_len += single_node->key_len;
                                node->key[node->key_len] = '\0';
                                node->value = single_node->value;
                                if (node->static_children != NULL)
                                    cel_list_free(node->static_children);
                                node->static_children = single_node->static_children;
                                node->terminal = single_node->terminal;
                                single_node->static_children = NULL;
                                single_node->value = NULL;
                                cel_pattrie_node_free(single_node, value_free_func);
                            }
                        }
                    }
                    else
                    {
                        puts("No such key is found for removal!");
                        return ;
                    }
                }
                else if (key_len > child_key_len)
                {
                    sub_key = &key[i];
                    _cel_pattrie_node_remove(
                        child, sub_key, key_len - i, value_free_func);
                    return ;
                }
                else
                {
                    puts("No such key is found for removal!");
                    return ;
                }
            }
            else //0 < j < child_key_len
            {
                puts("No such key is found for removal!");
                return ;
            }
            break;
        }
    }
    puts("No such key is found for removal!");
    return ;
}

void cel_pattrie_clear(CelPatTrie *pat_trie)
{
    if (pat_trie->root != NULL)
        cel_pattrie_node_free(pat_trie->root, pat_trie->value_free_func);
}
