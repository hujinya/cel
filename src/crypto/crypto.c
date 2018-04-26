#include "cel/crypto/crypto.h"
#include "cel/log.h"

/* Debug defines */
#define Debug(args) CEL_SETERRSTR(args) /*cel_log_debug args*/
#define Warning(args) CEL_SETERRSTR(args) /*cel_log_warning args*/
#define Err(args)   CEL_SETERRSTR(args) /*cel_log_err args*/

#ifndef HAVE_OPENSSL_CRYPTO_H

int  cel_mdcontext_init(CelMdContext *ctx, CelMd *md)
{
    return 0;
}

void cel_mdcontext_destroy(CelMdContext *ctx)
{
    ;
}

int cel_mdcontext_update(CelMdContext *ctx, const void *input, size_t ilen)
{
    return 0;
}

int cel_mdcontext_finish(CelMdContext *ctx, BYTE *out, size_t *olen)
{
    return 0;
}

int cel_mdcontext_md(CelMd *md, const void *input, size_t ilen, 
                     BYTE *out, size_t *olen)
{
    return 0;
}

int cel_mdcontext_hmac_init(CelMdContext *ctx, CelMd *md, 
                            const BYTE *key, size_t keyl)
{
    return 0;
}

void cel_mdcontext_hmac_destroy(CelMdContext *ctx)
{
    ;
}

int cel_mdcontext_hmac_update(CelMdContext *ctx, const void *input, size_t ilen)
{
    return 0;
}

int cel_mdcontext_hmac_finish(CelMdContext *ctx, BYTE *out, size_t *olen)
{
    return 0;
}

int cel_mdcontext_hmac_md(CelMd *md, const BYTE *key, size_t keyl,
                          const void *input, size_t ilen, 
                          BYTE *out, size_t *olen)
{
    retrun 0;
}

int cel_ciphercontext_init(CelCipherContext *ctx, CelCipher *cipher, 
                           const BYTE *key, const BYTE *iv, 
                           int enc)
{
    return 0;
}

void cel_ciphercontext_destroy(CelCipherContext *ctx)
{
    return 0;
}

int cel_ciphercontext_update(CelCipherContext *ctx, BYTE *input, size_t ilen, 
                             BYTE *out, size_t *olen)
{
    return 0;
}

int cel_ciphercontext_finish(CelCipherContext *ctx, BYTE *out, size_t *olen)
{
    return 0;
}
#else
#include "cel/thread.h"
#ifdef _CEL_MULTITHREAD
static CelMutex *s_cryptomutex = NULL;
static long *s_cryptomutex_count = NULL;

static unsigned long cel_cryptothread_id(void)
{
    unsigned long ret;

    ret = (unsigned long)cel_thread_getid();

    return ret;
}

static void cel_cryptothread_locking_callback(int mode, int type,
                                              const char *file, int line)
{
    (void)file;
    (void)line;
    if (mode & CRYPTO_LOCK)
    {
        if (cel_mutex_lock(&(s_cryptomutex[type])) != 0)
            _tprintf(_T("Crypto mutex %d lock failed.\r\n"), type);
        s_cryptomutex_count[type]++;
        /*_tprintf(_T("Type %d lock %ld, thread %d\r\n"), 
            type, s_cryptomutex_count[type], (int)cel_thread_getid());*/
    }
    else
    {
        cel_mutex_unlock(&(s_cryptomutex[type]));
        /*_tprintf(_T("Type %d unlock, thread %d\r\n"), 
            type, (int)cel_thread_getid());*/
    }
}
#endif

void cel_cryptomutex_register(CelCryptoThreadIdFunc threadidfunc, 
                              CelCryptoThreadLockingFunc threadlockingfunc)
{
#ifdef _CEL_MULTITHREAD
    int i;
#endif

    if (threadidfunc == NULL || threadlockingfunc == NULL)
    {
#ifdef _CEL_MULTITHREAD
        //_tprintf(_T("CRYPTO_num_locks %d\r\n"), CRYPTO_num_locks());
        s_cryptomutex = (CelMutex *)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(CelMutex));
        s_cryptomutex_count = (long *)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(long));

        for (i = 0; i < CRYPTO_num_locks(); i++)
        {
            s_cryptomutex_count[i] = 0;
            if (cel_mutex_init(&(s_cryptomutex[i]), NULL) != 0)
                _tprintf(_T("Crypto mutex %d init failed.\r\n"), i);
        }
        CRYPTO_set_id_callback(cel_cryptothread_id);
        CRYPTO_set_locking_callback(cel_cryptothread_locking_callback);

        return;
#else
        return;
#endif
    }
    CRYPTO_set_id_callback(threadidfunc);
    CRYPTO_set_locking_callback(threadlockingfunc);

    return;
}

void cel_cryptomutex_unregister(void)
{
#ifdef _CEL_MULTITHREAD
    int i;
#endif
   
    CRYPTO_set_id_callback(NULL);
    CRYPTO_set_locking_callback(NULL);
#ifdef _CEL_MULTITHREAD
    if (s_cryptomutex != NULL)
    {
        for (i = 0; i < CRYPTO_num_locks(); i++)
        {
            cel_mutex_destroy(&(s_cryptomutex[i]));
        }
        OPENSSL_free(s_cryptomutex);
        s_cryptomutex = NULL;
    }
    if (s_cryptomutex_count != NULL)
    {
        OPENSSL_free(s_cryptomutex_count);
        s_cryptomutex_count = NULL;
    }
#endif
}
#endif
