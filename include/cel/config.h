#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef _WIN32
#undef _CEL_UNIX
#define _CEL_WIN
#else
#undef _CEL_WIN
#define _CEL_UNIX
#endif

//#define _CEL_BIGEDIAN
#define _CEL_MULTITHREAD

#define HAVE_LIBZ             0

#define HAVE_OPENSSL_CRYPTO_H 1
#define HAVE_OPENSSL_SSL_H    1



#endif
