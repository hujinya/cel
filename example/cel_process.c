#include "cel/process.h"
#ifdef _CEL_UNIX
static int process_each(CelProcessStat *proc_stat, void *user_data)
{
    printf("%s %c %d %d\r\n", proc_stat->name, proc_stat->state, 
        proc_stat->pid, proc_stat->ppid);

    return 0;
}

int process_test(int argc, TCHAR *argv[])
{
    cel_process_foreach(process_each, NULL);

    return 0;
}
#endif
