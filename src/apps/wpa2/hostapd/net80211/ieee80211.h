/*-
 * Copyright (c) 2001 Atsushi Onoe
 * Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD: src/sys/net80211/ieee80211.h,v 1.8 2004/12/31 22:44:26 sam Exp $
 */
#ifndef _NET80211_IEEE80211_H_
#define _NET80211_IEEE80211_H_

#ifndef __packed
#define __packed   __attribute__((__packed__))
#endif

/*
 * 802.11 protocol definitions.
 */

#define	IEEE80211_ADDR_LEN	6		/* size of 802.11 address */
/* is 802.11 address multicast/broadcast? */
#define	IEEE80211_IS_MULTICAST(_a)	(*(_a) & 0x01)

/* IEEE 802.11 PLCP header */
struct ieee80211_plcp_hdr {
	u_int16_t	i_sfd;
	u_int8_t	i_signal;
	u_int8_t	i_service;
	u_int16_t	i_length;
	u_int16_t	i_crc;
} __packed;


typedef struct cntlist{
	char *pMac;
	int ath_no;
	int status;//0-authing;1-associating
	unsigned long jiffies;
	struct cntlist *pNext;
}CNTLIST;

#define TIMEOUT_JIFFIES		10*HZ

#define IEEE80211_PLCP_SFD      0xF3A0 
#define IEEE80211_PLCP_SERVICE  0x00

/*
 * generic definitions for IEEE 802.11 frames
 */
struct ieee80211_frame {
	u_int8_t	i_fc[2];
	u_int8_t	i_dur[2];
	u_int8_t	i_addr1[IEEE80211_ADDR_LEN];
	u_int8_t	i_addr2[IEEE80211_ADDR_LEN];
	u_int8_t	i_addr3[IEEE80211_ADDR_LEN];
	u_int8_t	i_seq[2];
	/* possibly followed by addr4[IEEE80211_ADDR_LEN]; */
	/* see below */
} __packed;

struct ieee80211_qosframe {
	u_int8_t	i_fc[2];
	u_int8_t	i_dur[2];
	u_int8_t	i_addr1[IEEE80211_ADDR_LEN];
	u_int8_t	i_addr2[IEEE80211_ADDR_LEN];
	u_int8_t	i_addr3[IEEE80211_ADDR_LEN];
	u_int8_t	i_seq[2];
	u_int8_t	i_qos[2];
	/* possibly followed by addr4[IEEE80211_ADDR_LEN]; */
	/* see below */
} __packed;

struct ieee80211_qoscntl {
	u_int8_t	i_qos[2];
};

struct ieee80211_frame_addr4 {
	u_int8_t	i_fc[2];
	u_int8_t	i_dur[2];
	u_int8_t	i_addr1[IEEE80211_ADDR_LEN];
	u_int8_t	i_addr2[IEEE80211_ADDR_LEN];
	u_int8_t	i_addr3[IEEE80211_ADDR_LEN];
	u_int8_t	i_seq[2];
	u_int8_t	i_addr4[IEEE80211_ADDR_LEN];
} __packed;


struct ieee80211_qosframe_addr4 {
	u_int8_t	i_fc[2];
	u_int8_t	i_dur[2];
	u_int8_t	i_addr1[IEEE80211_ADDR_LEN];
	u_int8_t	i_addr2[IEEE80211_ADDR_LEN];
	u_int8_t	i_addr3[IEEE80211_ADDR_LEN];
	u_int8_t	i_seq[2];
	u_int8_t	i_addr4[IEEE80211_ADDR_LEN];
	u_int8_t	i_qos[2];
} __packed;

struct ieee80211_ctlframe_addr2 {
	u_int8_t	i_fc[2];
	u_int8_t	i_aidordur[2]; /* AID or duration */
	u_int8_t	i_addr1[IEEE80211_ADDR_LEN];
	u_int8_t	i_addr2[IEEE80211_ADDR_LEN];
} __packed;

#define	IEEE80211_FC0_VERSION_MASK			0x03
#define	IEEE80211_FC0_VERSION_SHIFT			0
#define	IEEE80211_FC0_VERSION_0				0x00
#define	IEEE80211_FC0_TYPE_MASK				0x0c
#define	IEEE80211_FC0_TYPE_SHIFT			2
#define	IEEE80211_FC0_TYPE_MGT				0x00
#define	IEEE80211_FC0_TYPE_CTL				0x04
#define	IEEE80211_FC0_TYPE_DATA				0x08

#define	IEEE80211_FC0_SUBTYPE_MASK			0xf0
#define	IEEE80211_FC0_SUBTYPE_SHIFT			4
/* for TYPE_MGT */
#define	IEEE80211_FC0_SUBTYPE_ASSOC_REQ		0x00
#define	IEEE80211_FC0_SUBTYPE_ASSOC_RESP	0x10
#define	IEEE80211_FC0_SUBTYPE_REASSOC_REQ	0x20
#define	IEEE80211_FC0_SUBTYPE_REASSOC_RESP	0x30
#define	IEEE80211_FC0_SUBTYPE_PROBE_REQ		0x40
#define	IEEE80211_FC0_SUBTYPE_PROBE_RESP	0x50
#define	IEEE80211_FC0_SUBTYPE_BEACON		0x80
#define	IEEE80211_FC0_SUBTYPE_ATIM			0x90
#define	IEEE80211_FC0_SUBTYPE_DISASSOC		0xa0
#define	IEEE80211_FC0_SUBTYPE_AUTH			0xb0
#define	IEEE80211_FC0_SUBTYPE_DEAUTH		0xc0
#define IEEE80211_FC0_SUBTYPE_ACTION		0xd0
/* for TYPE_CTL */
#define	IEEE80211_FC0_SUBTYPE_BAR   		0x80
#define	IEEE80211_FC0_SUBTYPE_PS_POLL		0xa0
#define	IEEE80211_FC0_SUBTYPE_RTS			0xb0
#define	IEEE80211_FC0_SUBTYPE_CTS			0xc0
#define	IEEE80211_FC0_SUBTYPE_ACK			0xd0
#define	IEEE80211_FC0_SUBTYPE_CF_END		0xe0
#define	IEEE80211_FC0_SUBTYPE_CF_END_ACK	0xf0
/* for TYPE_DATA (bit combination) */
#define	IEEE80211_FC0_SUBTYPE_DATA		0x00
#define	IEEE80211_FC0_SUBTYPE_CF_ACK		0x10
#define	IEEE80211_FC0_SUBTYPE_CF_POLL		0x20
#define	IEEE80211_FC0_SUBTYPE_CF_ACPL		0x30
#define	IEEE80211_FC0_SUBTYPE_NODATA		0x40
#define	IEEE80211_FC0_SUBTYPE_CFACK		0x50
#define	IEEE80211_FC0_SUBTYPE_CFPOLL		0x60
#define	IEEE80211_FC0_SUBTYPE_CF_ACK_CF_ACK	0x70
#define	IEEE80211_FC0_SUBTYPE_QOS		0x80
#define	IEEE80211_FC0_SUBTYPE_QOS_NULL		0xc0

#define	IEEE80211_FC1_DIR_MASK			0x03
#define	IEEE80211_FC1_DIR_NODS			0x00	/* STA->STA */
#define	IEEE80211_FC1_DIR_TODS			0x01	/* STA->AP  */
#define	IEEE80211_FC1_DIR_FROMDS		0x02	/* AP ->STA */
#define	IEEE80211_FC1_DIR_DSTODS		0x03	/* AP ->AP  */

#define	IEEE80211_FC1_MORE_FRAG			0x04
#define	IEEE80211_FC1_RETRY			0x08
#define	IEEE80211_FC1_PWR_MGT			0x10
#define	IEEE80211_FC1_MORE_DATA			0x20
#define	IEEE80211_FC1_WEP			0x40
#define	IEEE80211_FC1_ORDER			0x80

#define	IEEE80211_SEQ_FRAG_MASK			0x000f
#define	IEEE80211_SEQ_FRAG_SHIFT		0
#define	IEEE80211_SEQ_SEQ_MASK			0xfff0
#define	IEEE80211_SEQ_SEQ_SHIFT			4
#define IEEE80211_SEQ_MAX               4096

#define	IEEE80211_SEQ_LEQ(a,b)	((int)((a)-(b)) <= 0)

#define	IEEE80211_NWID_LEN			32

#define	IEEE80211_QOS_TXOP			0x00ff

#define IEEE80211_QOS_AMSDU                     0x80
#define IEEE80211_QOS_AMSDU_S                   7
#define	IEEE80211_QOS_ACKPOLICY			0x60
#define	IEEE80211_QOS_ACKPOLICY_S		5
#define	IEEE80211_QOS_EOSP			0x10
#define	IEEE80211_QOS_EOSP_S			4
#define	IEEE80211_QOS_TID			0x0f

#define IEEE80211_IS_BEACON(_frame)    ((((_frame)->i_fc[0] & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_MGT) && \
                                        (((_frame)->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK) == IEEE80211_FC0_SUBTYPE_BEACON))
#define IEEE80211_IS_DATA(_frame)      (((_frame)->i_fc[0] & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_DATA)

#define IEEE80211_IS_MFP_FRAME(_frame) ((((_frame)->i_fc[0] & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_MGT) && \
                                        ((_frame)->i_fc[1] & IEEE80211_FC1_WEP) && \
                                        ((((_frame)->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK) == IEEE80211_FC0_SUBTYPE_DEAUTH) || \
                                         (((_frame)->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK) == IEEE80211_FC0_SUBTYPE_DISASSOC) || \
                                         (((_frame)->i_fc[0] & IEEE80211_FC0_SUBTYPE_MASK) == IEEE80211_FC0_SUBTYPE_ACTION)))

#define IEEE80211_RX_MCS_SINGLE_STREAM_BYTE_OFFSET 0
#define IEEE80211_RX_MCS_DUAL_STREAM_BYTE_OFFSET 1
#define IEEE80211_RX_MCS_ALL_NSTREAM_RATES 0xff
#define IEEE80211_TX_MCS_OFFSET 12

#define IEEE80211_TX_MCS_SET_DEFINED 0x80
#define IEEE80211_TX_RX_MCS_SET_NOT_EQUAL 0x40
#define IEEE80211_TX_1_SPATIAL_STREAMS 0x0
#define IEEE80211_TX_2_SPATIAL_STREAMS 0x10
#define IEEE80211_TX_3_SPATIAL_STREAMS 0x20
#define IEEE80211_TX_4_SPATIAL_STREAMS 0x30

#define IEEE80211_TX_MCS_SET 0xf8

#define IEEE80211_CHAIN_MASK_TO_STREAMS(_chainmask) ((_chainmask == 1) ? 1 : ((_chainmask == 2) ? 1 : ((_chainmask == 4) ? 1 : 2)))

/* 
 * Country information element.
 */
#define IEEE80211_COUNTRY_MAX_TRIPLETS (83)
struct ieee80211_ie_country {
	u_int8_t	country_id;
	u_int8_t	country_len;
	u_int8_t	country_str[3];
	u_int8_t	country_triplet[IEEE80211_COUNTRY_MAX_TRIPLETS*3];
} __packed;

/* does frame have QoS sequence control data */
#define	IEEE80211_QOS_HAS_SEQ(wh) \
	(((wh)->i_fc[0] & \
	  (IEEE80211_FC0_TYPE_MASK | IEEE80211_FC0_SUBTYPE_QOS)) == \
	  (IEEE80211_FC0_TYPE_DATA | IEEE80211_FC0_SUBTYPE_QOS))

#define WME_QOSINFO_COUNT	0x0f  /* Mask for Param Set Count field */
/*
 * WME/802.11e information element.
 */
struct ieee80211_ie_wme {
	u_int8_t	wme_id;		/* IEEE80211_ELEMID_VENDOR */
	u_int8_t	wme_len;	/* length in bytes */
	u_int8_t	wme_oui[3];	/* 0x00, 0x50, 0xf2 */
	u_int8_t	wme_type;	/* OUI type */
	u_int8_t	wme_subtype;	/* OUI subtype */
	u_int8_t	wme_version;	/* spec revision */
	u_int8_t	wme_info;	/* QoS info */
} __packed;

/*
 * WME/802.11e Tspec Element
 */
struct ieee80211_wme_tspec {
	u_int8_t	ts_id;
	u_int8_t	ts_len;
	u_int8_t	ts_oui[3];
	u_int8_t	ts_oui_type;
	u_int8_t	ts_oui_subtype;
	u_int8_t	ts_version;
	u_int8_t	ts_tsinfo[3];
	u_int8_t	ts_nom_msdu[2];
	u_int8_t	ts_max_msdu[2];
	u_int8_t	ts_min_svc[4];
	u_int8_t	ts_max_svc[4];
	u_int8_t	ts_inactv_intv[4];
	u_int8_t	ts_susp_intv[4];
	u_int8_t	ts_start_svc[4];
	u_int8_t	ts_min_rate[4];
	u_int8_t	ts_mean_rate[4];
	u_int8_t	ts_max_burst[4];
	u_int8_t	ts_min_phy[4];
	u_int8_t	ts_peak_rate[4];
	u_int8_t	ts_delay[4];
	u_int8_t	ts_surplus[2];
	u_int8_t	ts_medium_time[2];
} __packed;

/*
 * WME AC parameter field
 */

struct ieee80211_wme_acparams {
	u_int8_t	acp_aci_aifsn;
	u_int8_t	acp_logcwminmax;
	u_int16_t	acp_txop;
} __packed;

#define IEEE80211_WME_PARAM_LEN 24
#define WME_NUM_AC		4	/* 4 AC categories */

#define WME_PARAM_ACI		0x60	/* Mask for ACI field */
#define WME_PARAM_ACI_S		5	/* Shift for ACI field */
#define WME_PARAM_ACM		0x10	/* Mask for ACM bit */
#define WME_PARAM_ACM_S		4	/* Shift for ACM bit */
#define WME_PARAM_AIFSN		0x0f	/* Mask for aifsn field */
#define WME_PARAM_AIFSN_S	0	/* Shift for aifsn field */
#define WME_PARAM_LOGCWMIN	0x0f	/* Mask for CwMin field (in log) */
#define WME_PARAM_LOGCWMIN_S	0	/* Shift for CwMin field */
#define WME_PARAM_LOGCWMAX	0xf0	/* Mask for CwMax field (in log) */
#define WME_PARAM_LOGCWMAX_S	4	/* Shift for CwMax field */

#define WME_AC_TO_TID(_ac) (       \
	((_ac) == WME_AC_VO) ? 6 : \
	((_ac) == WME_AC_VI) ? 5 : \
	((_ac) == WME_AC_BK) ? 1 : \
	0)

#define TID_TO_WME_AC(_tid) (      \
    (((_tid) == 0) || ((_tid) == 3)) ? WME_AC_BE : \
    (((_tid) == 1) || ((_tid) == 2)) ? WME_AC_BK : \
    (((_tid) == 4) || ((_tid) == 5)) ? WME_AC_VI : \
	WME_AC_VO)

/*
 * WME Parameter Element
 */

struct ieee80211_wme_param {
	u_int8_t	param_id;
	u_int8_t	param_len;
	u_int8_t	param_oui[3];
	u_int8_t	param_oui_type;
	u_int8_t	param_oui_sybtype;
	u_int8_t	param_version;
	u_int8_t	param_qosInfo;
	u_int8_t	param_reserved;
	struct ieee80211_wme_acparams	params_acParams[WME_NUM_AC];
} __packed;

/*
 * WME U-APSD qos info field defines
 */
#define WME_CAPINFO_UAPSD_EN			0x00000080
#define WME_CAPINFO_UAPSD_VO			0x00000001
#define WME_CAPINFO_UAPSD_VI			0x00000002
#define WME_CAPINFO_UAPSD_BK			0x00000004
#define WME_CAPINFO_UAPSD_BE			0x00000008
#define WME_CAPINFO_UAPSD_ACFLAGS_SHIFT		0
#define WME_CAPINFO_UAPSD_ACFLAGS_MASK		0xF
#define WME_CAPINFO_UAPSD_MAXSP_SHIFT		5
#define WME_CAPINFO_UAPSD_MAXSP_MASK		0x3
#define WME_CAPINFO_IE_OFFSET			8
#define WME_UAPSD_MAXSP(_qosinfo) (((_qosinfo) >> WME_CAPINFO_UAPSD_MAXSP_SHIFT) & WME_CAPINFO_UAPSD_MAXSP_MASK)
#define WME_UAPSD_AC_ENABLED(_ac, _qosinfo) ( (1<<(3 - (_ac))) &   \
		(((_qosinfo) >> WME_CAPINFO_UAPSD_ACFLAGS_SHIFT) & WME_CAPINFO_UAPSD_ACFLAGS_MASK) )

/*
 * Atheros Advanced Capability information element.
 */
struct ieee80211_ie_athAdvCap {
	u_int8_t	athAdvCap_id;		/* IEEE80211_ELEMID_VENDOR */
	u_int8_t	athAdvCap_len;		/* length in bytes */
	u_int8_t	athAdvCap_oui[3];	/* 0x00, 0x03, 0x7f */
	u_int8_t	athAdvCap_type;		/* OUI type */
	u_int8_t	athAdvCap_subtype;	/* OUI subtype */
	u_int8_t	athAdvCap_version;	/* spec revision */
	u_int8_t	athAdvCap_capability;	/* Capability info */
	u_int16_t	athAdvCap_defKeyIndex;
} __packed;

/*
 * Atheros Extended Capability information element.
 */
struct ieee80211_ie_ath_extcap {
        u_int8_t        ath_extcap_id;  /* IEEE80211_ELEMID_VENDOR */
        u_int8_t        ath_extcap_len; /* length in bytes */
        u_int8_t        ath_extcap_oui[3];      /* 0x00, 0x03, 0x7f */
        u_int8_t        ath_extcap_type;        /* OUI type */
        u_int8_t        ath_extcap_subtype;     /* OUI subtype */
        u_int8_t        ath_extcap_version;     /* spec revision */

        u_int16_t       ath_extcap_extcap;              /* extended capabilities */
        u_int8_t        ath_extcap_weptkipaggr_rxdelim; /* num delimiters for receiving WEP/TKIP aggregates */
        u_int8_t        ath_extcap_reserved;            /* reserved */
} __packed;

/*
 * Atheros XR information element.
 */
struct ieee80211_xr_param {
	u_int8_t	param_id;
	u_int8_t	param_len;
	u_int8_t	param_oui[3];
	u_int8_t	param_oui_type;
	u_int8_t	param_oui_sybtype;
	u_int8_t	param_version;
	u_int8_t	param_Info;
	u_int8_t	param_base_bssid[IEEE80211_ADDR_LEN];
	u_int8_t	param_xr_bssid[IEEE80211_ADDR_LEN];
	u_int16_t	param_xr_beacon_interval;
	u_int8_t	param_base_ath_capability;
	u_int8_t	param_xr_ath_capability;
} __packed;

/* Atheros capabilities */
#define IEEE80211_ATHC_TURBOP	0x0001		/* Turbo Prime */
#define IEEE80211_ATHC_COMP	0x0002		/* Compression */
#define IEEE80211_ATHC_FF	0x0004		/* Fast Frames */
#define IEEE80211_ATHC_XR	0x0008		/* Xtended Range support */
#define IEEE80211_ATHC_AR	0x0010		/* Advanced Radar support */
#define IEEE80211_ATHC_BURST	0x0020		/* Bursting - not negotiated */
#define IEEE80211_ATHC_WME	0x0040		/* CWMin tuning */
#define IEEE80211_ATHC_BOOST	0x0080		/* Boost */

/* Atheros extended capabilities */
/* OWL device capable of WDS workaround */
#define IEEE80211_ATHEC_OWLWDSWAR        0x0001
#define IEEE80211_ATHEC_WEPTKIPAGGR      0x0002
#define IEEE80211_ATHEC_SINGLEAGGRSAFE   0X0004


/*
 * Management Action Frames 
 */

/* generic frame format */
struct ieee80211_action {
	u_int8_t	ia_category;
	u_int8_t	ia_action;
} __packed;

/* categories */
#define IEEE80211_ACTION_CAT_QOS		0	/* qos */
#define IEEE80211_ACTION_CAT_BA                 3   	/* BA */
#define IEEE80211_ACTION_CAT_COEX               4       /* Coex */
#define IEEE80211_ACTION_CAT_HT			7	/* HT per IEEE802.11n-D1.06 */

/* HT actions */
#define IEEE80211_ACTION_HT_TXCHWIDTH		0	/* recommended transmission channel width */
#define IEEE80211_ACTION_HT_SMPOWERSAVE		1	/* Spatial Multiplexing (SM) Power Save */


/* HT - recommended transmission channel width */
struct ieee80211_action_ht_txchwidth {
	struct ieee80211_action 	at_header;
	u_int8_t			at_chwidth;	
} __packed;

#define IEEE80211_A_HT_TXCHWIDTH_20	0
#define IEEE80211_A_HT_TXCHWIDTH_2040	1


/* HT - Spatial Multiplexing (SM) Power Save */
struct ieee80211_action_ht_smpowersave {
	struct ieee80211_action		as_header;
	u_int8_t			as_control;
} __packed;

/* values defined for 'as_control' field per 802.11n-D1.06 */
#define IEEE80211_A_HT_SMPOWERSAVE_DISABLED     0x00   /* SM Power Save Disabled, SM packets ok  */
#define IEEE80211_A_HT_SMPOWERSAVE_ENABLED      0x01   /* SM Power Save Enabled bit  */
#define IEEE80211_A_HT_SMPOWERSAVE_MODE         0x02   /* SM Power Save Mode bit */
#define IEEE80211_A_HT_SMPOWERSAVE_RESERVED     0xFC   /* SM Power Save Reserved bits */

/* values defined for SM Power Save Mode bit */
#define IEEE80211_A_HT_SMPOWERSAVE_STATIC       0x00   /* Static, SM packets not ok */
#define IEEE80211_A_HT_SMPOWERSAVE_DYNAMIC      0x02   /* Dynamic, SM packets ok if preceded by RTS */

/* BA actions */
#define IEEE80211_ACTION_BA_ADDBA_REQUEST       0   /* ADDBA request */
#define IEEE80211_ACTION_BA_ADDBA_RESPONSE      1   /* ADDBA response */
#define IEEE80211_ACTION_BA_DELBA	        2   /* DELBA */

struct ieee80211_ba_parameterset {
#if _BYTE_ORDER == _BIG_ENDIAN
        u_int16_t	buffersize      : 10,   /* B6-15  buffer size */
			tid             : 4,    /* B2-5   TID */
			bapolicy        : 1,    /* B1   block ack policy */
			amsdusupported  : 1;    /* B0   amsdu supported */
#else
        u_int16_t   amsdusupported  : 1,    /* B0   amsdu supported */
			bapolicy        : 1,    /* B1   block ack policy */
			tid             : 4,    /* B2-5   TID */
			buffersize      : 10;   /* B6-15  buffer size */
#endif
} __packed;

#define  IEEE80211_BA_POLICY_DELAYED      0
#define  IEEE80211_BA_POLICY_IMMEDIATE    1
#define  IEEE80211_BA_AMSDU_SUPPORTED     1

struct ieee80211_ba_seqctrl {
#if _BYTE_ORDER == _BIG_ENDIAN
        u_int16_t    	startseqnum     : 12,    /* B4-15  starting sequence number */
			fragnum         : 4;     /* B0-3  fragment number */
#else
        u_int16_t    	fragnum         : 4,     /* B0-3  fragment number */
			startseqnum     : 12;    /* B4-15  starting sequence number */
#endif
} __packed;

struct ieee80211_delba_parameterset {
#if _BYTE_ORDER == _BIG_ENDIAN
        u_int16_t	tid             : 4,     /* B12-15  tid */
			initiator       : 1,     /* B11     initiator */
			reserved0       : 11;    /* B0-10   reserved */
#else
        u_int16_t    	reserved0       : 11,    /* B0-10   reserved */
			initiator       : 1,     /* B11     initiator */
			tid             : 4;     /* B12-15  tid */
#endif
} __packed;

/* BA - ADDBA request */
struct ieee80211_action_ba_addbarequest {
	struct ieee80211_action			rq_header;
        u_int8_t                		rq_dialogtoken;
        struct ieee80211_ba_parameterset        rq_baparamset; 
        u_int16_t                	        rq_batimeout;	/* in TUs */
        struct ieee80211_ba_seqctrl     	rq_basequencectrl;
} __packed;

/* BA - ADDBA response */
struct ieee80211_action_ba_addbaresponse {
	struct ieee80211_action			rs_header;
        u_int8_t                		rs_dialogtoken;
        u_int16_t                		rs_statuscode;
        struct ieee80211_ba_parameterset        rs_baparamset; 
        u_int16_t                		rs_batimeout;          /* in TUs */
} __packed;

/* BA - DELBA */
struct ieee80211_action_ba_delba {
	struct ieee80211_action			dl_header;
        struct ieee80211_delba_parameterset     dl_delbaparamset;
        u_int16_t		                dl_reasoncode;
} __packed;

/*
 * Control frames.
 */
struct ieee80211_frame_min {
	u_int8_t	i_fc[2];
	u_int8_t	i_dur[2];
	u_int8_t	i_addr1[IEEE80211_ADDR_LEN];
	u_int8_t	i_addr2[IEEE80211_ADDR_LEN];
	/* FCS */
} __packed;

/*
 * BAR frame format
 */
#define IEEE80211_BAR_CTL_TID_M     0xF000      /* tid mask             */
#define IEEE80211_BAR_CTL_TID_S         12      /* tid shift            */
#define IEEE80211_BAR_CTL_NOACK     0x0001      /* no-ack policy        */
#define IEEE80211_BAR_CTL_COMBA     0x0004      /* compressed block-ack */
struct ieee80211_frame_bar {
	u_int8_t	i_fc[2];
	u_int8_t	i_dur[2];
	u_int8_t	i_ra[IEEE80211_ADDR_LEN];
	u_int8_t	i_ta[IEEE80211_ADDR_LEN];
    u_int16_t   i_ctl;
    u_int16_t   i_seq;
	/* FCS */
} __packed;

struct ieee80211_frame_rts {
	u_int8_t	i_fc[2];
	u_int8_t	i_dur[2];
	u_int8_t	i_ra[IEEE80211_ADDR_LEN];
	u_int8_t	i_ta[IEEE80211_ADDR_LEN];
	/* FCS */
} __packed;

struct ieee80211_frame_cts {
	u_int8_t	i_fc[2];
	u_int8_t	i_dur[2];
	u_int8_t	i_ra[IEEE80211_ADDR_LEN];
	/* FCS */
} __packed;

struct ieee80211_frame_ack {
	u_int8_t	i_fc[2];
	u_int8_t	i_dur[2];
	u_int8_t	i_ra[IEEE80211_ADDR_LEN];
	/* FCS */
} __packed;

struct ieee80211_frame_pspoll {
	u_int8_t	i_fc[2];
	u_int8_t	i_aid[2];
	u_int8_t	i_bssid[IEEE80211_ADDR_LEN];
	u_int8_t	i_ta[IEEE80211_ADDR_LEN];
	/* FCS */
} __packed;

struct ieee80211_frame_cfend {		/* NB: also CF-End+CF-Ack */
	u_int8_t	i_fc[2];
	u_int8_t	i_dur[2];	/* should be zero */
	u_int8_t	i_ra[IEEE80211_ADDR_LEN];
	u_int8_t	i_bssid[IEEE80211_ADDR_LEN];
	/* FCS */
} __packed;

/*
 * BEACON management packets
 *
 *	octet timestamp[8]
 *	octet beacon interval[2]
 *	octet capability information[2]
 *	information element
 *		octet elemid
 *		octet length
 *		octet information[length]
 */

typedef u_int8_t *ieee80211_mgt_beacon_t;

#define	IEEE80211_BEACON_INTERVAL(beacon) \
	((beacon)[8] | ((beacon)[9] << 8))
#define	IEEE80211_BEACON_CAPABILITY(beacon) \
	((beacon)[10] | ((beacon)[11] << 8))

#define	IEEE80211_CAPINFO_ESS			0x0001
#define	IEEE80211_CAPINFO_IBSS			0x0002
#define	IEEE80211_CAPINFO_CF_POLLABLE		0x0004
#define	IEEE80211_CAPINFO_CF_POLLREQ		0x0008
#define	IEEE80211_CAPINFO_PRIVACY		0x0010
#define	IEEE80211_CAPINFO_SHORT_PREAMBLE	0x0020
#define	IEEE80211_CAPINFO_PBCC			0x0040
#define	IEEE80211_CAPINFO_CHNL_AGILITY		0x0080
/* bits 8-9 are reserved (8 now for specturm management) */
#define IEEE80211_CAPINFO_SPECTRUM_MGMT		0x0100
#define	IEEE80211_CAPINFO_SHORT_SLOTTIME	0x0400
#define	IEEE80211_CAPINFO_RSN			0x0800
/* bit 12 is reserved */
#define	IEEE80211_CAPINFO_DSSSOFDM		0x2000
/* bits 14-15 are reserved */

/*
 * 802.11i/WPA information element (maximally sized).
 */
struct ieee80211_ie_wpa {
	u_int8_t	wpa_id;		/* IEEE80211_ELEMID_VENDOR */
	u_int8_t	wpa_len;	/* length in bytes */
	u_int8_t	wpa_oui[3];	/* 0x00, 0x50, 0xf2 */
	u_int8_t	wpa_type;	/* OUI type */
	u_int16_t	wpa_version;	/* spec revision */
	u_int32_t	wpa_mcipher[1];	/* multicast/group key cipher */
	u_int16_t	wpa_uciphercnt;	/* # pairwise key ciphers */
	u_int32_t	wpa_uciphers[8];/* ciphers */
	u_int16_t	wpa_authselcnt;	/* authentication selector cnt*/
	u_int32_t	wpa_authsels[8];/* selectors */
	u_int16_t	wpa_caps;	/* 802.11i capabilities */
	u_int16_t	wpa_pmkidcnt;	/* 802.11i pmkid count */
	u_int16_t	wpa_pmkids[8];	/* 802.11i pmkids */
} __packed;

/*
* SC_CPU Comments
* Add/Change By Terry Yang, For WPS  At 2007-7-12
* Begin ==>
*/

//#define WSC_FEATURE

#ifdef WSC_FEATURE
/*
 * WSC information element (maximally sized).
 */
#define WSC_HEADER_LEN 2
struct ieee80211_ie_wsc {
	u_int8_t	wsc_id;
	u_int8_t	wsc_len;
	u_int8_t	wsc_oui[3];
	u_int8_t	wsc_type;
	u_int8_t	wsc_data[251];
} __packed;

struct ieee8021x_ie_special_ssn {
	u_int8_t	temp_id;
	u_int8_t	temp_len;
	u_int8_t	temp_data[30];
} __packed;

#define BEACON_IE	1
#define RESP_IE		0
#define WSC_OUI_BYTES		0x00, 0x50, 0xf2
#define WSC_OUI_TYPE		0x04
#define MS_WPS_TYPE         0x05

extern int wsc_getAssocRespIe(char *ie_data);
extern void wsc_probeRequestHandler(unsigned char *mac, char *inputIE,
		int inputIELength,char *outputIE, int *outputIELength);
extern int wsc_wscIEChanged(void);
extern int wsc_wpaIEChanged(void);
extern int wsc_wpa2IEChanged(void);

extern int wsc_wpaIEGet(char *iestr, unsigned char *wpa_ie, int len);
extern int wsc_wscIEGet(char * iestr, int beacon);
extern int wsc_wpa2IEGet(char *iestr, unsigned char *wpa2_ie, int len);

extern unsigned char kget_wsc_enable(void);
extern unsigned char kget_wsc_devcfstat(void);
extern unsigned char kget_wsc_context(void);
extern unsigned char kget_wsc_version(void);
extern unsigned char *kget_wsc_mac(void);
extern unsigned char *kget_wsc_manfa(void);
extern unsigned char *kget_wsc_ssid(void);
extern unsigned char *kget_wsc_modelname(void);

extern unsigned char *kget_wsc_modelnumber(void);
extern unsigned char *kget_wsc_serialnumber(void);
extern unsigned char *kget_wsc_devicename(void);

extern int kget_wscadmin_role(void);
extern int kget_wscadmin_pwdMode(void);

extern char *kget_wscadmin_wsc_pin(void);

extern int kget_wscadmin_seesionTimeout(void);
extern int kget_wscadmin_retransmitTimeout(void);
extern int kget_wscadmin_retryLimit(void);
extern int kget_wscamin_messageTimeout(void);
extern unsigned char  kget_wscadmin_configured(void);
extern unsigned char kget_wscadmin_pbcIsRunning(void);
extern unsigned char  kget_wscadmin_selectedReg(void);
extern unsigned long kget_wscadmin_selectedRegTime(void);
extern unsigned short  kget_wscadmin_selectRegConfigMethod(void);
extern unsigned short  kget_wscadmin_selectRegDevPwdId(void);
extern unsigned char kget_wscadmin_selfPbcPressed(void);
extern unsigned long kget_wscadmin_selfPbcPressedTime(void);
extern unsigned char  kget_wpaIEneedChange(void);
extern unsigned char kget_wsc_encrytype(void);
extern int wsc_msWirelessProvisioningServiceIE(char *ie_data);

#define VAP_OF_WSC(x)    (!((x)->iv_myaddr[0]&(1<<1)))

#endif
/*
* <== Add End
*/

#ifndef _BYTE_ORDER
#error "Don't know native byte order"
#endif

#ifndef IEEE80211N_IE
/* Temporary vendor specific IE for 11n pre-standard interoperability */
#define HT_OUI 0x4c9000
#endif

struct ieee80211_ie_htcap_cmn {
	u_int16_t	hc_cap;			/* HT capabilities */
#if _BYTE_ORDER == _BIG_ENDIAN
	u_int8_t	hc_reserved 	: 3, 	/* B5-7 reserved */
                hc_mpdudensity 	: 3, 	/* B2-4 MPDU density (aka Minimum MPDU Start Spacing) */
                hc_maxampdu 	: 2;	/* B0-1 maximum rx A-MPDU factor */
#else
	u_int8_t	hc_maxampdu 	: 2,	/* B0-1 maximum rx A-MPDU factor */
                hc_mpdudensity 	: 3, 	/* B2-4 MPDU density (aka Minimum MPDU Start Spacing) */
                hc_reserved 	: 3; 	/* B5-7 reserved */
#endif
	u_int8_t 	hc_mcsset[16]; 		/* supported MCS set */
	u_int16_t	hc_extcap;		/* extended HT capabilities */
	u_int32_t	hc_txbf;		/* txbf capabilities */
	u_int8_t	hc_antenna;		/* antenna capabilities */
} __packed;

/*
 * 802.11n HT Capability IE
 */
struct ieee80211_ie_htcap {
	u_int8_t	                    hc_id;			/* element ID */
	u_int8_t	                    hc_len;			/* length in bytes */
	struct ieee80211_ie_htcap_cmn   hc_ie;
} __packed;

/*
 * Temporary vendor private HT Capability IE
 */
struct vendor_ie_htcap {
	u_int8_t	                    hc_id;			/* element ID */
	u_int8_t	                    hc_len;			/* length in bytes */
	u_int8_t                        hc_oui[3];
	u_int8_t                        hc_ouitype;
	struct ieee80211_ie_htcap_cmn   hc_ie;
} __packed;

/* HT capability flags */
#define	IEEE80211_HTCAP_C_ADVCODING		      0x0001
#define	IEEE80211_HTCAP_C_CHWIDTH40	      	  0x0002	
#define	IEEE80211_HTCAP_C_SMPOWERSAVE_STATIC      0x0000 /* Capable of SM Power Save (Static) */
#define	IEEE80211_HTCAP_C_SMPOWERSAVE_DYNAMIC     0x0004 /* Capable of SM Power Save (Dynamic) */
#define	IEEE80211_HTCAP_C_SM_RESERVED             0x0008 /* Reserved */
#define	IEEE80211_HTCAP_C_SM_ENABLED              0x000c /* SM enabled, no SM Power Save */
#define	IEEE80211_HTCAP_C_GREENFIELD	      0x0010
#define IEEE80211_HTCAP_C_SHORTGI20		      0x0020
#define IEEE80211_HTCAP_C_SHORTGI40     	  0x0040
#define IEEE80211_HTCAP_C_TXSTBC        	  0x0080
#define IEEE80211_HTCAP_C_RXSTBC        	  0x0300  /* 2 bits */
#define IEEE80211_HTCAP_C_RXSTBC_S      	       8
#define IEEE80211_HTCAP_C_DELAYEDBLKACK 	  0x0400
#define IEEE80211_HTCAP_C_MAXAMSDUSIZE  	  0x0800  /* 1 = 8K, 0 = 3839B */
#define IEEE80211_HTCAP_C_DSSSCCK40     	  0x1000  
#define IEEE80211_HTCAP_C_PSMP          	  0x2000  
#define IEEE80211_HTCAP_C_INTOLERANT40		  0x4000  
#define IEEE80211_HTCAP_C_LSIGTXOPPROT  	  0x8000  

#define	IEEE80211_HTCAP_C_SM_MASK                 0x000c /* Spatial Multiplexing (SM) capabitlity bitmask */

/* B0-1 maximum rx A-MPDU factor 2^(13+Max Rx A-MPDU Factor) */
enum {
	IEEE80211_HTCAP_MAXRXAMPDU_8192,	/* 2 ^ 13 */
	IEEE80211_HTCAP_MAXRXAMPDU_16384,   /* 2 ^ 14 */
	IEEE80211_HTCAP_MAXRXAMPDU_32768,   /* 2 ^ 15 */
	IEEE80211_HTCAP_MAXRXAMPDU_65536,   /* 2 ^ 16 */
};
#define IEEE80211_HTCAP_MAXRXAMPDU_FACTOR   13

/* B2-4 MPDU density (usec) */
enum {
	IEEE80211_HTCAP_MPDUDENSITY_NA,		/* No time restriction */
	IEEE80211_HTCAP_MPDUDENSITY_0_25,  	/* 1/4 usec */
	IEEE80211_HTCAP_MPDUDENSITY_0_5,    /* 1/2 usec */
	IEEE80211_HTCAP_MPDUDENSITY_1,     	/* 1 usec */
	IEEE80211_HTCAP_MPDUDENSITY_2,     	/* 2 usec */
	IEEE80211_HTCAP_MPDUDENSITY_4,     	/* 4 usec */
	IEEE80211_HTCAP_MPDUDENSITY_8,     	/* 8 usec */
	IEEE80211_HTCAP_MPDUDENSITY_16,     	/* 16 usec */
};

/* HT extended capability flags */
#define	IEEE80211_HTCAP_EXTC_PCO		        0x0001
#define IEEE80211_HTCAP_EXTC_TRANS_TIME_RSVD    0x0000  
#define IEEE80211_HTCAP_EXTC_TRANS_TIME_400     0x0002 /* 20-40 switch time */
#define IEEE80211_HTCAP_EXTC_TRANS_TIME_1500    0x0004 /* in us             */
#define IEEE80211_HTCAP_EXTC_TRANS_TIME_5000    0x0006 
#define IEEE80211_HTCAP_EXTC_RSVD_1             0x00f8
#define IEEE80211_HTCAP_EXTC_MCS_FEEDBACK_NONE  0x0000
#define IEEE80211_HTCAP_EXTC_MCS_FEEDBACK_RSVD  0x0100
#define IEEE80211_HTCAP_EXTC_MCS_FEEDBACK_UNSOL 0x0200
#define IEEE80211_HTCAP_EXTC_MCS_FEEDBACK_FULL  0x0300
#define IEEE80211_HTCAP_EXTC_RSVD_2             0xfc00

struct ieee80211_ie_htinfo_cmn {
	u_int8_t	hi_ctrlchannel;		/* control channel */
#if _BYTE_ORDER == _BIG_ENDIAN
	u_int8_t	hi_serviceinterval : 3,    /* B5-7 svc interval granularity */
                hi_ctrlaccess      : 1,    /* B4   controlled access only */
                hi_rifsmode        : 1,    /* B3   rifs mode */
                hi_txchwidth       : 1,    /* B2   recommended xmiss width set */
                hi_extchoff        : 2;    /* B0-1 extension channel offset */
#else
	u_int8_t    hi_extchoff        : 2,    /* B0-1 extension channel offset */
                hi_txchwidth       : 1,    /* B2   recommended xmiss width set */
                hi_rifsmode        : 1,    /* B3   rifs mode */
                hi_ctrlaccess      : 1,    /* B4   controlled access only */
                hi_serviceinterval : 3;    /* B5-7 svc interval granularity */
#endif
#if _BYTE_ORDER == _BIG_ENDIAN
    u_int8_t   hi_reserved0       : 3,   /* B5-7 Reserved */
                hi_obssnonhtpresent: 1,    /* B4    OBSS Non-HT STAs Present */
                hi_txburstlimit    : 1,    /* B3    Transmit Burst Limit */
                hi_nongfpresent    : 1,    /* B2    Non-greenfield STAs present */
                hi_opmode          : 2;    /* B0-1  Operating Mode */
#else
    u_int8_t   hi_opmode          : 2,    /* B0-1  Operating Mode */
                hi_nongfpresent    : 1,    /* B2    Non-greenfield STAs present */
                hi_txburstlimit    : 1,    /* B3    Transmit Burst Limit */
                hi_obssnonhtpresent: 1,    /* B4    OBSS Non-HT STAs Present */
                hi_reserved0       : 3;   /* B5-7 Reserved */
#endif
    u_int8_t    hi_reserved1;
    u_int16_t   hi_miscflags;

	u_int8_t 	hi_basicmcsset[16]; 	/* basic MCS set */
} __packed;

/*
 * 802.11n HT Information IE
 */
struct ieee80211_ie_htinfo {
	u_int8_t	                    hi_id;			/* element ID */
	u_int8_t	                    hi_len;			/* length in bytes */
	struct ieee80211_ie_htinfo_cmn  hi_ie;
} __packed;

/*
 * Temporary vendor private HT Information IE
 */
struct vendor_ie_htinfo {
	u_int8_t	                    hi_id;			/* element ID */
	u_int8_t                    	hi_len;			/* length in bytes */
    u_int8_t                        hi_oui[3];
    u_int8_t                        hi_ouitype;
    struct ieee80211_ie_htinfo_cmn  hi_ie;
} __packed;

/* extension channel offset (2 bit signed number) */
enum {
	IEEE80211_HTINFO_EXTOFFSET_NA	 = 0,	/* 0  no extension channel is present */			
	IEEE80211_HTINFO_EXTOFFSET_ABOVE = 1,   /* +1 extension channel above control channel */ 
	IEEE80211_HTINFO_EXTOFFSET_UNDEF = 2,   /* -2 undefined */ 
	IEEE80211_HTINFO_EXTOFFSET_BELOW = 3	/* -1 extension channel below control channel*/
};

/* recommended transmission width set */
enum {
	IEEE80211_HTINFO_TXWIDTH_20,		
	IEEE80211_HTINFO_TXWIDTH_2040      	
};

/* operating flags */
#define	IEEE80211_HTINFO_OPMODE_PURE		    0x00 /* no protection */
#define IEEE80211_HTINFO_OPMODE_MIXED_PROT_OPT	0x01 /* prot optional (legacy device maybe present) */      	
#define	IEEE80211_HTINFO_OPMODE_MIXED_PROT_40   0x02 /* prot required (20 MHz) */   
#define	IEEE80211_HTINFO_OPMODE_MIXED_PROT_ALL	0x03 /* prot required (legacy devices present) */      	
#define IEEE80211_HTINFO_OPMODE_MASK		0x03 /* For protection 0x00-0x03 */

/* Non-greenfield STAs present */
enum {
	IEEE80211_HTINFO_NON_GF_NOT_PRESENT,	/* Non-greenfield STAs not present */
	IEEE80211_HTINFO_NON_GF_PRESENT,	/* Non-greenfield STAs present */
};

/* Transmit Burst Limit */
enum {
	IEEE80211_HTINFO_TXBURST_UNLIMITED, /* Transmit Burst is unlimited */
	IEEE80211_HTINFO_TXBURST_LIMITED, /* Transmit Burst is limited */
};

/* OBSS Non-HT STAs present */
enum {
	IEEE80211_HTINFO_OBBSS_NONHT_NOT_PRESENT, /* OBSS Non-HT STAs not present */
	IEEE80211_HTINFO_OBBSS_NONHT_PRESENT, /* OBSS Non-HT STAs present */
};

/* misc flags */
#define IEEE80211_HTINFO_BASICSTBCMCS    0x007F    /* B0-6 basic STBC MCS */
#define IEEE80211_HTINFO_DUALSTBCPROT    0x0080    /* B7   dual stbc protection */
#define IEEE80211_HTINFO_SECONDARYBEACON 0x0100    /* B8   secondary beacon */
#define IEEE80211_HTINFO_LSIGTXOPPROT    0x0200    /* B9   lsig txop prot full support */
#define IEEE80211_HTINFO_PCOACTIVE       0x0400    /* B10  pco active */
#define IEEE80211_HTINFO_PCOPHASE        0x0800    /* B11  pco phase */

/* RIFS mode */
enum {
	IEEE80211_HTINFO_RIFSMODE_PROHIBITED,	/* use of rifs prohibited */
	IEEE80211_HTINFO_RIFSMODE_ALLOWED,	/* use of rifs permitted */
};

/*
 * Management information element payloads.
 */

enum {
	IEEE80211_ELEMID_SSID		= 0,
	IEEE80211_ELEMID_RATES		= 1,
	IEEE80211_ELEMID_FHPARMS	= 2,
	IEEE80211_ELEMID_DSPARMS	= 3,
	IEEE80211_ELEMID_CFPARMS	= 4,
	IEEE80211_ELEMID_TIM		= 5,
	IEEE80211_ELEMID_IBSSPARMS	= 6,
	IEEE80211_ELEMID_COUNTRY	= 7,
	IEEE80211_ELEMID_REQINFO	= 10,
	IEEE80211_ELEMID_CHALLENGE	= 16,
	/* 17-31 reserved for challenge text extension */
	IEEE80211_ELEMID_PWRCNSTR	= 32,
	IEEE80211_ELEMID_PWRCAP		= 33,
	IEEE80211_ELEMID_TPCREQ		= 34,
	IEEE80211_ELEMID_TPCREP		= 35,
	IEEE80211_ELEMID_SUPPCHAN	= 36,
	IEEE80211_ELEMID_CHANSWITCHANN	= 37,
	IEEE80211_ELEMID_MEASREQ	= 38,
	IEEE80211_ELEMID_MEASREP	= 39,
	IEEE80211_ELEMID_QUIET		= 40,
	IEEE80211_ELEMID_IBSSDFS	= 41,
	IEEE80211_ELEMID_ERP		= 42,
	IEEE80211_ELEMID_HTCAP_ANA	= 45,
	IEEE80211_ELEMID_RSN		= 48,
	IEEE80211_ELEMID_XRATES		= 50,
	IEEE80211_ELEMID_HTCAP		= 51,
	IEEE80211_ELEMID_HTINFO		= 52,
	IEEE80211_ELEMID_EXTCHANSWITCHANN = 60,	/* Fix this later as per ANA definition */
	IEEE80211_ELEMID_HTINFO_ANA	= 61,
	IEEE80211_ELEMID_2040_COEXT      = 72,
	IEEE80211_ELEMID_2040_INTOL      = 73,
	IEEE80211_ELEMID_OBSS_SCAN      = 74,
	IEEE80211_ELEMID_EXTCAP		= 127,
	IEEE80211_ELEMID_TPC		= 150,
	IEEE80211_ELEMID_CCKM		= 156,
	IEEE80211_ELEMID_VENDOR		= 221,	/* vendor private */
};

#define IEEE80211_CHANSWITCHANN_BYTES 5
#define IEEE80211_EXTCHANSWITCHANN_BYTES 6

struct ieee80211_tim_ie {
	u_int8_t	tim_ie;			/* IEEE80211_ELEMID_TIM */
	u_int8_t	tim_len;
	u_int8_t	tim_count;		/* DTIM count */
	u_int8_t	tim_period;		/* DTIM period */
	u_int8_t	tim_bitctl;		/* bitmap control */
	u_int8_t	tim_bitmap[1];		/* variable-length bitmap */
} __packed;

struct ieee80211_country_ie {
	u_int8_t	ie;			/* IEEE80211_ELEMID_COUNTRY */
	u_int8_t	len;
	u_int8_t	cc[3];			/* ISO CC+(I)ndoor/(O)utdoor */
	struct {
		u_int8_t schan;			/* starting channel */
		u_int8_t nchan;			/* number channels */
		u_int8_t maxtxpwr;		/* tx power cap */
	} __packed band[4];			/* up to 4 sub bands */
} __packed;

struct ieee80211_quiet_ie {
	u_int8_t	ie;                 /* IEEE80211_ELEMID_QUIET */
	u_int8_t	len;
	u_int8_t	tbttcount;          /* quiet start */
	u_int8_t	period;             /* beacon intervals between quiets*/
	u_int8_t	duration;           /* TUs of each quiet*/
	u_int8_t	offset;             /* TUs of from TBTT of quiet start*/
} __packed;

struct ieee80211_channelswitch_ie {
	u_int8_t	ie;			/* IEEE80211_ELEMID_CHANSWITCHANN */
	u_int8_t	len;
	u_int8_t	switchmode;
	u_int8_t	newchannel;
	u_int8_t	tbttcount;
} __packed;

struct ieee80211_tpc_ie {
        u_int8_t    ie;
        u_int8_t    len;
        u_int8_t    pwrlimit;
} __packed;

#define IEEE80211_CHALLENGE_LEN		128

#define IEEE80211_SUPPCHAN_LEN		26

#define	IEEE80211_RATE_BASIC		0x80
#define	IEEE80211_RATE_HT		0x80
#define	IEEE80211_RATE_VAL		0x7f

/* EPR information element flags */
#define	IEEE80211_ERP_NON_ERP_PRESENT	0x01
#define	IEEE80211_ERP_USE_PROTECTION	0x02
#define	IEEE80211_ERP_LONG_PREAMBLE	0x04

/* Atheros private advanced capabilities info */
#define	ATHEROS_CAP_TURBO_PRIME		0x01
#define	ATHEROS_CAP_COMPRESSION		0x02
#define	ATHEROS_CAP_FAST_FRAME		0x04
/* bits 3-6 reserved */
#define	ATHEROS_CAP_BOOST		0x80

#define	ATH_OUI			0x7f0300		/* Atheros OUI */
#define	ATH_OUI_TYPE		0x01
#define	ATH_OUI_SUBTYPE		0x01
#define ATH_OUI_VERSION		0x00
#define	ATH_OUI_TYPE_XR		0x03
#define	ATH_OUI_VER_XR		0x01
#define ATH_OUI_EXTCAP_TYPE     0x04    /* Atheros Extended Cap Type */
#define ATH_OUI_EXTCAP_SUBTYPE  0x01    /* Atheros Extended Cap Sub-type */
#define ATH_OUI_EXTCAP_VERSION  0x00    /* Atheros Extended Cap Version */


#define	WPA_OUI			0xf25000
#define	WPA_OUI_TYPE		0x01
#define	WPA_VERSION		1		/* current supported version */

#define WSC_OUI                 0x0050f204

#define	WPA_CSE_NULL		0x00
#define	WPA_CSE_WEP40		0x01
#define	WPA_CSE_TKIP		0x02
#define	WPA_CSE_CCMP		0x04
#define	WPA_CSE_WEP104		0x05

#define	WPA_ASE_NONE		0x00
#define	WPA_ASE_8021X_UNSPEC	0x01
#define	WPA_ASE_8021X_PSK	0x02

#define	RSN_OUI			0xac0f00
#define	RSN_VERSION		1		/* current supported version */

#define	RSN_CSE_NULL		0x00
#define	RSN_CSE_WEP40		0x01
#define	RSN_CSE_TKIP		0x02
#define	RSN_CSE_WRAP		0x03
#define	RSN_CSE_CCMP		0x04
#define	RSN_CSE_WEP104		0x05

#define	RSN_ASE_NONE		0x00
#define	RSN_ASE_8021X_UNSPEC	0x01
#define	RSN_ASE_8021X_PSK	0x02

#define	RSN_CAP_PREAUTH		0x01

#define	WME_OUI			0xf25000
#define	WME_OUI_TYPE		0x02
#define	WME_INFO_OUI_SUBTYPE	0x00
#define	WME_PARAM_OUI_SUBTYPE	0x01
#define	WME_VERSION		1

/* WME stream classes */
#define	WME_AC_BE	0		/* best effort */
#define	WME_AC_BK	1		/* background */
#define	WME_AC_VI	2		/* video */
#define	WME_AC_VO	3		/* voice */

#ifdef ATH_WPS_IE
#define WPS_OUI         0xf25000
#define WPS_OUI_TYPE        0x04
#endif /* ATH_WPS_IE */

/*
 * AUTH management packets
 *
 *	octet algo[2]
 *	octet seq[2]
 *	octet status[2]
 *	octet chal.id
 *	octet chal.length
 *	octet chal.text[253]
 */

typedef u_int8_t *ieee80211_mgt_auth_t;

#define	IEEE80211_AUTH_ALGORITHM(auth) \
	((auth)[0] | ((auth)[1] << 8))
#define	IEEE80211_AUTH_TRANSACTION(auth) \
	((auth)[2] | ((auth)[3] << 8))
#define	IEEE80211_AUTH_STATUS(auth) \
	((auth)[4] | ((auth)[5] << 8))

#define	IEEE80211_AUTH_ALG_OPEN		0x0000
#define	IEEE80211_AUTH_ALG_SHARED	0x0001
#define	IEEE80211_AUTH_ALG_LEAP		0x0080

enum {
	IEEE80211_AUTH_OPEN_REQUEST		= 1,
	IEEE80211_AUTH_OPEN_RESPONSE		= 2,
};

enum {
	IEEE80211_AUTH_SHARED_REQUEST		= 1,
	IEEE80211_AUTH_SHARED_CHALLENGE		= 2,
	IEEE80211_AUTH_SHARED_RESPONSE		= 3,
	IEEE80211_AUTH_SHARED_PASS		= 4,
};

/*
 * Reason codes
 *
 * Unlisted codes are reserved
 */

enum {
	IEEE80211_REASON_UNSPECIFIED		= 1,
	IEEE80211_REASON_AUTH_EXPIRE		= 2,
	IEEE80211_REASON_AUTH_LEAVE		= 3,
	IEEE80211_REASON_ASSOC_EXPIRE		= 4,
	IEEE80211_REASON_ASSOC_TOOMANY		= 5,
	IEEE80211_REASON_NOT_AUTHED		= 6,
	IEEE80211_REASON_NOT_ASSOCED		= 7,
	IEEE80211_REASON_ASSOC_LEAVE		= 8,
	IEEE80211_REASON_ASSOC_NOT_AUTHED	= 9,

	IEEE80211_REASON_RSN_REQUIRED		= 11,
	IEEE80211_REASON_RSN_INCONSISTENT	= 12,
	IEEE80211_REASON_IE_INVALID		= 13,
	IEEE80211_REASON_MIC_FAILURE		= 14,

	IEEE80211_STATUS_SUCCESS		= 0,
	IEEE80211_STATUS_UNSPECIFIED		= 1,
	IEEE80211_STATUS_CAPINFO		= 10,
	IEEE80211_STATUS_NOT_ASSOCED		= 11,
	IEEE80211_STATUS_OTHER			= 12,
	IEEE80211_STATUS_ALG			= 13,
	IEEE80211_STATUS_SEQUENCE		= 14,
	IEEE80211_STATUS_CHALLENGE		= 15,
	IEEE80211_STATUS_TIMEOUT		= 16,
	IEEE80211_STATUS_TOOMANY		= 17,
	IEEE80211_STATUS_BASIC_RATE		= 18,
	IEEE80211_STATUS_SP_REQUIRED		= 19,
	IEEE80211_STATUS_PBCC_REQUIRED		= 20,
	IEEE80211_STATUS_CA_REQUIRED		= 21,
	IEEE80211_STATUS_TOO_MANY_STATIONS	= 22,
	IEEE80211_STATUS_RATES			= 23,
	IEEE80211_STATUS_SHORTSLOT_REQUIRED	= 25,
	IEEE80211_STATUS_DSSSOFDM_REQUIRED	= 26,
	IEEE80211_STATUS_REFUSED		= 37,
	IEEE80211_STATUS_INVALID_PARAM		= 38,
};

#define	IEEE80211_WEP_KEYLEN		5	/* 40bit */
#define	IEEE80211_WEP_IVLEN		3	/* 24bit */
#define	IEEE80211_WEP_KIDLEN		1	/* 1 octet */
#define	IEEE80211_WEP_CRCLEN		4	/* CRC-32 */
#define	IEEE80211_WEP_NKID		4	/* number of key ids */

/*
 * 802.11i defines an extended IV for use with non-WEP ciphers.
 * When the EXTIV bit is set in the key id byte an additional
 * 4 bytes immediately follow the IV for TKIP.  For CCMP the
 * EXTIV bit is likewise set but the 8 bytes represent the
 * CCMP header rather than IV+extended-IV.
 */
#define	IEEE80211_WEP_EXTIV		0x20
#define	IEEE80211_WEP_EXTIVLEN		4	/* extended IV length */
#define	IEEE80211_WEP_MICLEN		8	/* trailing MIC */

#define	IEEE80211_CRC_LEN		4

/*
 * Maximum acceptable MTU is:
 *	IEEE80211_MAX_LEN - WEP overhead - CRC -
 *		QoS overhead - RSN/WPA overhead
 * Min is arbitrarily chosen > IEEE80211_MIN_LEN.  The default
 * mtu is Ethernet-compatible; it's set by ether_ifattach.
 */
#define	IEEE80211_MTU_MAX		2290
#define	IEEE80211_MTU_MIN		32

#define	IEEE80211_MAX_MPDU_LEN		(3840 + IEEE80211_CRC_LEN + \
    (IEEE80211_WEP_IVLEN + IEEE80211_WEP_KIDLEN + IEEE80211_WEP_CRCLEN))
#define	IEEE80211_ACK_LEN \
	(sizeof(struct ieee80211_frame_ack) + IEEE80211_CRC_LEN)
#define	IEEE80211_MIN_LEN \
	(sizeof(struct ieee80211_frame_min) + IEEE80211_CRC_LEN)

/* An 802.11 data frame can be one of three types:
1. An unaggregated frame: The maximum length of an unaggregated data frame is 2324 bytes + headers.
2. A data frame that is part of an AMPDU: The maximum length of an AMPDU may be upto 65535 bytes, but data frame is limited to 2324 bytes + header.
3. An AMSDU: The maximum length of an AMSDU is eihther 3839 or 7095 bytes.
The maximum frame length supported by hardware is 4095 bytes.
A length of 3839 bytes is chosen here to support unaggregated data frames, any size AMPDUs and 3839 byte AMSDUs.
*/
#define IEEE80211N_MAX_FRAMELEN  3839
#define IEEE80211N_MAX_LEN (IEEE80211N_MAX_FRAMELEN + IEEE80211_CRC_LEN + \
    (IEEE80211_WEP_IVLEN + IEEE80211_WEP_KIDLEN + IEEE80211_WEP_CRCLEN))

#define IEEE80211_TX_CHAINMASK_MIN	1
#define IEEE80211_TX_CHAINMASK_MAX	7

#define IEEE80211_RX_CHAINMASK_MIN	1
#define IEEE80211_RX_CHAINMASK_MAX	7

/*
 * The 802.11 spec says at most 2007 stations may be
 * associated at once.  For most AP's this is way more
 * than is feasible so we use a default of 128.  This
 * number may be overridden by the driver and/or by
 * user configuration.
 */
#define	IEEE80211_AID_MAX		2007
#define	IEEE80211_AID_DEF		128

#define	IEEE80211_AID(b)	((b) &~ 0xc000)

/* 
 * RTS frame length parameters.  The default is specified in
 * the 802.11 spec.  The max may be wrong for jumbo frames.
 */
#define	IEEE80211_RTS_DEFAULT		512
#define	IEEE80211_RTS_MIN		0
#define	IEEE80211_RTS_MAX		2347

/* 
 * Regulatory extention identifier for country IE.
 */
#define IEEE80211_REG_EXT_ID		201

#define IEEE80211_OBSS_SCAN_PASSIVE_DWELL_DEF  20
#define IEEE80211_OBSS_SCAN_ACTIVE_DWELL_DEF   10
#define IEEE80211_OBSS_SCAN_INTERVAL_DEF       300
#define IEEE80211_OBSS_SCAN_PASSIVE_TOTAL_DEF  200
#define IEEE80211_OBSS_SCAN_ACTIVE_TOTAL_DEF   20
#define IEEE80211_OBSS_SCAN_THRESH_DEF   25
#define IEEE80211_OBSS_SCAN_DELAY_DEF   5

/*
 * overlapping BSS scan ie
 */
struct ieee80211_ie_obss_scan {
        u_int8_t elem_id;
        u_int8_t elem_len;
        u_int16_t scan_passive_dwell;
        u_int16_t scan_active_dwell;
        u_int16_t scan_interval;
        u_int16_t scan_passive_total;
        u_int16_t scan_active_total;
        u_int16_t scan_delay;
        u_int16_t scan_thresh;
} __packed;

/*
 * Extended capability ie
 */
struct ieee80211_ie_ext_cap {
        u_int8_t elem_id;
        u_int8_t elem_len;
        u_int8_t ext_capflags;
} __packed;

/* 
 * 20/40 BSS coexistence ie
 */
struct ieee80211_ie_bss_coex {
        u_int8_t elem_id;
        u_int8_t elem_len;
#if _BYTE_ORDER == _BIG_ENDIAN
        u_int8_t reserved1          : 1,
                 reserved2          : 1,
                 reserved3          : 1,
                 obss_exempt_grant  : 1,
                 obss_exempt_req    : 1,
                 ht20_width_req       : 1,
                 ht40_intolerant      : 1,
                 inf_request        : 1;
#else
        u_int8_t inf_request        : 1,
                 ht40_intolerant      : 1,
                 ht20_width_req       : 1,
                 obss_exempt_req    : 1,
                 obss_exempt_grant  : 1,
                 reserved3          : 1,
                 reserved2          : 1,
                 reserved1          : 1;
#endif
} __packed;

/*
 * 20/40 BSS intolerant channel report ie
 */
struct ieee80211_ie_intolerant_report {
        u_int8_t elem_id;
        u_int8_t elem_len;
        u_int8_t reg_class;
        u_int8_t chan_list[1];          /* variable-length channel list */
} __packed;
 

/*
 * 20/40 coext management action frame 
 */
struct ieee80211_action_bss_coex_frame {
        struct ieee80211_action                ac_header;
        struct ieee80211_ie_bss_coex           coex;
        struct ieee80211_ie_intolerant_report    chan_report;
} __packed;


#endif /* _NET80211_IEEE80211_H_ */
