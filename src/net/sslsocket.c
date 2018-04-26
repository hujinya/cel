#include "cel/net/sslsocket.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

/* Debug defines */
#define Debug(args)   cel_log_debug args
#define Warning(args) CEL_SETERRSTR(args)/* cel_log_warning args */
#define Err(args)   /*CEL_SETERRSTR(args)*/ cel_log_err args

#define CEL_BIO_MEM_SIZE  8192

static int cel_sslsocket_post_send(CelSslSocket *ssl_sock,
                                   CelSocketAsyncArgs *socket_args, 
                                   void (*async_callback)(void *));
static int cel_sslsocket_post_recv(CelSslSocket *ssl_sock, 
                                   CelSocketAsyncArgs *socket_args, 
                                   void (*async_callback)(void *));
static void cel_sslsocket_do_handshake(CelSslAsyncHandshakeArgs *args);
static void cel_sslsocket_do_send(CelSslAsyncSendArgs *args);
static void cel_sslsocket_do_recv(CelSslAsyncRecvArgs *args);

int cel_bio_mem_grow(CelBio *bio)
{
    BUF_MEM *bm = (BUF_MEM *)bio->ptr;
    size_t length = bm->length, size;

    if ((size = cel_capacity_get_min(bm->max + 1)) < CEL_BIO_MEM_SIZE)
        size = CEL_BIO_MEM_SIZE;   
    if (BUF_MEM_grow_clean(bm, size) == (int)size)
    {
        bm->length = length;
        /*_tprintf(_T("grow %d len %d max %d\r\n"), 
            (int)size, (int)bm->length, (int)bm->max);*/
        return 0;
    }
    return -1;
}

char *cel_bio_mem_get_read_pointer(CelBio *bio)
{
    return ((BUF_MEM *)bio->ptr)->data;
}

char *cel_bio_mem_get_write_pointer(CelBio *bio)
{
    BUF_MEM *bm = (BUF_MEM *)bio->ptr;
    return &(bm->data[bm->length]);
}

int cel_bio_mem_read_seek(CelBio *bio, int seek)
{
    int ret= -1;
    BUF_MEM *bm;

    bm = (BUF_MEM *)bio->ptr;
    BIO_clear_retry_flags(bio);
    ret = (seek >= 0 && (size_t)seek > bm->length) ? (int)bm->length : seek;
    if (ret > 0)
    {
        bm->length -= ret;
        memmove(&(bm->data[0]), &(bm->data[ret]), bm->length);
    }
    else if (bm->length == 0)
    {
        ret = bio->num;
        if (ret != 0)
            BIO_set_retry_read(bio);
    }
    if (ret > 0)
        bio->num_read += (unsigned long)ret;
    return (ret);
}

int cel_bio_mem_write_seek(CelBio *bio, int seek)
{
    int ret= -1;
    BUF_MEM *bm;

    bm = (BUF_MEM *)bio->ptr;
    if (seek < 0 || bm->length + seek > bm->max)
    {
        Err((_T("cel_bio_mem_write_seek %d error."), seek));
    }
    else
    {
        BIO_clear_retry_flags(bio);
        bm->length += seek;
        ret = seek;
        if (ret > 0) 
            bio->num_write += (unsigned long)ret;
    }

    return(ret);
}

size_t cel_bio_mem_get_remaining(CelBio *bio)
{
    BUF_MEM *bm = (BUF_MEM *)bio->ptr;
    return bm->max - bm->length;
}

size_t cel_bio_mem_get_length(CelBio *bio)
{
    return ((BUF_MEM *)bio->ptr)->length;
}

void _cel_sslsocket_destroy_derefed(CelSslSocket *ssl_sock)
{
    //puts("_cel_sslsocket_destroy_derefed");
    if (ssl_sock->ssl != NULL)
        cel_ssl_free(ssl_sock->ssl);
    cel_socket_destroy(&(ssl_sock->sock));
}

void _cel_sslsocket_free_derefed(CelSslSocket *ssl_sock)
{
    _cel_sslsocket_destroy_derefed(ssl_sock);
    cel_free(ssl_sock);
}

int cel_sslsocket_init(CelSslSocket *ssl_sock, CelSocket *sock, CelSslContext *ssl_ctx)
{
    if (&(ssl_sock->sock) != sock)
        memcpy(&(ssl_sock->sock), sock, sizeof(CelSocket));
    if ((ssl_sock->ssl = cel_ssl_new(ssl_ctx)) != NULL)
    {
        SSL_set_mode(ssl_sock->ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);
        SSL_set_mode(ssl_sock->ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
        //SSL_set_mode(ssl_sock->ssl, SSL_MODE_RELEASE_BUFFERS);
        //SSL_CTRL_SET_READ_AHEAD
        if ((ssl_sock->r_bio = BIO_new(BIO_s_mem())) != NULL
            && (ssl_sock->w_bio = BIO_new(BIO_s_mem())) != NULL)
        {
            ssl_sock->use_ssl = TRUE;
            cel_bio_mem_grow(ssl_sock->r_bio);
            SSL_set_bio(ssl_sock->ssl, ssl_sock->r_bio, ssl_sock->w_bio);
            cel_refcounted_init(&(ssl_sock->ref_counted), 
                (CelFreeFunc)_cel_sslsocket_destroy_derefed);
            return 0;
        }
        cel_ssl_free(ssl_sock->ssl);
    }
    Err((_T("Ssl socket init failed(%s)."), cel_ssl_get_errstr(cel_ssl_get_errno())));

    return -1;
}

void cel_sslsocket_destroy(CelSslSocket *ssl_sock)
{
    //puts("cel_sslsocket_destroy");
    cel_refcounted_destroy(&(ssl_sock->ref_counted), ssl_sock);
}

CelSslSocket *cel_sslsocket_new(CelSocket *sock, CelSslContext *ssl_ctx)
{
    CelSslSocket *ssl_sock;

    if ((ssl_sock = (CelSslSocket *)cel_malloc(sizeof(CelSslSocket))) != NULL)
    {   
        if (cel_sslsocket_init(ssl_sock, sock, ssl_ctx) == 0)
        {
            cel_refcounted_init(&(ssl_sock->ref_counted),
                (CelFreeFunc)_cel_sslsocket_free_derefed);
            return ssl_sock;
        }
        cel_free(ssl_sock);
    }
    return ssl_sock;
}

void cel_sslsocket_free(CelSslSocket *ssl_sock)
{
    cel_refcounted_destroy(&(ssl_sock->ref_counted), ssl_sock);
}

int cel_sslsocket_send_bio_mem(CelSslSocket *ssl_sock)
{
    int _size, size = 0;

    while (cel_bio_mem_get_length(ssl_sock->w_bio) > 0)
    {
        if ((_size = cel_socket_send(&(ssl_sock->sock), 
            cel_bio_mem_get_read_pointer(ssl_sock->w_bio), 
            cel_bio_mem_get_length(ssl_sock->w_bio))) <= 0)
            return -1;
        cel_bio_mem_read_seek(ssl_sock->w_bio, _size);
        size += _size;
    }
    return size;
}

int cel_sslsocket_recv_bio_mem(CelSslSocket *ssl_sock)
{
    int _size, size = 0;

    if (cel_bio_mem_get_remaining(ssl_sock->r_bio) > 0)
    {
        if ((_size = cel_socket_send(&(ssl_sock->sock),
            cel_bio_mem_get_write_pointer(ssl_sock->r_bio),
            cel_bio_mem_get_remaining(ssl_sock->r_bio))) <= 0)
            return -1;
        cel_bio_mem_write_seek(ssl_sock->r_bio, _size);
        if (cel_bio_mem_get_remaining(ssl_sock->r_bio) == 0)
            cel_bio_mem_grow(ssl_sock->r_bio);
        size += _size;
    }
    return size;
}

int cel_sslsocket_handshake(CelSslSocket *ssl_sock)
{
    int result, error;

    while ((result = cel_ssl_handshake(ssl_sock->ssl)) != 1)
    {
        if ((error = SSL_get_error(ssl_sock->ssl, result)) == SSL_ERROR_WANT_READ
            || error == SSL_ERROR_WANT_WRITE)
        {
            if (cel_sslsocket_send_bio_mem(ssl_sock) > 0
                && cel_sslsocket_recv_bio_mem(ssl_sock) > 0)
                continue;
        }
        break;
    }
    return result;
}

int cel_sslsocket_send(CelSslSocket *ssl_sock, void *buf, size_t size)
{
    int result;

    if ((result = cel_ssl_write(ssl_sock->ssl, buf, size)) > 0)
    {
        if (cel_sslsocket_send_bio_mem(ssl_sock) < 0)
            return -1;
        return result;
    }
    return -1;
}

int cel_sslsocket_recv(CelSslSocket *ssl_sock, void *buf, size_t size)
{
    int result, error;

    while ((result = cel_ssl_read(ssl_sock->ssl, buf, size)) <= 0)
    {
        if ((error = SSL_get_error(ssl_sock->ssl, result)) == SSL_ERROR_WANT_READ
            || (ERR_get_error() == 0 && cel_sys_geterrno() == 0))
        {
            if (cel_sslsocket_recv_bio_mem(ssl_sock) > 0)
                continue;
        }
        return -1;
    }
    return result;
}

int cel_sslsocket_post_send(CelSslSocket *ssl_sock, 
                            CelSocketAsyncArgs *socket_args, 
                            void (*async_callback)(void *))
{
    memset(&(socket_args->ol), 0, sizeof(socket_args->ol));
    socket_args->send_args.socket = &(ssl_sock->sock);
    socket_args->send_args.async_callback = async_callback;
    ssl_sock->async_wbuf.buf = cel_bio_mem_get_read_pointer(ssl_sock->w_bio);
    ssl_sock->async_wbuf.len = cel_bio_mem_get_length(ssl_sock->w_bio);
    socket_args->send_args.buffer_count = 1;
    socket_args->send_args.buffers = &(ssl_sock->async_wbuf);
    //_tprintf(_T("post send %d\r\n"), (int)ssl_sock->async_wbuf.len);
    return cel_socket_async_send(&(socket_args->send_args));
}

int cel_sslsocket_post_recv(CelSslSocket *ssl_sock,
                            CelSocketAsyncArgs *socket_args, 
                            void (*async_callback)(void *))
{
    memset(&(socket_args->ol), 0, sizeof(socket_args->ol));
    socket_args->recv_args.socket = &(ssl_sock->sock);
    socket_args->recv_args.async_callback = async_callback;
    ssl_sock->async_rbuf.buf = cel_bio_mem_get_write_pointer(ssl_sock->r_bio);
    ssl_sock->async_rbuf.len = cel_bio_mem_get_remaining(ssl_sock->r_bio);
    socket_args->recv_args.buffer_count = 1;
    socket_args->recv_args.buffers = &(ssl_sock->async_rbuf);
    /*_tprintf(_T("post recv %p %d\r\n"), 
        ssl_sock->async_rbuf.buf, (int)ssl_sock->async_rbuf.len);*/
    return cel_socket_async_recv(&(socket_args->recv_args));
}

void cel_sslsocket_do_handshake(CelSslAsyncHandshakeArgs *args)
{
    CelSocketAsyncArgs *socket_args = &(args->socket_args);
    CelSslSocket *ssl_sock = args->ssl_sock;

    /*_tprintf(_T("fd %d ol type %d, result = %ld error = %d\r\n"), 
        ssl_sock->sock.fd,
        args->socket_args.ol.evt_type, 
        args->socket_args.ol.result, args->socket_args.ol.error);*/
    if (args->socket_args.ol.result <= 0)
    {
        /*_tprintf(_T("result %d, error %d,fd %d ol type %d, result = %ld error = %d\r\n"), 
            args->result, args->error, ssl_sock->sock.fd,
            args->socket_args.ol.evt_type, 
            args->socket_args.ol.result, args->socket_args.ol.error);*/
        args->result = args->socket_args.ol.result;
        args->error = args->socket_args.ol.error;
        args->async_callback(args);
        return ;
    }
    if (socket_args->ol.evt_type == CEL_EVENT_CHANNELIN)
    {
        cel_bio_mem_write_seek(ssl_sock->r_bio, args->socket_args.ol.result);
        if (cel_bio_mem_get_remaining(ssl_sock->r_bio) == 0)
            cel_bio_mem_grow(ssl_sock->r_bio);
    }
    else if (socket_args->ol.evt_type == CEL_EVENT_CHANNELOUT)
    {
        cel_bio_mem_read_seek(ssl_sock->w_bio, args->socket_args.ol.result);
    }
    if (args->result != 1)
    {
        args->result = cel_ssl_handshake(ssl_sock->ssl);
        if ((args->error = 
            SSL_get_error(ssl_sock->ssl, args->result)) == SSL_ERROR_NONE
            || args->error == SSL_ERROR_WANT_READ
            || args->error == SSL_ERROR_WANT_WRITE)
        {
            if (cel_bio_mem_get_length(ssl_sock->w_bio) > 0)
            {
                cel_sslsocket_post_send(ssl_sock, socket_args,
                    (void (*) (void *))cel_sslsocket_do_handshake);
                return ;
            }
            else if ((args->error == SSL_ERROR_WANT_READ))
            {
                cel_sslsocket_post_recv(ssl_sock, socket_args, 
                    (void (*) (void *))cel_sslsocket_do_handshake);
                return ;
            }
        }
        else
        {
            _tprintf(_T("ssl do handshake fd %d, result %ld error %d %s\r\n"), 
                ssl_sock->sock.fd, args->result, args->error, 
                cel_ssl_get_errstr(cel_ssl_get_errno()));
        }
    }
    else
    {
        if (cel_bio_mem_get_length(ssl_sock->w_bio) > 0)
        {
            cel_sslsocket_post_send(ssl_sock, socket_args,
                (void (*) (void *))cel_sslsocket_do_handshake);
            return ;
        }
    }
    args->async_callback(args);
}

void cel_sslsocket_do_send(CelSslAsyncSendArgs *args)
{
    CelSslSocket *ssl_sock = args->ssl_sock;
    CelSocketAsyncArgs *socket_args = &(args->socket_args);

    /*_tprintf(_T("ol type %d, result = %ld error = %d\r\n"), 
        args->socket_args.ol.evt_type, args->socket_args.ol.result, 
        args->socket_args.ol.error);*/
    if (args->socket_args.ol.result > 0)
    {
        cel_bio_mem_read_seek(ssl_sock->w_bio, args->socket_args.ol.result);
        if (cel_bio_mem_get_length(ssl_sock->w_bio) > 0)
        {
            cel_sslsocket_post_send(ssl_sock, socket_args,
                (void (*) (void *))cel_sslsocket_do_send);
            return ;
        }
    }
    else
    {
        args->result = args->socket_args.ol.result;
        args->error = args->socket_args.ol.error;
    }
    args->async_callback(args);
}

void cel_sslsocket_do_recv(CelSslAsyncRecvArgs *args)
{
    CelSocketAsyncArgs *socket_args = &(args->socket_args);
    CelSslSocket *ssl_sock = args->ssl_sock;

    /*_tprintf(_T("ol type %d, result = %ld error = %d\r\n"), 
        args->socket_args.ol.evt_type, args->socket_args.ol.result, 
        args->socket_args.ol.error);*/
    if (args->socket_args.ol.result > 0)
    {
        cel_bio_mem_write_seek(ssl_sock->r_bio, args->socket_args.ol.result);
        if (cel_bio_mem_get_remaining(ssl_sock->r_bio) == 0)
            cel_bio_mem_grow(ssl_sock->r_bio);
        if ((args->result = cel_ssl_read(
            ssl_sock->ssl, args->buffers->buf, args->buffers->len)) <= 0
            && (args->error = SSL_get_error(ssl_sock->ssl, args->result)) 
            != SSL_ERROR_ZERO_RETURN)
        {
            if (args->error == SSL_ERROR_WANT_READ)
            {
                cel_sslsocket_post_recv(ssl_sock, 
                    socket_args, (void (*) (void *))cel_sslsocket_do_recv);
                return ;
            }
            Err((_T("ssl do read (%p %d)%d error %d %s"), 
                args->buffers->buf, args->buffers->len, (int)args->result, 
                args->error, (args->error != SSL_ERROR_SYSCALL 
                ? cel_ssl_get_errstr(cel_ssl_get_errno())
                : cel_geterrstr(cel_sys_geterrno()))));
        }
    }
    else
    {
        args->result = args->socket_args.ol.result;
        args->error = args->socket_args.ol.error;
    }
    args->async_callback(args);
}

int cel_sslsocket_async_handshake(CelSslAsyncHandshakeArgs *args)
{
    CelSocketAsyncArgs *socket_args = &(args->socket_args);
    CelSslSocket *ssl_sock = args->ssl_sock;

    if ((args->result = cel_ssl_handshake(ssl_sock->ssl)) != 1)
    {
        if ((args->error = SSL_get_error(ssl_sock->ssl, args->result)) == SSL_ERROR_NONE
            || args->error == SSL_ERROR_WANT_READ
            || args->error == SSL_ERROR_WANT_WRITE)
        {
            if (cel_bio_mem_get_length(ssl_sock->w_bio) > 0)
            {
                return cel_sslsocket_post_send(ssl_sock, socket_args,
                    (void (*) (void *))cel_sslsocket_do_handshake);
            }
            else if ((args->error == SSL_ERROR_WANT_READ))
            {
                return cel_sslsocket_post_recv(ssl_sock, socket_args, 
                    (void (*) (void *))cel_sslsocket_do_handshake);
            }
        }
        Err((_T("ssl handshake %d error %d %s"), 
            args->result, args->error, cel_ssl_get_errstr(cel_ssl_get_errno())));
    }
    else
    {
        if (cel_bio_mem_get_length(ssl_sock->w_bio) > 0)
        {
            return cel_sslsocket_post_send(ssl_sock, 
                socket_args, (void (*) (void *))cel_sslsocket_do_handshake);
        }
    }
    args->async_callback(args);
    return 0;
}

int cel_sslsocket_async_send(CelSslAsyncSendArgs *args)
{
    CelSocketAsyncArgs *socket_args = &(args->socket_args);
    CelSslSocket *ssl_sock = args->ssl_sock;

    if ((args->result = cel_ssl_write(
        ssl_sock->ssl, args->buffers->buf, args->buffers->len)) > 0)
    {
        if (cel_bio_mem_get_length(ssl_sock->w_bio) > 0)
        {
            return cel_sslsocket_post_send(ssl_sock, 
            socket_args, (void (*) (void *))cel_sslsocket_do_send);
        }
    }
    args->result = -1;
    args->error = SSL_get_error(ssl_sock->ssl, args->result);
    Err((_T("ssl write error %d %s"), 
        args->error, cel_ssl_get_errstr(cel_ssl_get_errno())));
    args->async_callback(args);

    return 0;
}

int cel_sslsocket_async_recv(CelSslAsyncRecvArgs *args)
{
    CelSocketAsyncArgs *socket_args = &(args->socket_args);
    CelSslSocket *ssl_sock = args->ssl_sock;

    //ERR_clear_error();
    if ((args->result = cel_ssl_read(
        ssl_sock->ssl, args->buffers->buf, args->buffers->len)) <= 0)
    {
        if ((args->error = SSL_get_error(
            ssl_sock->ssl, args->result)) == SSL_ERROR_WANT_READ
            || (ERR_get_error() == 0 && cel_sys_geterrno() == 0))
        {
            return cel_sslsocket_post_recv(ssl_sock, 
                socket_args, (void (*) (void *))cel_sslsocket_do_recv);
        }
        Err((_T("ssl read (%p %d)%d error %d %s"), 
            args->buffers->buf, args->buffers->len, (int)args->result, 
            args->error, (args->error != SSL_ERROR_SYSCALL 
            ? cel_ssl_get_errstr(cel_ssl_get_errno())
            : cel_geterrstr(cel_sys_geterrno()))));
        args->result = -1;
    }
    args->async_callback(args);

    return 0;
}
