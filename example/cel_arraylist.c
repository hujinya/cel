#include "cel/arraylist.h"
#include "cel/timer.h"

#define BUFFER_SIZE 10000

int arraylist_print(void *value, void *user_data)
{
    _tprintf(_T("%d "), (*(int *)value));

    return 0;
}

int arraylist_test(int argc, TCHAR *argv[])
{
    int i, *p;
    int buf[BUFFER_SIZE];
    CelArrayList *array_list = cel_arraylist_new(NULL);
    unsigned long start_ticks = cel_getticks();

    for (i = 0; i < BUFFER_SIZE; i++)
        buf[i] = i;
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        //_tprintf("0x%x ",  &buf[i]);
        cel_arraylist_push_back(array_list, &buf[i]);
    }
    cel_arraylist_foreach(array_list, arraylist_print, NULL);
    _putts(_T("\r\n"));
    p = (int *)cel_arraylist_get_front(array_list);
    _tprintf(_T("p = %d\r\n"), *p);
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        cel_arraylist_pop_back(array_list);
    }
    //_putts(_T("xx"));
    for (i = 0; i < BUFFER_SIZE/2; i++)
    {
        cel_arraylist_push(array_list, 0, &buf[i]);
    }
    cel_arraylist_foreach(array_list, arraylist_print, NULL);
    for (i = 0; i < BUFFER_SIZE/2; i++)
    {
        cel_arraylist_push(array_list, i, &buf[i]);
    }
    
    for (i = 0; i < BUFFER_SIZE/2; i++)
    {
        cel_arraylist_pop(array_list, 0);
    }
    cel_arraylist_foreach(array_list, arraylist_print, NULL);
    _putts(_T("\r\n"));
    for (i = 0; i < BUFFER_SIZE/2; i++)
    {
        cel_arraylist_pop(array_list, i);
    }

    _tprintf(_T("Ticks %ld\r\n"), cel_getticks() - start_ticks);
    cel_arraylist_free(array_list);

    return 0;
}
