/*
 * Copyright (c) 2011,2017-2019,2021 Qualcomm Innovation Center, Inc.
 * All Rights Reserved
 * Confidential and Proprietary - Qualcomm Innovation Center, Inc.
 *
 * 2011 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 *
 *  Copyright (c) 2008 Atheros Communications Inc.
 * All Rights Reserved.
 */

#ifndef _NET80211_IEEE80211_CHAN_H
#define _NET80211_IEEE80211_CHAN_H

#include <ieee80211_var.h>

/* enum to define secondary 20 channel offset
 * @EXT_CHAN_OFFSET_NA: no extension channel is present
 * @EXT_CHAN_OFFSET_ABOVE: above control channel
 * @EXT_CHAN_OFFSET_BELOW: below control channel
 */
enum sec20_chan_offset {
        EXT_CHAN_OFFSET_NA    = 0,
        EXT_CHAN_OFFSET_ABOVE = 1,
        EXT_CHAN_OFFSET_BELOW = -1
};

/*
 * Internal API's for channel/freq/PHY_mode handling
 */

enum ieee80211_phymode ieee80211_chan2mode(const struct ieee80211_ath_channel *chan);
struct ieee80211_ath_channel *ieee80211_find_channel(struct ieee80211com *ic, int freq, uint16_t des_cfreq2, u_int64_t flags);
/**
 * wlan_update_current_mode_caps() - Update current mode capabilities based on
 * ic_channels list. Do not populate the radar found channel capabilities.
 * @ic: Pointer to ieee80211com structure.
 */
void wlan_update_current_mode_caps(struct ieee80211com *ic);

#if WLAN_SUPPORT_PRIMARY_ALLOWED_CHAN
bool ieee80211_check_allowed_prim_freqlist(struct ieee80211com *ic, uint16_t freq);
#endif

enum ieee80211_phymode ieee80211_get_phy_mode(struct ieee80211com *ic,
                                        struct ieee80211_ath_channel *bss_chan,
                                        enum ieee80211_phymode des_mode, enum ieee80211_phymode bss_mode);

void ieee80211_dcs_acs_event_handler(void *arg, wlan_chan_t channel);

void ieee80211_channel_notify_to_app(struct ieee80211com *ic);

int ieee80211_find_any_valid_channel(struct ieee80211com *ic,  u_int64_t chan_mode);

struct ieee80211_ath_channel *ieee80211_doth_findchan(struct ieee80211vap *vap, uint16_t chan_freq);
struct ieee80211_ath_channel *ieee80211_find_dot11_channel(struct ieee80211com *ic, uint16_t freq, uint16_t cfreq2, enum ieee80211_phymode mode);
int ieee80211_set_channel_for_cc_change(struct ieee80211com *ic, struct ieee80211_ath_channel *chan);

/**
 * ieee80211_setctry_tryretaining_curchan() - Set the country and try to retain
 * the input channel.
 * @ic: Pointer to ieee80211com structure.
 * @chan: Pointer to the channel that needs to be retained after setting the
 * country.
 */
int ieee80211_setctry_tryretaining_curchan(struct ieee80211com *ic, struct ieee80211_ath_channel *chan);

bool ieee80211_is_same_frequency_band(const struct ieee80211_ath_channel *chan1, const struct ieee80211_ath_channel *chan2);
int ieee80211_update_channellist(struct ieee80211com *ic, int exclude_11d,
                                 bool no_chanchange);
bool ieee80211_update_custom_scan_chan_list(struct ieee80211vap *vap, bool associated);
struct ieee80211_ath_channel *ieee80211_autoselect_adhoc_channel(struct ieee80211com *ic);
int ieee80211_set_channel(struct ieee80211com *ic, struct ieee80211_ath_channel *chan);
int ieee80211_setmode(struct ieee80211com *ic, enum ieee80211_phymode mode, enum ieee80211_opmode opmode);
struct ieee80211_ath_channel * ieee80211_doth_findchan(struct ieee80211vap *vap, uint16_t chan_freq);
/*
 * ieee80211_chan2freq() - Convert channel to frequency value.
 * @ic: Pointer to ieee80211com structure
 * @c: Pointer to ieee80211_ath_channel structure
 *
 * Return: Frequency value on successful conversion
 *         "0" otherwise
 */
u_int16_t
ieee80211_chan2freq(struct ieee80211com *ic, const struct ieee80211_ath_channel *c);
struct ieee80211_ath_channel * ieee80211_get_extchan(struct ieee80211com *ic);

extern int ieee80211_get_chan_width(struct ieee80211_ath_channel *chan);
enum phy_ch_width
ieee80211_get_phy_chan_width(struct ieee80211_ath_channel *chan);
extern int ieee80211_get_chan_centre_freq(struct ieee80211com *ic,
    struct ieee80211_ath_channel *chan);
extern void    ieee80211_get_extchaninfo(struct ieee80211com *ic, struct ieee80211_ath_channel *chan, struct ieee80211_ath_channel_list *chan_info);
int
ieee80211_check_chan_mode_consistency(struct ieee80211com *ic,int mode,struct ieee80211_ath_channel *c);
u_int8_t ieee80211_is_extchan_144(struct ieee80211com *ic,struct ieee80211_ath_channel *chan, u_int8_t extchan);
enum sec20_chan_offset ieee80211_secondary20_channel_offset(struct ieee80211_ath_channel *chan);

enum ieee80211_mode ieee80211_get_mode(const struct ieee80211_ath_channel *ch);

enum ieee80211_phymode ieee80211_get_composite_phymode (enum ieee80211_mode mode,
       enum ieee80211_cwm_width chwidth, uint8_t secchanoffset);

enum ieee80211_mode get_mode_from_phymode(enum ieee80211_phymode phymode);

enum ieee80211_cwm_width
ieee80211_get_cwm_width_from_channel(struct ieee80211_ath_channel *chan);

int ieee80211_check_overlap(int f1,
                     int w1,
                     int f2,
                     int w2);

void ieee80211_dfs_channel_change_by_precac(struct ieee80211com *ic);

void ieee80211_update_dfs_next_channel(struct ieee80211com *ic);

void ieee80211_vap_iter_update_des_chan(void *arg, struct ieee80211vap *vap);

void ieee80211_postnol_chan_switch(struct ieee80211com *ic,
                                   uint16_t des_freq,
				   uint16_t des_cfreq2,
                                   enum ieee80211_phymode des_mode);

void ieee80211_dfs_nol_timeout_notification(struct ieee80211com *ic);

int ieee80211_check_channel_overlap(struct ieee80211com *ic,
                                    struct ieee80211_ath_channel *chan,
                                    int nol_freq,
                                    int nol_width);

int ieee80211_dfs_get_cac_timeout(struct ieee80211com *ic,
                                  struct ieee80211_ath_channel *chan);

int ieee80211_reg_get_current_chan_list(struct ieee80211com *ic,
        struct regulatory_channel *curr_chan_list);

int ieee80211_reg_create_ieee_chan_list(struct ieee80211com *ic);

int ieee80211_reg_program_cc(struct ieee80211com *ic,
        char *isoName, u_int16_t cc);

/**
 * ieee80211_reg_program_opclass_tbl() - Set opclass table value.
 * @ic: Pointer to struct ieee80211com.
 * @opclass: Opclass table value.
 *
 * Return: 0 is success, else -EINVAL.
 */
int ieee80211_reg_program_opclass_tbl(struct ieee80211com *ic, uint8_t opclass);

/**
 * ieee80211_reg_get_opclass_tbl() - Get opclass table value.
 * @ic: Pointer to struct ieee80211com.
 * @opclass: Opclass table value.
 *
 * Return: 0 is success, else -EINVAL.
 */
int ieee80211_reg_get_opclass_tbl(struct ieee80211com *ic, uint8_t *opclass);

void ieee80211_get_current_regdomain(struct ieee80211com *ic,
        struct cur_regdmn_info *cur_regdmn);

enum ieee80211_phymode
ieee80211_get_phymode_from_chan_flag(
        struct ieee80211_ath_channel *curchan,
        uint64_t chan_flag);

INLINE static u_int64_t
ieee80211_chan_flags(struct ieee80211_ath_channel *chan)
{
    return chan->ic_flags;
}

INLINE static u_int8_t
ieee80211_chan_flagext(struct ieee80211_ath_channel *chan)
{
    return chan->ic_flagext;
}

extern const char *ieee80211_phymode_name[];

/*
 * Convert channel to IEEE channel number.
 */
static INLINE u_int8_t
ieee80211_chan2ieee(struct ieee80211com *ic, const struct ieee80211_ath_channel *c)
{
    if (c == NULL) {
        return 0;       /* XXX */
    }
    return (c == IEEE80211_CHAN_ANYC ?
            (u_int8_t)IEEE80211_CHAN_ANY : c->ic_ieee);
}

const char *ieee80211_phymode_to_name( enum ieee80211_phymode mode);

/*
 * Iterator for channel list
 */
#define ieee80211_enumerate_channels(_c, _ic, _index)    \
    for ((_index) = 0, (_c) = (_ic)->ic_channels;        \
         (_index) < (_ic)->ic_nchans;                    \
         (_index)++, (_c)++)

/*
 * Get channel by channel index
 */
#define ieee80211_ath_get_channel(_ic, _index)  (&((_ic)->ic_channels[(_index)]))

#define ieee80211_get_current_channel(_ic)  ((_ic)->ic_curchan)
#define ieee80211_get_home_channel(_vap)     ((_vap)->iv_bsschan)

/*
 * Get current operating PHY mode
 */
static INLINE enum ieee80211_phymode
ieee80211_get_current_phymode(struct ieee80211com *ic)
{
    return ieee80211_chan2mode(ic->ic_curchan);
}

/*
 * Set number of channels
 */
static INLINE void
ieee80211_set_nchannels(struct ieee80211com *ic, int nchans)
{
    ic->ic_nchans = nchans;
}

int wlan_get_target_phymode(struct ieee80211com *ic, uint32_t phymode,
                            bool is_2gvht_en);

/*
 * Set the channel
 */
#define IEEE80211_CHAN_SETUP(_c, _ieee, _freq, _flags, _extflags,  \
                            _maxregp, _maxp, _minp, _id)\
    do {                                                \
        (_c)->ic_freq = (_freq);                        \
        (_c)->ic_ieee = (_ieee);                        \
        (_c)->ic_flags = (_flags);                      \
        (_c)->ic_flagext = (_extflags);                 \
        (_c)->ic_maxregpower = (_maxregp);              \
        (_c)->ic_maxpower = (_maxp);                    \
        (_c)->ic_minpower = (_minp);                    \
        (_c)->ic_regClassId = (_id);                    \
    } while (0)

/*
 * Compare two channels
 */
#define IEEE80211_CHAN_MATCH(_c, _freq, _flags, _mask)      \
    (((_c)->ic_freq == (_freq)) &&                          \
     (((_c)->ic_flags & (_mask)) == ((_flags) & (_mask))))

/*
 * Check whether a phymode is supported
 */
#define IEEE80211_SUPPORT_PHY_MODE(_ic, _mode)  \
    (((_ic)->ic_modecaps & (1ULL << (_mode))) && \
    (((_ic)->ic_support_phy_mode == NULL) || \
    ((_ic)->ic_support_phy_mode((_ic), (_mode)))))

/*
 * Check whether a phymode is accepted
 */
#define IEEE80211_ACCEPT_PHY_MODE(_vap, _mode)  \
    ((_vap)->iv_des_modecaps & (1 << (_mode)))

#define IEEE80211_ACCEPT_ANY_PHY_MODE(_vap)     \
    ((_vap)->iv_des_modecaps & (1 << IEEE80211_MODE_AUTO))

#define IEEE80211_ACCEPT_PHY_MODE_11G(_vap)     \
    ((_vap)->iv_des_modecaps & (1 << IEEE80211_MODE_11G))

#define IEEE80211_ACCEPT_PHY_MODE_11A(_vap)     \
    ((_vap)->iv_des_modecaps & (1 << IEEE80211_MODE_11A))

/*
 * ieee80211_restart_vaps_with_non_dfs_channels() - Restart vaps with a non DFS
 * channel.
 * @ic - pointer to ieee80211com structure.
 * @no_chans_avail - 1 indicates no non-dfs channel is available.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
void ieee80211_restart_vaps_with_non_dfs_channels(struct ieee80211com *ic,
        int no_chans_avail);
#endif /* HOST_DFS_SPOOF_TEST */

enum phy_ch_width
ieee80211_get_chan_width_from_phymode(enum ieee80211_phymode mode);

/*
 * ieee80211_bringdown_vaps():
 * If there are no valid channels for vap to come up
 * (Reasons could be:
 * 1.Regdmn had only DFS channels and all DFS channels are in NOL.
 * 2.DFS Violation and hence it is mandated to have only non-DFS channel in
 * the radio and regdmn does not support non-DFS channels.)
 * bringdown all the vaps.
 * @ic - pointer to ieee80211com structure.
 */

void ieee80211_bringdown_vaps(struct ieee80211com *ic);

/*
 * ieee80211_bringdown_sta_vap();
 * If the sta vap has to be brought down.
 * @ic - pointer to ieee80211com structure.
 */

void ieee80211_bringdown_sta_vap(struct ieee80211com *ic);

void ieee80211_update_vdev_chan(struct wlan_channel *vdev_chan,
        struct ieee80211_ath_channel *curchan);

/*
 * ieee80211_dfs_deliver_event: Deliver DFS radar hit event
 * @ic: ic object
 * @freq: Frequency radar hit on
 * @event: Event being sent
 */
void ieee80211_dfs_deliver_event(struct ieee80211com *ic, u_int16_t freq, enum WLAN_DFS_EVENTS event);

/*
 * ieee80211_dfs_deliver_events: Deliver DFS radar hit event to channel list pointed by @chan
 * @ic: ic object
 * @chan: Channel list to send events to
 * @event: Event being sent
 */

void ieee80211_dfs_deliver_events(struct ieee80211com *ic, struct ieee80211_ath_channel *chan, enum WLAN_DFS_EVENTS event);

enum wlan_band_id ieee80211_get_ath_channel_band(struct ieee80211_ath_channel *chan);

enum reg_wifi_band wlan_band_id_to_reg_wifi_band(enum wlan_band_id wlan_band);

enum wlan_band_id reg_wifi_band_to_wlan_band_id(enum reg_wifi_band reg_wifi_band);

uint16_t wlan_get_wlan_band_id_chan_to_freq(struct wlan_objmgr_pdev *pdev,
        uint32_t chan, enum wlan_band_id wlan_band);

/*
 * wlan_band_id_chan_to_freq_from_cur_chan_list : Get IEEE Channel number and
 * band as the input and give channel center frequency as the output by searching
 * the regulatory current channel list.
 * @ic: Pointer to ic.
 * @chan: IEEE channel number
 * @wlan_band: band
 */
qdf_freq_t wlan_band_id_chan_to_freq_from_cur_chan_list(struct ieee80211com *ic,
                                                        uint32_t chan,
                                                        enum wlan_band_id wlan_band);
uint64_t ieee80211_get_band_flag(uint16_t freq);

#define IEEE80211_ARE_CHANS_INTERWIDEBAND(__chan1, __chan2)   \
                      ((IEEE80211_IS_CHAN_5GHZ(__chan1) &&    \
                        IEEE80211_IS_CHAN_6GHZ(__chan2)) ||   \
                       (IEEE80211_IS_CHAN_6GHZ(__chan1) &&    \
                        IEEE80211_IS_CHAN_5GHZ(__chan2)))

#define IEEE80211_ARE_CHANS_INTRABAND(__chan1, __chan2, band) \
                      (IEEE80211_IS_CHAN_##band(__chan1) &&   \
                       IEEE80211_IS_CHAN_##band(__chan2))

/*
 * wlan_dcs_wideband_policy_t:
 * Specfies the policy codes for the DCS wideband policy.
 * The policy needs to be used in the OL layer for DCS and UMAC layer for the
 * channel selection algorithm.
 */
typedef enum {
	DCS_WIDEBAND_POLICY_INTRABAND = 0,
	DCS_WIDEBAND_POLICY_INTERBAND = 1,
	DCS_WIDEBAND_POLICY_INVALID,
} wlan_dcs_wideband_policy_t;

/**
 * ieee80211_is_phymode_supported_by_channel() - Check the regulatory and device
 * support for given mode and channel combination.
 * @ic: Pointer to ieee80211com.
 * @freq: Frequency to check.
 * @mode: Phymode to check.
 *
 * Return true if channel supports given phymode, else false.
 */
bool ieee80211_is_phymode_supported_by_channel(struct ieee80211com *ic,
                                               qdf_freq_t freq,
                                               enum ieee80211_phymode mode);

/**
 * ieee80211_get_subchannels() - Find the subchannels given a primary frequency
 * and mode from regulatory.
 * @ic: Pointer to ieee80211com.
 * @freq: Primary frequency of the channel.
 * @mode: Phymode of the channel.
 * @subchannels: Array of 20MHZ frequencies in the channel.
 *
 * Return the count of subchannels, 0 if none found.
 */
int ieee80211_get_subchannels(struct ieee80211com *ic,
                              qdf_freq_t freq,
                              enum ieee80211_phymode mode,
                              qdf_freq_t *subchannels);
#endif /* _NET80211_IEEE80211_CHAN_H */
