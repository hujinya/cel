#include "cel/crypto/rc4.h"
#include "cel/log.h"

/* Debug defines */
#define Debug(args)  /* cel_log_debug args */
#define Warning(args) CEL_SETERRSTR(args) /* cel_log_warning args */
#define Err(args)   CEL_SETERRSTR(args) /* cel_log_err args */

/*
 * RC4 key schedule
 */
void cel_arc4_setup(CelRc4Context *ctx, const BYTE *key, unsigned int keylen)
{
    int i, j, a;
    unsigned int k;
    BYTE *m;

    ctx->x = 0;
    ctx->y = 0;
    m = ctx->m;

    for (i = 0; i < 256; i++)
        m[i] = (BYTE)i;
    j = k = 0;

    for (i = 0; i < 256; i++, k++)
    {
        if (k >= keylen) k = 0;

        a = m[i];
        j = (j + a + key[k]) & 0xFF;
        m[i] = m[j];
        m[j] = (BYTE)a;
    }
}

/*
 * RC4 cipher function
 */
int cel_arc4_crypt(CelRc4Context *ctx, size_t len, const BYTE *input, BYTE *output)
{
    int x, y, a, b;
    size_t i;
    BYTE *m;

    x = ctx->x;
    y = ctx->y;
    m = ctx->m;

    for (i = 0; i < len; i++)
    {
        x = (x + 1) & 0xFF; a = m[x];
        y = (y + a) & 0xFF; b = m[y];

        m[x] = (BYTE)b;
        m[y] = (BYTE)a;

        output[i] = (BYTE) (input[i] ^ m[(BYTE) (a + b)]);
    }

    ctx->x = x;
    ctx->y = y;

    return (0);
}
