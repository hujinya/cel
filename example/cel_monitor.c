#include "cel/multithread.h"
#include "cel/net/socket.h"
#include "cel/net/monitor.h"
#include "cel/error.h"
#include "cel/timer.h"
#include "cel/eventloop.h"

#define MONITOR_MAX 8

typedef enum _MonitorState
{
    MONITOR_DISCONNECTED,
    MONITOR_CONNECTING,
    MONITOR_CONNECTED,
    MONITOR_SENDING,
    MONITOR_RECVING,
    MONITOR_OK
}MonitorState;

typedef struct _Monitor
{
    CelSocketAsyncArgs args;
    BOOL active;
    MonitorState state;
    int down_cnt;
    CelTimerId timer_id;
    CelSocket clt_sock;
    CelSockAddr addr;
    CelAsyncBuf async_buf;
    char buf[1024];
    size_t offset, size;
}Monitor;

static CelEventLoop evt_loop;

void monitor_check(Monitor *monitor)
{
    CelSocketAsyncArgs *args = &(monitor->args);

    /*_tprintf(_T("** %s %d state %d\r\n"), 
            cel_sockaddr_ntop(&(monitor->addr)), monitor->clt_sock.fd, monitor->state);*/
    switch (monitor->state)
    {
    case MONITOR_DISCONNECTED:
        monitor->state = MONITOR_CONNECTING;
        if (cel_socket_init(&(monitor->clt_sock), AF_INET, SOCK_STREAM, IPPROTO_TCP) == 0)
        {
            //puts("dd");
            if (cel_socket_set_nonblock(&(monitor->clt_sock), 1) == 0
                && cel_eventloop_add_channel(
                &evt_loop, (CelChannel *)&(monitor->clt_sock), NULL) == 0)
            {
                //puts("xx");
                memset(&(args->connect_args.ol), 0, sizeof(sizeof(CelOverLapped)));
                args->connect_args.async_callback = (void (*) (void *))monitor_check;
                args->connect_args.socket = &(monitor->clt_sock);
                memset(args->connect_args.host, 0, CEL_HNLEN);
                memset(args->connect_args.service, 0, CEL_NPLEN);
                _tcscpy(args->connect_args.host, _T("192.168.0.197"));
                _tcscpy(args->connect_args.service, _T("80"));
                args->connect_args.buffer = NULL;
                args->connect_args.buffer_size = 0;
                if (cel_socket_async_connect_host(&(args->connect_args)) == 0)
                {
                    //_tprintf(_T("Post ok %p\r\n"), args);
                    return;
                }
            }
            cel_socket_destroy(&monitor->clt_sock);
        }
        monitor->state = MONITOR_DISCONNECTED;
        _putts(cel_geterrstr(cel_geterrno()));
        break;
    case MONITOR_CONNECTING:
        monitor->state = MONITOR_CONNECTED;
        if (args->connect_args.ol.result != 0
            || args->connect_args.socket->fd  == INVALID_SOCKET
#ifdef _CEL_WIN
            || cel_socket_update_connectcontext(args->connect_args.socket) != 0
#endif
            )
        {
            _tprintf(_T("Connect %s failed.(%s)\r\n"), 
                cel_sockaddr_ntop(&(args->connect_args.remote_addr)),
                cel_geterrstr(args->connect_args.ol.error));
            cel_socket_destroy(&monitor->clt_sock);
            monitor->state = MONITOR_DISCONNECTED;
            return;
        }
        _tprintf(_T("Connect %s ok.\r\n"), 
            cel_sockaddr_ntop(&(args->connect_args.remote_addr)));
        memcpy(&(monitor->addr), &(args->connect_args.remote_addr), sizeof(CelSockAddr));
        //cel_socket_destroy(&monitor->clt_sock);
        //monitor->state = MONITOR_OK;
    case MONITOR_CONNECTED:
        monitor->state = MONITOR_SENDING;
        monitor->offset = 0;
        monitor->size = snprintf(monitor->buf, 1024, 
            "GET / HTTP/1.1\r\n"
            "Connection: Keep-Alive\r\n"
            "Accept: */*\r\n\r\n");
        args->send_args.async_callback = (void (*) (void *))monitor_check;
        args->send_args.socket = &(monitor->clt_sock);
        monitor->async_buf.buf = monitor->buf + monitor->offset;
        monitor->async_buf.len = (unsigned long)(monitor->size - monitor->offset);
        args->send_args.buffers = &(monitor->async_buf);
        args->send_args.buffer_count = 1;
        if (cel_socket_async_send(&(args->send_args)) == -1)
        {
            _tprintf(_T("monitor %s post send failed(%s).\r\n"), 
                cel_sockaddr_ntop(&(monitor->addr)), cel_geterrstr(cel_geterrno()));
            cel_socket_destroy(&monitor->clt_sock);
            monitor->state = MONITOR_DISCONNECTED;
            return;
        }
        break;
    case MONITOR_SENDING:
        if (args->send_args.ol.result <= 0)
        {
            _tprintf(_T("monitor %s do send failed(%s).\r\n"), 
                cel_sockaddr_ntop(&(monitor->addr)), 
                cel_geterrstr(args->send_args.ol.error));
            cel_socket_destroy(&monitor->clt_sock);
            monitor->state = MONITOR_DISCONNECTED;
            return;
        }
        monitor->offset += args->send_args.ol.result;
        _tprintf(_T("%p %s send %d, offset %d ,size %d\r\n"), 
            monitor, cel_sockaddr_ntop(&(monitor->addr)),
            (int)args->send_args.ol.result, (int)monitor->offset, (int)monitor->size);
        if (monitor->offset < monitor->size)
        {
            args->send_args.async_callback = (void (*) (void *))monitor_check;
            args->send_args.socket = &(monitor->clt_sock);
            monitor->async_buf.buf = monitor->buf + monitor->offset;
            monitor->async_buf.len = (unsigned long)(monitor->size - monitor->offset);
            args->send_args.buffers = &(monitor->async_buf);
            args->send_args.buffer_count = 1;
            if (cel_socket_async_send(&(args->send_args)) == -1)
            {
                _tprintf(_T("monitor %s post send failed(%s).\r\n"), 
                    cel_sockaddr_ntop(&(monitor->addr)), cel_geterrstr(cel_geterrno()));
                cel_socket_destroy(&monitor->clt_sock);
                monitor->state = MONITOR_DISCONNECTED;
                return;
            }
        }
        else
        {
            monitor->state = MONITOR_RECVING;
            monitor->offset = monitor->size = 0;
            args->recv_args.async_callback = (void (*) (void *))monitor_check;
            args->recv_args.socket = &(monitor->clt_sock);
            monitor->async_buf.buf = monitor->buf;
            monitor->async_buf.len = 1024;
            args->recv_args.buffers = &(monitor->async_buf);
            args->recv_args.buffer_count = 1;
            if (cel_socket_async_recv(&(args->recv_args)) == -1)
            {
                _tprintf(_T("monitor %s post receive failed(%s).\r\n"), 
                    cel_sockaddr_ntop(&(monitor->addr)), cel_geterrstr(cel_geterrno()));
                cel_socket_destroy(&monitor->clt_sock);
                monitor->state = MONITOR_DISCONNECTED;
                return;
            }
        }
        break;
    case MONITOR_RECVING:
        if (args->send_args.ol.result <= 0)
        {
            _tprintf(_T("monitor %s do receive failed(%s).\r\n"), 
                cel_sockaddr_ntop(&(monitor->addr)), 
                cel_geterrstr(args->send_args.ol.error));
            cel_socket_destroy(&monitor->clt_sock);
            monitor->state = MONITOR_DISCONNECTED;
            return;
        }
        monitor->size += args->send_args.ol.result;
        monitor->buf[monitor->size] = '\0';
        monitor->state = MONITOR_OK;
        //puts(monitor->buf);
        /*_tprintf(_T("ok size = %d %p %s %d state %d\r\n"), 
            args->send_args.ol.result,
            monitor, cel_sockaddr_ntop(&(monitor->addr)), 
            monitor->clt_sock.fd, monitor->state);*/
        break;
    case MONITOR_OK:
        break;
    }
}

void monitor_check_state(Monitor *monitor)
{
    BOOL active = monitor->active;

    if (monitor->state == MONITOR_OK)
    {
        monitor->down_cnt = 0;
        active = TRUE;
    }
    else
    {
        if (++monitor->down_cnt >= 2)
        {
            active = FALSE;
            cel_socket_shutdown(&(monitor->clt_sock), SHUT_RDWR);
        }
    }
    if (active != monitor->active)
    {
        monitor->active = active;
        _tprintf(_T("Monitor %p state %s\r\n"), 
            monitor,  monitor->active ? _T("up"): _T("down")); 
    }
    _tprintf(_T("%p %s %d check state %d\r\n"), 
        monitor, cel_sockaddr_ntop(&(monitor->addr)), monitor->clt_sock.fd, monitor->state);
    if (monitor->state == MONITOR_DISCONNECTED)
    {
        monitor_check(monitor);
    }
    else if (monitor->state == MONITOR_OK)
    {
        monitor->state = MONITOR_CONNECTED;
        monitor_check(monitor);
    }
}

static int monitor_working(void *data)
{
    cel_eventloop_run(&evt_loop);
    cel_thread_exit(0);
    return 0;
}

int monitor_test(int argc, TCHAR *argv[])
{
    int i;
    void *ret_val;
    CelThread tds[CEL_THDNUM];
    Monitor monitors[MONITOR_MAX];

    cel_multithread_support();
#ifdef _CEL_WIN
    cel_wsastartup();
#endif
    if (cel_eventloop_init(&evt_loop, 4, 1024) != 0)
        return -1;
    for (i = 0; i < 9; i++)
    {
        if (cel_thread_create(&tds[i], NULL, &monitor_working, NULL) != 0)
        {
            return -1;
        }
    }
    for (i = 0; i < MONITOR_MAX; i++)
    {
        memset(&monitors[i], 0, sizeof(Monitor));
        monitor_check(&monitors[i]);
        monitors[i].timer_id = cel_eventloop_schedule_timer(&evt_loop, 3000, 1, 
            (CelTimerCallbackFunc)monitor_check_state, &monitors[i]);
    }

    cel_eventloop_run(&evt_loop);

    for (i = 0; i < 9; i++)
    {
        //_tprintf(_T("i %d \r\n"), i);
        cel_thread_join(tds[i], &ret_val);
    }
    cel_eventloop_destroy(&evt_loop);
#ifdef _CEL_WIN
    cel_wsacleanup();
#endif

    return 0;
}
