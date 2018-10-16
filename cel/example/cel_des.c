#include "cel/crypto/des.h"

/*
 * DES and 3DES test vectors from:
 *
 * http://csrc.nist.gov/groups/STM/cavp/documents/des/tripledes-vectors.zip
 */
static const unsigned char des3_test_keys[24] =
{
    0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
    0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01,
    0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01, 0x23
};

static const unsigned char des3_test_iv[8] =
{
    0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF,
};

static const unsigned char des3_test_buf[8] =
{
    0x4E, 0x6F, 0x77, 0x20, 0x69, 0x73, 0x20, 0x74
};

static const unsigned char des3_test_ecb_dec[3][8] =
{
    { 0xCD, 0xD6, 0x4F, 0x2F, 0x94, 0x27, 0xC1, 0x5D },
    { 0x69, 0x96, 0xC8, 0xFA, 0x47, 0xA2, 0xAB, 0xEB },
    { 0x83, 0x25, 0x39, 0x76, 0x44, 0x09, 0x1A, 0x0A }
};

static const unsigned char des3_test_ecb_enc[3][8] =
{
    { 0x6A, 0x2A, 0x19, 0xF4, 0x1E, 0xCA, 0x85, 0x4B },
    { 0x03, 0xE6, 0x9F, 0x5B, 0xFA, 0x58, 0xEB, 0x42 },
    { 0xDD, 0x17, 0xE8, 0xB8, 0xB4, 0x37, 0xD2, 0x32 }
};

static const unsigned char des3_test_cbc_dec[3][8] =
{
    { 0x12, 0x9F, 0x40, 0xB9, 0xD2, 0x00, 0x56, 0xB3 },
    { 0x47, 0x0E, 0xFC, 0x9A, 0x6B, 0x8E, 0xE3, 0x93 },
    { 0xC5, 0xCE, 0xCF, 0x63, 0xEC, 0xEC, 0x51, 0x4C }
};

static const unsigned char des3_test_cbc_enc[3][8] =
{
    { 0x54, 0xF1, 0x5A, 0xF6, 0xEB, 0xE3, 0xA4, 0xB4 },
    { 0x35, 0x76, 0x11, 0x56, 0x5F, 0xA1, 0x8E, 0x4D },
    { 0xCB, 0x19, 0x1F, 0x85, 0xD1, 0xED, 0x84, 0x39 }
};

/*
 * Checkup routine
 */
int des_test(int argc, TCHAR *argv[])
{
    int verbose = 1;

    int i, j, u, v;
    CelDesContext ctx;
    CelDes3Context ctx3;
    unsigned char key[24];
    unsigned char buf[8];
    unsigned char prv[8];
    unsigned char iv[8];

    memset( key, 0, 24 );

    /*
     * ECB mode
     */
    for( i = 0; i < 6; i++ )
    {
        u = i >> 1;
        v = i  & 1;

        if( verbose != 0 )
            printf( "  DES%c-ECB-%3d (%s): ",
                    ( u == 0 ) ? ' ' : '3', 56 + u * 56,
                    ( v == CEL_DES_DECRYPT ) ? "dec" : "enc" );

        memcpy( buf, des3_test_buf, 8 );

        switch( i )
        {
        case 0:
            cel_des_setkey_dec( &ctx, (unsigned char *) des3_test_keys );
            break;

        case 1:
            cel_des_setkey_enc( &ctx, (unsigned char *) des3_test_keys );
            break;

        case 2:
            cel_des3_set2key_dec( &ctx3, (unsigned char *) des3_test_keys );
            break;

        case 3:
            cel_des3_set2key_enc( &ctx3, (unsigned char *) des3_test_keys );
            break;

        case 4:
            cel_des3_set3key_dec( &ctx3, (unsigned char *) des3_test_keys );
            break;

        case 5:
            cel_des3_set3key_enc( &ctx3, (unsigned char *) des3_test_keys );
            break;

        default:
            return( 1 );
        }

        for( j = 0; j < 10000; j++ )
        {
            if( u == 0 )
                cel_des_crypt_ecb( &ctx, buf, buf );
            else
                cel_des3_crypt_ecb( &ctx3, buf, buf );
        }

        if( ( v == CEL_DES_DECRYPT &&
                memcmp( buf, des3_test_ecb_dec[u], 8 ) != 0 ) ||
            ( v != CEL_DES_DECRYPT &&
                memcmp( buf, des3_test_ecb_enc[u], 8 ) != 0 ) )
        {
            if( verbose != 0 )
                printf( "failed\n" );

            return( 1 );
        }

        if( verbose != 0 )
            printf( "passed\n" );
    }

    if( verbose != 0 )
        printf( "\n" );

    /*
     * CBC mode
     */
    for( i = 0; i < 6; i++ )
    {
        u = i >> 1;
        v = i  & 1;

        if( verbose != 0 )
            printf( "  DES%c-CBC-%3d (%s): ",
                    ( u == 0 ) ? ' ' : '3', 56 + u * 56,
                    ( v == CEL_DES_DECRYPT ) ? "dec" : "enc" );

        memcpy( iv,  des3_test_iv,  8 );
        memcpy( prv, des3_test_iv,  8 );
        memcpy( buf, des3_test_buf, 8 );

        switch( i )
        {
        case 0:
            cel_des_setkey_dec( &ctx, (unsigned char *) des3_test_keys );
            break;

        case 1:
            cel_des_setkey_enc( &ctx, (unsigned char *) des3_test_keys );
            break;

        case 2:
            cel_des3_set2key_dec( &ctx3, (unsigned char *) des3_test_keys );
            break;

        case 3:
            cel_des3_set2key_enc( &ctx3, (unsigned char *) des3_test_keys );
            break;

        case 4:
            cel_des3_set3key_dec( &ctx3, (unsigned char *) des3_test_keys );
            break;

        case 5:
            cel_des3_set3key_enc( &ctx3, (unsigned char *) des3_test_keys );
            break;

        default:
            return( 1 );
        }

        if( v == CEL_DES_DECRYPT )
        {
            for( j = 0; j < 10000; j++ )
            {
                if( u == 0 )
                    cel_des_crypt_cbc( &ctx, v, 8, iv, buf, buf );
                else
                    cel_des3_crypt_cbc( &ctx3, v, 8, iv, buf, buf );
            }
        }
        else
        {
            for( j = 0; j < 10000; j++ )
            {
                unsigned char tmp[8];

                if( u == 0 )
                    cel_des_crypt_cbc( &ctx, v, 8, iv, buf, buf );
                else
                    cel_des3_crypt_cbc( &ctx3, v, 8, iv, buf, buf );

                memcpy( tmp, prv, 8 );
                memcpy( prv, buf, 8 );
                memcpy( buf, tmp, 8 );
            }

            memcpy( buf, prv, 8 );
        }

        if( ( v == CEL_DES_DECRYPT &&
                memcmp( buf, des3_test_cbc_dec[u], 8 ) != 0 ) ||
            ( v != CEL_DES_DECRYPT &&
                memcmp( buf, des3_test_cbc_enc[u], 8 ) != 0 ) )
        {
            if( verbose != 0 )
                printf( "failed\n" );

            return( 1 );
        }

        if( verbose != 0 )
            printf( "passed\n" );
    }

    if( verbose != 0 )
        printf( "\n" );

    return( 0 );
}
