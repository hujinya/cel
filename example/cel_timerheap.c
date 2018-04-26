#include "cel/timerheap.h"

#define TIMER_NUM   100

void timerheap_timer_callback(void *user_data)
{
    printf("Timer %d expired.\r\n", *((int *)user_data));
}

int timerheap_test(int argc ,const char *argv[])
{
    int i, j;
    int timerid[TIMER_NUM];
    CelTimer *timer[TIMER_NUM], *expired_timer[2];
    CelTimerHeap *timer_heap;

    timer_heap = cel_timerheap_new(NULL);
    for (i = 0; i < TIMER_NUM; i++)
    {
        timerid[i] = i;
        timer[i] = cel_timer_new((TIMER_NUM - i) * 1000 + 9,
            0, timerheap_timer_callback, &timerid[i]);
        //printf("%x ", &timerid[i]);
        cel_timerheap_push(timer_heap, timer[i]);
    }
    while (!cel_timerheap_is_empty(timer_heap))
    {
        if ((i = cel_timerheap_pop_expired(timer_heap, expired_timer, 2, NULL)) > 0)
        {
            for (j = 0; j < i; j++)
                expired_timer[j]->call_back(expired_timer[j]->user_data);
        }
    }

    return 0;
}
