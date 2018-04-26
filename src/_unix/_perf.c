#include "cel/sys/perf.h"
#ifdef _CEL_UNIX
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"
#include "cel/timer.h"
#include "cel/convert.h"
#include "cel/file.h"
#include <ctype.h>
#include <mntent.h>
#include <sys/statfs.h>
#include <sys/times.h>

/* Debug defines */
#define Debug(args)   /*cel_log_debug args*/
#define Warning(args) CEL_SETERRSTR(args)/*cel_log_warning args*/
#define Err(args)   CEL_SETERRSTR(args)/*cel_log_err args*/

CelPerf s_perf = { { 0 }, { 0 }, { 0 }, { 0 }};

#define CPU_PROCFILE    "/proc/stat"
#define MEM_PROCFILE    "/proc/meminfo"
#define NET_PROCFILE    "/proc/net/dev"

CelCpuPerf *cel_getcpuperf(void)
{
    FILE *fp;
    char *p, line[CEL_LINELEN];
    unsigned long long idletime, totaltime, t_idletime, t_totaltime;
    CelCpuPerf *s_ci = &(s_perf.cpu), *c_ci;

    if ((fp = cel_fopen(CPU_PROCFILE, "r")) == NULL)
    {
        return NULL;
    }
    s_ci->num = 0;
    t_idletime = t_totaltime = 0;
    while (fgets(line, CEL_LINELEN, fp) != NULL 
        && (p = cel_skiptoken(line)) != NULL)
    {
        if (strncmp(line, "cpu", 3) != 0 || line[3] == ' ') 
            continue;
        if ((c_ci = s_ci->cpus[s_ci->num]) == NULL
            && (c_ci = s_ci->cpus[s_ci->num] = 
            (CelCpuPerf *)cel_calloc(1, sizeof(CelCpuPerf))) == NULL)
            break;
        c_ci->usertime = strtoull(p, &p, 10);   /**< user_jiffies */
        c_ci->usertime += strtoull(p, &p, 10);  /**< nice_jiffies */
        c_ci->systime = strtoull(p, &p, 10);    /**< system_jiffies */
        idletime = strtoull(p, &p, 10);         /**< idle_jiffies */
        totaltime = c_ci->usertime + c_ci->systime + idletime;
        if (totaltime != c_ci->totaltime)
        {
            if ((c_ci->maxusage < (c_ci->usage = 100 - 
                (BYTE) ((idletime - c_ci->idletime) * 100
                / (totaltime - c_ci->totaltime)))))
                c_ci->maxusage = c_ci->usage;
        }
        s_ci->systime += c_ci->systime;
        s_ci->usertime += c_ci->usertime;
        t_idletime += (c_ci->idletime = idletime);
        t_totaltime += (c_ci->totaltime = totaltime);
        if (++s_ci->num >= CEL_PRONUM) break;
    }
    cel_fclose(fp);
    if (t_totaltime != s_ci->totaltime)
    {
        s_ci->usage = 100 - (BYTE)((t_idletime - s_ci->idletime) * 100 
            / (t_totaltime - s_ci->totaltime));
        if (s_ci->maxusage < s_ci->usage) 
            s_ci->maxusage = s_ci->usage;
    }
    s_ci->idletime = t_idletime;
    s_ci->totaltime = t_totaltime;

    return s_ci;
}

static unsigned long long cel_getmemvalue(char *p)
{
    unsigned long long value;
    char *token;

    value = strtoul(p, &token, 10);
    while (*token == ' ') 
        ++token;
    if (*token == 'k') 
        value *= 1024;
    else if (*token == 'M')
        value *= (1024 * 1024);
    return value;
}

CelMemPerf *cel_getmemperf(void)
{
    char *p, line[CEL_LINELEN];
    FILE *fp = NULL;
    CelMemPerf *s_mi = &(s_perf.mem);

    /* Get system wide memory usage */
    if ((fp = cel_fopen(MEM_PROCFILE, "r")) == NULL)
    {
        return NULL;
    }
    while (fgets(line, CEL_LINELEN, fp) != NULL)
    {
        if (memcmp(line, "MemTotal:", 9) == 0)
        {
            p = line + 9;
            s_mi->total = cel_getmemvalue(p);

            while (fgets(line, CEL_LINELEN, fp) != NULL)
            {
                if (memcmp(line, "MemFree:", 8) == 0)
                {
                    p = line + 8;
                    s_mi->free = cel_getmemvalue(p);;

                    while (fgets(line, CEL_LINELEN, fp) != NULL)
                    {
                        if (memcmp(line, "Buffers:", 8) == 0)
                        {
                            //puts(line);
                            p = line + 8;
                            s_mi->cached = cel_getmemvalue(p);

                            while (fgets(line, CEL_LINELEN, fp) != NULL)
                            {
                                if (memcmp(line, "Cached:", 7) == 0)
                                {
                                    p = line + 7;
                                    s_mi->cached += cel_getmemvalue(p);

                                    s_mi->available = s_mi->free + s_mi->cached;
                                    if (s_mi->total != 0)
                                    {
                                        s_mi->usage = 
                                            (BYTE)((s_mi->total - s_mi->available) 
                                            * 100 / s_mi->total);
                                        if (s_mi->maxusage < s_mi->usage)
                                            s_mi->maxusage = s_mi->usage;
                                        cel_fclose(fp);
                                        return s_mi;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    cel_fclose(fp);
    Err(("Read file \"%s\" error.", MEM_PROCFILE));

    return NULL;
}

CelFsPerf *cel_getfsperf(void)
{  
    FILE* mount_table;
    struct mntent *mount_entry;
    struct statfs fs_stat;
    CelFsPerf *c_hi, *s_hi = &(s_perf.fs);

    if ((mount_table = setmntent(MOUNTED, "r")) == NULL)
    {
        return NULL;
    }
    s_hi->total = s_hi->available = s_hi->free = 0;
    s_hi->num = 0;
    while ((mount_entry = getmntent(mount_table)) != NULL)
    {
        if (strncmp(mount_entry->mnt_fsname, "/dev/", 5) != 0 
            || statfs(mount_entry->mnt_dir, &fs_stat) != 0)
            continue;
       /* printf("%s %s %s\r\n", 
            mount_entry->mnt_type, mount_entry->mnt_opts, c_hi->fsname);*/
        if ((c_hi = s_hi->fss[s_hi->num]) == NULL
            && (c_hi = (s_hi->fss[s_hi->num] = 
            (CelFsPerf *)cel_malloc(sizeof(CelFsPerf)))) == NULL)
            break;
        strncpy(c_hi->fsname, mount_entry->mnt_fsname, CEL_FNLEN);
        strncpy(c_hi->dir, mount_entry->mnt_dir, CEL_DIRLEN);
        c_hi->total = fs_stat.f_blocks * fs_stat.f_bsize;
        c_hi->available = fs_stat.f_bavail * fs_stat.f_bsize;
        c_hi->free = fs_stat.f_bfree * fs_stat.f_bsize;
        if (c_hi->total != 0)
        {
            if (c_hi->maxusage < (c_hi->usage = (BYTE)
                ((c_hi->total - c_hi->free) * 100 / c_hi->total)))
                c_hi->maxusage = c_hi->usage;
        }
        s_hi->total += c_hi->total;
        s_hi->available += c_hi->available;
        s_hi->free += c_hi->free;
        if (++s_hi->num >= CEL_HDNUM) break; 
    }
    endmntent(mount_table);
    if (s_hi->total != 0)
    {
        if (s_hi->maxusage < (s_hi->usage = (BYTE)
            ((s_hi->total - s_hi->free) * 100 / s_hi->total)))
            s_hi->maxusage = s_hi->usage;
    }  
    
    return s_hi;
}

CelNetPerf *cel_getnetperf(void)
{
    int i;
    char *p, line[CEL_LINELEN], ifname[CEL_IFNLEN];
    FILE *fp;
    unsigned long received, sent;
    unsigned long t_received, t_sent;
    unsigned long ticks;
    CelNetPerf *s_ni = &(s_perf.net), *c_ni;

    if ((fp = cel_fopen(NET_PROCFILE, "r")) == NULL)
    {
        return NULL;
    }
    ticks = cel_getticks();
    t_received = t_sent = 0;
    s_ni->num = 0;
    while (fgets(line, CEL_LINELEN, fp) != NULL)
    {
        p = line; i = 0;
        while (*p != '\0' && i < CEL_IFNLEN)
        {
            if (*p == ':') break;
            if (*p != ' ') ifname[i++] = *p;
            p++;
        }
        if (*(p++) != ':' || strncmp(ifname, "lo", i) == 0)      /* lo */
            continue;
        if ((c_ni = s_ni->nets[s_ni->num]) == NULL
            && (c_ni = s_ni->nets[s_ni->num] = 
                (CelNetPerf *)cel_malloc(sizeof(CelNetPerf))) == NULL)
            break;
        strncpy(c_ni->ifname, ifname ,i);
        received = strtoul(p, &p, 10);
        if ((p = cel_skiptoken(p)) == NULL         /* packets */
            || (p = cel_skiptoken(p)) == NULL      /* errs */
            || (p = cel_skiptoken(p)) == NULL      /* drop */
            || (p = cel_skiptoken(p)) == NULL      /* fifo */
            || (p = cel_skiptoken(p)) == NULL      /* frame */
            || (p = cel_skiptoken(p)) == NULL      /* compressed */
            || (p = cel_skiptoken(p)) == NULL)     /* multicast */
        { 
            Err(("Skip token return null."));
            return NULL;
        }
        sent = strtoul(p, &p, 10);
        if (ticks != c_ni->ticks)
        {
            c_ni->receiving = CEL_DIFF(received, c_ni->received) * ULL(1000) 
                / CEL_DIFF(ticks, c_ni->ticks);
            c_ni->sending = CEL_DIFF(sent, c_ni->sent) * ULL(1000) 
                / CEL_DIFF(ticks, c_ni->ticks);
            if (c_ni->max_receiving < c_ni->receiving)
                c_ni->max_receiving = c_ni->receiving;
            if (c_ni->max_sending < c_ni->sending)
                c_ni->max_sending = c_ni->sending; 
        }
        c_ni->ticks = ticks;
        t_received += (c_ni->received = received);
        t_sent += (c_ni->sent = sent);
        if ((++s_ni->num) >= CEL_NETDEVNUM) break;
    }
    cel_fclose(fp);
    if (ticks != s_ni->ticks)
    {
        s_ni->receiving = CEL_DIFF(t_received, s_ni->received) * ULL(1000) 
            / CEL_DIFF(ticks, s_ni->ticks);
        s_ni->sending = CEL_DIFF(t_sent, s_ni->sent) * ULL(1000) 
            / CEL_DIFF(ticks, s_ni->ticks);
        if (s_ni->max_receiving < s_ni->receiving)
            s_ni->max_receiving = s_ni->receiving;
        if (s_ni->max_sending < s_ni->sending)
            s_ni->max_sending = s_ni->sending; 
    }
    s_ni->ticks = ticks;
    s_ni->received = t_received;
    s_ni->sent = t_sent;

    return &s_perf.net;
}

#endif
