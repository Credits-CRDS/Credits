// Copyright (c) 2014-2017 The Dash Core Developers
// Copyright (c) 2017 Credits Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "masternode-payments.h"

#include "activemasternode.h"
#include "policy/fees.h"
#include "governance-classes.h"
#include "masternode-sync.h"
#include "masternodeman.h"
#include "messagesigner.h"
#include "netfulfilledman.h"
#include "spork.h"
#include "util.h"

#include <boost/lexical_cast.hpp>

/** Object for who's going to get paid on which blocks */
CMasternodePayments mnpayments;

CCriticalSection cs_vecPayees;
CCriticalSection cs_mapMasternodeBlocks;
CCriticalSection cs_mapMasternodePaymentVotes;

/**
* IsBlockValueValid
*
*   Determine if coinbase outgoing created money is the correct value
*
*   Why is this needed?
*   - In Credits some blocks are superblocks, which output much higher amounts of coins
*   - Otherblocks are 10% lower in outgoing value, so in total, no extra coins are created
*   - When non-superblocks are detected, the normal schedule should be maintained
*/

bool IsBlockValueValid(const CBlock& block, int nBlockHeight, CAmount blockReward,std::string &strErrorRet)
{
    strErrorRet = "";

    bool isBlockRewardValueMet = (block.vtx[0].GetValueOut() <= blockReward);
    if(fDebug) LogPrintf("block.vtx[0].GetValueOut() %lld <= blockReward %lld\n", block.vtx[0].GetValueOut(), blockReward);

    // we are still using budgets, but we have no data about them anymore,
    // all we know is predefined budget cycle and window

    const Consensus::Params& consensusParams = Params().GetConsensus();
     if(nBlockHeight < consensusParams.nSuperblockStartBlock) {
        int nOffset = nBlockHeight % consensusParams.nBudgetPaymentsCycleBlocks;
        if(nBlockHeight >= consensusParams.nBudgetPaymentsStartBlock &&
            nOffset < consensusParams.nBudgetPaymentsWindowBlocks) {
            if(masternodeSync.IsSynced() && !sporkManager.IsSporkActive(SPORK_13_OLD_SUPERBLOCK_FLAG)) {
                LogPrint("gobject", "IsBlockValueValid -- Client synced but budget spork is disabled, checking block value against block reward\n");
                if(!isBlockRewardValueMet) {
                    strErrorRet = strprintf("coinbase pays too much at height %d (actual=%d vs limit=%d), exceeded block reward, budgets are disabled",
                                            nBlockHeight, block.vtx[0].GetValueOut(), blockReward);
                }
                return isBlockRewardValueMet;
            }
            LogPrint("gobject", "IsBlockValueValid -- WARNING: Skipping budget block value checks, accepting block\n");
            // TODO: reprocess blocks to make sure they are legit?
            return true;
        }
        // LogPrint("gobject", "IsBlockValueValid -- Block is not in budget cycle window, checking block value against block reward\n");
        if(!isBlockRewardValueMet) {
            strErrorRet = strprintf("coinbase pays too much at height %d (actual=%d vs limit=%d), exceeded block reward, block is not in budget cycle window",
                                    nBlockHeight, block.vtx[0].GetValueOut(), blockReward);
        }
        return isBlockRewardValueMet;
    }

    // superblocks started

    CAmount nSuperblockMaxValue =  blockReward + CSuperblock::GetPaymentsLimit(nBlockHeight);
    bool isSuperblockMaxValueMet = (block.vtx[0].GetValueOut() <= nSuperblockMaxValue);

    LogPrint("gobject", "block.vtx[0].GetValueOut() %lld <= nSuperblockMaxValue %lld\n", block.vtx[0].GetValueOut(), nSuperblockMaxValue);

    if(!masternodeSync.IsSynced()) {
        // not enough data but at least it must NOT exceed superblock max value
        if(CSuperblock::IsValidBlockHeight(nBlockHeight)) {
            if(fDebug) LogPrintf("IsBlockPayeeValid -- WARNING: Client not synced, checking superblock max bounds only\n");
            if(!isSuperblockMaxValueMet) {
                strErrorRet = strprintf("coinbase pays too much at height %d (actual=%d vs limit=%d), exceeded superblock max value",
                                        nBlockHeight, block.vtx[0].GetValueOut(), nSuperblockMaxValue);
            }
            return isSuperblockMaxValueMet;
        }
        if(!isBlockRewardValueMet) {
            strErrorRet = strprintf("coinbase pays too much at height %d (actual=%d vs limit=%d), exceeded block reward, only regular blocks are allowed at this height",
                                    nBlockHeight, block.vtx[0].GetValueOut(), blockReward);
        }
        // it MUST be a regular block otherwise
        return isBlockRewardValueMet;
    }

    // we are synced, let's try to check as much data as we can

    if(sporkManager.IsSporkActive(SPORK_9_SUPERBLOCKS_ENABLED)) {
        // ONLY CHECK SUPERBLOCKS WHEN INITIALLY SYNCED AND CHECKING NEW BLOCK
        {
            // UP TO ONE HOUR OLD, OTHERWISE LONGEST CHAIN
            if(block.nTime + 60*60 < GetTime())
                return true;
        }

        if(CSuperblockManager::IsSuperblockTriggered(nBlockHeight)) {
            if(CSuperblockManager::IsValid(block.vtx[0], nBlockHeight, blockReward)) {
                LogPrint("gobject", "IsBlockValueValid -- Valid superblock at height %d: %s", nBlockHeight, block.vtx[0].ToString());
                // all checks are done in CSuperblock::IsValid, nothing to do here
                return true;
            }

            // triggered but invalid? that's weird
            LogPrintf("IsBlockValueValid -- ERROR: Invalid superblock detected at height %d: %s", nBlockHeight, block.vtx[0].ToString());
            // should NOT allow invalid superblocks, when superblocks are enabled
            strErrorRet = strprintf("invalid superblock detected at height %d", nBlockHeight);
            return false;
        }
        LogPrint("gobject", "IsBlockValueValid -- No triggered superblock detected at height %d\n", nBlockHeight);
        if(!isBlockRewardValueMet) {
            strErrorRet = strprintf("coinbase pays too much at height %d (actual=%d vs limit=%d), exceeded block reward, no triggered superblock detected",
                                    nBlockHeight, block.vtx[0].GetValueOut(), blockReward);
        }
    } else {
        // should NOT allow superblocks at all, when superblocks are disabled
        LogPrint("gobject", "IsBlockValueValid -- Superblocks are disabled, no superblocks allowed\n");
        if(!isBlockRewardValueMet) {
            strErrorRet = strprintf("coinbase pays too much at height %d (actual=%d vs limit=%d), exceeded block reward, superblocks are disabled",
                                    nBlockHeight, block.vtx[0].GetValueOut(), blockReward);
        }
    }

    // it MUST be a regular block
    return isBlockRewardValueMet;
}

bool IsBlockPayeeValid(const CTransaction& txNew, int nBlockHeight, CAmount blockReward)
{
    if(!masternodeSync.IsSynced()) {
        //there is no budget data to use to check anything, let's just accept the longest chain
        if(fDebug) LogPrintf("IsBlockPayeeValid -- WARNING: Client not synced, skipping block payee checks\n");
        return true;
    }

    // we are still using budgets, but we have no data about them anymore,
    // we can only check Masternode payments

    const Consensus::Params& consensusParams = Params().GetConsensus();

    if(nBlockHeight < consensusParams.nSuperblockStartBlock) {
        if(mnpayments.IsTransactionValid(txNew, nBlockHeight)) {
            LogPrint("mnpayments", "IsBlockPayeeValid -- Valid Masternode payment at height %d: %s", nBlockHeight, txNew.ToString());
            return true;
        }

        int nOffset = nBlockHeight % consensusParams.nBudgetPaymentsCycleBlocks;
        if(nBlockHeight >= consensusParams.nBudgetPaymentsStartBlock &&
            nOffset < consensusParams.nBudgetPaymentsWindowBlocks) {
            if(!sporkManager.IsSporkActive(SPORK_13_OLD_SUPERBLOCK_FLAG)) {
                // no budget blocks should be accepted here, if SPORK_13_OLD_SUPERBLOCK_FLAG is disabled
                LogPrint("gobject", "IsBlockPayeeValid -- ERROR: Client synced but budget spork is disabled and Masternode payment is invalid\n");
                return false;
            }
            // NOTE: this should never happen in real, SPORK_13_OLD_SUPERBLOCK_FLAG MUST be disabled when 12.1 starts to go live
            LogPrint("gobject", "IsBlockPayeeValid -- WARNING: Probably valid budget block, have no data, accepting\n");
            // TODO: reprocess blocks to make sure they are legit?
            return true;
        }

        if(sporkManager.IsSporkActive(SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT)) {
            LogPrintf("IsBlockPayeeValid -- ERROR: Invalid Masternode payment detected at height %d: %s", nBlockHeight, txNew.ToString());
            return false;
        }

        LogPrintf("IsBlockPayeeValid -- WARNING: Masternode payment enforcement is disabled, accepting any payee\n");
        return true;
    }

    // superblocks started
    // SEE IF THIS IS A VALID SUPERBLOCK

    if(!sporkManager.IsSporkActive(SPORK_9_SUPERBLOCKS_ENABLED)) {
        if(CSuperblockManager::IsSuperblockTriggered(nBlockHeight)) {
            if(CSuperblockManager::IsValid(txNew, nBlockHeight, blockReward)) {
                LogPrint("gobject", "IsBlockPayeeValid -- Valid superblock at height %d: %s", nBlockHeight, txNew.ToString());
                return true;
            }

            LogPrintf("IsBlockPayeeValid -- ERROR: Invalid superblock detected at height %d: %s", nBlockHeight, txNew.ToString());
            // should NOT allow such superblocks, when superblocks are enabled
            return false;
        }
        // continue validation, should pay MN
        LogPrint("gobject", "IsBlockPayeeValid -- No triggered superblock detected at height %d\n", nBlockHeight);
    } else {
        // should NOT allow superblocks at all, when superblocks are disabled
        LogPrint("gobject", "IsBlockPayeeValid -- Superblocks are disabled, no superblocks allowed\n");
    }

    // IF THIS ISN'T A SUPERBLOCK OR SUPERBLOCK IS INVALID, IT SHOULD PAY A MASTERNODE DIRECTLY
    if(mnpayments.IsTransactionValid(txNew, nBlockHeight)) {
        LogPrint("mnpayments", "IsBlockPayeeValid -- Valid Masternode payment at height %d: %s", nBlockHeight, txNew.ToString());
        return true;
    }

    if(sporkManager.IsSporkActive(SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT)) {
        LogPrintf("IsBlockPayeeValid -- ERROR: Invalid Masternode payment detected at height %d: %s", nBlockHeight, txNew.ToString());
        return false;
    }

    LogPrintf("IsBlockPayeeValid -- WARNING: Masternode payment enforcement is disabled, accepting any payee\n");
    return true;
}

void FillBlockPayments(CMutableTransaction& txNew, int nBlockHeight, CAmount blockReward, CTxOut& txoutMasternodeRet, std::vector<CTxOut>& voutSuperblockRet)
{
    // only create superblocks if spork is enabled AND if superblock is actually triggered
    // (height should be validated inside)
    if(sporkManager.IsSporkActive(SPORK_9_SUPERBLOCKS_ENABLED) &&
        CSuperblockManager::IsSuperblockTriggered(nBlockHeight)) {
            LogPrint("gobject", "FillBlockPayments -- triggered superblock creation at height %d\n", nBlockHeight);
            CSuperblockManager::CreateSuperblock(txNew, nBlockHeight, voutSuperblockRet);
            return;
    }

    if (chainActive.Height() > Params().GetConsensus().nMasternodePaymentsStartBlock) {
        // FILL BLOCK PAYEE WITH MASTERNODE PAYMENT OTHERWISE
        mnpayments.FillBlockPayee(txNew, nBlockHeight);
        LogPrint("mnpayments", "FillBlockPayments -- nBlockHeight %d blockReward %lld txoutMasternodeRet %s txNew %s",
                                nBlockHeight, blockReward, txoutMasternodeRet.ToString(), txNew.ToString());
    }
}

std::string GetRequiredPaymentsString(int nBlockHeight)
{
    // IF WE HAVE A ACTIVATED TRIGGER FOR THIS HEIGHT - IT IS A SUPERBLOCK, GET THE REQUIRED PAYEES
    if(CSuperblockManager::IsSuperblockTriggered(nBlockHeight)) {
        return CSuperblockManager::GetRequiredPaymentsString(nBlockHeight);
    }

    // OTHERWISE, PAY MASTERNODE
    return mnpayments.GetRequiredPaymentsString(nBlockHeight);
}

void CMasternodePayments::Clear()
{
    LOCK2(cs_mapMasternodeBlocks, cs_mapMasternodePaymentVotes);
    mapMasternodeBlocks.clear();
    mapMasternodePaymentVotes.clear();
}

bool CMasternodePayments::CanVote(COutPoint outMasternode, int nBlockHeight)
{
    LOCK(cs_mapMasternodePaymentVotes);

    if (mapMasternodesLastVote.count(outMasternode) && mapMasternodesLastVote[outMasternode] == nBlockHeight) {
        return false;
    }

    //record this Masternode voted
    mapMasternodesLastVote[outMasternode] = nBlockHeight;
    return true;
}

/**
*   FillBlockPayee
*
*   Fill Masternode ONLY payment block
*/

void CMasternodePayments::FillBlockPayee(CMutableTransaction& txNew, CAmount nFees)
{
    CBlockIndex* pindexPrev = chainActive.Tip();       
    if(!pindexPrev) return;        

    bool hasPayment = true;
    CScript payee;

    if (chainActive.Height() <= Params().GetConsensus().nMasternodePaymentsStartBlock){
            if (fDebug)
                LogPrintf("CreateNewBlock: No Masternode payments prior to block 20,546\n");
            hasPayment = false;
    }

    //spork
    if(!mnpayments.GetBlockPayee(pindexPrev->nHeight+1, payee)){       
        //no Masternode detected
        CMasternode* winningNode = mnodeman.Find(payee);
        if(winningNode){
            payee = GetScriptForDestination(winningNode->pubKeyCollateralAddress.GetID());
        } else {
            if (fDebug)
                LogPrintf("CreateNewBlock: Failed to detect Masternode to pay\n");
            hasPayment = false;
        }
    }

    CAmount blockValue;
    CAmount masternodePayment;

    if (chainActive.Height() == 0) { blockValue = 475000 * COIN; }
    else if (chainActive.Height() >= 1 && chainActive.Height() <= Params().GetConsensus().nYr1TotalBlocks) { blockValue = YEAR_1_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr1TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr2TotalBlocks) { blockValue = YEAR_2_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr2TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr3TotalBlocks) { blockValue = YEAR_3_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr3TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr4TotalBlocks) { blockValue = YEAR_4_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr4TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr5TotalBlocks) { blockValue = YEAR_5_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr5TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr6TotalBlocks) { blockValue = YEAR_6_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr6TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr7TotalBlocks) { blockValue = YEAR_7_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr7TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr8TotalBlocks) { blockValue = YEAR_8_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr8TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr9TotalBlocks) { blockValue = YEAR_9_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr9TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr10TotalBlocks) { blockValue = YEAR_10_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr10TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr11TotalBlocks) { blockValue = YEAR_11_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr11TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr12TotalBlocks) { blockValue = YEAR_12_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr12TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr13TotalBlocks) { blockValue = YEAR_13_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr13TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr14TotalBlocks) { blockValue = YEAR_14_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr14TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr15TotalBlocks) { blockValue = YEAR_15_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr15TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr16TotalBlocks) { blockValue = YEAR_16_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr16TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr17TotalBlocks) { blockValue = YEAR_17_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr17TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr18TotalBlocks) { blockValue = YEAR_18_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr18TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr19TotalBlocks) { blockValue = YEAR_19_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr19TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr20TotalBlocks) { blockValue = YEAR_20_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr20TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr21TotalBlocks) { blockValue = YEAR_21_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr21TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr22TotalBlocks) { blockValue = YEAR_22_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr22TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr23TotalBlocks) { blockValue = YEAR_23_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr23TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr24TotalBlocks) { blockValue = YEAR_24_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr24TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr25TotalBlocks) { blockValue = YEAR_25_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr25TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr26TotalBlocks) { blockValue = YEAR_26_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr26TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr27TotalBlocks) { blockValue = YEAR_27_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr27TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr28TotalBlocks) { blockValue = YEAR_28_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr28TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr29TotalBlocks) { blockValue = YEAR_29_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr29TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr30TotalBlocks) { blockValue = YEAR_30_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr30TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr31TotalBlocks) { blockValue = YEAR_31_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr31TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr32TotalBlocks) { blockValue = YEAR_32_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr32TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr33TotalBlocks) { blockValue = YEAR_33_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr33TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr34TotalBlocks) { blockValue = YEAR_34_POW_REWARD; }
    else if (chainActive.Height() > Params().GetConsensus().nYr34TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr35TotalBlocks) { blockValue = YEAR_35_POW_REWARD; }
    else { blockValue = YEAR_1_POW_REWARD; }

    if (hasPayment && chainActive.Height() > Params().GetConsensus().nMasternodePaymentsStartBlock && chainActive.Height() <= Params().GetConsensus().nYr1TotalBlocks) { masternodePayment = YEAR_1_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr1TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr2TotalBlocks) { masternodePayment = YEAR_2_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr2TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr3TotalBlocks) { masternodePayment = YEAR_3_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr3TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr4TotalBlocks) { masternodePayment = YEAR_4_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr4TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr5TotalBlocks) { masternodePayment = YEAR_5_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr5TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr6TotalBlocks) { masternodePayment = YEAR_6_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr6TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr7TotalBlocks) { masternodePayment = YEAR_7_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr7TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr8TotalBlocks) { masternodePayment = YEAR_8_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr8TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr9TotalBlocks) { masternodePayment = YEAR_9_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr9TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr10TotalBlocks) { masternodePayment = YEAR_10_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr10TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr11TotalBlocks) { masternodePayment = YEAR_11_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr11TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr12TotalBlocks) { masternodePayment = YEAR_12_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr12TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr13TotalBlocks) { masternodePayment = YEAR_13_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr13TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr14TotalBlocks) { masternodePayment = YEAR_14_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr14TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr15TotalBlocks) { masternodePayment = YEAR_15_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr15TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr16TotalBlocks) { masternodePayment = YEAR_16_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr16TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr17TotalBlocks) { masternodePayment = YEAR_17_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr17TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr18TotalBlocks) { masternodePayment = YEAR_18_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr18TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr19TotalBlocks) { masternodePayment = YEAR_19_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr19TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr20TotalBlocks) { masternodePayment = YEAR_20_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr20TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr21TotalBlocks) { masternodePayment = YEAR_21_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr21TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr22TotalBlocks) { masternodePayment = YEAR_22_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr22TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr23TotalBlocks) { masternodePayment = YEAR_23_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr23TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr24TotalBlocks) { masternodePayment = YEAR_24_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr24TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr25TotalBlocks) { masternodePayment = YEAR_25_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr25TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr26TotalBlocks) { masternodePayment = YEAR_26_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr26TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr27TotalBlocks) { masternodePayment = YEAR_27_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr27TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr28TotalBlocks) { masternodePayment = YEAR_28_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr28TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr29TotalBlocks) { masternodePayment = YEAR_29_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr29TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr30TotalBlocks) { masternodePayment = YEAR_30_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr30TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr31TotalBlocks) { masternodePayment = YEAR_31_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr31TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr32TotalBlocks) { masternodePayment = YEAR_32_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr32TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr33TotalBlocks) { masternodePayment = YEAR_33_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr33TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr34TotalBlocks) { masternodePayment = YEAR_34_MASTERNODE_PAYMENT; }
    else if (hasPayment && chainActive.Height() > Params().GetConsensus().nYr34TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr35TotalBlocks) { masternodePayment = YEAR_35_MASTERNODE_PAYMENT; }
    else  { masternodePayment = YEAR_1_MASTERNODE_PAYMENT; }

    txNew.vout[0].nValue = blockValue;

    if(hasPayment){
        txNew.vout.resize(2);

        txNew.vout[1].scriptPubKey = payee;
        txNew.vout[1].nValue = masternodePayment;

        if (chainActive.Height() == 0) { txNew.vout[0].nValue = 475000 * COIN; }
        else if (chainActive.Height() >= 1 && chainActive.Height() <= Params().GetConsensus().nYr1TotalBlocks) { txNew.vout[0].nValue = YEAR_1_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr1TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr2TotalBlocks) { txNew.vout[0].nValue = YEAR_2_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr2TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr3TotalBlocks) { txNew.vout[0].nValue = YEAR_3_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr3TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr4TotalBlocks) { txNew.vout[0].nValue = YEAR_4_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr4TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr5TotalBlocks) { txNew.vout[0].nValue = YEAR_5_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr5TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr6TotalBlocks) { txNew.vout[0].nValue = YEAR_6_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr6TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr7TotalBlocks) { txNew.vout[0].nValue = YEAR_7_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr7TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr8TotalBlocks) { txNew.vout[0].nValue = YEAR_8_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr8TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr9TotalBlocks) { txNew.vout[0].nValue = YEAR_9_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr9TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr10TotalBlocks) { txNew.vout[0].nValue = YEAR_10_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr10TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr11TotalBlocks) { txNew.vout[0].nValue = YEAR_11_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr11TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr12TotalBlocks) { txNew.vout[0].nValue = YEAR_12_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr12TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr13TotalBlocks) { txNew.vout[0].nValue = YEAR_13_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr13TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr14TotalBlocks) { txNew.vout[0].nValue = YEAR_14_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr14TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr15TotalBlocks) { txNew.vout[0].nValue = YEAR_15_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr15TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr16TotalBlocks) { txNew.vout[0].nValue = YEAR_16_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr16TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr17TotalBlocks) { txNew.vout[0].nValue = YEAR_17_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr17TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr18TotalBlocks) { txNew.vout[0].nValue = YEAR_18_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr18TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr19TotalBlocks) { txNew.vout[0].nValue = YEAR_19_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr19TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr20TotalBlocks) { txNew.vout[0].nValue = YEAR_20_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr20TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr21TotalBlocks) { txNew.vout[0].nValue = YEAR_21_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr21TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr22TotalBlocks) { txNew.vout[0].nValue = YEAR_22_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr22TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr23TotalBlocks) { txNew.vout[0].nValue = YEAR_23_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr23TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr24TotalBlocks) { txNew.vout[0].nValue = YEAR_24_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr24TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr25TotalBlocks) { txNew.vout[0].nValue = YEAR_25_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr25TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr26TotalBlocks) { txNew.vout[0].nValue = YEAR_26_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr26TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr27TotalBlocks) { txNew.vout[0].nValue = YEAR_27_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr27TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr28TotalBlocks) { txNew.vout[0].nValue = YEAR_28_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr28TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr29TotalBlocks) { txNew.vout[0].nValue = YEAR_29_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr29TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr30TotalBlocks) { txNew.vout[0].nValue = YEAR_30_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr30TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr31TotalBlocks) { txNew.vout[0].nValue = YEAR_31_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr31TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr32TotalBlocks) { txNew.vout[0].nValue = YEAR_32_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr32TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr33TotalBlocks) { txNew.vout[0].nValue = YEAR_33_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr33TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr34TotalBlocks) { txNew.vout[0].nValue = YEAR_34_POW_REWARD; }
        else if (chainActive.Height() > Params().GetConsensus().nYr34TotalBlocks && chainActive.Height() <= Params().GetConsensus().nYr35TotalBlocks) { txNew.vout[0].nValue = YEAR_35_POW_REWARD; }
        else { txNew.vout[0].nValue = YEAR_1_POW_REWARD; }

        CTxDestination address1;
        ExtractDestination(payee, address1);
        CCreditsAddress address2(address1);

        LogPrintf("CMasternodePayments::FillBlockPayee -- Masternode payment %lld to %s\n", masternodePayment, address2.ToString());
    }
}

int CMasternodePayments::GetMinMasternodePaymentsProto() {
    return sporkManager.IsSporkActive(SPORK_10_MASTERNODE_PAY_UPDATED_NODES);
}

void CMasternodePayments::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv)
{
    // Ignore any payments messages until Masternode list is synced
    if(!masternodeSync.IsMasternodeListSynced()) return;

    if(fLiteMode) return; // disable all Credits specific functionality

    if (strCommand == NetMsgType::MASTERNODEPAYMENTSYNC) { //Masternode Payments Request Sync

        // Ignore such requests until we are fully synced.
        // We could start processing this after Masternode list is synced
        // but this is a heavy one so it's better to finish sync first.
        if (!masternodeSync.IsSynced()) return;

        int nCountNeeded;
        vRecv >> nCountNeeded;
        
        int nMnCount = mnodeman.CountMasternodes();

        if (nMnCount > 200) {
            if(netfulfilledman.HasFulfilledRequest(pfrom->addr, NetMsgType::MASTERNODEPAYMENTSYNC)) {
                // Asking for the payments list multiple times in a short period of time is no good
                LogPrintf("MASTERNODEPAYMENTSYNC -- peer already asked me for the list, peer=%d\n", pfrom->id);
                Misbehaving(pfrom->GetId(), 20);
                return;
            }
        }

        netfulfilledman.AddFulfilledRequest(pfrom->addr, NetMsgType::MASTERNODEPAYMENTSYNC);

        Sync(pfrom);
        LogPrintf("MASTERNODEPAYMENTSYNC -- Sent Masternode payment votes to peer %d\n", pfrom->id);

    } else if (strCommand == NetMsgType::MASTERNODEPAYMENTVOTE) { // Masternode Payments Vote for the Winner

        CMasternodePaymentVote vote;
        vRecv >> vote;

        if(pfrom->nVersion < GetMinMasternodePaymentsProto()) return;

        if(!pCurrentBlockIndex) return;

        uint256 nHash = vote.GetHash();

        pfrom->setAskFor.erase(nHash);

        {
            LOCK(cs_mapMasternodePaymentVotes);
            if(mapMasternodePaymentVotes.count(nHash)) {
                LogPrint("mnpayments", "MASTERNODEPAYMENTVOTE -- hash=%s, nHeight=%d seen\n", nHash.ToString(), pCurrentBlockIndex->nHeight);
                return;
            }

            // Avoid processing same vote multiple times
            mapMasternodePaymentVotes[nHash] = vote;
            // but first mark vote as non-verified,
            // AddPaymentVote() below should take care of it if vote is actually ok
            mapMasternodePaymentVotes[nHash].MarkAsNotVerified();
        }

        int nFirstBlock = pCurrentBlockIndex->nHeight - GetStorageLimit();
        if(vote.nBlockHeight < nFirstBlock || vote.nBlockHeight > pCurrentBlockIndex->nHeight+20) {
            LogPrint("mnpayments", "MASTERNODEPAYMENTVOTE -- vote out of range: nFirstBlock=%d, nBlockHeight=%d, nHeight=%d\n", nFirstBlock, vote.nBlockHeight, pCurrentBlockIndex->nHeight);
            return;
        }

        std::string strError = "";
        if(!vote.IsValid(pfrom, pCurrentBlockIndex->nHeight, strError)) {
            LogPrint("mnpayments", "MASTERNODEPAYMENTVOTE -- invalid message, error: %s\n", strError);
            return;
        }

        if(!CanVote(vote.vinMasternode.prevout, vote.nBlockHeight)) {
            LogPrintf("MASTERNODEPAYMENTVOTE -- Masternode already voted, Masternode=%s\n", vote.vinMasternode.prevout.ToStringShort());
            return;
        }

        masternode_info_t mnInfo = mnodeman.GetMasternodeInfo(vote.vinMasternode);
        if(!mnInfo.fInfoValid) {
            // mn was not found, so we can't check vote, some info is probably missing
            LogPrintf("MASTERNODEPAYMENTVOTE -- Masternode is missing %s\n", vote.vinMasternode.prevout.ToStringShort());
            mnodeman.AskForMN(pfrom, vote.vinMasternode);
            return;
        }

        int nDos = 0;
        if(!vote.CheckSignature(mnInfo.pubKeyMasternode, pCurrentBlockIndex->nHeight, nDos)) {
            if(nDos) {
                LogPrintf("MASTERNODEPAYMENTVOTE -- ERROR: invalid signature\n");
                Misbehaving(pfrom->GetId(), nDos);
            } else {
                // only warn about anything non-critical (i.e. nDos == 0) in debug mode
                LogPrint("mnpayments", "MASTERNODEPAYMENTVOTE -- WARNING: invalid signature\n");
            }
            // Either our info or vote info could be outdated.
            // In case our info is outdated, ask for an update,
            mnodeman.AskForMN(pfrom, vote.vinMasternode);
            // but there is nothing we can do if vote info itself is outdated
            // (i.e. it was signed by a MN which changed its key),
            // so just quit here.
            return;
        }

        CTxDestination address1;
        ExtractDestination(vote.payee, address1);
        CCreditsAddress address2(address1);

        LogPrint("mnpayments", "MASTERNODEPAYMENTVOTE -- vote: address=%s, nBlockHeight=%d, nHeight=%d, prevout=%s\n", address2.ToString(), vote.nBlockHeight, pCurrentBlockIndex->nHeight, vote.vinMasternode.prevout.ToStringShort());

        if(AddPaymentVote(vote)){
            vote.Relay();
            masternodeSync.AddedPaymentVote();
        }
    }
}

bool CMasternodePaymentVote::Sign()
{
    std::string strError;
    std::string strMessage = vinMasternode.prevout.ToStringShort() +
                boost::lexical_cast<std::string>(nBlockHeight) +
                ScriptToAsmStr(payee);

    if(!CMessageSigner::SignMessage(strMessage, vchSig, activeMasternode.keyMasternode)) {
        LogPrintf("CMasternodePaymentVote::Sign -- SignMessage() failed\n");
        return false;
    }

    if(!CMessageSigner::VerifyMessage(activeMasternode.pubKeyMasternode, vchSig, strMessage, strError)) {
        LogPrintf("CMasternodePaymentVote::Sign -- VerifyMessage() failed, error: %s\n", strError);
        return false;
    }

    return true;
}

bool CMasternodePayments::GetBlockPayee(int nBlockHeight, CScript& payee)
{
    if(mapMasternodeBlocks.count(nBlockHeight)){
        return mapMasternodeBlocks[nBlockHeight].GetBestPayee(payee);
    }

    return false;
}

// Is this Masternode scheduled to get paid soon?
// -- Only look ahead up to 8 blocks to allow for propagation of the latest 2 blocks of votes
bool CMasternodePayments::IsScheduled(CMasternode& mn, int nNotBlockHeight)
{
    LOCK(cs_mapMasternodeBlocks);

    if(!pCurrentBlockIndex) return false;

    CScript mnpayee;
    mnpayee = GetScriptForDestination(mn.pubKeyCollateralAddress.GetID());

    CScript payee;
    for(int64_t h = pCurrentBlockIndex->nHeight; h <= pCurrentBlockIndex->nHeight + 8; h++){
        if(h == nNotBlockHeight) continue;
        if(mapMasternodeBlocks.count(h) && mapMasternodeBlocks[h].GetBestPayee(payee) && mnpayee == payee) {
            return true;
        }
    }

    return false;
}

bool CMasternodePayments::AddPaymentVote(const CMasternodePaymentVote& vote)
{
    uint256 blockHash = uint256();
    if(!GetBlockHash(blockHash, vote.nBlockHeight - 101)) return false;

    if(HasVerifiedPaymentVote(vote.GetHash())) return false;

    LOCK2(cs_mapMasternodeBlocks, cs_mapMasternodePaymentVotes);

    mapMasternodePaymentVotes[vote.GetHash()] = vote;

    if(!mapMasternodeBlocks.count(vote.nBlockHeight)) {
       CMasternodeBlockPayees blockPayees(vote.nBlockHeight);
       mapMasternodeBlocks[vote.nBlockHeight] = blockPayees;
    }

    mapMasternodeBlocks[vote.nBlockHeight].AddPayee(vote);

    return true;
}

bool CMasternodePayments::HasVerifiedPaymentVote(uint256 hashIn)
{
    LOCK(cs_mapMasternodePaymentVotes);
    std::map<uint256, CMasternodePaymentVote>::iterator it = mapMasternodePaymentVotes.find(hashIn);
    return it != mapMasternodePaymentVotes.end() && it->second.IsVerified();
}

void CMasternodeBlockPayees::AddPayee(const CMasternodePaymentVote& vote)
{
    LOCK(cs_vecPayees);

    BOOST_FOREACH(CMasternodePayee& payee, vecPayees) {
        if (payee.GetPayee() == vote.payee) {
            payee.AddVoteHash(vote.GetHash());
            return;
        }
    }
    CMasternodePayee payeeNew(vote.payee, vote.GetHash());
    vecPayees.push_back(payeeNew);
}

bool CMasternodeBlockPayees::GetBestPayee(CScript& payeeRet)
{
    LOCK(cs_vecPayees);

    if(!vecPayees.size()) {
        LogPrint("mnpayments", "CMasternodeBlockPayees::GetBestPayee -- ERROR: couldn't find any payee\n");
        return false;
    }

    int nVotes = -1;
    BOOST_FOREACH(CMasternodePayee& payee, vecPayees) {
        if (payee.GetVoteCount() > nVotes) {
            payeeRet = payee.GetPayee();
            nVotes = payee.GetVoteCount();
        }
    }

    return (nVotes > -1);
}

bool CMasternodeBlockPayees::HasPayeeWithVotes(CScript payeeIn, int nVotesReq)
{
    LOCK(cs_vecPayees);

    BOOST_FOREACH(CMasternodePayee& payee, vecPayees) {
        if (payee.GetVoteCount() >= nVotesReq && payee.GetPayee() == payeeIn) {
            return true;
        }
    }

    LogPrint("mnpayments", "CMasternodeBlockPayees::HasPayeeWithVotes -- ERROR: couldn't find any payee with %d+ votes\n", nVotesReq);
    return false;
}

bool CMasternodeBlockPayees::IsTransactionValid(const CTransaction& txNew)
{
    LOCK(cs_vecPayees);

    int nMaxSignatures = 0;
    std::string strPayeesPossible = "";

    CAmount nMasternodePayment = GetMasternodePayment();

    //require at least MNPAYMENTS_SIGNATURES_REQUIRED signatures

    BOOST_FOREACH(CMasternodePayee& payee, vecPayees) {
        if (payee.GetVoteCount() >= nMaxSignatures) {
            nMaxSignatures = payee.GetVoteCount();
        }
    }

    // if we don't have at least MNPAYMENTS_SIGNATURES_REQUIRED signatures on a payee, approve whichever is the longest chain
    if(nMaxSignatures < MNPAYMENTS_SIGNATURES_REQUIRED) return true;

    BOOST_FOREACH(CMasternodePayee& payee, vecPayees) {
        if (payee.GetVoteCount() >= MNPAYMENTS_SIGNATURES_REQUIRED) {
            BOOST_FOREACH(CTxOut txout, txNew.vout) {
                if (payee.GetPayee() == txout.scriptPubKey && nMasternodePayment == txout.nValue) {
                    LogPrint("mnpayments", "CMasternodeBlockPayees::IsTransactionValid -- Found required payment\n");
                    return true;
                }
            }

            CTxDestination address1;
            ExtractDestination(payee.GetPayee(), address1);
            CCreditsAddress address2(address1);

            if(strPayeesPossible == "") {
                strPayeesPossible = address2.ToString();
            } else {
                strPayeesPossible += "," + address2.ToString();
            }
        }
    }

    LogPrintf("CMasternodeBlockPayees::IsTransactionValid -- ERROR: Missing required payment, possible payees: '%s', amount: %f CRDS\n", strPayeesPossible, (float)nMasternodePayment/COIN);
    return false;
}

std::string CMasternodeBlockPayees::GetRequiredPaymentsString()
{
    LOCK(cs_vecPayees);

    std::string strRequiredPayments = "Unknown";

    BOOST_FOREACH(CMasternodePayee& payee, vecPayees)
    {
        CTxDestination address1;
        ExtractDestination(payee.GetPayee(), address1);
        CCreditsAddress address2(address1);

        if (strRequiredPayments != "Unknown") {
            strRequiredPayments += ", " + address2.ToString() + ":" + boost::lexical_cast<std::string>(payee.GetVoteCount());
        } else {
            strRequiredPayments = address2.ToString() + ":" + boost::lexical_cast<std::string>(payee.GetVoteCount());
        }
    }

    return strRequiredPayments;
}

std::string CMasternodePayments::GetRequiredPaymentsString(int nBlockHeight)
{
    LOCK(cs_mapMasternodeBlocks);

    if(mapMasternodeBlocks.count(nBlockHeight)){
        return mapMasternodeBlocks[nBlockHeight].GetRequiredPaymentsString();
    }

    return "Unknown";
}

bool CMasternodePayments::IsTransactionValid(const CTransaction& txNew, int nBlockHeight)
{
    LOCK(cs_mapMasternodeBlocks);

    if(mapMasternodeBlocks.count(nBlockHeight)){
        return mapMasternodeBlocks[nBlockHeight].IsTransactionValid(txNew);
    }

    return true;
}

void CMasternodePayments::CheckAndRemove()
{
    if(!pCurrentBlockIndex) return;

    LOCK2(cs_mapMasternodeBlocks, cs_mapMasternodePaymentVotes);

    int nLimit = GetStorageLimit();

    std::map<uint256, CMasternodePaymentVote>::iterator it = mapMasternodePaymentVotes.begin();
    while(it != mapMasternodePaymentVotes.end()) {
        CMasternodePaymentVote vote = (*it).second;

        if(pCurrentBlockIndex->nHeight - vote.nBlockHeight > nLimit) {
            LogPrint("mnpayments", "CMasternodePayments::CheckAndRemove -- Removing old Masternode payment: nBlockHeight=%d\n", vote.nBlockHeight);
            mapMasternodePaymentVotes.erase(it++);
            mapMasternodeBlocks.erase(vote.nBlockHeight);
        } else {
            ++it;
        }
    }
    LogPrintf("CMasternodePayments::CheckAndRemove -- %s\n", ToString());
}

bool CMasternodePaymentVote::IsValid(CNode* pnode, int nValidationHeight, std::string& strError)
{
    CMasternode* pmn = mnodeman.Find(vinMasternode);

    if(!pmn) {
        strError = strprintf("Unknown Masternode: prevout=%s", vinMasternode.prevout.ToStringShort());
        // Only ask if we are already synced and still have no idea about that Masternode
        if(masternodeSync.IsMasternodeListSynced()) {
            mnodeman.AskForMN(pnode, vinMasternode);
        }

        return false;
    }

    int nMinRequiredProtocol;
    if(nBlockHeight >= nValidationHeight) {
        // new votes must comply SPORK_10_MASTERNODE_PAY_UPDATED_NODES rules
        nMinRequiredProtocol = mnpayments.GetMinMasternodePaymentsProto();
    } else {
        // allow non-updated Masternodes for old blocks
        nMinRequiredProtocol = MIN_MASTERNODE_PAYMENT_PROTO_VERSION;
    }

    if(pmn->nProtocolVersion < nMinRequiredProtocol) {
        strError = strprintf("Masternode protocol is too old: nProtocolVersion=%d, nMinRequiredProtocol=%d", pmn->nProtocolVersion, nMinRequiredProtocol);
        return false;
    }

    // Only Masternodes should try to check Masternode rank for old votes - they need to pick the right winner for future blocks.
    // Regular clients (miners included) need to verify Masternode rank for future block votes only.
    if(!fMasterNode && nBlockHeight < nValidationHeight) return true;

    int nRank = mnodeman.GetMasternodeRank(vinMasternode, nBlockHeight - 101, nMinRequiredProtocol, false);

    if(nRank > MNPAYMENTS_SIGNATURES_TOTAL) {
        // It's common to have Masternodes mistakenly think they are in the top 10
        // We don't want to print all of these messages in normal mode, debug mode should print though
        strError = strprintf("Masternode is not in the top %d (%d)", MNPAYMENTS_SIGNATURES_TOTAL, nRank);
        // Only ban for new mnw which is out of bounds, for old mnw MN list itself might be way too much off
        if(nRank > MNPAYMENTS_SIGNATURES_TOTAL*2 && nBlockHeight > nValidationHeight) {
            strError = strprintf("Masternode is not in the top %d (%d)", MNPAYMENTS_SIGNATURES_TOTAL*2, nRank);
            LogPrintf("CMasternodePaymentVote::IsValid -- Error: %s\n", strError);
            Misbehaving(pnode->GetId(), 20);
        }
        // Still invalid however
        return false;
    }

    return true;
}

bool CMasternodePayments::ProcessBlock(int nBlockHeight)
{
    // DETERMINE IF WE SHOULD BE VOTING FOR THE NEXT PAYEE

    if(fLiteMode || !fMasterNode) return false;

    // We have little chances to pick the right winner if winners list is out of sync
    // but we have no choice, so we'll try. However it doesn't make sense to even try to do so
    // if we have not enough data about Masternodes.
    if(!masternodeSync.IsMasternodeListSynced()) return false;

    int nRank = mnodeman.GetMasternodeRank(activeMasternode.vin, nBlockHeight - 101, GetMinMasternodePaymentsProto(), false);

    if (nRank == -1) {
        LogPrint("mnpayments", "CMasternodePayments::ProcessBlock -- Unknown Masternode\n");
        return false;
    }

    if (nRank > MNPAYMENTS_SIGNATURES_TOTAL) {
        LogPrint("mnpayments", "CMasternodePayments::ProcessBlock -- Masternode not in the top %d (%d)\n", MNPAYMENTS_SIGNATURES_TOTAL, nRank);
        return false;
    }


    // LOCATE THE NEXT MASTERNODE WHICH SHOULD BE PAID

    LogPrintf("CMasternodePayments::ProcessBlock -- Start: nBlockHeight=%d, Masternode=%s\n", nBlockHeight, activeMasternode.vin.prevout.ToStringShort());

    // pay to the oldest MN that still had no payment but its input is old enough and it was active long enough
    int nCount = 0;
    CMasternode *pmn = mnodeman.GetNextMasternodeInQueueForPayment(nBlockHeight, true, nCount);

    if (pmn == NULL) {
        LogPrintf("CMasternodePayments::ProcessBlock -- ERROR: Failed to find Masternode to pay\n");
        return false;
    }

    LogPrintf("CMasternodePayments::ProcessBlock -- Masternode found by GetNextMasternodeInQueueForPayment(): %s\n", pmn->vin.prevout.ToStringShort());


    CScript payee = GetScriptForDestination(pmn->pubKeyCollateralAddress.GetID());

    CMasternodePaymentVote voteNew(activeMasternode.vin, nBlockHeight, payee);

    CTxDestination address1;
    ExtractDestination(payee, address1);
    CCreditsAddress address2(address1);

    LogPrintf("CMasternodePayments::ProcessBlock -- vote: payee=%s, nBlockHeight=%d\n", address2.ToString(), nBlockHeight);

    // SIGN MESSAGE TO NETWORK WITH OUR MASTERNODE KEYS

    LogPrintf("CMasternodePayments::ProcessBlock -- Signing vote\n");
    if (voteNew.Sign()) {
        LogPrintf("CMasternodePayments::ProcessBlock -- AddPaymentVote()\n");

        if (AddPaymentVote(voteNew)) {
            voteNew.Relay();
            return true;
        }
    }

    return false;
}

void CMasternodePaymentVote::Relay()
{
    // do not relay until synced
    if (!masternodeSync.IsWinnersListSynced()) return;
    CInv inv(MSG_MASTERNODE_PAYMENT_VOTE, GetHash());
    RelayInv(inv);
}

bool CMasternodePaymentVote::CheckSignature(const CPubKey& pubKeyMasternode, int nValidationHeight, int &nDos)
{
    // do not ban by default
    nDos = 0;

    std::string strMessage = vinMasternode.prevout.ToStringShort() +
                boost::lexical_cast<std::string>(nBlockHeight) +
                ScriptToAsmStr(payee);

    std::string strError = "";
    if (!CMessageSigner::VerifyMessage(pubKeyMasternode, vchSig, strMessage, strError)) {
        // Only ban for future block vote when we are already synced.
        // Otherwise it could be the case when MN which signed this vote is using another key now
        // and we have no idea about the old one.
        if(masternodeSync.IsMasternodeListSynced() && nBlockHeight > nValidationHeight) {
            nDos = 20;
        }
        return error("CMasternodePaymentVote::CheckSignature -- Got bad Masternode payment signature, Masternode=%s, error: %s", vinMasternode.prevout.ToStringShort().c_str(), strError);
    }

    return true;
}

std::string CMasternodePaymentVote::ToString() const
{
    std::ostringstream info;

    info << vinMasternode.prevout.ToStringShort() <<
            ", " << nBlockHeight <<
            ", " << ScriptToAsmStr(payee) <<
            ", " << (int)vchSig.size();

    return info.str();
}

// Send all votes up to nCountNeeded blocks (but not more than GetStorageLimit)        
void CMasternodePayments::Sync(CNode* pnode)
{
    LOCK(cs_mapMasternodeBlocks);

    if(!pCurrentBlockIndex) return;

    int nInvCount = 0;

    for(int h = pCurrentBlockIndex->nHeight; h < pCurrentBlockIndex->nHeight + 20; h++) {
        if(mapMasternodeBlocks.count(h)) {
            BOOST_FOREACH(CMasternodePayee& payee, mapMasternodeBlocks[h].vecPayees) {
                std::vector<uint256> vecVoteHashes = payee.GetVoteHashes();
                BOOST_FOREACH(uint256& hash, vecVoteHashes) {
                    if(!HasVerifiedPaymentVote(hash)) continue;
                    pnode->PushInventory(CInv(MSG_MASTERNODE_PAYMENT_VOTE, hash));
                    nInvCount++;
                }
            }
        }
    }

    LogPrintf("CMasternodePayments::Sync -- Sent %d votes to peer %d\n", nInvCount, pnode->id);
    pnode->PushMessage(NetMsgType::SYNCSTATUSCOUNT, MASTERNODE_SYNC_MNW, nInvCount);
}

// Request low data/unknown payment blocks in batches directly from some node instead of/after preliminary Sync.
void CMasternodePayments::RequestLowDataPaymentBlocks(CNode* pnode)
{
    if(!pCurrentBlockIndex) return;

    LOCK2(cs_main, cs_mapMasternodeBlocks);

    std::vector<CInv> vToFetch;
    int nLimit = GetStorageLimit();

    const CBlockIndex *pindex = pCurrentBlockIndex;

    while(pCurrentBlockIndex->nHeight - pindex->nHeight < nLimit) {
        if(!mapMasternodeBlocks.count(pindex->nHeight)) {
            // We have no idea about this block height, let's ask
            vToFetch.push_back(CInv(MSG_MASTERNODE_PAYMENT_BLOCK, pindex->GetBlockHash()));
            // We should not violate GETDATA rules
            if(vToFetch.size() == MAX_INV_SZ) {
                LogPrintf("CMasternodePayments::SyncLowDataPaymentBlocks -- asking peer %d for %d blocks\n", pnode->id, MAX_INV_SZ);
                pnode->PushMessage(NetMsgType::GETDATA, vToFetch);
                // Start filling new batch
                vToFetch.clear();
            }
        }
        if(!pindex->pprev) break;
        pindex = pindex->pprev;
    }

    std::map<int, CMasternodeBlockPayees>::iterator it = mapMasternodeBlocks.begin();

    while(it != mapMasternodeBlocks.end()) {
        int nTotalVotes = 0;
        bool fFound = false;
        BOOST_FOREACH(CMasternodePayee& payee, it->second.vecPayees) {
            if(payee.GetVoteCount() >= MNPAYMENTS_SIGNATURES_REQUIRED) {
                fFound = true;
                break;
            }
            nTotalVotes += payee.GetVoteCount();
        }
        // A clear winner (MNPAYMENTS_SIGNATURES_REQUIRED+ votes) was found
        // or no clear winner was found but there are at least avg number of votes
        if(fFound || nTotalVotes >= (MNPAYMENTS_SIGNATURES_TOTAL + MNPAYMENTS_SIGNATURES_REQUIRED)/2) {
            // so just move to the next block
            ++it;
            continue;
        }
        // DEBUG
        DBG (
            // Let's see why this failed
            BOOST_FOREACH(CMasternodePayee& payee, it->second.vecPayees) {
                CTxDestination address1;
                ExtractDestination(payee.GetPayee(), address1);
                CCreditsAddress address2(address1);
                printf("payee %s votes %d\n", address2.ToString().c_str(), payee.GetVoteCount());
            }
            printf("block %d votes total %d\n", it->first, nTotalVotes);
        )
        // END DEBUG
        // Low data block found, let's try to sync it
        uint256 hash;
        if(GetBlockHash(hash, it->first)) {
            vToFetch.push_back(CInv(MSG_MASTERNODE_PAYMENT_BLOCK, hash));
        }
        // We should not violate GETDATA rules
        if(vToFetch.size() == MAX_INV_SZ) {
            LogPrintf("CMasternodePayments::SyncLowDataPaymentBlocks -- asking peer %d for %d payment blocks\n", pnode->id, MAX_INV_SZ);
            pnode->PushMessage(NetMsgType::GETDATA, vToFetch);
            // Start filling new batch
            vToFetch.clear();
        }
        ++it;
    }
    // Ask for the rest of it
    if(!vToFetch.empty()) {
        LogPrintf("CMasternodePayments::SyncLowDataPaymentBlocks -- asking peer %d for %d payment blocks\n", pnode->id, vToFetch.size());
        pnode->PushMessage(NetMsgType::GETDATA, vToFetch);
    }
}

std::string CMasternodePayments::ToString() const
{
    std::ostringstream info;

    info << "Votes: " << (int)mapMasternodePaymentVotes.size() <<
            ", Blocks: " << (int)mapMasternodeBlocks.size();

    return info.str();
}

bool CMasternodePayments::IsEnoughData()
{
    float nAverageVotes = (MNPAYMENTS_SIGNATURES_TOTAL + MNPAYMENTS_SIGNATURES_REQUIRED) / 2;
    int nStorageLimit = GetStorageLimit();
    return GetBlockCount() > nStorageLimit && GetVoteCount() > nStorageLimit * nAverageVotes;
}

int CMasternodePayments::GetStorageLimit()
{
    return std::max(int(mnodeman.size() * nStorageCoeff), nMinBlocksToStore);
}

void CMasternodePayments::UpdatedBlockTip(const CBlockIndex *pindex)
{
    pCurrentBlockIndex = pindex;
    LogPrint("mnpayments", "CMasternodePayments::UpdatedBlockTip -- pCurrentBlockIndex->nHeight=%d\n", pCurrentBlockIndex->nHeight);

    ProcessBlock(pindex->nHeight + 10);
}
