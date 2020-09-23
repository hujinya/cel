/**
 * CEL(C Extension Library)
 * Copyright (C)2008 Hu Jinya(hu_jinya@163.com) 
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
#include "cel/net/ssl.h"
#include "cel/log.h"
#include "cel/error.h"

CelKeyword ssl_methods[] = 
{
    //{ sizeof(_T("DTLS")) - 1, _T("DTLS"), (const void *)DTLS_method },
    { sizeof(_T("DTLSv1")) - 1, _T("DTLSv1"), (const void *)DTLSv1_method },
    //{ sizeof(_T("DTLSv1.2")) - 1, _T("DTLSv1.2"), (const void *)DTLSv1_2_method }, 
#ifndef OPENSSL_NO_SSL2
    { sizeof(_T("SSLv2")) - 1, _T("SSLv2"), (const void *)SSLv2_method },
#endif
    { sizeof(_T("SSLv23")) - 1, _T("SSLv23"), (const void *)SSLv23_method },
    { sizeof(_T("SSLv3")) - 1, _T("SSLv3"), (const void *)SSLv3_method },
    { sizeof(_T("TLSv1")) - 1, _T("TLSv1"), (const void *)TLSv1_method },
    { sizeof(_T("TLSv1.1")) - 1, _T("TLSv1.1"), (const void *)TLSv1_1_method },
    { sizeof(_T("TLSv1.2")) - 1, _T("TLSv1.2"), (const void *)TLSv1_2_method }
};

typedef const SSL_METHOD *(*CelSslMethodFunc)(void);

WCHAR *cel_ssl_get_errstr_w(unsigned long err_no)
{
    CelErr *err = _cel_err(); 
    cel_mb2unicode(cel_ssl_get_errstr_a(err_no), -1, 
		err->stic.w_buffer, CEL_ERRSLEN);
    return err->stic.w_buffer;
}

CelSslContext *cel_sslcontext_new(CelSslMethod method)
{
    SSL_CTX *ctx;

    if (method == CEL_SSL_METHOD_UNDEFINED
        || method >= CEL_SSL_METHOD_COUNT)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("Ssl method undefined.")));
        return NULL;
    }
    //printf("method = %d\r\n", method);
    if ((ctx = SSL_CTX_new(((CelSslMethodFunc)ssl_methods[method].value)())) == NULL)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("(SSL_CTX_new:%s.)"), cel_ssl_get_errstr(cel_ssl_get_errno())));
        return NULL;
    }
    //SSL_CTX_set_quiet_shutdown(ctx, 1);
    
    SSL_CTX_set_options(ctx, SSL_OP_ALL);
    //SSL_CTX_set_mode(ctx, SSL_MODE_RELEASE_BUFFERS);
    SSL_CTX_set_read_ahead(ctx, 1);

    return ctx;
}

int cel_sslcontext_set_own_cert(CelSslContext *ssl, char *cert_file, 
                                char *key_file, char *pswd)
{
    /* Set cert file password */
    if (pswd !=NULL)
        SSL_CTX_set_default_passwd_cb_userdata(ssl, pswd);
    /* Load cert file */
    if (SSL_CTX_use_certificate_file(ssl, cert_file, SSL_FILETYPE_PEM) <= 0)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("(SSL_CTX_use_certificate_file %s:%s.)"), 
            cert_file,
            cel_ssl_get_errstr(cel_ssl_get_errno())));
        return -1;
    }
    /* Load private key */
    if (key_file == NULL)
        key_file = cert_file;
    if (SSL_CTX_use_PrivateKey_file(ssl, key_file, SSL_FILETYPE_PEM) <= 0)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("(SSL_CTX_use_PrivateKey_file %s:%s.)"), 
            key_file,
            cel_ssl_get_errstr(cel_ssl_get_errno())));
        return -1;
    }
    /* Check cert and key */
    if (!SSL_CTX_check_private_key(ssl))
    {
        CEL_SETERR((CEL_ERR_LIB, _T("(SSL_CTX_check_private_key:%s.)"), 
            cel_ssl_get_errstr(cel_ssl_get_errno())));
        return -1;
    }
    return 0;
}

int cel_sslcontext_set_ca_chain(CelSslContext *ssl, char *ca_file, char *ca_path)
{
    if (SSL_CTX_load_verify_locations(ssl, ca_file, ca_path) <= 0)
    {
        CEL_SETERR((CEL_ERR_LIB, _T("(SSL_CTX_load_verify_locations:%s.)"), 
            cel_ssl_get_errstr(cel_ssl_get_errno())));
        return -1;
    }
    if (ca_file != NULL)
    {
        SSL_CTX_set_client_CA_list(ssl, SSL_load_client_CA_file(ca_file));
    }
    return 0;
}
