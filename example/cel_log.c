#include "cel/log.h"
#include "cel/thread.h"
#include "cel/multithread.h"

int cel_log_write(void *user_data)
{
    int i = 0;
    while (TRUE)
    {
        while (i++ < 200000)
        {
            //if (i == 5)
                //usleep(10 * 1000);
            cel_log_debug(_T("Thread %ld log msg %d"), cel_thread_getid(), i);
        }
        usleep(100 * 1000);
    }
    return 0;
}

int log_test(int argc, TCHAR *argv[])
{
    int n;
    CelThread td[64];

    //cel_log_msghook_register
    cel_multithread_support();
    for (n = 0; n < 64; n++)
        cel_thread_create(&td[n], NULL, cel_log_write, NULL);
    while (TRUE)
    {
        usleep(10 * 1000);
        cel_log_flush();
        //printf("drop = %d\r\n", g_logger.n_drop);
    }
    return 0;
}

