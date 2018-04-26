#include "cel/coroutine.h"  
  
void test3(void *ud)  
{  
    int *data = (int*)ud;

    printf("test3 data =%d\n",*data); 
    cel_coroutine_yield(cel_coroutine_self());
}
  
void coroutine_test1()  
{  
    int a = 11;  
    CelCoroutine *co1 = NULL, *co2 = NULL, *co3 = NULL;
    int id1 = cel_coroutine_create(co1, NULL, test3, &a);
    int id2 = cel_coroutine_create(co2, NULL, test3, &a);
    int id3;
  
    printf("cel_coroutine_test3 begin\n");  
    while (cel_coroutine_status(co1)
        && cel_coroutine_status(co2))  
    {  
        printf("\nresume co id = %d.\n",id1);  
        cel_coroutine_resume(co1);  
        printf("resume co id = %d.\n", id2);  
        cel_coroutine_resume(co2);  
    }  
  
    id3 = cel_coroutine_create(co3, NULL, test3, &a);  
    while (cel_coroutine_status(co3))  
    {  
        printf("\nresume co id = %d.\n", id3);  
        cel_coroutine_resume(co3);  
    }  
    printf("cel_coroutine_test3 end\n");  
}  
  
  
int coroutine_test()  
{  
    coroutine_test1();  
    return 0;  
}  
