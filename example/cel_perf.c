#include "cel/sys/perf.h"

#ifdef __UNIX__
#define XLEFT(x)  printf("\033[%dD", x)
#define YUP(y)    printf("\033[%dA", y)
#define CLL       printf("\033[K")
#else
#define XLEFT(x)  
#define YUP(y)    
#define CLL       
#endif

int perf_test(int argc, TCHAR *argv[])
{
    int i, x = 0, y = 0;
    CelPerf *pi;

    while (1)
    {
        sleep(1);
        pi = cel_getperf(NULL, 0);
        //printf("%ul - %ul %ul\r\n", pi->net.ticks, pi->net.receiving, pi->net.sending);
        while ((--y) >= 0)
        {
            CLL;
            YUP(1);
        }
        XLEFT(x);
        x = y = 0;
        _tprintf(_T("CPU: %lld total, %lld user, %lld system, %lld idle, %d%%(%d%%)usage(max)\r\n"),
            pi->cpu.totaltime, pi->cpu.usertime, pi->cpu.systime, pi->cpu.idletime,
            pi->cpu.usage, pi->cpu.maxusage);
        _putts(_T("  Id   Total         User          System        Idle          Usage(Max)"));
        y += 2;
        for (i = 0; i < pi->cpu.num; i++)
        {
            CLL;
            _tprintf(_T("  %-3d  %-12lld  %-12lld  %-12lld  %-12lld  %d%%(%d%%)\r\n"),
                i,
                pi->cpu.cpus[i]->totaltime, 
                pi->cpu.cpus[i]->usertime, 
                pi->cpu.cpus[i]->systime, 
                pi->cpu.cpus[i]->idletime,
                pi->cpu.cpus[i]->usage, pi->cpu.cpus[i]->maxusage);
            y++;
        }
        _tprintf(_T("Memory: %ldKB total, %ldKB avail, %ldKB cached, %ldKB free, %d%%(%d%%)usage(max)\r\n"), 
            (long)(pi->mem.total >> 10), 
            (long)(pi->mem.available >> 10), (long)(pi->mem.cached >> 10), (long)(pi->mem.free >> 10),
            pi->mem.usage, pi->mem.maxusage);
        y += 1;
        _tprintf(_T("File Systems: %ldMB total, %ldMB avail, %ldMB free, %d%%(%d%%)usage(max)\r\n"), 
            (long)(pi->fs.total >> 20), 
            (long)(pi->fs.available >> 20), (long)(pi->fs.free >> 20),
            pi->fs.usage, pi->fs.maxusage);
        _putts(_T("  Dir               Total(MB)   Avail(MB)   Free(MB)    Usage(Max)"));
        y += 2;
        for (i = 0; i < pi->fs.num; i++)
        {
            if (_tcslen(pi->fs.fss[i]->dir) > 16)
            {
                _tprintf(_T("  %-16s\r\n                    %-10ld  %-10ld  %-10ld  %d%%(%d%%)\r\n"),
                    pi->fs.fss[i]->dir,
                    (long)(pi->fs.fss[i]->total >> 20), 
                    (long)(pi->fs.fss[i]->available >> 20), (long)(pi->fs.fss[i]->free >> 20),
                    pi->fs.fss[i]->usage, pi->fs.fss[i]->maxusage);
                y += 2;
            }
            else
            {
                _tprintf(_T("  %-16s  %-10ld  %-10ld  %-10ld  %d%%(%d%%)\r\n"),
                    pi->fs.fss[i]->dir,
                    (long)(pi->fs.fss[i]->total >> 20), 
                    (long)(pi->fs.fss[i]->available >> 20), (long)(pi->fs.fss[i]->free >> 20),
                    pi->fs.fss[i]->usage, pi->fs.fss[i]->maxusage);
                y++;
            }
        }
        _tprintf(_T("Net: %ldKB received, %ldKB/s(%ldKB/s)receiving(max), ")
            _T("%ldKB sent, %ldKB/s(%ldKB/s)sending(max)\r\n"), 
            (long)(pi->net.received >> 10), 
            (long)(pi->net.receiving >> 10), (long)(pi->net.max_receiving >> 10),
            (long)(pi->net.sent >> 10), 
            (long)(pi->net.sending >> 10), (long)(pi->net.max_sending >> 10));
        x = _putts(_T("  Interface  Received(KB)  Receiving/Max(KB/s) Sent(KB)       Sending/Max(KB/s)"));
        y += 2;
        for (i = 0; i < pi->net.num; i++)
        {
            if (_tcslen(pi->net.nets[i]->ifname) > 9)
            {
                x = _tprintf(_T("  %-9s\r\n             %-12ld   %8ld/%-8ld  %-12ld  %8ld/%-8ld\r\n"), 
                pi->net.nets[i]->ifname,
                (long)(pi->net.nets[i]->received >> 10), 
                (long)(pi->net.nets[i]->receiving >> 10), (long)(pi->net.nets[i]->max_receiving >> 10),
                (long)(pi->net.nets[i]->sent >> 10), 
                (long)(pi->net.nets[i]->sending >> 10), (long)(pi->net.nets[i]->max_sending >> 10));
                y += 2;
            }
            else
            {
                x = _tprintf(_T("  %-9s  %-12ld   %8ld/%-8ld  %-12ld  %8ld/%-8ld\r\n"), 
                    pi->net.nets[i]->ifname,
                    (long)(pi->net.nets[i]->received >> 10), 
                    (long)(pi->net.nets[i]->receiving >> 10), (long)(pi->net.nets[i]->max_receiving >> 10),
                    (long)(pi->net.nets[i]->sent >> 10), 
                    (long)(pi->net.nets[i]->sending >> 10), (long)(pi->net.nets[i]->max_sending >> 10));
                y++;
            }
        }
    }
    return 0;
}
