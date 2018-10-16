#include "cel/threadpool.h"
#include "cel/multithread.h"
#include "cel/error.h"

void thread_worker(void *data, void *user_data)
{
   _tprintf(_T("Task %d is running by thread [%ld].\r\n"), 
       *((int*)data), cel_thread_getid());
   //_putts(cel_geterrstr(cel_geterrno()));
   sleep(1);
}

int threadpool_test(int argc, TCHAR *argv[])
{
    int i, data[100];
    CelThreadPool *tp;

    cel_multithread_support();
    tp = cel_threadpool_new(thread_worker, NULL, 8, FALSE);
    cel_threadpool_set_max_unused_threads(4);
    for (i = 0; i < 10; i++)
    {
        data[i] = i;
        cel_threadpool_push_task(tp, &data[i]);
    };
    sleep(5);
    _tprintf(_T("Thread pool unprocessed task %d, thread num %d.\r\n"), 
        cel_threadpool_get_unprocessed(tp), cel_threadpool_get_num_threads(tp));
    for (i = 0; i < 100; i++)
    {
        data[i] = i;
        cel_threadpool_push_task(tp, &data[i]);
    };
    _tprintf(_T("Thread pool unprocessed task %d, thread num %d.\r\n"), 
        cel_threadpool_get_unprocessed(tp), cel_threadpool_get_num_threads(tp));
    cel_threadpool_free(tp, FALSE, TRUE);

    return 0;
}
