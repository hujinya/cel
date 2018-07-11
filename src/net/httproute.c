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
#include "cel/net/httproute.h"
#include "cel/log.h"

int cel_httproutedata_init(CelHttpRouteData *rt_data)
{
    rt_data->n_params = 0;
    return 0;
}

void cel_httproutedata_destroy(CelHttpRouteData *rt_data)
{
    rt_data->n_params = 0;
}

char *cel_httproutedata_get(CelHttpRouteData *rt_data,
                            const char *key, char *value, size_t *size)
{
    int i;

    for (i = 0; i < rt_data->n_params; i++)
    {
        if (strcmp(key, rt_data->params[i].key) == 0)
        {
            if (*size > rt_data->params[i].value_size)
                *size = rt_data->params[i].value_size;
            memcpy(value, rt_data->params[i].value, *size);
            value[*size] = '\0';
            return value;
        }
    }
    *size = 0;
    return NULL;
}

static CelHttpRouteEntity *cel_httprouteentity_new(const char *key, 
                                                   CelHttpRouteEntityType type)
{
    CelHttpRouteEntity *new_entity;

    //_tprintf("New %s\r\n", key);
    if ((new_entity = cel_calloc(1, sizeof(CelHttpRouteEntity))) != NULL)
    {
        new_entity->key = cel_strdup(key);
        new_entity->type = type;
        return new_entity;
    }
    return NULL;
}

static void cel_httprouteentity_free(CelHttpRouteEntity *entity)
{
    //_tprintf("Free %s\r\n", entity->key);
    if (entity->key != NULL)
        cel_free(entity->key);
    if (entity->next_key_entity != NULL)
        cel_rbtree_free(entity->next_key_entity);
    if (entity->next_param_entity != NULL)
        cel_httprouteentity_free(entity->next_param_entity);
    entity->handle_func = NULL;
    cel_free(entity);
}

int cel_httproute_init(CelHttpRoute *route)
{
    int i = 0;
    CelHttpRouteEntity *entity;

    for (i = 0; i < CEL_HTTPM_CONUT; i++)
    {
        entity = &(route->root_entities[i]);
        entity->key = NULL;
        entity->type = CEL_HTTPROUTEENTITY_ROOT;
        entity->next_key_entity = NULL;
        entity->next_param_entity = NULL;
        entity->handle_func = NULL;
    }
    return 0;
}

void cel_httproute_destroy(CelHttpRoute *route)
{
    int i = 0;
    CelHttpRouteEntity *entity;

    for (i = 0; i < CEL_HTTPM_CONUT; i++)
    {
        entity = &(route->root_entities[i]);
        if (entity->key != NULL)
            cel_free(entity->key);
        if (entity->next_key_entity != NULL)
            cel_rbtree_free(entity->next_key_entity);
        if (entity->next_param_entity != NULL)
            cel_httprouteentity_free(entity->next_param_entity);
        entity->handle_func = NULL;
    }
}

CelHttpRoute *cel_httproute_new(void)
{
    CelHttpRoute *route;

    if ((route = (CelHttpRoute *)cel_malloc(sizeof(CelHttpRoute))) != NULL)
    {
        if (cel_httproute_init(route) == 0)
            return route;
        cel_free(route);
    }
    return NULL;
}

void cel_httproute_free(CelHttpRoute *route)
{
    cel_httproute_destroy(route);
    cel_free(route);
}

int cel_httproute_add(CelHttpRoute *route, 
                      CelHttpMethod method, const char *path, 
                      CelHttpHandleFunc handle_func)
{
    int i, j;
    char key[256];
    CelHttpRouteEntity *entity, *new_entity;

    entity = &(route->root_entities[method]);
    i = 0;
    while (path[i] != '\0')
    {
        j = 0;
        while (path[i] != '\0')
        {
            if (path[i] == '/')
            {
                i++;
                break;
            }
            key[j++] = path[i++];
        }
        key[j] = '\0';
        if (key[0] != '{')
        {
            //_tprintf("key %s\r\n", key);
            if (entity->next_key_entity == NULL)
            {
                if ((entity->next_key_entity = 
                    cel_rbtree_new((CelCompareFunc)strcmp, 
                    NULL, (CelFreeFunc)cel_httprouteentity_free)) == NULL)
                    return -1;
            }
            else if ((new_entity = cel_rbtree_lookup(
                entity->next_key_entity, key)) != NULL)
            {
                entity = new_entity;
                continue;
            }
            if ((new_entity = cel_httprouteentity_new(
                key, CEL_HTTPROUTEENTITY_KEY)) == NULL)
                return -1;
            cel_rbtree_insert(entity->next_key_entity, 
                new_entity->key, new_entity);
            entity = new_entity;
        }
        else
        {
            if (key[j - 1] == '}')
                key[j - 1] = '\0';
            if ((new_entity = cel_httprouteentity_new(
                key + 1, CEL_HTTPROUTEENTITY_PARAM)) == NULL)
                return -1;
            entity->next_param_entity = new_entity;
            entity = new_entity;
        }
    }
    entity->handle_func = handle_func;
    return 0;
}

int cel_httproute_remove(CelHttpRoute *route, 
                         CelHttpMethod method, const char *path)
{
    int i, j;
    char key[256];
    CelHttpRouteEntity *entity, *next_entity;

    entity = &(route->root_entities[method]);
    i = 0;
    while (path[i] != '\0')
    {
        j = 0;
        while (path[i] != '\0')
        {
            if (path[i] == '/')
            {
                i++;
                break;
            }
            key[j++] = path[i++];
        }
        key[j] = '\0';
        if (key[0] != '{')
        {
            if (entity->next_key_entity != NULL
                && (next_entity = cel_rbtree_lookup(
                entity->next_key_entity, key)) != NULL)
            {
                if (path[i] != '\0')
                {
                    cel_rbtree_remove(entity->next_key_entity, next_entity);
                    return 0;
                }
                entity = next_entity;
                continue;
            }
        }
        else if (next_entity->next_param_entity != NULL)
        {
            if (path[i] != '\0')
            {
                cel_httprouteentity_free(entity->next_param_entity);
                entity->next_param_entity = NULL;
                return 0;
            }
            continue;
        }
        return -1;
    }
    return -1;
}

CelHttpHandleFunc cel_httproute_handler(CelHttpRoute *route,
                                        CelHttpMethod method, 
                                        const char *path,
                                        CelHttpRouteData *rt_data)
{
    int i, j;
    char key[256];
    CelHttpRouteEntity *entity, *next_entity;

    if (rt_data != NULL)
        rt_data->n_params = 0;
    entity = &(route->root_entities[method]);
    i = 0;
    while (path[i] != '\0')
    {
        j = 0;
        while (path[i] != '\0')
        {
            if (path[i] == '/')
            {
                i++;
                break;
            }
            key[j++] = path[i++];
        }
        key[j] = '\0';
        //_tprintf("key %s\r\n", key);
        if (entity->next_key_entity != NULL
            && (next_entity = cel_rbtree_lookup(
            entity->next_key_entity, key)) != NULL)
        {
            entity = next_entity;
            continue;
        }
        if (entity->next_param_entity != NULL)
        {
            entity = entity->next_param_entity;
            if (rt_data != NULL)
            {
                rt_data->params[rt_data->n_params].key = entity->key;
                if (path[i] == '\0')
                    rt_data->params[rt_data->n_params].value = &path[i - j];
                else
                    rt_data->params[rt_data->n_params].value = &path[i - 1 - j];
                rt_data->params[rt_data->n_params].value_size = j;
                (rt_data->n_params)++;
            }
            continue;
        }
        _tprintf("key '%s' not found\r\n", key);
        return NULL;
    }
    return entity->handle_func;
}
