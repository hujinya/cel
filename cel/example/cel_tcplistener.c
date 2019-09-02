#include "cel/net/tcplistener.h"
#include "cel/file.h"
#include "cel/eventloop.h"
#include "cel/sys/envinfo.h"
#include "cel/multithread.h"
#include "cel/crypto/crypto.h"

#define MAX_FD 10240

static CelEventLoop evt_loop;
static CelSslContext *sslctx;
static CelTcpListener s_listener;
static CelTcpClient s_client[MAX_FD];
static CelStream s_stream[MAX_FD];
static char *rsp = "HTTP/1.1 200 OK\r\nContent-Length: 4096\r\n\r\n";
#ifdef _CEL_UNIX
static CelSignals s_signals[] = 
{
    { SIGPIPE, "sigpipe", SIG_IGN }, 
    { 0, NULL, NULL }
};
static CelResourceLimits s_rlimits = { 0, MAX_FD, MAX_FD };
#endif

static void tcpclient_accept_completion(CelTcpListener *listener,
                                        CelTcpClient *client, CelAsyncResult *async_result);

void tcpclient_send_completion(CelTcpClient *client, 
                               CelStream *s, CelAsyncResult *async_result)
{
    _tprintf("Client %s send %ld, error %d, remaining %d.\r\n",
        cel_sockaddr_ntop(&(client->remote_addr)),
        async_result->ret, async_result->error, 
        (int)cel_stream_get_remaining_length(s));
    if (async_result->ret > 0)
    {
        if (cel_stream_get_remaining_length(s) > 0)
        {
            cel_tcpclient_async_send(client, s, &tcpclient_send_completion, NULL);
        }
        /*_tprintf("Client %s %d closed.(%s)\r\n", 
            cel_sockaddr_ntop(&(client->remote_addr)), client->sock.fd, 
            cel_geterrstr());*/
        cel_stream_destroy(s);
        cel_tcpclient_destroy(client);
        return ;
    }
    _tprintf("Client %s %d send failed.(%s)\r\n", 
        cel_sockaddr_ntop(&(client->remote_addr)), client->sock.fd, 
        cel_geterrstr());
    cel_stream_destroy(s);
    cel_tcpclient_destroy(client);
}

void tcpclient_recv_completion(CelTcpClient *client,
                               CelStream *s, CelAsyncResult *async_result)
{
    if (async_result->ret > 0)
    {
        _tprintf("Client %s recv %ld, %s.\r\n",
            cel_sockaddr_ntop(&(client->remote_addr)), async_result->ret,
        cel_stream_get_buffer(s));
        cel_stream_set_position(s, 0);
        cel_stream_write(s, rsp, strlen(rsp));
        cel_stream_zero(s, 4096);
        cel_stream_seal_length(s);
        cel_stream_set_position(s, 0);
        cel_tcpclient_async_send(client, s, &tcpclient_send_completion, NULL);
        return;
    }
    _tprintf("Client %s %d recv failed.(%s)\r\n", 
        cel_sockaddr_ntop(&(client->remote_addr)), client->sock.fd, 
        cel_geterrstr());
    cel_stream_destroy(s);
    cel_tcpclient_destroy(client);
}

void tcpclient_handshake_completion(CelTcpClient *client,
                                    CelAsyncResult *async_result)
{
    CelStream *s = &(s_stream[client->sock.fd]);

    if (async_result->ret == 1)
    {
        _tprintf(_T("Client %s %d handshake successed, protocol \"%s, %s\".\r\n"), 
            cel_sockaddr_ntop(&(client->remote_addr)), client->sock.fd,
            SSL_get_cipher_version(client->ssl_sock.ssl), 
            SSL_get_cipher(client->ssl_sock.ssl));
        cel_stream_init(s);
        cel_stream_resize(s, 81920); 
        cel_tcpclient_async_recv(client, s, &tcpclient_recv_completion, NULL);
    }
    else
    {
        _tprintf("Client %s %d handshake failed.(%s)\r\n", 
            cel_sockaddr_ntop(&(client->remote_addr)), client->sock.fd, 
            cel_geterrstr());
        cel_stream_destroy(s);
        cel_tcpclient_destroy(client);
    }
}

void tcpclient_accept_completion(CelTcpListener *listener, 
                                 CelTcpClient *client,
                                 CelAsyncResult *async_result)
{
    CelTcpClient *new_client;
    //CelStream *s = &(s_stream[client->sock.fd]);

    new_client = &s_client[client->sock.fd];
    memcpy(new_client, client, sizeof(CelTcpClient));
    _tprintf(_T("Accept %s fd %d\r\n"), 
        cel_tcpclient_get_remoteaddr_str(new_client), new_client->sock.fd);
    cel_tcplistener_async_accept(&s_listener, client, &tcpclient_accept_completion, NULL);
    cel_tcpclient_set_nonblock(&new_client, 1);
    cel_eventloop_add_channel(&evt_loop, &(new_client->sock.channel), NULL);
    if (cel_tcpclient_async_handshake(
        new_client, &tcpclient_handshake_completion, NULL) == -1)
    {
        _tprintf("Client %s %d post handshake failed\r\n", 
            cel_sockaddr_ntop(&(new_client->remote_addr)), new_client->sock.fd);
        cel_tcpclient_destroy(new_client);
    }
}

void tcpclient_co_func(CelTcpClient *client)
{
    CelCoroutine co = cel_coroutine_self();
    CelStream *s = &(s_stream[client->sock.fd]);

    cel_stream_init(s);
    cel_stream_resize(s, 81920); 
    cel_tcpclient_async_handshake(client, NULL, &co);
    printf("cel_tcpclient_co_handshake %d %d\r\n", 
        (int)cel_coroutine_getid(), (int)cel_thread_getid());
    cel_tcpclient_async_recv(client, s, NULL, &co);
    printf("cel_tcpclient_co_recv %d %d\r\n",
        (int)cel_coroutine_getid(), (int)cel_thread_getid());
    cel_stream_set_position(s, 0);
    cel_stream_write(s, rsp, strlen(rsp));
    cel_stream_zero(s, 4096);
    cel_stream_seal_length(s);
    cel_stream_set_position(s, 0);
    cel_tcpclient_async_send(client, s, NULL, &co);
    printf("cel_tcpclient_co_send %d %d\r\n",
        (int)cel_coroutine_getid(), (int)cel_thread_getid());
    cel_tcpclient_destroy(client);
}

void tcpclient_accept_co(CelTcpListener *listener, 
                         CelTcpClient *client, CelAsyncResult *async_result)
{
    CelCoroutine co;
    CelTcpClient *new_client;

    new_client = &s_client[client->sock.fd];
    memcpy(new_client, client, sizeof(CelTcpClient));
    _tprintf(_T("Accept %s fd %d\r\n"), 
        cel_tcpclient_get_remoteaddr_str(new_client), new_client->sock.fd);
    cel_tcplistener_async_accept(&s_listener, client, &tcpclient_accept_co, NULL);
    cel_tcpclient_set_nonblock(&new_client, 1);
    cel_eventloop_add_channel(&evt_loop, &(new_client->sock.channel), NULL);

    cel_coroutine_create(&co, NULL, (CelCoroutineFunc)tcpclient_co_func, new_client);
    cel_coroutine_resume(&co);
}

static int tcplistener_working(void *data)
{
    puts("tcplistener_working");
    cel_eventloop_run(&evt_loop);
    /*_tprintf(_T("Event loop thread %d exit.(%s)"), 
       cel_thread_getid(), cel_geterrstr());*/
    cel_thread_exit(0);

    return 0;
}

int tcplistener_test(int argc, TCHAR *argv[])
{
    int i, td_num, work_num;
    void *ret_val;
    CelCpuMask mask;
    CelThread tds[CEL_THDNUM];

    cel_multithread_support();
    cel_cryptomutex_register(NULL, NULL);
    cel_ssllibrary_init();
#ifdef _CEL_UNIX
    cel_signals_init(s_signals);
    cel_resourcelimits_set(&s_rlimits);
#endif
    cel_wsastartup();
    work_num = (td_num = (int)cel_getnprocs());
    if (work_num > CEL_THDNUM)
        work_num = work_num / 2;
    else if (work_num < 2)
        work_num = 2;
    work_num = 1;
    if (cel_eventloop_init(&evt_loop, td_num, 10240) == -1)
        return 1;
    for (i = 0; i < work_num; i++)
    {
        cel_setcpumask(&mask, i%td_num);
        if (cel_thread_create(&tds[i], NULL, &tcplistener_working, NULL) != 0
           || cel_thread_setaffinity(&tds[i], &mask) != 0)
        {
            _tprintf(_T("Work thread create failed.(%s)\r\n"), cel_geterrstr());
            return 1;
        }
    }
    if (((sslctx = cel_sslcontext_new(
        cel_sslcontext_method(_T("SSLv23")))) == NULL
        || cel_sslcontext_set_own_cert(sslctx, 
        cel_fullpath_a("../data/etc/vas-server.crt"), 
        cel_fullpath_a("../data/etc/vas-server.key"), _T("38288446")) == -1
        || cel_sslcontext_set_ciphersuites(
        sslctx, _T("AES:ALL:!aNULL:!eNULL:+RC4:@STRENGTH")) == -1))
    {
        printf("Ssl context init failed.(%s)\r\n", cel_sys_strerror(cel_sys_geterrno()));
        return -1;
    }

    if (cel_tcplistener_init_str(&s_listener, _T("0.0.0.0:9443"), sslctx) != 0)
    {
        printf("cel_tcplistener_init_str failed.(%s)\r\n", cel_sys_strerror(cel_sys_geterrno()));
        return -1;
    }
    cel_socket_set_nonblock(&(s_listener.sock), 1);
    cel_eventloop_add_channel(&evt_loop, &(s_listener.sock.channel), NULL);
    puts("listen...");
    cel_tcplistener_async_accept(&s_listener, &s_client[0], &tcpclient_accept_co, NULL);
    for (i = 0; i < work_num; i++)
    {
        //_tprintf(_T("i %d \r\n"), i);
        cel_thread_join(tds[i], &ret_val);
    }


    return 0;
}
