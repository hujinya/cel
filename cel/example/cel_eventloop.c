#include "cel/multithread.h"
#include "cel/error.h"
#include "cel/eventloop.h"

#define TIMER_MAX 1000

void timer_expired(void *user_data)
{
    printf("Timer expired %d, pid %d\r\n", (*(int *)user_data), (int)cel_thread_getid());
    usleep(1);
}

int eventloop_run(void *loop)
{
    cel_eventloop_run((CelEventLoop *)loop);
    cel_thread_exit(0);

    return 0;
}

int eventloop_test(int argc, TCHAR *argv[])
{
    CelEventLoop *loop;
    CelThread td[64];
    int n, i[TIMER_MAX];
    unsigned long start, end;
    CelTimerId t[TIMER_MAX];
    void *status;

    int j = 10;

    start = cel_getticks();
    cel_multithread_support();
    while ((--j) >= 0)
    {
        if ((loop = cel_eventloop_new(10, 1024)) == NULL)
        {
            _putts(cel_geterrstr(cel_geterrno()));
            return -1;
        }
        _tprintf(_T("New loop ok j = %d\r\n"), j);
        for (n = 0; n < TIMER_MAX; n++)
        {
            i[n] = n;
            t[n] = cel_eventloop_schedule_timer(loop, 1000 + n * 5, 1, timer_expired, &i[n]);
        }
        for (n = 0; n < 10; n++)
            cel_thread_create(&td[n], NULL, eventloop_run, loop);
        n = 0;
        while (n < TIMER_MAX / 2)
        {
            usleep(20 * 1000);
            cel_timer_stop(t[(n++ * 2)], NULL);
        }
        sleep(2);
        cel_eventloop_exit(loop);
        end = cel_getticks();
        _tprintf(_T("exit j = %d %ld\r\n"), j, end - start);
        for (n = 0; n < 10; n++)
            cel_thread_join(td[n], &status);
        _tprintf(_T("exit ok j = %d\r\n"), j);
        cel_eventloop_free(loop);
    }

    return 0;
}
