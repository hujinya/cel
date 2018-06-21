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
#include "cel/sys/license.h"
#include "cel/crypto/base64.h"
#include "cel/crypto/crypto.h"

//static int license_verify(const char *from, 
//                          const char *license, const char *private_key)
//{
//    return -1;
//}

//int license_decrypt(const char *plicense, long *expired_hours)
//{
//    TCHAR *buf;
//    BIO *bio;
//    RSA *rsa;
//    
//    OpenSSL_add_all_algorithms();
//    if ((bio = BIO_new(BIO_s_file())) == NULL) 
//        return -1;
//    BIO_read_filename(bio, PRIVATE_KEY_PATH);
//    rsa = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
//    BIO_free_all(bio);
//    if (rsa == NULL)
//        return -1;
//    return 0;
//}
//
//
//static int license_decrypt(const char *plicense, long *expired_hours)
//{
//    BIO *bio;
//    RSA *rsa;
//
//    static char key[LICENSE_LEN + 1];
//    static char license[LICENSE_LEN + 1];
//    int pllen, llen, pldlen = 0, klen = 0, blen;
//
//    /*
//    SSL_load_error_strings();
//    ERR_load_BIO_strings();
//    */
//    /* Get rsa private key */
//    OpenSSL_add_all_algorithms();
//    if ((bio = BIO_new(BIO_s_file())) == NULL) 
//        return -1;
//
//    BIO_read_filename(bio, PRIVATE_KEY_PATH);
//    rsa = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
//    BIO_free_all(bio);
//    if (rsa == NULL)
//        return -1;
//
//    /* Base64 decode */
//    pllen = strlen(plicense);
//    if (pllen > LICENSE_LEN) pllen = LICENSE_LEN;
//    llen = pllen;
//
//    while (plicense[--pllen] == '=') pldlen++;
//    if ((llen = (EVP_DecodeBlock((BYTE *)license,
//        (const BYTE *)plicense, llen) - pldlen)) <= 0)
//    {
//        RSA_free(rsa);
//        return -1;
//    }
//    /* Decrypts data with rsa private key */
//    OpenSSL_add_all_ciphers();
//    blen = RSA_size(rsa) - RSA_PADDING_LEN;
//    pllen = 0;
//    while (pllen < llen && klen < LICENSE_LEN)
//    {
//        if (blen > llen - pllen) 
//            blen = llen - pllen;
//        if ((klen += RSA_private_decrypt(blen, (const BYTE *)license + pllen, 
//            (BYTE *)key + klen, rsa, RSA_PADDING)) <= 0)
//        {
//            Err(("License decrypt failed(%s).", 
//                cel_ssl_get_errstr(cel_ssl_get_errno())));
//            return -1;
//        }
//        pllen += blen;
//    }
//    key[klen] = '\0';
//    RSA_free(rsa);
//
//    int i = 0;
//    while (i < klen)
//    {
//        if (strncmp(&key[i], "\"regExpired\":", 13) == 0)
//        {
//            i += 13;
//            *expired_hours = atol(&key[i]) * 30 * 24;
//            //_putts(key);
//            Notice(("License verify successed, %ld.", *expired_hours));
//            return 0;
//        }
//        i++;
//    }
//    return -1;
//}
//
//static int license_compare(const char *plicense, const char *buffer)
//{
//    int i = 0, j;
//
//    while (buffer[i] != ':' && buffer[i] != '\0') i++;
//    i++; j = 0;
//    while (buffer[i] != '\r' && buffer[i] != '\n' &&  buffer[i] != '\0')
//    {
//        if (buffer[i++] != plicense[j++]) 
//            return -1;
//    }
//    return 0;
//}
//
//long license_expired(const char *plicense, long *expired_hours)
//{
//    static char buffer[LICENSE_LEN];
//    FILE *file, *file_;
//    int reg = 0, size;
//
//    *expired_hours = -1;
//    if (plicense == NULL) 
//        return -1;
//
//    if ((file_ = cel_fopen(KEY_INSTAAL_PATH_, "a+")) != NULL)
//    {
//        if ((file = cel_fopen(KEY_INSTAAL_PATH, "w")) != NULL)
//        {
//            while(fgets(buffer, LICENSE_LEN, file_) != NULL)
//            {
//                if (reg == 0 && license_compare(plicense, buffer) == 0)
//                {
//                    reg = 1;
//                    if ((*expired_hours = strtol(buffer, NULL, 10)) != 0)
//                    {
//                        if ((*expired_hours -= 1) == 0)
//                            *expired_hours = -1;
//                    }
//                    size = snprintf(
//                        buffer, LICENSE_LEN, "%ld:%s", *expired_hours, plicense);
//                    if (buffer[size - 1] != '\n')
//                        buffer[size++] = '\n';
//                    buffer[size] = '\0';
//                }
//                fputs(buffer, file);
//                buffer[0] = '\0';
//            }
//            cel_fclose(file);
//        }
//        cel_fclose(file_);
//    }
//    if (reg == 0 && license_decrypt(plicense, expired_hours) == 0)
//    {
//        if ((file_ = fopen(KEY_INSTAAL_PATH_, "a+")) != NULL)
//        {
//            if ((file = fopen(KEY_INSTAAL_PATH, "w")) != NULL)
//            {
//                reg = 1;
//                size = snprintf(
//                    buffer, LICENSE_LEN, "%ld:%s", *expired_hours, plicense);
//                if (buffer[size - 1] != '\n')
//                    buffer[size++] = '\n';
//                buffer[size] = '\0';
//                fputs(buffer, file);
//                buffer[0] = '\0';
//                while(fgets(buffer, LICENSE_LEN, file_) != NULL)
//                {
//                    fputs(buffer, file);
//                    buffer[0] = '\0';
//                }
//                fclose(file);
//            }
//            fclose(file_);
//        }
//    }
//    if (reg == 1)
//    {
//        cel_fsync(KEY_INSTAAL_PATH_, KEY_INSTAAL_PATH);
//        return 0;
//    }
//    //Warning((_T("Licenes expired.")));
//    return -1;
//}
