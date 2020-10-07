#include "cel/error.h"
#include "cel/multithread.h"
#include "cel/crypto/crypto.h"
#include "cel/thread.h"
#include "cel/eventloop.h"
#include "cel/net/sslsocket.h"

static CelEventLoop evt_loop;
static CelSocket ssl_listener;
static CelSslContext *ssl_ctx;
static CelSockAddr addr;
static CelSslSocket ssl_socket;
static CelAsyncBuf async_buf;
static char buf[1024 * 60];

static void send_completion(CelSslSocket *ssl_sock, CelAsyncBuf *buffers, int count,
                            CelAsyncResult *result);
static void recv_completion(CelSslSocket *ssl_sock, CelAsyncBuf *buffers, int count,
                            CelAsyncResult *result);
static void handshake_completion(CelSslSocket *ssl_sock, CelAsyncResult *result);
static void accept_completion(CelSslSocket *ssl_listener,
                              CelSslSocket *ssl_socket, CelAsyncResult *result);

static int sslsocket_working(void *data)
{
    cel_eventloop_run(&evt_loop);
    //_tprintf(_T("Event loop thread %d exit.(%s)"), 
    //   cel_thread_getid(), cel_geterrstr());

    cel_thread_exit(0);

    return 0;
}

void send_completion(CelSslSocket *ssl_sock, CelAsyncBuf *buffers, int count,
                     CelAsyncResult *result)
{
    puts("send ok");
    cel_sslsocket_destroy(ssl_sock);
    cel_socket_async_accept(&ssl_listener, &(ssl_sock->sock), &addr,
        (CelSocketAcceptCallbackFunc)accept_completion);
}

void recv_completion(CelSslSocket *ssl_sock, CelAsyncBuf *buffers, int count,
                     CelAsyncResult *result)
{
    buf[result->ret] = '\0';
    puts(buf);
    async_buf.buf = buf;
    memset(buf, 'h', 1024 * 60);
    snprintf(buf, 1024, "HTTP/1.1 200 OK\r\nContent-Length: 10000\r\n\r\n");
    async_buf.len = 1024 * 60;
    cel_sslsocket_async_send(ssl_sock, &async_buf, 1, 
        (CelSslSocketSendCallbackFunc)send_completion);
}

void handshake_completion(CelSslSocket *ssl_sock, CelAsyncResult *result)
{
    puts("handshake ok");
    async_buf.buf = buf;
    async_buf.len = 1024 * 60;
    cel_sslsocket_async_recv(ssl_sock, &async_buf, 1, 
        (CelSslSocketRecvCallbackFunc)recv_completion);
}

void accept_completion(CelSslSocket *ssl_listener,
                       CelSslSocket *ssl_sock, CelAsyncResult *result)
{
    puts("accept ok");
    cel_sslsocket_init(ssl_sock, &(ssl_sock->sock), ssl_ctx);
    if (cel_socket_set_nonblock(&(ssl_sock->sock), 1) != 0
        || cel_eventloop_add_channel(&evt_loop, &(ssl_socket.sock.channel), NULL) != 0)
    {
        puts("accept_completion error");
        return ;
    }
    cel_sslsocket_async_accept(ssl_sock, 
        (CelSslSocketHandshakeCallbackFunc)handshake_completion);
}

int sslsocket_test(int argc, TCHAR *argv[])
{
    int i;
    void *ret_val;
    CelThread tds[CEL_THDNUM];
    
    CelSockAddr addr;
    

    cel_multithread_support();
#ifdef _CEL_WIN
    cel_wsastartup();
#endif
    cel_cryptomutex_register(NULL, NULL);
    cel_ssllibrary_init();

    if (cel_eventloop_init(&evt_loop, 10, 1024) == -1)
        return 1;

    for (i = 0; i < (4 - 1); i++)
    {
        if (cel_thread_create(&tds[i], NULL, &sslsocket_working, NULL) != 0)
        {
            return 1;
        }
    }
    if ((ssl_ctx = cel_sslcontext_new(CEL_SSL_METHOD_SSLv23)) == NULL
        || cel_sslcontext_set_own_cert(ssl_ctx, "/etc/sunrun/gateway-server.crt", 
        "/etc/sunrun/gateway-server.key", "123456") == -1
        || cel_sslcontext_set_ciphersuites(ssl_ctx, 
        "ALL:!ADH:!EXPORT56:RC4+RSA:+HIGH:+MEDIUM:+LOW:+SSLv2:+EXP") == -1 )
    {
        _tprintf(_T("ssl init failed. %s\r\n"), cel_geterrstr());
        return -1;
    }

    if (cel_sockaddr_init_str(&addr, _T("0.0.0.0:9443")) != 0
        || cel_socket_init(&(ssl_listener), AF_INET, SOCK_STREAM, IPPROTO_TCP) != 0
        || cel_socket_set_reuseaddr(&(ssl_listener), 1) != 0
        || cel_socket_listen(&(ssl_listener), &addr, 1024) != 0
        || cel_socket_set_nonblock(&(ssl_listener), 1) != 0
        || cel_eventloop_add_channel(&evt_loop, &(ssl_listener.channel), NULL) != 0)
    {
        _tprintf(_T("listen failed. %s\r\n"), cel_geterrstr());
        return -1;
    }
    _tprintf(_T("%s listen.\r\n"), cel_sockaddr_ntop(&addr));
    
    cel_socket_async_accept(&(ssl_listener), &(ssl_socket.sock), &addr,
        (CelSocketAcceptCallbackFunc)accept_completion);

    for (i = 0; i < (4 - 1); i++)
    {
        //_tprintf(_T("i %d \r\n"), i);
        cel_thread_join(tds[i], &ret_val);
    }

    return 0;
}
