/*
 * Generated by util/mkerr.pl DO NOT EDIT
 * Copyright 1995-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OPENSSL_DSAERR_H
# define OPENSSL_DSAERR_H
# pragma once

# include <openssl/opensslconf.h>
# include <openssl/symhacks.h>
# include <openssl/cryptoerr_legacy.h>


# ifndef OPENSSL_NO_DSA


/*
 * DSA function codes.
 */
#  ifndef OPENSSL_NO_DEPRECATED_3_0
#   define DSA_F_DSAPARAMS_PRINT                            0
#   define DSA_F_DSAPARAMS_PRINT_FP                         0
#   define DSA_F_DSA_BUILTIN_PARAMGEN                       0
#   define DSA_F_DSA_BUILTIN_PARAMGEN2                      0
#   define DSA_F_DSA_DO_SIGN                                0
#   define DSA_F_DSA_DO_VERIFY                              0
#   define DSA_F_DSA_METH_DUP                               0
#   define DSA_F_DSA_METH_NEW                               0
#   define DSA_F_DSA_METH_SET1_NAME                         0
#   define DSA_F_DSA_NEW_METHOD                             0
#   define DSA_F_DSA_PARAM_DECODE                           0
#   define DSA_F_DSA_PRINT_FP                               0
#   define DSA_F_DSA_PRIV_DECODE                            0
#   define DSA_F_DSA_PRIV_ENCODE                            0
#   define DSA_F_DSA_PUB_DECODE                             0
#   define DSA_F_DSA_PUB_ENCODE                             0
#   define DSA_F_DSA_SIGN                                   0
#   define DSA_F_DSA_SIGN_SETUP                             0
#   define DSA_F_DSA_SIG_NEW                                0
#   define DSA_F_OLD_DSA_PRIV_DECODE                        0
#   define DSA_F_PKEY_DSA_CTRL                              0
#   define DSA_F_PKEY_DSA_CTRL_STR                          0
#   define DSA_F_PKEY_DSA_KEYGEN                            0
#  endif

/*
 * DSA reason codes.
 */
#  define DSA_R_BAD_FFC_PARAMETERS                         114
#  define DSA_R_BAD_Q_VALUE                                102
#  define DSA_R_BN_DECODE_ERROR                            108
#  define DSA_R_BN_ERROR                                   109
#  define DSA_R_DECODE_ERROR                               104
#  define DSA_R_INVALID_DIGEST_TYPE                        106
#  define DSA_R_INVALID_PARAMETERS                         112
#  define DSA_R_MISSING_PARAMETERS                         101
#  define DSA_R_MISSING_PRIVATE_KEY                        111
#  define DSA_R_MODULUS_TOO_LARGE                          103
#  define DSA_R_NO_PARAMETERS_SET                          107
#  define DSA_R_PARAMETER_ENCODING_ERROR                   105
#  define DSA_R_Q_NOT_PRIME                                113
#  define DSA_R_SEED_LEN_SMALL                             110

# endif
#endif