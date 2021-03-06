// vim: set et sw=4 sts=4 cindent:
/*
 * @File: steeralg.c
 *
 * @Abstract: Implementation of BSS steeralg
 *
 * @Notes:
 *
 * @@-COPYRIGHT-START-@@
 *
 * Copyright (c) 2015-2019 Qualcomm Technologies, Inc.
 *
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * 2015-2016 Qualcomm Atheros, Inc.
 *
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 *
 * @@-COPYRIGHT-END-@@
 *
 */
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#include <dbg.h>
#include <evloop.h>

#ifdef LBD_DBG_MENU
#include <cmd.h>
#endif

#include "lb_common.h"
#include "lb_assert.h"
#include "module.h"
#include "profile.h"
#include "stadb.h"
#include "bandmon.h"
#include "steerexec.h"
#include "estimator.h"

#include "steeralg.h"
#include "steeralgCmn.h"

#ifdef SON_MEMORY_DEBUG

#include "qca-son-mem-debug.h"
#undef QCA_MOD_INPUT
#define QCA_MOD_INPUT QCA_MOD_LBD_STEERALG
#include "son-mem-debug.h"

#endif /* SON_MEMORY_DEBUG */

steeralgPriv_t steeralgState;

/**
 * @brief Paramters used when iterating over STA database to determine
 *        the active steering candidate for offloading channel
 */
typedef struct steeralgCmnSelectOffloadingCandidatesParams_t {
    /// The AP on which to select offloading candidates
    lbd_apId_t apId;

    /// The channel on which to select offloading candidates
    lbd_channelId_t channelId;

    /// The freq on which to select offloading candidates
    u_int16_t freq;

    /// Number of candidates allowed in candidateList
    size_t numCandidatesAllocated;

    /// If the channel to offload is on 5 GHz, whether it is
    /// the strongest 5 GHz channel (with highest Tx power)
    LBD_BOOL isStrongestChannel5G;

    /// If the channel to offload is on 6 GHz, whether it is
    /// the strongest 6 GHz channel (with highest Tx power)
    LBD_BOOL isStrongestChannel6G;

} steeralgCmnSelectOffloadingCandidatesParams_t;

// Forward declarations
static void steeralgMenuInit(void);
static u_int32_t steeralgCmnIdleSteerCallback(stadbEntry_handle_t entry,
                                              stadbEntry_bssStatsHandle_t bssHandle,
                                              void *cookie);
static LBD_STATUS steeralgCmnFindCandidatesForIdleClient(
        stadbEntry_handle_t entry, size_t *maxNumBSS,
        steeralgCmnServingBSSInfo_t *servingBSS, lbd_bssInfo_t *bssCandidates);
static void steeralgCmnHandleOverloadChangeEvent(struct mdEventNode *event);
static void steeralgCmnHandleUtilizationUpdateEvent(struct mdEventNode *event);
static void steeralgCmnHandleUtilizationUpdate(size_t numOverloadedChannels);
static void steeralgCmnHandleSTAMetricsCompleteEvent(struct mdEventNode *event);
static time_t steeralgCmnGetTimestamp(void);
static lbd_channelId_t steeralgCmnSelectOverloadedChannel(
    lbd_apId_t apId, size_t numChannels, const lbd_channelId_t *channelList,
    const u_int16_t *freqList, u_int16_t *pselectedFreq);
static void steeralgCmnOffloadOverloadedChannel(size_t numOverloadedChannels,
                                                lbd_apId_t apId);
static void steeralgCmnResetServedChannels(void);
static LBD_BOOL steeralgCmnIsChannelServedBefore(lbd_channelId_t channelId,
                                                 time_t *lastServedTime);
static void steeralgCmnAddChannelToServedChannels(lbd_channelId_t channelId);
static u_int32_t steeralgCmnComputeIdleSteerMetric(
        stadbEntry_handle_t handle, stadbEntry_bssStatsHandle_t bssStats,
        wlanif_phymode_e bestPHYMode);
static void steeralgCmnHandlePerSTAAirtimeCompleteEvent(struct mdEventNode *event);
static void steeralgCmnSelectOffloadingCandidateCB(stadbEntry_handle_t entry,
                                                   void *cookie);
static void steeralgCmnContinueSteerActiveClientsOverload(lbd_airtime_t offloadedAirtime,
                                                          LBD_BOOL lastComplete);
static void steeralgCmnRecordOffloadingCandidate(
        stadbEntry_handle_t entry, steeralgCmnSelectOffloadingCandidatesParams_t *params,
        lbd_airtime_t airtime);
static void steeralgCmnSortOffloadCandidates(void);
static int steeralgCmnCompareOffloadCandidates(const void *candidate1, const void *candidate2);
static LBD_BOOL steeralgCmnIsSTAMetricsTriggeredByOffloading(
        stadbEntry_handle_t entry, lbd_airtime_t *occupiedAirtime);
static u_int32_t steeralgCmnActiveSteerCallback(
        stadbEntry_handle_t entry, stadbEntry_bssStatsHandle_t bssHandle,
        void *cookie);
static steeralg_rateSteerEligibility_e steeralgCmnEligibleActiveSteerRateAndRSSI(
    stadbEntry_handle_t entry, const struct ether_addr *addr,
    steeralgCmnServingBSSInfo_t *servingBSS, lbd_rssi_t *rssi);
static LBD_STATUS steeralgCmnFindCandidatesForActiveClient(
        stadbEntry_handle_t entry, const struct ether_addr *staAddr,
        steeralgCmnServingBSSInfo_t *servingBSS, size_t *maxNumBSS,
        lbd_bssInfo_t *bssCandidates);
static u_int32_t steeralgCmnGetActiveClientMetric(
        stadbEntry_handle_t entry, stadbEntry_bssStatsHandle_t bssHandle,
        const lbd_bssInfo_t *bss, lbd_linkCapacity_t dlEstimate,
        lbd_airtime_t availableAirtime);
static LBD_BOOL steeralgCmnIsCandidateForOffloadingSTA(
        stadbEntry_handle_t entry, stadbEntry_bssStatsHandle_t targetBSS,
        const lbd_bssInfo_t *bssInfo, lbd_linkCapacity_t dlEstimate,
        time_t deltaSecs, lbd_airtime_t *availableAirtime);
static LBD_BOOL steeralgCmnIsRateAboveThreshold(
        stadbEntry_handle_t entry, stadbEntry_bssStatsHandle_t bssHandle,
        const lbd_bssInfo_t *bss, lbd_linkCapacity_t threshold,
        lbd_linkCapacity_t dlEstimate, time_t ageSecs);
static void steeralgCmnUpdatePHYBasedPrioritization(u_int8_t numActiveChannels);
static void steeralgCmnHandleNumActiveChannelsChangeEvent(
    struct mdEventNode *event);

/**
 * @brief Default configuration values.
 *
 * These are used if the config file does not specify them.
 */
static struct profileElement steeralgCmnElementDefaultTable[] = {
    { STEERALG_INACT_RSSI_THRESHOLD_W2_KEY,          "5" },
    { STEERALG_INACT_RSSI_THRESHOLD_W5_KEY,         "30" },
    { STEERALG_HIGH_TX_RATE_XING_THRESHOLD,         "50000"},
    { STEERALG_HIGH_RATE_RSSI_XING_THRESHOLD,       "40"},
    { STEERALG_LOW_TX_RATE_XING_THRESHOLD,          "6000" },
    { STEERALG_LOW_RATE_RSSI_XING_THRESHOLD,        "0"},
    { STEERALG_MIN_TXRATE_INCREASE_THRESHOLD_KEY,   "53"},
    { STEERALG_AGE_LIMIT_KEY,                        "5" },
    { STEERALG_BACKHAUL_AGE_LIMIT_KEY,               "60" },
    { STEERALG_LEGACY_CLIENT_AGE_LIMIT_KEY,          "15" },
    { STEERALG_PHY_BASED_PRIORITIZATION,             "0" },
    { STEERALG_RSSI_SAFETY_THRESHOLD_KEY,            "20" },
    { STEERALG_MAX_STEERING_TARGET_COUNT_KEY,        "1" },
    { STEERALG_APPLY_ESTIMATED_AIRTIME_ON_STEERING_KEY, "1" },
    { STEERALG_USE_PATH_CAP_TO_SELECT_BSS_KEY,          "1" },

    // Default value for AP steering threshold is set to invalid, to
    // avoid crossing event in single-AP setup. These values must be
    // set through config file in multi-AP setup.
    { STEERALG_AP_STEER_TO_ROOT_MIN_RSSI_INC_KEY,    "127" }, // INT8_MAX
    { STEERALG_AP_STEER_TO_LEAF_MIN_RSSI_INC_KEY,    "127" },
    { STEERALG_AP_STEER_TO_PEER_MIN_RSSI_INC_KEY,    "127" },
    { STEERALG_DL_RSSI_THRESHOLD_W5_KEY,             "0" },
    { STEERALG_DL_RSSI_THRESHOLD_W6_KEY,             "0" },
    { STEERALG_AP_STEER_TOP_CAP_THRESHOLD_W2_KEY,    "70"},
    { STEERALG_AP_STEER_TOP_CAP_THRESHOLD_W5_KEY,    "70"},
    { STEERALG_AP_STEER_TOP_CAP_THRESHOLD_W6_KEY,    "70"},
    { STEERALG_AP_STEER_MAX_RETRY_COUNT_KEY,         "2"},
    { STEERALG_ENABLE_MULTI_11KREQUEST,              "1"},
    { NULL, NULL }
};


// ====================================================================
// Public API
// ====================================================================

LBD_STATUS steeralg_init(void) {
    u_int8_t index = 0;
    int value[BSTEERING_MAX_CLIENT_CLASS_GROUP];
    steeralgState.dbgModule = dbgModuleFind("steeralg");
    steeralgState.dbgModule->Level = DBGINFO;

    if (profileGetOptsIntArray(mdModuleID_SteerAlg,
        STEERALG_INACT_RSSI_THRESHOLD_W2_KEY,
        steeralgCmnElementDefaultTable, value) == LBD_NOK) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "[Band %u] Unable to parse %s",
             wlanif_band_24g, STEERALG_INACT_RSSI_THRESHOLD_W2_KEY);
        return LBD_NOK;
    }
    for (index = 0; index < BSTEERING_MAX_CLIENT_CLASS_GROUP; index++) {
        steeralgState.config.inactRSSIXingThreshold[wlanif_band_24g][index] = value[index];
    }

    if (profileGetOptsIntArray(mdModuleID_SteerAlg,
        STEERALG_INACT_RSSI_THRESHOLD_W5_KEY,
        steeralgCmnElementDefaultTable, value) == LBD_NOK) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "[Band %u] Unable to parse %s",
             wlanif_band_5g, STEERALG_INACT_RSSI_THRESHOLD_W5_KEY);
        return LBD_NOK;
    }
    for (index = 0; index < BSTEERING_MAX_CLIENT_CLASS_GROUP; index++) {
        steeralgState.config.inactRSSIXingThreshold[wlanif_band_5g][index] = value[index];
    }

    if (profileGetOptsIntArray(mdModuleID_SteerAlg,
        STEERALG_INACT_RSSI_THRESHOLD_W6_KEY,
        steeralgCmnElementDefaultTable, value) == LBD_NOK) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "[Band %u] Unable to parse %s",
             wlanif_band_6g, STEERALG_INACT_RSSI_THRESHOLD_W6_KEY);
        return LBD_NOK;
    }
    for (index = 0; index < BSTEERING_MAX_CLIENT_CLASS_GROUP; index++) {
        steeralgState.config.inactRSSIXingThreshold[wlanif_band_6g][index] = value[index];
    }

    if (profileGetOptsIntArray(mdModuleID_SteerAlg,
        STEERALG_HIGH_TX_RATE_XING_THRESHOLD,
        steeralgCmnElementDefaultTable, value) == LBD_NOK) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "Unable to parse %s",
             STEERALG_HIGH_TX_RATE_XING_THRESHOLD);
        return LBD_NOK;
    }

    // Convert to Mbps for steeralg
    for (index = 0; index < BSTEERING_MAX_CLIENT_CLASS_GROUP; index++) {
        steeralgState.config.highTxRateThreshold[index] = value[index] / 1000;
    }

    u_int32_t rateThreshold =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_LOW_TX_RATE_XING_THRESHOLD,
                          steeralgCmnElementDefaultTable);

    // Convert to Mbps for steeralg
    steeralgState.config.lowTxRateThreshold = rateThreshold / 1000;

    steeralgState.config.minTxRateIncreaseThreshold =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_MIN_TXRATE_INCREASE_THRESHOLD_KEY,
                          steeralgCmnElementDefaultTable);

    steeralgState.config.freshnessLimit =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_AGE_LIMIT_KEY,
                          steeralgCmnElementDefaultTable);

    steeralgState.config.multi11krequest =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_ENABLE_MULTI_11KREQUEST,
                          steeralgCmnElementDefaultTable);

    steeralgState.config.backhaulFreshnessLimit =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_BACKHAUL_AGE_LIMIT_KEY,
                          steeralgCmnElementDefaultTable);

    steeralgState.config.legacyClientFreshnessLimit =
        profileGetOptsInt(mdModuleID_SteerAlg, STEERALG_LEGACY_CLIENT_AGE_LIMIT_KEY,
                          steeralgCmnElementDefaultTable);

    steeralgState.config.lowRateRSSIXingThreshold =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_LOW_RATE_RSSI_XING_THRESHOLD,
                          steeralgCmnElementDefaultTable);

    if (profileGetOptsIntArray(mdModuleID_SteerAlg,
        STEERALG_HIGH_RATE_RSSI_XING_THRESHOLD,
        steeralgCmnElementDefaultTable, value) == LBD_NOK) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "Unable to parse %s",
             STEERALG_HIGH_RATE_RSSI_XING_THRESHOLD);
        return LBD_NOK;
    }
    for (index = 0; index < BSTEERING_MAX_CLIENT_CLASS_GROUP; index++) {
        steeralgState.config.highRateRSSIXingThreshold[index] = value[index];
    }

    steeralgState.config.phyBasedPrioritization =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_PHY_BASED_PRIORITIZATION,
                          steeralgCmnElementDefaultTable) > 0;

    steeralgState.numActiveChannels = bandmon_getNumActiveChannels();
    steeralgCmnUpdatePHYBasedPrioritization(steeralgState.numActiveChannels);

    if (profileGetOptsIntArray(mdModuleID_SteerAlg,
        STEERALG_RSSI_SAFETY_THRESHOLD_KEY,
        steeralgCmnElementDefaultTable, value) == LBD_NOK) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "Unable to parse %s",
             STEERALG_RSSI_SAFETY_THRESHOLD_KEY);
        return LBD_NOK;
    }
    for (index = 0; index < BSTEERING_MAX_CLIENT_CLASS_GROUP; index++) {
        steeralgState.config.rssiSafetyThreshold[index] = value[index];
    }

    steeralgState.config.apSteerToRootMinRSSIIncreaseThreshold =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_AP_STEER_TO_ROOT_MIN_RSSI_INC_KEY,
                          steeralgCmnElementDefaultTable);

    steeralgState.config.apSteerToLeafMinRSSIIncreaseThreshold =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_AP_STEER_TO_LEAF_MIN_RSSI_INC_KEY,
                          steeralgCmnElementDefaultTable);

    steeralgState.config.apSteerToPeerMinRSSIIncreaseThreshold =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_AP_STEER_TO_PEER_MIN_RSSI_INC_KEY,
                          steeralgCmnElementDefaultTable);

    steeralgState.config.apSteerTopCapThreshold[wlanif_band_24g] =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_AP_STEER_TOP_CAP_THRESHOLD_W2_KEY,
                          steeralgCmnElementDefaultTable);

    steeralgState.config.apSteerTopCapThreshold[wlanif_band_5g] =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_AP_STEER_TOP_CAP_THRESHOLD_W5_KEY,
                          steeralgCmnElementDefaultTable);

    steeralgState.config.apSteerTopCapThreshold[wlanif_band_6g] =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_AP_STEER_TOP_CAP_THRESHOLD_W6_KEY,
                          steeralgCmnElementDefaultTable);

    steeralgState.config.apSteerMaxRetryCount =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_AP_STEER_MAX_RETRY_COUNT_KEY,
                          steeralgCmnElementDefaultTable);

    steeralgState.config.dlRSSIThresholdW5 =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_DL_RSSI_THRESHOLD_W5_KEY,
                          steeralgCmnElementDefaultTable);

    steeralgState.config.dlRSSIThresholdW6 =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_DL_RSSI_THRESHOLD_W6_KEY,
                          steeralgCmnElementDefaultTable);

    steeralgState.config.maxSteeringTargetCount =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_MAX_STEERING_TARGET_COUNT_KEY,
                          steeralgCmnElementDefaultTable);
    if (steeralgState.config.maxSteeringTargetCount > STEEREXEC_MAX_CANDIDATES ||
        steeralgState.config.maxSteeringTargetCount < 1) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: Invalid %s provided: %u (must be in range [1,%u]), "
             "will use maximum allowed value: %u",
             __func__, STEERALG_MAX_STEERING_TARGET_COUNT_KEY,
             steeralgState.config.maxSteeringTargetCount,
             STEEREXEC_MAX_CANDIDATES, STEEREXEC_MAX_CANDIDATES);
        steeralgState.config.maxSteeringTargetCount = STEEREXEC_MAX_CANDIDATES;
    }

    steeralgState.config.applyEstimatedAirTimeOnSteering =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_APPLY_ESTIMATED_AIRTIME_ON_STEERING_KEY,
                          steeralgCmnElementDefaultTable);

    steeralgState.config.usePathCapToSelectBSS =
        profileGetOptsInt(mdModuleID_SteerAlg,
                          STEERALG_USE_PATH_CAP_TO_SELECT_BSS_KEY,
                          steeralgCmnElementDefaultTable);

    mdListenTableRegister(mdModuleID_BandMon, bandmon_event_overload_change,
                          steeralgCmnHandleOverloadChangeEvent);
    mdListenTableRegister(mdModuleID_BandMon, bandmon_event_utilization_update,
                          steeralgCmnHandleUtilizationUpdateEvent);
    mdListenTableRegister(mdModuleID_Estimator,
                          estimator_event_staDataMetricsComplete,
                          steeralgCmnHandleSTAMetricsCompleteEvent);
    mdListenTableRegister(mdModuleID_Estimator, estimator_event_perSTAAirtimeComplete,
                          steeralgCmnHandlePerSTAAirtimeCompleteEvent);
    mdListenTableRegister(mdModuleID_BandMon, bandmon_event_num_active_channels_change,
                          steeralgCmnHandleNumActiveChannelsChangeEvent);

    steeralgCmnResetServedChannels();

    steeralgMenuInit();
    return LBD_OK;
}

LBD_STATUS steeralg_fini(void) {
    steeralgCmnFinishOffloading(LBD_FALSE /* requestOneShotUtil */);
    return LBD_OK;
}

LBD_STATUS steeralg_steerIdleClient(stadbEntry_handle_t entry) {
    if (!entry || !stadbEntry_getServingBSS(entry, NULL)) {
        // Ignore disassociated STA
        return LBD_NOK;
    }

    steeralgCmnServingBSSInfo_t servingBSS;

    size_t maxNumBSS = steeralgState.config.maxSteeringTargetCount;
    lbd_bssInfo_t bss[STEEREXEC_MAX_CANDIDATES] = {{0}};
    if (LBD_NOK == steeralgCmnFindCandidatesForIdleClient(entry, &maxNumBSS,
                                                          &servingBSS, bss) ||
        !maxNumBSS) {
        return LBD_NOK;
    }
    lbDbgAssertExit(steeralgState.dbgModule,
                    maxNumBSS <= steeralgState.config.maxSteeringTargetCount);

    // Determine the reason for the steer decision
    steerexec_reason_e reason;
    if (servingBSS.isOverloaded) {
        reason = steerexec_reason_idleOffload;
    } else if (servingBSS.isPolluted) {
        reason = steerexec_reason_interferenceAvoidance;
    } else if (servingBSS.band == wlanif_band_24g) {
        reason = steerexec_reason_idleUpgrade;
    } else {
        reason = steerexec_reason_idleDowngrade;
    }

    lbDbgAssertExit(steeralgState.dbgModule,
                    maxNumBSS <= steeralgState.config.maxSteeringTargetCount);
    return steeralgCmnDoSteering(entry, maxNumBSS, bss, reason);
}

LBD_STATUS steeralg_select11kChannel(stadbEntry_handle_t entry,
                                          steerexec_reason_e trigger,
                                          uint8_t *maxNumSelectedChannels,
                                          lbd_channelId_t *dot11kChannel,
                                          uint16_t *dot11kFreq) {

    if (!entry) {
        return LBD_NOK;
    }

    const struct ether_addr *staAddr = stadbEntry_getAddr(entry);
    lbDbgAssertExit(steeralgState.dbgModule, staAddr);
    steeralgCmnServingBSSInfo_t servingBSS = {0};
    servingBSS.stats = stadbEntry_getServingBSS(entry, NULL);
    lbDbgAssertExit(steeralgState.dbgModule, servingBSS.stats);
    servingBSS.bssInfo = stadbEntry_resolveBSSInfo(servingBSS.stats);
    lbDbgAssertExit(steeralgState.dbgModule, servingBSS.bssInfo);
    servingBSS.band =
        wlanif_resolveBandFromFreq(servingBSS.bssInfo->freq);
    lbDbgAssertExit(steeralgState.dbgModule,
                    servingBSS.band != wlanif_band_invalid);
    servingBSS.bestPHYMode = stadbEntry_getBestPHYMode(entry);
    lbDbgAssertExit(steeralgState.dbgModule,
                    servingBSS.bestPHYMode != wlanif_phymode_invalid);
    servingBSS.trigger = trigger;

    // Only need to pick the best channel for 11k measurement
    size_t maxNumBSS = 1;
    lbd_bssInfo_t selectedBSS = {0};

    if (LBD_NOK == stadbEntry_iterateBSSStats(
                       entry, steeralgSelect11kChannelCallback,
                       &servingBSS, &maxNumBSS, &selectedBSS)) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: Failed to iterate BSS info for "lbMACAddFmt(":"),
             __func__, lbMACAddData(staAddr->ether_addr_octet));
        return LBD_NOK;
    } else if (maxNumBSS == 0) {
        dbgf(steeralgState.dbgModule, DBGDEBUG,
             "%s: No BSS candidate for 802.11k measurement for "lbMACAddFmt(":"),
             __func__, lbMACAddData(staAddr->ether_addr_octet));
        return LBD_NOK;
    }
    dot11kChannel[0] = selectedBSS.channelId;
    dot11kFreq[0]=selectedBSS.freq;
    *maxNumSelectedChannels = maxNumBSS;

    if (steeralgState.config.multi11krequest) {
        u_int8_t validchannellist = 0;
        if (LBD_NOK == steeralgSelect11kMultiChannel(
                        entry, dot11kChannel, &validchannellist) ||
                            (validchannellist == 0)) {
            dbgf(steeralgState.dbgModule, DBGERR,
                "%s: Failed to iterate BSS info for "lbMACAddFmt(":"),
                __func__, lbMACAddData(staAddr->ether_addr_octet));
        } else {
            *maxNumSelectedChannels = validchannellist;
        }

        u_int8_t i = 0;
        for (i = 0; i < *maxNumSelectedChannels; i++) {
            dbgf(steeralgState.dbgModule, DBGDEBUG,
                "%s: Selected 11k channel %d", __func__, dot11kChannel[i]);
        }
    }
    return LBD_OK;
}

steeralg_rateSteerEligibility_e steeralg_determineRateSteerEligibility(
    stadbEntry_handle_t handle,
    lbd_linkCapacity_t txRate,
    wlanif_band_e band) {
    // Never attempt to do rate based steering with a rate of 0
    if (!txRate) {
        return steeralg_rateSteer_none;
    }

    u_int8_t clientClassGroup = 0;
    if (LBD_NOK == stadbEntry_getClientClassGroup(handle, &clientClassGroup)) {
        dbgf(steeralgState.dbgModule, DBGERR,
                         "%s: Failed to resolve operating class", __func__);
        return steeralg_rateSteer_none;
    }
    if ( stadbEntry_isBandSupported(handle, wlanif_band_6g) ){
        if ( band == wlanif_band_6g && txRate < steeralgState.config.lowTxRateThreshold) {
            return steeralg_rateSteer_downgrade;
        } else if ( (band == wlanif_band_5g || band == wlanif_band_24g ) &&
                    (txRate > steeralgState.config.highTxRateThreshold[clientClassGroup]) ) {
            return steeralg_rateSteer_upgrade;
        } else {
            return steeralg_rateSteer_none;
        }
    }
    else if ((band == wlanif_band_5g) &&
        (txRate < steeralgState.config.lowTxRateThreshold)) {
        // Eligible for downgrade.
        return steeralg_rateSteer_downgrade;
    } else if ((band == wlanif_band_24g) &&
               (txRate > steeralgState.config.highTxRateThreshold[clientClassGroup])) {
        // Eligible for upgrade.
        return steeralg_rateSteer_upgrade;
    } else {
        // Rate is neither sufficient for upgrade or downgrade.
        return steeralg_rateSteer_none;
    }
}

/********************************************************************
 * Internal functions
 ********************************************************************/
/**
 * @brief Get a timestamp in seconds for use in delta computations.
 *
 * @return the current time in seconds
 */
static time_t steeralgCmnGetTimestamp(void) {
    struct timespec ts = {0};
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec;
}

/**
 * @brief Callback function to check if a BSS is a candidate for
 *        active steering
 *
 * @pre the STA entry is associated
 *
 * @param [in] entry  the STA entry
 * @param [in] bssHandle  the BSS handle to check
 * @param [in] cookie  pointer to steeralgCmnServingBSSInfo_t
 *
 * @return LBD_TRUE if the BSS is a steering candidate; otherwise return LBD_FALSE
 */
static u_int32_t steeralgCmnActiveSteerCallback(stadbEntry_handle_t entry,
                                                stadbEntry_bssStatsHandle_t bssHandle,
                                                void *cookie) {
    lbDbgAssertExit(steeralgState.dbgModule, cookie);
    steeralgCmnServingBSSInfo_t *servingBSS = (steeralgCmnServingBSSInfo_t *)cookie;
    const struct ether_addr *staAddr = stadbEntry_getAddr(entry);
    lbDbgAssertExit(steeralgState.dbgModule, staAddr);

    if (servingBSS->stats == bssHandle) {
        // Ignore current BSS
        return 0;
    }

    const lbd_bssInfo_t *servingBSSInfo = stadbEntry_resolveBSSInfo(servingBSS->stats);
    lbDbgAssertExit(steeralgState.dbgModule, servingBSSInfo);
    const lbd_bssInfo_t *bssInfo = stadbEntry_resolveBSSInfo(bssHandle);
    lbDbgAssertExit(steeralgState.dbgModule, bssInfo);
    if (bssInfo->apId != servingBSSInfo->apId) {
        // Ignore BSS not on serving AP
        return 0;
    }

    if (lbIsBSSLocal(bssInfo) &&
           LBD_NOK == wlanif_getIfaceStatus(bssInfo)) {
       // Ignore BSS whose interface is down
       return 0;
    }

    LBD_BOOL polluted = LBD_FALSE;
    if (LBD_NOK == stadbEntry_getPolluted(entry, bssHandle, &polluted, NULL) || polluted) {
        // Ignore all polluted BSS
        dbgf(steeralgState.dbgModule, DBGDUMP,
             "%s: " lbBSSInfoAddFmt() " is not a candidate for "
             lbMACAddFmt(":") " due to polluted",
             __func__, lbBSSInfoAddData(bssInfo), lbMACAddData(staAddr));
        return 0;
    }

    wlanif_band_e targetBand = wlanif_resolveBandFromFreq(bssInfo->freq);

    LBD_BOOL isCandidate = LBD_FALSE;
    lbd_airtime_t availableAirtime = LBD_INVALID_AIRTIME;

    time_t deltaSecs = 0xFFFFFFFF;
    lbd_linkCapacity_t dlEstimate;
    stadbEntry_getFullCapacities(entry, bssHandle, NULL /*ulCapacity*/, &dlEstimate,
            NULL /*deltaUlSecs*/, &deltaSecs);

    if ( stadbEntry_isBandSupported(entry, wlanif_band_6g) ) {
        if (servingBSS->band == wlanif_band_6g) {
            if ((servingBSS->rateSteerEligibility == steeralg_rateSteer_downgrade) ||
                (servingBSS->rateSteerEligibility == steeralg_rateSteer_downgradeRSSI)) {
                if ((targetBand == wlanif_band_24g || targetBand == wlanif_band_5g) &&
                     dlEstimate != LBD_INVALID_LINK_CAP &&
                     deltaSecs <= steeralgState.config.freshnessLimit &&
                     steeralgCmnCanBSSSupportClient(entry, bssHandle, bssInfo,
                                               LBD_TRUE /* isActive */, &availableAirtime)) {
                    isCandidate = LBD_TRUE;
                }
            } else {
                lbDbgAssertExit(steeralgState.dbgModule,
                            servingBSS->isOverloaded || servingBSS->isPolluted);
                if (dlEstimate != LBD_INVALID_LINK_CAP) {
                    isCandidate = steeralgCmnIsCandidateForOffloadingSTA(
                                   entry, bssHandle, bssInfo, dlEstimate,
                                   deltaSecs, &availableAirtime);
                }
            }
        }
        else {
            if ( (servingBSS->isOverloaded || servingBSS->isPolluted) &&
                  dlEstimate != LBD_INVALID_LINK_CAP) {
                isCandidate = steeralgCmnIsCandidateForOffloadingSTA(
                               entry, bssHandle, bssInfo, dlEstimate,
                               deltaSecs, &availableAirtime);
            } else {
                lbDbgAssertExit(steeralgState.dbgModule,
                            servingBSS->rateSteerEligibility == steeralg_rateSteer_upgrade);
                if (targetBand == wlanif_band_6g && dlEstimate != LBD_INVALID_LINK_CAP &&
                    steeralgCmnCanBSSSupportClient(entry, bssHandle, bssInfo,
                                               LBD_TRUE /* isActive */, &availableAirtime) &&
                    steeralgCmnIsRateAboveThreshold(entry, bssHandle, bssInfo, servingBSS->dlRate,
                                                dlEstimate, deltaSecs)) {
                    isCandidate = LBD_TRUE;
                }
            }
        }
    } else {
        if (servingBSS->band == wlanif_band_5g) {
            // For a 5 GHz serving BSS, it shall be either for downgrade or for offloading
            if ((servingBSS->rateSteerEligibility == steeralg_rateSteer_downgrade) ||
                (servingBSS->rateSteerEligibility == steeralg_rateSteer_downgradeRSSI)) {
                // Downgrade operation: Must be 2.4GHz BSS that can accommodate the traffic
                if (targetBand == wlanif_band_24g && dlEstimate != LBD_INVALID_LINK_CAP &&
                    deltaSecs <= steeralgState.config.freshnessLimit &&
                    steeralgCmnCanBSSSupportClient(entry, bssHandle, bssInfo,
                                                   LBD_TRUE /* isActive */, &availableAirtime)) {
                    isCandidate = LBD_TRUE;
                }
            } else { // For offloading or IAS
                lbDbgAssertExit(steeralgState.dbgModule,
                                servingBSS->isOverloaded || servingBSS->isPolluted);
                if (dlEstimate != LBD_INVALID_LINK_CAP) {
                    isCandidate = steeralgCmnIsCandidateForOffloadingSTA(
                                       entry, bssHandle, bssInfo, dlEstimate,
                                       deltaSecs, &availableAirtime);
                }
            }
        } else {
            // For a 2.4 GHz serving BSS, it shall be either for offloading/IAS or for upgrade
            if ( (servingBSS->isOverloaded || servingBSS->isPolluted)
                    && dlEstimate != LBD_INVALID_LINK_CAP) {
                isCandidate = steeralgCmnIsCandidateForOffloadingSTA(
                                   entry, bssHandle, bssInfo, dlEstimate,
                                   deltaSecs, &availableAirtime);
            } else {
                lbDbgAssertExit(steeralgState.dbgModule,
                                servingBSS->rateSteerEligibility == steeralg_rateSteer_upgrade);
                // Upgrade operation: target channel must be able to accommodate the traffic
                // of the client and offer higher MCS rate
                if (targetBand == wlanif_band_5g && dlEstimate != LBD_INVALID_LINK_CAP &&
                    steeralgCmnCanBSSSupportClient(entry, bssHandle, bssInfo,
                                                   LBD_TRUE /* isActive */, &availableAirtime) &&
                    steeralgCmnIsRateAboveThreshold(entry, bssHandle, bssInfo, servingBSS->dlRate,
                                                    dlEstimate, deltaSecs)) {
                    isCandidate = LBD_TRUE;
                }
            }
        }
    }
    if (isCandidate) {
        u_int32_t metric = steeralgCmnGetActiveClientMetric(entry, bssHandle, bssInfo,
                                                            dlEstimate, availableAirtime);
        dbgf(steeralgState.dbgModule, DBGDEBUG,
             "%s: BSS " lbBSSInfoAddFmt()
             " [metric 0x%x] added as an active steering candidate for "
             lbMACAddFmt(":"), __func__, lbBSSInfoAddData(bssInfo),
             metric, lbMACAddData(staAddr->ether_addr_octet));
        return metric;
    }

    // Not selected as an active steering candidate, the reason should have been printed
    return 0;
}

/**
 * @brief Check the rate and RSSI to determine if this STA is
 *        eligible for active steering
 *
 * @param [in] entry  STA entry to check
 * @param [in] addr MAC address of STA
 * @param [in] servingBSS info about STA association
 * @param [out] rssi  filled in with the uplink RSSI, or
 *                    LBD_INVALID_RSSI if it is not measured or
 *                    invalid
 *
 * @return enum code indicating if STA is eligible for upgrade,
 *         downgrade, or none.
 */
static steeralg_rateSteerEligibility_e steeralgCmnEligibleActiveSteerRateAndRSSI(
    stadbEntry_handle_t entry, const struct ether_addr *addr,
    steeralgCmnServingBSSInfo_t *servingBSS, lbd_rssi_t *rssi) {
    // Check the rate
    steeralg_rateSteerEligibility_e eligibility =
        steeralg_determineRateSteerEligibility(entry, servingBSS->dlRate, servingBSS->band);

    if (((servingBSS->band == wlanif_band_6g) &&
         (eligibility == steeralg_rateSteer_downgrade)) ||
        ((servingBSS->band == wlanif_band_5g) &&
         (eligibility == steeralg_rateSteer_downgrade)) ||
        ((servingBSS->band == wlanif_band_24g) &&
         (eligibility == steeralg_rateSteer_none))) {
        // For downgrade, need either rate or RSSI to be eligible
        // For upgrade, need rate and RSSI to be eligible
        // For either of these cases, don't need to check the RSSI as well
        *rssi = LBD_INVALID_RSSI;
        return eligibility;
    }

    // Check the RSSI
    time_t ageSecs = 0xFFFFFFFF;
    u_int8_t probeCount = 0;
    *rssi = stadbEntry_getUplinkRSSI(entry, servingBSS->stats,
                                     &ageSecs, &probeCount);
    if (*rssi == LBD_INVALID_RSSI ||
        ageSecs > steeralgState.config.freshnessLimit ||
        probeCount) {
        *rssi = LBD_INVALID_RSSI;
        // RSSI is either too old or invalid
        return steeralg_rateSteer_none;
    }

    u_int8_t clientClassGroup = 0;
    if (LBD_NOK == stadbEntry_getClientClassGroup(entry, &clientClassGroup)) {
        dbgf(steeralgState.dbgModule, DBGERR,
                         "%s: Failed to resolve operating class", __func__);
        return steeralg_rateSteer_none;
    }

    if (stadbEntry_isBandSupported(entry, wlanif_band_6g)){
        if ((servingBSS->band == wlanif_band_6g) &&
            (*rssi < steeralgState.config.lowRateRSSIXingThreshold)) {
            return steeralg_rateSteer_downgradeRSSI;
        } else if ((servingBSS->band == wlanif_band_24g || servingBSS->band == wlanif_band_5g) &&
               (*rssi > steeralgState.config.highRateRSSIXingThreshold[clientClassGroup])) {
           return steeralg_rateSteer_upgrade;
        } else {
            return steeralg_rateSteer_none;
        }
    } else {
        if ((servingBSS->band == wlanif_band_5g) &&
            (*rssi < steeralgState.config.lowRateRSSIXingThreshold)) {
            // Eligible for downgrade.
            return steeralg_rateSteer_downgradeRSSI;
        } else if ((servingBSS->band == wlanif_band_24g) &&
                   (*rssi > steeralgState.config.highRateRSSIXingThreshold[clientClassGroup])) {
            // Eligible for upgrade.
            return steeralg_rateSteer_upgrade;
        } else {
            // RSSI is neither sufficient for upgrade or downgrade.
            return steeralg_rateSteer_none;
        }
    }
}

/**
 * @brief Find candidate BSS(es) to steer for an active client
 *
 * @param [in] entry  the handle to the client
 * @param [in] staAddr MAC address of the client
 * @param [in] servingBSS set of parameters for the serving BSS
 * @param [inout] maxNumBSS  on input, it specifies maximum number of BSS info entries
 *                           allowed; on output, it returns the number of BSS info
 *                           entries populated on success
 * @param [out] bssCandidates  the BSSes that are eligible to steer to on success
 *
 * @return LBD_NOK if failed to iterate all BSSes; otherwise return LBD_OK
 */
static LBD_STATUS steeralgCmnFindCandidatesForActiveClient(
        stadbEntry_handle_t entry, const struct ether_addr *staAddr,
        steeralgCmnServingBSSInfo_t *servingBSS, size_t *maxNumBSS,
        lbd_bssInfo_t *bssCandidates) {
    if (LBD_NOK == stadbEntry_iterateBSSStats(
                       entry, steeralgCmnActiveSteerCallback,
                       servingBSS, maxNumBSS, bssCandidates)) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: Failed to iterate BSS info for "lbMACAddFmt(":"),
             __func__, lbMACAddData(staAddr->ether_addr_octet));
        return LBD_NOK;
    } else if (!(*maxNumBSS)) {
        dbgf(steeralgState.dbgModule, DBGDEBUG,
             "%s: No BSS candidate for rate based active steering for "lbMACAddFmt(":"),
             __func__, lbMACAddData(staAddr->ether_addr_octet));
    }

    return LBD_OK;
}

/**
 * @brief Get the metric used to evaluate active clients
 *
 * @pre At this point, the BSS has been selected as an active
 *      steering candidate, so it must have a valid available airtime.
 *      Since estimator will estimate DL rate info before estimating airtime,
 *      the DL rate info must also be valid at this point.
 *
 * The following rules are used to compute metric:
 * bit 31: set if STA has reserved airtime on the BSS
 * bit 0-30: the product of DL rate and available airtime
 *
 * @param [in] entry  STA entry to calculate metric for
 * @param [in] bssHandle  stats handle for the BSS
 * @param [in] bss  BSS entry to calculate metric for
 * @param [in] dlEstimate  estimated Tx rate
 * @param [in] availableAirtime  available airtime on the BSS
 *
 * @return Calculated metric
 */
static u_int32_t steeralgCmnGetActiveClientMetric(
        stadbEntry_handle_t entry, stadbEntry_bssStatsHandle_t bssHandle,
        const lbd_bssInfo_t *bss, lbd_linkCapacity_t dlEstimate,
        lbd_airtime_t availableAirtime) {
#define METRIC_OFFSET_RESERVED_AIRTIME 31
    lbDbgAssertExit(steeralgState.dbgModule, availableAirtime != LBD_INVALID_AIRTIME);
    lbDbgAssertExit(steeralgState.dbgModule, dlEstimate != LBD_INVALID_LINK_CAP);

    u_int32_t metric = dlEstimate * availableAirtime;
    // Since the DL estimate is in Mbps, there should be no chance for the MSB to be set.
    lbDbgAssertExit(steeralgState.dbgModule, metric >> METRIC_OFFSET_RESERVED_AIRTIME == 0);

    // Prefer BSS on which STA has reserved airtime
    if (stadbEntry_getReservedAirtime(entry, bssHandle) != LBD_INVALID_AIRTIME) {
        metric |= 1 << METRIC_OFFSET_RESERVED_AIRTIME;
    }

    return metric;
#undef METRIC_OFFSET_RESERVED_AIRTIME
}

/**
 * @brief Determine if a target BSS can be a steering candidate for
 *        STA during offloading
 *
 * Target channel must be able to accommodate the traffic of the client
 * and MCS higher than Th_L5 + Th_MinMCSIncrease if on 5 GHz.
 *
 * Note: To simplify offloading logic, we assume that if the Th_MinMCSIncrease is high
 * enough, the uplink RSSI must be greater than LowTxRateXingThreshold. So RSSI check
 * is skipped here.
 *
 * @param [in] entry  the handle to the STA to offload
 * @param [in] targetBSS  the handle to the target BSS to check
 * @param [in] bssInfo  basic info for target BSS
 * @param [in] dlEstimate  the estimated Tx rate
 * @param [in] deltaSecs  seconds since the Tx rate was estimated
 * @param [out] availableAirtime  available airtime on the BSS
 *
 * @return LBD_TRUE if the target BSS is a steering candidate for offloading;
 *         otherwise return LBD_FALSE
 */
static LBD_BOOL steeralgCmnIsCandidateForOffloadingSTA(
        stadbEntry_handle_t entry, stadbEntry_bssStatsHandle_t targetBSS,
        const lbd_bssInfo_t *bssInfo, lbd_linkCapacity_t dlEstimate,
        time_t deltaSecs, lbd_airtime_t *availableAirtime) {
    wlanif_band_e targetBand = wlanif_resolveBandFromFreq(bssInfo->freq);
    return (steeralgCmnCanBSSSupportClient(entry, targetBSS, bssInfo,
                                           LBD_TRUE /* isActive */, availableAirtime) &&
            (targetBand == wlanif_band_24g ||
             steeralgCmnIsRateAboveThreshold(entry, targetBSS, bssInfo,
                                             steeralgState.config.lowTxRateThreshold +
                                                 steeralgState.config.minTxRateIncreaseThreshold,
                                             dlEstimate, deltaSecs)));
}

/**
 * @brief Check the validity of the rate of a STA on a BSS and
 *        whether it is above certain threshold
 *
 * @param [in] entry  the handle to the STA
 * @param [in] bssHandle  the handle to the BSS
 * @param [in] bss  basic BSS information
 * @param [in] threshold  the threshold to compare with
 * @param [in] dlEstimate  the estimated Tx rate
 * @param [in] ageSecs  seconds since the Tx rate was estimated
 *
 * @return LBD_TRUE if the rate if above the threshold; otherwise return LBD_FALSE
 */
static LBD_BOOL steeralgCmnIsRateAboveThreshold(stadbEntry_handle_t entry,
                                                stadbEntry_bssStatsHandle_t bssHandle,
                                                const lbd_bssInfo_t *bss,
                                                lbd_linkCapacity_t threshold,
                                                lbd_linkCapacity_t dlEstimate,
                                                time_t ageSecs) {
    if (ageSecs > steeralgState.config.freshnessLimit) {
        // Capacity measurement is stale
        dbgf(steeralgState.dbgModule, DBGDEBUG,
             "%s: BSS " lbBSSInfoAddFmt()
             " not a steering candidate because capacity measurement is stale"
             " (age (%lu) > freshness limit (%u))",
             __func__, lbBSSInfoAddData(bss),
             ageSecs, steeralgState.config.freshnessLimit);
        return LBD_FALSE;
    } else if (dlEstimate == LBD_INVALID_LINK_CAP || dlEstimate <= threshold) {
         dbgf(steeralgState.dbgModule, DBGDEBUG,
             "%s: BSS " lbBSSInfoAddFmt() " not a steering candidate because "
             "link capacity (%u) not greater than threshold (%u)",
             __func__, lbBSSInfoAddData(bss), dlEstimate, threshold);
       return LBD_FALSE;
    }

    return LBD_TRUE;
}

/**
 * @brief Add projected airtime to all candidate BSSes when
 *        trying to steer a client
 *
 * @param [in] entry  the handle to the client
 * @param [in] allowAboveSafety  whether to allow the projected airtime
 *                               to go above the safety threshold
 * @param [in] candidateList  all candidate BSSes to add projected airtime
 * @param [in] maxNumBSS  number of BSSes in candidateList
 *
 * @return LBD_OK if estimated airtime has been added to all candidates successfully;
 *         otherwise return LBD_NOK
 */
LBD_STATUS steeralgCmnUpdateCandidateProjectedAirtime(
        stadbEntry_handle_t entry, LBD_BOOL allowAboveSafety,
        const lbd_bssInfo_t *candidateList, size_t maxNumBSS) {
    size_t i = 0;
    // Only allow projected airtime go above threshold for downgrade scenario
    for (i = 0; i < maxNumBSS; i++) {
        stadbEntry_bssStatsHandle_t bssHandle =
            stadbEntry_findMatchBSSStats(entry, &candidateList[i]);
        lbDbgAssertExit(steeralgState.dbgModule, bssHandle);

        lbd_airtime_t expectedAirtime = stadbEntry_getAirtime(entry, bssHandle, NULL);
        lbDbgAssertExit(steeralgState.dbgModule, expectedAirtime != LBD_INVALID_AIRTIME);

        if (LBD_NOK == bandmon_addProjectedAirtime(candidateList[i].channelId,
                                                   candidateList[i].freq,
                                                   expectedAirtime,
                                                   allowAboveSafety)) {
            return LBD_NOK;
        }
    }
    return LBD_OK;
}


/**
 * @brief Helper function to compute metric for idle steering candidate BSS
 *
 * The following rules are used to compute metric:
 * bit 0: always set for all candidates
 * bit 31: set if STA has reserved airtime on the BSS
 * bit 30: set if utilization below safety threshold
 * bit 29: set if 5 GHz channel
 * bit 28: set if a channel with higher Tx power for 11ac clients
 *         or if a channel with lower Tx power for non-11ac clients
 * bit 20-27: set with medium utilization measured on this channel
 *
 * @param [in] entry  the entry to find idle steering candidate for
 * @param [in] bssStats  the candidate BSS
 * @param [in] bestPHYMode  the best PHY mode the client supports
 *
 * @return the computed metric
 */
static u_int32_t steeralgCmnComputeIdleSteerMetric(
        stadbEntry_handle_t entry, stadbEntry_bssStatsHandle_t bssStats,
        wlanif_phymode_e bestPHYMode) {
#define METRIC_OFFSET_RESERVED_AIRTIME 31
#define METRIC_OFFSET_SAFETY (METRIC_OFFSET_RESERVED_AIRTIME - 1)
#define METRIC_OFFSET_BAND (METRIC_OFFSET_SAFETY - 1)
#define METRIC_OFFSET_PHY_CAP (METRIC_OFFSET_BAND - 1)
#define METRIC_OFFSET_UTIL (METRIC_OFFSET_PHY_CAP - sizeof(lbd_airtime_t) * 8)
    u_int32_t metric =  steeralgCmnComputeBSSMetric(entry, bssStats,
                                                    stadbEntry_isBandSupported(entry, wlanif_band_6g) ? wlanif_band_6g :
                                                                                                       wlanif_band_5g, // Always prefer 5 GHz
                                                    bestPHYMode,
                                                    METRIC_OFFSET_BAND,
                                                    METRIC_OFFSET_PHY_CAP,
                                                    METRIC_OFFSET_RESERVED_AIRTIME,
                                                    METRIC_OFFSET_SAFETY,
                                                    METRIC_OFFSET_UTIL);

    const struct ether_addr *staAddr = stadbEntry_getAddr(entry);
    lbDbgAssertExit(steeralgState.dbgModule, staAddr);
    const lbd_bssInfo_t *bssInfo = stadbEntry_resolveBSSInfo(bssStats);
    lbDbgAssertExit(steeralgState.dbgModule, bssInfo);
    dbgf(steeralgState.dbgModule, DBGDEBUG,
         "%s: " lbBSSInfoAddFmt() "is selected as idle steering candidate with metric 0x%x "
         "for " lbMACAddFmt(":"),
         __func__, lbBSSInfoAddData(bssInfo), metric, lbMACAddData(staAddr->ether_addr_octet));
    return metric;
#undef METRIC_OFFSET_RESERVED_AIRTIME
#undef METRIC_OFFSET_SAFETY
#undef METRIC_OFFSET_BAND
#undef METRIC_OFFSET_PHY_CAP
#undef METRIC_OFFSET_UTIL
}

/**
 * @brief Callback function to check if a BSS is a candidate for idle steering
 *
 * If a BSS is a candidate, it must return a non-zero metric.
 *
 * @pre the STA entry is associated
 *
 * @param [in] entry  the STA entry
 * @param [in] bssHandle  the BSS handle to check
 * @param [in] cookie  currently not used
 *
 * @return the non-zero metric if the BSS is a steering candidate;
 *         otherwise return 0
 */
static u_int32_t steeralgCmnIdleSteerCallback(stadbEntry_handle_t entry,
                                              stadbEntry_bssStatsHandle_t bssHandle,
                                              void *cookie) {
    lbDbgAssertExit(steeralgState.dbgModule, cookie);
    steeralgCmnServingBSSInfo_t *servingBSS = (steeralgCmnServingBSSInfo_t *)cookie;
    if (servingBSS->stats == bssHandle) {
        // Ignore current BSS
        return 0;
    }

    const lbd_bssInfo_t *servingBSSInfo = stadbEntry_resolveBSSInfo(servingBSS->stats);
    lbDbgAssertExit(steeralgState.dbgModule, servingBSSInfo);
    const lbd_bssInfo_t *bssInfo = stadbEntry_resolveBSSInfo(bssHandle);
    lbDbgAssertExit(steeralgState.dbgModule, bssInfo);
    if (bssInfo->apId != servingBSSInfo->apId) {
        // Ignore BSS not on serving AP
        return 0;
    }

    if (lbIsBSSLocal(bssInfo) &&
          LBD_NOK == wlanif_getIfaceStatus(bssInfo)) {
        //Ignore BSS whose interface is down
        return 0;
    }

    const struct ether_addr *addr = stadbEntry_getAddr(entry);
    lbDbgAssertExit(steeralgState.dbgModule, addr);

    LBD_BOOL isOverloaded = LBD_FALSE;
    if (LBD_NOK == bandmon_isChannelOverloaded(bssInfo->channelId, bssInfo->freq, &isOverloaded) ||
        isOverloaded) {
        // Ignore all overloaded channels
        dbgf(steeralgState.dbgModule, DBGDUMP,
             "%s: " lbBSSInfoAddFmt() " is not a candidate for "
             lbMACAddFmt(":") " due to overload",
             __func__, lbBSSInfoAddData(bssInfo), lbMACAddData(addr));
        return 0;
    }

    LBD_BOOL polluted = LBD_FALSE;
    if (LBD_NOK == stadbEntry_getPolluted(entry, bssHandle, &polluted, NULL) || polluted) {
        // Ignore all polluted BSS
        dbgf(steeralgState.dbgModule, DBGDUMP,
             "%s: " lbBSSInfoAddFmt() " is not a candidate for "
             lbMACAddFmt(":") " due to polluted",
             __func__, lbBSSInfoAddData(bssInfo), lbMACAddData(addr));
        return 0;
    }

    wlanif_band_e targetBand = wlanif_resolveBandFromFreq(bssInfo->freq);
    lbd_rssi_t rssi = LBD_INVALID_RSSI;
    LBD_BOOL isCandidate = LBD_FALSE;
    u_int8_t clientClassGroup = 0;

    if (LBD_NOK == stadbEntry_getClientClassGroup(entry, &clientClassGroup)) {
        dbgf(steeralgState.dbgModule, DBGERR,
                         "%s: Failed to resolve operating class", __func__);
        return 0;
    }

    u_int8_t inactRSSIXingThreshold =
                steeralgState.config.inactRSSIXingThreshold[targetBand][clientClassGroup];

    if (servingBSS->isOverloaded || servingBSS->isPolluted) {
        isCandidate = LBD_TRUE;
    } else if (stadbEntry_isBandSupported(entry, wlanif_band_6g)) {
        if ((servingBSS->band == wlanif_band_24g) &&
            (targetBand == wlanif_band_6g || targetBand == wlanif_band_5g)) {
            rssi = stadbEntry_getUplinkRSSI(entry, bssHandle, NULL, NULL);
            if (rssi != LBD_INVALID_RSSI &&
                rssi > inactRSSIXingThreshold) {
                isCandidate = LBD_TRUE;
            }
        } else if (servingBSS->band == wlanif_band_6g) {
            rssi = stadbEntry_getUplinkRSSI(entry, bssHandle, NULL, NULL);
            if (rssi != LBD_INVALID_RSSI) {
                if ((targetBand == wlanif_band_24g || targetBand == wlanif_band_5g) &&
                    (rssi < inactRSSIXingThreshold)) {
                    // Idle downgrade
                    isCandidate = LBD_TRUE;
                } else if (steeralgState.config.phyBasedPrioritization &&
                           targetBand == wlanif_band_6g &&
                           rssi > inactRSSIXingThreshold) {
                    LBD_BOOL isStrongestChannel = LBD_FALSE;
                    if (LBD_NOK == wlanif_isBSSOnStrongestChannel(
                                       bssInfo, &isStrongestChannel)) {
                        dbgf(steeralgState.dbgModule, DBGERR,
                             "%s: Failed to check Tx power status for " lbBSSInfoAddFmt(),
                             __func__, lbBSSInfoAddData(bssInfo));
                    } else {
                        // Idle upgrade 11ac capable client from weaker 5 GHz channel to
                        // the strongest one, or idle downgrade 11ac non-capable client
                        // from stronger 5 GHz channel to a weaker one
                        isCandidate = ((servingBSS->bestPHYMode == wlanif_phymode_he ||
                                           servingBSS->bestPHYMode == wlanif_phymode_vht) &&
                                           !servingBSS->isOnStrongest6G &&
                                           isStrongestChannel) ||
                                      ((servingBSS->bestPHYMode != wlanif_phymode_he &&
                                           servingBSS->bestPHYMode != wlanif_phymode_vht) &&
                                           servingBSS->isOnStrongest6G &&
                                           !isStrongestChannel);
                    }
                }
            }
        }
    } else { // Current channel is not overloaded
        if (servingBSS->band == wlanif_band_24g &&
            targetBand == wlanif_band_5g) {
            rssi = stadbEntry_getUplinkRSSI(entry, bssHandle, NULL, NULL);
            if (rssi != LBD_INVALID_RSSI &&
                rssi > inactRSSIXingThreshold) {
                isCandidate = LBD_TRUE;
            }
        } else if (servingBSS->band == wlanif_band_5g) {
            rssi = stadbEntry_getUplinkRSSI(entry, bssHandle, NULL, NULL);
            if (rssi != LBD_INVALID_RSSI) {
                if (targetBand == wlanif_band_24g &&
                    rssi < inactRSSIXingThreshold) {
                    // Idle downgrade
                    isCandidate = LBD_TRUE;
                } else if (steeralgState.config.phyBasedPrioritization &&
                           targetBand == wlanif_band_5g &&
                           rssi > inactRSSIXingThreshold) {
                    LBD_BOOL isStrongestChannel = LBD_FALSE;
                    if (LBD_NOK == wlanif_isBSSOnStrongestChannel(
                                       bssInfo, &isStrongestChannel)) {
                        dbgf(steeralgState.dbgModule, DBGERR,
                             "%s: Failed to check Tx power status for " lbBSSInfoAddFmt(),
                             __func__, lbBSSInfoAddData(bssInfo));
                    } else {
                        // Idle upgrade 11ac capable client from weaker 5 GHz channel to
                        // the strongest one, or idle downgrade 11ac non-capable client
                        // from stronger 5 GHz channel to a weaker one
                        isCandidate = ((servingBSS->bestPHYMode == wlanif_phymode_he ||
                                           servingBSS->bestPHYMode == wlanif_phymode_vht) &&
                                           !servingBSS->isOnStrongest5G &&
                                           isStrongestChannel) ||
                                      ((servingBSS->bestPHYMode != wlanif_phymode_he &&
                                           servingBSS->bestPHYMode != wlanif_phymode_vht) &&
                                           servingBSS->isOnStrongest5G &&
                                           !isStrongestChannel);
                    }
                }
            }
        }
    }

    if (isCandidate) {
        return steeralgCmnComputeIdleSteerMetric(entry, bssHandle, servingBSS->bestPHYMode);
    } else {
        dbgf(steeralgState.dbgModule, DBGDUMP,
             "%s: " lbBSSInfoAddFmt() " is not a candidate for "
             lbMACAddFmt(":") " due to RSSI %u",
             __func__, lbBSSInfoAddData(bssInfo), lbMACAddData(addr), rssi);
        return 0;
    }
}

/**
 * @brief Find candidate BSS(es) to steer for an idle client
 *
 * @param [in] entry  the handle to the idle client
 * @param [inout] maxNumBSS  on input, it specifies maximum number of BSS info entries
 *                           allowed; on output, it returns the number of BSS info
 *                           entries populated on success
 * @param [out] servingBSS  information about the serving BSS
 * @param [out] bssCandidates  the BSSes that are eligible to steer to on success
 *
 * @return LBD_NOK if failed to iterate all BSSes; otherwise return LBD_OK
 */
static LBD_STATUS steeralgCmnFindCandidatesForIdleClient(
        stadbEntry_handle_t entry, size_t *maxNumBSS,
        steeralgCmnServingBSSInfo_t *servingBSS,
        lbd_bssInfo_t *bssCandidates) {

    const struct ether_addr *staAddr = stadbEntry_getAddr(entry);
    lbDbgAssertExit(steeralgState.dbgModule, staAddr);

    servingBSS->stats = stadbEntry_getServingBSS(entry, NULL);
    lbDbgAssertExit(steeralgState.dbgModule, servingBSS->stats);
    servingBSS->bssInfo = stadbEntry_resolveBSSInfo(servingBSS->stats);
    lbDbgAssertExit(steeralgState.dbgModule, servingBSS->bssInfo);
    servingBSS->band =
        wlanif_resolveBandFromFreq(servingBSS->bssInfo->freq);
    lbDbgAssertExit(steeralgState.dbgModule,
                    servingBSS->band != wlanif_band_invalid);

    if (steeralgState.config.phyBasedPrioritization) {
        servingBSS->bestPHYMode = stadbEntry_getBestPHYMode(entry);
        lbDbgAssertExit(steeralgState.dbgModule,
                        servingBSS->bestPHYMode != wlanif_phymode_invalid);
        if (servingBSS->band == wlanif_band_5g) {
            // When it's associated on 5 GHz, check if it can be upgraded to a
            // stronger 5 GHz channel for 11ac capable clients or downgraded to
            // a weaker 5 GHz channel for 11ac non-capable clients.
            // We do not want to apply this logic on 2.4 GHz, since if the power becomes
            // weaker on 2.4 GHz, it will break some of our assumptions, e.g RSSI mapping
            // between 2.4 GHz and 5 GHz, and trigger more changes.
            if (LBD_NOK == wlanif_isBSSOnStrongestChannel(
                               servingBSS->bssInfo, &servingBSS->isOnStrongest5G)) {
                dbgf(steeralgState.dbgModule, DBGERR,
                     "%s: Failed to check Tx power status for channel %u",
                     __func__, servingBSS->bssInfo->channelId);
                return LBD_NOK;
            }
        } else if (servingBSS->band == wlanif_band_6g) {
            if (LBD_NOK == wlanif_isBSSOnStrongestChannel( servingBSS->bssInfo, &servingBSS->isOnStrongest6G)) {
                dbgf(steeralgState.dbgModule, DBGERR,
                     "%s: Failed to check Tx power status for channel %u",
                     __func__, servingBSS->bssInfo->channelId);
                return LBD_NOK;
            }
        }
    }

    if (LBD_NOK == bandmon_isChannelOverloaded(servingBSS->bssInfo->channelId,
                                               servingBSS->bssInfo->freq,
                                               &servingBSS->isOverloaded)) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: Failed to get overload status on channel %u for "
             lbMACAddFmt(":"),
             __func__, servingBSS->bssInfo->channelId,
             lbMACAddData(staAddr->ether_addr_octet));
        return LBD_NOK;
    }
    if (LBD_NOK == stadbEntry_getPolluted(entry, servingBSS->stats, &servingBSS->isPolluted, NULL)) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: Failed to get polluted state on " lbBSSInfoAddFmt() " for " lbMACAddFmt(":"),
             __func__, lbBSSInfoAddData(servingBSS->bssInfo),
             lbMACAddData(staAddr->ether_addr_octet));
        return LBD_NOK;
    }

    if (servingBSS->isOverloaded || servingBSS->isPolluted) {
        // Idle steering will only be triggered after RSSI on all BSSes are valid and
        // recent. So there is no need to check RSSI age here.
        u_int8_t clientClassGroup = 0;
        lbd_rssi_t rssi = stadbEntry_getUplinkRSSI(entry, servingBSS->stats, NULL, NULL);
        lbDbgAssertExit(steeralgState.dbgModule, rssi != LBD_INVALID_RSSI);
        stadbEntry_getClientClassGroup(entry, &clientClassGroup);

        if (rssi <= steeralgState.config.rssiSafetyThreshold[clientClassGroup]) {
            dbgf(steeralgState.dbgModule, DBGERR,
                 "%s: No BSS can be idle offloading or IAS candidate due to "
                 "serving channel RSSI not high enough (%u dB)",
                 __func__, rssi);
            return LBD_NOK;
        }
    }

    if (LBD_NOK == stadbEntry_iterateBSSStats(
                       entry, steeralgCmnIdleSteerCallback,
                       servingBSS, maxNumBSS, bssCandidates)) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: Failed to iterate BSS info for "lbMACAddFmt(":"),
             __func__, lbMACAddData(staAddr->ether_addr_octet));
        return LBD_NOK;
    } else if (*maxNumBSS == 0) {
        dbgf(steeralgState.dbgModule, DBGDEBUG,
             "%s: No BSS candidate for idle steering for "lbMACAddFmt(":"),
             __func__, lbMACAddData(staAddr->ether_addr_octet));
    }

    return LBD_OK;
}

/**
 * @brief React to the event indicating overload status change
 *
 * @param [in] event  the event received
 */
static void steeralgCmnHandleOverloadChangeEvent(struct mdEventNode *event) {
    const bandmon_overloadChangeEvent_t *overloadChangeEvent =
        (const bandmon_overloadChangeEvent_t *) event->Data;

    lbDbgAssertExit(steeralgState.dbgModule, overloadChangeEvent);

    steeralgCmnHandleUtilizationUpdate(overloadChangeEvent->numOverloadedChannels);
}

/**
 * @brief React to the event indicating utilization has been updated
 *
 * @param [in] event  the event received
 */
static void steeralgCmnHandleUtilizationUpdateEvent(struct mdEventNode *event) {
    const bandmon_utilizationUpdateEvent_t *utilizationUpdateEvent =
        (const bandmon_utilizationUpdateEvent_t *) event->Data;

    lbDbgAssertExit(steeralgState.dbgModule, utilizationUpdateEvent);

    steeralgCmnHandleUtilizationUpdate(utilizationUpdateEvent->numOverloadedChannels);
}

/**
 * @brief Handle utilization and overloaded channels update
 *
 * @param [in] numOverloadedChannels  number of overloaded channels reported
 *                                    by bandmon
 */
static void steeralgCmnHandleUtilizationUpdate(size_t numOverloadedChannels) {
    if (steeralgState.offloadState.inProgress) {
        // This is possibly due to 802.11k measurement take too long, stop offloading,
        // try on next utilization update.
        steeralgCmnFinishOffloading(LBD_TRUE /* requestOneShotUtil */);
    }

    lbd_apId_t apId = bandmon_getCurrentOffloadingAP();
    if (bandmon_isInSteeringBlackout(apId)) {
        // When T_settleDown timer is running, do nothing
        if (numOverloadedChannels) {
            // Request another util update so we can keep trying to offload
            bandmon_enableOneShotUtilizationEvent();
        }
        return;
    }

    if (!numOverloadedChannels) {
        steeralgCmnResetServedChannels();
    } else if (!steeralgState.offloadState.airtimeEstimatePending) {
        steeralgCmnOffloadOverloadedChannel(numOverloadedChannels, apId);
    }
}

/**
 * @brief React to the event indicating metric collection is
 *        complete for a single STA.
 *
 * @param [in] event  the event received
 */
static void steeralgCmnHandleSTAMetricsCompleteEvent(struct mdEventNode *event) {
    const estimator_staDataMetricsCompleteEvent_t *metricEvent =
        (const estimator_staDataMetricsCompleteEvent_t *)event->Data;

    lbDbgAssertExit(steeralgState.dbgModule, metricEvent);

    // Get the stadb entry for the event
    stadbEntry_handle_t entry = stadb_find(&metricEvent->addr);
    if (!entry) {
        // Unknown MAC address
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: Received STA metrics complete event from unknown MAC address: "
             lbMACAddFmt(":"),
             __func__, lbMACAddData(metricEvent->addr.ether_addr_octet));
        return;
    }

    lbd_airtime_t occupiedAirtime = LBD_INVALID_AIRTIME,
                  offloadedAirtime = LBD_INVALID_AIRTIME;
    LBD_BOOL offloadEntryComplete =
        steeralgCmnIsSTAMetricsTriggeredByOffloading(entry, &occupiedAirtime);

    LBD_BOOL isActive = LBD_FALSE;
    if (LBD_NOK == stadbEntry_getActStatus(entry, &isActive, NULL)) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: Received STA metrics for MAC address: " lbMACAddFmt(":")
             ", but failed to check activity status",
             __func__, lbMACAddData(metricEvent->addr.ether_addr_octet));
        return;
    }

    LBD_STATUS status = LBD_NOK;
    if (isActive) {
        status = steeralgHandleSTAMetricsForActiveClient(entry, metricEvent);
    } else {
        steeralgHandleSTAMetricsForIdleClient(entry, metricEvent);
        // for idle client, it no longer has traffic, so its occupied
        // airtime (if any) can be counted as being offloaded.
        status = LBD_OK;
    }

    if (status == LBD_OK) {
        offloadedAirtime = occupiedAirtime;
    }

    if (steeralgState.offloadState.inProgress) {
        steeralgCmnContinueSteerActiveClientsOverload(offloadedAirtime, offloadEntryComplete);
    }
}

/**
 * @brief Reset served channel list for offloading
 */
static void steeralgCmnResetServedChannels(void) {
    memset(steeralgState.servedChannels, 0,
           sizeof(steeralgState.servedChannels));
    size_t i;
    for (i = 0; i < WLANIF_MAX_RADIOS; ++i) {
        steeralgState.servedChannels[i].channelId = LBD_CHANNEL_INVALID;
    }
}

/**
 * @brief Auto-disable PHY based prioritization for 3 radio platform.
 *
 * NOTE: numActiveChannels represents all local and remote active.
 *       channels. This assumes the active channels are all on the
 *       same device and may incorrectly disable PHY based
 *       prioritization when remote-only channels are added except when called
 *       from steeralgCmnHandleNumActiveChannelsChangeEvent.
 *
 * @param [in] numActiveChannels  number of active channels
 */
static void steeralgCmnUpdatePHYBasedPrioritization(u_int8_t numActiveChannels) {
    if (numActiveChannels >= WLANIF_MAX_RADIOS &&
        steeralgState.config.phyBasedPrioritization) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: PHY based prioritization automatically disabled due to %u active "
             "channels ",
             __func__, numActiveChannels);
        steeralgState.config.phyBasedPrioritization = LBD_FALSE;
    }
}

/**
 * @brief React to an event indicating that the number of active channels
 *        is greater than 2 to disable phyBasedPrioritization
 *
 * @param [in] event the event received
 */
static void steeralgCmnHandleNumActiveChannelsChangeEvent(struct mdEventNode *event) {
    const bandmon_numActiveChannelsChangeEvent_t *numActiveChannelsChangeEvent =
        (bandmon_numActiveChannelsChangeEvent_t *)event->Data;
    lbDbgAssertExit(steeralgState.dbgModule, numActiveChannelsChangeEvent);

    steeralgState.numActiveChannels = numActiveChannelsChangeEvent->numActiveChannels;
    steeralgCmnUpdatePHYBasedPrioritization(
        numActiveChannelsChangeEvent->numActiveChannels);
}

/**
 * @brief Offload overloaded channels by steering qualified active clients
 *
 * @pre at least one channel is overloaded
 *
 * @param [in] numOverloadedChannels  number of overloaded channels
 * @param [in] apId  the AP on which to offload channels
 */
static void steeralgCmnOffloadOverloadedChannel(size_t numOverloadedChannels,
                                                lbd_apId_t apId) {
    size_t overloadedChannelCount = 0, numChannels = 0;
    lbd_channelId_t overloadedChannels[WLANIF_MAX_RADIOS];
    u_int16_t overloadedFreqs[WLANIF_MAX_RADIOS], overloadedSrcFreq;

    // Recheck active channels here in case channel changes, but total number of channels
    // should not change
    numChannels = bandmon_getNumActiveChannels();

    if (numChannels < numOverloadedChannels) {
        dbgf(steeralgState.dbgModule, DBGERR, "%s: Get %zu overload channels, but only %zu active channels",
             __func__, numOverloadedChannels, numChannels);
        return;
    }

    overloadedChannelCount =
      bandmon_getOverloadedChannelList(overloadedChannels, overloadedFreqs, WLANIF_MAX_RADIOS);

    if (overloadedChannelCount != numOverloadedChannels) {
        dbgf(steeralgState.dbgModule, DBGERR, "%s: Expect %zu overloaded channels, get %zu",
             __func__, numOverloadedChannels, overloadedChannelCount);
        return;
    }

    lbd_channelId_t overloadedSrcChannel = steeralgCmnSelectOverloadedChannel(
        apId, numOverloadedChannels, overloadedChannels, overloadedFreqs, &overloadedSrcFreq);

    if (overloadedSrcChannel == LBD_CHANNEL_INVALID) {
        dbgf(steeralgState.dbgModule, DBGINFO,
             "%s: Skipping offload for AP %u due to no steerable STAs on overloaded "
             "channel(s)", __func__, apId);
        return;
    }

    if (LBD_NOK == estimator_estimatePerSTAAirtimeOnChannel(apId,
                                                            overloadedSrcChannel,
                                                            overloadedSrcFreq)) {
        dbgf(steeralgState.dbgModule, DBGERR, "%s: Failed to request airtime estimate on channel %u",
             __func__, overloadedSrcChannel);
    } else {
        dbgf(steeralgState.dbgModule, DBGINFO, "%s: Requested airtime estimate on channel %u",
             __func__, overloadedSrcChannel);
        steeralgState.offloadState.airtimeEstimatePending = LBD_TRUE;
    }
}

/**
 * @brief STA iterator callback to determine if STA is steering eligble
 *
 * @param [in] entry  the STA DB entry to check if steering eligible
 * @param [in] cookie  the steerable flag
 */
static void steeralgCmnSteerableSTAIterateCB(stadbEntry_handle_t entry, void *cookie) {
    LBD_BOOL *hasSteerableSTAs = (LBD_BOOL *)cookie;
    if (stadbEntry_isDualBand(entry) && stadbEntry_isRRMSupported(entry)) {
        *hasSteerableSTAs = LBD_TRUE;
    }
}

/**
 * @brief Determine if there are any steerable STAs on given AP and channel
 *
 * @param [in] apId  AP the STAs must be associated to
 * @param [in] channelId  channel STA must be on
 *
 * @return LBD_TRUE when the given AP and channel has steerable STAs
 */
static LBD_BOOL steeralgCmnHasSteerableSTA(lbd_apId_t apId, lbd_channelId_t channelId) {
    LBD_BOOL hasSteerableSTAs = LBD_FALSE;
    if (LBD_NOK == stadb_iterateFilter(steeralgCmnSteerableSTAIterateCB, apId, channelId,
                                       &hasSteerableSTAs)) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: Failed to iterate station database. Default to no steerable STAs",
             __func__);
        return LBD_FALSE;

    }
    return hasSteerableSTAs;
}

/**
 * @brief Select one overloaded channel to do offloading
 *
 * Only attempt to offload a channel if the AP has steerable STAs
 * on that channel.
 *
 * @param [in] numChannels  number of channels in channelList
 * @param [in] channelList  list of overloaded channels
 *
 * @return The selected channel, or LBD_CHANNEL_INVALID if no
 * overloaded channels have steerable STAs
 */
static lbd_channelId_t steeralgCmnSelectOverloadedChannel(lbd_apId_t apId,
                                                          size_t numChannels,
                                                          const lbd_channelId_t *channelList,
                                                          const u_int16_t *freqList,
                                                          u_int16_t *pselectedFreq) {
    lbd_channelId_t selectedChannel = LBD_CHANNEL_INVALID;
    u_int16_t selectedFreq = LBD_FREQ_INVALID;
    if (numChannels == 1) {
        // When there is only one overloaded channel, select it if there
        // is a steerable STA
        if (steeralgCmnHasSteerableSTA(apId, channelList[0])) {
            selectedChannel = channelList[0];
            selectedFreq = freqList[0];
        }
    } else {
        // When there are multiple overloaded channel, select based on last
        // serving time and load.
        lbd_channelId_t notServedChannel = LBD_CHANNEL_INVALID,
                        oldestServedChannel = LBD_CHANNEL_INVALID;
        u_int16_t notServedFreq = LBD_FREQ_INVALID,
                  oldestServedFreq = LBD_FREQ_INVALID;
        lbd_airtime_t mostLoadedUtil = 0;
        size_t i;
        time_t oldestServedTime = LONG_MAX;

        for (i = 0; i < numChannels; ++i) {
            if (!steeralgCmnHasSteerableSTA(apId, channelList[i])) {
                // Skip channels with no steerable STAs
                continue;
            }
            time_t lastServedTime;
            if (!steeralgCmnIsChannelServedBefore(channelList[i], &lastServedTime)) {
                lbd_airtime_t util = bandmon_getMeasuredUtilization(channelList[i],
                                                                    freqList[i]);
                if (LBD_CHANNEL_INVALID == notServedChannel ||
                    mostLoadedUtil < util) {
                    notServedChannel = channelList[i];
                    notServedFreq = freqList[i];
                    mostLoadedUtil = util;
                }
            } else if ((LBD_CHANNEL_INVALID == oldestServedChannel) ||
                       (lastServedTime < oldestServedTime)) {
                oldestServedChannel = channelList[i];
                oldestServedFreq = freqList[i];
                oldestServedTime = lastServedTime;
            }
        }

        if (notServedChannel != LBD_CHANNEL_INVALID) {
            selectedChannel = notServedChannel;
            selectedFreq = notServedFreq;
        } else if (oldestServedChannel != LBD_CHANNEL_INVALID) {
            selectedChannel = oldestServedChannel;
            selectedFreq = oldestServedFreq;
        }
    }

    if (selectedChannel != LBD_CHANNEL_INVALID) {
        steeralgCmnAddChannelToServedChannels(selectedChannel);
    }
    *pselectedFreq = selectedFreq;
    return selectedChannel;
}

/**
 * @brief Check if a channel has been served before
 *
 * @param [in] channelId  the channel to check
 * @param [out] lastServedTime  the timestamp of when the channel
 *                              was served last time if any
 *
 * @return LBD_TRUE if the channel has been served before, otherwise return LBD_FALSE
 */
static LBD_BOOL steeralgCmnIsChannelServedBefore(lbd_channelId_t channelId,
                                                 time_t *lastServedTime) {
    size_t j;
    for (j = 0; j < WLANIF_MAX_RADIOS; ++j) {
        if (LBD_CHANNEL_INVALID == steeralgState.servedChannels[j].channelId) {
            continue;
        } else if (channelId == steeralgState.servedChannels[j].channelId) {
            *lastServedTime = steeralgState.servedChannels[j].lastServingTime;
            return LBD_TRUE;
        }
    }

    return LBD_FALSE;
}

/**
 * @brief Add selected overloaded channel to served channel list
 *
 * If the channel already exists in the list, update timestamp;
 * otherwise, add it to the list.
 *
 * @param [in] channelId  the channel to be added
 */
static void steeralgCmnAddChannelToServedChannels(lbd_channelId_t channelId) {
    size_t i, entryIdx = WLANIF_MAX_RADIOS;
    for (i = 0; i < WLANIF_MAX_RADIOS; ++i) {
        if (LBD_CHANNEL_INVALID == steeralgState.servedChannels[i].channelId) {
            // When there is an empty entry, remember it but still looking for a matching entry
            entryIdx = i;
        } else if (channelId == steeralgState.servedChannels[i].channelId) {
            // When there is a matching entry, do not look further
            entryIdx = i;
            break;
        }
    }

    lbDbgAssertExit(steeralgState.dbgModule, entryIdx < WLANIF_MAX_RADIOS);
    steeralgState.servedChannels[entryIdx].channelId = channelId;
    steeralgState.servedChannels[entryIdx].lastServingTime = steeralgCmnGetTimestamp();
}

/**
 * @brief React to the event indicating per STA airtime estimate is complete
 *
 * @param [in] event  the event received
 */
static void steeralgCmnHandlePerSTAAirtimeCompleteEvent(struct mdEventNode *event) {
    const estimator_perSTAAirtimeCompleteEvent_t *airtimeCompleteEvent =
        (const estimator_perSTAAirtimeCompleteEvent_t *) event->Data;

    lbDbgAssertExit(steeralgState.dbgModule, airtimeCompleteEvent);
    steeralgState.offloadState.airtimeEstimatePending = LBD_FALSE;

    if (airtimeCompleteEvent->channelId == LBD_CHANNEL_INVALID) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: Received perSTAAirtimeComplete event on invalid channel",
             __func__);
        return;
    }

    if (steeralgState.offloadState.inProgress) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: Unexpected perSTAAirtimeComplete event on channel %u "
             "due to offloading channel %u in progress",
             __func__, airtimeCompleteEvent->channelId,
             steeralgState.offloadState.channelId);
        return;
    } else if (!airtimeCompleteEvent->numSTAsEstimated) {
        dbgf(steeralgState.dbgModule, DBGINFO,
             "%s: Ignore perSTAAirtimeComplete event on channel %u "
             "due to no STA's airtime was estimated",
             __func__, airtimeCompleteEvent->channelId);
        // This should happen rarely, in the situation where the potential
        // candidate is still in 11k prohibit period. Request a new utilization
        // update to help find new steering opportunities
        steeralgCmnFinishOffloading(LBD_TRUE /* requestOneShotUtil */);
        return;
    }

    if (bandmon_isInSteeringBlackout(airtimeCompleteEvent->apId)) {
        dbgf(steeralgState.dbgModule, DBGINFO,
             "%s: Ignore perSTAAirtimeComplete event on channel %u "
             "due to steering blackout",
             __func__, airtimeCompleteEvent->channelId);
        return;
    }

    LBD_BOOL isOverloaded = LBD_FALSE;
    if (LBD_NOK == bandmon_isChannelOverloaded(airtimeCompleteEvent->channelId,
                                               airtimeCompleteEvent->freq,
                                               &isOverloaded) ||
        !isOverloaded) {
        dbgf(steeralgState.dbgModule, DBGDEBUG,
             "%s: Ignore perSTAAirtimeComplete event on channel %u "
             "due to channel not overloaded or overload status is unknown",
             __func__, airtimeCompleteEvent->channelId);
        return;
    }

    // It's possible that some client's airtime is not estimated but still recent,
    // give us some extra headroom to reduce the chance of memory reallocation
    size_t numCandidatesAllocated = airtimeCompleteEvent->numSTAsEstimated + 5;
    steeralgState.offloadState.candidateList =
        calloc(numCandidatesAllocated, sizeof(steeralgCmnOffloadingCandidateEntry_t));
    if (!steeralgState.offloadState.candidateList) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: Failed to allocate memory for offloading candidates",
             __func__);
        return;
    }

    do {
        steeralgCmnSelectOffloadingCandidatesParams_t params = {
            airtimeCompleteEvent->apId, airtimeCompleteEvent->channelId, airtimeCompleteEvent->freq,
            numCandidatesAllocated, LBD_FALSE /* hasStrongerChannel5G */
        };
        if (steeralgState.config.phyBasedPrioritization ) {
            if (wlanif_band_6g == wlanif_resolveBandFromFreq(
                                  airtimeCompleteEvent->freq)) {
                if (LBD_NOK == wlanif_isStrongestChannel(airtimeCompleteEvent->channelId,
                                                     &params.isStrongestChannel6G)) {
                    dbgf(steeralgState.dbgModule, DBGERR,
                         "%s: Failed to check Tx power status on channel %u",
                         __func__, airtimeCompleteEvent->channelId);
                    return;
                }
            }
            else if (wlanif_band_5g == wlanif_resolveBandFromFreq(
                                  airtimeCompleteEvent->freq)) {
                if (LBD_NOK == wlanif_isStrongestChannel(airtimeCompleteEvent->channelId,
                                                     &params.isStrongestChannel5G)) {
                    dbgf(steeralgState.dbgModule, DBGERR,
                         "%s: Failed to check Tx power status on channel %u",
                         __func__, airtimeCompleteEvent->channelId);
                    return;
                }
            }
        }
        if (LBD_NOK == stadb_iterate(steeralgCmnSelectOffloadingCandidateCB, &params)) {
            dbgf(steeralgState.dbgModule, DBGERR, "%s: Failed to iterate over STA database",
                 __func__);
            break;
        }

        if (steeralgState.offloadState.numCandidates) {
            steeralgCmnSortOffloadCandidates();
            // Start offloading process
            steeralgState.offloadState.inProgress = LBD_TRUE;
            steeralgState.offloadState.channelId = airtimeCompleteEvent->channelId;
            steeralgState.offloadState.freq = airtimeCompleteEvent->freq;
            steeralgState.offloadState.headIdx = 0;
            // No offloaded airtime and pending data measurement at start
            steeralgCmnContinueSteerActiveClientsOverload(LBD_INVALID_AIRTIME,
                                                       LBD_TRUE /* lastComplete */);
        } else {
            dbgf(steeralgState.dbgModule, DBGDEBUG, "%s: No candidates to offload channel %u",
                 __func__, params.channelId);
            break;
        }

        return;
    } while(0);

    steeralgCmnFinishOffloading(LBD_TRUE /* requestOneShotUtil */);
}

/**
 * @brief Sort all offload candidates based on metrics in descending order
 */
static void steeralgCmnSortOffloadCandidates(void) {
    qsort(steeralgState.offloadState.candidateList,
          steeralgState.offloadState.numCandidates,
          sizeof(steeralgCmnOffloadingCandidateEntry_t),
          steeralgCmnCompareOffloadCandidates);
}

/**
 * @brief Helper function used by qsort() to compare two offload candidates
 *
 * The higher metric, the higher priority the candidate has.
 *
 * @param [in] candidate1  first candidate
 * @param [in] candidate2  second candidate
 *
 * @return the difference of metrics of two candidate STAs
 */
static int steeralgCmnCompareOffloadCandidates(const void *candidate1,
                                               const void *candidate2) {
    const steeralgCmnOffloadingCandidateEntry_t *entry1 =
        (const steeralgCmnOffloadingCandidateEntry_t *)candidate1;
    const steeralgCmnOffloadingCandidateEntry_t *entry2 =
        (const steeralgCmnOffloadingCandidateEntry_t *)candidate2;

    return entry2->metric - entry1->metric;
}

/**
 * @brief Callback function for iterating station database for offloading candidate
 *
 * Clients that are active steering eligible will be added to the candidate list
 * sorted by descending estimated airtime.
 *
 * @param [in] entry  current STA entry to check
 * @param [inout] cookie  @see steeralgCmnSelectOffloadingCandidatesParams_t for details
 */
static void steeralgCmnSelectOffloadingCandidateCB(stadbEntry_handle_t entry,
                                                   void *cookie) {
    steeralgCmnSelectOffloadingCandidatesParams_t *params =
        (steeralgCmnSelectOffloadingCandidatesParams_t *) cookie;
    lbDbgAssertExit(steeralgState.dbgModule, params);

    const struct ether_addr *staAddr = stadbEntry_getAddr(entry);
    lbDbgAssertExit(steeralgState.dbgModule, staAddr);

    stadbEntry_bssStatsHandle_t bssStats = stadbEntry_getServingBSS(entry, NULL);
    if (!bssStats) {
        dbgf(steeralgState.dbgModule, DBGDUMP,
             "%s: " lbMACAddFmt(":") " is not an offloading candidate for channel %u "
             "due to not associated",
             __func__, lbMACAddData(staAddr->ether_addr_octet), params->channelId);
        return;
    }

    const lbd_bssInfo_t *bss = stadbEntry_resolveBSSInfo(bssStats);
    lbDbgAssertExit(steeralgState.dbgModule, bss);
    if (bss->channelId != params->channelId || bss->apId != params->apId) {
        dbgf(steeralgState.dbgModule, DBGDUMP,
             "%s: " lbMACAddFmt(":") " is not an offloading candidate for channel %u "
             "due to associated on different channel [%u] or AP [%u]",
             __func__, lbMACAddData(staAddr->ether_addr_octet), params->channelId,
             bss->channelId, bss->apId);
        return;
    }

    LBD_BOOL isActive = LBD_FALSE;
    if (LBD_NOK == stadbEntry_getActStatus(entry, &isActive, NULL) ||
        !isActive) {
        dbgf(steeralgState.dbgModule, DBGDUMP,
             "%s: " lbMACAddFmt(":") " is not an offloading candidate for channel %u "
             "due to not active or activity status unknown",
             __func__, lbMACAddData(staAddr->ether_addr_octet), params->channelId);
        return;
    }

    if (steerexec_steerEligibility_active !=
            steerexec_determineSteeringEligibility(entry)) {
        dbgf(steeralgState.dbgModule, DBGDEBUG,
             "%s: " lbMACAddFmt(":") " is not an offloading candidate for channel %u "
             "due to not eligible for active steering",
             __func__, lbMACAddData(staAddr->ether_addr_octet), params->channelId);
        return;
    }

    time_t ageSecs = 0xFFFFFFFF;
    lbd_airtime_t airtime = stadbEntry_getAirtime(entry, bssStats, &ageSecs);
    if (LBD_INVALID_AIRTIME == airtime ||
        ageSecs > steeralgState.config.freshnessLimit) {
        dbgf(steeralgState.dbgModule, DBGDEBUG,
             "%s: " lbMACAddFmt(":") " is not an offloading candidate for channel %u "
             "due to invalid or old airtime ([%u%%] %lu seconds)",
             __func__, lbMACAddData(staAddr->ether_addr_octet), params->channelId,
             airtime, ageSecs);
        return;
    }

    steeralgCmnRecordOffloadingCandidate(entry, params, airtime);
}

/**
 * @brief Add a client to offloading candidate list
 *
 * It will resize the candidate array if there are more candidates than requested.
 *
 * @param [in] entry  the handle to the client
 * @param [inout] params  callback parameters containing offload candidates list selected
 * @param [in] airtime  the estimated airtime of this client
 */
static void steeralgCmnRecordOffloadingCandidate(
        stadbEntry_handle_t entry, steeralgCmnSelectOffloadingCandidatesParams_t *params,
        lbd_airtime_t airtime) {
    if (steeralgState.offloadState.numCandidates == params->numCandidatesAllocated) {
        // If there is no enough space, reallocate memory.
        // This will only happen if there are more than five clients
        // getting their airtime estimation through full data rate estimation
        // in last freshnessLimit (5) seconds. So the chance of this operation
        // should be very low.
        params->numCandidatesAllocated += 5;
        steeralgCmnOffloadingCandidateEntry_t *tempCandidateList =
            (steeralgCmnOffloadingCandidateEntry_t *)realloc(
                    steeralgState.offloadState.candidateList,
                    sizeof(lbd_bssInfo_t) * params->numCandidatesAllocated);
        if (!tempCandidateList) {
            dbgf(steeralgState.dbgModule, DBGERR,
                 "%s: Failed to reallocate memory for offload candidates",
                 __func__);
            params->numCandidatesAllocated -= 5;
            return;
        } else {
            steeralgState.offloadState.candidateList = tempCandidateList;
        }
    }

    const struct ether_addr *staAddr = stadbEntry_getAddr(entry);
    lbDbgAssertExit(steeralgState.dbgModule, staAddr);

    wlanif_band_e band = wlanif_resolveBandFromFreq(params->freq);
    lbDbgAssertExit(steeralgState.dbgModule, band != wlanif_band_invalid);

    steeralgCmnOffloadingCandidateEntry_t *curEntry =
        &steeralgState.offloadState.candidateList[steeralgState.offloadState.numCandidates];
    curEntry->entry = entry;
    curEntry->metric = airtime;
    if (steeralgState.config.phyBasedPrioritization) {
        wlanif_phymode_e bestPHYMode = stadbEntry_getBestPHYMode(entry);
        LBD_BOOL preferedCandidate = LBD_FALSE;
        switch (bestPHYMode) {
            case wlanif_phymode_basic:
            case wlanif_phymode_ht:
                // Prefer 11n and basic clients if offloading stronger
                // 5 GHz channel or 6 GHz channel
                if ( stadbEntry_isBandSupported(entry, wlanif_band_6g) ){
                    if (band == wlanif_band_6g && params->isStrongestChannel6G) {
                        preferedCandidate = 1;
                    }
                }
                else {
                    if ( band == wlanif_band_5g && params->isStrongestChannel5G) {
                        preferedCandidate = 1;
                    }
                }
                break;
            case wlanif_phymode_vht:
            case wlanif_phymode_he:
                // Prefer 11ac or 11ax clients if offloading 2.4 GHz or
                // weaker 5 GHz channel or 6G GHz channel if 6G band available
                if (stadbEntry_isBandSupported(entry, wlanif_band_6g)) {
                    preferedCandidate = (band == wlanif_band_24g) ||
                                        !params->isStrongestChannel6G;
                } else {
                    preferedCandidate = (band == wlanif_band_24g) ||
                                        !params->isStrongestChannel5G;
                }
                break;
            default:
                // This should never happen now.
                dbgf(steeralgState.dbgModule, DBGERR,
                     "%s: " lbMACAddFmt(":") " unknown PHY mode %u",
                     __func__, lbMACAddData(staAddr->ether_addr_octet), bestPHYMode);
                break;
        }
        curEntry->metric |=
            preferedCandidate ? 1 << STEERALG_OFFLOADING_CANDIDATE_PREFER_BIT : 0;
    }
    ++steeralgState.offloadState.numCandidates;

    dbgf(steeralgState.dbgModule, DBGDEBUG,
         "%s: " lbMACAddFmt(":") " [%u%%][metric 0x%04x] is an offloading candidate for channel %u",
         __func__, lbMACAddData(staAddr->ether_addr_octet), airtime,
         curEntry->metric, params->channelId);
}

/**
 * @brief Try to steer next active client listed in candidate list,
 *        until enough have been steered to mitigate overload condition
 *
 * @param [in] offloadedAirtime  the airtime offloaded from moving previous client
 * @param [in] lastComplete  whether offloading last entry has completed, if not,
 *                           it means there is a measurement pending, we may want to
 *                           avoid sending too many concurrent measurement requests
 */
static void steeralgCmnContinueSteerActiveClientsOverload(lbd_airtime_t offloadedAirtime,
                                                          LBD_BOOL lastComplete) {
    if (offloadedAirtime != LBD_INVALID_AIRTIME) {
        steeralgState.offloadState.totalAirtimeOffloaded += offloadedAirtime;
    }

    LBD_BOOL isBelow;
    if (LBD_NOK == bandmon_isExpectedBelowSafety(steeralgState.offloadState.channelId,
                                                 steeralgState.offloadState.freq,
                                                 steeralgState.offloadState.totalAirtimeOffloaded,
                                                 &isBelow)) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: Failed to check if the MU on channel %u is expected to go below safety threshold",
             __func__, steeralgState.offloadState.channelId);
        steeralgCmnFinishOffloading(LBD_TRUE /* requestOneShotUtil */);
        return;
    } else if (isBelow) {
        dbgf(steeralgState.dbgModule, DBGINFO,
             "%s: Enough clients have been steered from channel %u, stop offloading",
             __func__, steeralgState.offloadState.channelId);
        steeralgCmnFinishOffloading(LBD_TRUE /* requestOneShotUtil */);
        return;
    }

    if (!lastComplete) { return; }

    do {
        stadbEntry_handle_t entry = steeralgState.offloadState.candidateList[
                                        steeralgState.offloadState.headIdx].entry;
        if (entry &&
            LBD_OK == estimator_estimateSTADataMetrics(entry,
                                                       steerexec_reason_activeOffload)) {
            return;
        }

        // If entry current head is not valid (probably due to concurrent
        // MCS bases steering) or failed to request STA metrics, continue
        // to next entry if any
        ++steeralgState.offloadState.headIdx;
    } while (steeralgState.offloadState.headIdx < steeralgState.offloadState.numCandidates);

    //If it reaches here, there must be no more candidate, iterate over stadb to find out
    // all STAs who rejected the BTM Request made for Load Balancing

    // If it reaches here, there must be no more candidate, finish offloading
    steeralgCmnFinishOffloading(LBD_TRUE /* requestOneShotUtil */);
}

/**
 * @brief Check if a full data metric report is triggered by offloading
 *
 * If the entry is in offloading candidate list, the entry will be marked
 * as processed and its occupied airtime will be returned.
 *
 * @param [in] entry  the handle to the STA
 * @param [out] occupiedAirtime  the occupied airtime if the STA is an offloading
 *                               candidate, otherwise LBD_INVALID_AIRTIME
 *
 * @return LBD_TRUE if the STA is at the head of offloading candidate list;
 *         otherwise return LBD_FALSE
 */
static LBD_BOOL steeralgCmnIsSTAMetricsTriggeredByOffloading(
        stadbEntry_handle_t entry, lbd_airtime_t *occupiedAirtime) {
    *occupiedAirtime = LBD_INVALID_AIRTIME;
    LBD_BOOL complete = LBD_FALSE;
    if (!steeralgState.offloadState.inProgress) {
        return complete;
    }

    size_t entryIdx = steeralgState.offloadState.numCandidates;
    if (entry == steeralgState.offloadState.candidateList[
                     steeralgState.offloadState.headIdx].entry) {
        entryIdx = steeralgState.offloadState.headIdx;
        ++steeralgState.offloadState.headIdx;
        complete = LBD_TRUE;
    } else {
        for (entryIdx = steeralgState.offloadState.headIdx;
             entryIdx < steeralgState.offloadState.numCandidates; ++entryIdx) {
            if (steeralgState.offloadState.candidateList[entryIdx].entry == entry) {
                break;
            }
        }
    }

    if (entryIdx < steeralgState.offloadState.numCandidates) {
        // Mark the entry as being processed
        steeralgState.offloadState.candidateList[entryIdx].entry = NULL;
        *occupiedAirtime = steeralgState.offloadState.candidateList[entryIdx].metric &
                               STEERALG_OFFLOADING_CANDIDATE_AIRTIME_MASK;
    }

    return complete;
}

// ====================================================================
// Package level functions
// ====================================================================

LBD_BOOL steeralgCmnIsActiveUpgradeDowngrade(steerexec_reason_e reason) {
    switch (reason) {
        case steerexec_reason_activeUpgrade:
        case steerexec_reason_activeDowngradeRate:
        case steerexec_reason_activeDowngradeRSSI:
            return LBD_TRUE;

        default:
            return LBD_FALSE;
    }
}

LBD_BOOL steeralgCmnIsActiveSteer(steerexec_reason_e reason) {
    switch (reason) {
        case steerexec_reason_activeUpgrade:
        case steerexec_reason_activeDowngradeRate:
        case steerexec_reason_activeDowngradeRSSI:
        case steerexec_reason_activeOffload:
        case steerexec_reason_activeAPSteering:
        case steerexec_reason_interferenceAvoidance:
        case steerexec_reason_activeLegacyAPSteering:
            return LBD_TRUE;

        default:
            return LBD_FALSE;
    }
}

u_int32_t steeralgCmnComputeBSSMetric(stadbEntry_handle_t entry,
                                      stadbEntry_bssStatsHandle_t bssHandle,
                                      wlanif_band_e preferedBand,
                                      wlanif_phymode_e bestPHYMode,
                                      u_int32_t offsetBand,
                                      u_int32_t offsetPHYCap,
                                      u_int32_t offsetReservedAirtime,
                                      u_int32_t offsetSafety,
                                      u_int32_t offsetUtil) {
    wlanif_band_e supported_band;
    u_int32_t metric = 1; // Bit 0 is used to mark it as a candidate
    const lbd_bssInfo_t *bssInfo = stadbEntry_resolveBSSInfo(bssHandle);
    lbDbgAssertExit(steeralgState.dbgModule, bssInfo);
    wlanif_band_e band = wlanif_resolveBandFromFreq(bssInfo->freq);
    lbDbgAssertExit(steeralgState.dbgModule, band < wlanif_band_invalid);

    if (stadbEntry_getReservedAirtime(entry, bssHandle) != LBD_INVALID_AIRTIME) {
        metric |= 1 << offsetReservedAirtime;
    }
    if (bandmon_canSupportClient(bssInfo->channelId, bssInfo->freq, 0 /* airtime */)
            != LBD_INVALID_AIRTIME) {
        metric |= 1 << offsetSafety;
    }
    if (band == preferedBand) {
        metric |= 1 << offsetBand;
    }
    supported_band = stadbEntry_isBandSupported(entry, wlanif_band_6g) ? wlanif_band_6g :
                                                                         wlanif_band_5g;
    if ( steeralgState.config.phyBasedPrioritization ) {
        if (band == supported_band) {
            // PHY capability bit only matters for 5 GHz target channel
            LBD_BOOL isStrongestChannel = LBD_FALSE;
            if (LBD_NOK == wlanif_isBSSOnStrongestChannel(bssInfo, &isStrongestChannel)) {
                dbgf(steeralgState.dbgModule, DBGERR,
                     "%s: Failed to check Tx power status for " lbBSSInfoAddFmt(),
                     __func__, lbBSSInfoAddData(bssInfo));
            } else if (!(isStrongestChannel ^ (bestPHYMode == wlanif_phymode_vht ||
                                           bestPHYMode == wlanif_phymode_he))) {
                metric |= 1 << offsetPHYCap;
            }
        }
    }

    lbd_airtime_t util = bandmon_getMeasuredUtilization(bssInfo->channelId,
                                                        bssInfo->freq);
    if (util == LBD_INVALID_AIRTIME) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: Failed to get measured utilization on channel %u",
             __func__, bssInfo->channelId);
    } else {
        // The lower the utilization, the better
        metric |= (100 - util) << offsetUtil;
    }

    return metric;
}

void steeralgCmnFinishOffloading(LBD_BOOL requestOneShotUtil) {
    free(steeralgState.offloadState.candidateList);
    memset(&steeralgState.offloadState, 0, sizeof(steeralgState.offloadState));
    if (requestOneShotUtil) {
        bandmon_enableOneShotUtilizationEvent();
    }
}

LBD_STATUS steeralgCmnDoSteering(stadbEntry_handle_t entry, size_t numBSS,
                                 const lbd_bssInfo_t *bssCandidates,
                                 steerexec_reason_e reason) {
    LBD_STATUS result = LBD_NOK;
    LBD_BOOL ignored;
    const struct ether_addr *staAddr = stadbEntry_getAddr(entry);
    lbDbgAssertExit(steeralgState.dbgModule, staAddr);

    if (LBD_NOK == steerexec_steer(entry, numBSS, bssCandidates, reason,
                                   &ignored)) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: Failed to steer " lbMACAddFmt(":"),
              __func__, lbMACAddData(staAddr->ether_addr_octet));
    } else if (!ignored){
        char prefix[100];
        snprintf(prefix, sizeof(prefix), lbMACAddFmt(":") " is being steered to",
                 lbMACAddData(staAddr->ether_addr_octet));
        lbLogBSSInfoCandidates(steeralgState.dbgModule, DBGINFO,
                               __func__, prefix, numBSS, bssCandidates);
        result = LBD_OK;
    }

    return result;
}

LBD_STATUS steeralgCmnSteerActiveClient(stadbEntry_handle_t entry,
                                        const struct ether_addr *staAddr) {
    steeralgCmnServingBSSInfo_t servingBSS;

    servingBSS.stats = stadbEntry_getServingBSS(entry, NULL);
    if (!servingBSS.stats) {
        // Ignore disassociated STA
        return LBD_NOK;
    }

    servingBSS.bssInfo = stadbEntry_resolveBSSInfo(servingBSS.stats);
    lbDbgAssertExit(steeralgState.dbgModule, servingBSS.bssInfo);
    servingBSS.band =
        wlanif_resolveBandFromFreq(servingBSS.bssInfo->freq);
    lbDbgAssertExit(steeralgState.dbgModule, servingBSS.band != wlanif_band_invalid);

    if (LBD_NOK == bandmon_isChannelOverloaded(servingBSS.bssInfo->channelId,
                                               servingBSS.bssInfo->freq,
                                               &servingBSS.isOverloaded)) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: Failed to get overload status for serving channel %u",
             __func__, servingBSS.bssInfo->channelId);
        return LBD_NOK;
    }

    if (LBD_NOK == stadbEntry_getPolluted(entry, servingBSS.stats,
                                          &servingBSS.isPolluted, NULL)) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: Failed to get pollution state for serving BSS " lbBSSInfoAddFmt(),
             __func__, lbBSSInfoAddData(servingBSS.bssInfo));
        return LBD_NOK;
    }

    // Get the last measured rate.
    time_t deltaSecs;
    stadbEntry_getFullCapacities(entry, servingBSS.stats, NULL /*ulCapacity*/,
            &servingBSS.dlRate, NULL /*deltaUlSecs*/, &deltaSecs);
    if (servingBSS.dlRate == LBD_INVALID_LINK_CAP) {
        dbgf(steeralgState.dbgModule, DBGERR,
             "%s: Couldn't get Tx rate for MAC address: " lbMACAddFmt(":"),
             __func__, lbMACAddData(staAddr->ether_addr_octet));
        return LBD_NOK;
    }

    if (deltaSecs > steeralgState.config.freshnessLimit) {
        // Rate value is too old.
        dbgf(steeralgState.dbgModule, DBGINFO,
             "%s: Collected metrics for MAC address: " lbMACAddFmt(":")
              ", but Tx rate measurement is stale, will not steer",
             __func__, lbMACAddData(staAddr->ether_addr_octet));
        return LBD_NOK;
    }

    lbd_rssi_t rssi;
    servingBSS.rateSteerEligibility =
        steeralgCmnEligibleActiveSteerRateAndRSSI(entry, staAddr,
                                                  &servingBSS, &rssi);
    if (servingBSS.rateSteerEligibility == steeralg_rateSteer_none &&
        !servingBSS.isOverloaded && !servingBSS.isPolluted) {
        dbgf(steeralgState.dbgModule, DBGINFO,
             "%s: Collected metrics for MAC address: " lbMACAddFmt(":")
             ", but it is not a candidate for either offloading, IAS or MCS "
             "based steering (rate %u, rssi %u), will not steer",
             __func__, lbMACAddData(staAddr->ether_addr_octet),
             servingBSS.dlRate, rssi);
        return LBD_NOK;
    }

    if (bandmon_isInSteeringBlackout(servingBSS.bssInfo->apId)) {
        if (steeralgState.offloadState.inProgress) {
            steeralgCmnFinishOffloading(LBD_TRUE /* requestOneShotUtil */);
        }
        if ((servingBSS.rateSteerEligibility != steeralg_rateSteer_downgrade) &&
            (servingBSS.rateSteerEligibility != steeralg_rateSteer_downgradeRSSI) &&
            !servingBSS.isPolluted) {
            dbgf(steeralgState.dbgModule, DBGINFO,
                 "%s: Only allow downgrade and IAS operation while in blackout period",
                 __func__);
            return LBD_NOK;
        }
    }

    size_t maxNumBSS = steeralgState.config.maxSteeringTargetCount;
    lbd_bssInfo_t candidates[STEEREXEC_MAX_CANDIDATES];
    if (LBD_NOK ==
        steeralgCmnFindCandidatesForActiveClient(entry, staAddr,
                                                 &servingBSS, &maxNumBSS,
                                                 candidates) || !maxNumBSS) {
        return LBD_NOK;
    }
    lbDbgAssertExit(steeralgState.dbgModule,
                    maxNumBSS <= steeralgState.config.maxSteeringTargetCount);

    if (LBD_NOK == steeralgCmnUpdateCandidateProjectedAirtime(
                       entry, (servingBSS.rateSteerEligibility ==
                                   steeralg_rateSteer_downgrade ||
                               servingBSS.rateSteerEligibility ==
                                   steeralg_rateSteer_downgradeRSSI),
                       candidates, maxNumBSS)) {
        return LBD_NOK;
    }

    // Determine the reason for the steer decision
    steerexec_reason_e reason;
    if (servingBSS.rateSteerEligibility == steeralg_rateSteer_downgrade) {
        reason = steerexec_reason_activeDowngradeRate;
    } else if (servingBSS.rateSteerEligibility == steeralg_rateSteer_downgradeRSSI) {
        reason = steerexec_reason_activeDowngradeRSSI;
    } else if (servingBSS.rateSteerEligibility == steeralg_rateSteer_upgrade) {
        reason = steerexec_reason_activeUpgrade;
    } else if (servingBSS.isOverloaded) {
        reason = steerexec_reason_activeOffload;
    } else {
        reason = steerexec_reason_interferenceAvoidance;
    }

    if ((stadbEntryReadActiveLoadBalancingRejected(entry) == stadbEntryLoadBalancingStaState_markedForceSteer && reason != steerexec_reason_activeOffload) ||
       (stadbEntryReadActiveLoadBalancingRejected(entry) == stadbEntryLoadBalancingStaState_forceSteerComplete))
       {
           stadbEntryMarkActiveLoadBalancingRejected(entry, stadbEntryLoadBalancingStaState_initial);
       }

    if (LBD_NOK == steeralgCmnDoSteering(entry, maxNumBSS, candidates, reason)) {
        return LBD_NOK;
    }

    return LBD_OK;
}

/**
 * @brief Calculate the airtime change if moves to the target BSS.
 *
 * If the target channel is the same as the current channel,
 * the airtime change will be the delta between the projected airtime
 * and the current airtime; otherwise it will be the projected airtime.
 *
 * @param [in] entry  the handle to the STA
 * @param [in] bssHandle,  the target BSS stats handler
 * @param [out] deltaSecs,  seconds that have elapsed since last airtime estimation
 *
 * @return the expected airtime change;
 *         therwise return LBD_INVALID_AIRTIME
 */
static lbd_airtime_t steeralgCmnExpectedAirtimeChange(const stadbEntry_handle_t entry,
                                                      const stadbEntry_bssStatsHandle_t bssHandle,
                                                      time_t *deltaSecs) {
    lbd_airtime_t expectedAirtime = stadbEntry_getAirtime(entry, bssHandle, deltaSecs);
    const lbd_bssInfo_t *bssInfo = stadbEntry_resolveBSSInfo(bssHandle);
    lbDbgAssertExit(steeralgState.dbgModule, bssInfo);

    lbd_channelId_t  assocChannel = LBD_CHANNEL_INVALID;
    const stadbEntry_bssStatsHandle_t servingBSSHandle = stadbEntry_getServingBSS(entry,  NULL);
    const lbd_bssInfo_t *servingBssInfo = stadbEntry_resolveBSSInfo(servingBSSHandle);
    if(servingBssInfo) {
        assocChannel = servingBssInfo->channelId;
    }

    dbgf(steeralgState.dbgModule, DBGDEBUG, "%s: BSS: " lbBSSInfoAddFmt() ", airtime: %u, channel:%u",
         __func__, lbBSSInfoAddData(bssInfo), expectedAirtime, bssInfo->channelId);

    if(expectedAirtime != LBD_INVALID_AIRTIME &&
       assocChannel != LBD_CHANNEL_INVALID &&
       assocChannel == bssInfo->channelId) {
        lbd_airtime_t currentAirtime = stadbEntry_getAirtime(entry, servingBSSHandle, deltaSecs);

        dbgf(steeralgState.dbgModule, DBGDEBUG,
             "%s: Serving BSS" lbBSSInfoAddFmt() ", current channel:%u, current airtime: %u",
              __func__, lbBSSInfoAddData(servingBssInfo), assocChannel, currentAirtime);

        if(currentAirtime != LBD_INVALID_AIRTIME) {
            if(currentAirtime < expectedAirtime) {
                return expectedAirtime - currentAirtime;
            } else {
                return 0;
            }
        }
    }

    return expectedAirtime;
}

LBD_BOOL steeralgCmnCanBSSSupportClient(stadbEntry_handle_t entry,
                                        stadbEntry_bssStatsHandle_t bssHandle,
                                        const lbd_bssInfo_t *bss,
                                        LBD_BOOL isActive,
                                        lbd_airtime_t *availableAirtime) {
    *availableAirtime = LBD_INVALID_AIRTIME;

    LBD_BOOL isOverloaded = LBD_FALSE, canSupport = LBD_FALSE;
    if (LBD_NOK == bandmon_isChannelOverloaded(bss->channelId,
                                               bss->freq, &isOverloaded) ||
        isOverloaded) {
        dbgf(steeralgState.dbgModule, DBGDEBUG,
             "%s: BSS " lbBSSInfoAddFmt()
             " not a steering candidate because it is overloaded or overload status not found",
              __func__, lbBSSInfoAddData(bss));
        return LBD_FALSE;
    }

    lbd_airtime_t expectedAirtime = 0;
    if (isActive) {
        time_t deltaSecs = 0xFFFFFFFF;
        expectedAirtime = steeralgCmnExpectedAirtimeChange(entry, bssHandle, &deltaSecs);
        if (expectedAirtime == LBD_INVALID_AIRTIME ||
            deltaSecs > steeralgState.config.freshnessLimit) {
            // Couldn't estimate airtime
            dbgf(steeralgState.dbgModule, DBGDEBUG,
                 "%s: BSS " lbBSSInfoAddFmt()
                 " not a steering candidate because couldn't estimate airtime",
                  __func__, lbBSSInfoAddData(bss));
            return LBD_FALSE;
        }
    }

    *availableAirtime = bandmon_canSupportClient(bss->channelId,
                                                 bss->freq, expectedAirtime);
    canSupport = (*availableAirtime != LBD_INVALID_AIRTIME);
    if (!canSupport) {
        dbgf(steeralgState.dbgModule, DBGDEBUG,
             "%s: BSS " lbBSSInfoAddFmt()
             " not a steering candidate because cannot support client "
             "with expected airtime %u%%",
              __func__, lbBSSInfoAddData(bss), expectedAirtime);
    }

    return canSupport;
}

#ifdef LBD_DBG_MENU

static const char *steeralgMenuStatusHelp[] = {
    "s -- print steering algorithm status",
    NULL
};

#define MAC_ADDR_STR_LEN 25
#define AIRTIME_STR_LEN 20
#define PREFERED_STR_LEN 20

#ifndef GMOCK_UNIT_TESTS
static
#endif
void steeralgMenuStatusHandler(struct cmdContext *context, const char *cmd) {
    cmdf(context, "PHY based optimization: %s%s\n",
         steeralgState.config.phyBasedPrioritization ? "Enabled" : "Disabled",
         steeralgState.numActiveChannels >= WLANIF_MAX_RADIOS ?
             " (Always disabled for three-radio platform)" : "");
    cmdf(context, "Maximum number of steering target allowed: %u\n",
         steeralgState.config.maxSteeringTargetCount);
    if (steeralgState.offloadState.inProgress) {
        cmdf(context, "Offloading in progress on channel %u. %u clients to be offloaded:\n",
             steeralgState.offloadState.channelId,
             steeralgState.offloadState.numCandidates - steeralgState.offloadState.headIdx);
        cmdf(context, "\t%-*s%-*s%-*s\n", MAC_ADDR_STR_LEN, "MAC Address",
             AIRTIME_STR_LEN, "Occupied Airtime", PREFERED_STR_LEN, "Prefered PHY");
        size_t i;
        for (i = steeralgState.offloadState.headIdx;
             i < steeralgState.offloadState.numCandidates; ++i) {
            steeralgCmnOffloadingCandidateEntry_t *candidate =
                &steeralgState.offloadState.candidateList[i];
            const struct ether_addr *addr = stadbEntry_getAddr(candidate->entry);
            lbDbgAssertExit(steeralgState.dbgModule, addr);
            cmdf(context, "\t" lbMACAddFmt(":") "%*u%%%*s\n",
                 lbMACAddData(addr->ether_addr_octet), AIRTIME_STR_LEN - 1,
                 candidate->metric & STEERALG_OFFLOADING_CANDIDATE_AIRTIME_MASK,
                 PREFERED_STR_LEN - 3,
                 candidate->metric & (1 << STEERALG_OFFLOADING_CANDIDATE_PREFER_BIT) ?
                     "Yes" : "No");
        }
    }
}

static const char *steeralgMenuPathCapacityHelp[] = {
    "pc -- print the path capacity from a STA to CAP in debug mode",
    "Usage: pc <STA MAC, e.g. xx:xx:xx:xx:xx:xx> (<optional BSSID, e.g xx:xx:xx:xx:xx:xx>)"
    "  When BSSID is not specified, print current serving capacity",
    NULL};

static const struct cmdMenuItem steeralgMenu[] = {
    CMD_MENU_STANDARD_STUFF(),
    { "s", steeralgMenuStatusHandler, NULL, steeralgMenuStatusHelp },
    { "pc", steeralgMenuPathCapacityHandler, NULL, steeralgMenuPathCapacityHelp },
    CMD_MENU_END()
};

static const char *steeralgMenuHelp[] = {
    "steeralg -- Steering Algorithm",
    NULL
};

static const struct cmdMenuItem steeralgMenuItem = {
    "steeralg",
    cmdMenu,
    (struct cmdMenuItem *) steeralgMenu,
    steeralgMenuHelp
};

#endif /* LBD_DBG_MENU */

static void steeralgMenuInit(void) {
#ifdef LBD_DBG_MENU
    cmdMainMenuAdd(&steeralgMenuItem);
#endif /* LBD_DBG_MENU */
}
