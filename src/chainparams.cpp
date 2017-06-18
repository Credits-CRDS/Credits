// Copyright (c) 2009-2017 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Developers
// Copyright (c) 2014-2017 The Dash Core Developers
// Copyright (c) 2017 Credits Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"

#include "arith_uint256.h"
#include "hash.h"
#include "consensus/merkle.h"
#include "streams.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include "uint256.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"

const arith_uint256 maxUint = UintToArith256(uint256S("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, const uint32_t nTime, const uint32_t nNonce, const uint32_t nBits, const int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 1497704700 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

static void MineGenesis(CBlockHeader& genesisBlock, const uint256& powLimit, bool noProduction)
{
    if(noProduction)
        genesisBlock.nTime = std::time(0);
    genesisBlock.nNonce = 0;

    printf("NOTE: Genesis nTime = %u \n", genesisBlock.nTime);
    printf("WARN: Genesis nNonce (BLANK!) = %u \n", genesisBlock.nNonce);

    arith_uint256 besthash;
    memset(&besthash,0xFF,32);
    arith_uint256 hashTarget = UintToArith256(powLimit);
    printf("Target: %s\n", hashTarget.GetHex().c_str());
    arith_uint256 newhash = UintToArith256(genesisBlock.GetHash());
    while (newhash > hashTarget) {
        genesisBlock.nNonce++;
        if (genesisBlock.nNonce == 0) {
            printf("NONCE WRAPPED, incrementing time\n");
            ++genesisBlock.nTime;
        }
        // If nothing found after trying for a while, print status
        if ((genesisBlock.nNonce & 0xfff) == 0)
            printf("nonce %08X: hash = %s (target = %s)\n",
                   genesisBlock.nNonce, newhash.ToString().c_str(),
                   hashTarget.ToString().c_str());

        if(newhash < besthash) {
            besthash = newhash;
            printf("New best: %s\n", newhash.GetHex().c_str());
        }
        newhash = UintToArith256(genesisBlock.GetHash());
    }
    printf("Genesis nTime = %u \n", genesisBlock.nTime);
    printf("Genesis nNonce = %u \n", genesisBlock.nNonce);
    printf("Genesis nBits: %08x\n", genesisBlock.nBits);
    printf("Genesis Hash = %s\n", newhash.ToString().c_str());
    printf("Genesis Hash Merkle Root = %s\n", genesisBlock.hashMerkleRoot.ToString().c_str());
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=00000ffd590b14, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=e0028e, nTime=1390095618, nBits=1e0ffff0, nNonce=28917698, vtx=1)
 *   CTransaction(hash=e0028e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d01044c5957697265642030392f4a616e2f3230313420546865204772616e64204578706572696d656e7420476f6573204c6976653a204f76657273746f636b2e636f6d204973204e6f7720416363657074696e6720426974636f696e73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0xA9037BAC7050C479B121CF)
 *   vMerkleTree: e0028e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "17th of June 2017: Credits(CRDS) Launched";
    const CScript genesisOutputScript = CScript() << ParseHex("") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Main network
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nMasternodePaymentsStartBlock = 100; // Masternode Payments begin on block 20546
        consensus.nInstantSendKeepLock = 24;
        consensus.nBudgetPaymentsStartBlock = 20545; // actual historical value
        consensus.nBudgetPaymentsCycleBlocks = 20545; //Blocks per month
        consensus.nBudgetPaymentsWindowBlocks = 100;
        consensus.nBudgetProposalEstablishingTime = 24 * 60 * 60;
        consensus.nSuperblockStartBlock = 20546;
        consensus.nSuperblockCycle = 20545; // 675 (Blocks per day) x 365.25 (Days per Year) / 12 = 20545
        consensus.nGovernanceMinQuorum = 10;
        consensus.nGovernanceFilterElements = 20000;
        consensus.nMasternodeMinimumConfirmations = 15;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.powLimit = uint256S("0000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 24 * 60 * 60; // Credits: 24 hours
        consensus.nPowTargetSpacing = 2 * 64; // Credits: 128 seconds
        consensus.nPowMaxAdjustDown = 32; // Credits: 32% adjustment down
        consensus.nPowMaxAdjustUp = 16; // Credits: 16% adjustment up
		consensus.nPowAveragingWindow = 17;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
		consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 321; // 95% of nMinerConfirmationWindow
        consensus.nMinerConfirmationWindow = 338; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1497694200; // June 17th 2017 10:10:00
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1529236800; // June 17th 2018 12:00:00

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x2f;
        pchMessageStart[1] = 0x32;
        pchMessageStart[2] = 0x45;
        pchMessageStart[3] = 0x51;
        vAlertPubKey = ParseHex("04244e071357b9b970e501d45181797f1fd675f19c62fb92252d3a63e31c95f94b488d95e9704b6e2985d76a6b05b4f0fa4b22027e734064f86c63480a75965a32");
        nDefaultPort = 31000;
        nMaxTipAge = 24 * 60 * 60;
        nPruneAfterHeight = 20545;
        startNewChain = false;

        genesis = CreateGenesisBlock(1497712544, 27540, UintToArith256(consensus.powLimit).GetCompact(), 1, (1 * COIN));
        if(startNewChain == true) { MineGenesis(genesis, consensus.powLimit, true); }

        consensus.hashGenesisBlock = genesis.GetHash();
        		
        if(!startNewChain) {
            assert(consensus.hashGenesisBlock == uint256S("0x0000dea5d2c92cf3f1dce5031cc2b368b2a5e3ebea73ea1278fef673d10b1345"));
            assert(genesis.hashMerkleRoot == uint256S("0x1df9b425c9a06de51b3fb210ffd2e051e05718e264e8ee4692592746c1566a0c"));
		}
		
        /*
        vSeeds.push_back(CDNSSeedData("", ""));
        vSeeds.push_back(CDNSSeedData("", "dyn.dnsseeder.com"));
        vSeeds.push_back(CDNSSeedData("dnsseeder.host", "dyn.dnsseeder.host"));
        vSeeds.push_back(CDNSSeedData("dnsseeder.net", "dyn.dnsseeder.net"));
        */
        
        // Credits addresses start with 'C'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,28);
        // Credits script addresses start with '5'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,10);
        // Credits private keys start with 'y'
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,140);
        // Credits BIP32 pubkeys start with 'xpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        // Credits BIP32 prvkeys start with 'xprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();
        // Credits BIP44 coin type is '5'
        nExtCoinType = 5;

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 60 * 60; // fulfilled requests expire in 1 hour
        strSporkPubKey = "04244e071357b9b970e501d45181797f1fd675f19c62fb92252d3a63e31c95f94b488d95e9704b6e2985d76a6b05b4f0fa4b22027e734064f86c63480a75965a32";
        strMasternodePaymentsPubKey = "04244e071357b9b970e501d45181797f1fd675f19c62fb92252d3a63e31c95f94b488d95e9704b6e2985d76a6b05b4f0fa4b22027e734064f86c63480a75965a32";

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (  0, uint256S("0x0000dea5d2c92cf3f1dce5031cc2b368b2a5e3ebea73ea1278fef673d10b1345")),
            1497712544, // * UNIX timestamp of last checkpoint block
            0,          // * total number of transactions between genesis and last checkpoint
            //   (the tx=... number in the SetBestChain debug.log lines)
            2000        // * estimated number of transactions per day after checkpoint
        };

        consensus.nYr1TotalBlocks = 246544;
        consensus.nYr2TotalBlocks = 493088;
        consensus.nYr3TotalBlocks = 739631;
        consensus.nYr4TotalBlocks = 986175;
        consensus.nYr5TotalBlocks = 1232719;
        consensus.nYr6TotalBlocks = 1479263;
        consensus.nYr7TotalBlocks = 1725806;
        consensus.nYr8TotalBlocks = 1972350;
        consensus.nYr9TotalBlocks = 2218894;
        consensus.nYr10TotalBlocks = 2465438;
        consensus.nYr11TotalBlocks = 2711981;
        consensus.nYr12TotalBlocks = 2958525;
        consensus.nYr13TotalBlocks = 3205069;
        consensus.nYr14TotalBlocks = 3451613;
        consensus.nYr15TotalBlocks = 3698156;
        consensus.nYr16TotalBlocks = 3944700;
        consensus.nYr17TotalBlocks = 4191244;
        consensus.nYr18TotalBlocks = 4437788;
        consensus.nYr19TotalBlocks = 4684331;
        consensus.nYr20TotalBlocks = 4930875;
        consensus.nYr21TotalBlocks = 5177419;
        consensus.nYr22TotalBlocks = 5423963;
        consensus.nYr23TotalBlocks = 5670506;
        consensus.nYr24TotalBlocks = 5917050;
        consensus.nYr25TotalBlocks = 6163594;
        consensus.nYr26TotalBlocks = 6410138;
        consensus.nYr27TotalBlocks = 6656681;
        consensus.nYr28TotalBlocks = 6903225;
        consensus.nYr29TotalBlocks = 7149769;
        consensus.nYr30TotalBlocks = 7396313;
        consensus.nYr31TotalBlocks = 7642856;
        consensus.nYr32TotalBlocks = 7889400;
        consensus.nYr33TotalBlocks = 8135944;
        consensus.nYr34TotalBlocks = 8382488;
        consensus.nYr35TotalBlocks = 8629031;
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nMasternodePaymentsStartBlock = 0;
        consensus.nInstantSendKeepLock = 24;
        consensus.nBudgetPaymentsStartBlock = 200;
        consensus.nBudgetPaymentsCycleBlocks = 50;
        consensus.nBudgetPaymentsWindowBlocks = 10;
        consensus.nBudgetProposalEstablishingTime = 60 * 20;
        consensus.nSuperblockStartBlock = 0;
        consensus.nSuperblockCycle = 24; // Superblocks can be issued hourly on testnet
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 500;
        consensus.nMasternodeMinimumConfirmations = 1;
        consensus.nMajorityEnforceBlockUpgrade = 510;
        consensus.nMajorityRejectBlockOutdated = 750;
        consensus.nMajorityWindow = 1000;
        consensus.powLimit = uint256S("000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowAveragingWindow = 17;
        consensus.nPowTargetTimespan = 24 * 60 * 60; // Credits: 24 hours
        consensus.nPowTargetSpacing = 2 * 64; // Credits: 256 seconds
        consensus.nPowMaxAdjustDown = 32; // Credits: 32% adjustment down
        consensus.nPowMaxAdjustUp = 16; // Credits: 16% adjustment up
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 254; // 75% of nMinerConfirmationWindow
        consensus.nMinerConfirmationWindow = 338; // nPowTargetTimespan / nPowTargetSpacing
		consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1497694200; // June 17th 2017 10:10:00
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1529236800; // June 17th 2018 12:00:00

        pchMessageStart[0] = 0x1f;
        pchMessageStart[1] = 0x22;
        pchMessageStart[2] = 0x05;
        pchMessageStart[3] = 0x30;
        vAlertPubKey = ParseHex("04244e071357b9b970e501d45181797f1fd675f19c62fb92252d3a63e31c95f94b488d95e9704b6e2985d76a6b05b4f0fa4b22027e734064f86c63480a75965a32");
        nDefaultPort = 31400;
        nMaxTipAge = 24 * 60 * 64;
        nPruneAfterHeight = 100;
        startNewChain = false;

        genesis = CreateGenesisBlock(1497712720, 4579, UintToArith256(consensus.powLimit).GetCompact(), 1, (1 * COIN));
        if(startNewChain == true) {
            MineGenesis(genesis, consensus.powLimit, true);
        }

        consensus.hashGenesisBlock = genesis.GetHash();

        if(!startNewChain)
            assert(consensus.hashGenesisBlock == uint256S("0x000d1affae588c5c766b2488fc9211801a6d53b06ec9bd8e237092798a4bb242"));
            assert(genesis.hashMerkleRoot == uint256S("0x1df9b425c9a06de51b3fb210ffd2e051e05718e264e8ee4692592746c1566a0c"));

        vFixedSeeds.clear();
        vSeeds.clear();
        //vSeeds.push_back(CDNSSeedData("",  ""));
        //vSeeds.push_back(CDNSSeedData("", ""));

        // Testnet Credits addresses start with 'C'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,28);
        // Testnet Credits script addresses start with '8' or '9'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,10);
        // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,158);
        // Testnet Credits BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        // Testnet Credits BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();
        // Testnet Credits BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 5 * 60; // fulfilled requests expire in 5 minutes
        strSporkPubKey = "04244e071357b9b970e501d45181797f1fd675f19c62fb92252d3a63e31c95f94b488d95e9704b6e2985d76a6b05b4f0fa4b22027e734064f86c63480a75965a32";
        strMasternodePaymentsPubKey = "04244e071357b9b970e501d45181797f1fd675f19c62fb92252d3a63e31c95f94b488d95e9704b6e2985d76a6b05b4f0fa4b22027e734064f86c63480a75965a32";

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (  0, uint256S("0x000d1affae588c5c766b2488fc9211801a6d53b06ec9bd8e237092798a4bb242")),
            1497712720, // * UNIX timestamp of last checkpoint block
            0,    // * total number of transactions between genesis and last checkpoint
            //   (the tx=... number in the SetBestChain debug.log lines)
            1000        // * estimated number of transactions per day after checkpoint
        };

        consensus.nYr1TotalBlocks = 246544;
        consensus.nYr2TotalBlocks = 493088;
        consensus.nYr3TotalBlocks = 739631;
        consensus.nYr4TotalBlocks = 986175;
        consensus.nYr5TotalBlocks = 1232719;
        consensus.nYr6TotalBlocks = 1479263;
        consensus.nYr7TotalBlocks = 1725806;
        consensus.nYr8TotalBlocks = 1972350;
        consensus.nYr9TotalBlocks = 2218894;
        consensus.nYr10TotalBlocks = 2465438;
        consensus.nYr11TotalBlocks = 2711981;
        consensus.nYr12TotalBlocks = 2958525;
        consensus.nYr13TotalBlocks = 3205069;
        consensus.nYr14TotalBlocks = 3451613;
        consensus.nYr15TotalBlocks = 3698156;
        consensus.nYr16TotalBlocks = 3944700;
        consensus.nYr17TotalBlocks = 4191244;
        consensus.nYr18TotalBlocks = 4437788;
        consensus.nYr19TotalBlocks = 4684331;
        consensus.nYr20TotalBlocks = 4930875;
        consensus.nYr21TotalBlocks = 5177419;
        consensus.nYr22TotalBlocks = 5423963;
        consensus.nYr23TotalBlocks = 5670506;
        consensus.nYr24TotalBlocks = 5917050;
        consensus.nYr25TotalBlocks = 6163594;
        consensus.nYr26TotalBlocks = 6410138;
        consensus.nYr27TotalBlocks = 6656681;
        consensus.nYr28TotalBlocks = 6903225;
        consensus.nYr29TotalBlocks = 7149769;
        consensus.nYr30TotalBlocks = 7396313;
        consensus.nYr31TotalBlocks = 7642856;
        consensus.nYr32TotalBlocks = 7889400;
        consensus.nYr33TotalBlocks = 8135944;
        consensus.nYr34TotalBlocks = 8382488;
        consensus.nYr35TotalBlocks = 8629031;
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nMasternodePaymentsStartBlock = 0;
        consensus.nInstantSendKeepLock = 24;
        consensus.nBudgetPaymentsStartBlock = 1000;
        consensus.nBudgetPaymentsCycleBlocks = 50;
        consensus.nBudgetPaymentsWindowBlocks = 10;
        consensus.nBudgetProposalEstablishingTime = 60 * 20;
        consensus.nSuperblockStartBlock = 0;
        consensus.nSuperblockCycle = 10;
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 100;
        consensus.nMasternodeMinimumConfirmations = 1;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.powLimit = uint256S("00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 24 * 60 * 60; // Credits: 24 hours
        consensus.nPowTargetSpacing = 2 * 64; // Credits: 256 seconds
        consensus.nPowMaxAdjustDown = 32; // Credits: 32% adjustment down
        consensus.nPowMaxAdjustUp = 16; // Credits: 16% adjustment up
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 254; // 75% of nMinerConfirmationWindow
        consensus.nMinerConfirmationWindow = 338; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;

        pchMessageStart[0] = 0x1f;
        pchMessageStart[1] = 0x22;
        pchMessageStart[2] = 0x05;
        pchMessageStart[3] = 0x2f;
        nMaxTipAge = 24 * 60 * 64;
        nDefaultPort = 31500;
        nPruneAfterHeight = 100;
        startNewChain = false;

        genesis = CreateGenesisBlock(1497712816, 40, UintToArith256(consensus.powLimit).GetCompact(), 1, (1 * COIN));
        if(startNewChain == true) {
            MineGenesis(genesis, consensus.powLimit, true);
        }

        consensus.hashGenesisBlock = genesis.GetHash();

        if(!startNewChain)
            assert(consensus.hashGenesisBlock == uint256S("0x0059cc0c9e478d929bf09cc1062f78cd0b335f8d2051adb83fec545e54c52bd2"));
            assert(genesis.hashMerkleRoot == uint256S("0x1df9b425c9a06de51b3fb210ffd2e051e05718e264e8ee4692592746c1566a0c"));

        vFixedSeeds.clear(); //! Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();  //! Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        nFulfilledRequestExpireTime = 5 * 60; // fulfilled requests expire in 5 minutes
        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (  0, uint256S("0x0059cc0c9e478d929bf09cc1062f78cd0b335f8d2051adb83fec545e54c52bd2")),
            1497712816, // * UNIX timestamp of last checkpoint block
            0,    // * total number of transactions between genesis and last checkpoint
            //   (the tx=... number in the SetBestChain debug.log lines)
            500        // * estimated number of transactions per day after checkpoint
        };
        // Regtest Credits addresses start with 'C'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,28);
        // Regtest Credits script addresses start with '8' or '9'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,19);
        // Regtest private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        // Regtest Credits BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        // Regtest Credits BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();
        // Regtest Credits BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;

        consensus.nYr1TotalBlocks = 246544;
        consensus.nYr2TotalBlocks = 493088;
        consensus.nYr3TotalBlocks = 739631;
        consensus.nYr4TotalBlocks = 986175;
        consensus.nYr5TotalBlocks = 1232719;
        consensus.nYr6TotalBlocks = 1479263;
        consensus.nYr7TotalBlocks = 1725806;
        consensus.nYr8TotalBlocks = 1972350;
        consensus.nYr9TotalBlocks = 2218894;
        consensus.nYr10TotalBlocks = 2465438;
        consensus.nYr11TotalBlocks = 2711981;
        consensus.nYr12TotalBlocks = 2958525;
        consensus.nYr13TotalBlocks = 3205069;
        consensus.nYr14TotalBlocks = 3451613;
        consensus.nYr15TotalBlocks = 3698156;
        consensus.nYr16TotalBlocks = 3944700;
        consensus.nYr17TotalBlocks = 4191244;
        consensus.nYr18TotalBlocks = 4437788;
        consensus.nYr19TotalBlocks = 4684331;
        consensus.nYr20TotalBlocks = 4930875;
        consensus.nYr21TotalBlocks = 5177419;
        consensus.nYr22TotalBlocks = 5423963;
        consensus.nYr23TotalBlocks = 5670506;
        consensus.nYr24TotalBlocks = 5917050;
        consensus.nYr25TotalBlocks = 6163594;
        consensus.nYr26TotalBlocks = 6410138;
        consensus.nYr27TotalBlocks = 6656681;
        consensus.nYr28TotalBlocks = 6903225;
        consensus.nYr29TotalBlocks = 7149769;
        consensus.nYr30TotalBlocks = 7396313;
        consensus.nYr31TotalBlocks = 7642856;
        consensus.nYr32TotalBlocks = 7889400;
        consensus.nYr33TotalBlocks = 8135944;
        consensus.nYr34TotalBlocks = 8382488;
        consensus.nYr35TotalBlocks = 8629031;
    }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
        return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
        return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}
