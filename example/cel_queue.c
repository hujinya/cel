#include "cel/queue.h"
#include "cel/timer.h"

#define BUFFER_SIZE 100000

int queue_print(void *value, void *user_data)
{
    printf("%d ", (*(int *)value));

    return 0;
}


int queue_test(int argc, TCHAR *argv[])
{
    int i;
    int *buf = (int *)malloc(BUFFER_SIZE * sizeof(int));
    CelQueue *queue = cel_queue_new(NULL);
    unsigned long start_ticks = cel_getticks();

    for (i = 0; i < BUFFER_SIZE; i++)
        buf[i] = i;
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        cel_queue_push_back(queue, &buf[i]);
    }
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        cel_queue_pop_front(queue);
    }
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        cel_queue_push_front(queue, &buf[i]);
    }
    cel_queue_foreach(queue, queue_print, NULL);
    cel_queue_free(queue);
    _tprintf(_T("\r\nTicks %ld\r\n"), cel_getticks() - start_ticks);

    return 0;
}
