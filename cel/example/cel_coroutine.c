#include "cel/coroutine.h"  
  
void test3(void *ud)  
{  
    int *data = (int*)ud;
    CelCoroutine co = cel_coroutine_self();
    //char x[1024];

    while ((*data)-- >= 0)
    {
        printf("test3 data =%d\n",*data); 
        cel_coroutine_yield(&co);
    }
}
  
void coroutine_test1()  
{  
    int a = 11;  
    CelCoroutine co1, co2, co3;
  
    cel_coroutine_create(&co1, NULL, test3, &a);
    cel_coroutine_create(&co2, NULL, test3, &a);
    printf("cel_coroutine_test3 begin\n");
    while (cel_coroutine_status(&co1)
        && cel_coroutine_status(&co2))
    {  
        printf("\nresume co id = %p.\n",co1);
        cel_coroutine_resume(&co1);  
        printf("resume co id = %p.\n", co2);
        cel_coroutine_resume(&co2);  
    }  
  
    cel_coroutine_create(&co3, NULL, test3, &a);
    while (cel_coroutine_status(&co3))  
    {  
        printf("\nresume co id = %p.\n", co3);
        cel_coroutine_resume(&co3);
    }  
    printf("cel_coroutine_test3 end\n");
}

int coroutine_test()  
{  
    coroutine_test1();  
    return 0;  
}  
