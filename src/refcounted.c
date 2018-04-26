#include "cel/refcounted.h"
#include "cel/allocator.h"

int cel_refcounted_init(CelRefCounted *ref_counted, CelFreeFunc free_func)
{
    ref_counted->cnt = 1;
    ref_counted->free_func = free_func;

    return 0;
}

void cel_refcounted_destroy(CelRefCounted *ref_counted, void *ptr)
{
    if (cel_atomic_dec(&(ref_counted->cnt)) <= 0)
    {
        if (ref_counted->free_func != NULL)
            ref_counted->free_func(ptr);
    }
}

CelRefCounted *cel_refcounted_ref(CelRefCounted *ref_counted)
{
    cel_atomic_inc(&(ref_counted->cnt));
    return ref_counted;
}

void *cel_refcounted_ref_ptr(CelRefCounted *ref_counted, void *ptr)
{
    cel_atomic_inc(&(ref_counted->cnt));
    return ptr;
}

void cel_refcounted_deref(CelRefCounted *ref_counted, void *ptr)
{
    if (cel_atomic_dec(&(ref_counted->cnt)) <= 0)
    {
        if (ref_counted->free_func != NULL)
            ref_counted->free_func(ptr);
    }
}
