#include "cel/hashtable.h"

#define BUFFER_SIZE 1000

int hashtable_print(void *key, void *value, void *user_data)
{
    printf("%d ", (*(int *)key));

    return 0;
}

int hashtable_test(int argc, TCHAR *argv[])
{
    int i;
    int buf[BUFFER_SIZE];
    CelHashTable *hashtable = cel_hashtable_new(cel_int_hash, cel_int_equal, NULL, NULL);

    for (i = 0; i < BUFFER_SIZE; i++)
        buf[i] = BUFFER_SIZE - i;
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        printf("Insert value %d\r\n",  buf[i]);
        cel_hashtable_insert(hashtable, &buf[i], NULL);  
    }
    cel_hashtable_foreach(hashtable, hashtable_print, NULL);
    puts("\r\n");
    for (i = BUFFER_SIZE - 1; i > 0; i--)
    {
        printf("Remove value %d\r\n", buf[i]);
        cel_hashtable_remove(hashtable, &buf[i]);
    }
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        printf("Insert value %d\r\n",  buf[i]);
        cel_hashtable_insert(hashtable, &buf[i], NULL);  
    }
    cel_hashtable_foreach(hashtable, hashtable_print, NULL);
    puts("\r\n");
    cel_hashtable_free(hashtable);

    return 0;
}
