// Copyright (c) 2009-2017 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Developers
// Copyright (c) 2014-2017 The Dash Core Developers
// Copyright (c) 2017 Credits Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CREDITS_CORE_MEMUSAGE_H
#define CREDITS_CORE_MEMUSAGE_H

#include "memusage.h"

#include "primitives/block.h"
#include "primitives/transaction.h"

static inline size_t RecursiveCreditsUsage(const CScript& script) {
    return memusage::CreditsUsage(*static_cast<const CScriptBase*>(&script));
}

static inline size_t RecursiveCreditsUsage(const COutPoint& out) {
    return 0;
}

static inline size_t RecursiveCreditsUsage(const CTxIn& in) {
    return RecursiveCreditsUsage(in.scriptSig) + RecursiveCreditsUsage(in.prevout);
}

static inline size_t RecursiveCreditsUsage(const CTxOut& out) {
    return RecursiveCreditsUsage(out.scriptPubKey);
}

static inline size_t RecursiveCreditsUsage(const CTransaction& tx) {
    size_t mem = memusage::CreditsUsage(tx.vin) + memusage::CreditsUsage(tx.vout);
    for (std::vector<CTxIn>::const_iterator it = tx.vin.begin(); it != tx.vin.end(); it++) {
        mem += RecursiveCreditsUsage(*it);
    }
    for (std::vector<CTxOut>::const_iterator it = tx.vout.begin(); it != tx.vout.end(); it++) {
        mem += RecursiveCreditsUsage(*it);
    }
    return mem;
}

static inline size_t RecursiveCreditsUsage(const CMutableTransaction& tx) {
    size_t mem = memusage::CreditsUsage(tx.vin) + memusage::CreditsUsage(tx.vout);
    for (std::vector<CTxIn>::const_iterator it = tx.vin.begin(); it != tx.vin.end(); it++) {
        mem += RecursiveCreditsUsage(*it);
    }
    for (std::vector<CTxOut>::const_iterator it = tx.vout.begin(); it != tx.vout.end(); it++) {
        mem += RecursiveCreditsUsage(*it);
    }
    return mem;
}

static inline size_t RecursiveCreditsUsage(const CBlock& block) {
    size_t mem = memusage::CreditsUsage(block.vtx);
    for (std::vector<CTransaction>::const_iterator it = block.vtx.begin(); it != block.vtx.end(); it++) {
        mem += RecursiveCreditsUsage(*it);
    }
    return mem;
}

static inline size_t RecursiveCreditsUsage(const CBlockLocator& locator) {
    return memusage::CreditsUsage(locator.vHave);
}

#endif // CREDITS_CORE_MEMUSAGE_H
