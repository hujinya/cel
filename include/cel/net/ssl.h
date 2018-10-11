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
#ifndef __CEL_SSL_H__
#define __CEL_SSL_H__

#include "cel/config.h"
#ifdef HAVE_OPENSSL_SSL_H
#include "cel/types.h"
#include "cel/allocator.h"
#include "cel/error.h"
#include "cel/convert.h"
#include "cel/keyword.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _CelSslMethod
{
    CEL_SSL_METHOD_UNDEFINED = -1,
    //CEL_SSL_METHOD_DTLS,     /* DTLS 1.0 and 1.2 */
    CEL_SSL_METHOD_DTLSv1,   /* DTLSv1.0 */
    //CEL_SSL_METHOD_DTLSv1_2, /* DTLSv1.2 */
#ifndef OPENSSL_NO_SSL2
    CEL_SSL_METHOD_SSLv2,    /* SSLv2 */
#endif
    CEL_SSL_METHOD_SSLv23,   /* Negotiate highest available SSL/TLS version */
    CEL_SSL_METHOD_SSLv3,    /* SSLv3 */
    CEL_SSL_METHOD_TLSv1,    /* TLSv1.0 */
    CEL_SSL_METHOD_TLSv1_1,  /* TLSv1.1 */
    CEL_SSL_METHOD_TLSv1_2,  /* TLSv1.2 */
    CEL_SSL_METHOD_COUNT
}CelSslMethod;

typedef enum _CelSslVerifyMode
{
    CEL_SSLVM_NONE, 
    CEL_SSLVM_OPTIONAL, 
    CEL_SSLVM_REQUIRED, 
}CelSslVerifyMode;

typedef enum _CelSslEndpoint
{
    CEL_SSLEP_CLIENT, 
    CEL_SSLEP_SERVER
}CelSslEndpoint;

typedef int (* CelSslVerifyCallbackFunc) (int ok, X509_STORE_CTX *peer);
typedef BIO CelBio;
typedef SSL_CTX CelSslContext;
typedef SSL CelSsl;

extern CelKeyword ssl_methods[];

#define cel_ssllibrary_init() \
    /*CRYPTO_set_mem_functions(cel_malloc, cel_realloc, cel_free),*/\
    SSL_load_error_strings(), \
    SSL_library_init()
/* SSL_library_init include OpenSSL_add_all_algorithms */

#define cel_ssl_clear_error() ERR_clear_error()
#define cel_ssl_get_error(ssl, ret) SSL_get_error(ssl, ret)

#define cel_ssl_get_errno() ERR_get_error()

#define cel_ssl_get_errstr_a(err_no) ERR_error_string((err_no), NULL)
WCHAR *cel_ssl_get_errstr_w(unsigned long err_no);

#define cel_sslcontext_method(method_str) \
    (CelSslMethod)cel_keyword_binary_search(\
    ssl_methods, CEL_SSL_METHOD_COUNT, method_str, _tcslen(method_str))

CelSslContext *cel_sslcontext_new(CelSslMethod method);

/* void cel_sslcontext_free(CelSslContext *ctx); */
#define cel_sslcontext_free(ctx) SSL_CTX_free(ctx)

int cel_sslcontext_set_own_cert(CelSslContext *ctx, 
                                char *cert_file, char *key_file, char *pswd);
/*
 * SSL_VERIFY_NONE:     peer certificate is not checked (default), this is 
 *                      insecure and SHOULD be avoided.
 * SSL_VERIFY_OPTIONAL: peer certificate is checked, however the handshake 
 *                      continues  even if verification failed;  
 *                      ssl_get_verify_result() can becalled after 
 *                      the handshake is complete.
 * SSL_VERIFY_REQUIRED: peer *must* present a valid certificate, handshake is 
 *                      aborted if verification failed.
 */
static __inline 
void cel_sslcontext_set_verify(CelSslContext *ctx, 
                               CelSslVerifyMode mode, 
                               CelSslVerifyCallbackFunc callback_func)
{
    int _mode;

    if (mode == CEL_SSLVM_REQUIRED)
    {
        _mode = SSL_VERIFY_PEER 
            |SSL_VERIFY_FAIL_IF_NO_PEER_CERT
            |SSL_VERIFY_CLIENT_ONCE;
    }
    SSL_CTX_set_verify(ctx, _mode, callback_func);
}    
/* void cel_sslcontext_set_verify_depth(CelSslContext *ctx, int depth)*/
#define cel_sslcontext_set_verify_depth(ctx, depth) \
    SSL_CTX_set_verify_depth(ctx, depth)
int cel_sslcontext_set_ca_chain(CelSslContext *ctx, 
                                char *ca_file, char *ca_path);

/* int cel_sslcontext_set_ciphersuites(CelSslContext *ctx, cosnt char *str)*/
#define cel_sslcontext_set_ciphersuites(ctx, str) \
    (SSL_CTX_set_cipher_list(ctx, str) == 1 ? 0 : -1)
/* const char *cel_sslcontext_get_ciphersuites(CelSslContext *ctx)*/
#define cel_sslcontext_get_ciphersuites(ctx) \
    SSL_CIPHER_get_name(SSL_get_current_cipher(ctx))

/* CelSsl *cel_ssl_new(CelSslContext *ctx); */
#define cel_ssl_new(ctx) SSL_new(ctx)
/* void cel_ssl_free(CelSsl *ssl); */
#define cel_ssl_free(ssl) SSL_free(ssl)

/* int cel_ssl_set_fd(CelSsl *ssl, int fd); */
#define cel_ssl_set_fd(ssl, fd) SSL_set_fd(ssl, fd)
/* void cel_ssl_set_endpoint(CelSsl *ssl, CelSslEndpoint endpoint)*/
#define cel_ssl_set_endpoint(ssl, endpoint) (endpoint == CEL_SSLEP_CLIENT \
    ? SSL_set_connect_state(ssl) : SSL_set_accept_state(ssl))
/* void cel_ssl_set_timeout(CelSsl *ssl, int timeout)*/
#define cel_ssl_set_timeout(ssl, timeout) SSL_set_timeout(ssl, timeout)

#define cel_ssl_get_cipher_version(ssl) SSL_CIPHER_get_version(ssl)
#define cel_ssl_get_cipher(ssl) SSL_get_cipher(ssl)

#include "cel/multithread.h"

static __inline int cel_ssl_accept(CelSsl *ssl)
{
    int ret;
    cel_multithread_mutex_lock(CEL_MT_MUTEX_SSL_HANDSHAKE);
    ret = SSL_accept(ssl);
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_SSL_HANDSHAKE);
    return ret;
};
static __inline int cel_ssl_connect(CelSsl *ssl)
{
    int ret;
    cel_multithread_mutex_lock(CEL_MT_MUTEX_SSL_HANDSHAKE);
    ret = SSL_connect(ssl);
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_SSL_HANDSHAKE);
    return ret;
}
static __inline int cel_ssl_handshake(CelSsl *ssl) 
{
    int ret;
    cel_multithread_mutex_lock(CEL_MT_MUTEX_SSL_HANDSHAKE);
    ret = SSL_do_handshake(ssl);
    cel_multithread_mutex_unlock(CEL_MT_MUTEX_SSL_HANDSHAKE);
    return ret;
}
/* int cel_ssl_read(CelSsl *ssl, void *buf, int size); */
#define cel_ssl_read(ssl, buf, size) SSL_read(ssl, buf, size)
/* int cel_ssl_write(CelSsl *ssl, void *buf, int size); */
#define cel_ssl_write(ssl, buf, size) SSL_write(ssl, buf, size)

#define cel_ssl_shutdown(ssl) SSL_shutdown(ssl)

#ifdef _UNICODE
#define cel_ssl_get_errstr cel_ssl_get_errstr_w
#else
#define cel_ssl_get_errstr cel_ssl_get_errstr_a
#endif

#ifdef __cplusplus
}
#endif

#endif

#endif //HAVE_OPENSSL_SSL_H
