/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com) 
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either version 2 
 * of the License, or (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 */
#include "cel/net/sslsocket.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/log.h"

/* 
crypto/bio/bss_mem.c 
crypto/buffer.c
*/
#define CEL_BIO_MEM_SIZE  8192

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
    CEL_ERR((_T("cel_bio_mem_grow failed")));
    return -1;
}

char *cel_bio_mem_get_read_pointer(CelBio *bio)
{
    CEL_ASSERT(((BUF_MEM *)bio->ptr)->data != NULL);
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
    ret = ((seek >= 0 && (size_t)seek > bm->length) ? (int)bm->length : seek);
    //printf("cel_bio_mem_read_seek ret = %d\r\n", ret);
    if (ret > 0)
    {
        (bm->length) -= ret;
        if (bm->length > 0)
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
        CEL_ERR((_T("cel_bio_mem_write_seek %d error."), seek));
    }
    else
    {
        BIO_clear_retry_flags(bio);
        (bm->length) += seek;
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
    {
        cel_ssl_free(ssl_sock->ssl);
        ssl_sock->ssl = NULL;
    }
    ssl_sock->r_bio = NULL;
    ssl_sock->w_bio = NULL;
    cel_socket_destroy(&(ssl_sock->sock));
}

void _cel_sslsocket_free_derefed(CelSslSocket *ssl_sock)
{
    _cel_sslsocket_destroy_derefed(ssl_sock);
    cel_free(ssl_sock);
}

int cel_sslsocket_init(CelSslSocket *ssl_sock, 
                       CelSocket *sock, CelSslContext *ssl_ctx)
{
    if (&(ssl_sock->sock) != sock)
        memcpy(&(ssl_sock->sock), sock, sizeof(CelSocket));
    if ((ssl_sock->ssl = cel_ssl_new(ssl_ctx)) != NULL)
    {
        SSL_set_mode(ssl_sock->ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);
        SSL_set_mode(ssl_sock->ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
        //SSL_set_mode(ssl_sock->ssl, SSL_MODE_RELEASE_BUFFERS);
        //SSL_set_mode(ssl_sock->ssl, SSL_CTRL_SET_READ_AHEAD);
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
    CEL_ERR((_T("Ssl socket init failed(%s)."), 
        cel_ssl_get_errstr(cel_ssl_get_errno())));

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
        ssl_sock->out.sock_buf.buf = 
            cel_bio_mem_get_read_pointer(ssl_sock->w_bio);
        ssl_sock->out.sock_buf.len = cel_bio_mem_get_length(ssl_sock->w_bio);
        if ((_size = cel_socket_send(&(ssl_sock->sock), 
            &(ssl_sock->out.sock_buf), 1)) <= 0)
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
        ssl_sock->in.sock_buf.buf = 
            cel_bio_mem_get_write_pointer(ssl_sock->r_bio);
        ssl_sock->in.sock_buf.len = 
            cel_bio_mem_get_remaining(ssl_sock->r_bio);
        if ((_size = cel_socket_send(&(ssl_sock->sock),
            &(ssl_sock->in.sock_buf), 1)) <= 0)
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
        if ((error = SSL_get_error(ssl_sock->ssl, result)) 
            == SSL_ERROR_WANT_READ
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

int cel_sslsocket_send(CelSslSocket *ssl_sock, CelAsyncBuf *buffers, int count)
{
    int result;

    if ((result = cel_ssl_write(
        ssl_sock->ssl, buffers->buf, buffers->len)) > 0)
    {
        if (cel_sslsocket_send_bio_mem(ssl_sock) < 0)
            return -1;
        return result;
    }
    return -1;
}

int cel_sslsocket_recv(CelSslSocket *ssl_sock, CelAsyncBuf *buffers, int count)
{
    int result, error;

    while ((result = cel_ssl_read(
        ssl_sock->ssl, buffers->buf, buffers->len)) <= 0)
    {
        if ((error = SSL_get_error(ssl_sock->ssl, result)) 
            == SSL_ERROR_WANT_READ
            || (ERR_get_error() == 0 && cel_sys_geterrno() == 0))
        {
            if (cel_sslsocket_recv_bio_mem(ssl_sock) > 0)
                continue;
        }
        return -1;
    }
    return result;
}

int cel_sslsocket_shutdown(CelSslSocket *ssl_sock, int how)
{
    int result;

    if ((result = cel_ssl_shutdown(ssl_sock->ssl)) >= 0)
    {
        if (cel_sslsocket_send_bio_mem(ssl_sock) < 0)
            return -1;
        cel_socket_shutdown(&(ssl_sock->sock), how);
        return result;
    }
    return -1;
}

int cel_sslsocket_post_send(CelSslSocket *ssl_sock, 
                            CelSocketSendCallbackFunc async_callback)
{
    ssl_sock->out.sock_buf.buf = cel_bio_mem_get_read_pointer(ssl_sock->w_bio);
    ssl_sock->out.sock_buf.len = cel_bio_mem_get_length(ssl_sock->w_bio);
    return cel_socket_async_send(&(ssl_sock->sock),
        &(ssl_sock->out.sock_buf), 1, async_callback, NULL);
}

int cel_sslsocket_post_recv(CelSslSocket *ssl_sock, 
                            CelSocketRecvCallbackFunc async_callback)
{
    ssl_sock->in.sock_buf.buf = cel_bio_mem_get_write_pointer(ssl_sock->r_bio);
    ssl_sock->in.sock_buf.len = cel_bio_mem_get_remaining(ssl_sock->r_bio);
    return cel_socket_async_recv(&(ssl_sock->sock),
        &(ssl_sock->in.sock_buf), 1, async_callback, NULL);
}

void cel_sslsocket_do_handshake(CelSocket *sock, 
                                CelAsyncBuf *buffers, int count, 
                                CelAsyncResult *result)
{
    CelSslSocket *ssl_sock = (CelSslSocket *)sock;
    CelSslAsyncArgs *args = &(ssl_sock->out);

    //CEL_DEBUG((_T("cel_sslsocket_do_handshake")));
    /*_tprintf(_T("fd %d result = %ld error = %d\r\n"), 
        ssl_sock->sock.fd, result->ret, result->error);*/
    if (result->ret <= 0)
    {
        CEL_ERR((_T("cel_sslsocket_do_handshake return failed,")
            _T("result = %ld error = %d"), 
            result->ret, result->error);
        args->result.ret = result->ret;
        args->result.error = result->error;
        args->handshake_callback(ssl_sock, &(args->result)));
        return ;
    }
    if (args->want == SSL_ERROR_WANT_READ)
    {
        cel_bio_mem_write_seek(ssl_sock->r_bio, result->ret);
        if (cel_bio_mem_get_remaining(ssl_sock->r_bio) == 0)
            cel_bio_mem_grow(ssl_sock->r_bio);
    }
    else if (args->want == SSL_ERROR_WANT_WRITE)
    {
        cel_bio_mem_read_seek(ssl_sock->w_bio, result->ret);
    }
    if (args->result.ret != 1)
    {
        cel_ssl_clear_error();
        args->result.ret = cel_ssl_handshake(ssl_sock->ssl);
        if ((args->result.error = 
            SSL_get_error(ssl_sock->ssl, args->result.ret)) == SSL_ERROR_NONE
            || args->result.error == SSL_ERROR_WANT_READ
            || args->result.error == SSL_ERROR_WANT_WRITE)
        {
            if (cel_bio_mem_get_length(ssl_sock->w_bio) > 0)
            {
                args->want = SSL_ERROR_WANT_WRITE;
                if (cel_sslsocket_post_send(
                    ssl_sock, cel_sslsocket_do_handshake) != -1)
                    return ;
                args->result.ret = -1;
            }
            else if ((args->result.error == SSL_ERROR_WANT_READ))
            {
                args->want = SSL_ERROR_WANT_READ;
                if (cel_sslsocket_post_recv(
                    ssl_sock, cel_sslsocket_do_handshake) != -1)
                    return ;
                args->result.ret = -1;
            }
        }
        else
        {
            CEL_ERR((_T("cel_sslsocket_do_handshake  failed,")
                    _T(" fd %d, result %ld error %d %s"), 
                ssl_sock->sock.fd, args->result.ret, args->result.error, 
                cel_ssl_get_errstr(cel_ssl_get_errno())));
        }
    }
    else
    {
        if (cel_bio_mem_get_length(ssl_sock->w_bio) > 0)
        {
            args->want = SSL_ERROR_WANT_WRITE;
            if (cel_sslsocket_post_send(
                ssl_sock, cel_sslsocket_do_handshake) != -1)
                return ;
            args->result.ret = -1;
        }
    }
    args->handshake_callback(ssl_sock, &(args->result));
}

int cel_sslsocket_async_handshake(CelSslSocket *ssl_sock,
                                  CelSslSocketHandshakeCallbackFunc callback,
                                  CelCoroutine *co)
{
    CelSslAsyncArgs *args = &(ssl_sock->out);

    CEL_ASSERT(callback != NULL || co != NULL);
    args->handshake_callback = callback;
    args->co = co;
    cel_ssl_clear_error();
    if ((args->result.ret = cel_ssl_handshake(ssl_sock->ssl)) != 1)
    {
        if ((args->result.error = 
            SSL_get_error(ssl_sock->ssl, args->result.ret)) == SSL_ERROR_NONE
            || args->result.error == SSL_ERROR_WANT_READ
            || args->result.error == SSL_ERROR_WANT_WRITE)
        {
            if (cel_bio_mem_get_length(ssl_sock->w_bio) > 0)
            {
                args->want = SSL_ERROR_WANT_WRITE;
                return cel_sslsocket_post_send(ssl_sock,
                    cel_sslsocket_do_handshake);
            }
            else if ((args->result.error == SSL_ERROR_WANT_READ))
            {
                args->want = SSL_ERROR_WANT_READ;
                return cel_sslsocket_post_recv(
                    ssl_sock, cel_sslsocket_do_handshake);
            }
        }
        CEL_ERR((_T("ssl handshake %d error %d %s"), 
            args->result.ret, args->result.error, 
            cel_ssl_get_errstr(cel_ssl_get_errno())));
    }
    else
    {
        if (cel_bio_mem_get_length(ssl_sock->w_bio) > 0)
        {
            args->want = SSL_ERROR_WANT_WRITE;
            return cel_sslsocket_post_send(
                ssl_sock, cel_sslsocket_do_handshake);
        }
    }
    args->handshake_callback(ssl_sock, &(args->result));
    return 0;
}

void cel_sslsocket_do_send(CelSocket *sock, 
                           CelAsyncBuf *buffers, int count, 
                           CelAsyncResult *result)
{
    CelSslSocket *ssl_sock = (CelSslSocket *)sock;
    CelSslAsyncArgs *args = &(ssl_sock->out);

    /*_tprintf(_T("result = %ld error = %d\r\n"), 
        result->ret, result->error);*/
    if (result->ret > 0)
    {
        cel_bio_mem_read_seek(ssl_sock->w_bio, result->ret);
        if (cel_bio_mem_get_length(ssl_sock->w_bio) > 0)
        {
            if (cel_sslsocket_post_send(ssl_sock, cel_sslsocket_do_send) != -1)
                return ;
            args->result.ret = -1;
        }
    }
    else
    {
        args->result.ret = result->ret;
        args->result.error = result->error;
    }
    args->send_callback(ssl_sock, args->ssl_buf, 1, &(args->result));
}

int cel_sslsocket_async_send(CelSslSocket *ssl_sock, 
                             CelAsyncBuf *buffers, int count,
                             CelSslSocketSendCallbackFunc callback,
                             CelCoroutine *co)
{
    CelSslAsyncArgs *args = &(ssl_sock->out);

    CEL_ASSERT(callback != NULL || co != NULL);
    args->ssl_buf = buffers;
    args->send_callback = callback;
    args->co = co;
    //printf("args->ssl_buf->len = %d\r\n", args->ssl_buf->len);
    cel_ssl_clear_error();
    if ((args->result.ret = cel_ssl_write(
        ssl_sock->ssl, args->ssl_buf->buf, args->ssl_buf->len)) > 0)
    {
        if (cel_bio_mem_get_length(ssl_sock->w_bio) > 0)
        {
            /*_tprintf(_T("remaining %d ret %d\r\n"), 
                (int)cel_bio_mem_get_length(ssl_sock->w_bio), args->result.ret);*/
            return cel_sslsocket_post_send(ssl_sock, cel_sslsocket_do_send);
        }
    }
    args->result.ret = -1;
    args->result.error = SSL_get_error(ssl_sock->ssl, args->result.ret);
    CEL_ERR((_T("ssl async write error %d %s"), 
        args->result.error, cel_ssl_get_errstr(cel_ssl_get_errno())));
    args->send_callback(ssl_sock, buffers, 1, &(args->result));

    return 0;
}

void cel_sslsocket_do_recv(CelSocket *sock, 
                           CelAsyncBuf *buffers, int count, 
                           CelAsyncResult *result)
{
    CelSslSocket *ssl_sock = (CelSslSocket *)sock;
    CelSslAsyncArgs *args = &(ssl_sock->in);

    /*_tprintf(_T("result = %ld error = %d\r\n"), 
        result->ret, result->error);*/
    if (result->ret > 0)
    {
        cel_bio_mem_write_seek(ssl_sock->r_bio, result->ret);
        if (cel_bio_mem_get_remaining(ssl_sock->r_bio) == 0)
            cel_bio_mem_grow(ssl_sock->r_bio);
        cel_ssl_clear_error();
        if ((args->result.ret = cel_ssl_read(
            ssl_sock->ssl, args->ssl_buf->buf, args->ssl_buf->len)) <= 0
            && (args->result.error = SSL_get_error(
            ssl_sock->ssl, args->result.ret)) != SSL_ERROR_ZERO_RETURN)
        {
            if (args->result.error == SSL_ERROR_WANT_READ)
            {
                if (cel_sslsocket_post_recv(
                    ssl_sock, cel_sslsocket_do_recv) != -1)
                    return ;
                args->result.ret = -1;
            }
            CEL_ERR((_T("ssl do read (%p %d)%d error %d %s"), 
                buffers->buf, buffers->len, (int)args->result.ret, 
                args->result.error, 
                (args->result.error != SSL_ERROR_SYSCALL 
                ? cel_ssl_get_errstr(cel_ssl_get_errno())
                : cel_geterrstr(cel_sys_geterrno()))));
        }
        //puts(buffers->buf);
    }
    else
    {
        args->result.ret = result->ret;
        args->result.error = result->error;
    }
    args->recv_callback(ssl_sock, args->ssl_buf, 1, &(args->result));
}

int cel_sslsocket_async_recv(CelSslSocket *ssl_sock, 
                             CelAsyncBuf *buffers, int count,
                             CelSslSocketSendCallbackFunc callback,
                             CelCoroutine *co)
{
    CelSslAsyncArgs *args = &(ssl_sock->in);

    CEL_ASSERT(callback != NULL || co != NULL);
    args->ssl_buf = buffers;
    args->recv_callback = callback;
    args->co = co;
    cel_ssl_clear_error();
    if ((args->result.ret = cel_ssl_read(
        ssl_sock->ssl, args->ssl_buf->buf, args->ssl_buf->len)) <= 0)
    {
        if ((args->result.error = SSL_get_error(
            ssl_sock->ssl, args->result.ret)) == SSL_ERROR_WANT_READ
            || (ERR_get_error() == 0 && cel_sys_geterrno() == 0))
        {
            return cel_sslsocket_post_recv(ssl_sock, cel_sslsocket_do_recv);
        }
        CEL_ERR((_T("ssl async read (%p %d)%d error %d %s"), 
            buffers->buf, buffers->len, (int)args->result.ret, 
            args->result.error, (args->result.error != SSL_ERROR_SYSCALL 
            ? cel_ssl_get_errstr(cel_ssl_get_errno())
            : cel_geterrstr(cel_sys_geterrno()))));
        args->result.ret = -1;
    }
    args->recv_callback(ssl_sock, args->ssl_buf, 1, &(args->result));

    return 0;
}

void cel_sslsocket_do_shutdown(CelSocket *sock, 
                               CelAsyncBuf *buffers, int count, 
                               CelAsyncResult *result)
{
    CelSslSocket *ssl_sock = (CelSslSocket *)sock;
    CelSslAsyncArgs *args = &(ssl_sock->out);

    /*_tprintf(_T("result = %ld error = %d\r\n"), 
        result->ret, result->error);*/
    if (result->ret > 0)
    {
        cel_bio_mem_read_seek(ssl_sock->w_bio, result->ret);
        if (cel_bio_mem_get_length(ssl_sock->w_bio) > 0)
        {
            if (cel_sslsocket_post_send(
                ssl_sock, cel_sslsocket_do_shutdown) != -1)
                return ;
            args->result.ret = -1;
        }
    }
    else
    {
        args->result.ret = result->ret;
        args->result.error = result->error;
    }
    cel_socket_shutdown(&(ssl_sock->sock), 2);
    args->shutdown_callback(ssl_sock, &(args->result));
}

int cel_sslsocket_async_shutdown(CelSslSocket *ssl_sock, 
                                 CelSslSocketShutdownCallbackFunc callback,
                                 CelCoroutine *co)
{
    CelSslAsyncArgs *args = &(ssl_sock->out);

    CEL_ASSERT(callback != NULL || co != NULL);
    args->shutdown_callback = callback;
    args->co = co;
    //printf("args->ssl_buf->len = %d\r\n", args->ssl_buf->len);
    cel_ssl_clear_error();
    if ((args->result.ret = cel_ssl_shutdown(ssl_sock->ssl)) >= 0)
    {
        if (cel_bio_mem_get_length(ssl_sock->w_bio) > 0)
        {
            /*_tprintf(_T("remaining %d ret %d\r\n"), 
            (int)cel_bio_mem_get_length(ssl_sock->w_bio), args->result.ret);*/
            return cel_sslsocket_post_send(
                ssl_sock, cel_sslsocket_do_shutdown);
        }
    }
    args->result.ret = -1;
    args->result.error = SSL_get_error(ssl_sock->ssl, args->result.ret);
    CEL_ERR((_T("ssl async shutdown error %d %s"), 
        args->result.error, cel_ssl_get_errstr(cel_ssl_get_errno())));
    cel_socket_shutdown(&(ssl_sock->sock), 2);
    args->shutdown_callback(ssl_sock, &(args->result));

    return 0;
}
