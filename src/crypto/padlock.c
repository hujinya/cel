#include "cel/crypto/_padlock.h"

#ifdef CEL_PADLOCK
/*
 * PadLock detection routine
 */
int cel_padlock_supports(int feature)
{
    static int flags = -1;
    int ebx, edx;

    if( flags == -1 )
    {
        asm("movl  %%ebx, %0           \n"     \
            "movl  $0xC0000000, %%eax  \n"     \
            "cpuid                     \n"     \
            "cmpl  $0xC0000001, %%eax  \n"     \
            "movl  $0, %%edx           \n"     \
            "jb    unsupported         \n"     \
            "movl  $0xC0000001, %%eax  \n"     \
            "cpuid                     \n"     \
            "unsupported:              \n"     \
            "movl  %%edx, %1           \n"     \
            "movl  %2, %%ebx           \n"
            : "=m" (ebx), "=m" (edx)
            :  "m" (ebx)
            : "eax", "ecx", "edx" );

        flags = edx;
    }

    return( flags & feature );
}

#endif
