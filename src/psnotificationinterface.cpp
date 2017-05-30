// Copyright (c) 2014-2017 The Dash Core Developers
// Copyright (c) 2017 Credits Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "psnotificationinterface.h"

#include "masternodeman.h"
#include "masternode-payments.h"
#include "masternode-sync.h"
#include "governance.h"
#include "instantsend.h"
#include "privatesend.h"

CPSNotificationInterface::CPSNotificationInterface()
{
}

CPSNotificationInterface::~CPSNotificationInterface()
{
}

void CPSNotificationInterface::UpdatedBlockTip(const CBlockIndex *pindex)
{
    mnodeman.UpdatedBlockTip(pindex);
    privateSendPool.UpdatedBlockTip(pindex);
    instantsend.UpdatedBlockTip(pindex);
    mnpayments.UpdatedBlockTip(pindex);
    governance.UpdatedBlockTip(pindex);
    masternodeSync.UpdatedBlockTip(pindex);
}

void CPSNotificationInterface::SyncTransaction(const CTransaction &tx, const CBlock *pblock)
{
    instantsend.SyncTransaction(tx, pblock);
}
