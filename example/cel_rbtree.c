#include "cel/rbtree.h"

#define BUFFER_SIZE 100

int rbtree_print(void *key, void *value, void *user_data)
{
    printf("%d ", (*(int *)key));

    return 0;
}

int rbtree_int_compare(const int *str, const int *dest)
{
    if (*str > *dest) return 1;
    else if (*str < *dest) return -1;
    return 0;
}

int rbtree_test(int argc, TCHAR *argv[])
{
    int i;
    int buf[BUFFER_SIZE];
    CelRbTree *rbtree = cel_rbtree_new((CelCompareFunc)rbtree_int_compare, NULL, NULL);

    for (i = 0; i < BUFFER_SIZE; i++)
        buf[i] = BUFFER_SIZE - i;
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        printf("Insert value %d\r\n",  buf[i]);
        cel_rbtree_insert(rbtree, &buf[i], NULL);  
    }
    cel_rbtree_foreach(rbtree, rbtree_print, NULL);
    puts("\r\n");
    for (i = BUFFER_SIZE - 1; i > 0; i--)
    {
        printf("Remove value %d\r\n", buf[i]);
        cel_rbtree_remove(rbtree, &buf[i]);
        i--;
    }
    cel_rbtree_foreach(rbtree, rbtree_print, NULL);
    puts("\r\n");
    cel_rbtree_free(rbtree);

    return 0;
}
