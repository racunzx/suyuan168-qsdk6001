/*
 * Copyright (c) 2017-2018, 2020-2021 Qualcomm Innovation Center, Inc.
 * All Rights Reserved
 * Confidential and Proprietary - Qualcomm Innovation Center, Inc.
*/
#ifndef __WLAN_SON_PUB_H_
#define __WLAN_SON_PUB_H__

#include <wlan_son_types.h>
#include <cdp_txrx_ctrl.h>
#include <wdi_event_api.h>
#include <wlan_son_ald.h>
#include <ald_netlink.h>

#define SON_INVALID_ROOT_AP_DISTANCE 255

/**
 * SON WiFi backhaul type is defined 1 and
 * Eth backhaul type is defined 2, PLC is defined as 3 respectively.
 */
enum {
	SON_BACKHAUL_TYPE_NOT_FOUND = 0,
	SON_BACKHAUL_TYPE_WIFI = 1,
	SON_BACKHAUL_TYPE_ETHER = 2,
	SON_BACKHAUL_TYPE_PLC = 3,
};

/**
 * @brief init function for WLAN SON
 *
 * @param [in] void
 *
 * @return QDF_STATUS_SUCCESS for success otherwise
 *  QDF_STATUS_FAIL
 */

QDF_STATUS wlan_son_init(void);

/**
 * @brief Deinit SON infrasturcture.
 *
 * @param [in] void
 * @return QDF_STATUS_SUCCESS for success otherwise
 *  QDF_STATUS_FAIL
 * @return
 */
QDF_STATUS wlan_son_deinit(void);
/**
 * @brief Return psoc priv handler for SON
 *
 * @param [in] psoc.
 *
 * @return Return son psoc priv  or NULL if not found
 */

struct son_pdev_priv *wlan_son_get_pdev_priv(struct wlan_objmgr_pdev *pdev);

/**
 * @brief Return psoc priv handler for SON
 *
 * @param [in] psoc.
 *
 * @return Return son psoc priv  or NULL if not found
 */

inline struct son_psoc_priv *wlan_son_get_psoc_priv(
	struct wlan_objmgr_psoc *psoc);
/**
 * @brief Return vdev priv handler for SON
 *
 * @param [in] vdev.
 *
 * @return Return son vdevc priv  or NULL if not found
 */

struct son_vdev_priv *wlan_son_get_vdev_priv(
	struct wlan_objmgr_vdev *vdev);
/**
 * @brief Return vdev priv handler for SON
 *
 * @param [in] vdev.
 *
 * @return Return son vdevc priv  or NULL if not found
 */

struct son_peer_priv *wlan_son_get_peer_priv(
	struct wlan_objmgr_peer *peer);

/**
 * @brief Verify that the son handle is valid within the
 *        struct psoc provided.
 *
 * @param [in] psoc  the handle to the radio where the band steering state
 *                 resides
 *
 * @return true if handle is valid; otherwise false
 */

bool wlan_son_is_pdev_valid(struct wlan_objmgr_pdev *pdev);

/**
 * @brief Determine whether the band steering module is enabled or not.
 *
 * @param [in] pdev  the handle to the radio where the band steering state
 *                 resides
 *
 * @return non-zero if it is enabled; otherwise 0
 */

u_int8_t wlan_son_is_pdev_enabled(struct wlan_objmgr_pdev *pdev);

/**
 * @brief Determine whether broadcasting of band steering events is enabled on
 *        a vdev.
 *
 * @param [in] vdev to check
 *
 * @return non-zero if it is enabled; otherwise 0
 */

u_int8_t wlan_son_is_vdev_event_bcast_enabled(struct wlan_objmgr_vdev *vdev);

/**
 * @brief Enable/Disable broadcasting of band steering events.
 *
 * @param [in] vdev to check and flag to enable/disable
 *
 * @return non-zero if it is failure; otherwise 0
 */

int son_core_enable_disable_vdev_bcast_events(struct wlan_objmgr_vdev *vdev ,
					u_int8_t enable);

/**
 * @brief Determine whether band steering events are enabled on
 *        a VAP.
 *
 * @param [in] vap  VAP to check
 *
 * @return non-zero if it is enabled; otherwise 0
 */

u_int8_t wlan_son_is_vdev_event_enabled(struct wlan_objmgr_vdev *vdev);

/**
 * @brief Determine whether the VAP handle is valid, has a valid band
 *        steering handle, is operating in a mode where band steering
 *        is relevant, and is not in the process of being deleted.
 *
 * @return true if the VAP is valid; otherwise false
 */

bool wlan_son_is_vdev_valid(struct wlan_objmgr_vdev *vdev);

/**
 * @brief Determine whether the VAP has band steering enabled.
 *
 * Validate that the VAP has a valid band steering handle, that
 * it is operating in the right mode (AP mode), and that band steering has been
 * enabled on the VAP.
 *
 * @param [in] vap  the VAP to check
 *
 * @return true if the VAP is valid and has band steering enabled; otherwise
 *         false
 */

bool wlan_son_is_vdev_enabled(struct wlan_objmgr_vdev *vdev);

PUBLIC void son_peer_authorize(struct wlan_objmgr_peer *peer);

/**
 * @brief Mark peer as inactive .
 *
 * This is used by Rx path to reset inactive flag.
 *
 * @param [in] peer
 * @param [in] bool inactive flag .
 *
 * @param [inout] void.
 */

PUBLIC void son_mark_node_inact(struct wlan_objmgr_peer *peer, bool inactive);

/**
 * @brief indicate user space about change in activity.
 * @param [in] pdev.
 * @param [in] mac address to be reported.
 * @param [in] bool active or inactive flag.
 * @param [inout] void.
 */
PUBLIC void son_record_act_change(struct wlan_objmgr_pdev *pdev,
				  u_int8_t *mac_addr,
				  bool active);

/**
 * @brief Enable/disable inst rssi log for son
 *
 * @param [in] enable/disable log
 *
 * @param [inout] void.
 */

PUBLIC void son_record_inst_rssi_log_enable (struct wlan_objmgr_vdev *vdev, int enable);

/**
 * @brief Send error to user space if instantanous rssi was failed.
 *
 * This is used by rssi threshold logic in steering.
 *
 * @param [in] peer
 *
 * @param [inout] void.
 */

PUBLIC void son_record_inst_peer_rssi_err(struct wlan_objmgr_peer *peer);

/**
 * @brief Record RSSI per peer.
 *
 * @param [in] peer to update RSSI.
 * @param [in] rssi value
 *
 * @param [inout] void.
 */


PUBLIC void son_record_inst_peer_rssi(struct wlan_objmgr_peer *peer,
				      u_int8_t rssi);

/**
 * @brief to record rssi per peer.
 *
 * This is used by steering routine to record rssi per peer.
 *
 * @param [in] peer
 * @param [in] rssi to be stored.
 *
 * @param [inout] void.
 */

PUBLIC void son_record_peer_rssi(struct wlan_objmgr_peer *peer, u_int8_t rssi);

/**
 * @brief check if rssi seq is matching with last received frame.
 * @param [in] peer .
 * @param [in] rssi sequnece number.
 *
 * @param [inout] True if matches .
 */

PUBLIC int32_t son_match_peer_rssi_seq(struct wlan_objmgr_peer *peer,
				       u_int32_t rssi_seq);

/**
 * @brief update user space if current rate changes from last reported rate.
 *
 * This is used by rate indication logic in steering.
 *
 * @param [in] peer
 * @param [in] curent rate.
 * @param [in] last tx rate.
 *
 * @param [inout] void.
 */

PUBLIC void son_update_peer_rate(struct wlan_objmgr_peer *peer,
				 u_int32_t rssi, u_int32_t last_rate);

/**
 * @brief Called when firmware stats are updated for a STA, with
 *        RSSI changed and a valid Tx rate
 *
 * @param [in] ni  the node for which the stats are updated
 * @param [in] current_vdev  vdev for which stats up to this point
 *                          have been collected.  If it does not
 *                          match the VAP the current node is
 *                          on, should start message over so
 *                          each message is only STAs on a
 *                          particular VAP
 * @param [inout] sta_stats  the structure to update with STA
 *                           stats
 *
 * @return true if stats are updated, interference detection is
 *         enabled on the radio, and band steering is enabled on
 *         the VAP; false otherwise
 */
PUBLIC bool son_update_sta_stats(struct wlan_objmgr_peer *peer,
				 struct wlan_objmgr_vdev *current_vdev,
				 struct bs_sta_stats_ind *sta_stats,
				 void *stats);
/**
 * @brief store pid per vdev for multi instances of user space daemons.
 *
 * This is used by steering infrastructure.
 *
 * @param [in] vdev
 * @param [in] pid to be stored.
 *
 * @param [inout] void.
 */

PUBLIC void son_set_vdev_lbd_pid(struct wlan_objmgr_vdev *vdev, u_int32_t pid);


/**
 * @brief To enable or disable SON timers.
 *
 * This is used by vap create or delete functionality to enable or disable SON timers.
 *
 * @param [in] vdev.
 * @param [in] address 2 from mac frame.
 */

PUBLIC int son_enable_disable_steering(struct wlan_objmgr_vdev *vdev,
				 bool enable);


PUBLIC u_int8_t son_is_ackrssi_enabled(struct wlan_objmgr_vdev *vdev);

/**
 * @brief To check if probe response are withheld during steering.
 *
 * This is used by frame input routine to check withheld condition.
 *
 * @param [in] vdev.
 * @param [in] address 2 from mac frame.
 * @param [in] rssi of frame received.
 * @param [inout] false if not enabled otherwise true.
 */

PUBLIC bool son_is_probe_resp_wh_2G(struct wlan_objmgr_vdev *vdev,
				    u_int8_t *mac_addr,
				    u_int8_t sta_rssi);

/**
 * @brief Query the band steering module for whether it is withholding
 *        probe responses for the given MAC address on this VAP.
 *
 * @param [in] vdev  the VAP on which the probe request was received
 * @param [in] mac_addr  the MAC address of the client that sent the probe
 *                       request
 *
 * @return true if the response should be withheld; otherwise false
 */

PUBLIC bool son_is_probe_resp_wh(struct wlan_objmgr_vdev *vdev,
				 const u_int8_t *mac_addr, u_int8_t probe_rssi);

/**
 * @brief Inform the band steering module of a channel utilization measurement.
 *
 * If the necessary number of utilization measurements have been obtained,
 * this will result in an event being generated.
 *
 * @param [in] vdev  the vdev for which the utilization report occurred
 * @param [in] ieee_chan_num  the channel on which the utilization measurement
 *                            took place
 * @param [in] chan_utilization  the actual utilization measurement
 */

PUBLIC void son_record_utilization(struct wlan_objmgr_vdev *vdev,
				   u_int ieee_chan_num,
				   u_int32_t chan_utilization);

/**
 * @brief Increment repeater count per vap.
 *
 * This is used by assoication routine based on ie detection.
 *
 * @param [in] vdev VAP for incrementing repeater count.
 *
 * @param [inout] void .
 */

PUBLIC void son_repeater_cnt_inc(struct wlan_objmgr_vdev *vdev);

/**
 * @brief Decrement repeater count per vap.
 *
 * This is used by assoication routine based on ie detection.
 *
 * @param [in] vdev VAP for incrementing repeater count.
 *
 * @param [inout] void .
 */

PUBLIC void son_repeater_cnt_dec(struct wlan_objmgr_vdev *vdev);

/**
 * @brief Get connected repeater count.
 *
 * This is used by uplink node detection
 *
 * @param [in] Vdev participating.
 *
 * @param [inout] count of connected repeater.
 */

PUBLIC u_int8_t son_repeater_cnt_get(struct wlan_objmgr_vdev *vdev);

/**
 * @brief To update backhaul rate.
 *
 * This is used by uplink node detection logic.
 *
 * @param [in] vdev participating in uplink node detection.
 * @param [in] rate to be updated.
 * @param [in] bool self , update self rate if true or  uplink rate.
 *
 * @param [inout] void
 */

PUBLIC void son_update_backhaul_rate(struct wlan_objmgr_vdev *vdev, u_int16_t rate, bool self);

/**
 * @brief To NSS.
 *
 * This is used by uplink node detection logic.
 *
 * @param [in] vdev participating in uplink node detection.
 * @param [in] nss to be updated.
 *
 * @param [inout] void
 */

PUBLIC void son_update_nss(struct wlan_objmgr_vdev *vdev, u_int8_t nss);

/**
 * @brief To get NSS
 *
 * @param [in] vdev participating in uplink node detection.
 *
 * @param [out] return least nss [either ap / local nss]
 */

PUBLIC u_int8_t son_get_nss(struct wlan_objmgr_vdev *vdev);

/**
 * @brief To update uplink (backhaul link) snr
 *
 * @param [in] vdev participating in uplink node detection.
 * @param [in] snr to be updated.
 *
 * @param [inout] void
 */

PUBLIC void son_update_uplink_snr(struct wlan_objmgr_vdev *vdev, u_int8_t snr);

/**
 * @brief Get backhaul rate.
 *
 * This is used by uplink node detection logic.
 *
 * @param [in] vdev participating in detection logic.
 * @param [in] bool self  true means for serving ap rate
 *
 * @param [inout] return rate.
 */

PUBLIC u_int16_t son_get_backhaul_rate(struct wlan_objmgr_vdev *vdev, bool self);

/**
 * @brief Store uplink bssid for SON.
 *
 * This is used by association routine to store bssid upon association.
 *
 * @param [in] pdev
 * @param [in] bssid to store
 *
 * @param [inout] void.
 */

PUBLIC void son_update_uplink_bssid(struct wlan_objmgr_pdev *pdev , char *bssid);

/**
 * @brief update son ie for all vaps.
 * @param [in] vdev.
 * @param [inout] void.
 */

PUBLIC void son_update_bss_ie(struct wlan_objmgr_vdev *vdev);

/**
 * @brief update assoc frame
 * @param [in] wbuf
 * @param [in] peer
 */

PUBLIC void son_update_assoc_frame(struct wlan_objmgr_peer *peer, wbuf_t wbuf);

/**
 * get uplink estimated rate
 * @param      vaphandle   : vap handle
 * @param      snr         : rssi value from scan entry
 *
 * @return : estimated rate.
 *           only valid for STA vaps;
 *
 */
PUBLIC u_int16_t son_get_uplinkrate(struct wlan_objmgr_vdev *vdev, u_int8_t snr);


/**
 * Function work under vdev_lock.
 * @brief: Change feature extented capa per vdev.
 * @param  vaphandle   : vap handle
 * @param action: SET, GET and clear.
 * @param cap: capabilites to change.
 * @return :in case of get it return positive value if cap
 *           is set, defualt is EOK and it can return -EINVAL
 *           if vdev is null.
 *           only valid for STA vaps;
 */

PUBLIC u_int8_t son_vdev_fext_capablity(struct wlan_objmgr_vdev *vdev,
					son_capability_action action,
					u_int32_t cap);


/**
 * Function work under vdev_lock.
 * @brief: Change feature capablities per vdev.
 * @param  vaphandle   : vap handle
 * @param action: SET, GET and clear.
 * @param cap: capabilites to change.
 * @return :in case of get it return positive value if cap
 *           is set, defualt is EOK and it can return -EINVAL
 *           if vdev is null.
 *           only valid for STA vaps;
 *
 */

PUBLIC u_int8_t son_vdev_feat_capablity(struct wlan_objmgr_vdev *vdev,
					son_capability_action action,
					u_int32_t cap);

/**
 * @brief Set/Clear MAP Capabilities per vdev
 * @param vdev
 * @param cap MAP Capability to be set/cleared
 * @param value To set or clear the capability flags
 *              MAP Capability SON_MAP_VAP_TYPE is used to set/clear following
 *              capability flags by making use of specific bits of the value :
 *                  Fronthaul BSS : Set/Cleared according to Bit 5 of value
 *                  Backhaul BSS : Set/Cleared according to Bit 6 of value
 *                  Backhaul STA : Set/Cleared according to Bit 7 of value
 * @return Default is EOK, -EINVAL if vdev is invalid
 */
PUBLIC int son_vdev_map_capability_set(struct wlan_objmgr_vdev *vdev,
				       son_map_capability cap, int value);

/**
 * @brief Get MAP Capabilities per vdev
 * @param vdev
 * @param cap MAP Capability to get
 * @return Value of Map Capability
 */
PUBLIC int son_vdev_map_capability_get(struct wlan_objmgr_vdev *vdev,
				       son_map_capability cap);

/**
 * Function work under vdev_lock.
 * @brief: Get the count of son enabled vdev .
 * @param  vaphandle   : vap handle
 * @param action: GET
 * @return :It returns the number of vaps for which SON is enabled
 *            default is EOK and it can return -EINVAL
 *           if vdev is null.
 *
 */
PUBLIC u_int8_t son_vdev_get_count(struct wlan_objmgr_vdev *vdev,
        son_capability_action action);


/**
 * Function work under peer_ref count held
 * @brief: get if steering is in progress for node.
 * @param  peer;
 * @return : true if steering in progress otherwise false.
 */

PUBLIC bool son_is_steer_in_prog(struct wlan_objmgr_peer *peer);

PUBLIC void son_set_whc_apinfo_flag(struct wlan_objmgr_peer *peer, u_int8_t flag);

PUBLIC void son_clear_whc_apinfo_flag(struct wlan_objmgr_peer *peer, u_int8_t flag);

PUBLIC int son_has_whc_apinfo_flag(struct wlan_objmgr_peer *peer, u_int8_t flag);

PUBLIC void son_set_whc_rept_info(struct wlan_objmgr_peer *peer);

PUBLIC void son_clear_whc_rept_info(struct wlan_objmgr_peer *peer);

PUBLIC int32_t son_get_whc_rept_info(struct wlan_objmgr_peer *peer);

/**
 * @brief  Attach module to open socket to user space.
 * @param [in]
 *
 * @param [inout] void .
 */

PUBLIC int8_t son_netlink_attach(void);

/**
 * @brief To close socket to user layer.
 * @param [in] void
 *
 * @param [inout] void .
 */

PUBLIC int8_t son_netlink_destroy(void);

/**
 * @brief send activity change event to user space.
 * @param [in] vdev, vdev to which station is connected.
 * @param [in] macaddr, sta mac address.
 * @param [in] boolean flag indicating activity of client.
 * @param [inout] void .
 */

PUBLIC void son_notify_activity_change(struct wlan_objmgr_vdev *vdev,
				       char *macaddr, bool inactive);

/**
 * @brief Send STA stats to userspace via netlink message
 *
 * @param [in] vdev  the first vdev on the radio for which STA
 *                  stats were updated
 * @param [in] sta_stats  the STA stats to send
 */

PUBLIC void son_send_sta_stats_event(struct wlan_objmgr_vdev *vdev,
				     struct bs_sta_stats_ind *sta_stats);

/**
 * @brief Generate an event indicating that a probe request was received.
 *
 * This is used by user space to determine which nodes are dual band
 * capable.
 *
 * @param [in] vap  the VAP on which the probe was received
 * @param [in] mac_addr  the MAC address of the client that sent the probe
 *                       request
 * @param [in] rssi  the RSSI of the received probe request
 * @param [in] blocked  flag to indicate whether probe response was blocked
 * @param [in] ssid_null  flag to indicate whether SSID was null in the
 *                        incoming probe-request
 */

PUBLIC void son_send_probereq_event(struct wlan_objmgr_vdev *vdev,
				    const u_int8_t *mac_addr,
				    u_int8_t rssi,
				    bool blocked,
				    bool ssid_null);

/**
 * @brief Generate an event with the provided channel utilization measurement.
 *
 * @pre vap has already been checked and confirmed to be valid and band
 *      steering has been confirmed to be enabled
 *
 * @param [in] vap  the VAP that was used for the sampling
 * @param [in] chan_utilization  the utilization in percent
 * @param [in] is_debug  whether the log generated should be a debug event
 *                       or a regular event; debug events represent an
 *                       instantaneous measurement whereas normal (non-debug)
 *                       represent a filtered/averaged measurement
 */

PUBLIC void son_send_utilization_event(struct wlan_objmgr_vdev *vdev,
				       u_int8_t chan_utilization,
				       bool is_debug);

/**
 * @brief Send an event to user space when requested RSSI measurement is available
 *
 * @pre vap has already been checked and confirmed to be valid and band
 *      steering has already been confirmed to be enabled
 *
 * @param [in] vap  the VAP that the client whose RSSI is measured associated to
 * @param [in] mac_addr  the MAC address of the client
 * @param [in] rssi  the measured RSSI
 * @param [in] is_debug  whether the log generated should be a debug event
 *                       or a regular event; debug events represent an
 *                       instantaneous (but averaged by firmware already) RSSI
 *                       measurement whereas normal (non-debug) represent the
 *                       RSSI measured by sending NDPs
 */

PUBLIC void son_send_rssi_measurement_event(struct wlan_objmgr_vdev *vdev,
					    const u_int8_t *mac_addr,
					    u_int8_t rssi,
					    bool is_debug);

/**
 * @brief Send an event to user space on RSSI measurement crossed threshold
 *
 * @pre vap has already been checked and confirmed to be valid and band
 *      steering has confirmed to be enabled
 *
 * @param [in] vap  the VAP that the client whose RSSI is measured associated to
 * @param [in] mac_addr  the MAC address of the client
 * @param [in] rssi  the measured RSSI
 * @param [in] inact_xing  flag indicating if the RSSI crossed inactivity RSSI threshold.
 * @param [in] low_xing  flag indicating if the RSSI crossed low RSSI threshold
 * @param [in] rate_xing  flag indicating if the RSSI crossed
 *                        the rate RSSI threshold
 * @param [in] ap_xing  flag indicating if the RSSI crossed the AP steering RSSI threshold
 */
PUBLIC void son_send_rssi_xing_event(struct wlan_objmgr_vdev *vdev,
				     const u_int8_t *mac_addr,
				     u_int8_t rssi,
				     BSTEERING_XING_DIRECTION inact_xing,
				     BSTEERING_XING_DIRECTION low_xing,
				     BSTEERING_XING_DIRECTION rate_xing,
				     BSTEERING_XING_DIRECTION ap_xing);

/**
 * @brief Send an event to user space on RSSI measurement crossed threshold for MAP
 *
 * @pre vdev has already been checked and confirmed to be valid and band
 *      steering has confirmed to be enabled
 *
 * @param [in] vdev  the VAP that the client whose RSSI is measured associated to
 * @param [in] mac_addr  the MAC address of the client
 * @param [in] rssi  the measured RSSI
 * @param [in] map_xing  flag indicating if the RSSI crossed the threshold
 */
PUBLIC void son_send_rssi_xing_map_event(struct wlan_objmgr_vdev *vdev,
					 const u_int8_t *mac_addr,
					 u_int8_t rssi,
					 BSTEERING_XING_DIRECTION map_xing);

/**
 * @brief Send an event to user space if the Tx rate crossed a
 *        threshold
 *
 * @pre vdev has already been checked and confirmed to be valid
 *      and band steering has confirmed to be enabled
 *
 * @param [in] vdev  the VAP that the client whose Tx rate is
 *                  measured associated to
 * @param [in] mac_addr  the MAC address of the client
 * @param [in] tx_rate  the Tx rate
 * @param [in] xing  flag indicating the direction of the Tx
 *                   rate crossing.
 */

PUBLIC void son_send_tx_rate_xing_event(struct wlan_objmgr_vdev *vdev,
					const u_int8_t *mac_addr,
					u_int32_t tx_rate,
					BSTEERING_XING_DIRECTION xing);

/**
 * @brief Send an event to user space when the Tx rate changes.
 *        Note this is a debug only message.
 *
 * @pre vap has already been checked and confirmed to be valid and band
 *      steering has already been confirmed to be enabled
 *
 * @param [in] vap  the VAP that the client whose RSSI is measured associated to
 * @param [in] mac_addr  the MAC address of the client
 * @param [in] tx_rate  the latest Tx rate
 */
PUBLIC void son_send_tx_rate_measurement_event(struct wlan_objmgr_vdev *vdev,
					       const u_int8_t *mac_addr,
					       u_int32_t tx_rate);
/**
 * @brief Inform the band steering module that a node is now
 *        associated
 *
 * @param [in] vap  the VAP on which the change occurred
 * @param [in] mac_addr  the MAC address of the client who
 *                       associated
 * @param [in] isBTMSupported  set to true if BSS Transition
 *                             Management is supported by this
 *                             STA (as indicated in the
 *                             association request frame)
 */

PUBLIC void son_send_node_associated_event(struct wlan_objmgr_vdev *vdev,
					   struct wlan_objmgr_peer *peer);

/**
 * @brief Generate an event when beacon frame report response is received
 *
 * It will generate a netlink event if at least one report is received
 *
 * @param [in] vdev  vdev on which the report is received
 * @param [in] token  dialog token matching the one provided in the request
 * @param [in] macaddr  MAC address of the reporter station
 * @param [inout] bcnrpt  beacon report(s) received
 * @param [in] report_len  total length of the report
 * @param [in] num_rpt_elem  number of the beacon report element(s) to send
 */
PUBLIC void son_send_rrm_frame_bcnrpt_event(struct wlan_objmgr_vdev *vdev,
					    u_int32_t token, u_int8_t *macaddr,
					    const u_int8_t *bcnrpt,
					    size_t report_len,
					    u_int8_t num_rpt_elem);

/**
 * @brief Called when a VAP is stopped (this is only seen on a
 *        RE when the uplink STA interface is disassociated)
 *
 * @param [in] vap  VAP that has stopped
 */

PUBLIC void son_send_vap_stop_event(struct wlan_objmgr_vdev *vdev);

/**
 * @brief Generate an event when Tx power change on a VAP
 *
 * @param [in] vap  the VAP on which Tx power changes
 * @param [in] tx_power  the new Tx power
 */

PUBLIC void son_send_txpower_change_event(struct wlan_objmgr_vdev *vdev,
					  u_int16_t tx_power);

/**
  * @brief provides per-user rx_stats including frame_control and qos_control
  *
  * callback api to identify the peer and derive queue_size from frame_control
  * and qos_control
  *
  * @param [in] pointer to psoc object
  * @param [in] WDI event enum value
  * @param [in] data pointer having qos stats
  * @param [in] data length
  * @param [in] status
  *
  * @param [inout] void.
  */

PUBLIC void son_qos_stats_update_cb(void *psoc_obj, enum WDI_EVENT event,
				    void *data, uint16_t data_len,
				    uint32_t status);

/**
 * @brief Check if received frame has son repeater ie or not.
 *
 * @param [in] pointer to  ie location in frame.
 * @param [in] whc_caps  the bitmask that describes the AP's capabilities
 * @return True is ie are present otherwise false.
 */
PUBLIC int isqca_son_rept_oui(u_int8_t *frm, u_int8_t whc_subtype);


/**
 * @brief Check if received frame has son AP ie or not.
 *
 * @param [in] pointer to  ie location in frame.
 * @param [in] whc_caps  the bitmask that describes the AP's capabilities
 * @return True is ie are present otherwise false.
 */
PUBLIC int isqca_son_oui(u_int8_t *frm, u_int8_t whc_subtype);

/**
 * @brief Add the Whole Home Coverage AP Info information element to app_ie.
 *
 * @param [in] app_ie pointer where AP info IE to be written
 * @param [in] whc_caps  the bitmask that describes the AP's capabilities
 * @param [in] vap handle
 *
 * @return app_ie pointer
 */
PUBLIC u_int8_t *son_add_ap_appie(u_int8_t *app_ie, u_int16_t whc_caps, struct wlan_objmgr_vdev *vdev);
/**
 * @brief Add the Whole Home Coverage AP Info information element to a frame.
 *
 * @param [in] frm  the place in the frame to which to start writing the IE
 * @param [in] whc_caps  the bitmask that describes the AP's capabilities
 * @param [in] vap handle
 * @param [in] Pointer to length which gets total IE length
 *
 * @return pointer to where the remainder of the frame should be written
 */
PUBLIC u_int8_t *son_add_ap_info_ie(u_int8_t *frm, u_int16_t whc_caps,
				    struct wlan_objmgr_vdev *vdev, u_int16_t* ie_len);

/**
 * @brief Add Multi AP IE to a frame
 *
 * @param [in] frm  the place in the frame to which to start writing the IE
 * @param [in] vdev handle
 * @param [in] ie_len Pointer to length which gets total IE length
 *
 * @return pointer to where the remainder of the frame should be written
 */
PUBLIC u_int8_t *son_add_multi_ap_ie(u_int8_t *frm, struct wlan_objmgr_vdev *vdev,
				     u_int16_t* ie_len);
/**
 * @brief Add the Whole Home Coverage repater Info information element to app_ie pointer
 *
 * @param [in] app_ie pointer where repeater info IE to be written
 *
 * @return app_ie pointer
 */

PUBLIC u_int8_t *son_add_rept_appie(u_int8_t *app_ie);
/**
 * @brief Add the Whole Home Coverage repater Info information element to a frame.
 *
 * @param [in] frm  the place in the frame to which to start writing the IE
 * @param [in] whc_caps  the bitmask that describes the AP's capabilities
 * @param [in] vap handle
 * @param [in] Pointer to length which gets total IE length
 *
 * @return pointer to where the remainder of the frame should be written
 */

PUBLIC u_int8_t *son_add_rept_info_ie(u_int8_t *frm);

/**
 * @brief Parse a Whole Home Coverage AP Info IE, storing the capabilities
 *        of the AP.
 *
 * @param [in] ni  the BSS node for the AP that advertised these capabilities
 * @param [in] ie  the beginning of the information element
 */

PUBLIC void son_process_whc_apinfo_ie(struct wlan_objmgr_peer *peer,
				      const u_int8_t *ie);

PUBLIC void son_process_whc_rept_info_ie(struct wlan_objmgr_peer *peer,
					 const u_int8_t *ie);
/**
 * @brief Parse a Easy Mesh AP Info IE from assoc response, storing the capabilities
 *        of the AP.
 *
 * @param [in] ni  the BSS node for the AP that advertised these capabilities
 * @param [in] ie  assoc response ie
 * @param [in] ie_len  assoc response ie len
 */

PUBLIC void son_process_assoc_resp_ie(struct wlan_objmgr_peer *peer, u_int8_t *ie,
                                      u_int32_t ie_len);

PUBLIC QDF_STATUS wlan_son_psoc_close(struct wlan_objmgr_psoc *psoc);
PUBLIC QDF_STATUS wlan_son_psoc_open(struct wlan_objmgr_psoc *psoc);

PUBLIC u_int8_t ucfg_son_get_root_dist(struct wlan_objmgr_vdev *vdev);

PUBLIC int8_t ucfg_son_set_root_dist(struct wlan_objmgr_vdev *vdev,
				     u_int8_t root_distance);

PUBLIC u_int8_t ucfg_son_get_bestul_hyst(struct wlan_objmgr_vdev *vdev);
PUBLIC void ucfg_son_set_bestul_hyst(struct wlan_objmgr_vdev *vdev, u_int8_t hyst);
void son_bs_stats_update_cb(void *psoc, enum WDI_EVENT event,
			    void *data, uint16_t data_len,
			    uint32_t status);

/**
 * @brief Called to get the in network table for 2G
 *
 * @param [in] vap  the VAP on which the table is requested for
 * @param [inout] num_entries  if 0 return total entries in table,
 *                          else return all entries
 * @param [in] channel  return mac address for this channel,
 * @param [inout] data  pointer to hold data to return
 */

PUBLIC int8_t son_get_innetwork_table(struct wlan_objmgr_vdev *vdev, void *data, int *num_entries,
                                      int8_t channel);
/**
 * @brief To set sonmode and backhaul type for mixed backhaul.
 *
 * @param [in] vdev the VAP
 * @param [in] u_int16_t sonmode (CAP or RE) and backhaul type used.
 *
 * @return true if set ul rate succesfully, otherwise return false.
 */
bool son_set_backhaul_type_mixedbh(struct wlan_objmgr_vdev *vdev, u_int8_t backhaul_type);


/**
 * @brief To set uplink_rate for mixed backhaul.
 *
 * @param [in] vdev the VAP to set.
 * @param [in] u_int16_t ulrate.
 *
 * @return true if set ul rate succesfully, otherwise return false.
 */
bool son_set_ul_mixedbh(struct wlan_objmgr_vdev *vdev, u_int16_t ulrate);

/**
 * @brief To get uplink_rate for mixed backhaul.
 *
 * @param [in] vdev the VAP to get.
 *
 * @return uplink_rate.
 */
u_int16_t son_get_ul_mixedbh(struct wlan_objmgr_vdev *vdev);

#if QCA_SUPPORT_SSID_STEERING
/**
 * @brief Attach module to open socket to user space.
 * @param [in] void.
 *
 * @return EOK for success and -EINVAL for failure.
 */
PUBLIC int8_t son_ssid_steering_netlink_attach(void);

/**
 * @brief To close socket to user layer.
 * @param [in] void.
 *
 * @return EOK for success and -EINVAL for failure.
 */
PUBLIC int8_t son_ssid_steering_netlink_destroy(void);

/**
 * @brief Event to send macaddress of client to user layer.
 *
 * @param [in] macaddress of client.
 *
 * @return void.
 */
PUBLIC void son_send_ssid_steering_event(u_int8_t *macaddr);

/**
 * @brief To get vdev config type of ssid steering.
 *
 * @param [in] vdev the VAP.
 *
 * @return true if VAP is private.
 */
PUBLIC bool son_get_ssid_steering_vdev_is_pvt(struct wlan_objmgr_vdev *vdev);

/**
 * @brief To get vdev config type of ssid steering.
 *
 * @param [in] vdev the VAP.
 *
 * @return true if VAP is public.
 */
PUBLIC bool son_get_ssid_steering_vdev_is_pub(struct wlan_objmgr_vdev *vdev);
#endif

/**
 * @brief To update SON IEs in vap appie list
 *
 * @param [in] vdev the VAP
 * @param [in] frame type
 *
 * @return 0 if IE updated sucessfully
 */
PUBLIC int son_update_appielist(struct wlan_objmgr_vdev *vdev,
		ieee80211_frame_type ftype);

/**
 * @brief To update SON IEs for all vaps
 *
 * @param [in] ic
 *
 */
PUBLIC void son_pdev_appie_update(struct ieee80211com *ic);
/**
 * @brief To get snr value.
 *
 * @param [in] scan_entry the scan_entry structure.
 * @param [in] vap the VAP.
 *
 * @return snr value.
 */
PUBLIC int son_get_snr(ieee80211_scan_entry_t  scan_entry,
		struct ieee80211vap *vap);

/**
 * @brief To indicate mgmt/action frame.
 *
 * @param [in] vdev the VAP.
 * @param [in] peer the Peer.
 * @param [in] subtype the frame subtype.
 * @param [in] frame the frame pointer.
 * @param [in] frame_len the frame length in bytes.
 * @param [in] meta_data the meta data about frame.
 *
 * @return 0 if frame is accepted.
 */
PUBLIC int son_update_mgmt_frame(struct wlan_objmgr_vdev *vdev,
				 struct wlan_objmgr_peer *peer, int subtype,
				 u_int8_t *frame, u_int16_t frame_len,
				 void *meta_data);

/**
 * @brief To indicate mlme/control-path event.
 *
 * @param [in] vdev the VAP.
 * @param [in] peer the Peer.
 * @param [in] event the event_type.
 * @param [in] event_data the frame length in bytes.
 *
 * @return 0 if event is processed.
 */
PUBLIC int son_update_mlme_event(struct wlan_objmgr_vdev *vdev,
				 struct wlan_objmgr_peer *peer,
				 enum son_event_type event, void *event_data);

/**
 * @brief To enable or disable peer ext stats.
 *
 * @param [in] peer.
 * @param [in] enable - enable or disable peer ext stats.
 *
 * @return 0 if success else error code
 */
PUBLIC int son_enable_disable_peer_ext_stats(struct wlan_objmgr_peer *peer,
					     uint32_t enable);

/**
 * @brief To set ald record free desc.
 *
 * @param [in] psoc.
 * @param [in] descs - free descs.
 *
 */
PUBLIC void son_ald_record_set_free_descs(struct wlan_objmgr_psoc *psoc,
					  u_int32_t descs);

/**
 * @brief To get ald record free desc.
 *
 * @param [in] psoc.
 *
 */
PUBLIC u_int32_t son_ald_record_get_free_descs(struct wlan_objmgr_psoc *psoc);

/**
 * @brief To set/reset ald record bufful warn.
 *
 * @param [in] psoc.
 * @param [in] enable - 1-enable, 0-disable
 *
 */
PUBLIC void son_ald_record_set_buff_full_warn(struct wlan_objmgr_psoc *psoc,
					      u_int8_t enable);

/**
 * @brief To get ald record bufful warn.
 *
 * @param [in] psoc.
 *
 */
PUBLIC u_int8_t son_ald_record_get_buff_full_warn(struct wlan_objmgr_psoc *psoc);

/**
 * @brief To set ald record free buff lvl.
 *
 * @param [in] psoc.
 * @param [in] thres - threshold.
 *
 */
PUBLIC void son_ald_record_set_buff_lvl(struct wlan_objmgr_psoc *psoc,
					int thres);

/**
 * @brief To get ald record free buff lvl.
 *
 * @param [in] psoc.
 *
 */
PUBLIC u_int16_t son_ald_record_get_buff_lvl(struct wlan_objmgr_psoc *psoc);

/**
 * @brief To set ald record pool size.
 *
 * @param [in] psoc.
 * @param [in] sz - pool size.
 *
 */
PUBLIC void son_ald_record_set_pool_size(struct wlan_objmgr_psoc *psoc,
					 u_int32_t sz);

/**
 * @brief To get ald record pool size.
 *
 * @param [in] psoc.
 *
 */
PUBLIC u_int32_t son_ald_record_get_pool_size(struct wlan_objmgr_psoc *psoc);

/**
 * @brief To indicate event using BSTEER NL.
 *
 * @param [in] vdev the VAP.
 * @param [in] type band steering event type.
 * @param [in] len length of event.
 * @param [in] data event data.
 *
 * @return void.
 */
PUBLIC void son_notify_user(struct wlan_objmgr_vdev *vdev,
			    ATH_BSTEERING_EVENT type,
			    uint32_t len, const char *data);

#endif /* __WLAN_SON_PUB_H__*/
