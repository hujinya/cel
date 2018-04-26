#include "cel/arraylist.h"
#include "cel/error.h"
#include "cel/log.h"
#include "cel/allocator.h"

/* Debug defines */
#define Debug(args)   /*cel_log_debug args*/
#define Warning(args) CEL_SETERRSTR(args)/* cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args)/* cel_log_err args */

int cel_arraylist_init(CelArrayList *array_list, CelFreeFunc free_func)
{
    array_list->free_func = free_func;
    array_list->size = 0;
    array_list->reserve = 0;
    array_list->capacity = 0;
    array_list->items = NULL;

    return 0; 
}

void cel_arraylist_destroy(CelArrayList *array_list) 
{
    cel_arraylist_clear(array_list);
    array_list->reserve = 0;
    array_list->capacity = 0;
    array_list->size = 0;
    if (array_list->items != NULL)
        cel_free(array_list->items);
    array_list->items = NULL;
    array_list->free_func = NULL;
}

CelArrayList *cel_arraylist_new(CelFreeFunc free_func)
{
    CelArrayList *array_list;

    if ((array_list = (CelArrayList *)cel_malloc(
        sizeof(CelArrayList))) != NULL)
    {
        if (cel_arraylist_init(array_list, free_func) == 0)
            return array_list;
        cel_free(array_list);
    }

    return NULL;
}

void cel_arraylist_free(CelArrayList *array_list)
{
    cel_arraylist_destroy(array_list);
    cel_free(array_list);
}

BOOL cel_arraylist_maybe_larger(CelArrayList *array_list, size_t *new_capacity)
{
    if (array_list->size == array_list->capacity)
    {
        if (array_list->reserve > 0)
        {
            *new_capacity = array_list->capacity + array_list->reserve;
            return TRUE;
        }
        else if (array_list->capacity < CEL_CAPACITY_MAX)
        {
            *new_capacity = cel_capacity_get_min(array_list->capacity);
            return TRUE;
        }
        Err((_T("Number %d exceeds the maximum capacity %d."), 
            array_list->capacity, CEL_CAPACITY_MAX));
    }
    return FALSE;
}

BOOL cel_arraylist_maybe_smaller(CelArrayList *array_list, 
                                 size_t *new_capacity)
{
    if (array_list->capacity >= (3 * array_list->size))
    {
        if (array_list->reserve > 0)
        {
            *new_capacity = array_list->size - array_list->reserve;
            return TRUE;
        }
        else if (array_list->capacity > CEL_CAPACITY_MIN)
        {
            *new_capacity = cel_capacity_get_min(array_list->size);
            return TRUE;
        }
    }
    return FALSE;
}

int cel_arraylist_resize(CelArrayList *array_list, size_t new_capacity)
{
    /*_tprintf(_T("Arrary resize(%p:%d),
    new capacity %d, old capacity %d.\r\n"), 
        array_list->items, array_list->size, 
        new_capacity, array_list->capacity);*/

    if ((array_list->items = (void **)cel_realloc(
        array_list->items, new_capacity * sizeof(void *))) == NULL)
    {
        return -1;
    }
    array_list->capacity = new_capacity;

    return 0;
}

int cel_arraylist_push(CelArrayList *array_list, size_t position, void *item)
{
    size_t new_capacity, i;

    if (position < 0 || position > array_list->size)
        return -1;
    if (cel_arraylist_maybe_larger(array_list, &new_capacity)
        && cel_arraylist_resize(array_list, new_capacity) == -1)
        return -1;
    if ((i = array_list->size - position) > 0)
        memmove(&(array_list->items[position + 1]), 
        &(array_list->items[position]), i * sizeof(void *));
    array_list->items[position] = item;
    array_list->size++;
    //_tprintf(_T("Push %p(%p) \r\n"), value, array_list->items[index + 1]);

    return 0;
}

void cel_arraylist_push_back(CelArrayList *array_list, void *item)
{
    size_t new_capacity;

    if (cel_arraylist_maybe_larger(array_list, &new_capacity)
        && cel_arraylist_resize(array_list, new_capacity) == -1)
        return ;
    array_list->items[array_list->size] = item;
    array_list->size++;
}

void *cel_arraylist_pop(CelArrayList *array_list, size_t position)
{
    size_t new_capacity, i;
    void *item;

    if (array_list->size < 0 || position >= array_list->size)
        return NULL;
    
    item = array_list->items[position];
    if ((i = array_list->size - position - 1) > 0)
        memmove(&(array_list->items[position]), 
        &(array_list->items[position + 1]), i * sizeof(void *));
    array_list->size--;
    if (cel_arraylist_maybe_smaller(array_list, &new_capacity)
        && cel_arraylist_resize(array_list, new_capacity) == -1)
        return NULL;
    return item;
}

void *cel_arraylist_pop_back(CelArrayList *array_list)
{
    size_t new_capacity;
    void *item;

    if (array_list->size < 0)
        return NULL;
    array_list->size--;
    item = array_list->items[array_list->size];
    if (cel_arraylist_maybe_smaller(array_list, &new_capacity)
        && cel_arraylist_resize(array_list, new_capacity) == -1)
        return NULL;
    return item;
}

int cel_arraylist_foreach(CelArrayList *array_list, 
                          CelEachFunc each_func, void *user_data)
{
    int ret;
    size_t i;

    for (i = 0; i < array_list->size; i++)
    {
        //_tprintf(_T("%x "), array_list->items[i]);
        if ((ret = each_func(array_list->items[i], user_data)) != 0)
            return ret;
    }

    return 0;
}

void cel_arraylist_clear(CelArrayList *array_list)
{
    size_t new_capacity;

    for (; array_list->size > 0; array_list->size--)
    {
        if (array_list->free_func != NULL)
            array_list->free_func(array_list->items[array_list->size - 1]);
    }
    if (cel_arraylist_maybe_smaller(array_list, &new_capacity))
        cel_arraylist_resize(array_list, new_capacity);
}
