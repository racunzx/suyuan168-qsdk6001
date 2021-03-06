// vim: set et sw=4 sts=4 cindent:
/*
 * @File: stadbEntry.h
 *
 * @Abstract: A single entry in the station database, corresponding to a known
 *            Wi-Fi STA
 *
 * @Notes:
 *
 * @@-COPYRIGHT-START-@@
 *
 * Copyright (c) 2014-2019 Qualcomm Technologies, Inc.
 *
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * 2014-2016 Qualcomm Atheros, Inc.
 *
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 *
 * @@-COPYRIGHT-END-@@
 */

#ifndef stadbEntry__h
#define stadbEntry__h

#include "lbd_types.h"  // for LBD_STATUS
#include "wlanif.h"  // for wlanif_band_e
#include "jansson.h" // for persistence

#if defined(__cplusplus)
extern "C" {
#endif
// opaque forward declaration
struct stadbEntryPriv_t;
struct stadbEntryPriv_bssStats_t;
typedef struct stadbEntryPriv_t *stadbEntry_handle_t;
typedef struct stadbEntryPriv_bssStats_t *stadbEntry_bssStatsHandle_t;

/**
 * @brief Enumeration for different entry types
 */
typedef enum stadbEntryLoadBalancingStaState_t {
    /// Initial State
    stadbEntryLoadBalancingStaState_initial,
    /// Marked for Forced Steer
    stadbEntryLoadBalancingStaState_markedForceSteer,
    /// Disassoc Imminent Sent to STA
    stadbEntryLoadBalancingStaState_disassocSent,
    /// Disassoc Timeout happened
    stadbEntryLoadBalancingStaState_disassocTimerExpired,
    /// Force Steer Complete
    stadbEntryLoadBalancingStaState_forceSteerComplete,

    stadbEntryLoadBalancingStaState_invalid
} stadbEntryLoadBalancingStaState_t;


// Maximum number of BSS stats per STA
#define STADB_ENTRY_MAX_BSS_STATS 3

/**
 * @brief Function to invoke to request that the internal state of a another
 *        module that is stored in a stadbEntry be destroyed or updated when
 *        the entry is being destroyed or reallocated, respectively.
 *
 * @param [in] handle  the entry being either destroyed (if NULL) or
 *                     reallocated (if non-NULL); when being reallocated, this
 *                     represents the new value
 * @param [in] state  the state corresponding to the entry being
 *                    managed
 */
typedef void (*stadbEntry_stateLifecycleCB_t)(stadbEntry_handle_t handle, void *state);

/**
 * @brief Obtain the MAC address for the provided station entry.
 *
 * @param [in] handle  the handle to the entry for which to obtain the address
 *
 * @return  the MAC address of the entry, or NULL if the entry is invalid
 */
const struct ether_addr *stadbEntry_getAddr(const stadbEntry_handle_t handle);

/**
 * @brief Determine if the provided entry matches the MAC address given.
 *
 * @param [in] handle  the handle to the entry to compare
 * @param [in] addr  the MAC address to compare to
 *
 * @return LBD_TRUE if the addresses match; otherwise LBD_FALSE
 */
LBD_BOOL stadbEntry_isMatchingAddr(const stadbEntry_handle_t handle,
                                   const struct ether_addr *addr);

/**
 * @brief Determine whether the band provided is supported or not for the
 *        given STA.
 *
 * @param [in] handle  the handle to the entry to check
 * @param [in] band  the band for which to check
 *
 * @return LBD_TRUE if the band is supported; otherwise LBD_FALSE
 */
LBD_BOOL stadbEntry_isBandSupported(const stadbEntry_handle_t handle,
                                    wlanif_band_e band);

/**
 * @brief Mark loadbalancingAttemptedButRejected field for this entry as TRUE
 *
 * This private function will be used to mark the STA entries for
 * loadbalancingAttemptedButRejected
 *
 * @param [in] entry the STA entry to set loadbalancingAttemptedButRejected
*/
LBD_STATUS stadbEntryMarkActiveLoadBalancingRejected(stadbEntry_handle_t entry,
                                                     u_int8_t loadbalancingStaState);

/**
 * @brief Mark loadbalancingAttemptedButRejected field for this entry as False
 *
 * This private function will be used to unmark the STA entries for
 * loadbalancingAttemptedButRejected
 *
 * @param [in] entry the STA entry to reset loadbalancingAttemptedButRejected
*/
LBD_STATUS stadbEntryUnMarkActiveLoadBalancingRejected(stadbEntry_handle_t entry);

/**
 * @brief Access loadbalancingAttemptedButRejected field for this entry
 *
 * This private function will be used to read/access the STA entries for
 * loadbalancingAttemptedButRejected flag value
 *
 * @param [in] entry the STA entry to access the flag vaue for
*/
u_int8_t stadbEntryReadActiveLoadBalancingRejected(stadbEntry_handle_t entry);

/**
 * @brief Mark the STA entry given by the handle as steering disallowed
 *
 * @param [in] handle  the handle of the entry
 *
 * @return LBD_OK on success; LBD_NOK on failure
 */
LBD_STATUS stadbEntry_markSteeringDisallowed(stadbEntry_handle_t entry);

/**
 * @brief Obtain whether the STA is prohibited from steering under
 *        all conditions by debug CLI control
 *
 * @param [in] handle  the handle of the entry to query
 *
 * @return LBD_TRUE if steering is adminstratively disallowed for the STA;
 *         otherwise LBD_FALSE
 */
LBD_BOOL stadbEntry_isSteeringDisallowed(const stadbEntry_handle_t handle);

/**
 * @brief Record that the STA tracked by the handle is actually a backhaul
 *        interface on an AP.
 *
 * @param [in] handle  the handle of the entry
 *
 * @return LBD_OK on success; LBD_NOK on failure
 */
LBD_STATUS stadbEntry_markBackhaulSTA(stadbEntry_handle_t handle);

/**
 * @brief Obtain whether the STA given by the handle has been identified as
 *        a backhaul interface of an AP.
 *
 * @param [in] handle  the handle of the entry to query
 *
 * @return LBD_TRUE if the STA belongs to an AP; otherwise LBD_FALSE
 */
LBD_BOOL stadbEntry_isBackhaulSTA(const stadbEntry_handle_t handle);

/**
 * @brief Mark that the band provided is supported by the STA.
 *
 * @param [in] handle  the handle to update
 * @param [in] band  the band to mark as supported
 *
 * @return LBD_OK if successful; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_setBandSupported(const stadbEntry_handle_t handle,
                                       wlanif_band_e band);

/**
 * @brief Mark that the band provided is supported by the STA when the
 *        information comes from a remote hybrid device
 *
 * @pre The entry is associated locally
 *
 * It will populate local BSS entries on the given band as supported.
 *
 * @param [in] handle  the handle to update
 * @param [in] band  the band to mark as supported
 *
 * @return LBD_OK if successful; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_setRemoteBandSupported(const stadbEntry_handle_t handle,
                                             wlanif_band_e band);

/**
 * @brief Determine whether the entry provided supports both the 2.4 GHz
 *        and 5 GHz bands.
 *
 * @param [in] handle  the handle to the entry to check
 *
 * @return LBD_TRUE if both bands are supported; otherwise LBD_FALSE
 */
LBD_BOOL stadbEntry_isDualBand(const stadbEntry_handle_t handle);

/**
 * @brief Determine the band on which the device is associated, and
 *        optionally how long ago that occurred.
 *
 * @param [in] handle  the handle of the entry to query
 * @param [out] deltaSecs  the number of seconds that have elapsed since
 *                         the device last associated
 *
 * @return the band on which it is associated, or wlanif_band_invalid if the
 *         device is not currently associated or the handle is invalid
 */
wlanif_band_e stadbEntry_getAssociatedBand(const stadbEntry_handle_t handle,
                                           time_t *deltaSecs);

/**
 * @brief Determine if the device ever associated (and thus should be
 *        considered an in-network device).
 *
 * @param [in] handle  the handle of the entry to query
 *
 * @return LBD_TRUE if the device has been associated; otherwise LBD_FALSE
 */
LBD_BOOL stadbEntry_isInNetwork(const stadbEntry_handle_t handle);

/**
 * @brief Determine how old the entry is (where age is defined as the number
 *        of seconds since it was last updated).
 *
 * @param [in] handle  the handle for which to obtain the age
 * @param [out] ageSecs  the age of the entry, in seconds
 *
 * @return LBD_OK if the entry was found and the age is valid; otherwise
 *         LBD_NOK
 */
LBD_STATUS stadbEntry_getAge(const stadbEntry_handle_t handle, time_t *ageSecs);

/**
 * @brief Obtain the opaque handle stored previously as the steering state
 *        (if there was one).
 *
 * @param [in] handle  the handle of the entry for which to get the steering
 *                     state
 *
 * @return the steering state, or NULL if none has been stored (or the entry
 *         handle is invalid)
 */
void *stadbEntry_getSteeringState(stadbEntry_handle_t handle);

/**
 * @brief Store an opaque steering state pointer in the entry for later lookup.
 *
 * @param [in] handle  the handle of the entry for which to get the steering
 *                     state
 * @param [in] state  the state to store
 * @param [in] callback  the function to use when managing the lifecycle of
 *                       the entry
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_setSteeringState(
        stadbEntry_handle_t handle, void *state,
        stadbEntry_stateLifecycleCB_t callback);

/**
 * @brief Obtain the opaque handle stored previously as the estimator
 *        state (if there was one).
 *
 * @param [in] handle  the handle of the entry for which to get the estimator
 *                     state
 *
 * @return the estimator state, or NULL if none has been stored (or the entry
 *         handle is invalid)
 */
void *stadbEntry_getEstimatorState(stadbEntry_handle_t handle);

/**
 * @brief Store an opaque estimator state pointer in the entry for later lookup.
 *
 * @param [in] handle  the handle of the entry for which to get the estimator
 *                     state
 * @param [in] state  the state to store
 * @param [in] callback  the function to use when managing the lifecycle of
 *                       the entry
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_setEstimatorState(
        stadbEntry_handle_t handle, void *state,
        stadbEntry_stateLifecycleCB_t callback);

/**
 * @brief Obtain the opaque handle stored previously as the steering messaging
 *        state (if there was one).
 *
 * @param [in] handle  the handle of the entry for which to get the estimator
 *                     state
 *
 * @return the steering messaging state, or NULL if none has been stored (or
 *         the entry handle is invalid)
 */
void *stadbEntry_getSteerMsgState(stadbEntry_handle_t handle);

/**
 * @brief Obtain the opaque handle stored previously as the MAP Service
 *        state (if there was one).
 *
 * @param [in] handle  the handle of the entry for which to get the MAP
 *                     Service state
 *
 * @return the MAP Service state, or NULL if none has been stored (or
 *         the entry handle is invalid)
 */
void *stadbEntry_getMapServiceState(stadbEntry_handle_t handle);

/**
 * @brief Obtain the opaque handle stored previously as the monitoring
 *        state (if there was one).
 *
 * @param [in] handle  the handle of the entry for which to get the monitor
 *                     state
 *
 * @return the monitor state, or NULL if none has been stored (or
 *         the entry handle is invalid)
 */
void *stadbEntry_getMonitorState(stadbEntry_handle_t entry);

/**
 * @brief Store an opaque steering messaging state pointer in the entry for
 *        later lookup.
 *
 * @param [in] handle  the handle of the entry for which to set the steering
 *                     messaging state
 * @param [in] state  the state to store
 * @param [in] callback  the function to use when managing the lifecycle of
 *                       the entry
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_setSteerMsgState(
        stadbEntry_handle_t handle, void *state,
        stadbEntry_stateLifecycleCB_t callback);

/**
 * @brief Store an opaque MAP Service state pointer in the entry for
 *        later lookup.
 *
 * @param [in] handle  the handle of the entry for which to set the MAP
 *                     Service state
 * @param [in] state  the state to store
 * @param [in] callback  the function to use when managing the lifecycle of
 *                       the entry
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_setMapServiceState(
        stadbEntry_handle_t handle, void *state,
        stadbEntry_stateLifecycleCB_t callback);

/**
 * @brief Store an opaque monitoring state pointer in the entry for
 *        later lookup.
 *
 * @param [in] handle  the handle of the entry for which to set the monitoring
 *                     state
 * @param [in] state  the state to store
 * @param [in] callback  the function to use when managing the lifecycle of
 *                       the entry
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_setMonitorState(
        stadbEntry_handle_t handle, void *state,
        stadbEntry_stateLifecycleCB_t callback);

/**
 * @brief Get activity status of a STA
 *
 * @param [in] handle  the handle for which to check idle status
 * @param [out] active  on success this will contain the activity status of this STA
 * @param [out] deltaSecs  if non-NULL, on success this will contain the number
 *                         of seconds that have elapsed since the last time activity
 *                         status is recorded
 *
 * @return LBD_NOK if the parameters are invalid or the STA is not associated;
 *         otherwise LBD_OK
 */
LBD_STATUS stadbEntry_getActStatus(const stadbEntry_handle_t entry, LBD_BOOL *active, time_t *deltaSecs);

/**
 * @brief Set activity status of a STA
 *
 * @param [in] entry  the handle of the STA for which to set activity status
 * @param [in] active  activity status of this STA
 * @param [out] actChanged  flag to indicate whether activity status has changed from the
 *                          previous entry or not
 *
 * @return LBD_OK if the activity state of the STA is set successfully. Otherwise
 *         return LBD_NOK.
 */
LBD_STATUS stadbEntry_setActStatus(const stadbEntry_handle_t entry, LBD_BOOL active,
                                   LBD_BOOL *actChanged);

/**
 * @brief Return whether or not BTM is supported for the entry
 *
 * @param [in] handle entry to check for BTM support
 *
 * @return LBD_BOOL LBD_TRUE if BTM is supported, false
 *                  otherwise
 */
LBD_BOOL stadbEntry_isBTMSupported(const stadbEntry_handle_t handle);

/**
 * @brief Update whether the STA of the provided entry supports
 *        BSS Transition Management (BTM).
 *
 * @param [in] entry  the handle of the entry to update
 * @param [in] isBTMSupported  LBD_TRUE if BTM is supported
 * @param [out]  changed  set to LBD_TRUE if the BTM support
 *                        status changed
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_updateIsBTMSupported(stadbEntry_handle_t entry,
                                           LBD_BOOL isBTMSupported,
                                           LBD_BOOL *changed);

/**
 * @brief Return whether or not RRM is supported for the entry
 *
 * @param [in] handle entry to check for RRM support
 *
 * @return LBD_BOOL LBD_TRUE if RRM is supported, false
 *                  otherwise
 */
LBD_BOOL stadbEntry_isRRMSupported(const stadbEntry_handle_t handle);


/**
 * @brief Return RcpiType from entry handle
 *
 * @param [in] handle stadb entry
 *
 * @param [out] clientRcpiType
 *
 * @return LBD_BOOL LBD_TRUE if rcpiType is supported, false
 *                  otherwise
 */
LBD_BOOL stadbEntry_getClientRcpiType(const stadbEntry_handle_t handle, u_int8_t *clientRcpiType);

/**
 * @brief Update whether the STA of the provided entry supports
 *        802.11k Radio Resource Management (RRM). Called after
 *        receiving an association request
 *
 * @param [in] entry  the handle of the entry to update
 * @param [in] isRRMSupported  LBD_TRUE if RRM is supported
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_updateIsRRMSupported(stadbEntry_handle_t entry,
                                           LBD_BOOL isRRMSupported);

/**
 * @brief Get the client classification group info the sta is
 *        operating at.
 *
 * @param [in] entry  the handle of the entry to update
 * @param [in] sta_classa  the client classification group
 *
 * @return LBD_NOK if the parameters are invalid or the STA is not associated;
 *         otherwise LBD_OK
 */

LBD_STATUS stadbEntry_getClientClassGroup(stadbEntry_handle_t entry,
                                        u_int8_t *sta_class);

/**
 * @brief Return whether or not we have a pending beacon report
 *
 * @param [in] handle entry to check Pending beacon report
 *
 * @return LBD_BOOL LBD_TRUE if we have a pending beacon report, false
 *                  otherwise
 */
LBD_STATUS stadbEntry_getPendingBeaconReport(const stadbEntry_handle_t entry,
                                        u_int8_t *pendingBcnRpt);

/**
 * @brief Return whether or not we have a pending beacon request to be
 * sent on a different channel
 *
 * @param [in] handle entry to check Pending beacon report request
 *
 * @return LBD_BOOL LBD_TRUE if we have a pending beacon request, false
 *                  otherwise
 */
LBD_STATUS stadbEntry_getPendingBeaconReportRequest(const stadbEntry_handle_t entry,
                                        u_int8_t *pendingBcnRptReq);

/**
 * @brief Return the maximum number of channels selected for 11k request
 *
 * @param [in] handle entry to check Pending beacon report
 *
 * @return LBD_BOOL LBD_TRUE if maximum number of channels selected, false
 *                  otherwise
 */
LBD_STATUS stadbEntry_getMaxNumSelectedChannels(const stadbEntry_handle_t entry,
                                        u_int8_t *maxNumSelectedChannels);

/**
 * @brief Return the requested channels selected for 11k request
 *
 * @param [in] handle entry to check Pending beacon report
 *
 * @return LBD_BOOL LBD_TRUE if the requested channel, false
 *                  otherwise
 */
LBD_STATUS stadbEntry_getBeaconReportChannelsRequested(const stadbEntry_handle_t entry,
                                        lbd_channelId_t *dot11kChannel,
                                        u_int16_t *dot11kFreq);

/**
 * @brief set the STA with operating client classification
 *        group info.
 *
 * @param [in] entry  the handle of the entry to update
 * @param [in] sta_class the client classification group
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */

LBD_STATUS stadbEntry_setClientClassGroup(stadbEntry_handle_t entry,
                                        u_int8_t sta_class);

/**
 * @brief set the rcpiType value
 *
 * @param [in] entry  the handle of the entry to update
 * @param [in] rcpiType
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */

LBD_STATUS stadbEntry_setClientRcpiType(stadbEntry_handle_t entry, u_int8_t rcpiType);


/**
 * @brief set the Pending beacon report
 *
 * @param [in] entry  the handle of the entry to update
 * @param [in] pendingBcnRpt Number of pending BCNReport
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_setPendingBeaconReport(const stadbEntry_handle_t entry,
                                        u_int8_t pendingBcnRpt);

/**
 * @brief set the Max number of channels selected
 *
 * @param [in] entry  the handle of the entry to update
 * @param [in] maxNumSelectedChannels the number of channels selected
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_setMaxNumSelectedChannels(const stadbEntry_handle_t entry,
                                        u_int8_t maxNumSelectedChannels);

/**
 * @brief set the Pending beacon report request
 *
 * @param [in] entry  the handle of the entry to update
 * @param [in] pendingBcnRptReq Number of pending BCNReport request
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_setPendingBeaconReportRequest(const stadbEntry_handle_t entry,
                                        u_int8_t pendingBcnRptReq);

/**
 * @brief set the channels selected for 11k
 *
 * @param [in] entry  the handle of the entry to update
 * @param [in] dot11kChannel channels selected
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_setBeaconReportChannelsRequested(const stadbEntry_handle_t entry,
                                        lbd_channelId_t *dot11kChannel,
                                        u_int16_t *dot11kFreq);

/**
 * @brief Access to update beacon reportfor this entry
 *
 * This private function will be used to read/access the STA entries for
 * which we have received the beacon report
 *
 * @param [in] entry the STA entry to access the flag vaue for
*/
u_int8_t stadbEntryUpdateBeaconReport(stadbEntry_handle_t entry,
                                        const wlanif_beaconReportEvent_t *bcnrptEvent);

/**
 * @brief Retrive the beacon report
 *
 * @param [in] entry  the handle of the entry to update
 * @param [in] bcnrpt retrive the beacon report
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntryRetrieveBeaconReport(stadbEntry_handle_t entry,
                                        wlanif_beaconReport_t *bcnrpt);

/**
 * @brief Reset of beacon Report
 *
 */
void stadbEntry_resetBeaconReport(stadbEntry_handle_t entry);

/**
 * @brief Return whether or not MU-MIMO is supported for the
 *        entry
 *
 * @param [in] handle entry to check for MU-MIMO support
 *
 * @return LBD_BOOL LBD_TRUE if MU-MIMO is supported, false
 *                  otherwise
 */
LBD_BOOL stadbEntry_isMUMIMOSupported(const stadbEntry_handle_t handle);

/**
 * @brief Update whether the STA of the provided entry supports
 *        MU-MIMO. Called after
 *        receiving an association request
 *
 * @param [in] entry  the handle of the entry to update
 * @param [in] isMUMIMOSupported  LBD_TRUE if MU-MIMO is supported
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_updateIsMUMIMOSupported(stadbEntry_handle_t entry,
                                              LBD_BOOL isMUMIMOSupported);

/**
 * @brief Callback function type for iterating all BSSes supported of a STA to
 *        determine if the BSS info gets filled in the output parameter provided
 *        in the stadbEntry_iterateBSSStats function.
 *
 * For each BSS that should be filled in the output parameter, a non-zero metric
 * must be provided. This metric must be the larger the better.
 *
 * @param [in] entry  the STA entry that is currently examined
 * @param [in] bssHandle  the stats handle of the BSS
 * @param [in] cookie  the argument provided to stadbEntry_iterateBSSStats
 *
 * @return the metric if the BSS meets the requirement; otherwise return 0
 */
typedef u_int32_t (*stadbEntry_iterBSSFunc_t)(stadbEntry_handle_t entry,
                                              stadbEntry_bssStatsHandle_t bssHandle,
                                              void *cookie);
/**
 * @brief Iterate all BSSes of a STA entry, invoking callback function on each BSS
 *
 * @param [in] entry  the STA entry to check
 * @param [in] callback  the callback function to invoke
 * @param [in] cookie  opaque parameter to provide in the callback
 * @param [in|out] maxNumBSS  on input, it specifies maximum number of BSS info entries
 *                            expected; on output, it returns the number of BSS info
 *                            entries populated on success
 * @param [out] bssInfo  If not NULL, fill in the basic information of all BSSes
 *                       meets the requirement on success
 *
 * @return LBD_OK if the iteration succeeds; otherwise return LBD_NOK
 */
LBD_STATUS stadbEntry_iterateBSSStats(stadbEntry_handle_t entry, stadbEntry_iterBSSFunc_t callback,
                                      void *cookie, size_t *maxNumBSS, lbd_bssInfo_t *bssInfo);

/**
 * @brief Query PHY capability information of a STA on a BSS
 *
 * @param [in] entry  the STA to query PHY capability info
 * @param [in] bssHandle  the stats handle of the BSS
 * @param [out] phyCapInfo  set to the PHY capability info on success
 *
 * @return LBD_OK on success; otherwise return LBD_NOK
 */
LBD_STATUS stadbEntry_getPHYCapInfo(const stadbEntry_handle_t entry,
                                    const stadbEntry_bssStatsHandle_t bssHandle,
                                    wlanif_phyCapInfo_t *phyCapInfo);

/**
 * @brief Query PHY capability information of a STA on a given band.
 *
 * @param [in] entry  the STA to query PHY capability for
 * @param [in] band  the band on which the information is needed
 * @param [out] phyCapInfo  set to the PHY capability info on success
 *
 * @return LBD_OK on success; otherwise return LBD_NOK
 */
LBD_STATUS stadbEntry_getPHYCapInfoByBand(const stadbEntry_handle_t entry,
                                          wlanif_band_e band,
                                          wlanif_phyCapInfo_t *phyCapInfo);

/**
 * @brief Obtain uplink RSSI information of a given entry on a specific BSS
 *
 * @param [in] handle  the handle of the entry from which to retrieve RSSI information
 * @param [in] bssHandle  the stats handle of the BSS
 * @param [out] ageSecs  if not NULL, set to the age of current RSSI value (in seconds) on success
 * @param [out] probeCount  if the RSSI is measured from probe requests, set to the number of
 *                          probe requests being averaged for this value; otherwise, set to 0
 *
 * @return the RSSI value on success; otherwise, return LBD_INVALID_RSSI
 */
lbd_rssi_t stadbEntry_getUplinkRSSI(const stadbEntry_handle_t handle,
                                    const stadbEntry_bssStatsHandle_t bssHandle,
                                    time_t *ageSecs, u_int8_t *probeCount);

/**
 * @brief Set uplink RSSI value for a STA on a specific BSS
 *
 * It can set either estimated uplink RSSI value or the value
 * obtained from IAS driver stats.
 *
 * @param [in] entry  the STA to set RSSI
 * @param [in] bssHandle  the stats handle of the BSS
 * @param [in] rssi  the RSSI value
 * @param [in] estimated  whether the RSSI value is estimated or not
 *
 * @return LBD_OK on success; otherwise return LBD_NOK
 */
LBD_STATUS stadbEntry_setUplinkRSSI(stadbEntry_handle_t entry,
                                    stadbEntry_bssStatsHandle_t bssHandle,
                                    lbd_rssi_t rssi, LBD_BOOL estimated);

/**
 * @brief Set uplink RSSI value and RSSI Age for a STA on a specific BSS
 *
 * RSSI value is obtained from the UnassocSTALinkMetricsResponse Message
 * along with the age of the measurement.
 *
 * @param [in] entry      the STA to set RSSI
 * @param [in] bssHandle  the stats handle of the BSS
 * @param [in] rssi       the RSSI value
 * @param [in] rssiAge    the age of current RSSI value (in seconds)
 *
 * @return LBD_OK on success; otherwise return LBD_NOK
 */
LBD_STATUS stadbEntry_setUplinkRSSIWithAge(stadbEntry_handle_t entry,
                                           stadbEntry_bssStatsHandle_t bssHandle,
                                           lbd_rssi_t rssi, time_t rssiAge);

/**
 * @brief Obtain the full capacity information (maximum data rate assuming
 *        STA can monopolize the channel) for a specific STA on a specific
 *        channel on both the uplink and downlink.
 *
 * Optionally also get the number of seconds that have elapsed since the
 * estimate was updated.
 *
 * @param [in] handle  the handle of the entry from which to retrieve the
 *                     estimated full capacity
 * @param [in] bssHandle  the stats handle of the BSS
 * @param [out] ulCapacity  the last capacity estimate, in Mbps, or
 *                          LBD_INVALID_LINK_CAP if no capacity information is
 *                          available
 * @param [out] dlCapacity  the last capacity estimate, in Mbps, or
 *                          LBD_INVALID_LINK_CAP if no capacity information is
 *                          available
 * @param [out] deltaUlSecs  if non-NULL, on success this will contain the number
 *                           of seconds that have elapsed since the last uplink
 *                           capacity update for this channel. Not valid if last
 *                           uplink capacity estimate is LBD_INVALID_LINK_CAP.
 * @param [out] deltaDlSecs  if non-NULL, on success this will contain the number
 *                           of seconds that have elapsed since the last downlink
 *                           capacity update for this channel. Not valid if last
 *                           downlink capacity estimate is LBD_INVALID_LINK_CAP.
 */
void stadbEntry_getFullCapacities(const stadbEntry_handle_t handle,
                                  const stadbEntry_bssStatsHandle_t bssHandle,
                                  lbd_linkCapacity_t *ulCapacity,
                                  lbd_linkCapacity_t *dlCapacity,
                                  time_t *deltaUlSecs,
                                  time_t *deltaDlSecs);

/**
 * @brief Store the estimated uplink and downlink capacity information
 *        (maximum data rate assuming STA can monopolize the channel)
 *        for a specific STA on a specific channel.
 *
 * This API is used to update the value stored for a BSS for which stats
 * already exist.
 *
 * @param [in] handle  the handle of the entry from which to store the
 *                     estimated full capacity
 * @param [in] bssHandle  the stats handle of the BSS
 * @param [in] ulCapacity  the estimated uplink capacity, in Mbps. No update
                           if the value is LBD_UNKNOWN_LINK_CAP.
 * @param [in] dlCapacity  the estimated downlink capacity, in Mbps. No update
                           if the value is LBD_UNKNOWN_LINK_CAP.
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_setFullCapacities(stadbEntry_handle_t handle,
                                        stadbEntry_bssStatsHandle_t bssHandle,
                                        lbd_linkCapacity_t ulCapacity,
                                        lbd_linkCapacity_t dlCapacity);

/**
 * @brief Obtain the current estimated airtime for the given STA on a specific BSS.
 *
 * @param [in] handle  the handle of the entry for which to get airtime
 * @param [in] bssHandle  the stats handle of the BSS
 * @param [out] deltaSecs  if non-NULL, on success this will contain the number
 *                         of seconds that have elapsed since last airtime estimation
 *
 * @return the estimated airtime on the BSS, or LBD_INVALID_AIRTIME
 */
lbd_airtime_t stadbEntry_getAirtime(const stadbEntry_handle_t handle,
                                    const stadbEntry_bssStatsHandle_t bssHandle,
                                    time_t *deltaSecs);

/**
 * @brief Store the estimated airtime for the STA on a specific BSS
 *
 * This API is used to update the value stored for a BSS for which stats
 * already exist.
 *
 * @param [in] handle  the handle of the entry for which to store airtime
 * @param [in] bssHandle  the stats handle of the BSS
 * @param [in] airtime  the estimated airtime
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_setAirtime(stadbEntry_handle_t handle,
                                 stadbEntry_bssStatsHandle_t bssHandle,
                                 lbd_airtime_t airtime);

/**
 * @brief Store the estimated airtime for the STA on a given BSS
 *
 * This API is used when only the identifying info is known for the BSS. If
 * no existing stats entry exists for the BSS, one will be created.
 *
 * @param [in] handle  the handle of the entry for which to store airtime
 * @param [in] bss  the BSS on which the measurement is received and airtime
 *                  is estimated
 * @param [in] airtime  the estimated airtime
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_setAirtimeByBSSInfo(stadbEntry_handle_t handle,
                                          const lbd_bssInfo_t *bss,
                                          lbd_airtime_t airtime);

/**
 * @brief Obtain the downlink RCPI for the given STA on a specific BSS.
 *
 * @param [in] handle  the handle of the entry for which to get RCPI
 * @param [in] bssHandle  the stats handle of the BSS
 * @param [out] deltaSecs  if non-NULL, on success this will contain the number
 *                         of seconds that have elapsed since last RCPI recorded
 *
 * @return the downlink RCPI on the BSS, or LBD_INVALID_RCPI
 */
lbd_rcpi_t stadbEntry_getRCPI(const stadbEntry_handle_t handle,
                              const stadbEntry_bssStatsHandle_t bssHandle,
                              time_t *deltaSecs);

/**
 * @brief Store the estimated RCPI for the STA on a specific BSS
 *
 * This API is used to update the value stored for a BSS for which stats
 * already exist.
 *
 * @param [in] handle  the handle of the entry for which to store rcpi
 * @param [in] bssHandle  the stats handle of the BSS
 * @param [in] rcpi  the estimated rcpi
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_setRCPI(stadbEntry_handle_t handle,
                              stadbEntry_bssStatsHandle_t bssHandle,
                              lbd_rcpi_t rcpi);

/**
 * @brief Store the estimated RCPI for the STA on a given BSS
 *
 * This API is used when only the identifying info is known for the BSS. If
 * no existing stats entry exists for the BSS, one will be created.
 *
 * @param [in] handle  the handle of the entry for which to store RCPI
 * @param [in] bss  the BSS on which the RCPI measurement is received
 * @param [in] rcpi  the measured RCPI
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_setRCPIByBSSInfo(stadbEntry_handle_t handle,
                                       const lbd_bssInfo_t *bss,
                                       lbd_rcpi_t rcpi);

/**
 * @brief Obtain the current measured data rate for the given STA.
 *
 * Optionally also get the number of seconds that have elapsed since the
 * estimate was updated.
 *
 * @param [in] handle  the handle of the entry for which to get the data rate
 * @param [out] dlRate  the downlink data rate in Mbps
 * @param [out] ulRate  the uplink data rate in Mbps
 * @param [out] deltaSecs  if non-NULL, on success this will contain the number
 *                         of seconds that have elapsed since the last capacity
 *                         update for this channel
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_getLastDataRate(const stadbEntry_handle_t handle,
                                      lbd_linkCapacity_t *dlRate,
                                      lbd_linkCapacity_t *ulRate,
                                      time_t *deltaSecs);

/**
 * @brief Store the current data rates for the STA as seen by its serving AP.
 *
 * @param [in] handle  the handle of the entry for which to store the
 *                     data rate
 * @param [in] txRate  the downlink data rate in Mbps
 * @param [in] rxRate  the uplink data rate in Mbps
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_setLastDataRate(stadbEntry_handle_t handle,
                                      lbd_linkCapacity_t dlRate,
                                      lbd_linkCapacity_t ulRate);

/**
 * @brief Increase the numnber of steering attempts for a STA.
 *
 * @param [in] entry  the handle of the STA entry
 *
 * @return LBD_OK on success. Otherwise LBD_NOK.
 */
LBD_STATUS stadbEntry_incSteerAttemptCount(stadbEntry_handle_t entry);

/**
 * @brief Get the numnber of steering attempts for a STA.
 *
 * @param [in] entry  the handle of the STA entry
 * @param [out] cnt  the number of steering that has been attempted for the STA
 *
 * @return LBD_OK on success. Otherwise LBD_NOK.
 */
LBD_STATUS stadbEntry_getSteerAttemptCount(stadbEntry_handle_t entry, u_int8_t *cnt);

/**
 * @brief Reset the numnber of steering attempts for a STA.
 *
 * Currently this is only invoked when the STA association changes.
 *
 * @param [in] entry  the handle of the STA entry
 *
 * @return LBD_OK on success. Otherwise LBD_NOK.
 */
LBD_STATUS stadbEntry_resetSteerAttemptCount(stadbEntry_handle_t entry);

/**
 * @brief Record when interference pollution has been detected on a given BSS.
 *
 * This will also record the time at which the pollution expires, indicated as
 * a number of seconds from now.
 *
 * @param [in] handle  the handle of the entry for which to update the
 *                     interference detection state
 * @param [in] bssStats  the handle to the BSS
 * @param [in] expirySecs  number of seconds before the interference pollution expires
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_setPolluted(stadbEntry_handle_t handle,
                                  stadbEntry_bssStatsHandle_t bssHandle,
                                  time_t expirySecs);

/**
 * @brief Clear when interference is no longer detected on a given BSS
 *
 * @param [in] handle  the handle of the entry for which to update the
 *                     interference detection state
 * @param [in] bssStats  the handle to the BSS
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_clearPolluted(stadbEntry_handle_t handle,
                                    stadbEntry_bssStatsHandle_t bssHandle);

/**
 * @brief Obtain whether interference has been detected on a given BSS
 *        or not and how many seconds left before this info expires.
 *
 * @param [in] handle  the handle of the entry for which to obtain the
 *                     interference info
 * @param [in] bssStats  the handle to the BSS
 * @param [out] polluted  indicates whether the BSS is marked as polluted
 * @param [out] deltaSecs  if polluted, indicates the number of seconds before
 *                         this info expires. If already expired or not marked
 *                         as polluted, will return 0
 *
 * @return LBD_OK on success; otherwise LBD_NOK
 */
LBD_STATUS stadbEntry_getPolluted(stadbEntry_handle_t handle,
                                  stadbEntry_bssStatsHandle_t bssHandle,
                                  LBD_BOOL *polluted, time_t *expirySecs);

/**
 * @brief Determine whether the channel provided is supported or not for the
 *        given STA.
 *
 * @param [in] handle  the handle to the entry to check
 * @param [in] channel  the channel for which to check
 *
 * @return LBD_TRUE if the channel is supported; otherwise LBD_FALSE
 */

LBD_BOOL stadbEntry_isChannelSupported(const stadbEntry_handle_t handle,
                                       lbd_channelId_t channel);

/**
 * @brief Determine the BSS on which the device is associated, and
 *        optionally how long ago that occurred.
 *
 * @param [in] handle  the handle of the entry to query
 * @param [out] deltaSecs  the number of seconds that have elapsed since
 *                         the device last associated
 *
 * @return the BSS handle on which it is associated, or NULL if the device is not
 *         currently associated or the entry handle is invalid
 */
stadbEntry_bssStatsHandle_t stadbEntry_getServingBSS(
        const stadbEntry_handle_t handle, time_t *deltaSecs);

/**
 * @brief Look up BSS info from BSS stats handle
 *
 * @param [in] bssHandle  the BSS handle to check
 *
 * @return the BSS info on success, or NULL if the BSS handle is invalid
 */
const lbd_bssInfo_t *stadbEntry_resolveBSSInfo(const stadbEntry_bssStatsHandle_t bssHandle);

/**
 * @brief Get the ESS ID the STA last associates on
 *
 * @param [in] handle  the handle to the STA entry
 *
 * @return the last associated ESS ID, or LBD_ESSID_INVALID if the STA has
 *         never associated before
 */
lbd_essId_t stadbEntry_getLastServingESS(stadbEntry_handle_t handle);

/**
 * @brief Look up band from BSS stats handle
 *
 * @param [in] bssHandle  the BSS handle to check
 *
 * @return the band on success, or wlanif_band_invalid if the BSS handle is invalid
 */
wlanif_band_e stadbEntry_resolveBandFromBSSStats(const stadbEntry_bssStatsHandle_t bssHandle);

/**
 * @brief Find the BSS stats entry matching the given BSS info
 *
 * @param [in] handle  the handle to the entry to find BSS stats
 * @param [in] bss  the BSS information to look for an entry
 *
 * @return the mathcing BSS stats handle found, or NULL if not found
 */
stadbEntry_bssStatsHandle_t stadbEntry_findMatchBSSStats(stadbEntry_handle_t handle,
                                                         const lbd_bssInfo_t *bss);

/**
 * @brief Check if the given STA has reserved airtime on any BSS
 *
 * @param [in] handle  the handle to the entry to check reserved airtime
 *
 * @return LBD_TRUE if the STA has reserved airtime; otherwise return LBD_FALSE
 */
LBD_BOOL stadbEntry_hasReservedAirtime(stadbEntry_handle_t handle);

/**
 * @brief Obtain the reserved airtime for the STA on the given BSS
 *
 * @param [in] handle  the handle to the STA entry
 * @param [in] bssHandle  the handle to the BSS stats
 *
 * @return the reserved airtime if any; otherwise return LBD_INVALID_AIRTIME
 */
lbd_airtime_t stadbEntry_getReservedAirtime(stadbEntry_handle_t handle,
                                            stadbEntry_bssStatsHandle_t bssHandle);

/**
 * @brief Obtain the best PHY mode supported by the client across all bands
 *
 * @param [in] handle  the handle to the STA entry
 *
 * @return the best PHY mode supported by this STA; return wlanif_phymode_invalid
 *         if the STA is not valid
 */
wlanif_phymode_e stadbEntry_getBestPHYMode(stadbEntry_handle_t handle);

/**
 * @brief Find the BSS stats entry matching the given remote BSS info
 *
 * If no matching BSS stats is found, will create one. Local BSS should
 * use stadbEntry_findMatchBSSStats, and will be ignored here.
 *
 * @param [in] handle  the handle to the entry to find BSS stats
 * @param [in] bss  the remote BSS information to look for an entry
 *
 * @return the mathcing BSS stats handle
 */
stadbEntry_bssStatsHandle_t stadbEntry_findMatchRemoteBSSStats(stadbEntry_handle_t handle,
                                                               const lbd_bssInfo_t *bss);

/**
 * @brief Determine if any BSS on the given channel is marked as polluted for this STA
 *
 * @param [in] handle  the handle to the STA
 * @param [in] channel  the given channel number
 * @param [out] pollutionFree  on success, set to LBD_TRUE if no BSS on the given
 *                             channel is marked as polluted; otherwise, set to LBD_FALSE
 *
 * @return LBD_OK if the channel is supported by the STA; otherwise return LBD_NOK
 */
LBD_STATUS stadbEntry_isChannelPollutionFree(stadbEntry_handle_t handle,
                                             lbd_channelId_t channel,
                                             LBD_BOOL *pollutionFree);

/**
 * @brief Update the timestamp for this entry
 *
 * This function will be used to update the timestamp whenver a steering
 * has been initiated for this client. And check if the steering has reached the threshold.
 * If so then find  if the time difference is less than the Upgrade allowed time interval.
 *
 * @param [in] entry the STA entry for which the timestamp is to be updated
 *
 * @return LBD_TRUE is Client has to be made legacy Upgrade steering unfriendly
 */
LBD_BOOL stadbUpdateLegacyUpgradeSteeringTimestamp(stadbEntry_handle_t entry);

/**
 * @brief Function to invoke to jsonize steerExec state
 *
 * @param [in] handle  the entry being jsonized
 * @return a pointer to a json object that represents the steerExec state
 */
typedef json_t *(*stadbEntry_jsonizeSteerExecCB_t)(stadbEntry_handle_t entry);

/**
 * @brief Convert a given entry to a Jansson object
 *
 * @param [in] entry the handle to the STA
 *
 * @return a pointer to a new json object if success, otherwise return NULL
 */
json_t *stadbEntryJsonize(const stadbEntry_handle_t entry,
                          stadbEntry_jsonizeSteerExecCB_t jseCB);

/**
 * @brief Set dirty flag based if the entry is in network
 *
 * @param [in] entry the entry that was changed
 */
void stadbEntrySetDirtyIfInNetwork(stadbEntry_handle_t entry);

#if defined(__cplusplus)
}
#endif

#endif
