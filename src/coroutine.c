#include "cel/coroutine.h"
#include "cel/allocator.h"
#include "cel/multithread.h"

CelCoroutineScheduler *_cel_coroutinescheduler_get()
{
    CelCoroutineScheduler *co_schd;

    if ((co_schd = (CelCoroutineScheduler *)
        cel_multithread_get_keyvalue(CEL_MT_KEY_COROUTINE)) == NULL)
    {
        if ((co_schd = cel_coroutinescheduler_new()) != NULL)
        {
            if (cel_multithread_set_keyvalue(
                CEL_MT_KEY_COROUTINE, co_schd) != -1
                && cel_multithread_set_keydestructor(
                CEL_MT_KEY_COROUTINE, 
                (CelFreeFunc)cel_coroutinescheduler_free) != -1)
                return co_schd;
            cel_coroutinescheduler_free(co_schd);
        }
        return NULL;
    }
    return co_schd;
}
