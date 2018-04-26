#include "cel/atomic.h"
#include "cel/multithread.h"

#ifdef CEL_ARCH_ATOMIC_INC
long cel_arch_atomic_inc(CelAtomic *v)
{ 
    cel_multithread_mutex_lock(CEL_MT_MUTEX_ATOMIC);
    (*v)++;
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_ATOMIC);
    return *v;
}
#endif

#ifdef CEL_ARCH_ATOMIC_DEC
long cel_arch_atomic_dec(CelAtomic *v)
{
    cel_multithread_mutex_lock(CEL_MT_MUTEX_ATOMIC);
    (*v)--;
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_ATOMIC);

    return *v;
}
#endif

#ifdef CEL_ARCH_ATOMIC_ADD
long cel_arch_atomic_add(CelAtomic *v, long i)
{
    cel_multithread_mutex_lock(CEL_MT_MUTEX_ATOMIC);
    (*v)--;
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_ATOMIC);

    return *v;
}
#endif

#ifdef CEL_ARCH_ATOMIC_SUB
long cel_arch_atomic_sub(CelAtomic *v, long i)
{
    cel_multithread_mutex_lock(CEL_MT_MUTEX_ATOMIC);
    (*v)--;
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_ATOMIC);

    return *v;
}
#endif

#ifdef CEL_ARCH_ATOMIC_OR
long cel_arch_atomic_or(CelAtomic *v, long i)
{
    cel_multithread_mutex_lock(CEL_MT_MUTEX_ATOMIC);
    *v = *v|i;
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_ATOMIC);
    return *v;
}
#endif

#ifdef CEL_ARCH_ATOMIC_AND
long cel_arch_atomic_and(CelAtomic *v, long i)
{
    cel_multithread_mutex_lock(CEL_MT_MUTEX_ATOMIC);
    *v = *v & i;
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_ATOMIC);

    return *v;
}
#endif

#ifdef CEL_ARCH_ATOMIC_XOR
long cel_arch_atomic_xor(CelAtomic *v, long i)
{
    cel_multithread_mutex_lock(CEL_MT_MUTEX_ATOMIC);
    *v = *v ^ i;
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_ATOMIC);

    return *v;
}
#endif
