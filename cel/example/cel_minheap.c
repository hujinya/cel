#include "cel/minheap.h"

#define BUFFER_SIZE 1000

int minheap_print(void *value, void *user_data)
{
    printf("%d ", (*(int *)value));

    return 0;
}

int minheap_int_compare(const int *str, const int *dest)
{
    //printf("%d(%p), %d(%p)\r\n", **str, *str, **dest, *dest);
    if (*str > *dest) return 1;
    else if (*str < *dest) return -1;
    return 0;
}

int minheap_test(int argc, TCHAR *argv[])
{
    int i, *p = NULL;
    int buf[BUFFER_SIZE];
    CelMinHeap *minheap = cel_minheap_new((CelCompareFunc)minheap_int_compare, NULL);

    for (i = 0; i < BUFFER_SIZE; i++)
        buf[i] = BUFFER_SIZE - i;
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        printf("%p\r\n", &buf[i]);
        cel_minheap_insert(minheap, &buf[i]);
    }
    for (i = 0; i < 10; i++)
    {
        p = (int *)cel_minheap_get_min(minheap);
        printf("Pop %d\r\n", *p);
        cel_minheap_pop_min(minheap);
        cel_minheap_foreach(minheap, minheap_print, NULL);
        puts("\r\n");
    }
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        cel_minheap_insert(minheap, &buf[i]);
    }
    cel_minheap_foreach(minheap, minheap_print, NULL);
    puts("\r\n");
    cel_minheap_free(minheap);

    return 0;
}