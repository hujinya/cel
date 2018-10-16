#include "cel/allocator.h"
#include "cel/multithread.h"

int allocator_run(void *data)
{
    int i = 0;
    char *mem[100];

    for (i = 0 ;i < 100; i++)
    {
        usleep(10);
        if ((mem[i] = (char *)cel_malloc((i * 3) * 1024)) != NULL)
            memset(mem[i], 0, (i * 3) * 1024);
    }
    for (i = 0 ;i < 100; i++)
    {
        //printf("i=%d\r\n", i);
        usleep(10);
        if (mem[i] != NULL)
            cel_free(mem[i]);
    }
    for (i = 0 ;i < 100; i++)
    {
        usleep(10);
        if ((mem[i] = (char *)cel_malloc((i * 3) * 1024)) != NULL)
            memset(mem[i], 0, (i * 3) * 1024);
    }
    for (i = 0 ;i < 100; i++)
    {
        //printf("i=%d\r\n", i);
        usleep(10);
        if (mem[i] != NULL)
            cel_free(mem[i]);
    }
    printf("%d exit\r\n", *((int*)data));
    cel_thread_exit(0);

    return 0;
}

int allocator_test(int argc, TCHAR *argv[])
{
    CelThread td[64];
    int n, x[64];
    void *status;

    cel_multithread_support();
    cel_allocator_hook_register(NULL, NULL, NULL);
    for (n = 0; n < 12; n++)
    {
        x[n] = n;
        cel_thread_create(&td[n], NULL, allocator_run, &x[n]);
    }
    for (n = 0; n < 12; n++)
    {
        cel_thread_join(td[n], &status);
        printf("%d join\r\n", n);
    }
    cel_allocator_hook_unregister();

    return 0;
}