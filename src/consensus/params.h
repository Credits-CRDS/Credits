// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CREDITS_CONSENSUS_PARAMS_H
#define CREDITS_CONSENSUS_PARAMS_H

#include "uint256.h"

#include <map>
#include <string>

namespace Consensus {

enum DeploymentPos
{
    DEPLOYMENT_TESTDUMMY,
    DEPLOYMENT_CSV, // Deployment of BIP68, BIP112, and BIP113.
    MAX_VERSION_BITS_DEPLOYMENTS // NOTE: Also add new deployments to VersionBitsDeploymentInfo in versionbits.cpp
};

/**
 * Struct for each individual consensus rule change using BIP9.
 */
struct BIP9Deployment {
    /** Bit position to select the particular bit in nVersion. */
    int bit;
    /** Start MedianTime for version bits miner confirmation. Can be a date in the past */
    int64_t nStartTime;
    /** Timeout/expiry MedianTime for the deployment attempt. */
    int64_t nTimeout;
};

/**
 * Parameters that influence chain consensus.
 */
struct Params {
    uint256 hashGenesisBlock;
    int nMasternodePaymentsStartBlock;
    int nInstantSendKeepLock; // in blocks
    int nBudgetPaymentsStartBlock;
    int nBudgetPaymentsCycleBlocks;
    int nBudgetPaymentsWindowBlocks;
    int nBudgetProposalEstablishingTime; // in seconds
    int nSuperblockStartBlock;
    int nSuperblockCycle; // in blocks
    int nGovernanceMinQuorum; // Min absolute vote count to trigger an action
    int nGovernanceFilterElements;
    int nMasternodeMinimumConfirmations;
    /** Used to check majorities for block version upgrade */
    int nMajorityEnforceBlockUpgrade;
    int nMajorityRejectBlockOutdated;
    int nMajorityWindow;

    int nYr1TotalBlocks;
    int nYr2TotalBlocks;
    int nYr3TotalBlocks;
    int nYr4TotalBlocks;
    int nYr5TotalBlocks;
    int nYr6TotalBlocks;
    int nYr7TotalBlocks;
    int nYr8TotalBlocks;
    int nYr9TotalBlocks;
    int nYr10TotalBlocks;
    int nYr11TotalBlocks;
    int nYr12TotalBlocks;
    int nYr13TotalBlocks;
    int nYr14TotalBlocks;
    int nYr15TotalBlocks;
    int nYr16TotalBlocks;
    int nYr17TotalBlocks;
    int nYr18TotalBlocks;
    int nYr19TotalBlocks;
    int nYr20TotalBlocks;
    int nYr21TotalBlocks;
    int nYr22TotalBlocks;
    int nYr23TotalBlocks;
    int nYr24TotalBlocks;
    int nYr25TotalBlocks;
    int nYr26TotalBlocks;
    int nYr27TotalBlocks;
    int nYr28TotalBlocks;
    int nYr29TotalBlocks;
    int nYr30TotalBlocks;
    int nYr31TotalBlocks;
    int nYr32TotalBlocks;
    int nYr33TotalBlocks;
    int nYr34TotalBlocks;
    int nYr35TotalBlocks;
    /**
     * Minimum blocks including miner confirmation of the total of 2016 blocks in a retargetting period,
     * (nPowTargetTimespan / nPowTargetSpacing) which is also used for BIP9 deployments.
     * Examples: 1916 for 95%, 1512 for testchains.
     */
    uint32_t nRuleChangeActivationThreshold;
    uint32_t nMinerConfirmationWindow;
    BIP9Deployment vDeployments[MAX_VERSION_BITS_DEPLOYMENTS];
    /** Proof of work parameters */
    uint256 powLimit;
    bool fPowAllowMinDifficultyBlocks;
    bool fPowNoRetargeting;
    int64_t nPowTargetSpacing;
    int64_t nPowTargetTimespan;
    int64_t nPowAveragingWindow;
    int64_t nPowMaxAdjustDown;
    int64_t nPowMaxAdjustUp;
	int64_t AveragingWindowTimespan() const { return nPowAveragingWindow * nPowTargetSpacing; }
    int64_t MinActualTimespan() const { return (AveragingWindowTimespan() * (100 - nPowMaxAdjustUp  )) / 100; }
    int64_t MaxActualTimespan() const { return (AveragingWindowTimespan() * (100 + nPowMaxAdjustDown)) / 100; }
    int64_t DifficultyAdjustmentInterval() const { return nPowTargetTimespan / nPowTargetSpacing; }
};
} // namespace Consensus

#endif // CREDITS_CONSENSUS_PARAMS_H
