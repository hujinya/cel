#include "cel/timerwheel.h"

#define TIMER_NUM   200

void timerwheel_timer_callback(void *user_data)
{
    printf("Timer %d expired.\r\n", *((int *)user_data));
}

int timerwheel_test(int argc ,const char *argv[])
{
    int i, j;
    int timerid[TIMER_NUM];
    CelTimer *timer[TIMER_NUM], *expired_timer[2];
    CelTimerWheel *timer_wheel;

    timer_wheel = cel_timerwheel_new(NULL);
    printf("Time wheel size %ld\r\n", sizeof(CelTimerWheel));
    for (i = 0; i < TIMER_NUM; i++)
    {
        timerid[i] = i;
        timer[i] = cel_timer_new((TIMER_NUM - i) * 2000 + 9,
            0, timerwheel_timer_callback, &timerid[i]);
        cel_timerwheel_push(timer_wheel, timer[i]);
    }
    while (!cel_timerwheel_is_empty(timer_wheel))
    {
        if ((i = cel_timerwheel_pop_expired(timer_wheel, expired_timer, 2, NULL)) > 0)
        {
            for (j = 0; j < i; j++)
                expired_timer[j]->call_back(expired_timer[j]->user_data);
        }
        usleep(10 * 1000);
    }
    cel_timerwheel_free(timer_wheel);

    return 0;
}
