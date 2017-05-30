// Copyright (c) 2009-2017 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Developers
// Copyright (c) 2014-2017 The Dash Core Developers
// Copyright (c) 2017 Credits Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CREDITS_CREDITSCONSENSUS_H
#define CREDITS_CREDITSCONSENSUS_H

#if defined(BUILD_CREDITS_INTERNAL) && defined(HAVE_CONFIG_H)
#include "config/credits-config.h"
  #if defined(_WIN32)
    #if defined(DLL_EXPORT)
      #if defined(HAVE_FUNC_ATTRIBUTE_DLLEXPORT)
        #define EXPORT_SYMBOL __declspec(dllexport)
      #else
        #define EXPORT_SYMBOL
      #endif
    #endif
  #elif defined(HAVE_FUNC_ATTRIBUTE_VISIBILITY)
    #define EXPORT_SYMBOL __attribute__ ((visibility ("default")))
  #endif
#elif defined(MSC_VER) && !defined(STATIC_LIBCREDITSCONSENSUS)
  #define EXPORT_SYMBOL __declspec(dllimport)
#endif

#ifndef EXPORT_SYMBOL
  #define EXPORT_SYMBOL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define CREDITSCONSENSUS_API_VER 0

typedef enum creditsconsensus_error_t
{
    creditsconsensus_ERR_OK = 0,
    creditsconsensus_ERR_TX_INDEX,
    creditsconsensus_ERR_TX_SIZE_MISMATCH,
    creditsconsensus_ERR_TX_DESERIALIZE,
    creditsconsensus_ERR_INVALID_FLAGS,
} creditsconsensus_error;

/** Script verification flags */
enum
{
    creditsconsensus_SCRIPT_FLAGS_VERIFY_NONE                = 0,
    creditsconsensus_SCRIPT_FLAGS_VERIFY_P2SH                = (1U << 0), // evaluate P2SH (BIP16) subscripts
    creditsconsensus_SCRIPT_FLAGS_VERIFY_DERSIG              = (1U << 2), // enforce strict DER (BIP66) compliance
    creditsconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY = (1U << 9), // enable CHECKLOCKTIMEVERIFY (BIP65)
    creditsconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY = (1U << 10), // enable CHECKSEQUENCEVERIFY (BIP112)
    creditsconsensus_SCRIPT_FLAGS_VERIFY_ALL                 = creditsconsensus_SCRIPT_FLAGS_VERIFY_P2SH | creditsconsensus_SCRIPT_FLAGS_VERIFY_DERSIG |
                                                               creditsconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY | creditsconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY
};

/// Returns 1 if the input nIn of the serialized transaction pointed to by
/// txTo correctly spends the scriptPubKey pointed to by scriptPubKey under
/// the additional constraints specified by flags.
/// If not NULL, err will contain an error/success code for the operation
EXPORT_SYMBOL int creditsconsensus_verify_script(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen,
                                    const unsigned char *txTo        , unsigned int txToLen,
                                    unsigned int nIn, unsigned int flags, creditsconsensus_error* err);

EXPORT_SYMBOL unsigned int creditsconsensus_version();

#ifdef __cplusplus
} // extern "C"
#endif

#undef EXPORT_SYMBOL

#endif // CREDITS_CREDITSCONSENSUS_H
