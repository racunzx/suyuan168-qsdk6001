/*
 * Copyright (c) 2017 Qualcomm Innovation Center, Inc.
 * All Rights Reserved
 * Confidential and Proprietary - Qualcomm Innovation Center, Inc.
 */

#ifndef _IEEE80211_MLME_DFS_INTERFACE_H_
#define _IEEE80211_MLME_DFS_INTERFACE_H_

#include <wlan_objmgr_pdev_obj.h>

/**
 * mlme_dfs_start_rcsa() - Send RCSA to RootAP.
 * @pdev: Pointer to DFS pdev object.
 */
QDF_STATUS mlme_dfs_start_rcsa(struct wlan_objmgr_pdev *pdev,
		bool *wait_for_csa);

/**
 * mlme_dfs_mark_dfs_for_freq() - Mark the channel in the channel list.
 * @pdev: Pointer to DFS pdev object.
 * @ieee: Channel number.
 * @freq: Channel frequency.
 * @vhtop_ch_freq_seg2: VHT80 Cfreq2.
 * @flags: channel flags.
 */
QDF_STATUS mlme_dfs_mark_dfs(struct wlan_objmgr_pdev *pdev,
			     uint8_t ieee,
			     uint16_t freq,
			     uint16_t vhtop_ch_freq_seg2_mhz,
			     uint64_t flags);

/**
 * mlme_dfs_start_csa_for_freq() - Sends CSA in ieeeChan
 * @pdev: Pointer to DFS pdev object.
 * @ieeeChan: Channel number.
 * @freq: Channel frequency.
 * @cfreq2: VHT80 Cfreq2 in Mhz.
 * @flags: channel flags.
*/
QDF_STATUS mlme_dfs_start_csa_for_freq(struct wlan_objmgr_pdev *pdev,
				       uint8_t ieeeChan,
				       uint16_t freq,
				       uint16_t cfreq2,
				       uint64_t flags);

/**
 * mlme_dfs_proc_cac() - Process the CAC completion event.
 * @pdev: Pointer to DFS pdev object.
 */
QDF_STATUS mlme_dfs_proc_cac(struct wlan_objmgr_pdev *pdev);

/**
 * mlme_dfs_get_dfs_ch_nchans() - Get number of channels in the channel list
 * @pdev: Pointer to DFS pdev object.
 * @nchans: Pointer to save the channel number.
 */
QDF_STATUS mlme_dfs_get_dfs_ch_nchans(struct wlan_objmgr_pdev *pdev,
		int *nchans);

/**
 * mlme_dfs_get_extchan_for_freq() - Get extension channel.
 * @pdev: Pointer to DFS pdev object.
 * @dfs_ch_freq:                Frequency in Mhz.
 * @dfs_ch_flags:               Channel flags.
 * @dfs_ch_flagext:             Extended channel flags.
 * @dfs_ch_ieee:                IEEE channel number.
 * @dfs_ch_vhtop_ch_freq_seg1:  Channel Center IEEE.
 * @dfs_ch_vhtop_ch_freq_seg2:  Channel Center IEEE applicable for 80+80MHz
 *                          mode of operation.
 * @dfs_ch_mhz_freq_seg1: Primary channel center frequency.
 * @dfs_ch_mhz_freq_seg2: Secondary channel center frequency applicable for
 *                        80+80 MHZ mode of operation.
 */
QDF_STATUS mlme_dfs_get_extchan_for_freq(struct wlan_objmgr_pdev *pdev,
					 uint16_t *dfs_ch_freq,
					 uint64_t *dfs_ch_flags,
					 uint16_t *dfs_ch_flagext,
					 uint8_t *dfs_ch_ieee,
					 uint8_t *dfs_ch_vhtop_ch_freq_seg1,
					 uint8_t *dfs_ch_vhtop_ch_freq_seg2,
					 uint16_t *dfs_ch_mhz_freq_seg1,
					 uint16_t *dfs_ch_mhz_freq_seg2);

/**
 * mlme_dfs_set_no_chans_available() - Set no_chans_available flag.
 * @pdev: Pointer to DFS pdev object.
 * @val: Set this value to no_chans_available flag.
 */
QDF_STATUS mlme_dfs_set_no_chans_available(struct wlan_objmgr_pdev *pdev,
		int val);

/**
 * mlme_dfs_find_dot11_channel_for_freq() - Get dot11 channel from ieee, cfreq2
 * and mode.
 * @pdev: Pointer to DFS pdev object.
 * @ieee: Channel number.
 * @des_cfreq2: cfreq2
 * @mode: Phymode
 * @dfs_ch_freq:                Frequency in Mhz.
 * @dfs_ch_flags:               Channel flags.
 * @dfs_ch_flagext:             Extended channel flags.
 * @dfs_ch_ieee:                IEEE channel number.
 * @dfs_ch_vhtop_ch_freq_seg1:  Channel Center frequency.
 * @dfs_ch_vhtop_ch_freq_seg2:  Channel Center frequency applicable for 80+80MHz
 *                          mode of operation.
 * @dfs_ch_mhz_freq_seg1:  Primary Channel Center frequency
 * @dfs_ch_mhz_freq_seg2:  Channel Center frequency applicable for 80+80MHz
 *                          mode of operation.
 */
QDF_STATUS mlme_dfs_find_dot11_chan_for_freq(struct wlan_objmgr_pdev *pdev,
		uint16_t freq,
		uint16_t des_cfreq2_mhz,
		int mode,
		uint16_t *dfs_ch_freq,
		uint64_t *dfs_ch_flags,
		uint16_t *dfs_ch_flagext,
		uint8_t *dfs_ch_ieee,
		uint8_t *dfs_ch_vhtop_ch_freq_seg1,
		uint8_t *dfs_ch_vhtop_ch_freq_seg2,
		uint16_t *dfs_ch_mhz_freq_seg1,
		uint16_t *dfs_ch_mhz_freq_seg2);

/**
 * mlme_dfs_get_dfs_channels_for_freq() - Get channel from channel list.
 * @pdev: Pointer to DFS pdev object.
 * @dfs_ch_freq:                Frequency in Mhz.
 * @dfs_ch_flags:               Channel flags.
 * @dfs_ch_flagext:             Extended channel flags.
 * @dfs_ch_ieee:                IEEE channel number.
 * @dfs_ch_vhtop_ch_freq_seg1:  Channel Center frequency.
 * @dfs_ch_vhtop_ch_freq_seg2:  Channel Center frequency applicable for 80+80MHz
 *                          mode of operation.
 * @index: Index into channel list.
 */
QDF_STATUS mlme_dfs_get_dfs_channels_for_freq(struct wlan_objmgr_pdev *pdev,
		uint16_t *dfs_ch_freq,
		uint64_t *dfs_ch_flags,
		uint16_t *dfs_ch_flagext,
		uint8_t *dfs_ch_ieee,
		uint8_t *dfs_ch_vhtop_ch_freq_seg1,
		uint8_t *dfs_ch_vhtop_ch_freq_seg2,
		uint16_t *dfs_ch_mhz_freq_seg1,
		uint16_t *dfs_ch_mhz_freq_seg2,
		int index);

/**
 * mlme_dfs_dfs_ch_flags_ext() - Get extension channel flags.
 * @pdev: Pointer to DFS pdev object.
 */
QDF_STATUS mlme_dfs_dfs_ch_flags_ext(struct wlan_objmgr_pdev *pdev,
		uint16_t *flag_ext);

/**
 * mlme_dfs_channel_change_by_precac() - Channel change by PreCAC.
 * @pdev: Pointer to DFS pdev object.
 */
QDF_STATUS mlme_dfs_channel_change_by_precac(struct wlan_objmgr_pdev *pdev);

/**
 * mlme_dfs_deliver_event_up_after_cac() - Send a CAC timeout, VAP up event to
 *                                         userspace.
 * @pdev: Pointer to DFS pdev object.
 */
QDF_STATUS mlme_dfs_deliver_event_up_after_cac(struct wlan_objmgr_pdev *pdev);

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
/**
 * mlme_dfs_precac_chan_change_csa_for_freq() - Channel change by PreCAC.
 * @pdev:      Pointer to DFS pdev object.
 * @des_chan:  Desired Channel frequency.
 * @des_mode:  Desired channel.
 */

QDF_STATUS mlme_dfs_precac_chan_change_csa_for_freq(struct wlan_objmgr_pdev *pdev,
						    uint16_t des_chan_freq,
						    enum wlan_phymode des_mode);
#endif

#ifdef QCA_SUPPORT_DFS_CHAN_POSTNOL
/**
 * mlme_dfs_postnol_chan_switch() - Channel change post NOL.
 * @pdev:           Pointer to DFS pdev object.
 * @des_chan_freq:  Desired Channel frequency.
 * @des_cfreq2:     Desired secondary center frequency.
 * @des_mode:       Desired channel.
 */

QDF_STATUS mlme_dfs_postnol_chan_switch(struct wlan_objmgr_pdev *pdev,
					uint16_t des_chan_freq,
					uint16_t des_cfreq2,
					enum wlan_phymode des_mode);
#endif
/**
 * mlme_dfs_nol_timeout_notification() - NOL timeout notification to userspace.
 * @pdev: Pointer to DFS pdev object.
 */
QDF_STATUS mlme_dfs_nol_timeout_notification(struct wlan_objmgr_pdev *pdev);

/**
 * mlme_dfs_ieee2mhz() - Get the frequency from channel number.
 * @pdev: Pointer to DFS pdev object.
 * @ieee: Channel number.
 * @flag: Channel flag.
 */
QDF_STATUS mlme_dfs_ieee2mhz(struct wlan_objmgr_pdev *pdev,
		int ieee,
		uint64_t flag,
		int *freq);

/**
 * mlme_dfs_clist_update() - Mark the channel as RADAR.
 * @pdev: Pointer to DFS pdev object.
 * @nollist: Pointer to NOL list.
 * @nentries: Number of channels in the NOL list.
 */
QDF_STATUS mlme_dfs_clist_update(struct wlan_objmgr_pdev *pdev,
		void *nollist,
		int nentries);

/**
 * mlme_dfs_get_cac_timeout_for_freq() - Get cac_timeout.
 * @pdev: Pointer to DFS pdev object.
 * @dfs_ch_freq:                Frequency in Mhz.
 * @dfs_ch_vhtop_ch_freq_seg2:  Channel Center frequency applicable for 80+80MHz
 *                              mode of operation.
 * @dfs_ch_flags:               Channel flags.
 */

QDF_STATUS mlme_dfs_get_cac_timeout_for_freq(struct wlan_objmgr_pdev *pdev,
		uint16_t dfs_ch_freq,
		uint16_t dfs_ch_vhtop_ch_freq_seg2,
		uint64_t dfs_ch_flags,
		int *cac_timeout);

/**
 * mlme_dfs_is_opmode_sta() - Check if ic_opmode is STA.
 * @pedv: Pointer to DFS pdev object.
 *
 * Return: true if ic_opmode is STA, else false.
 */
bool mlme_dfs_is_opmode_sta(struct wlan_objmgr_pdev *pdev);

/**
 * mlme_dfs_rebuild_chan_list_with_non_dfs_channels() - rebuild channel list
 * with non DFS channels.
 * @pdev: Pointer to DFS pdev object.
 */
QDF_STATUS mlme_dfs_rebuild_chan_list_with_non_dfs_channels(
		struct wlan_objmgr_pdev *pdev);

/**
 * mlme_dfs_restart_vaps_with_non_dfs_chan() - choose a non DFS channel and
 * restart vaps in that channel.
 * @pdev: Pointer to DFS pdev object.
 * @no_chans_avail: indicates if no channels are available.
 */
QDF_STATUS mlme_dfs_restart_vaps_with_non_dfs_chan(
		struct wlan_objmgr_pdev *pdev, int no_chans_avail);


/**
 * mlme_dfs_check_allowed_prim_chanlist - To check whether the given channel is
 *                                        present in the primary allowed channel
 *                                        list or not.
 * @pedv: Pointer to DFS pdev object.
 * @chan_num: Channel number
 */
#if WLAN_SUPPORT_PRIMARY_ALLOWED_CHAN
bool mlme_dfs_check_allowed_prim_chanlist(struct wlan_objmgr_pdev *pdev,
                uint32_t chan_num);
#else
static inline bool mlme_dfs_check_allowed_prim_chanlist(
				struct wlan_objmgr_pdev *pdev, uint32_t chan_num)
{
		return true;
}
#endif /* WLAN_SUPPORT_PRIMARY_ALLOWED_CHAN*/

/**
 * mlme_dfs_bringdown_vaps() - bring down all vaps if
 * there is no channel available.
 * @pdev: Pointer to DFS pdev object.
 */
QDF_STATUS mlme_dfs_bringdown_vaps(struct wlan_objmgr_pdev *pdev);

/**
 * mlme_dfs_deliver_event() - Deliver DFS events to user space
 * @pdev: Pointer to DFS pdev object.
 * @freq: Frequency Radar Hit on
 * @event: Event being sent
 */
void mlme_dfs_deliver_event(struct wlan_objmgr_pdev *pdev,
				  u_int16_t freq, enum WLAN_DFS_EVENTS event);

/**
 * mlme_dfs_update_scan_channel_list() - update scan channel list.
 * @pdev: Pointer to pdev object.
 */
#if defined(WLAN_DFS_FULL_OFFLOAD) && defined(QCA_DFS_NOL_OFFLOAD)
QDF_STATUS mlme_dfs_update_scan_channel_list(
		struct wlan_objmgr_pdev *pdev);
#else
static inline QDF_STATUS mlme_dfs_update_scan_channel_list(
		struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif

void mlme_acquire_radar_mode_switch_lock(struct wlan_objmgr_pdev *pdev);

void mlme_release_radar_mode_switch_lock(struct wlan_objmgr_pdev *pdev);

/**
 * mlme_is_inter_band_chan_switch_allowed() - Check if inter-band channel
 * switch is allowed.
 * @pdev: Pointer to pdev object.
 */
bool mlme_is_inter_band_chan_switch_allowed(struct wlan_objmgr_pdev *pdev);

/**
 * mlme_dfs_proc_spoof_success() - Process the Spoof status success from FW.
 * @pdev: Pointer to DFS pdev object.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
QDF_STATUS mlme_dfs_proc_spoof_success(struct wlan_objmgr_pdev *pdev);
#else
static inline QDF_STATUS
mlme_dfs_proc_spoof_success(struct wlan_objmgr_pdev *pdev)
{
    return QDF_STATUS_SUCCESS;
}
#endif
#endif
