/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#include "cel/crypto/jwt.h"
#include "cel/allocator.h"
#include "cel/keyword.h"
#include "cel/crypto/base64.h"
#include "cel/crypto/crypto.h"
#include <string.h> /* strcasecmp */

static CelKeywordA s_jwtalg[] =
{
    { sizeof("ES256") - 1, "ES256"},
    { sizeof("ES384") - 1, "ES384"},
    { sizeof("ES512") - 1, "ES512"},
    { sizeof("HS256") - 1, "HS256"},
    { sizeof("HS512") - 1, "HS512"},
    { sizeof("RS256") - 1, "RS256"},
    { sizeof("RS384") - 1, "RS384"},
    { sizeof("RS512") - 1, "RS512"}
};

int cel_jwt_init(CelJwt *jwt)
{
    memset(jwt, 0, sizeof(CelJwt));
    return cel_json_init(&(jwt->payload));
}

void cel_jwt_destroy(CelJwt *jwt)
{
    jwt->alg = CEL_JWT_ALG_NONE;
    cel_json_destroy(&(jwt->payload));
}

CelJwt *cel_jwt_new()
{
    CelJwt *jwt;

    if ((jwt = (CelJwt *)cel_malloc(sizeof(CelJwt))) != NULL)
    {
        if (cel_jwt_init(jwt) == 0)
            return jwt;
        cel_free(jwt);
    }
    return NULL;
}

void cel_jwt_free(CelJwt *jwt)
{
    cel_jwt_destroy(jwt);
    cel_free(jwt);
}

static int cel_jwt_base64url_decode_json(CelJson *json, const char *src, int n)
{
    size_t len;
    BYTE *buf;

    len = cel_base64url_decode_size(n);
    buf = (BYTE *)alloca(len);
    if (cel_base64url_decode(buf, &len, (BYTE *)src, n) == 0
        && cel_json_init_buffer(json, (char *)buf, len) == 0)
        return 0;
    return -1;
}

static int cel_jwt_sign_sha_hmac(BYTE *out, size_t *out_len,
                                 const EVP_MD *alg, const char *str, size_t n,
                                 const BYTE *key, int key_len)
{
    HMAC(alg, key, key_len, (const unsigned char *)str, n, 
        out, (unsigned int *)out_len);
    return 0;
}

static int cel_jwt_verify_sha_hmac(const EVP_MD *alg, 
                                   const char *head, size_t head_len, 
                                   const char *sig, size_t sig_len,
                                   const BYTE *key, int key_len)
{
    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    BYTE *new_sig;
    size_t new_sig_len;

    if (HMAC(alg, key, key_len, 
        (const unsigned char *)head, head_len, md, &md_len) == NULL)
    {
        puts("HMAC return NULL");
        return -1;
    }
    new_sig_len = cel_base64url_encode_size(md_len);
    new_sig = (BYTE *)alloca(new_sig_len);
    if (cel_base64url_encode(new_sig, &new_sig_len, md, md_len) != 0)
    {
        puts("cel_jwt_verify_sha_hmac->cel_base64url_encode return -1");
        return -1;
    }
    if (sig_len != new_sig_len
        || memcmp(new_sig, sig, sig_len) != 0)
    {
        printf("new sig(%d) %s not match sig(%d) %s\r\n", 
            (int)new_sig_len, new_sig, (int)sig_len, sig);
        return -1;
    }
    return 0;
}

static int cel_jwt_sign_sha_pem(BYTE *out, size_t *out_len, 
                                const EVP_MD *alg, const char *str, size_t n,
                                int type,
                                const BYTE *key, int key_len)
{
    EVP_MD_CTX *mdctx = NULL;
    ECDSA_SIG *ec_sig = NULL;
    BIO *bufkey = NULL;
    EVP_PKEY *pkey = NULL;
    unsigned char *sig;
    int ret = 0;
    size_t slen;
    unsigned int degree, bn_len, r_len, s_len, buf_len;
    unsigned char *raw_buf;

    bufkey = BIO_new_mem_buf(key, key_len);
    if (bufkey == NULL)
    {
        ret = -1;
        goto cel_jwt_sign_sha_pem_done;
    }
    /* 
     * This uses OpenSSL's default passphrase callback if needed. The
     * library caller can override this in many ways, all of which are
     * outside of the scope of LibJWT and this is documented in jwt.h. 
     */
    pkey = PEM_read_bio_PrivateKey(bufkey, NULL, NULL, NULL);
    if (pkey == NULL)
    {
        ret = -1;
        goto cel_jwt_sign_sha_pem_done;
    }

    if (pkey->type != type)
    {
        ret = -1;
        goto cel_jwt_sign_sha_pem_done;
    }

    mdctx = EVP_MD_CTX_create();
    if (mdctx == NULL)
    {
        ret = -1;
        goto cel_jwt_sign_sha_pem_done;
    }

    /* Initialize the DigestSign operation using alg */
    if (EVP_DigestSignInit(mdctx, NULL, alg, NULL, pkey) != 1)
    {
        ret = -1;
        goto cel_jwt_sign_sha_pem_done;
    }

    /* Call update with the message */
    if (EVP_DigestSignUpdate(mdctx, str, n) != 1)
    {
        ret = -1;
        goto cel_jwt_sign_sha_pem_done;
    }

    /* 
     * First, call EVP_DigestSignFinal with a NULL sig parameter to get length
     * of sig. Length is returned in slen 
     */
    if (EVP_DigestSignFinal(mdctx, NULL, &slen) != 1)
    {
        ret = -1;
        goto cel_jwt_sign_sha_pem_done;
    }

    /* Allocate memory for signature based on returned size */
    sig = (unsigned char *)alloca(slen);
    if (sig == NULL)
    {
        ret = -1;
        goto cel_jwt_sign_sha_pem_done;
    }

    /* Get the signature */
    if (EVP_DigestSignFinal(mdctx, sig, &slen) != 1)
    {
        ret = -1;
        goto cel_jwt_sign_sha_pem_done;
    }

    if (pkey->type != EVP_PKEY_EC) 
    {
        if (slen > *out_len - 1)
        {
            ret = -1;
            goto cel_jwt_sign_sha_pem_done;
        }
        memcpy(out, sig, slen);
        out[slen] = '\0';
    } 
    else
    {
        EC_KEY *ec_key;

        /* For EC we need to convert to a raw format of R/S. */

        /* Get the actual ec_key */
        ec_key = EVP_PKEY_get1_EC_KEY(pkey);
        if (ec_key == NULL)
        {
            ret = -1;
            goto cel_jwt_sign_sha_pem_done;
        }

        degree = EC_GROUP_get_degree(EC_KEY_get0_group(ec_key));

        EC_KEY_free(ec_key);

        /* Get the sig from the DER encoded version. */
        ec_sig = d2i_ECDSA_SIG(NULL, (const unsigned char **)&sig, slen);
        if (ec_sig == NULL)
        {
            ret = -1;
            goto cel_jwt_sign_sha_pem_done;
        }

        r_len = BN_num_bytes(ec_sig->r);
        s_len = BN_num_bytes(ec_sig->s);
        bn_len = (degree + 7) / 8;
        if ((r_len > bn_len) || (s_len > bn_len))
        {
            ret = -1;
            goto cel_jwt_sign_sha_pem_done;
        }

        buf_len = 2 * bn_len;
        raw_buf = (unsigned char *)alloca(buf_len);
        if (raw_buf == NULL)
        {
            ret = -1;
            goto cel_jwt_sign_sha_pem_done;
        }

        /* Pad the bignums with leading zeroes. */
        memset(raw_buf, 0, buf_len);
        BN_bn2bin(ec_sig->r, raw_buf + bn_len - r_len);
        BN_bn2bin(ec_sig->s, raw_buf + buf_len - s_len);

        /*BIO_write(out, raw_buf, buf_len);
        if (BIO_flush(out));*/
        if (buf_len > *out_len - 1)
        {
            ret = -1;
            goto cel_jwt_sign_sha_pem_done;
        }
        memcpy(out, raw_buf, buf_len);
        out[buf_len] = '\0';
    }

cel_jwt_sign_sha_pem_done:
    if (bufkey)
        BIO_free(bufkey);
    if (pkey)
        EVP_PKEY_free(pkey);
    if (mdctx)
        EVP_MD_CTX_destroy(mdctx);
    if (ec_sig)
        ECDSA_SIG_free(ec_sig);

    return ret;
}

static int cel_jwt_verify_sha_pem(const EVP_MD *alg, int type,
                                  const char *head, size_t head_len,
                                  const char *sig_b64, size_t sig_len,
                                  const BYTE *key, int key_len)
{
    unsigned char *sig = NULL;
    EVP_MD_CTX *mdctx = NULL;
    ECDSA_SIG *ec_sig = NULL;
    EVP_PKEY *pkey = NULL;
    BIO *bufkey = NULL;
    int ret = 0;
    int slen;

    slen = cel_base64url_decode_size(sig_len);
    sig = (unsigned char *)alloca(slen);
    if (cel_base64url_decode(sig, (size_t *)&slen,
        (const BYTE *)sig_b64, sig_len) == -1)
        goto cel_jwt_verify_sha_pem_done;

    bufkey = BIO_new_mem_buf(key, key_len);
    if (bufkey == NULL)
        goto cel_jwt_verify_sha_pem_done;

    /* 
     * This uses OpenSSL's default passphrase callback if needed. The
     * library caller can override this in many ways, all of which are
     * outside of the scope of LibJWT and this is documented in jwt.h.
     */
    pkey = PEM_read_bio_PUBKEY(bufkey, NULL, NULL, NULL);
    if (pkey == NULL)
        goto cel_jwt_verify_sha_pem_done;

    if (pkey->type != type)
        goto cel_jwt_verify_sha_pem_done;

    /* Convert EC sigs back to ASN1. */
    if (pkey->type == EVP_PKEY_EC) 
    {
        unsigned int degree, bn_len;
        unsigned char *p;
        EC_KEY *ec_key;

        ec_sig = ECDSA_SIG_new();
        if (ec_sig == NULL)
            goto cel_jwt_verify_sha_pem_done;

        /* Get the actual ec_key */
        ec_key = EVP_PKEY_get1_EC_KEY(pkey);
        if (ec_key == NULL)
            goto cel_jwt_verify_sha_pem_done;

        degree = EC_GROUP_get_degree(EC_KEY_get0_group(ec_key));

        EC_KEY_free(ec_key);

        bn_len = (degree + 7) / 8;
        if ((bn_len * 2) != (unsigned int)slen)
            goto cel_jwt_verify_sha_pem_done;

        if ((BN_bin2bn(sig, bn_len, ec_sig->r) == NULL) 
            || (BN_bin2bn(sig + bn_len, bn_len, ec_sig->s) == NULL))
            goto cel_jwt_verify_sha_pem_done;


        slen = i2d_ECDSA_SIG(ec_sig, NULL);
        sig = (unsigned char *)cel_malloc(slen);
        if (sig == NULL)
            goto cel_jwt_verify_sha_pem_done;

        p = sig;
        slen = i2d_ECDSA_SIG(ec_sig, &p);

        if (slen == 0)
            goto cel_jwt_verify_sha_pem_done;
    }

    mdctx = EVP_MD_CTX_create();
    if (mdctx == NULL)
        goto cel_jwt_verify_sha_pem_done;

    /* Initialize the DigestVerify operation using alg */
    if (EVP_DigestVerifyInit(mdctx, NULL, alg, NULL, pkey) != 1)
        goto cel_jwt_verify_sha_pem_done;

    /* Call update with the message */
    if (EVP_DigestVerifyUpdate(mdctx, head, head_len) != 1)
        goto cel_jwt_verify_sha_pem_done;

    /* Now check the sig for validity. */
    if (EVP_DigestVerifyFinal(mdctx, sig, slen) != 1)
        goto cel_jwt_verify_sha_pem_done;

cel_jwt_verify_sha_pem_done:
    if (bufkey != NULL)
        BIO_free(bufkey);
    if (pkey != NULL)
        EVP_PKEY_free(pkey);
    if (mdctx != NULL)
        EVP_MD_CTX_destroy(mdctx);
    if (ec_sig != NULL)
        ECDSA_SIG_free(ec_sig);

    return ret;
}

static int cel_jwt_sign(CelJwt *jwt, BYTE *out, size_t *out_len, 
                        const char *str, size_t n,
                        const BYTE *key, int key_len)
{
    switch (jwt->alg) 
    {
        /* HMAC */
    case CEL_JWT_ALG_HS256:
        return cel_jwt_sign_sha_hmac(
            out, out_len, EVP_sha256(), str, n, key, key_len);
    case CEL_JWT_ALG_HS384:
        return cel_jwt_sign_sha_hmac(
            out, out_len, EVP_sha384(), str, n, key, key_len);
    case CEL_JWT_ALG_HS512:
        return cel_jwt_sign_sha_hmac(
            out, out_len, EVP_sha512(), str, n, key, key_len);
        /* RSA */
    case CEL_JWT_ALG_RS256:
        return cel_jwt_sign_sha_pem(
            out, out_len, EVP_sha256(), str, n, EVP_PKEY_RSA, key, key_len);
    case CEL_JWT_ALG_RS384:
        return cel_jwt_sign_sha_pem(
            out, out_len, EVP_sha384(), str, n, EVP_PKEY_RSA, key, key_len);
    case CEL_JWT_ALG_RS512:
        return cel_jwt_sign_sha_pem(
            out, out_len, EVP_sha512(), str, n, EVP_PKEY_RSA, key, key_len);
        /* ECC */
    case CEL_JWT_ALG_ES256:
        return cel_jwt_sign_sha_pem(
            out, out_len, EVP_sha256(), str, n, EVP_PKEY_EC, key, key_len);
    case CEL_JWT_ALG_ES384:
        return cel_jwt_sign_sha_pem(
            out, out_len, EVP_sha384(), str, n, EVP_PKEY_EC, key, key_len);
    case CEL_JWT_ALG_ES512:
        return cel_jwt_sign_sha_pem(
            out, out_len, EVP_sha512(), str, n, EVP_PKEY_EC, key, key_len);
        /* You wut, mate? */
    default:
        return EINVAL;
    }
}

static int cel_jwt_verify(CelJwt *jwt, 
                          const char *head, size_t head_len,
                          const char *sig, size_t sig_len,
                          const BYTE *key, int key_len)
{
    switch (jwt->alg) 
    {
        /* HMAC */
    case CEL_JWT_ALG_HS256:
        return cel_jwt_verify_sha_hmac( 
            EVP_sha256(), head, head_len, sig, sig_len, key, key_len);
    case CEL_JWT_ALG_HS384:
        return cel_jwt_verify_sha_hmac(
            EVP_sha384(), head, head_len, sig, sig_len, key, key_len);
    case CEL_JWT_ALG_HS512:
        return cel_jwt_verify_sha_hmac(
            EVP_sha512(), head, head_len, sig, sig_len, key, key_len);
        /* RSA */
    case CEL_JWT_ALG_RS256:
        return cel_jwt_verify_sha_pem(
            EVP_sha256(), EVP_PKEY_RSA, 
            head, head_len, sig, sig_len, key, key_len);
    case CEL_JWT_ALG_RS384:
        return cel_jwt_verify_sha_pem( 
            EVP_sha384(), EVP_PKEY_RSA,
            head, head_len, sig, sig_len, key, key_len);
    case CEL_JWT_ALG_RS512:
        return cel_jwt_verify_sha_pem( 
            EVP_sha512(), EVP_PKEY_RSA,
            head, head_len, sig, sig_len, key, key_len);
        /* ECC */
    case CEL_JWT_ALG_ES256:
        return cel_jwt_verify_sha_pem(
            EVP_sha256(), EVP_PKEY_EC,
            head, head_len, sig, sig_len, key, key_len);
    case CEL_JWT_ALG_ES384:
        return cel_jwt_verify_sha_pem(
            EVP_sha384(), EVP_PKEY_EC,
            head, head_len, sig, sig_len, key, key_len);
    case CEL_JWT_ALG_ES512:
        return cel_jwt_verify_sha_pem(
            EVP_sha512(), EVP_PKEY_EC,
            head, head_len, sig, sig_len, key, key_len);
    default:
        return EINVAL;
    }
}

static int cel_jwt_verify_head(CelJwt *jwt, const char *head, size_t head_len)
{
    CelJson *head_json;
    char val[6];
    int ret = 0;

    if ((head_json = cel_json_new()) == NULL
        || cel_jwt_base64url_decode_json(head_json, head, head_len) == -1)
    {
        puts("cel_jwt_base64url_decode_json return error");
        ret = -1;
        goto verify_head_done;
    }
    if (cel_json_object_get_string(head_json->root_node, "alg", val, 6) == -1
        || (jwt->alg = cel_keyword_binary_search_a(
        s_jwtalg, CEL_JWT_ALG_COUNT, val, strlen(val))) == -1)
    {
        puts("alg cmp error");
        ret = -1;
        goto verify_head_done;
    }
    /* If alg is not NONE, there should be a typ. */
    if (jwt->alg != CEL_JWT_ALG_NONE) 
    {
        if (cel_json_object_get_string(
            head_json->root_node, "typ", val, 6) == -1 
            || strcasecmp(val, "JWT") != 0)
        {
            ret = -1;
        };
    }
verify_head_done:
    if (head_json != NULL)
        cel_json_free(head_json);

    return ret;
}

int cel_jwt_decode(CelJwt *jwt, const char *token, size_t token_len,
                   const BYTE *key, int key_len)
{
    const char *head, *payload, *sig;
    int head_len, payload_len, sig_len;
    int ret = 0;

    head = token;
    head_len = 0;
    while (head[head_len] != '.')
    {
        if (head[head_len] == '\0')
            goto decode_done;
        head_len++;
    }
    payload = &head[head_len + 1];
    payload_len = 0;
    while (payload[payload_len] != '.')
    {
        if (payload[payload_len] == '\0')
            goto decode_done;
        payload_len++;
    }
    sig = &payload[payload_len + 1];
    if ((sig_len =  token_len - payload_len - head_len - 2) <= 0)
        goto decode_done;
    if ((ret = cel_jwt_verify_head(jwt, head, head_len)) == -1)
        goto decode_done;
    if ((ret = cel_jwt_base64url_decode_json(
        &(jwt->payload), payload, payload_len)) == -1)
        goto decode_done;
    /* Check the signature */
    if (jwt->alg != CEL_JWT_ALG_NONE)
    {
        /* Re-add this since it's part of the verified data. */
        ret = cel_jwt_verify(jwt, 
            token, head_len + payload_len + 1, sig, sig_len, key, key_len);
    } 
    else 
    {
        ret = 0;
    }
decode_done:
    if (ret)
        cel_jwt_destroy(jwt);

    return ret;
}

static void cel_jwt_write_head(CelJwt *jwt, 
                               char *out, size_t *out_len, int is_indent)
{
    if (is_indent)
        *out_len = snprintf(out, *out_len, 
        "{"CEL_CRLF
        "    \"typ\":\"JWT\","CEL_CRLF
        "    \"alg\":\"%s\""CEL_CRLF
        "}"CEL_CRLF, s_jwtalg[jwt->alg].key_word);
    else
        *out_len = snprintf(out, *out_len, 
        "{\"typ\":\"JWT\",\"alg\":\"%s\"}", s_jwtalg[jwt->alg].key_word);
}

static void cel_jwt_write_payload(CelJwt *jwt, 
                                  char *out, size_t *out_len, int is_indent)
{
    cel_json_serialize_starts(&(jwt->payload), is_indent);
    cel_json_serialize_update(&(jwt->payload), out, out_len);
    out[*out_len] = '\0';
    //puts(out);
    cel_json_serialize_finish(&(jwt->payload));
}

char *cel_jwt_encode(CelJwt *jwt, char *token, size_t *token_len, 
                     const BYTE *key, int key_len)
{
    char *buf;
    size_t _token_len = *token_len;
    size_t _len1, _len2;

    buf = cel_malloc(_token_len);

    _len1 = _token_len;
    cel_jwt_write_head(jwt, buf, &_len1, 0);
    _len2 = *token_len;
    cel_base64url_encode((BYTE *)token, &_len2, (BYTE *)buf, _len1);
    *token_len = _len2;
    token[*token_len] = '.';
    (*token_len)++;

    _len1 = _token_len;
    cel_jwt_write_payload(jwt, buf, &_len1, 0);
    _len2 = _token_len - *token_len;
    cel_base64url_encode((BYTE *)token + *token_len,
        &_len2, (BYTE *)buf, _len1);
    (*token_len) += _len2;
    token[*token_len] = '.';
    (*token_len)++;

    _len1 = _token_len;
    cel_jwt_sign(jwt, (BYTE *)buf, &_len1,
        token, *token_len - 1, key, key_len);
    _len2 = _token_len - *token_len;
    cel_base64url_encode((BYTE *)token + *token_len, &_len2,
        (BYTE *)buf, _len1);
    (*token_len) += _len2;
    token[*token_len] = '\0';

    cel_free(buf);

    return token;
}
