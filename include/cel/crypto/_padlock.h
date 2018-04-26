/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 */
#ifndef __CEL_CRYPTO_PADLOCK_H__
#define __CEL_CRYPTO_PADLOCK_H__

#include "cel/types.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define __CEL_PADLOCK__

#ifdef __CEL_PADLOCK__

#define PADLOCK_RNG 0x000C
#define PADLOCK_ACE 0x00C0
#define PADLOCK_PHE 0x0C00
#define PADLOCK_PMM 0x3000

#define PADLOCK_ALIGN16(x) (unsigned long *) (16 + ((long) x & ~15))


/**
 * \brief          PadLock detection routine
 *
 * \param          The feature to detect
 *
 * \return         1 if CPU has support for the feature, 0 otherwise
 */
int cel_padlock_supports(int feature);

#endif

#ifdef __cplusplus
}
#endif

#endif /* padlock.h */
