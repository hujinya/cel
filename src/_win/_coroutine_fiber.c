#include "cel/coroutine.h"
#include "cel/allocator.h"
#ifdef _CEL_WIN

void __stdcall _os_coroutine_main(OsCoroutineScheduler *schd)
{
    OsCoroutine *co = schd->co[schd->co_running];

    (co->func)(co->user_data);

    schd->co_running = -1;
    --schd->co_num;
    schd->co[schd->co_running] = NULL;
    //_coroutine_free(co);
    SwitchToFiber(schd->main_ctx);
}

OsCoroutine *_coroutine_new(OsCoroutineScheduler *schd, 
                            OsCoroutineFunc func, void *user_data) 
{
    OsCoroutine *co;

    if ((co = cel_malloc(sizeof(OsCoroutine))) != NULL)
    {
        co->schd = schd;
        co->status = CEL_COROUTINE_READY;
        co->func = func;
        co->user_data = user_data;
        co->ctx = CreateFiberEx(CEL_COROUTINE_STACK_SIZE, 0, 
            FIBER_FLAG_FLOAT_SWITCH, _os_coroutine_main, schd);
        co->id = -1;
        co->status = CEL_COROUTINE_READY;
        return co;
    }
    return NULL;
}

static void _coroutine_free(OsCoroutine *co)
{
    // If the currently running fiber calls DeleteFiber, 
    // its thread calls ExitThread and terminates.  
    // However, if a currently running fiber is deleted by another fiber,
    // the thread running the   
    // deleted fiber is likely to terminate abnormally 
    // because the fiber stack has been freed.  
    DeleteFiber(co->ctx);
    free(co);
}

OsCoroutineScheduler *os_coroutinescheduler_new()
{
    OsCoroutineScheduler *schd;

    if ((schd = cel_malloc(sizeof(OsCoroutineScheduler))) != NULL)
    {
        schd->co_capacity = CEL_COROUTINE_CAP;
        schd->co_num = 0;
        schd->co_running = -1;
        if ((schd->co = cel_malloc(
            sizeof(OsCoroutine *) * schd->co_capacity)) != NULL)
        {
            memset(schd->co, 0, sizeof(OsCoroutine *) * schd->co_capacity);
            schd->main_ctx = 
                ConvertThreadToFiberEx(NULL, FIBER_FLAG_FLOAT_SWITCH);
            return schd;
        }
        cel_free(schd);
    }
    return NULL;
}

void os_coroutinescheduler_free(OsCoroutineScheduler *schd)
{
    int i;

    for (i = 0; i < schd->co_capacity; i++)
    {  
        if (schd->co[i] != NULL) 
            _coroutine_free(schd->co[i]);
    }  
    cel_free(schd->co);
    schd->co = NULL;
    cel_free(schd);
}

int os_coroutinescheduler_add(OsCoroutineScheduler *schd, OsCoroutine *co)
{
    int i, co_id, new_cap;

    if (schd->co_num >= schd->co_capacity)
    {
        new_cap = schd->co_capacity * 2;
        if ((schd->co = (OsCoroutine **)
            realloc(schd->co, new_cap * sizeof(OsCoroutine *))) == NULL)
        {
            return -1;
        }
        memset(schd->co + schd->co_capacity , 
            0 , sizeof(OsCoroutine *) * schd->co_capacity);
        schd->co_capacity *= 2;
        co_id = schd->co_num;
        schd->co[co_id] = co;
        ++schd->co_num;
         return co_id;
    } 
    else 
    {
        for (i = 0;i < schd->co_capacity;i++)
        {
            co_id = ((i + schd->co_num) % schd->co_capacity);
            if (schd->co[co_id] == NULL) 
            {
                schd->co[co_id] = co;
                ++schd->co_num;
                return co_id;
            }
        }
    }
    return -1;
}

int os_coroutine_create(OsCoroutine *co, OsCoroutineScheduler *schd,
                        OsCoroutineAttr *attr,
                        OsCoroutineFunc func, void *user_data)
{
    if ((co = _coroutine_new(schd, func, user_data)) != NULL)
    {
        if ((co->id = os_coroutinescheduler_add(schd, co)) != -1)
            return co->id;
        _coroutine_free(co);
    }
    return -1;
}

void os_coroutine_resume(OsCoroutine *co)
{
    OsCoroutineScheduler *schd = co->schd;

    switch (co->status)
    {  
    case CEL_COROUTINE_READY:
    case CEL_COROUTINE_SUSPEND:
        co->status = CEL_COROUTINE_RUNNING;
        schd->co_running = co->id;
        SwitchToFiber(co->ctx);
        if (schd->co[co->id] != NULL) 
            _coroutine_free(co);
        break;
    default:
        //assert(0);
        break;
    }
}

void os_coroutine_yield(OsCoroutine *co)
{
    OsCoroutineScheduler *schd = co->schd;

    co->status = CEL_COROUTINE_SUSPEND;
    schd->co_running = -1;
    SwitchToFiber(schd->main_ctx);
}

#endif
