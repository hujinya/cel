/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_CRYPTO_RSA_H__
#define __CEL_CRYPTO_RSA_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef x
/*
 * RSA Error codes
 */
#define POLARSSL_ERR_RSA_BAD_INPUT_DATA                    -0x4080  /**< Bad input parameters to function. */
#define POLARSSL_ERR_RSA_INVALID_PADDING                   -0x4100  /**< Input data contains invalid padding and is rejected. */
#define POLARSSL_ERR_RSA_KEY_GEN_FAILED                    -0x4180  /**< Something failed during generation of a key. */
#define POLARSSL_ERR_RSA_KEY_CHECK_FAILED                  -0x4200  /**< Key failed to pass the libraries validity check. */
#define POLARSSL_ERR_RSA_PUBLIC_FAILED                     -0x4280  /**< The public key operation failed. */
#define POLARSSL_ERR_RSA_PRIVATE_FAILED                    -0x4300  /**< The private key operation failed. */
#define POLARSSL_ERR_RSA_VERIFY_FAILED                     -0x4380  /**< The PKCS#1 verification failed. */
#define POLARSSL_ERR_RSA_OUTPUT_TOO_LARGE                  -0x4400  /**< The output buffer for decryption is not large enough. */
#define POLARSSL_ERR_RSA_RNG_FAILED                        -0x4480  /**< The random generator failed to generate non-zeros. */

/*
 * PKCS#1 constants
 */
#define SIG_RSA_RAW     0
#define SIG_RSA_MD2     2
#define SIG_RSA_MD4     3
#define SIG_RSA_MD5     4
#define SIG_RSA_SHA1    5
#define SIG_RSA_SHA224 14
#define SIG_RSA_SHA256 11
#define SIG_RSA_SHA384 12
#define SIG_RSA_SHA512 13

#define RSA_PUBLIC      0
#define RSA_PRIVATE     1

#define RSA_PKCS_V15    0
#define RSA_PKCS_V21    1

#define RSA_SIGN        1
#define RSA_CRYPT       2

#define ASN1_STR_CONSTRUCTED_SEQUENCE   "\x30"
#define ASN1_STR_NULL                   "\x05"
#define ASN1_STR_OID                    "\x06"
#define ASN1_STR_OCTET_STRING           "\x04"

#define OID_DIGEST_ALG_MDX              "\x2A\x86\x48\x86\xF7\x0D\x02\x00"
#define OID_HASH_ALG_SHA1               "\x2b\x0e\x03\x02\x1a"
#define OID_HASH_ALG_SHA2X              "\x60\x86\x48\x01\x65\x03\x04\x02\x00"

#define OID_ISO_MEMBER_BODIES           "\x2a"
#define OID_ISO_IDENTIFIED_ORG          "\x2b"

/*
 * ISO Member bodies OID parts
 */
#define OID_COUNTRY_US                  "\x86\x48"
#define OID_RSA_DATA_SECURITY           "\x86\xf7\x0d"

/*
 * ISO Identified organization OID parts
 */
#define OID_OIW_SECSIG_SHA1             "\x0e\x03\x02\x1a"

/*
 * DigestInfo ::= SEQUENCE {
 *   digestAlgorithm DigestAlgorithmIdentifier,
 *   digest Digest }
 *
 * DigestAlgorithmIdentifier ::= AlgorithmIdentifier
 *
 * Digest ::= OCTET STRING
 */
#define ASN1_HASH_MDX                           \
(                                               \
    ASN1_STR_CONSTRUCTED_SEQUENCE "\x20"        \
      ASN1_STR_CONSTRUCTED_SEQUENCE "\x0C"      \
        ASN1_STR_OID "\x08"                     \
      OID_DIGEST_ALG_MDX                        \
    ASN1_STR_NULL "\x00"                        \
      ASN1_STR_OCTET_STRING "\x10"              \
)

#define ASN1_HASH_SHA1                          \
    ASN1_STR_CONSTRUCTED_SEQUENCE "\x21"        \
      ASN1_STR_CONSTRUCTED_SEQUENCE "\x09"      \
        ASN1_STR_OID "\x05"                     \
      OID_HASH_ALG_SHA1                         \
        ASN1_STR_NULL "\x00"                    \
      ASN1_STR_OCTET_STRING "\x14"

#define ASN1_HASH_SHA1_ALT                      \
    ASN1_STR_CONSTRUCTED_SEQUENCE "\x1F"        \
      ASN1_STR_CONSTRUCTED_SEQUENCE "\x07"      \
        ASN1_STR_OID "\x05"                     \
      OID_HASH_ALG_SHA1                         \
      ASN1_STR_OCTET_STRING "\x14"

#define ASN1_HASH_SHA2X                         \
    ASN1_STR_CONSTRUCTED_SEQUENCE "\x11"        \
      ASN1_STR_CONSTRUCTED_SEQUENCE "\x0d"      \
        ASN1_STR_OID "\x09"                     \
      OID_HASH_ALG_SHA2X                        \
        ASN1_STR_NULL "\x00"                    \
      ASN1_STR_OCTET_STRING "\x00"

/**
 * \brief          RSA context structure
 */
typedef struct _CelRsaContext
{
    int ver;                    /**< Always 0          */
    size_t len;                 /**< Size(N)in chars   */

    mpi N;                      /**< Public modulus    */
    mpi E;                      /**< Public exponent   */

    mpi D;                      /**< Private exponent  */
    mpi P;                      /**< 1st prime factor  */
    mpi Q;                      /**< 2nd prime factor  */
    mpi DP;                     /**< D % (P - 1)       */
    mpi DQ;                     /**< D % (Q - 1)       */
    mpi QP;                     /**< 1 / (Q % P)       */

    mpi RN;                     /**< Cached R^2 mod N  */
    mpi RP;                     /**< Cached R^2 mod P  */
    mpi RQ;                     /**< Ccached R^2 mod Q */

    int padding;                /**< RSA_PKCS_V15 for 1.5 padding and RSA_PKCS_v21 for OAEP/PSS */
    int hash_id;                /**< Hash identifier of md_type_t as
                                   specified in the md.h header file
                                   for the EME-OAEP and EMSA-PSS encoding */
}CelRsaContext;

/**
 * \brief          Initialize an RSA context
 *
 * \param ctx      RSA context to be initialized
 * \param padding  RSA_PKCS_V15 or RSA_PKCS_V21
 * \param hash_id  RSA_PKCS_V21 hash identifier
 *
 * \note           The hash_id parameter is actually ignored
 *                 when using RSA_PKCS_V15 padding.
 */
void cel_rsa_init(CelRsaContext *ctx, int padding, int hash_id);

/**
 * \brief          Generate an RSA keypair
 *
 * \param ctx      RSA context that will hold the key
 * \param f_rng    RNG function
 * \param p_rng    RNG parameter
 * \param nbits    size of the public key in bits
 * \param exponent public exponent (e.g., 65537)
 *
 * \note           cel_rsa_init()must be called beforehand to setup
 *                 the RSA context.
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 */
int cel_rsa_gen_key(CelRsaContext *ctx,
                    int (*f_rng) (void *, BYTE *, size_t),
                    void *p_rng,
                    unsigned int nbits, int exponent);

/**
 * \brief          Check a public RSA key
 *
 * \param ctx      RSA context to be checked
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 */
int cel_rsa_check_pubkey(const CelRsaContext *ctx);

/**
 * \brief          Check a private RSA key
 *
 * \param ctx      RSA context to be checked
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 */
int cel_rsa_check_privkey(const CelRsaContext *ctx);

/**
 * \brief          Do an RSA public key operation
 *
 * \param ctx      RSA context
 * \param input    input buffer
 * \param output   output buffer
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           This function does NOT take care of message
 *                 padding. Also, be sure to set input[0] = 0 or assure that
 *                 input is smaller than N.
 *
 * \note           The input and output buffers must be large
 *                 enough (eg. 128 bytes if RSA-1024 is used).
 */
int cel_rsa_public(CelRsaContext *ctx,
                   const BYTE *input,
                   BYTE *output);

/**
 * \brief          Do an RSA private key operation
 *
 * \param ctx      RSA context
 * \param input    input buffer
 * \param output   output buffer
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The input and output buffers must be large
 *                 enough (eg. 128 bytes if RSA-1024 is used).
 */
int cel_rsa_private(CelRsaContext *ctx, 
                    const BYTE *input, BYTE *output);

/**
 * \brief          Add the message padding, then do an RSA operation
 *
 * \param ctx      RSA context
 * \param f_rng    RNG function (Needed for padding and PKCS#1 v2.1 encoding)
 * \param p_rng    RNG parameter
 * \param mode     RSA_PUBLIC or RSA_PRIVATE
 * \param ilen     contains the plaintext length
 * \param input    buffer holding the data to be encrypted
 * \param output   buffer that will hold the ciphertext
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The output buffer must be as large as the size
 *                 of ctx->N (eg. 128 bytes if RSA-1024 is used).
 */
int cel_rsa_pkcs1_encrypt(CelRsaContext *ctx,
                          int (*f_rng) (void *, BYTE *, size_t),
                          void *p_rng,
                          int mode, size_t ilen,
                          const BYTE *input,
                          BYTE *output);

/**
 * \brief          Do an RSA operation, then remove the message padding
 *
 * \param ctx      RSA context
 * \param mode     RSA_PUBLIC or RSA_PRIVATE
 * \param olen     will contain the plaintext length
 * \param input    buffer holding the encrypted data
 * \param output   buffer that will hold the plaintext
 * \param output_max_len    maximum length of the output buffer
 *
 * \return         0 if successful, or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The output buffer must be as large as the size
 *                 of ctx->N (eg. 128 bytes if RSA-1024 is used)otherwise
 *                 an error is thrown.
 */
int cel_rsa_pkcs1_decrypt(CelRsaContext *ctx,
                          int mode, size_t *olen,
                          const BYTE *input,
                          BYTE *output,
                          size_t output_max_len);

/**
 * \brief          Do a private RSA to sign a message digest
 *
 * \param ctx      RSA context
 * \param f_rng    RNG function (Needed for PKCS#1 v2.1 encoding)
 * \param p_rng    RNG parameter
 * \param mode     RSA_PUBLIC or RSA_PRIVATE
 * \param hash_id  SIG_RSA_RAW, SIG_RSA_MD{2,4,5} or SIG_RSA_SHA{1,224,256,384,512}
 * \param hashlen  message digest length (for SIG_RSA_RAW only)
 * \param hash     buffer holding the message digest
 * \param sig      buffer that will hold the ciphertext
 *
 * \return         0 if the signing operation was successful,
 *                 or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The "sig" buffer must be as large as the size
 *                 of ctx->N (eg. 128 bytes if RSA-1024 is used).
 *
 * \note           In case of PKCS#1 v2.1 encoding keep in mind that
 *                 the hash_id in the RSA context is the one used for the
 *                 encoding. hash_id in the function call is the type of hash
 *                 that is encoded. According to RFC 3447 it is advised to
 *                 keep both hashes the same.
 */
int cel_rsa_pkcs1_sign(CelRsaContext *ctx,
                       int (*f_rng) (void *, BYTE *, size_t),
                       void *p_rng,
                       int mode,
                       int hash_id,
                       unsigned int hashlen,
                       const BYTE *hash,
                       BYTE *sig);

/**
 * \brief          Do a public RSA and check the message digest
 *
 * \param ctx      points to an RSA public key
 * \param mode     RSA_PUBLIC or RSA_PRIVATE
 * \param hash_id  SIG_RSA_RAW, SIG_RSA_MD{2,4,5} or SIG_RSA_SHA{1,224,256,384,512}
 * \param hashlen  message digest length (for SIG_RSA_RAW only)
 * \param hash     buffer holding the message digest
 * \param sig      buffer holding the ciphertext
 *
 * \return         0 if the verify operation was successful,
 *                 or an POLARSSL_ERR_RSA_XXX error code
 *
 * \note           The "sig" buffer must be as large as the size
 *                 of ctx->N (eg. 128 bytes if RSA-1024 is used).
 *
 * \note           In case of PKCS#1 v2.1 encoding keep in mind that
 *                 the hash_id in the RSA context is the one used for the
 *                 verification. hash_id in the function call is the type of hash
 *                 that is verified. According to RFC 3447 it is advised to
 *                 keep both hashes the same.
 */
int cel_rsa_pkcs1_verify(CelRsaContext *ctx,
                         int mode,
                         int hash_id,
                         unsigned int hashlen,
                         const BYTE *hash,
                         BYTE *sig);

/**
 * \brief          Free the components of an RSA key
 *
 * \param ctx      RSA Context to free
 */
void cel_rsa_free(CelRsaContext *ctx);

#endif

#ifdef __cplusplus
}
#endif

#endif
