/*
 * Copyright (c) 2017, 2020 Qualcomm Innovation Center, Inc.
 * All Rights Reserved
 * Confidential and Proprietary - Qualcomm Innovation Center, Inc.
 *
 * Copyright (c) 2011, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * LMAC offload interface functions for UMAC - for power and performance offload model
 */
#include "ol_if_athvar.h"
#include <ol_if_athpriv.h>
#ifdef WIFI_TARGET_TYPE_2_0
#include "sw_version.h"
#endif
#include "targaddrs.h"
#include "ol_helper.h"

#if ATH_PERF_PWR_OFFLOAD

typedef struct _ol_ath_rate_table {
    u_int32_t num_rates;
    struct {
        u_int8_t    valid;      /* valid for rate control use */
        u_int8_t    phy;        /* IEEE80211_T_DS/IEEE80211_T_OFDM/XR */
        u_int8_t    dot11_rate; /* value for supported rates. info element of MLME */
    } info[IEEE80211_RATE_MAXSIZE];
} ol_ath_rate_table;

ol_ath_rate_table ol_if_11a_table = {
	8,  /* number of rates */
	{
/*--- IEEE80211_T_OFDM rates ---*/
/*   6 Mb */ {  TRUE, IEEE80211_T_OFDM,  (0x80|12)},
/*   9 Mb */ {  TRUE, IEEE80211_T_OFDM,  18       },
/*  12 Mb */ {  TRUE, IEEE80211_T_OFDM,  (0x80|24)},
/*  18 Mb */ {  TRUE, IEEE80211_T_OFDM,  36       },
/*  24 Mb */ {  TRUE, IEEE80211_T_OFDM,  (0x80|48)},
/*  36 Mb */ {  TRUE, IEEE80211_T_OFDM,  72       },
/*  48 Mb */ {  TRUE, IEEE80211_T_OFDM,  96       },
/*  54 Mb */ {  TRUE, IEEE80211_T_OFDM,  108      }
	},
};

ol_ath_rate_table ol_if_11a_halfrate_table = {
	8,  /* number of rates */
	{
/*--- IEEE80211_T_OFDM rates ---*/
/*   6 Mb */ {  TRUE, IEEE80211_T_OFDM,  (0x80|6)},
/*   9 Mb */ {  TRUE, IEEE80211_T_OFDM,  9       },
/*  12 Mb */ {  TRUE, IEEE80211_T_OFDM,  (0x80|12)},
/*  18 Mb */ {  TRUE, IEEE80211_T_OFDM,  18       },
/*  24 Mb */ {  TRUE, IEEE80211_T_OFDM,  (0x80|24)},
/*  36 Mb */ {  TRUE, IEEE80211_T_OFDM,  36       },
/*  48 Mb */ {  TRUE, IEEE80211_T_OFDM,  48       },
/*  54 Mb */ {  TRUE, IEEE80211_T_OFDM,  54      }
	},
};

ol_ath_rate_table ol_if_11a_quarterrate_table = {
	8,  /* number of rates */
	{
/*--- IEEE80211_T_OFDM rates ---*/
/*   6 Mb */ {  TRUE, IEEE80211_T_OFDM,  (0x80|3)},
/*   9 Mb */ {  TRUE, IEEE80211_T_OFDM,  4       },
/*  12 Mb */ {  TRUE, IEEE80211_T_OFDM,  (0x80|6)},
/*  18 Mb */ {  TRUE, IEEE80211_T_OFDM,  9       },
/*  24 Mb */ {  TRUE, IEEE80211_T_OFDM,  (0x80|12)},
/*  36 Mb */ {  TRUE, IEEE80211_T_OFDM,  18       },
/*  48 Mb */ {  TRUE, IEEE80211_T_OFDM,  24       },
/*  54 Mb */ {  TRUE, IEEE80211_T_OFDM,  27      }
	},
};

ol_ath_rate_table ol_if_11b_table = {
	4,  /* number of rates */
	{
/*   1 Mb */ {  TRUE,  IEEE80211_T_DS,  (0x80| 2)},
/*   2 Mb */ {  TRUE,  IEEE80211_T_DS,  (0x80| 4)},
/* 5.5 Mb */ {  TRUE,  IEEE80211_T_DS,  (0x80|11)},
/*  11 Mb */ {  TRUE,  IEEE80211_T_DS,  (0x80|22)}
	},
};


ol_ath_rate_table ol_if_11g_table = {
	12,  /* number of rates */
	{
/*   1 Mb */ {  TRUE, IEEE80211_T_DS,   (0x80| 2)},
/*   2 Mb */ {  TRUE, IEEE80211_T_DS,   (0x80| 4)},
/* 5.5 Mb */ {  TRUE, IEEE80211_T_DS,   (0x80|11)},
/*  11 Mb */ {  TRUE, IEEE80211_T_DS,   (0x80|22)},
/* Hardware workaround - remove rates 6, 9 from rate ctrl */
/*--- IEEE80211_T_OFDM rates ---*/
/*   6 Mb */ { FALSE, IEEE80211_T_OFDM,  12},
/*   9 Mb */ { FALSE, IEEE80211_T_OFDM,  18},
/*  12 Mb */ {  TRUE, IEEE80211_T_OFDM,  24},
/*  18 Mb */ {  TRUE, IEEE80211_T_OFDM,  36},
/*  24 Mb */ {  TRUE, IEEE80211_T_OFDM,  48},
/*  36 Mb */ {  TRUE, IEEE80211_T_OFDM,  72},
/*  48 Mb */ {  TRUE, IEEE80211_T_OFDM,  96},
/*  54 Mb */ {  TRUE, IEEE80211_T_OFDM,  108}
	},
};


ol_ath_rate_table ol_if_11ng_table = {

    44,  /* number of rates */
	{
/*   1 Mb */ {  TRUE, IEEE80211_T_DS,   (0x80| 2)},
/*   2 Mb */ {  TRUE, IEEE80211_T_DS,   (0x80| 4)},
/* 5.5 Mb */ {  TRUE, IEEE80211_T_DS,   (0x80|11)},
/*  11 Mb */ {  TRUE, IEEE80211_T_DS,   (0x80|22)},
/* Hardware workaround - remove rates 6, 9 from rate ctrl */
/*--- IEEE80211_T_OFDM rates ---*/
/*   6 Mb */ { FALSE, IEEE80211_T_OFDM,  12},
/*   9 Mb */ { FALSE, IEEE80211_T_OFDM,  18},
/*  12 Mb */ {  TRUE, IEEE80211_T_OFDM,  24},
/*  18 Mb */ {  TRUE, IEEE80211_T_OFDM,  36},
/*  24 Mb */ {  TRUE, IEEE80211_T_OFDM,  48},
/*  36 Mb */ {  TRUE, IEEE80211_T_OFDM,  72},
/*  48 Mb */ {  TRUE, IEEE80211_T_OFDM,  96},
/*  54 Mb */ {  TRUE, IEEE80211_T_OFDM,  108},
/*--- IEEE80211_T_HT SS rates ---*/
/* 6.5 Mb */ {  TRUE, IEEE80211_T_HT,   0},
/*  13 Mb */ {  TRUE, IEEE80211_T_HT,  	1},
/*19.5 Mb */ {  TRUE, IEEE80211_T_HT,   2},
/*  26 Mb */ {  TRUE, IEEE80211_T_HT,  	3},
/*  39 Mb */ {  TRUE, IEEE80211_T_HT,  	4},
/*  52 Mb */ {  TRUE, IEEE80211_T_HT, 	5},
/*58.5 Mb */ {  TRUE, IEEE80211_T_HT,  	6},
/*  65 Mb */ {  TRUE, IEEE80211_T_HT,  	7},
/*--- IEEE80211_T_HT DS rates ---*/
/*  13 Mb */ {  TRUE, IEEE80211_T_HT,  	8},
/*  26 Mb */ {  TRUE, IEEE80211_T_HT,  	9},
/*  39 Mb */ {  TRUE, IEEE80211_T_HT,  	10},
/*  52 Mb */ {  TRUE, IEEE80211_T_HT,  	11},
/*  78 Mb */ {  TRUE, IEEE80211_T_HT,  	12},
/* 104 Mb */ {  TRUE, IEEE80211_T_HT,   13},
/* 117 Mb */ {  TRUE, IEEE80211_T_HT,   14},
/* 130 Mb */ {  TRUE, IEEE80211_T_HT,   15},
/*--- IEEE80211_T_HT TS rates ---*/
/*19.5 Mb */ {  TRUE, IEEE80211_T_HT,   16},
/* 39  Mb */ {  TRUE, IEEE80211_T_HT,   17},
/*58.5 Mb */ {  TRUE, IEEE80211_T_HT,   18},
/* 78  Mb */ {  TRUE, IEEE80211_T_HT,   19},
/*117  Mb */ {  TRUE, IEEE80211_T_HT,   20},
/*156  Mb */ {  TRUE, IEEE80211_T_HT,   21},
/*175.5Mb */ {  TRUE, IEEE80211_T_HT,   22},
/*195  Mb */ {  TRUE, IEEE80211_T_HT,   23},
/*--- IEEE80211_T_HT FS rates ---*/
/*19.5 Mb */ {  TRUE, IEEE80211_T_HT,   24},
/* 39  Mb */ {  TRUE, IEEE80211_T_HT,   25},
/*58.5 Mb */ {  TRUE, IEEE80211_T_HT,   26},
/* 78  Mb */ {  TRUE, IEEE80211_T_HT,   27},
/*117  Mb */ {  TRUE, IEEE80211_T_HT,   28},
/*156  Mb */ {  TRUE, IEEE80211_T_HT,   29},
/*175.5Mb */ {  TRUE, IEEE80211_T_HT,   30},
/*195  Mb */ {  TRUE, IEEE80211_T_HT,   31},
	},
};

static ol_ath_rate_table ol_if_11na_table = {

    40,  /* number of rates */
	{
/*--- IEEE80211_T_OFDM rates ---*/
/*   6 Mb */ {  TRUE, IEEE80211_T_OFDM,(0x80|12)},
/*   9 Mb */ {  TRUE, IEEE80211_T_OFDM,18       },
/*  12 Mb */ {  TRUE, IEEE80211_T_OFDM,(0x80|24)},
/*  18 Mb */ {  TRUE, IEEE80211_T_OFDM,36       },
/*  24 Mb */ {  TRUE, IEEE80211_T_OFDM,(0x80|48)},
/*  36 Mb */ {  TRUE, IEEE80211_T_OFDM,72       },
/*  48 Mb */ {  TRUE, IEEE80211_T_OFDM,96       },
/*  54 Mb */ {  TRUE, IEEE80211_T_OFDM,108      },
/*--- IEEE80211_T_HT SS rates ---*/
/* 6.5 Mb */ {  TRUE, IEEE80211_T_HT,  0},
/*  13 Mb */ {  TRUE, IEEE80211_T_HT,  1},
/*19.5 Mb */ {  TRUE, IEEE80211_T_HT,  2},
/*  26 Mb */ {  TRUE, IEEE80211_T_HT,  3},
/*  39 Mb */ {  TRUE, IEEE80211_T_HT,  4},
/*  52 Mb */ {  TRUE, IEEE80211_T_HT,  5},
/*58.5 Mb */ {  TRUE, IEEE80211_T_HT,  6},
/*  65 Mb */ {  TRUE, IEEE80211_T_HT,  7},
/*--- IEEE80211_T_HT DS rates ---*/
/*  13 Mb */ {  TRUE, IEEE80211_T_HT,  8},
/*  26 Mb */ {  TRUE, IEEE80211_T_HT,  9},
/*  39 Mb */ {  TRUE, IEEE80211_T_HT, 10},
/*  52 Mb */ {  TRUE, IEEE80211_T_HT, 11},
/*  78 Mb */ {  TRUE, IEEE80211_T_HT, 12},
/* 104 Mb */ {  TRUE, IEEE80211_T_HT, 13},
/* 117 Mb */ {  TRUE, IEEE80211_T_HT, 14},
/* 130 Mb */ {  TRUE, IEEE80211_T_HT, 15},
/*--- IEEE80211_T_HT TS rates ---*/
/*19.5 Mb */ {  TRUE, IEEE80211_T_HT, 16},
/* 39  Mb */ {  TRUE, IEEE80211_T_HT, 17},
/*58.5 Mb */ {  TRUE, IEEE80211_T_HT, 18},
/* 78  Mb */ {  TRUE, IEEE80211_T_HT, 19},
/*117  Mb */ {  TRUE, IEEE80211_T_HT, 20},
/*156  Mb */ {  TRUE, IEEE80211_T_HT, 21},
/*175.5Mb */ {  TRUE, IEEE80211_T_HT, 22},
/*195  Mb */ {  TRUE, IEEE80211_T_HT, 23},
/*--- IEEE80211_T_HT FS rates ---*/
/*19.5 Mb */ {  TRUE, IEEE80211_T_HT, 24},
/* 39  Mb */ {  TRUE, IEEE80211_T_HT, 25},
/*58.5 Mb */ {  TRUE, IEEE80211_T_HT, 26},
/* 78  Mb */ {  TRUE, IEEE80211_T_HT, 27},
/*117  Mb */ {  TRUE, IEEE80211_T_HT, 28},
/*156  Mb */ {  TRUE, IEEE80211_T_HT, 29},
/*175.5Mb */ {  TRUE, IEEE80211_T_HT, 30},
/*195  Mb */ {  TRUE, IEEE80211_T_HT, 31},
	},
};


typedef enum _ol_ath_rate_type {
    OL_ATH_NORMAL_RATE,
   OL_ATH_HALF_RATE,
   OL_ATH_QUARTER_RATE
} ol_ath_rate_type;


static void
ol_ath_rate_setup(struct ieee80211com *ic, enum ieee80211_phymode mode,
                  ol_ath_rate_type type, const ol_ath_rate_table *rt)
{
    struct ieee80211_rateset *rs;
    int i, maxrates, rix;

    if (mode >= IEEE80211_MODE_MAX) {
        return;
    }

    if (rt->num_rates > IEEE80211_RATE_MAXSIZE) {
        maxrates = IEEE80211_RATE_MAXSIZE;
    } else {
        maxrates = rt->num_rates;
    }

    switch (type) {
    case OL_ATH_NORMAL_RATE:
        rs = IEEE80211_SUPPORTED_RATES(ic, mode);
        break;
    case OL_ATH_HALF_RATE:
        rs = IEEE80211_HALF_RATES(ic);
        break;
    case OL_ATH_QUARTER_RATE:
        rs = IEEE80211_QUARTER_RATES(ic);
        break;
    default:
        return;
    }

    /* supported rates (non IEEE80211_T_HT) */
    rix = 0;
    for (i = 0; i < maxrates; i++) {
        if ((rt->info[i].phy == IEEE80211_T_HT))
            continue;
        rs->rs_rates[rix++] = rt->info[i].dot11_rate;
    }
    rs->rs_nrates = (u_int8_t)rix;
    if (ieee80211_is_phymode_not_basic(mode)) {
        /* supported rates (HT) */
        rix = 0;
        rs = IEEE80211_HT_RATES(ic, mode);
        for (i = 0; i < maxrates; i++) {
            if (rt->info[i].phy == IEEE80211_T_HT) {
                rs->rs_rates[rix++] = rt->info[i].dot11_rate;
            }
        }
        rs->rs_nrates = (u_int8_t)rix;
    }
}

void ol_ath_vht_rate_setup(struct ieee80211com *ic, uint32_t mcs_map,
                           uint16_t max_datarate, uint16_t basic_mcs)
{
    /* Set the VHT Supported MCS subset and highest data rate*/
    ieee80211com_set_vht_high_data_rate(ic, max_datarate);
    ieee80211com_set_vht_mcs_map( ic, mcs_map);

    /* Set up the VHT Basic MCS rate set. Use only the relevant 16 bits */
    ieee80211com_set_vhtop_basic_mcs_map(ic, basic_mcs);
}

void ol_ath_setup_rates(struct ieee80211com *ic)
{
    ol_ath_rate_setup(ic, IEEE80211_MODE_11A, OL_ATH_NORMAL_RATE, &ol_if_11a_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11A, OL_ATH_HALF_RATE, &ol_if_11a_halfrate_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11A, OL_ATH_QUARTER_RATE, &ol_if_11a_quarterrate_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11G, OL_ATH_NORMAL_RATE, &ol_if_11g_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11B, OL_ATH_NORMAL_RATE, &ol_if_11b_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11NA_HT20, OL_ATH_NORMAL_RATE, &ol_if_11na_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11NA_HT40PLUS, OL_ATH_NORMAL_RATE, &ol_if_11na_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11NA_HT40MINUS, OL_ATH_NORMAL_RATE, &ol_if_11na_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11NA_HT40, OL_ATH_NORMAL_RATE, &ol_if_11na_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11NG_HT20, OL_ATH_NORMAL_RATE, &ol_if_11ng_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11NG_HT40PLUS, OL_ATH_NORMAL_RATE, &ol_if_11ng_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11NG_HT40MINUS, OL_ATH_NORMAL_RATE, &ol_if_11ng_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11NG_HT40, OL_ATH_NORMAL_RATE, &ol_if_11ng_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11AC_VHT20, OL_ATH_NORMAL_RATE, &ol_if_11na_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11AC_VHT40PLUS, OL_ATH_NORMAL_RATE, &ol_if_11na_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11AC_VHT40MINUS, OL_ATH_NORMAL_RATE, &ol_if_11na_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11AC_VHT40, OL_ATH_NORMAL_RATE, &ol_if_11na_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11AC_VHT80, OL_ATH_NORMAL_RATE, &ol_if_11na_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11AC_VHT160, OL_ATH_NORMAL_RATE, &ol_if_11na_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11AC_VHT80_80, OL_ATH_NORMAL_RATE, &ol_if_11na_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11AXA_HE20, OL_ATH_NORMAL_RATE, &ol_if_11na_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11AXG_HE20, OL_ATH_NORMAL_RATE, &ol_if_11ng_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11AXA_HE40PLUS, OL_ATH_NORMAL_RATE, &ol_if_11na_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11AXA_HE40MINUS, OL_ATH_NORMAL_RATE, &ol_if_11na_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11AXG_HE40PLUS, OL_ATH_NORMAL_RATE, &ol_if_11ng_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11AXG_HE40MINUS, OL_ATH_NORMAL_RATE, &ol_if_11ng_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11AXA_HE40, OL_ATH_NORMAL_RATE, &ol_if_11na_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11AXG_HE40, OL_ATH_NORMAL_RATE, &ol_if_11ng_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11AXA_HE80, OL_ATH_NORMAL_RATE, &ol_if_11na_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11AXA_HE160, OL_ATH_NORMAL_RATE, &ol_if_11na_table );
    ol_ath_rate_setup(ic, IEEE80211_MODE_11AXA_HE80_80, OL_ATH_NORMAL_RATE, &ol_if_11na_table );

    /* TO-DO: quarter and half rates */
}


/*
 * Rate Control Table Legacy/HT/VHT/Hawkeye and APIs for RC extraction
 */

/*
 * These MACROS should be defined per chipsets and must be first before any
 * definitions.
 */
#define NUM_SPATIAL_STREAMS 8
#define WHAL_160MHZ_SUPPORT
#define MAX_SPATIAL_STREAMS_SUPPORTED_AT_160MHZ 4  //TODO ratecontrol fix this for hawkeye
#define VHT_EXTRA_MCS_SUPPORT
#define CONFIG_160MHZ_SUPPORT 1

typedef enum {
	MODE_11A        = 0,   /* 11a Mode */
	MODE_11G        = 1,   /* 11b/g Mode */
	MODE_11B        = 2,   /* 11b Mode */
	MODE_11GONLY    = 3,   /* 11g only Mode */
	MODE_11NA_HT20   = 4,  /* 11a HT20 mode */
	MODE_11NG_HT20   = 5,  /* 11g HT20 mode */
	MODE_11NA_HT40   = 6,  /* 11a HT40 mode */
	MODE_11NG_HT40   = 7,  /* 11g HT40 mode */
	MODE_11AC_VHT20 = 8,
	MODE_11AC_VHT40 = 9,
	MODE_11AC_VHT80 = 10,
	MODE_11AC_VHT20_2G = 11,
	MODE_11AC_VHT40_2G = 12,
	MODE_11AC_VHT80_2G = 13,
#if CONFIG_160MHZ_SUPPORT
	MODE_11AC_VHT80_80 = 14,
	MODE_11AC_VHT160   = 15,
#endif

#if SUPPORT_11AX
	MODE_11AX_HE20 = 16,
	MODE_11AX_HE40 = 17,
	MODE_11AX_HE80 = 18,
	MODE_11AX_HE80_80 = 19,
	MODE_11AX_HE160 = 20,
	MODE_11AX_HE20_2G = 21,
	MODE_11AX_HE40_2G = 22,
	MODE_11AX_HE80_2G = 23,
#endif

	/*
	 * MODE_UNKNOWN should not be used within the host / target interface.
	 * Thus, it is permissible for ODE_UNKNOWN to be conditionally-defined,
	 * taking different values when compiling for different targets.
	 */
	MODE_UNKNOWN,
	MODE_UNKNOWN_NO_160MHZ_SUPPORT = 14, /* not needed? */
#if 0
	MODE_UNKNOWN_NO_11AX_SUPPORT = 16, /* not needed? */
	MODE_UNKNOWN_11AX_SUPPORT = 24, /* not needed? */
#endif
	MODE_UNKNOWN_160MHZ_SUPPORT = MODE_UNKNOWN, /* not needed? */

#ifdef ATHR_WIN_NWF
	PHY_MODE_MAX    = MODE_UNKNOWN,
	PHY_MODE_MAX_NO_160_MHZ_SUPPORT = MODE_UNKNOWN_NO_160MHZ_SUPPORT,
	PHY_MODE_MAX_160_MHZ_SUPPORT    = MODE_UNKNOWN_160MHZ_SUPPORT,
#else
	MODE_MAX        = MODE_UNKNOWN,
	MODE_MAX_NO_160_MHZ_SUPPORT = MODE_UNKNOWN_NO_160MHZ_SUPPORT,
	MODE_MAX_160_MHZ_SUPPORT    = MODE_UNKNOWN_160MHZ_SUPPORT,
#endif
} WLAN_PHY_MODE;


enum WIFI_HW_RATECODE_PREAM_TYPE {
	WIFI_HW_RATECODE_PREAM_OFDM,
	WIFI_HW_RATECODE_PREAM_CCK,
	WIFI_HW_RATECODE_PREAM_HT,
	WIFI_HW_RATECODE_PREAM_VHT,
#if SUPPORT_11AX
	WIFI_HW_RATECODE_PREAM_HE,
#endif
	WIFI_HW_RATECODE_PREAM_COUNT,
};


/* HW Ratecode definition */

#if (NUM_SPATIAL_STREAMS > 4) || SUPPORT_11AX
#define HW_RATECODE_CCK_SHORT_PREAM_MASK  0x4
#define IS_HW_RATECODE_CCK(_rc) ((((_rc) >> 8) & 0x7) == 1)
#define IS_CCK_SHORT_PREAM_RC(_rc) (((_rc) & 0x71C) == 0x104)
#define IS_HW_RATECODE_HT(_rc)  (((_rc) & 0x700) == 0x200)
#define IS_HW_RATECODE_VHT(_rc) (((_rc) & 0x700) == 0x300)
#if SUPPORT_11AX
#define IS_HW_RATECODE_HE(_rc) (((_rc) & 0x700) == 0x400)
#endif /* SUPPORT_11AX */
#else
#define HW_RATECODE_CCK_SHORT_PREAM_MASK  0x4
#define IS_HW_RATECODE_CCK(_rc) ((((_rc) >> 6) & 0x3) == 1)
#define IS_CCK_SHORT_PREAM_RC(_rc) (((_rc) & 0xcc) == 0x44)
#define IS_HW_RATECODE_HT(_rc)  (((_rc) & 0xc0) == 0x80)
#define IS_HW_RATECODE_VHT(_rc) (((_rc) & 0xc0) == 0xc0)
#endif /* (NUM_SPATIAL_STREAMS > 4) || SUPPORT_11AX */

#define NUM_HT_MCS 8
#ifdef VHT_EXTRA_MCS_SUPPORT
#define NUM_VHT_MCS 12
#else
#define NUM_VHT_MCS 10
#endif
#define NUM_HE_MCS 12

#if (NUM_SPATIAL_STREAMS > 4)
#define NUM_HT_SPATIAL_STREAM 4
#else
#define NUM_HT_SPATIAL_STREAM NUM_SPATIAL_STREAMS
#endif

#define NUM_HT_RIX_PER_BW (NUM_HT_MCS * NUM_HT_SPATIAL_STREAM)
#define NUM_VHT_RIX_PER_BW (NUM_VHT_MCS * NUM_SPATIAL_STREAMS)
#define NUM_HE_RIX_PER_BW (NUM_HE_MCS * NUM_SPATIAL_STREAMS)

#ifdef WHAL_160MHZ_SUPPORT
#define NUM_VHT_RIX_FOR_160MHZ (NUM_VHT_MCS * MAX_SPATIAL_STREAMS_SUPPORTED_AT_160MHZ)
#define NUM_HE_RIX_FOR_160MHZ (NUM_HE_MCS * MAX_SPATIAL_STREAMS_SUPPORTED_AT_160MHZ)
#endif

/* CCK/DSSS */
#define CCK_RATE_TABLE_INDEX 0
#define CCK_RATE_11M_INDEX 0
#define CCK_FALLBACK_MIN_RATE 0x3 /** 1 Mbps */
#define CCK_FALLBACK_MAX_RATE 0x2 /** 2 Mbps */

/* OFDM */
#define OFDM_RATE_TABLE_INDEX 4
#define OFDMA_RATE_54M_INDEX 8

/* HT */

#define HT_20_RATE_TABLE_INDEX 12
#define HT_40_RATE_TABLE_INDEX (HT_20_RATE_TABLE_INDEX + NUM_HT_RIX_PER_BW)

/*
 * Data Structures definition starts here
 */
/* The following defines valid tx rate masks for each bw for HT and VHT
 *  */
#if SUPPORT_11AX
#ifdef WHAL_160MHZ_SUPPORT
#define NUM_VALID_RC_MASK 11 /* 4 for HE20/40/80/160, + 4 for VHT 20/40/80/160, + 2 for HT20/40 + 1 other */
#else
#define NUM_VALID_RC_MASK 9 /* 3 for HE20/40/80, + 3 for VHT 20/40/80, + 2 for HT20/40 + 1 other */
#endif
#else
#ifdef WHAL_160MHZ_SUPPORT
#define NUM_VALID_RC_MASK 7 /* 4 for VHT 20/40/80/160, + 2 for HT20/40 + 1 other */
#else
#define NUM_VALID_RC_MASK 6 /* 3 for VHT 20/40/80, + 2 for HT20/40 + 1 other */
#endif
#endif /* SUPPORT_11AX */


#if (NUM_SPATIAL_STREAMS > 8)
#error Rate table does not support more than 8 NSS

#else

#define VHT_20_RATE_TABLE_INDEX (HT_40_RATE_TABLE_INDEX + NUM_HT_RIX_PER_BW)
#define VHT_40_RATE_TABLE_INDEX (VHT_20_RATE_TABLE_INDEX + NUM_VHT_RIX_PER_BW)
#define VHT_80_RATE_TABLE_INDEX (VHT_40_RATE_TABLE_INDEX + NUM_VHT_RIX_PER_BW)

#ifdef WHAL_160MHZ_SUPPORT

#define VHT_160_RATE_TABLE_INDEX (VHT_80_RATE_TABLE_INDEX + NUM_VHT_RIX_PER_BW)
#define VHT_LAST_RIX_PLUS_ONE (VHT_160_RATE_TABLE_INDEX + NUM_VHT_RIX_FOR_160MHZ)

#else

#define VHT_LAST_RIX_PLUS_ONE (VHT_80_RATE_TABLE_INDEX + NUM_VHT_RIX_PER_BW)

#endif /* WHAL_160MHZ_SUPPORT */

#if SUPPORT_11AX

#define HE_20_RATE_TABLE_INDEX VHT_LAST_RIX_PLUS_ONE
#define HE_40_RATE_TABLE_INDEX (HE_20_RATE_TABLE_INDEX + NUM_HE_RIX_PER_BW)
#define HE_80_RATE_TABLE_INDEX (HE_40_RATE_TABLE_INDEX + NUM_HE_RIX_PER_BW)

#ifdef WHAL_160MHZ_SUPPORT

#define HE_160_RATE_TABLE_INDEX (HE_80_RATE_TABLE_INDEX + NUM_HE_RIX_PER_BW)
#define RATE_TABLE_SIZE (HE_160_RATE_TABLE_INDEX + NUM_HE_RIX_FOR_160MHZ)

#else

#define RATE_TABLE_SIZE (HE_80_RATE_TABLE_INDEX + NUM_HE_RIX_PER_BW)

#endif /* WHAL_160MHZ_SUPPORT */

#else

#define RATE_TABLE_SIZE VHT_LAST_RIX_PLUS_ONE

#endif /* SUPPORT_11AX */

#endif /* (NUM_SPATIAL_STREAMS > 8) */

#define RT_INVALID_INDEX ((uint16_t)(~((uint16_t)0)))

/* NOTE: Some thought went into rate selection via MODE. In Fusion the code does the following;
 *    -if 40MHz rates are valid then MCS 20MHz and OFDM are invalid.
 *  -if any (20/40) MCS rates are valid then OFDM is invalid.
 *  -CCK rates are always valid in 2.4 GHz.
 *
 * For ar6003/ar600P we adopted a slightly different behavior as follows;
 *    - if MCS 40Mhz is valid then MCS 20MHz is also valid.
 *  - because 20MHz rates are valid MCS 0@40mhz and MCS 1@40mhz will never be valid
 *        as there are MCS 20MHz rates with better throughput and sensitivity then
 *        these 40Mhz rates.
 *    - OFDM rates are invalid if any MCS rates are valid.
 *  - MCS 40MHz rates are always invalid in the 2.4 GHz band.
 *  - When 40MHz rates are valid only some of the MCS 20MHz rates are also valid.
 *        MCS 4-7 @ 20MHz are invalid when 40MHz MCS is valid as there are 40MHz rates
 *        with better throughput and sensitivity.
 */


/*Use the highest bit to indicate the invalid bcc rates accorss different PHYMODE*/
#define INVALID_BCC_RATE (1 << MODE_UNKNOWN)

#define CCK_MODE_VALID_MASK ((1 << MODE_11G) | (1 << MODE_11B) | \
		                             (1 << MODE_11NG_HT20) | (1 << MODE_11NG_HT40) | \
		                             (1 << MODE_11AC_VHT40_2G) | (1 << MODE_11AC_VHT20_2G) | (1 << MODE_11AC_VHT80_2G))

#define OFDM_MODE_VALID_MASK ((1 << MODE_11A) | (1 << MODE_11G) | \
		                              (1 << MODE_11GONLY) | (1 << MODE_11NA_HT20) | (1 << MODE_11NG_HT20) \
		                              | (1 << MODE_11NA_HT40) | (1 << MODE_11NG_HT40) \
		                              | (1 << MODE_11AC_VHT40) | (1 << MODE_11AC_VHT20) | (1 << MODE_11AC_VHT80) \
		                              | (1 << MODE_11AC_VHT40_2G) | (1 << MODE_11AC_VHT20_2G) | (1 << MODE_11AC_VHT80_2G) \
		                              | (1 << MODE_11AC_VHT160) | (1 << MODE_11AC_VHT80_80))

#define HT20_MODE_VALID_MASK ((1 << MODE_11NA_HT20) | (1 << MODE_11NG_HT20) \
		                              | (1 << MODE_11NA_HT40) | (1 << MODE_11NG_HT40) \
		                              | (1 << MODE_11AC_VHT40) | (1 << MODE_11AC_VHT20) | (1 << MODE_11AC_VHT80) \
		                              | (1 << MODE_11AC_VHT40_2G) | (1 << MODE_11AC_VHT20_2G) | (1 << MODE_11AC_VHT80_2G) \
		                              | (1 << MODE_11AC_VHT160) | (1 << MODE_11AC_VHT80_80))

#define HT40_MODE_VALID_MASK ((1 << MODE_11NA_HT40) | (1 << MODE_11NG_HT40) \
		                              | (1 << MODE_11AC_VHT40) | (1 << MODE_11AC_VHT80) \
		                              | (1 << MODE_11AC_VHT40_2G) | (1 << MODE_11AC_VHT80_2G) \
		                              | (1 << MODE_11AC_VHT160) | (1 << MODE_11AC_VHT80_80))

#define VHT20_MODE_VALID_MASK ((1 << MODE_11AC_VHT20) | \
		                               (1 << MODE_11AC_VHT40) | (1 << MODE_11AC_VHT80) | \
		                               (1 << MODE_11AC_VHT40_2G) | (1 << MODE_11AC_VHT20_2G) | (1 << MODE_11AC_VHT80_2G) | \
		                               (1 << MODE_11AC_VHT160) | (1 << MODE_11AC_VHT80_80))

#define VHT40_MODE_VALID_MASK ((1 << MODE_11AC_VHT40) | (1 << MODE_11AC_VHT80) | \
		                               (1 << MODE_11AC_VHT40_2G) | (1 << MODE_11AC_VHT80_2G) | \
		                               (1 << MODE_11AC_VHT160) | (1 << MODE_11AC_VHT80_80))

#define VHT80_MODE_VALID_MASK ((1 << MODE_11AC_VHT80) | (1 << MODE_11AC_VHT80_2G) | \
		                               (1 << MODE_11AC_VHT160) | (1 << MODE_11AC_VHT80_80))

#define VHT160_MODE_VALID_MASK ((1 << MODE_11AC_VHT160) | (1 << MODE_11AC_VHT80_80))

#define VHT20_LDPC_ONLY_MASKS (VHT20_MODE_VALID_MASK | INVALID_BCC_RATE)
#define VHT40_LDPC_ONLY_MASKS (VHT40_MODE_VALID_MASK | INVALID_BCC_RATE)
#define VHT80_LDPC_ONLY_MASKS (VHT80_MODE_VALID_MASK | INVALID_BCC_RATE)
#define VHT160_LDPC_ONLY_MASKS (VHT160_MODE_VALID_MASK | INVALID_BCC_RATE)

#define VHT_INVALID_MCS (0xFF)
#define VHT_INVALID_RATES_MASK 0

#define HE20_MODE_VALID_MASK ((1 << MODE_11AX_HE20) | (1 << MODE_11AX_HE40) | \
		                              (1 << MODE_11AX_HE80) | (1 << MODE_11AX_HE20_2G) | (1 << MODE_11AX_HE40_2G) | \
		                              (1 << MODE_11AX_HE80_2G) | (1 << MODE_11AX_HE80_80) | (1 << MODE_11AX_HE160))


#define HE40_MODE_VALID_MASK ((1 << MODE_11AX_HE40) | \
		                              (1 << MODE_11AX_HE80) | (1 << MODE_11AX_HE40_2G) | \
		                              (1 << MODE_11AX_HE80_2G) | (1 << MODE_11AX_HE80_80) | (1 << MODE_11AX_HE160))

#define HE80_MODE_VALID_MASK ((1 << MODE_11AX_HE80) | (1 << MODE_11AX_HE80_2G) | \
		                              (1 << MODE_11AX_HE80_80) | (1 << MODE_11AX_HE160))

#define HE160_MODE_VALID_MASK ((1 << MODE_11AX_HE80_80) | (1 << MODE_11AX_HE160))

#define HE20_LDPC_ONLY_MASKS (HE20_MODE_VALID_MASK | INVALID_BCC_RATE)
#define HE40_LDPC_ONLY_MASKS (HE40_MODE_VALID_MASK | INVALID_BCC_RATE)
#define HE80_LDPC_ONLY_MASKS (HE80_MODE_VALID_MASK | INVALID_BCC_RATE)
#define HE160_LDPC_ONLY_MASKS (HE160_MODE_VALID_MASK | INVALID_BCC_RATE)

#define HE_INVALID_RATES_MASK 0

/*
 * The following would span more than one octet when 160MHz BW defined for VHT
 * Also it's important to maintain the ordering of this enum else it would break other
 * rate adapation functions.
 */

typedef enum {
	WHAL_MOD_IEEE80211_T_DS,   /* direct sequence spread spectrum */
	WHAL_MOD_IEEE80211_T_OFDM, /* frequency division multiplexing */
	WHAL_MOD_IEEE80211_T_HT_20,
	WHAL_MOD_IEEE80211_T_HT_40,
	WHAL_MOD_IEEE80211_T_VHT_20,
	WHAL_MOD_IEEE80211_T_VHT_40,
	WHAL_MOD_IEEE80211_T_VHT_80,
#if defined(WHAL_160MHZ_SUPPORT)
	WHAL_MOD_IEEE80211_T_VHT_160,
#endif /* WHAL_160MHZ_SUPPORT */
#if SUPPORT_11AX
	WHAL_MOD_IEEE80211_T_HE_20,
	WHAL_MOD_IEEE80211_T_HE_40,
	WHAL_MOD_IEEE80211_T_HE_80,
#if defined(WHAL_160MHZ_SUPPORT)
	WHAL_MOD_IEEE80211_T_HE_160,
#endif /* WHAL_160MHZ_SUPPORT */
#endif /* SUPPORT_11AX */
	WHAL_MOD_IEEE80211_T_MAX_PHY
#define WHAL_MOD_IEEE80211_T_CCK WHAL_MOD_IEEE80211_T_DS /* more common nomenclatur */
} OL_WLAN_MODULATION_TYPE;

struct _OL_WHAL_RATE_TABLE {
	struct {
		uint32_t validModeMask;                /* bit mask where 1 indicates the rate is valid for that mode
							* bit position is (1<<mode) */
		OL_WLAN_MODULATION_TYPE phy;         /* CCK/OFDM/MCS */
		uint32_t                  rateKbps;    /* Rate in Kbits per second */
		uint32_t                  rateKbpsSGI; /* Rate in kbits per second if HT SGI is enabled */
#if (NUM_SPATIAL_STREAMS > 4) || SUPPORT_11AX
		uint32_t rateKbpsDGI; /* Rate in kbits per second if 1.6us GI is enabled */
		uint32_t rateKbpsQGI; /* Rate in kbits per second if 3.2us GI is enabled */
		uint32_t rateKbpsDCM; /* Rate in kbits per second if DCM is applied */
#endif
		uint32_t   userRateKbps;  /* User rate in KBits per second */
		uint16_t rateCode;      /* rate that goes into hw descriptors */
	} info[RATE_TABLE_SIZE];
};
typedef struct _OL_WHAL_RATE_TABLE OL_WHAL_RATE_TABLE;


static const OL_WHAL_RATE_TABLE whal_11abgnRateTable = {
    {
#if (NUM_SPATIAL_STREAMS > 4) || SUPPORT_11AX

     /****************************************************************************************************************************************************************************************/
     /* Legacy rates                                                                                                                                                                         */
     /****************************************************************************************************************************************************************************************/
     /*                                                                                                                SGI       DGI       QGI       DCM       User           */
     /*                       validModeMask                   phy                                     Kbps      Kbps      Kbps      Kbps      Kbps      Kbps    RC     */
     /*************************************************************************************************************************************************************************/
     /*       0  11 Mb  */ { CCK_MODE_VALID_MASK,    WHAL_MOD_IEEE80211_T_CCK,                               11000,    11000,        0,        0,        0,    11000,  0x100  },
     /*       1 5.5 Mb  */ { CCK_MODE_VALID_MASK,    WHAL_MOD_IEEE80211_T_CCK,                                5500,     5500,        0,        0,        0,     5500,  0x101  },
     /*       2   2 Mb  */ { CCK_MODE_VALID_MASK,    WHAL_MOD_IEEE80211_T_CCK,                                2000,     2000,        0,        0,        0,     2000,  0x102  },
     /*       3   1 Mb  */ { CCK_MODE_VALID_MASK,    WHAL_MOD_IEEE80211_T_CCK,                                1000,     1000,        0,        0,        0,     1000,  0x103  },
     /*       4  48 Mb  */ { OFDM_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_OFDM,                              48000,    48000,        0,        0,        0,    48000,  0x000  },
     /*       5  24 Mb  */ { OFDM_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_OFDM,                              24000,    24000,        0,        0,        0,    24000,  0x001  },
     /*       6  12 Mb  */ { OFDM_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_OFDM,                              12000,    12000,        0,        0,        0,    12000,  0x002  },
     /*       7   6 Mb  */ { OFDM_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_OFDM,                               6000,     6000,        0,        0,        0,     6000,  0x003  },
     /*       8  54 Mb  */ { OFDM_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_OFDM,                              54000,    54000,        0,        0,        0,    54000,  0x004  },
     /*       9  36 Mb  */ { OFDM_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_OFDM,                              36000,    36000,        0,        0,        0,    36000,  0x005  },
     /*      10  18 Mb  */ { OFDM_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_OFDM,                              18000,    18000,        0,        0,        0,    18000,  0x006  },
     /*      11   9 Mb  */ { OFDM_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_OFDM,                               9000,     9000,        0,        0,        0,     9000,  0x007  },

     /*************************************************************************************************************************************************************************/
     /* 11n HT20 rates                                                                                                                                                        */
     /*************************************************************************************************************************************************************************/
     /*                                                                                                                SGI       DGI       QGI       DCM       User         */
     /*                       validModeMask                   phy                                     Kbps      Kbps      Kbps      Kbps      Kbps      Kbps    RC  */
     /*************************************************************************************************************************************************************************/
     /*      12 MCS-00 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                              6500,     7200,        0,        0,        0,     6500,  0x200   },
     /*      13 MCS-01 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             13000,    14400,        0,        0,        0,    13000,  0x201   },
     /*      14 MCS-02 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             19500,    21700,        0,        0,        0,    19500,  0x202   },
     /*      15 MCS-03 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             26000,    28900,        0,        0,        0,    26000,  0x203   },
     /*      16 MCS-04 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             39000,    43300,        0,        0,        0,    39000,  0x204   },
     /*      17 MCS-05 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             52000,    57800,        0,        0,        0,    52000,  0x205   },
     /*      18 MCS-06 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             58500,    65000,        0,        0,        0,    58500,  0x206   },
     /*      19 MCS-07 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             65000,    72200,        0,        0,        0,    65000,  0x207   },
#if (NUM_SPATIAL_STREAMS > 1)
     /*      20 MCS-00 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             13000,    14400,        0,        0,        0,    13000,  0x220   },
     /*      21 MCS-01 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             26000,    28900,        0,        0,        0,    26000,  0x221   },
     /*      22 MCS-02 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             39000,    43300,        0,        0,        0,    39000,  0x222   },
     /*      23 MCS-03 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             52000,    57800,        0,        0,        0,    52000,  0x223   },
     /*      24 MCS-04 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             78000,    86700,        0,        0,        0,    78000,  0x224   },
     /*      25 MCS-05 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                            104000,   115600,        0,        0,        0,   104000,  0x225   },
     /*      26 MCS-06 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                            117000,   130000,        0,        0,        0,   117000,  0x226   },
     /*      27 MCS-07 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                            130000,   144000,        0,        0,        0,   130000,  0x227   },
#endif
#if (NUM_SPATIAL_STREAMS > 2)
     /*      28 MCS-00 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             19500,    21700,        0,        0,        0,    19500,  0x240   },
     /*      29 MCS-01 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             39000,    43300,        0,        0,        0,    39000,  0x241   },
     /*      30 MCS-02 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             58500,    65000,        0,        0,        0,    58500,  0x242   },
     /*      31 MCS-03 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             78000,    86700,        0,        0,        0,    78000,  0x243   },
     /*      32 MCS-04 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                            117000,   130000,        0,        0,        0,   117000,  0x244   },
     /*      33 MCS-05 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                            156000,   173300,        0,        0,        0,   156000,  0x245   },
     /*      34 MCS-06 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                            175500,   195000,        0,        0,        0,   175500,  0x246   },
     /*      35 MCS-07 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                            195000,   216700,        0,        0,        0,   195000,  0x247   },
#endif
#if (NUM_SPATIAL_STREAMS > 3)
     /*      36 MCS-00 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             26000,    28900,        0,        0,        0,    26000,  0x260   },
     /*      37 MCS-01 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             52000,    57800,        0,        0,        0,    52000,  0x261   },
     /*      38 MCS-02 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                             78000,    86700,        0,        0,        0,    78000,  0x262   },
     /*      39 MCS-03 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                            104000,   115600,        0,        0,        0,   104000,  0x263   },
     /*      40 MCS-04 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                            156000,   173300,        0,        0,        0,   156000,  0x264   },
     /*      41 MCS-05 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                            208000,   231100,        0,        0,        0,   208000,  0x265   },
     /*      42 MCS-06 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                            234000,   260000,        0,        0,        0,   234000,  0x266   },
     /*      43 MCS-07 */ { HT20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_20,                            260000,   288900,        0,        0,        0,   260000,  0x267   },
#endif

     /***********************************************************************************************************************************************************************/
     /* 11n HT40 rates                                                                                                                                                      */
     /***********************************************************************************************************************************************************************/
     /*                                                                                                                SGI       DGI       QGI       DCM       User         */
     /*                       validModeMask                   phy                                     Kbps      Kbps      Kbps      Kbps      Kbps      Kbps    RC   */
     /***********************************************************************************************************************************************************************/
     /*      44 MCS-00 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                             13500,    15000,        0,        0,        0,    13500,  0x200  },
     /*      45 MCS-01 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                             27000,    30000,        0,        0,        0,    27000,  0x201  },
     /*      46 MCS-02 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                             40500,    45000,        0,        0,        0,    40500,  0x202  },
     /*      47 MCS-03 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                             54000,    60000,        0,        0,        0,    54000,  0x203  },
     /*      48 MCS-04 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                             81500,    90000,        0,        0,        0,    81500,  0x204  },
     /*      49 MCS-05 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            108000,   120000,        0,        0,        0,   108000,  0x205  },
     /*      50 MCS-06 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            121500,   135000,        0,        0,        0,   121500,  0x206  },
     /*      51 MCS-07 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            135000,   150000,        0,        0,        0,   135000,  0x207  },
#if (NUM_SPATIAL_STREAMS > 1)
     /*      52 MCS-00 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                             27000,    30000,        0,        0,        0,    27000,  0x220  },
     /*      53 MCS-01 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                             54000,    60000,        0,        0,        0,    54000,  0x221  },
     /*      54 MCS-02 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                             81000,    90000,        0,        0,        0,    81000,  0x222  },
     /*      55 MCS-03 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            108000,   120000,        0,        0,        0,   108000,  0x223  },
     /*      56 MCS-04 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            162000,   180000,        0,        0,        0,   162000,  0x224  },
     /*      57 MCS-05 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            216000,   240000,        0,        0,        0,   216000,  0x225  },
     /*      58 MCS-06 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            243000,   270000,        0,        0,        0,   243000,  0x226  },
     /*      59 MCS-07 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            270000,   300000,        0,        0,        0,   270000,  0x227  },
#endif
#if (NUM_SPATIAL_STREAMS > 2)
     /*      60 MCS-00 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                             40500,    45000,        0,        0,        0,    40500,  0x240  },
     /*      61 MCS-01 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                             81000,    90000,        0,        0,        0,    81000,  0x241  },
     /*      62 MCS-02 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            121500,   135000,        0,        0,        0,   121500,  0x242  },
     /*      63 MCS-03 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            162000,   180000,        0,        0,        0,   162000,  0x243  },
     /*      64 MCS-04 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            243000,   270000,        0,        0,        0,   243000,  0x244  },
     /*      65 MCS-05 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            324000,   360000,        0,        0,        0,   324000,  0x245  },
     /*      66 MCS-06 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            364500,   405000,        0,        0,        0,   364500,  0x246  },
     /*      67 MCS-07 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            405000,   450000,        0,        0,        0,   405000,  0x247  },
#endif
#if (NUM_SPATIAL_STREAMS > 3)
     /*      68 MCS-00 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                             54000,    60000,        0,        0,        0,    54000,  0x260  },
     /*      69 MCS-01 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            108000,   120000,        0,        0,        0,   108000,  0x261  },
     /*      70 MCS-02 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            162000,   180000,        0,        0,        0,   162000,  0x262  },
     /*      71 MCS-03 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            216000,   240000,        0,        0,        0,   216000,  0x263  },
     /*      72 MCS-04 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            324000,   360000,        0,        0,        0,   324000,  0x264  },
     /*      73 MCS-05 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            432000,   480000,        0,        0,        0,   432000,  0x265  },
     /*      74 MCS-06 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            486000,   540000,        0,        0,        0,   486000,  0x266  },
     /*      75 MCS-07 */ { HT40_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HT_40,                            540000,   600000,        0,        0,        0,   540000,  0x267  },
#endif

     /************************************************************************************************************************************************************************/
     /* 11ac VHT20 rates                                                                                                                                                     */
     /************************************************************************************************************************************************************************/
     /*                                                                                                                SGI       DGI       QGI       DCM       User         */
     /*                       validModeMask                   phy                                     Kbps      Kbps      Kbps      Kbps      Kbps      Kbps    RC   */
     /***********************************************************************************************************************************************************************/
     /*      76 MCS-00 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                             6500,     7200,        0,        0,        0,     6500,  0x300  },
     /*      77 MCS-01 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            13000,    14400,        0,        0,        0,    13000,  0x301  },
     /*      78 MCS-02 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            19500,    21700,        0,        0,        0,    19500,  0x302  },
     /*      79 MCS-03 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            26000,    28900,        0,        0,        0,    26000,  0x303  },
     /*      80 MCS-04 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            39000,    43300,        0,        0,        0,    39000,  0x304  },
     /*      81 MCS-05 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            52000,    57800,        0,        0,        0,    52000,  0x305  },
     /*      82 MCS-06 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            58500,    65000,        0,        0,        0,    58500,  0x306  },
     /*      83 MCS-07 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            65000,    72200,        0,        0,        0,    65000,  0x307  },
     /*      84 MCS-08 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            78000,    86700,        0,        0,        0,    78000,  0x308  },
     /*      85 MCS-09 */ { VHT20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_20,                            86500,    96000,        0,        0,        0,    86500,  0x309 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*   	 86 MCS-10 */ { VHT20_LDPC_ONLY_MASKS,	WHAL_MOD_IEEE80211_T_VHT_20,                    		97500,   108300,	    0,        0,		0,    97500,  0x30a},
	 /*	     87 MCS-11 */ { VHT20_LDPC_ONLY_MASKS,	WHAL_MOD_IEEE80211_T_VHT_20,                    		108300,  120400,	    0,        0,		0,   108300,  0x30b},
#endif
#if (NUM_SPATIAL_STREAMS > 1)
     /*      88 MCS-00 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            13000,    14400,        0,        0,        0,    13000,  0x320 },
     /*      89 MCS-01 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            26000,    28900,        0,        0,        0,    26000,  0x321 },
     /*      90 MCS-02 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            39000,    43300,        0,        0,        0,    39000,  0x322 },
     /*      91 MCS-03 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            52000,    57800,        0,        0,        0,    52000,  0x323 },
     /*      92 MCS-04 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            78000,    86700,        0,        0,        0,    78000,  0x324 },
     /*      93 MCS-05 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                           104000,   115600,        0,        0,        0,   104000,  0x325 },
     /*      94 MCS-06 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                           117000,   130000,        0,        0,        0,   117000,  0x326 },
     /*      95 MCS-07 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                           130000,   144400,        0,        0,        0,   130000,  0x327 },
     /*      96 MCS-08 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                           156000,   173300,        0,        0,        0,   156000,  0x328 },
     /*      97 MCS-09 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           173000,   192000,        0,        0,        0,   173000,  0x329 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*	 98 MCS-10 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                   		   195000,	 216700, 	    0,		 0, 	   0,	 195000,  0x32a },
     /*	 99 MCS-11 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                  	           216700,	 240700, 	    0,		 0, 	   0,	 216700,  0x32b    },
#endif
#endif
#if (NUM_SPATIAL_STREAMS > 2)
     /*     100 MCS-00 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            19500,    21700,        0,        0,        0,    19500,  0x340 },
     /*     101 MCS-01 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            39000,    43300,        0,        0,        0,    39000,  0x341 },
     /*     102 MCS-02 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            58500,    65000,        0,        0,        0,    58500,  0x342 },
     /*     103 MCS-03 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            78000,    86700,        0,        0,        0,    78000,  0x343 },
     /*     104 MCS-04 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                           117000,   130000,        0,        0,        0,   117000,  0x344 },
     /*     105 MCS-05 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                           156000,   173300,        0,        0,        0,   156000,  0x345 },
     /*     106 MCS-06 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                           175500,   195000,        0,        0,        0,   175500,  0x346 },
     /*     107 MCS-07 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                           195000,   216700,        0,        0,        0,   195000,  0x347 },
     /*     108 MCS-08 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                           234000,   260000,        0,        0,        0,   234000,  0x348 },
     /*     109 MCS-09 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                           260000,   288900,        0,        0,        0,   260000,  0x349 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*	    110 MCS-10 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                   		   292500,	 325000, 	    0,		  0, 	    0,	 292500,  0x34a},
     /*	    111 MCS-11 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                   		   325000,	 361100, 	    0,		  0, 	    0,	 325000,  0x34b},
#endif
#endif
#if (NUM_SPATIAL_STREAMS > 3)
     /*     112 MCS-00 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            26000,    28900,        0,        0,        0,    26000,  0x360 },
     /*     113 MCS-01 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            52000,    57800,        0,        0,        0,    52000,  0x361 },
     /*     114 MCS-02 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                            78000,    86700,        0,        0,        0,    78000,  0x362 },
     /*     115 MCS-03 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                           104000,   115600,        0,        0,        0,   104000,  0x363 },
     /*     116 MCS-04 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                           156000,   173300,        0,        0,        0,   156000,  0x364 },
     /*     117 MCS-05 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                           208000,   231100,        0,        0,        0,   208000,  0x365 },
     /*     118 MCS-06 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                           234000,   260000,        0,        0,        0,   234000,  0x366 },
     /*     119 MCS-07 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                           260000,   288900,        0,        0,        0,   260000,  0x367 },
     /*     120 MCS-08 */ { VHT20_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_20,                           312000,   346700,        0,        0,        0,   312000,  0x368 },
     /*     121 MCS-09 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           344000,   378400,        0,        0,        0,   344000,  0x369 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*	    122 MCS-10 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                   	   390000,   433300,        0,	      0, 	0,    390000,  0x36a},
     /*     123 MCS-11 */ { VHT20_LDPC_ONLY_MASKS,	WHAL_MOD_IEEE80211_T_VHT_20,                       433300,   481500,        0,	      0,	0,    433300,  0x36b},
#endif
#endif
#if (NUM_SPATIAL_STREAMS > 4)
     /*     124 MCS-00 */ { VHT20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_20,                            32500,    36100,        0,        0,        0,    32500,  0x380 },
     /*     125 MCS-01 */ { VHT20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_20,                            65000,    72200,        0,        0,        0,    65000,  0x381 },
     /*     126 MCS-02 */ { VHT20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_20,                            97500,   108300,        0,        0,        0,    97500,  0x382 },
     /*     127 MCS-03 */ { VHT20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_20,                           130000,   144400,        0,        0,        0,   130000,  0x383 },
     /*     128 MCS-04 */ { VHT20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_20,                           195000,   216700,        0,        0,        0,   195000,  0x384 },
     /*     129 MCS-05 */ { VHT20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_20,                           260000,   288900,        0,        0,        0,   260000,  0x385 },
     /*     130 MCS-06 */ { VHT20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_20,                           292500,   325000,        0,        0,        0,   292500,  0x386 },
     /*     131 MCS-07 */ { VHT20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_20,                           325000,   361100,        0,        0,        0,   325000,  0x387 },
     /*     132 MCS-08 */ { VHT20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_20,                           390000,   433300,        0,        0,        0,   390000,  0x388 },
     /*     133 MCS-09 */ { VHT20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_20,                           433300,   481500,        0,        0,        0,   433300,  0x389 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     134 MCS-10 */ { VHT20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_20,                           487500,   541700,        0,        0,        0,   487500,  0x38a },
     /*     135 MCS-11 */ { VHT20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_20,                           541700,   601900,        0,        0,        0,   541700,  0x38b },
#endif
#endif
#if (NUM_SPATIAL_STREAMS > 5)
     /*     136 MCS-00 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                            39000,    43300,        0,        0,        0,    39000,  0x3a0 },
     /*     137 MCS-01 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                            78000,    86700,        0,        0,        0,    78000,  0x3a1 },
     /*     138 MCS-02 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           117000,   130000,        0,        0,        0,   117000,  0x3a2 },
     /*     139 MCS-03 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           156000,   173300,        0,        0,        0,   156000,  0x3a3 },
     /*     140 MCS-04 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           234000,   260000,        0,        0,        0,   234000,  0x3a4 },
     /*     141 MCS-05 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           312000,   346700,        0,        0,        0,   312000,  0x3a5 },
     /*     142 MCS-06 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           351000,   390000,        0,        0,        0,   351000,  0x3a6 },
     /*     143 MCS-07 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           390000,   433300,        0,        0,        0,   390000,  0x3a7 },
     /*     144 MCS-08 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           468000,   520000,        0,        0,        0,   468000,  0x3a8 },
     /*     145 MCS-09 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           520000,   577800,        0,        0,        0,   520000,  0x3a9 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*	    146 MCS-10 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                   		  585000,	650000, 	   0,		 0, 	   0,	585000,  0x3aa },
     /* 	147 MCS-11 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                    	      650000,	722200,	       0,		 0,		   0,	650000,  0x3ab },
#endif
#endif
#if (NUM_SPATIAL_STREAMS > 6)
     /*     148 MCS-00 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                            45500,    50600,        0,        0,        0,    45500,  0x3c0 },
     /*     149 MCS-01 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                            91000,   101100,        0,        0,        0,    91000,  0x3c1 },
     /*     150 MCS-02 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           136500,   151700,        0,        0,        0,   136500,  0x3c2 },
     /*     151 MCS-03 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           182000,   202200,        0,        0,        0,   182000,  0x3c3 },
     /*     152 MCS-04 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           273000,   303300,        0,        0,        0,   273000,  0x3c4 },
     /*     153 MCS-05 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           364000,   404400,        0,        0,        0,   364000,  0x3c5 },
     /*     154 MCS-06 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           409500,   455000,        0,        0,        0,   409500,  0x3c6 },
     /*     155 MCS-07 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           455000,   505600,        0,        0,        0,   455000,  0x3c7 },
     /*     156 MCS-08 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           546000,   606700,        0,        0,        0,   546000,  0x3c8 },
     /*     157 MCS-09 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           606700,   674100,        0,        0,        0,   606700,  0x3c9 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     158 MCS-10 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           682500,   758300,        0,        0,        0,   682500,  0x3ca },
     /*     159 MCS-11 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           758300,   842600,        0,        0,        0,   758300,  0x3cb },
#endif
#endif
#if (NUM_SPATIAL_STREAMS > 7)
     /*     160 MCS-00 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                            52000,    57800,        0,        0,        0,    52000,  0x3e0 },
     /*     161 MCS-01 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           104000,   115600,        0,        0,        0,   104000,  0x3e1 },
     /*     162 MCS-02 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           156000,   173300,        0,        0,        0,   156000,  0x3e2 },
     /*     163 MCS-03 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           208000,   231100,        0,        0,        0,   208000,  0x3e3 },
     /*     164 MCS-04 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           312000,   346700,        0,        0,        0,   312000,  0x3e4 },
     /*     165 MCS-05 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           416000,   462200,        0,        0,        0,   416000,  0x3e5 },
     /*     166 MCS-06 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           468000,   520000,        0,        0,        0,   468000,  0x3e6 },
     /*     167 MCS-07 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           520000,   577800,        0,        0,        0,   520000,  0x3e7 },
     /*     168 MCS-08 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           624000,   693300,        0,        0,        0,   624000,  0x3e8 },
     /*     169 MCS-09 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           693300,   770400,        0,        0,        0,   693300,  0x3e9 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     170 MCS-10 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           780000,   866700,        0,        0,        0,   780000,  0x3ea },
     /*     171 MCS-11 */ { VHT20_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_20,                           866700,   963000,        0,        0,        0,   866700,  0x3eb },
#endif
#endif

     /***********************************************************************************************************************************************************************/
     /* 11ac VHT40 rates                                                                                                                                                    */
     /***********************************************************************************************************************************************************************/
     /*                                                                                                                SGI       DGI       QGI       DCM       User         */
     /*                       validModeMask                   phy                                     Kbps      Kbps      Kbps      Kbps      Kbps      Kbps    RC   */
     /***********************************************************************************************************************************************************************/
     /*     172 MCS-00 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                            13500,    15000,        0,        0,        0,    13500,  0x300 },
     /*     173 MCS-01 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                            27000,    30000,        0,        0,        0,    27000,  0x301 },
     /*     174 MCS-02 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                            40500,    45000,        0,        0,        0,    40500,  0x302 },
     /*     175 MCS-03 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                            54000,    60000,        0,        0,        0,    54000,  0x303 },
     /*     176 MCS-04 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                            81000,    90000,        0,        0,        0,    81000,  0x304 },
     /*     177 MCS-05 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           108000,   120000,        0,        0,        0,   108000,  0x305 },
     /*     178 MCS-06 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           121500,   135000,        0,        0,        0,   121500,  0x306 },
     /*     179 MCS-07 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           135000,   150000,        0,        0,        0,   135000,  0x307 },
     /*     180 MCS-08 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           162000,   180000,        0,        0,        0,   162000,  0x308 },
     /*     181 MCS-09 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           180000,   200000,        0,        0,        0,   180000,  0x309 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     182 MCS-10 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           202500,   225000,        0,        0,        0,   202500,  0x30a },
     /*     183 MCS-11 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           225000,   250000,        0,        0,        0,   225000,  0x30b },
#endif
#if (NUM_SPATIAL_STREAMS > 1)
     /*     184 MCS-00 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                            27000,    30000,        0,        0,        0,    27000,  0x320 },
     /*     185 MCS-01 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                            54000,    60000,        0,        0,        0,    54000,  0x321 },
     /*     186 MCS-02 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                            81000,    90000,        0,        0,        0,    81000,  0x322 },
     /*     187 MCS-03 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           108000,   120000,        0,        0,        0,   108000,  0x323 },
     /*     188 MCS-04 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           162000,   180000,        0,        0,        0,   162000,  0x324 },
     /*     189 MCS-05 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           216000,   240000,        0,        0,        0,   216000,  0x325 },
     /*     190 MCS-06 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           243000,   270000,        0,        0,        0,   243000,  0x326 },
     /*     191 MCS-07 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           270000,   300000,        0,        0,        0,   270000,  0x327 },
     /*     192 MCS-08 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           324000,   360000,        0,        0,        0,   324000,  0x328 },
     /*     193 MCS-09 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           360000,   400000,        0,        0,        0,   360000,  0x329 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     194 MCS-10 */ { VHT40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_40,                           405000,   450000,        0,        0,        0,   405000,  0x32a },
     /*     195 MCS-11 */ { VHT40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_40,                           450000,   500000,        0,        0,        0,   450000,  0x32b },
#endif
#endif
#if (NUM_SPATIAL_STREAMS > 2)
     /*     196 MCS-00 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                            40500,    45000,        0,        0,        0,    40500,  0x340 },
     /*     197 MCS-01 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                            81000,    90000,        0,        0,        0,    81000,  0x341 },
     /*     198 MCS-02 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           121500,   135000,        0,        0,        0,   121500,  0x342 },
     /*     199 MCS-03 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           162000,   180000,        0,        0,        0,   162000,  0x343 },
     /*     200 MCS-04 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           243000,   270000,        0,        0,        0,   243000,  0x344 },
     /*     201 MCS-05 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           324000,   360000,        0,        0,        0,   324000,  0x345 },
     /*     202 MCS-06 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           364500,   405000,        0,        0,        0,   364500,  0x346 },
     /*     203 MCS-07 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           405000,   450000,        0,        0,        0,   405000,  0x347 },
     /*     204 MCS-08 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           486000,   540000,        0,        0,        0,   486000,  0x348 },
     /*     205 MCS-09 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           540000,   600000,        0,        0,        0,   540000,  0x349 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     206 MCS-10 */ { VHT40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_40,                           607500,   675000,        0,        0,        0,   607500,  0x34a},
     /*     207 MCS-11 */ { VHT40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_40,                           675000,   750000,        0,        0,        0,   675000,  0x34b},
#endif
#endif
#if (NUM_SPATIAL_STREAMS > 3)
     /*     208 MCS-00 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                            54000,    60000,        0,        0,        0,    54000,  0x360},
     /*     209 MCS-01 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           108000,   120000,        0,        0,        0,   108000,  0x361},
     /*     210 MCS-02 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           162000,   180000,        0,        0,        0,   162000,  0x362},
     /*     211 MCS-03 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           216000,   240000,        0,        0,        0,   216000,  0x363},
     /*     212 MCS-04 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           324000,   260000,        0,        0,        0,   324000,  0x364},
     /*     213 MCS-05 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           432000,   480000,        0,        0,        0,   432000,  0x365},
     /*     214 MCS-06 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           486000,   540000,        0,        0,        0,   486000,  0x366},
     /*     215 MCS-07 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           540000,   600000,        0,        0,        0,   540000,  0x367},
     /*     216 MCS-08 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           648000,   720000,        0,        0,        0,   648000,  0x368},
     /*     217 MCS-09 */ { VHT40_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_40,                           720000,   800000,        0,        0,        0,   720000,  0x369},
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     218 MCS-10 */ { VHT40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_40,                           810000,   900000,        0,        0,        0,   810000,  0x36a },
     /*     219 MCS-11 */ { VHT40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_40,                           900000,  1000000,        0,        0,        0,   900000,  0x36b },
#endif
#endif
#if (NUM_SPATIAL_STREAMS > 4)
     /*     220 MCS-00 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                            67500,    75000,        0,        0,        0,    67500,  0x380 },
     /*     221 MCS-01 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           135000,   150000,        0,        0,        0,   135000,  0x381 },
     /*     222 MCS-02 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           202500,   225000,        0,        0,        0,   202500,  0x382 },
     /*     223 MCS-03 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           270000,   300000,        0,        0,        0,   270000,  0x383 },
     /*     224 MCS-04 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           405000,   450000,        0,        0,        0,   405000,  0x384 },
     /*     225 MCS-05 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           540000,   600000,        0,        0,        0,   540000,  0x385 },
     /*     226 MCS-06 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           607500,   675000,        0,        0,        0,   607500,  0x386 },
     /*     227 MCS-07 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           675000,   750000,        0,        0,        0,   675000,  0x387 },
     /*     228 MCS-08 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           810000,   900000,        0,        0,        0,   810000,  0x388 },
     /*     229 MCS-09 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           900000,  1000000,        0,        0,        0,   900000,  0x389 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     230 MCS-10 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                          1012500,  1125000,        0,        0,        0,   1012500, 0x38a },
     /*     231 MCS-11 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                          1125000,  1250000,        0,        0,        0,   1125000, 0x38b },
#endif
#endif
#if (NUM_SPATIAL_STREAMS > 5)
     /*     232 MCS-00 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                            81000,    90000,        0,        0,        0,    81000,  0x3a0 },
     /*     233 MCS-01 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           162000,   180000,        0,        0,        0,   162000,  0x3a1 },
     /*     234 MCS-02 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           243000,   270000,        0,        0,        0,   243000,  0x3a2 },
     /*     235 MCS-03 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           324000,   360000,        0,        0,        0,   324000,  0x3a3 },
     /*     236 MCS-04 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           486000,   540000,        0,        0,        0,   486000,  0x3a4 },
     /*     237 MCS-05 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           648000,   720000,        0,        0,        0,   648000,  0x3a5 },
     /*     238 MCS-06 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           729000,   815000,        0,        0,        0,   729000,  0x3a6 },
     /*     239 MCS-07 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           810000,   900000,        0,        0,        0,   810000,  0x3a7 },
     /*     240 MCS-08 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           972000,  1080000,        0,        0,        0,   972000,  0x3a8 },
     /*     241 MCS-09 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                          1080000,  1200000,        0,        0,        0,  1080000,  0x3a9 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     242 MCS-10 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                          1215000,  1350000,        0,        0,        0,  1215000,  0x3aa },
     /*     243 MCS-11 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                          1350000,  1500000,        0,        0,        0,  1350000,  0x3ab },
#endif
#endif
#if (NUM_SPATIAL_STREAMS > 6)
     /*     244 MCS-00 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                            94500,   105000,        0,        0,        0,    94500,  0x3c0 },
     /*     245 MCS-01 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           189000,   210000,        0,        0,        0,   189000,  0x3c1 },
     /*     246 MCS-02 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           283500,   315000,        0,        0,        0,   283500,  0x3c2 },
     /*     247 MCS-03 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           378000,   420000,        0,        0,        0,   378000,  0x3c3 },
     /*     248 MCS-04 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           567000,   630000,        0,        0,        0,   567000,  0x3c4 },
     /*     249 MCS-05 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           756000,   840000,        0,        0,        0,   756000,  0x3c5 },
     /*     250 MCS-06 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           850500,   945000,        0,        0,        0,   850500,  0x3c6 },
     /*     251 MCS-07 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           945000,  1050000,        0,        0,        0,   945000,  0x3c7 },
     /*     252 MCS-08 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                          1134000,  1260000,        0,        0,        0,  1134000,  0x3c8 },
     /*     253 MCS-09 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                          1260000,  1400000,        0,        0,        0,  1260000,  0x3c9 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     254 MCS-10 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                          1417500,  1575000,        0,        0,        0,  1417500,  0x3ca },
     /*     255 MCS-11 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                          1575000,  1750000,        0,        0,        0,  1575000,  0x3cb },
#endif
#endif
#if (NUM_SPATIAL_STREAMS > 7)
     /*     256 MCS-00 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           108000,   120000,        0,        0,        0,   108000,  0x3e0 },
     /*     257 MCS-01 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           216000,   240000,        0,        0,        0,   216000,  0x3e1 },
     /*     258 MCS-02 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           324000,   360000,        0,        0,        0,   324000,  0x3e2 },
     /*     259 MCS-03 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           432000,   480000,        0,        0,        0,   432000,  0x3e3 },
     /*     260 MCS-04 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           648000,   720000,        0,        0,        0,   648000,  0x3e4 },
     /*     261 MCS-05 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           864000,   960000,        0,        0,        0,   864000,  0x3e5 },
     /*     262 MCS-06 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                           972000,  1080000,        0,        0,        0,   972000,  0x3e6 },
     /*     263 MCS-07 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                          1080000,  1200000,        0,        0,        0,  1080000,  0x3e7 },
     /*     264 MCS-08 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                          1296000,  1440000,        0,        0,        0,  1296000,  0x3e8 },
     /*     265 MCS-09 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                          1440000,  1600000,        0,        0,        0,  1440000,  0x3e9 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     266 MCS-10 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                          1620000,  1800000,        0,        0,        0,  1620000,  0x3ea },
     /*     267 MCS-11 */ { VHT40_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_40,                          1800000,  2000000,        0,        0,        0,  1800000,  0x3eb },
#endif
#endif

     /***********************************************************************************************************************************************************************************************/
     /* 11ac VHT80 rates                                                                                                                                                                            */
     /***********************************************************************************************************************************************************************************************/
     /*                                                                                                                SGI       DGI       QGI       DCM       User                                 */
     /*                       validModeMask                   phy                                     Kbps      Kbps      Kbps      Kbps      Kbps      Kbps    RC                           */
     /***********************************************************************************************************************************************************************************************/
     /*     268 MCS-00 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                            29300,    32500,        0,        0,        0,    29300,  0x300},
     /*     269 MCS-01 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                            58500,    65000,        0,        0,        0,    58500,  0x301},
     /*     270 MCS-02 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                            87800,    97500,        0,        0,        0,    87800,  0x302},
     /*     271 MCS-03 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           117000,   130000,        0,        0,        0,   117000,  0x303},
     /*     272 MCS-04 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           175500,   195000,        0,        0,        0,   175500,  0x304},
     /*     273 MCS-05 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           234000,   260000,        0,        0,        0,   234000,  0x305},
     /*     274 MCS-06 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           263300,   292500,        0,        0,        0,   263300,  0x306},
     /*     275 MCS-07 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           292500,   325000,        0,        0,        0,   292500,  0x307},
     /*     276 MCS-08 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           351000,   390000,        0,        0,        0,   351000,  0x308},
     /*     277 MCS-09 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           390000,   433300,        0,        0,        0,   390000,  0x309},
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     278 MCS-10 */ { VHT80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_80,                           438800,   487500,        0,        0,        0,   438800,  0x30a},
     /*     279 MCS-11 */ { VHT80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_80,                           487500,   541700,        0,        0,        0,   487500,  0x30b},
#endif
#if (NUM_SPATIAL_STREAMS > 1)
     /*     280 MCS-00 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                            58500,    65000,        0,        0,        0,    58500,  0x320},
     /*     281 MCS-01 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           117000,   130000,        0,        0,        0,   117000,  0x321},
     /*     282 MCS-02 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           175500,   195000,        0,        0,        0,   175500,  0x322},
     /*     283 MCS-03 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           234000,   260000,        0,        0,        0,   234000,  0x323},
     /*     284 MCS-04 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           351000,   390000,        0,        0,        0,   351000,  0x324},
     /*     285 MCS-05 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           468000,   520000,        0,        0,        0,   468000,  0x325},
     /*     286 MCS-06 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           526500,   585000,        0,        0,        0,   526500,  0x326},
     /*     287 MCS-07 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           585000,   650000,        0,        0,        0,   585000,  0x327},
     /*     288 MCS-08 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           702000,   780000,        0,        0,        0,   702000,  0x328},
     /*     289 MCS-09 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           780000,   866700,        0,        0,        0,   780000,  0x329},
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     290 MCS-10 */ { VHT80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_80,                           877500,   975000,        0,        0,        0,   877500,  0x32a},
     /*     291 MCS-11 */ { VHT80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_80,                           975000,  1083300,        0,        0,        0,   975000,  0x32b},
#endif
#endif
#if (NUM_SPATIAL_STREAMS > 2)
     /*     292 MCS-00 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                            87800,    97500,        0,        0,        0,    87800,  0x340 },
     /*     293 MCS-01 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           175500,   195000,        0,        0,        0,   175500,  0x341 },
     /*     294 MCS-02 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           263300,   292500,        0,        0,        0,   263300,  0x342 },
     /*     295 MCS-03 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           351000,   390000,        0,        0,        0,   351000,  0x343 },
     /*     296 MCS-04 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           526500,   585000,        0,        0,        0,   526500,  0x344 },
     /*     297 MCS-05 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           702000,   780000,        0,        0,        0,   702000,  0x345 },
     /*     298 MCS-06 */ { VHT_INVALID_RATES_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           789800,   877500,        0,        0,        0,   789800,  0x346 },
     /*     299 MCS-07 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           877500,   975000,        0,        0,        0,   877500,  0x347 },
     /*     300 MCS-08 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                          1053000,  1170000,        0,        0,        0,  1053000,  0x348 },
     /*     301 MCS-09 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                          1170000,  1300000,        0,        0,        0,  1170000,  0x349 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     302 MCS-10 */ { VHT80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_80,                          1316300,  1462500,        0,        0,        0,  1316300,  0x34a },
     /*     303 MCS-11 */ { VHT80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_80,                          1462500,  1625000,        0,        0,        0,  1462500,  0x34b },
#endif
#endif
#if (NUM_SPATIAL_STREAMS > 3)
     /*     304 MCS-00 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           117000,   130000,        0,        0,        0,   117000,  0x360 },
     /*     305 MCS-01 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           234000,   260000,        0,        0,        0,   234000,  0x361 },
     /*     306 MCS-02 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           351000,   390000,        0,        0,        0,   351000,  0x362 },
     /*     307 MCS-03 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           468000,   520000,        0,        0,        0,   468000,  0x363 },
     /*     308 MCS-04 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           702000,   780000,        0,        0,        0,   702000,  0x364 },
     /*     309 MCS-05 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                           936000,  1040000,        0,        0,        0,   936000,  0x365 },
     /*     310 MCS-06 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                          1053000,  1170000,        0,        0,        0,  1053000,  0x366 },
     /*     311 MCS-07 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                          1170000,  1300000,        0,        0,        0,  1170000,  0x367 },
     /*     312 MCS-08 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                          1404000,  1560000,        0,        0,        0,  1404000,  0x368 },
     /*     313 MCS-09 */ { VHT80_MODE_VALID_MASK,  WHAL_MOD_IEEE80211_T_VHT_80,                          1560000,  1733000,        0,        0,        0,  1560000,  0x369 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     314 MCS-08 */ { VHT80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_80,                          1755000,  1950000,        0,        0,        0,  1755000,  0x36a },
     /*     315 MCS-09 */ { VHT80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_80,                          1950000,  2166700,        0,        0,        0,  1950000,  0x36b },
#endif
#endif
#if (NUM_SPATIAL_STREAMS > 4)
     /*     316 MCS-00 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                           146300,   162500,        0,        0,        0,   146300,  0x380 },
     /*     317 MCS-01 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                           292500,   325000,        0,        0,        0,   292500,  0x381 },
     /*     318 MCS-02 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                           438800,   487500,        0,        0,        0,   438800,  0x382 },
     /*     319 MCS-03 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                           585000,   650000,        0,        0,        0,   585000,  0x383 },
     /*     320 MCS-04 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                           877500,   975000,        0,        0,        0,   877500,  0x384 },
     /*     321 MCS-05 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          1170000,  1300000,        0,        0,        0,  1170000,  0x385 },
     /*     322 MCS-06 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          1316300,  1462500,        0,        0,        0,  1316300,  0x386 },
     /*     323 MCS-07 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          1462500,  1625000,        0,        0,        0,  1462500,  0x387 },
     /*     324 MCS-08 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          1755000,  1950000,        0,        0,        0,  1755000,  0x388 },
     /*     325 MCS-09 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          1950000,  2166700,        0,        0,        0,  1950000,  0x389 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     326 MCS-10 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          2193800,  2437500,        0,        0,        0,  2193800,  0x38a },
     /*     327 MCS-11 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          2437500,  2708300,        0,        0,        0,  2437500,  0x38b },
#endif
#endif
#if (NUM_SPATIAL_STREAMS > 5)
     /*     328 MCS-00 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                           175500,   195000,        0,        0,        0,   175500,  0x3a0 },
     /*     329 MCS-01 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                           351000,   390000,        0,        0,        0,   351000,  0x3a1 },
     /*     330 MCS-02 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                           526500,   585500,        0,        0,        0,   526500,  0x3a2 },
     /*     331 MCS-03 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                           702000,   780000,        0,        0,        0,   702000,  0x3a3 },
     /*     332 MCS-04 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          1053000,  1170000,        0,        0,        0,  1053000,  0x3a4 },
     /*     333 MCS-05 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          1404000,  1560000,        0,        0,        0,  1404000,  0x3a5 },
     /*     334 MCS-06 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          1579500,  1755000,        0,        0,        0,  1579500,  0x3a6 },
     /*     335 MCS-07 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          1755000,  1950000,        0,        0,        0,  1755000,  0x3a7 },
     /*     336 MCS-08 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          2106000,  2340000,        0,        0,        0,  2106000,  0x3a8 },
     /*     337 MCS-09 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          2340000,  2600000,        0,        0,        0,  2340000,  0x3a9 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     338 MCS-10 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          2632500,  2925000,        0,        0,        0,  2632500,  0x3aa },
     /*     339 MCS-11 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          2925000,  3250000,        0,        0,        0,  2925000,  0x3ab },
#endif
#endif
#if (NUM_SPATIAL_STREAMS > 6)
     /*     340 MCS-00 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                           204800,   227500,        0,        0,        0,   204800,  0x3c0 },
     /*     341 MCS-01 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                           409500,   455000,        0,        0,        0,   409500,  0x3c1 },
     /*     342 MCS-02 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                           614300,   682500,        0,        0,        0,   614300,  0x3c2 },
     /*     343 MCS-03 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                           819000,   910000,        0,        0,        0,   819000,  0x3c3 },
     /*     344 MCS-04 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          1288500,  1365000,        0,        0,        0,  1288500,  0x3c4 },
     /*     345 MCS-05 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          1638000,  1820000,        0,        0,        0,  1638000,  0x3c5 },
     /*     346 MCS-06 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          1842800,  2047500,        0,        0,        0,  1842800,  0x3c6 },
     /*     347 MCS-07 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          2047500,  2275000,        0,        0,        0,  2047500,  0x3c7 },
     /*     348 MCS-08 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          2457000,  2730000,        0,        0,        0,  2457000,  0x3c8 },
     /*     349 MCS-09 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          2730000,  3033300,        0,        0,        0,  2730000,  0x3c9 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     350 MCS-10 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          3071300,  3412500,        0,        0,        0,  3071300,  0x3ca },
     /*     351 MCS-11 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          3412500,  3791700,        0,        0,        0,  3412500,  0x3cb },
#endif
#endif
#if (NUM_SPATIAL_STREAMS > 7)
     /*     352 MCS-00 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                           234000,   260000,        0,        0,        0,   234000,  0x3e0 },
     /*     353 MCS-01 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                           468000,   520000,        0,        0,        0,   468000,  0x3e1},
     /*     354 MCS-02 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                           702000,   780000,        0,        0,        0,   702000,  0x3e2},
     /*     355 MCS-03 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                           936000,  1040000,        0,        0,        0,   936000,  0x3e3},
     /*     356 MCS-04 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          1404000,  1560000,        0,        0,        0,  1404000,  0x3e4},
     /*     357 MCS-05 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          1872000,  2080000,        0,        0,        0,  1872000,  0x3e5},
     /*     358 MCS-06 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          2106000,  2340000,        0,        0,        0,  2106000,  0x3e6},
     /*     359 MCS-07 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          2340000,  2600000,        0,        0,        0,  2340000,  0x3e7},
     /*     360 MCS-08 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          2808000,  3120000,        0,        0,        0,  2808000,  0x3e8},
     /*     361 MCS-09 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          3120000,  3466700,        0,        0,        0,  3120000,  0x3e9},
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     362 MCS-10 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          3510000,  3900000,        0,        0,        0,  3510000,  0x3ea},
     /*     363 MCS-11 */ { VHT80_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_VHT_80,                          3900000,  4333300,        0,        0,        0,  3900000,  0x3eb},
#endif
#endif

#if CONFIG_160MHZ_SUPPORT != 0
     /***********************************************************************************************************************************************************************************************/
     /* 11ac VHT160 rates                                                                                                                                                                           */
     /***********************************************************************************************************************************************************************************************/
     /*                                                                                                                SGI       DGI       QGI       DCM       User                                 */
     /*                       validModeMask                   phy                                     Kbps      Kbps      Kbps      Kbps      Kbps      Kbps    RC  x                        */
     /***********************************************************************************************************************************************************************************************/
     /*     364 MCS-00 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                           58500,    65000,        0,        0,        0,    58500,  0x300},
     /*     365 MCS-01 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          117000,   130000,        0,        0,        0,   117000,  0x301},
     /*     366 MCS-02 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          175500,   195000,        0,        0,        0,   175500,  0x302},
     /*     367 MCS-03 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          234000,   260000,        0,        0,        0,   234000,  0x303},
     /*     368 MCS-04 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          351000,   390000,        0,        0,        0,   351000,  0x304},
     /*     369 MCS-05 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          468000,   520000,        0,        0,        0,   468000,  0x305},
     /*     370 MCS-06 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          526500,   585000,        0,        0,        0,   526500,  0x306},
     /*     371 MCS-07 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          585000,   650000,        0,        0,        0,   585000,  0x307},
     /*     372 MCS-08 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          702000,   780000,        0,        0,        0,   702000,  0x308},
     /*     373 MCS-09 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          780000,   866700,        0,        0,        0,   780000,  0x309},
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     374 MCS-10 */ { VHT160_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_160,                          877500,   975000,        0,        0,        0,   877500,  0x30a },
     /*     375 MCS-11 */ { VHT160_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_160,                          975000,  1083300,        0,        0,        0,   975000,  0x30b },
#endif
#if (MAX_SPATIAL_STREAMS_SUPPORTED_AT_160MHZ > 1)
     /*     376 MCS-00 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          117000,   130000,        0,        0,        0,   117000,  0x320},
     /*     377 MCS-01 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          234000,   260000,        0,        0,        0,   234000,  0x321},
     /*     378 MCS-02 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          351000,   390000,        0,        0,        0,   351000,  0x322},
     /*     379 MCS-03 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          468000,   520000,        0,        0,        0,   468000,  0x323},
     /*     380 MCS-04 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          702000,   780000,        0,        0,        0,   702000,  0x324},
     /*     381 MCS-05 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          936000,  1040000,        0,        0,        0,   936000,  0x325},
     /*     382 MCS-06 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                         1053000,  1170000,        0,        0,        0,  1053000,  0x326},
     /*     383 MCS-07 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                         1170000,  1300000,        0,        0,        0,  1170000,  0x327},
     /*     384 MCS-08 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                         1404000,  1560000,        0,        0,        0,  1404000,  0x328},
     /*     385 MCS-09 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                         1560000,  1733300,        0,        0,        0,  1560000,  0x329},
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     386 MCS-10 */ { VHT160_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_160,                         1755000,  1950000,        0,        0,        0,  1755000,  0x32a},
     /*     387 MCS-11 */ { VHT160_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_160,                         1950000,  2166700,        0,        0,        0,  1950000,  0x32b},
#endif
#endif
#if (MAX_SPATIAL_STREAMS_SUPPORTED_AT_160MHZ > 2)
     /*     388 MCS-00 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          175500,   195000,        0,        0,        0,   175500,  0x340 },
     /*     389 MCS-01 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          351000,   390000,        0,        0,        0,   351000,  0x341 },
     /*     390 MCS-02 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          526500,   585000,        0,        0,        0,   526500,  0x342 },
     /*     391 MCS-03 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          702000,   780000,        0,        0,        0,   702000,  0x343 },
     /*     392 MCS-04 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                         1053000,  1170000,        0,        0,        0,  1053000,  0x344 },
     /*     393 MCS-05 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                         1404000,  1560000,        0,        0,        0,  1404000,  0x345 },
     /*     394 MCS-06 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                         1579500,  1755000,        0,        0,        0,  1579500,  0x346 },
     /*     395 MCS-07 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                         1755000,  1755000,        0,        0,        0,  1755000,  0x347 },
     /*     396 MCS-08 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                         2106000,  2340000,        0,        0,        0,  2106000,  0x348 },
     /*     397 MCS-09 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                         2340000,  2600000,        0,        0,        0,  2340000,  0x349 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     398 MCS-10 */ { VHT160_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_160,                         2632500,  2925000,        0,        0,        0,  2632500,  0x34a },
     /*     399 MCS-11 */ { VHT160_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_160,                         2925000,  3250000,        0,        0,        0,  2925000,  0x34b },
#endif
#endif
#if (MAX_SPATIAL_STREAMS_SUPPORTED_AT_160MHZ > 3)
     /*     400 MCS-00 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          234000,   260000,        0,        0,        0,   234000,  0x360 },
     /*     401 MCS-01 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          468000,   520000,        0,        0,        0,   468000,  0x361 },
     /*     402 MCS-02 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          702000,   780000,        0,        0,        0,   702000,  0x362 },
     /*     403 MCS-03 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                          936000,  1040000,        0,        0,        0,   936000,  0x363 },
     /*     404 MCS-04 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                         1404000,  1560000,        0,        0,        0,  1404000,  0x364 },
     /*     405 MCS-05 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                         1872000,  2080000,        0,        0,        0,  1872000,  0x365 },
     /*     406 MCS-06 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                         2106000,  2340000,        0,        0,        0,  2106000,  0x366 },
     /*     407 MCS-07 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                         2340000,  2600000,        0,        0,        0,  2340000,  0x367 },
     /*     408 MCS-08 */ { VHT160_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_160,                         2808000,  3120000,        0,        0,        0,  2808000,  0x368 },
     /*     409 MCS-09 */ { VHT160_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_160,                         3120000,  3466700,        0,        0,        0,  3120000,  0x369 },
#ifdef VHT_EXTRA_MCS_SUPPORT
     /*     410 MCS-10 */ { VHT160_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_160,                         3510000,  3900000,        0,        0,        0,  3510000,  0x36a },
     /*     411 MCS-11 */ { VHT160_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_VHT_160,                         3900000,  4333300,        0,        0,        0,  3900000,  0x36b },
#endif
#endif

#endif /* CONFIG_160MHZ_SUPPORT != 0 */

#if SUPPORT_11AX
     /***********************************************************************************************************************************************************************************************/
     /* 11ax RU242 rates                                                                                                                                                                            */
     /***********************************************************************************************************************************************************************************************/
     /*                                                                                                                SGI       DGI       QGI       DCM       User                                 */
     /*                       validModeMask                   phy                                     Kbps      Kbps      Kbps      Kbps      Kbps      Kbps    RC  x                        */
     /***********************************************************************************************************************************************************************************************/
     /*     412 MCS-00 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                              8600,     8900,     8100,     7300,     4300,     8600,  0x400},
     /*     413 MCS-01 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                             17200,    17700,    16300,    14600,     8600,    17200,  0x401},
     /*     414 MCS-02 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                             25800,    26600,    24400,    21900,        0,    25800,  0x402},
     /*     415 MCS-03 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                             34400,    35500,    32500,    29300,    17700,    34400,  0x403},
     /*     416 MCS-04 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                             51600,    53200,    48800,    43900,    25800,    51600,  0x404},
     /*     417 MCS-05 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                             68800,    70900,    65000,    58500,        0,    68800,  0x405},
     /*     418 MCS-06 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                             77400,    79800,    73100,    65800,        0,    77400,  0x406},
     /*     419 MCS-07 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                             86000,    88600,    81300,    73100,        0,    86000,  0x407},
     /*     420 MCS-08 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            103200,   106400,    97500,    87800,        0,   103200,  0x408},
     /*     421 MCS-09 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            114700,   118200,   108300,    97500,        0,   114700,  0x409},
     /*     422 MCS-10 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            129000,   133000,   121900,   109700,        0,   129000,  0x40a},
     /*     423 MCS-11 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            143400,   147700,   135400,   121900,        0,   143400,  0x40b},
#if (NUM_SPATIAL_STREAMS > 1)
     /*     424 MCS-00 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                             17200,    17700,    16300,    14600,     8600,    17200,  0x420},
     /*     425 MCS-01 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                             34400,    35500,    32500,    29300,    17700,    34400,  0x421},
     /*     426 MCS-02 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                             51600,    53200,    48800,    43900,        0,    51600,  0x422},
     /*     427 MCS-03 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                             68800,    70900,    65000,    58500,    34400,    68800,  0x423},
     /*     428 MCS-04 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            103200,   106400,    97500,    87800,    51600,   103200,  0x424},
     /*     429 MCS-05 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            137600,   141800,   130000,   117000,        0,   137600,  0x425},
     /*     430 MCS-06 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            154900,   159500,   146300,   131600,        0,   154900,  0x426},
     /*     431 MCS-07 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            172100,   177300,   162500,   146300,        0,   172100,  0x427},
     /*     432 MCS-08 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            206500,   212700,   195000,   175500,        0,   206500,  0x428},
     /*     433 MCS-09 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            229400,   236400,   216700,   195000,        0,   229400,  0x429},
     /*     434 MCS-10 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            258100,   265900,   243800,   219400,        0,   258100,  0x42a},
     /*     435 MCS-11 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            286800,   295500,   270800,   243800,        0,   286800,  0x42b},
#endif
#if (NUM_SPATIAL_STREAMS > 2)
     /*     436 MCS-00 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                             25800,    26600,    24400,    21900,    12900,    25800,  0x440},
     /*     437 MCS-01 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                             51600,    53200,    48800,    43900,    25800,    51600,  0x441},
     /*     438 MCS-02 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                             77400,    79800,    73100,    65800,        0,    77400,  0x442},
     /*     439 MCS-03 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            103200,   106400,    97500,    87800,    51600,   103200,  0x443},
     /*     440 MCS-04 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            154900,   159500,   146300,   131600,    77400,   154900,  0x444},
     /*     441 MCS-05 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            206500,   212700,   195000,   175500,        0,   206500,  0x445},
     /*     442 MCS-06 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            232300,   239300,   219400,   197400,        0,   232300,  0x446},
     /*     443 MCS-07 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            258100,   265900,   243800,   219400,        0,   258100,  0x447},
     /*     444 MCS-08 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            309700,   319100,   292500,   263300,        0,   309700,  0x448},
     /*     445 MCS-09 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            344100,   354500,   325000,   292500,        0,   344100,  0x449},
     /*     446 MCS-10 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            387100,   398900,   365600,   329100,        0,   387100,  0x44a},
     /*     447 MCS-11 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            430100,   443200,   406300,   365600,        0,   430100,  0x44b},
#endif
#if (NUM_SPATIAL_STREAMS > 3)
     /*     448 MCS-00 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                             34400,    35500,    32500,    29300,    17700,    34400,  0x460},
     /*     449 MCS-01 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                             68800,    70900,    65000,    58500,    34400,    68800,  0x461},
     /*     450 MCS-02 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            103200,   106400,    97500,    87800,        0,   103200,  0x462},
     /*     451 MCS-03 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            137600,   141800,   130000,   117000,    68800,   137600,  0x463},
     /*     452 MCS-04 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            206500,   212700,   195000,   175500,   103200,   206500,  0x464},
     /*     453 MCS-05 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            275300,   283600,   260000,   234000,        0,   275300,  0x465},
     /*     454 MCS-06 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            309700,   319100,   292500,   263300,        0,   309700,  0x466},
     /*     455 MCS-07 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            344100,   354500,   325000,   292500,        0,   344100,  0x467},
     /*     456 MCS-08 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            412900,   425500,   390000,   351000,        0,   412900,  0x468},
     /*     457 MCS-09 */ { HE20_MODE_VALID_MASK,   WHAL_MOD_IEEE80211_T_HE_20,                            455800,   472700,   433300,   390000,        0,   455800,  0x469},
     /*     458 MCS-10 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            516200,   531800,   487500,   438800,        0,   516200,  0x46a},
     /*     459 MCS-11 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            573500,   590900,   541700,   487500,        0,   573500,  0x46b},
#endif
#if (NUM_SPATIAL_STREAMS > 4)
     /*     460 MCS-00 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                             43000,    43300,    40600,    36600,    21500,    43000,  0x480},
     /*     461 MCS-01 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                             86000,    88600,    81300,    73100,    43000,    86000,  0x481},
     /*     462 MCS-02 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            129000,   133000,   121900,   109700,        0,   129000,  0x482},
     /*     463 MCS-03 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            172100,   177300,   162500,   146300,    86000,   172100,  0x483},
     /*     464 MCS-04 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            258100,   265900,   243800,   219400,   129000,   258100,  0x484},
     /*     465 MCS-05 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            344100,   354500,   325000,   292500,        0,   344100,  0x485},
     /*     466 MCS-06 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            387100,   398900,   365600,   329100,        0,   387100,  0x486},
     /*     467 MCS-07 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            430100,   443200,   406300,   365600,        0,   430100,  0x487},
     /*     468 MCS-08 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            516200,   531800,   487500,   438800,        0,   516200,  0x488},
     /*     469 MCS-09 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            573500,   590900,   541700,   487500,        0,   573500,  0x489},
     /*     470 MCS-10 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            645200,   664800,   609400,   548400,        0,   645200,  0x48a},
     /*     471 MCS-11 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            716900,   738600,   677100,   609400,        0,   716900,  0x48b},
#endif
#if (NUM_SPATIAL_STREAMS > 5)
     /*     472 MCS-00 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                             51600,    53200,    48800,    43900,    25800,    51600,  0x4a0},
     /*     473 MCS-01 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            103200,   106400,    97500,    87800,    51600,   103200,  0x4a1},
     /*     474 MCS-02 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            154900,   159500,   146300,   131600,        0,   154900,  0x4a2},
     /*     475 MCS-03 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            206500,   212700,   195000,   175500,   103200,   206500,  0x4a3},
     /*     476 MCS-04 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            309700,   319100,   292500,   263300,   154900,   309700,  0x4a4},
     /*     477 MCS-05 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            412900,   425500,   390000,   351000,        0,   412900,  0x4a5},
     /*     478 MCS-06 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            464600,   478600,   438000,   394900,        0,   464600,  0x4a6},
     /*     479 MCS-07 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            516200,   531800,   487500,   438800,        0,   516200,  0x4a7},
     /*     480 MCS-08 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            619400,   638200,   585000,   526500,        0,   619400,  0x4a8},
     /*     481 MCS-09 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            688200,   709100,   650000,   585000,        0,   688200,  0x4a9},
     /*     482 MCS-10 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            774300,   797700,   731300,   658100,        0,   774300,  0x4aa},
     /*     483 MCS-11 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            860300,   886400,   812500,   731300,        0,   860300,  0x4ab},
#endif
#if (NUM_SPATIAL_STREAMS > 6)
     /*     484 MCS-00 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                             60200,    62000,    56900,    51200,    30100,    60200,  0x4c0},
     /*     485 MCS-01 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            120400,   124100,   113800,   102400,    60200,   120400,  0x4c1},
     /*     486 MCS-02 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            180700,   186100,   170600,   153600,        0,   180700,  0x4c2},
     /*     487 MCS-03 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            240900,   248200,   227500,   204800,   120400,   240900,  0x4c3},
     /*     488 MCS-04 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            361300,   372300,   341300,   307100,   180700,   361300,  0x4c4},
     /*     489 MCS-05 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            481800,   496400,   455000,   409500,        0,   481800,  0x4c5},
     /*     490 MCS-06 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            542000,   558400,   511900,   460700,        0,   542000,  0x4c6},
     /*     491 MCS-07 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            602200,   620500,   568800,   511900,        0,   602200,  0x4c7},
     /*     492 MCS-08 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            722600,   744500,   682500,   614300,        0,   722600,  0x4c8},
     /*     493 MCS-09 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            802900,   827300,   758300,   682500,        0,   802900,  0x4c9},
     /*     494 MCS-10 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            903300,   930700,   853100,   767800,        0,   903300,  0x4ca},
     /*     495 MCS-11 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                           1003700,  1034100,   947900,   853100,        0,  1003700,  0x4cb},
#endif
#if (NUM_SPATIAL_STREAMS > 7)
     /*     496 MCS-00 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                             68800,    70900,    65000,    58500,    34400,    68800,  0x4e0},
     /*     497 MCS-01 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            137600,   141800,   130000,   117000,    68800,   137600,  0x4e1},
     /*     498 MCS-02 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            206500,   212700,   195000,   175500,        0,   206500,  0x4e2},
     /*     499 MCS-03 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            275300,   283600,   260000,   234000,   137600,   275300,  0x4e3},
     /*     500 MCS-04 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            412900,   425500,   390000,   351000,   206500,   412900,  0x4e4},
     /*     501 MCS-05 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            550600,   567300,   520000,   468000,        0,   550600,  0x4e5},
     /*     502 MCS-06 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            619400,   638200,   585000,   526500,        0,   619400,  0x4e6},
     /*     503 MCS-07 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            688200,   709100,   650000,   585000,        0,   688200,  0x4e7},
     /*     504 MCS-08 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            825900,   850900,   780000,   702000,        0,   825900,  0x4e8},
     /*     505 MCS-09 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                            917600,   945500,   866700,   780000,        0,   917600,  0x4e9},
     /*     506 MCS-10 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                           1032400,  1063600,   975000,   877500,        0,  1032400,  0x4ea},
     /*     507 MCS-11 */ { HE20_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_20,                           1147100,  1181800,  1083300,   975000,        0,  1147100,  0x4eb},
#endif

     /**********************************************************************************************************************************************************************/
     /* 11ax RU484 rates                                                                                                                                                   */
     /**********************************************************************************************************************************************************************/
     /*                                                                                                                SGI       DGI       QGI       DCM       User        */
     /*                       validModeMask                   phy                                     Kbps      Kbps      Kbps      Kbps      Kbps      Kbps    RC  */
     /**********************************************************************************************************************************************************************/
     /*     508 MCS-00 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                             17200,    17700,    16300,    14600,     8600,    17200,  0x400 },
     /*     509 MCS-01 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                             34400,    35500,    32500,    29300,    17700,    34400,  0x401 },
     /*     510 MCS-02 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                             51600,    53200,    48800,    43900,    25800,    51600,  0x402 },
     /*     511 MCS-03 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                             68800,    70900,    65000,    58500,        0,    68800,  0x403 },
     /*     512 MCS-04 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            103200,   106400,    97500,    87800,        0,   103200,  0x404 },
     /*     513 MCS-05 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            137600,   141800,   130000,   117000,        0,   137600,  0x405 },
     /*     514 MCS-06 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            154900,   159500,   146300,   131600,        0,   154900,  0x406 },
     /*     515 MCS-07 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            172100,   177300,   162500,   146300,        0,   172100,  0x407 },
     /*     516 MCS-08 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            206500,   212700,   195000,   175500,        0,   206500,  0x408 },
     /*     517 MCS-09 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            229400,   236400,   216700,   195000,        0,   229400,  0x409 },
     /*     518 MCS-10 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            258100,   265900,   243800,   219400,        0,   258100,  0x40a },
     /*     519 MCS-11 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            286800,   295500,   270800,   243800,        0,   286800,  0x40b },
#if (NUM_SPATIAL_STREAMS > 1)
     /*     520 MCS-00 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                             34400,    35500,    32500,    29300,    17700,    34400,  0x420 },
     /*     521 MCS-01 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                             68800,    70900,    65000,    58500,    34400,    68800,  0x421 },
     /*     522 MCS-02 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            103200,   106400,    97500,    87800,        0,   103200,  0x422 },
     /*     523 MCS-03 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            137600,   141800,   130000,   117000,    68800,   137600,  0x423 },
     /*     524 MCS-04 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            206500,   212700,   195000,   175500,   103200,   206500,  0x424 },
     /*     525 MCS-05 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            275300,   283600,   260000,   234000,        0,   275300,  0x425 },
     /*     526 MCS-06 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            309700,   319100,   292500,   263300,        0,   309700,  0x426 },
     /*     527 MCS-07 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            344100,   354500,   325000,   292500,        0,   344100,  0x427 },
     /*     528 MCS-08 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            412900,   425500,   390000,   351000,        0,   412900,  0x428 },
     /*     529 MCS-09 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            455800,   472700,   433300,   390000,        0,   455800,  0x429 },
     /*     530 MCS-10 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            516200,   531800,   487500,   438800,        0,   516200,  0x42a },
     /*     531 MCS-11 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            573500,   590900,   541700,   487500,        0,   573500,  0x42b },
#endif
#if (NUM_SPATIAL_STREAMS > 2)
     /*     532 MCS-00 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                             51600,    53200,    48800,    43900,    25800,    51600,  0x440 },
     /*     533 MCS-01 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            103200,   106400,    97500,    87800,    51600,   103200,  0x441 },
     /*     534 MCS-02 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            154900,   159500,   146300,   131600,        0,   154900,  0x442 },
     /*     535 MCS-03 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            206500,   212700,   195000,   175500,   103200,   206500,  0x443 },
     /*     536 MCS-04 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            309700,   319100,   292500,   263300,   154900,   309700,  0x444 },
     /*     537 MCS-05 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            412900,   425500,   390000,   351000,        0,   412900,  0x445 },
     /*     538 MCS-06 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            464600,   478600,   438000,   394900,        0,   464600,  0x446 },
     /*     539 MCS-07 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            516200,   531800,   487500,   438800,        0,   516200,  0x447 },
     /*     540 MCS-08 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            619400,   638200,   585000,   526500,        0,   619400,  0x448 },
     /*     541 MCS-09 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            688200,   709100,   650000,   585000,        0,   688200,  0x449 },
     /*     542 MCS-10 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            774300,   797700,   731300,   658100,        0,   774300,  0x44a },
     /*     543 MCS-11 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            860300,   886400,   812500,   731300,        0,   860300,  0x44b },
#endif
#if (NUM_SPATIAL_STREAMS > 3)
     /*     544 MCS-00 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                             68800,    70900,    65000,    58500,    34400,    68800,  0x460 },
     /*     545 MCS-01 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            137600,   141800,   130000,   117000,    68800,   137600,  0x461 },
     /*     546 MCS-02 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            206500,   212700,   195000,   175500,        0,   206500,  0x462 },
     /*     547 MCS-03 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            275300,   283600,   260000,   234000,   137600,   275300,  0x463 },
     /*     548 MCS-04 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            412900,   425500,   390000,   351000,   206500,   412900,  0x464 },
     /*     549 MCS-05 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            550600,   567300,   520000,   468000,        0,   550600,  0x465 },
     /*     550 MCS-06 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            619400,   638200,   585000,   526500,        0,   619400,  0x466 },
     /*     551 MCS-07 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            688200,   709100,   650000,   585000,        0,   688200,  0x467 },
     /*     552 MCS-08 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            825900,   850900,   780000,   702000,        0,   825900,  0x468 },
     /*     553 MCS-09 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            917600,   945500,   866700,   780000,        0,   917600,  0x469 },
     /*     554 MCS-10 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1032400,  1063600,   975000,   877500,        0,  1032400,  0x46a },
     /*     555 MCS-11 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1147100,  1181800,  1083300,   975000,        0,  1147100,  0x46b },
#endif
#if (NUM_SPATIAL_STREAMS > 4)
     /*     556 MCS-00 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                             86000,    88600,    81300,    73100,    43000,    86000,  0x480 },
     /*     557 MCS-01 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            172100,   177300,   162500,   146300,    86000,   172100,  0x481 },
     /*     558 MCS-02 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            258100,   265900,   243800,   219400,        0,   258100,  0x482 },
     /*     559 MCS-03 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            344100,   354500,   325000,   292500,   172100,   344100,  0x483 },
     /*     560 MCS-04 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            516200,   531800,   487500,   438800,   258100,   516200,  0x484 },
     /*     561 MCS-05 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            688200,   709100,   650000,   585000,        0,   688200,  0x485 },
     /*     562 MCS-06 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            774300,   797700,   731300,   658100,        0,   774300,  0x486 },
     /*     563 MCS-07 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            860300,   886400,   812500,   731300,        0,   860300,  0x487 },
     /*     564 MCS-08 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1032400,  1063600,   975000,   877500,        0,  1032400,  0x488 },
     /*     565 MCS-09 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1147100,  1181800,  1083300,   975000,        0,  1147100,  0x489 },
     /*     566 MCS-10 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1290400,  1329500,  1218800,  1096900,        0,  1290400,  0x48a },
     /*     567 MCS-11 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1433800,  1477300,  1354200,  1218800,        0,  1433800,  0x48b },
#endif
#if (NUM_SPATIAL_STREAMS > 5)
     /*     568 MCS-00 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            103200,   106400,    97500,    87800,    51600,   103200,  0x4a0 },
     /*     569 MCS-01 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            206500,   212700,   195000,   175500,   103200,   206500,  0x4a1 },
     /*     570 MCS-02 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            309700,   319100,   292500,   263300,        0,   309700,  0x4a2 },
     /*     571 MCS-03 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            412900,   425500,   390000,   351000,   206500,   412900,  0x4a3 },
     /*     572 MCS-04 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            619400,   638200,   585000,   526500,   309700,   619400,  0x4a4 },
     /*     573 MCS-05 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            825900,   850900,   780000,   702000,        0,   825900,  0x4a5 },
     /*     574 MCS-06 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            929100,   957300,   877500,   789800,        0,   929100,  0x4a6 },
     /*     575 MCS-07 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1032400,  1063600,   975000,   877500,        0,  1032400,  0x4a7 },
     /*     576 MCS-08 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1238800,  1276400,  1170000,  1053000,        0,  1238800,  0x4a8 },
     /*     577 MCS-09 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1376500,  1418200,  1300000,  1170000,        0,  1376500,  0x4a9 },
     /*     578 MCS-10 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1548500,  1595500,  1462500,  1316300,        0,  1548500,  0x4aa },
     /*     579 MCS-11 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1720600,  1772700,  1625000,  1462500,        0,  1720600,  0x4ab },
#endif
#if (NUM_SPATIAL_STREAMS > 6)
     /*     580 MCS-00 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            120400,   124100,   113800,   102400,    60200,   120400,  0x4c0 },
     /*     581 MCS-01 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            240900,   248200,   227500,   204800,   120400,   240900,  0x4c1 },
     /*     582 MCS-02 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            361300,   372300,   341300,   307100,   180600,   361300,  0x4c2 },
     /*     583 MCS-03 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            481800,   496400,   455000,   409500,        0,   481800,  0x4c3 },
     /*     584 MCS-04 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            722600,   744500,   682500,   614300,        0,   722600,  0x4c4 },
     /*     585 MCS-05 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            963500,   992700,   910000,   819000,        0,   963500,  0x4c5 },
     /*     586 MCS-06 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1084000,  1116800,  1023800,   921400,        0,  1084000,  0x4c6 },
     /*     587 MCS-07 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1204400,  1240900,  1137500,  1023800,        0,  1204400,  0x4c7 },
     /*     588 MCS-08 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1445300,  1489100,  1365000,  1228500,        0,  1445300,  0x4c8 },
     /*     589 MCS-09 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1605900,  1654500,  1516700,  1365000,        0,  1605900,  0x4c9 },
     /*     590 MCS-10 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1806600,  1861400,  1706300,  1535600,        0,  1806600,  0x4ca },
     /*     591 MCS-11 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           2007400,  2068200,  1895800,  1706300,        0,  2007400,  0x4cb },
#endif
#if (NUM_SPATIAL_STREAMS > 7)
     /*     592 MCS-00 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            137600,   141800,   130000,   117000,    68800,   137600,  0x4e0 },
     /*     593 MCS-01 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            275300,   283600,   260000,   234000,   137600,   275300,  0x4e1 },
     /*     594 MCS-02 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            412900,   425500,   390000,   351000,   206500,   412900,  0x4e2 },
     /*     595 MCS-03 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            550600,   567300,   520000,   468000,        0,   550600,  0x4e3 },
     /*     596 MCS-04 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                            825900,   850900,   780000,   702000,        0,   825900,  0x4e4 },
     /*     597 MCS-05 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1101200,  1134500,  1040000,   936000,        0,  1101200,  0x4e5 },
     /*     598 MCS-06 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1238800,  1276400,  1170000,  1053000,        0,  1238800,  0x4e6 },
     /*     599 MCS-07 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1376500,  1418200,  1300000,  1170000,        0,  1376500,  0x4e7 },
     /*     600 MCS-08 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1651800,  1701800,  1560000,  1404000,        0,  1651800,  0x4e8 },
     /*     601 MCS-09 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           1835300,  1890900,  1733300,  1560000,        0,  1835300,  0x4e9 },
     /*     602 MCS-10 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           2064700,  2127300,  1950000,  1755000,        0,  2064700,  0x4ea },
     /*     603 MCS-11 */ { HE40_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_40,                           2294100,  2363600,  2166700,  1950000,        0,  2294100,  0x4eb },
#endif

     /***********************************************************************************************************************************************************************************************/
     /* 11ax RU996 rates                                                                                                                                                                            */
     /***********************************************************************************************************************************************************************************************/
     /*                                                                                                                SGI       DGI       QGI       DCM       User                            */
     /*                       validModeMask                   phy                                     Kbps      Kbps      Kbps      Kbps      Kbps      Kbps    RC                      */
     /***********************************************************************************************************************************************************************************************/
     /*     604 MCS-00 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                             36000,    37100,    34000,    30600,    18000,    36000,  0x400 },
     /*     605 MCS-01 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                             72100,    74200,    68100,    61300,    36000,    72100,  0x401 },
     /*     606 MCS-02 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            108100,   111400,   102100,    91900,        0,   108100,  0x402 },
     /*     607 MCS-03 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            144100,   148500,   136100,   122500,    72100,   144100,  0x403 },
     /*     608 MCS-04 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            216200,   222700,   204200,   183800,   108100,   216200,  0x404 },
     /*     609 MCS-05 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            288200,   297000,   272200,   245000,        0,   288200,  0x405 },
     /*     610 MCS-06 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            324300,   334100,   306300,   275600,        0,   324300,  0x406 },
     /*     611 MCS-07 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            360300,   371200,   340300,   306300,        0,   360300,  0x407 },
     /*     612 MCS-08 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            432400,   445500,   408300,   367500,        0,   432400,  0x408 },
     /*     613 MCS-09 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            480400,   494900,   453700,   408300,        0,   480400,  0x409 },
     /*     614 MCS-10 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            540400,   556800,   510400,   459400,        0,   540400,  0x40a },
     /*     615 MCS-11 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            600500,   618700,   567100,   510400,        0,   600500,  0x40b },
#if (NUM_SPATIAL_STREAMS > 1)
     /*     616 MCS-00 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                             72100,    74200,    68100,    61300,    36000,    72100,  0x420 },
     /*     617 MCS-01 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            144100,   148500,   136100,   122500,    72100,   144100,  0x421 },
     /*     618 MCS-02 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            216200,   222700,   204200,   183800,        0,   216200,  0x422 },
     /*     619 MCS-03 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            288200,   297000,   272200,   245000,   144100,   288200,  0x423 },
     /*     620 MCS-04 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            432400,   445500,   408300,   367500,   216200,   432400,  0x424 },
     /*     621 MCS-05 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            576500,   593900,   544400,   490000,        0,   576500,  0x425 },
     /*     622 MCS-06 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            648500,   668200,   612500,   551300,        0,   648500,  0x426 },
     /*     623 MCS-07 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            720600,   742400,   680600,   612500,        0,   720600,  0x427 },
     /*     624 MCS-08 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            864700,   890900,   816700,   735000,        0,   864700,  0x428 },
     /*     625 MCS-09 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            960800,   989900,   907400,   816700,        0,   960800,  0x429 },
     /*     626 MCS-10 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1080900,  1113600,  1020800,   918800,        0,  1080900,  0x42a },
     /*     627 MCS-11 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1201000,  1237400,  1134300,  1020800,        0,  1201000,  0x42b },
#endif
#if (NUM_SPATIAL_STREAMS > 2)
     /*     628 MCS-00 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            108100,   111400,   102100,    91900,    54000,   108100,  0x440 },
     /*     629 MCS-01 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            216200,   222700,   204200,   183800,   108100,   216200,  0x441 },
     /*     630 MCS-02 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            324300,   334100,   306300,   275600,        0,   324300,  0x442 },
     /*     631 MCS-03 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            432400,   445500,   408300,   367500,        0,   432400,  0x443 },
     /*     632 MCS-04 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            648500,   668200,   612500,   551300,        0,   648500,  0x444 },
     /*     633 MCS-05 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            864700,   890900,   816700,   735000,        0,   864700,  0x445 },
     /*     634 MCS-06 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            972800,  1002300,   918800,   826900,        0,   972800,  0x446 },
     /*     635 MCS-07 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1080900,  1113600,  1020800,   918800,        0,  1080900,  0x447 },
     /*     636 MCS-08 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1297100,  1336400,  1225000,  1102500,        0,  1297100,  0x448 },
     /*     637 MCS-09 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1441200,  1484800,  1361100,  1225000,        0,  1441200,  0x449 },
     /*     638 MCS-10 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1621300,  1670500,  1531300,  1378100,        0,  1621300,  0x44a },
     /*     639 MCS-11 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1801500,  1856100,  1701400,  1531300,        0,  1801500,  0x44b },
#endif
#if (NUM_SPATIAL_STREAMS > 3)
     /*     640 MCS-00 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            144100,   148500,   136100,   122500,    72100,   144100,  0x460 },
     /*     641 MCS-01 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            288200,   297000,   272200,   245000,   144100,   288200,  0x461 },
     /*     642 MCS-02 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            432400,   445500,   408300,   367500,        0,   432400,  0x462 },
     /*     643 MCS-03 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            576500,   593900,   544400,   490000,   288200,   576500,  0x463 },
     /*     644 MCS-04 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            864700,   890900,   816700,   735000,   432400,   864700,  0x464 },
     /*     645 MCS-05 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1152900,  1187900,  1088900,   980000,        0,  1152900,  0x465 },
     /*     646 MCS-06 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1297100,  1336400,  1225000,  1102500,        0,  1297100,  0x466 },
     /*     647 MCS-07 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1441200,  1484800,  1361100,  1225000,        0,  1441200,  0x467 },
     /*     648 MCS-08 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1729400,  1781800,  1633300,  1470000,        0,  1729400,  0x468 },
     /*     649 MCS-09 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1921600,  1979800,  1814800,  1633300,        0,  1921600,  0x469 },
     /*     650 MCS-10 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           2161800,  2227300,  2041700,  1837500,        0,  2161800,  0x46a },
     /*     651 MCS-11 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           2402000,  2474700,  2268500,  2041700,        0,  2402000,  0x46b },
#endif
#if (NUM_SPATIAL_STREAMS > 4)
     /*     652 MCS-00 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            180100,   185600,   170100,   153100,    90100,   180100,  0x480 },
     /*     653 MCS-01 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            360300,   371200,   340300,   306300,   180100,   360300,  0x481 },
     /*     654 MCS-02 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            540400,   556800,   510400,   459400,        0,   540400,  0x482 },
     /*     655 MCS-03 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            720600,   742400,   680600,   612500,        0,   720600,  0x483 },
     /*     656 MCS-04 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1080900,  1113600,  1020800,   918800,        0,  1080900,  0x484 },
     /*     657 MCS-05 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1441200,  1484800,  1361100,  1225000,        0,  1441200,  0x485 },
     /*     658 MCS-06 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1621300,  1670500,  1531300,  1378100,        0,  1621300,  0x486 },
     /*     659 MCS-07 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1801500,  1856100,  1701400,  1531300,        0,  1801500,  0x487 },
     /*     660 MCS-08 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           2161800,  2227300,  2041700,  1837500,        0,  2161800,  0x488 },
     /*     661 MCS-09 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           2402000,  2474700,  2268500,  2041700,        0,  2402000,  0x489 },
     /*     662 MCS-10 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           2702200,  2784100,  2552100,  2296900,        0,  2702200,  0x48a },
     /*     663 MCS-11 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           3002500,  3093400,  2835600,  2552100,        0,  3002500,  0x48b },
#endif
#if (NUM_SPATIAL_STREAMS > 5)
     /*     664 MCS-00 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            216200,   222700,   204200,   183800,   108100,   216200,  0x4a0 },
     /*     665 MCS-01 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            432400,   445500,   408300,   367500,   216200,   432400,  0x4a1 },
     /*     666 MCS-02 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            648500,   668200,   612500,   551300,        0,   648500,  0x4a2 },
     /*     667 MCS-03 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            864700,   890900,   816700,   735000,   432400,   864700,  0x4a3 },
     /*     668 MCS-04 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1297100,  1336400,  1225000,  1102500,   648500,  1297100,  0x4a4 },
     /*     669 MCS-05 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1729400,  1781800,  1633300,  1470000,        0,  1729400,  0x4a5 },
     /*     670 MCS-06 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1945600,  2004500,  1837500,  1653800,        0,  1945600,  0x4a6 },
     /*     671 MCS-07 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           2161800,  2227300,  2041700,  1837500,        0,  2161800,  0x4a7 },
     /*     672 MCS-08 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           2594100,  2672700,  2450000,  2205000,        0,  2594100,  0x4a8 },
     /*     673 MCS-09 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           2882400,  2969700,  2722200,  2450000,        0,  2882400,  0x4a9 },
     /*     674 MCS-10 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           3242600,  3340900,  3062500,  2756300,        0,  3242600,  0x4aa },
     /*     675 MCS-11 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           3602900,  3712100,  3402800,  3062500,        0,  3602900,  0x4ab },
#endif
#if (NUM_SPATIAL_STREAMS > 6)
     /*     676 MCS-00 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            252200,   259800,   238200,   214400,   129900,   252200,  0x4c0 },
     /*     677 MCS-01 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            504400,   519700,   476400,   428800,   252200,   504400,  0x4c1 },
     /*     678 MCS-02 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            756600,   779500,   714600,   643100,        0,   756600,  0x4c2 },
     /*     679 MCS-03 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1008800,  1039400,   952800,   857500,   504400,  1008800,  0x4c3 },
     /*     680 MCS-04 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1513200,  1559100,  1429200,  1286300,   756600,  1513200,  0x4c4 },
     /*     681 MCS-05 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           2017600,  2078800,  1905600,  1715000,        0,  2017600,  0x4c5 },
     /*     682 MCS-06 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           2269900,  2338600,  2143800,  1929400,        0,  2269900,  0x4c6 },
     /*     683 MCS-07 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           2522100,  2598500,  2381900,  2143800,        0,  2522100,  0x4c7 },
     /*     684 MCS-08 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           3026500,  3118200,  2858300,  2572500,        0,  3026500,  0x4c8 },
     /*     685 MCS-09 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           3362700,  3464600,  3175900,  2858300,        0,  3362700,  0x4c9 },
     /*     686 MCS-10 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           3783100,  3897700,  3572900,  3215600,        0,  3783100,  0x4ca },
     /*     687 MCS-11 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           4203400,  4330800,  3969900,  3572900,        0,  4203400,  0x4cb },
#endif
#if (NUM_SPATIAL_STREAMS > 7)
     /*     688 MCS-00 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            288200,   297000,   272200,   245000,   144100,   288200,  0x4e0 },
     /*     689 MCS-01 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            576500,   593900,   544400,   490000,   288200,   576500,  0x4e1 },
     /*     690 MCS-02 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                            864700,   890900,   816700,   735000,        0,   864700,  0x4e2 },
     /*     691 MCS-03 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1152900,  1187900,  1088900,   980000,   576500,  1152900,  0x4e3 },
     /*     692 MCS-04 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           1729400,  1781800,  1633300,  1470000,   864700,  1729400,  0x4e4 },
     /*     693 MCS-05 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           2305900,  2375800,  2177800,  1960000,        0,  2305900,  0x4e5 },
     /*     694 MCS-06 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           2594100,  2672700,  2450000,  2205000,        0,  2594100,  0x4e6 },
     /*     695 MCS-07 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           2882400,  2969700,  2722200,  2450000,        0,  2882400,  0x4e7 },
     /*     696 MCS-08 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           3458800,  3563600,  3266700,  2940000,        0,  3458800,  0x4e8 },
     /*     697 MCS-09 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           3843100,  3959600,  3629600,  3266700,        0,  3843100,  0x4e9 },
     /*     698 MCS-10 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           4323500,  4454500,  4083300,  3675000,        0,  4323500,  0x4ea },
     /*     699 MCS-11 */ { HE80_LDPC_ONLY_MASKS,   WHAL_MOD_IEEE80211_T_HE_80,                           4803900,  4949500,  4537000,  4083300,        0,  4803900,  0x4eb },
#endif

#if CONFIG_160MHZ_SUPPORT != 0
     /***********************************************************************************************************************************************************************************************/
     /* 11ax RU996x2 rates                                                                                                                                                                          */
     /***********************************************************************************************************************************************************************************************/
     /*                                                                                                                SGI       DGI       QGI       DCM       User                                 */
     /*                       validModeMask                   phy                                     Kbps      Kbps      Kbps      Kbps      Kbps      Kbps    RC                         */
     /***********************************************************************************************************************************************************************************************/
     /*     700 MCS-00 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                            72100,    74200,    68100,    61300,    36000,    72100,  0x400},
     /*     701 MCS-01 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           144100,   148500,   136100,   122500,    72100,   144100,  0x401},
     /*     702 MCS-02 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           216200,   222700,   204200,   183800,        0,   216200,  0x402},
     /*     703 MCS-03 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           288200,   297000,   272200,   245000,   144100,   288200,  0x403},
     /*     704 MCS-04 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           432400,   445500,   408300,   367500,   216200,   432400,  0x404},
     /*     705 MCS-05 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           576500,   593900,   544400,   490000,        0,   576500,  0x405},
     /*     706 MCS-06 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           648500,   668200,   612500,   551300,        0,   648500,  0x406},
     /*     707 MCS-07 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           720600,   742400,   680600,   612500,        0,   720600,  0x407},
     /*     708 MCS-08 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           864700,   890900,   816700,   735000,        0,   864700,  0x408},
     /*     709 MCS-09 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           960800,   989900,   907400,   816700,        0,   960800,  0x409},
     /*     710 MCS-10 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          1080900,  1113600,  1020800,   918800,        0,  1080900,  0x40a},
     /*     711 MCS-11 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          1201000,  1237400,  1134300,  1020800,        0,  1201000,  0x40b},
#if (MAX_SPATIAL_STREAMS_SUPPORTED_AT_160MHZ > 1)
     /*     712 MCS-00 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           144100,   148500,   136100,   122500,    72100,   144100,  0x420},
     /*     713 MCS-01 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           288200,   297000,   272200,   245000,   144100,   288200,  0x421},
     /*     714 MCS-02 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           432400,   445500,   408300,   367500,        0,   432400,  0x422},
     /*     715 MCS-03 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           576500,   593900,   544400,   490000,   288200,   576500,  0x423},
     /*     716 MCS-04 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           864700,   890900,   816700,   735000,   432400,   864700,  0x424},
     /*     717 MCS-05 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          1152900,  1187900,  1088900,   980000,        0,  1152900,  0x425},
     /*     718 MCS-06 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          1297100,  1336400,  1225000,  1102500,        0,  1297100,  0x426},
     /*     719 MCS-07 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          1441200,  1484800,  1361100,  1225000,        0,  1441200,  0x427},
     /*     720 MCS-08 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          1729400,  1781800,  1633300,  1470000,        0,  1729400,  0x428},
     /*     721 MCS-09 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          1921600,  1979800,  1814800,  1633300,        0,  1921600,  0x429},
     /*     722 MCS-10 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          2161800,  2227300,  2041700,  1837500,        0,  2161800,  0x42a},
     /*     723 MCS-11 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          2402000,  2474700,  2268500,  2041700,        0,  2402000,  0x42b},
#endif
#if (MAX_SPATIAL_STREAMS_SUPPORTED_AT_160MHZ > 2)
     /*     724 MCS-00 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           216200,   222700,   204200,   183800,   108100,   216200,  0x440},
     /*     725 MCS-01 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           432400,   445500,   408300,   367500,   216200,   432400,  0x441},
     /*     726 MCS-02 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           648500,   668200,   612500,   551300,        0,   648500,  0x442},
     /*     727 MCS-03 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           864700,   890900,   816700,   735000,   432400,   864700,  0x443},
     /*     728 MCS-04 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          1297100,  1336400,  1225000,  1102500,   648500,  1297100,  0x444},
     /*     729 MCS-05 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          1729400,  1781800,  1633300,  1470000,        0,  1729400,  0x445},
     /*     730 MCS-06 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          1945600,  2004500,  1837500,  1653800,        0,  1945600,  0x446},
     /*     731 MCS-07 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          2161800,  2227300,  2041700,  1837500,        0,  2161800,  0x447},
     /*     732 MCS-08 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          2594100,  2672700,  2450000,  2205000,        0,  2594100,  0x448},
     /*     733 MCS-09 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          2882400,  2969700,  2722200,  2450000,        0,  2882400,  0x449},
     /*     734 MCS-10 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          3242600,  3340900,  3062500,  2756300,        0,  3242600,  0x44a},
     /*     735 MCS-11 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          3602900,  3712100,  3402800,  3062500,        0,  3602900,  0x44b},
#endif
#if (MAX_SPATIAL_STREAMS_SUPPORTED_AT_160MHZ > 3)
     /*     736 MCS-00 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           288200,   297000,   272200,   245000,   144100,   288200,  0x460},
     /*     737 MCS-01 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           576500,   593900,   544400,   490000,   288200,   576500,  0x461},
     /*     738 MCS-02 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                           864700,   890900,   816700,   735000,        0,   864700,  0x462},
     /*     739 MCS-03 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          1152900,  1187900,  1088900,   980000,   576500,  1152900,  0x463},
     /*     740 MCS-04 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          1729400,  1781800,  1633300,  1470000,   864700,  1729400,  0x464},
     /*     741 MCS-05 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          2305900,  2375800,  2177800,  1960000,        0,  2305900,  0x465},
     /*     742 MCS-06 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          2594100,  2672700,  2450000,  2205000,        0,  2594100,  0x466},
     /*     743 MCS-07 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          2882400,  2969700,  2722200,  2450000,        0,  2882400,  0x467},
     /*     744 MCS-08 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          3458800,  3563600,  3266700,  2940000,        0,  3458800,  0x468},
     /*     745 MCS-09 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          3843100,  3959600,  3629600,  3266700,        0,  3843100,  0x469},
     /*     746 MCS-10 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          4323500,  4454500,  4083300,  3675000,        0,  4323500,  0x46a},
     /*     747 MCS-11 */ { HE160_LDPC_ONLY_MASKS,  WHAL_MOD_IEEE80211_T_HE_160,                          4803900,  4949500,  4537000,  4083300,        0,  4803900,  0x46b}
#endif

#endif /* CONFIG_160MHZ_SUPPORT != 0 */
#endif /* SUPPORT_11AX */

#else /* (NUM_SPATIAL_STREAMS > 4) || SUPPORT_11AX */
     /*                                                                                        SGI                 */
     /*                                                                                Kbps    Kbps   uKbps   RC   */
     /* 0   11 Mb */ { CCK_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_CCK,                 11000,  11000,  8100,  0x40 },
     /* 1  5.5 Mb */ { CCK_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_CCK,                  5500,   5500,  4900,  0x41 },
     /* 2    2 Mb */ { CCK_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_CCK,                  2000,   2000,  1900,  0x42 },
     /* 3    1 Mb */ { CCK_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_CCK,                  1000,   1000,   900,  0x43 },
     /* 4   48 Mb */ { OFDM_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_OFDM,                48000,  48000, 27400,  0x00 },
     /* 5   24 Mb */ { OFDM_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_OFDM,                24000,  24000, 17700,  0x01 },
     /* 6   12 Mb */ { OFDM_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_OFDM,                12000,  12000, 10100,  0x02 },
     /* 7    6 Mb */ { OFDM_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_OFDM,                 6000,   6000,  5400,  0x03 },
     /* 8   54 Mb */ { OFDM_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_OFDM,                54000,  54000, 30900,  0x04 }
     /* 9   36 Mb */ { OFDM_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_OFDM,                36000,  36000, 23700,  0x05 },
     /* 10   18 Mb */ { OFDM_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_OFDM,                18000,  18000, 14100, 0x06     },
     /* 11    9 Mb */ { OFDM_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_OFDM,                 9000,   9000,  7800, 0x07     },

/* 11n HT rates */

     /*12  6.5 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,                6500,   7200,  5850,  0x80     },
     /*13   13 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,               13000,  14400, 11700,  0x81     },
     /*14 19.5 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,               19500,  21700, 17650,  0x82     },
     /*15   26 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,               26000,  28900, 23490,  0x83     },
     /*16   39 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,               39000,  43300, 35410,  0x84     },
     /*17   52 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,               52000,  57800, 47050,  0x85     },
     /*18 58.5 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,               58500,  65000, 53120,  0x86     },
     /*19   65 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,               65000,  72200, 59130,  0x87     },
#if (NUM_SPATIAL_STREAMS > 1)
     /*20   13 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,               13000,  14400, 11620,  0x90     },
     /*21   26 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,               26000,  28900, 23400,  0x91     },
     /*22   39 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,               39000,  43300, 35270,  0x92     },
     /*23   52 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,               52000,  57800, 47000,  0x93     },
     /*24   78 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,               78000,  86700, 70750,  0x94     },
     /*25  104 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,              104000, 115600, 94450,  0x95     },
     /*26  117 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,              117000, 130000, 106130, 0x96    },
     /*27  130 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,              130000, 144000, 118140, 0x97    },
#endif

#if (NUM_SPATIAL_STREAMS > 2)
     /*28   19.5 Mb*/{ HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,              19500,  21700,  17500,  0xa0 },
     /*29   39 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,               39000,  43300,  35200, 0xa1},
     /*30   58.5 Mb*/{ HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,               58500,  65000,  52800, 0xa2},
     /*31   78 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,               78000,  86700,  70610, 0xa3},
     /*32   117 Mb */{ HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,              117000, 130000, 105920, 0xa4},
     /*33  156 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,              156000, 173300, 141220, 0xa5},
     /*34  175.5 Mb*/{ HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,              175500, 195000, 158880, 0xa6},
     /*35  195 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,              195000, 216700, 176870, 0xa7},
#endif
 /* HT MCS0-9 NSS 4 20 MHz */
#if (NUM_SPATIAL_STREAMS > 3)

      /*36  26 Mb */ { HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,               26000, 28900,  23210,  0xb0 },
      /*37   52 Mb */ {HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,               52000, 57800,  46750,  0xb1 },
      /*38 78 Mb */ {  HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,               78000, 86700,  70400,  0xb2 },
      /*39   104 Mb */{HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,              104000, 115600, 94270,  0xb3 },
      /*40   156 Mb */{HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,              156000, 173300, 141220, 0xb4 },
      /*41   208 Mb */{HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,              208000, 231100, 188530, 0xb5 },
      /*42 234Mb */ {  HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,              234000, 260000, 212180, 0xb6 },
      /*43   260 Mb */{HT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_20,              260000, 288900, 235830, 0xb7 },

#endif


     /*44 13.5 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,               13500,  15000, 12060,  0x80},
     /*45 27.0 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,               27000,  30000, 24260,  0x81},
     /*46 40.5 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,               40500,  45000, 36590,  0x82},
     /*47   54 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,               54000,  60000, 48760,  0x83},
     /*48   81 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,               81500,  90000, 73400,  0x84},
     /*49  108 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,              108000, 120000, 97970,  0x85},
     /*50 121.5Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,              121500, 135000,110550,  0x86},
     /*51  135 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,              135000, 150000,122590,  0x87},

#if (NUM_SPATIAL_STREAMS > 1)
     /*52   27 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,               27000,  30000, 24090,  0x90  },
     /*53   54 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,               54000,  60000, 48550,  0x91  },
     /*54   81 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,               81000,  90000, 73100,  0x92  },
     /*55  108 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,              108000, 120000, 97870,  0x93  },
     /*56  162 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,              162000, 180000, 147110, 0x94 },
     /*57  216 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,              216000, 240000, 196340, 0x95 },
     /*58  243 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,              243000, 270000, 220880, 0x96 },
     /*59  270 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,              270000, 300000, 245420, 0x97 },
#endif

#if (NUM_SPATIAL_STREAMS > 2)

     /*60  40.5 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,              40500,  45000,  36250, 0xa0     },
     /*61   54 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,               81000,  90000,  72940, 0xa1     },
     /*62   81 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,              121500, 135000, 109910, 0xa2     },
     /*63  108 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,              162000, 180000, 146820, 0xa3     },
     /*64  162 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,              243000, 270000, 220460, 0xa4     },
     /*65  216 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,              324000, 360000, 293950, 0xa5     },
     /*66  243 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,              364500, 405000, 330680, 0xa6     },
     /*67  270 Mb */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,              405000, 450000, 367070, 0xa7     },
#endif

#if (NUM_SPATIAL_STREAMS > 3)

       /*68  */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,                54000,   60000,  48120,    0xb0},
       /*69   */ {HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,               108000,  120000,  97380,    0xb1},
       /*70  */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,               162000,  180000, 146390,    0xb2},
       /*71   */ {HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,               216000,  240000, 195960,    0xb3},
       /*72   */ {HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,               324000,  360000, 293950,    0xb4},
       /*73   */ {HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,               432000,  480000, 391920,    0xb5},
       /*74  */ { HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,               486000,  540000, 440910,    0xb6},
       /*75   */ {HT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_HT_40,               540000,  600000, 489900,    0xb7},

#endif

/*
                 TODO
                 Fix all arguments to be consistents with definitions
*/

/* VHT MCS0-9 NSS 1 20 MHz */

     /*76 6.5 Mb */ {VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                       6500,   7200,  5850,  0xc0 },
     /*77   13 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    13000,  14400, 11700,  0xc1 },
     /*78 19.5 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    19500,  21700, 17650,  0xc2 },
     /*79   26 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    26000,  28900, 23490,  0xc3 },
     /*80   39 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    39000,  43300, 35410,  0xc4 },
     /*81   52 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    52000,  57800, 47050,  0xc5 },
     /*82 58.5 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    58500,  65000, 53120,  0xc6 },
     /*83   65 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    65000,  72200, 59130,  0xc7 },
     /*84   78 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    78000,  86700, 70820,  0xc8 },
     /*85   MCS9 */ { VHT_INVALID_BCC_RATE,  WHAL_MOD_IEEE80211_T_VHT_20,                    86500,  96000,  78730, 0xc9 },



/* VHT MCS0-9 NSS 2 20 MHz */
#if (NUM_SPATIAL_STREAMS > 1)

      /*86  13 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    13000,  14400,  11620,  0xd0 },
      /*87  26 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    26000,  28900,  23400,  0xd1 },
      /*88  39 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    39000,  43300,  35270,  0xd2 },
      /*89  52 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    52000,  57800,  47000,  0xd3 },
      /*90  78 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    78000,  86700,  70750,  0xd4 },
      /*91  104 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                  104000, 115600,  94450,  0xd5 },
      /*92  117 Mb */ {VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                   117000, 130000, 106130,  0xd6 },
      /*93  130 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                  130000, 144400, 118140,  0xd7 },
      /*94  156Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                   156000, 173300, 141510,  0xd8 },
     /*95   MCS9 */ { VHT_INVALID_BCC_RATE, WHAL_MOD_IEEE80211_T_VHT_20,                   173000, 192000, 157300,  0xd9 },


#endif
 /* VHT MCS0-9 NSS 3 20 MHz */

#if (NUM_SPATIAL_STREAMS > 2)

      /*96  19.5 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                     19500,   21700,  17500,  0xe0 },
      /*97   39 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                      39000,   43300,  35200,  0xe1 },
      /*98 58.5 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                      58500,   65000,  52800,  0xe2 },
      /*99   78 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                      78000,   86700,  70610,  0xe3 },
      /*100   117 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    117000,  130000, 105920,  0xe4 },
      /*101   156 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    156000,  173300, 141220,  0xe5 },
      /*102 175.5 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    175500,  195000, 158880,  0xe6 },
      /*103   195 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    195000,  216700, 176870,  0xe7 },
      /*104  234 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    234000,  260000, 212180,  0xe8 },
      /*105  260 */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                        260000,  288900, 235830,  0xe9 },

#endif
 /* VHT MCS0-9 NSS 4 20 MHz */
#if (NUM_SPATIAL_STREAMS > 3)

      /*106  26 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                     26000,   28900,  23210,  0xf0 },
      /*107   52 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    52000,  57800, 46750,  0xf1 },
      /*108 78 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    78000,  86700, 70400,  0xf2 },
      /*109   104 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    104000,  115600, 94270,  0xf3 },
      /*110   156 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    156000,  173300, 141220,  0xf4 },
      /*111   208 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    208000,  231100, 188530,  0xf5 },
      /*112 234Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    234000,  260000, 212180,  0xf6},
      /*113   260 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    260000,  288900, 235830,  0xf7 },
      /*114   312 Mb */ { VHT20_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_20,                    312000,  346700, 282800,  0xf8 },
      /*115   312 Mb */ { VHT_INVALID_BCC_RATE, WHAL_MOD_IEEE80211_T_VHT_20,                    344000,  378400, 314330,  0xf9 },

#endif
 /* VHT MCS0-9 NSS 1 40 MHz */

     /*116   */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                      13500,   15000,  12060,  0xc0 },
      /*117  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                      27000,   30000,  24260,  0xc1 },
      /*118  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                      40500,   45000,  36590,  0xc2 },
      /*119  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                      54000,   60000,  48760,  0xc3 },
      /*120  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                      81000,   90000,  73400,  0xc4 },
      /*121  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                     108000,  120000,  97970,  0xc5 },
      /*122   */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    121500,  135000, 110550,  0xc6 },
      /*123  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                     135000,  150000, 122590,  0xc7 },
      /*124  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                     162000,  180000, 147250,  0xc8 },
     /*125 9 */ { VHT40_MODE_VALID_MASK, WHAL_MOD_IEEE80211_T_VHT_40,                    180000,  200000, 163620,  0xc9 },

 /* VHT MCS0-9 NSS 2 40 MHz */

#if (NUM_SPATIAL_STREAMS > 1)

     /*126 */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    27000,  30000,  24090,  0xd0 },
     /*127 */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    54000,  60000,  48550,  0xd1 },
     /*128 */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    81000,  90000,  73100,  0xd2 },
     /*129 */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                   108000, 120000,  97870,  0xd3 },
     /*130 */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                   162000, 180000, 147110,  0xd4 },
     /*131 */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                   216000, 240000, 196340,  0xd5 },
     /*132 */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                   243000, 270000, 220880,  0xd6 },
     /*133 */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                   270000, 300000, 245420,  0xd7 },
     /*134 */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                   324000, 360000, 294510,  0xd8 },
     /*135 */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                   360000, 400000, 326910,  0xd9 },

#endif
 /* VHT MCS0-9 NSS 3 40 MHz */

#if (NUM_SPATIAL_STREAMS > 2)

      /*136  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                     40500,   45000,  36250,  0xe0  },
      /*137  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                     81000,   90000,  72940,  0xe1 },
      /*138  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    121500,  135000, 109910,  0xe2 },
      /*139  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    162000,  180000, 146820,  0xe3 },
      /*140  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    243000,  270000, 220460,  0xe4 },
      /*141  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    324000,  360000, 293950,  0xe5 },
      /*142  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    364500,  405000, 330680,  0xe6 },
      /*143  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    405000,  450000, 367070,  0xe7 },
      /*144  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    486000,  540000, 440910,  0xe8 },
      /*145  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    540000,  600000, 489900,  0xe9 },

#endif

 /* VHT MCS0-9 NSS 4 40 MHz */
#if (NUM_SPATIAL_STREAMS > 3)

      /*146  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                     54000,   60000,  48120,  0xf0 },
      /*147   */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    108000,  120000, 97380,  0xf1 },
      /*148  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    162000,  180000, 146390,  0xf2 },
      /*149   */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    216000,  240000, 195960,  0xf3 },
      /*150   */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    324000,  360000, 293950,  0xf4 },
      /*151   */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    432000,  480000, 391920,  0xf5 },
      /*152  */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    486000,  540000, 440910,  0xf6 },
      /*153   */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    540000,  600000, 489900,  0xf7 },
      /*154   */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    648000,  720000, 585370,  0xf8 },
      /*155   MCS9 */ { VHT40_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_40,                    720000,  800000, 646270, 0xf9 },

#endif

 /* VHT MCS0-9 NSS 1 80 MHz */

      /*156 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    29300,   32500,   26350,  0xc0 },
      /*157 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    58500,   65000,   52960,  0xc1 },
      /*158 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    87800,   97500,   79440,  0xc2 },
      /*159 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                   117000,  130000,  106240,  0xc3 },
      /*160 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                   175500,  195000,  159720,  0xc4 },
      /*161 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                   234000,  260000,  212790,  0xc5 },
      /*162 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                   263300,  292500,  239480,  0xc6 },
      /*163 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                   292500,  325000,  265910,  0xc7 },
      /*164 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                   351000,  390000,  319440,  0xc8 },
      /*165 */ {VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    390000,  433300,  354760,  0xc9 },

 /* VHT MCS0-9 NSS 2 80 MHz */
#if (NUM_SPATIAL_STREAMS > 1)

      /*166 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                     58500,   65000,  52590,  0xd0 },
      /*167 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    117000,  130000, 105810,  0xd1 },
      /*168 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    175500,  195000, 159120,  0xd2 },
      /*169 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    234000,  260000, 212590,  0xd3 },
      /*170 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    351000,  390000, 319140,  0xd4 },
      /*171 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    468000,  520000, 425180,  0xd5 },
      /*172 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    526500,  585000, 478710,  0xd6 },
      /*173 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    585000,  650000, 531630,  0xd7 },
      /*174 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    702000,  780000, 632530,  0xd8 },
      /*175 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    780000,  866700, 698160,  0xd9 },

#endif
 /* VHT MCS0-9 NSS 3 80 MHz */
#if (NUM_SPATIAL_STREAMS > 2)

      /*176 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                      87800,    97500,   78730,  0xe0 },
      /*177 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                     175500,   195000,  158400,  0xe1 },
      /*178 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                     263300,   292500,  238100,  0xe2 },
      /*179 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                     351000,   390000,  318000,  0xe3 },
      /*180 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                     526500,   585000,  477580,  0xe4 },
      /*181 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                     702000,   780000,  631120,  0xe5 },
      /*182 */ { VHT_INVALID_BCC_RATE,WHAL_MOD_IEEE80211_T_VHT_80,                      789800,   877500,  705130,  0xe6 },
      /*183 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                     877500,   975000,  777910,  0xe7 },
      /*184 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    1053000,  1170000,  919660,  0xe8 },
      /*185 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    1170000,  1300000,  1011860,  0xe9 },

#endif
 /* VHT MCS0-9 NSS 4 80 MHz */

#if (NUM_SPATIAL_STREAMS > 3)

      /*186  */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                     117000,   130000,  104970,  0xf0 },
      /*187   */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    234000,  260000, 211570,  0xf1 },
      /*188 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    351000,  390000, 317380,  0xf2 },
      /*189   */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    468000,  520000, 424360,  0xf3 },
      /*190   */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    702000,  780000, 631120,  0xf4 },
      /*191   */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    936000,  1040000, 825630,  0xf5 },
      /*192  */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    1053000,  1170000, 919660,  0xf6 },
      /*193   */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    1170000,  1300000, 1011860,  0xf7 },
      /*194   */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    1404000,  1560000, 1190940,  0xf8},
      /*195   MCS9 */ { VHT80_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_80,                    1560000,  1733300, 1306550,  0xf9 },
#endif

#if CONFIG_160MHZ_SUPPORT != 0
/* Todo Extend support for 80+80/160 MHz in future */
	 /* VHT MCS0-9 NSS 1 160 MHz */

	  /*196 */ { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                  	58500,	 65000,  53500,      0xc0     },
	  /*197 */ { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                    117000,	130000, 105290,  0xc1 },
	  /*198 */ { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                    175500,	195000, 157560,  0xc2 },
	  /*199 */ { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                    234000,	260000, 211200,  0xc3 	 },
	  /*200 */ { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                    351000,	390000, 317470,  0xc4 	 },
	  /*201 */ { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                    468000,	520000, 419810,  0xc5 	 },
	  /*202 */ { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                    526500,	585000, 468920,  0xc6 	 },
	  /*203 */ { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                    585000,	650000, 517180,  0xc7 	 },
	  /*204 */ { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                    702000,	780000, 612110,  0xc8 	 },
	  /*205 */ { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                    780000,	866700, 673370,  0xc9 	 },
//#endif

	 /* VHT MCS0-9 NSS 2 160 MHz */
#if (MAX_SPATIAL_STREAMS_SUPPORTED_AT_160MHZ > 1)

	  /*206  */  { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                  	 117000,   130000,	100000,  0xd0 },
	  /*207   */ { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                    234000,  260000, 174000,      0xd1    },
	  /*208 */  { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                    351000,	390000, 233000,      0xd2    },
	  /*209   */ { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                    468000,  520000, 281000,      0xd3    },
	  /*210   */ { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                    702000,  780000, 353000,      0xd4    },
	  /*211   */ { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                    936000,  1040000, 406000,	 0xd5  },
	  /*212  */  { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                  	1053000,  1170000, 426700,	 0xd6  },
	  /*213   */ { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                    1170000,  1300000, 426700,    0xd7  },
	  /*214   */ { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                    1404000,  1560000, 476300,    0xd8  },
	  /*215   */ { VHT160_MODE_VALID_MASK,WHAL_MOD_IEEE80211_T_VHT_160,                    1560000,	1733300, 476300,     0xd9    },

#endif
#endif

#endif /* (NUM_SPATIAL_STREAMS > 4) || SUPPORT_11AX */

    },
};

/*
 * func to return rate modulation given code spatial width
 */
OL_WLAN_MODULATION_TYPE ol_getmodulation(
        uint16_t pream_type,
        uint8_t width)
{
    static const OL_WLAN_MODULATION_TYPE _vht_bw_mod[] = {
        WHAL_MOD_IEEE80211_T_VHT_20,
        WHAL_MOD_IEEE80211_T_VHT_40,
        WHAL_MOD_IEEE80211_T_VHT_80,
#ifdef WHAL_160MHZ_SUPPORT
        WHAL_MOD_IEEE80211_T_VHT_160
#endif
    };

#if SUPPORT_11AX
    static const OL_WLAN_MODULATION_TYPE _he_bw_mod[] = {
        WHAL_MOD_IEEE80211_T_HE_20,
        WHAL_MOD_IEEE80211_T_HE_40,
        WHAL_MOD_IEEE80211_T_HE_80,
#ifdef WHAL_160MHZ_SUPPORT
        WHAL_MOD_IEEE80211_T_HE_160
#endif
    };
#endif /* if SUPPORT_11AX */

    OL_WLAN_MODULATION_TYPE modulation;

    A_ASSERT(width < BW_CNT);

    switch (pream_type) {
    case WIFI_HW_RATECODE_PREAM_HT:

        if (width) {
            modulation = WHAL_MOD_IEEE80211_T_HT_40;
        } else {
            modulation = WHAL_MOD_IEEE80211_T_HT_20;
        }
        break;

    case WIFI_HW_RATECODE_PREAM_CCK:
        modulation = WHAL_MOD_IEEE80211_T_CCK;
        break;

    case WIFI_HW_RATECODE_PREAM_VHT:
        modulation = _vht_bw_mod[width];
        break;

#if SUPPORT_11AX
    case WIFI_HW_RATECODE_PREAM_HE:
        modulation = _he_bw_mod[width];
        break;
#endif

    default:
        modulation = WHAL_MOD_IEEE80211_T_OFDM;
        break;
    }

    return modulation;
}

static const uint16_t _rc_idx[WHAL_MOD_IEEE80211_T_MAX_PHY] = {
	CCK_RATE_TABLE_INDEX,
	OFDM_RATE_TABLE_INDEX,
	HT_20_RATE_TABLE_INDEX,
	HT_40_RATE_TABLE_INDEX,
	VHT_20_RATE_TABLE_INDEX,
	VHT_40_RATE_TABLE_INDEX,
	VHT_80_RATE_TABLE_INDEX,
#if CONFIG_160MHZ_SUPPORT != 0
	VHT_160_RATE_TABLE_INDEX,
#endif

#ifdef SUPPORT_11AX
	HE_20_RATE_TABLE_INDEX,
	HE_40_RATE_TABLE_INDEX,
	HE_80_RATE_TABLE_INDEX,
#if CONFIG_160MHZ_SUPPORT != 0
	HE_160_RATE_TABLE_INDEX,
#endif
#endif // SUPPORT_11AX
};


/*
 * return rate table index given mcs/nss/preamble/bw parameters.
 */
uint32_t
ol_if_getrateindex(uint16_t mcs, uint8_t nss, uint8_t preamble, uint8_t bw)
{
    uint32_t ratekbps = 0, res = RT_INVALID_INDEX; /* represents failure */
    uint16_t rc;
    OL_WLAN_MODULATION_TYPE mod;

    mod = ol_getmodulation(preamble, bw);
    rc = mcs;
    /* get the base of corresponding rate table  entry */
    res = _rc_idx[mod];

    switch (preamble) {
#ifdef SUPPORT_11AX
    case WIFI_HW_RATECODE_PREAM_HE:
        res += rc + nss * NUM_HE_MCS;
        break;
#endif

    case WIFI_HW_RATECODE_PREAM_VHT:
        res += rc + nss * NUM_VHT_MCS;
        break;

    case WIFI_HW_RATECODE_PREAM_HT:
        res += rc + nss * NUM_HT_MCS;
        break;

    case WIFI_HW_RATECODE_PREAM_CCK:
        rc  &= ~HW_RATECODE_CCK_SHORT_PREAM_MASK;
        res += rc;
        break;

    case WIFI_HW_RATECODE_PREAM_OFDM:
        res += rc;
        break;

    default:
        break;
    }

    if (res >= RATE_TABLE_SIZE)
        return ratekbps;

    ratekbps = whal_11abgnRateTable.info[res].userRateKbps;

    return ratekbps;
}

#endif // ATH_PERF_PWR_OFFLOAD
