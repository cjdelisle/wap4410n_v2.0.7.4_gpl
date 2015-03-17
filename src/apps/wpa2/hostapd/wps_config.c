/**************************************************************************
//
//  Copyright (c) 2006-2007 Sony Corporation. All Rights Reserved.
//
//  File Name: wps_config.c
//  Description: EAP-WPS config source
//
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions
//   are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in
//       the documentation and/or other materials provided with the
//       distribution.
//     * Neither the name of Sony Corporation nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
//   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************/

#include "includes.h"

#include "defs.h"
#include "common.h"
#include "hostapd.h"
#include "config.h"
#include "wps_config.h"
#include "wps_parser.h"
#include "eap_wps.h"
#ifndef WPS_WRITE_CHANGES_ONLY
#include "config_rewrite.h"
#endif
#include "nvram.h" //Raul Add for WPS configure settings
extern int eap_wps_free_dh(void **dh);

static int is_hex(const u8 *data, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++) {
		if (data[i] < 32 || data[i] >= 127)
			return 1;
	}
	return 0;
}

static inline int num2hex(unsigned num, int unused)
{
    return (num < 10) ? (num+'0') : ((num < 16) ? (num+'A'-10) : '?');
}


int wps_config_free_dh(void **dh)
{
	return eap_wps_free_dh(dh);
}

int wps_get_ap_ssid_configuration(
        void *ctx,      /* struct hostapd_data *hapd */ 
        u8 **buf,       /* output, allocated i.e. buffer */
        size_t *len,    /* output, length of data in buffer */
        int inband,     /* nonzero if via wifi */
        int autoconfig) /* nonzero to invent ssid+psk (and maybe mod modes)*/
{
	int ret = -1;
	struct hostapd_data *hapd = ctx;
	struct hostapd_bss_config *conf = hapd->conf;
	struct wps_data *wps = 0;
	struct hostapd_ssid *ssid;
	u16 auth, encr;
	Boolean enabled8021x = 0;
	int nwKeyIdx = -1;
	u8 *nwKey = NULL;
        size_t nw_key_length = 0;
        int allocated = 0;
        u8 nwKeyBuf[2*PMK_LEN+1];

	do {
		if (!conf || !buf || !len) {
			break;
                }
		*buf = 0;
		*len = 0;

		ssid = &conf->ssid;

		if (conf->wpa_key_mgmt & WPA_KEY_MGMT_IEEE8021X) {
			auth = 0;
			if (conf->wpa & HOSTAPD_WPA_VERSION_WPA2)
				auth = WPS_AUTHTYPE_WPA2;
			else if (conf->wpa & HOSTAPD_WPA_VERSION_WPA)
				auth = WPS_AUTHTYPE_WPA;

			if (!auth) {
				auth = WPS_AUTHTYPE_OPEN;
				encr = WPS_ENCRTYPE_WEP;
			} else {
				if (conf->wpa_pairwise & WPA_CIPHER_CCMP)
					encr = WPS_ENCRTYPE_AES;
				else if (conf->wpa_pairwise & WPA_CIPHER_TKIP)
					encr = WPS_ENCRTYPE_TKIP;
			}
			enabled8021x = 1;
		} else if (conf->wpa_key_mgmt & WPA_KEY_MGMT_PSK) {
			if (conf->wpa & HOSTAPD_WPA_VERSION_WPA2)
				auth = WPS_AUTHTYPE_WPA2PSK;
			else if (conf->wpa & HOSTAPD_WPA_VERSION_WPA)
				auth = WPS_AUTHTYPE_WPAPSK;
			else
				break;

			if (conf->wpa_pairwise & WPA_CIPHER_CCMP)
				encr = WPS_ENCRTYPE_AES;
			else if (conf->wpa_pairwise & WPA_CIPHER_TKIP)
				encr = WPS_ENCRTYPE_TKIP;
			nwKeyIdx = 0;
		} else {
			if (conf->auth_algs & HOSTAPD_AUTH_SHARED_KEY) {
				auth = WPS_AUTHTYPE_SHARED;
				nwKeyIdx = ssid->wep.idx + 1;
			} else
				auth = WPS_AUTHTYPE_OPEN;

			if (ssid->wep.keys_set) {
				encr = WPS_ENCRTYPE_WEP;
				nwKeyIdx = ssid->wep.idx + 1;
			} else
				encr = WPS_ENCRTYPE_NONE;
		}

                if (autoconfig) {
                    /* If doing autoconfig, retain configured authentication
                     * and encryption ONLY for WPA/WPA2 and TKIP/AES ...
                     * with PSK only...
                     * we could retain more but we'd have to add more
                     * code here.
                     */
                    nwKeyIdx = 0;
                    if (auth == WPS_AUTHTYPE_WPA) {
                        auth = WPS_AUTHTYPE_WPAPSK;
                    } else
                    if (auth == WPS_AUTHTYPE_WPA2) {
                        auth = WPS_AUTHTYPE_WPA2PSK;
                    }
                    if (auth == WPS_AUTHTYPE_WPA2PSK || 
                                auth == WPS_AUTHTYPE_WPAPSK) { 
                        if (encr == WPS_ENCRTYPE_AES || 
                                    encr == WPS_ENCRTYPE_TKIP ) {
                            /* ok */
                        } else {
                            if (auth == WPS_AUTHTYPE_WPA2PSK)
                                encr = WPS_ENCRTYPE_AES;
                            else
                                encr = WPS_ENCRTYPE_TKIP;
                        }
                    } else {
                        wpa_printf(MSG_INFO,
                            "Auto-config during WPS, set WPA2PSK-AES");
                        auth = WPS_AUTHTYPE_WPA2PSK;
                        encr = WPS_ENCRTYPE_AES;
                    }
                    enabled8021x = 0;   /* don't support this for now */
                    nwKeyIdx = -1;
                }


		if (wps_create_wps_data(&wps))
			break;

		/* SSID */
                if (autoconfig) {
	            char new_ssid[32 + 1];
                    #if 0  /* Works, but not what was wanted */
                    u8 tmp[16];
                    int i;
		    if (eap_wps_generate_device_password(tmp, SIZE_16_BYTES))
			break;
		    for (i = 0; i < 16; i++) {
			new_ssid[(2 * i) + 0] = num2hex((tmp[i] >> 4) & 0x0f, 1);
			new_ssid[(2 * i) + 1] = num2hex((tmp[i] >> 0) & 0x0f, 1);
		    }
		    new_ssid[32] = 0;
                    #else
                    sprintf(new_ssid, "Network-%02x%02x%02x%02x%02x%02x",
                        hapd->own_addr[0], hapd->own_addr[1],
                        hapd->own_addr[2], hapd->own_addr[3],
                        hapd->own_addr[4], hapd->own_addr[5]);
                    #endif
		    if (wps_set_value(wps, WPS_TYPE_SSID, new_ssid, strlen(new_ssid)))
			break;
                } else
		if (wps_set_value(wps, WPS_TYPE_SSID, ssid->ssid, ssid->ssid_len))
			break;

		if (inband) {
			/* MAC Address */
			if (wps_set_value(wps, WPS_TYPE_MAC_ADDR, hapd->own_addr, ETH_ALEN))
				break;
		}

		/* Authentication Type */
		if (wps_set_value(wps, WPS_TYPE_AUTH_TYPE, &auth, 0))
			break;

		/* Encryption Type */
		if (wps_set_value(wps, WPS_TYPE_ENCR_TYPE, &encr, 0))
			break;

                /* Network key */
                if (autoconfig ) {
                        /* Only wpa/wpa2 supported for autoconfig for now */
                        u8 psk[PMK_LEN];
                        int i;
		        if (eap_wps_generate_device_password(psk, PMK_LEN))
			    break;
                        nwKey = nwKeyBuf;
			allocated = 0;
		        for (i = 0; i < PMK_LEN; i++) {
			    nwKey[(2 * i) + 0] = num2hex((psk[i] >> 4) & 0x0f, 1);
			    nwKey[(2 * i) + 1] = num2hex((psk[i] >> 0) & 0x0f, 1);
		        }
		        nwKey[2*PMK_LEN] = 0;
			nw_key_length = PMK_LEN*2;
                } else
		if (nwKeyIdx > 0) { /* WEP Network Key*/
				/* cast nwKeyIdx to u8 since wps_set_value using lower bits to set key index */
				u8 tnwKeyIdx = nwKeyIdx; 
				if (wps_set_value(wps, WPS_TYPE_NW_KEY_INDEX, &tnwKeyIdx , 0))
					break;
				if (is_hex(ssid->wep.key[nwKeyIdx - 1], ssid->wep.len[nwKeyIdx - 1])) {
					nwKey = wpa_zalloc(ssid->wep.len[nwKeyIdx - 1] * 2 + 1);
					if (!nwKey)
						break;
					allocated = 1;
				nw_key_length = wpa_snprintf_hex_uppercase((char *)nwKey, ssid->wep.len[nwKeyIdx - 1] * 2 + 1,
														ssid->wep.key[nwKeyIdx - 1], ssid->wep.len[nwKeyIdx - 1]);
				if (nw_key_length != ssid->wep.len[nwKeyIdx - 1] * 2) {
						os_free(nwKey);
						allocated = 0;
						break;
					}
				} else {
				nw_key_length = ssid->wep.len[nwKeyIdx - 1];
				nwKey = wpa_zalloc(nw_key_length + 1);
					if (!nwKey)
						break;
					allocated = 1;
				os_memcpy((char *)nwKey, ssid->wep.key[nwKeyIdx - 1], nw_key_length);
			}
                } else
                if (nwKeyIdx == 0) {
			/* Not WEP */
			if (ssid->wpa_passphrase) {
				nw_key_length = os_strlen(ssid->wpa_passphrase);
				nwKey = (u8 *)ssid->wpa_passphrase;
			} else if (ssid->wpa_psk) {
				nwKey = wpa_zalloc(PMK_LEN * 2 + 1);
				if (!nwKey)
					break;
				allocated = 1;
				nw_key_length = wpa_snprintf_hex_uppercase((char *)nwKey, PMK_LEN * 2 + 1, ssid->wpa_psk->psk, sizeof(ssid->wpa_psk->psk));
				if (nw_key_length != PMK_LEN * 2) {
					os_free(nwKey);
					allocated = 0;
				break;
			        }
			}
		}
		/* Network Key */
                if (nwKey && nw_key_length) {
		        if (wps_set_value(wps, WPS_TYPE_NW_KEY, nwKey, nw_key_length)) {
				break;
			}
		} else {
                        /* Bug fix for 53328 - NW key is required in credential, set to NULL for open
                        mode security.*/
		        if (wps_set_value(wps, WPS_TYPE_NW_KEY, NULL, 0)) {
                                wpa_printf(MSG_INFO, "%s %d nw_key_length=%d\n", __func__, __LINE__, nw_key_length);
				break;
			}
                }
                if (nwKey && allocated) {
                        os_free(nwKey);
                        nwKey = NULL;
                }

		if (!inband) {
			/* MAC Address */
			if (wps_set_value(wps, WPS_TYPE_MAC_ADDR, hapd->own_addr, ETH_ALEN))
				break;
		}

		if (enabled8021x) {
			/* EAP Type (Option) */

			/* EAP Identity (Option) */

			/* Key Provided Automaticaly (Option) */

			/* 802.1X Enabled (Option) */
			if (wps_set_value(wps, WPS_TYPE_8021X_ENABLED, &enabled8021x, 0))
				break;
		}

		if (wps_write_wps_data(wps, buf, len))
			break;
		ret = 0;
	} while (0);

	(void)wps_destroy_wps_data(&wps);

	if (ret) {
		if (buf && *buf) {
			os_free(*buf);
			*buf = 0;
		}
		if (len)
			*len = 0;
	}
	return ret;
}


#if 0   /* Was, from Sony */
int wps_get_ap_auto_configuration(void *ctx, u8 **buf, size_t *len)
{
	int ret = -1;
	struct hostapd_data *hapd = ctx;
	struct wps_data *wps = 0;
	char ssid[SIZE_32_BYTES + 1];
	u16 auth, encr;
	u8 nwKeyIdx;
	char nwKey[SIZE_64_BYTES + 1];
	u8 tmp[SIZE_32_BYTES];
	int i;

	do {
		if (!buf || !len)
			break;
		*buf = 0;
		*len = 0;

		if (wps_create_wps_data(&wps))
			break;

		/* Generate SSID */
		if (eap_wps_generate_device_password(tmp, SIZE_16_BYTES))
			break;
		for (i = 0; i < SIZE_16_BYTES; i++) {
			ssid[(2 * i) + 0] = num2hex((tmp[i] >> 4) & 0x0f, 1);
			ssid[(2 * i) + 1] = num2hex((tmp[i] >> 0) & 0x0f, 1);
		}
		ssid[SIZE_32_BYTES] = 0;
		/* SSID */
		if (wps_set_value(wps, WPS_TYPE_SSID, ssid, SIZE_32_BYTES))
			break;

		/* Authentication Type */
		auth = WPS_AUTHTYPE_WPAPSK;
		if (wps_set_value(wps, WPS_TYPE_AUTH_TYPE, &auth, 0))
			break;

		/* Encryption Type */
		encr = WPS_ENCRTYPE_TKIP;
		if (wps_set_value(wps, WPS_TYPE_ENCR_TYPE, &encr, 0))
			break;

		/* Network Key Index (Option) */
		nwKeyIdx = 1;
		if (wps_set_value(wps, WPS_TYPE_NW_KEY_INDEX, &nwKeyIdx, 0))
			break;

		/* Generate Network Key */
		if (eap_wps_generate_device_password(tmp, SIZE_32_BYTES))
			break;
		for (i = 0; i < SIZE_32_BYTES; i++) {
			nwKey[(2 * i) + 0] = num2hex((tmp[i] >> 4) & 0x0f, 1);
			nwKey[(2 * i) + 1] = num2hex((tmp[i] >> 0) & 0x0f, 1);
		}
		nwKey[SIZE_64_BYTES] = 0;
		/* Network Key */
		if (wps_set_value(wps, WPS_TYPE_NW_KEY, nwKey, SIZE_64_BYTES))
			break;

		/* MAC Address */
		if (wps_set_value(wps, WPS_TYPE_MAC_ADDR, hapd->own_addr, ETH_ALEN))
			break;

		if (wps_write_wps_data(wps, buf, len))
			break;

		ret = 0;
	} while (0);

	(void)wps_destroy_wps_data(&wps);

	if (ret) {
		if (buf && *buf) {
			os_free(*buf);
			*buf = 0;
		}
		if (len)
			*len = 0;
	}

	return ret;
}
#endif  /* Was, from Sony */


/*
 * Make a temporary filepath in same directory as permanent file.
 * Returns NULL if error, else buffer containing new filepath.
 */
char *wps_config_temp_filepath_make(
        const char *original_filepath,
        const char *filename            /* new name to use (no slashes) */
        )
{
        int original_len = strlen(original_filepath);
        int name_len = strlen(filename);
        char *filepath;

        filepath = os_malloc(original_len + name_len + 1); /* more than big enough */
        if (filepath == NULL) return NULL;
        /* Make temporary file in same directory as final one */
        strcpy(filepath, original_filepath);
        if (strrchr(filepath,'/')) {
                strrchr(filepath,'/')[1] = 0;
        } else {
                filepath[0] = 0;
        }
        strcat(filepath, filename);
        return filepath;
}



/* Copy WPS configuration data from WPS information elements (e.g. from M8
 *      message) into configuration file; the configuration file is so edited.
 *      Optionally, the "wps_configured" field can be written as well
 *      (if wps_configured passed nonzero); if wps_configured is passed
 *      as zero, then the value for the "wps_configured" field is NOT
 *      changed in the configuration file.
 */
int wps_set_ap_ssid_configuration(void *ctx, char *filename, u8 *buf, size_t len,
                int wps_configured)
{
	int ret = -1;
	struct hostapd_data *hapd = ctx;
	struct hostapd_bss_config *conf;
	struct wps_data *wps = 0;
	u8 nwIdx = 0;
	u8 str_ssid[33];
	size_t ssid_length;
	u16 auth, encr;
	u8 nwKeyIdx;
	u8 *nwKey = 0;
	size_t nwKey_length;
	u8 macAddr[6];
	char *eapType = 0;
	char *eapIdentity = 0;
	Boolean keyProvideAuto;
	Boolean enabled8021X;
	size_t length;
	int value, wpa;
	char *tmp;
        #ifdef WPS_WRITE_CHANGES_ONLY   /* the original Sony code */
	FILE *f = 0;
        #else
        struct config_rewrite *rewrite = NULL;
        #endif
        char tmpbuf[256];  /* temporary formatting buffer */

	do {
		if (!hapd || !hapd->conf)
			break;
		conf = hapd->conf;

		if (wps_create_wps_data(&wps))
			break;

		if(wps_parse_wps_data(buf, len, wps))
			break;

		/* Network Index */
		(void)wps_get_value(wps, WPS_TYPE_NW_INDEX, &nwIdx, NULL);
		/** Ignore Network Index **/

		/* SSID */
		ssid_length = sizeof(str_ssid);
		if(wps_get_value(wps, WPS_TYPE_SSID, str_ssid, &ssid_length))
			break;
		str_ssid[ssid_length] = 0;

		/* Authentication Type */
		if (wps_get_value(wps, WPS_TYPE_AUTH_TYPE, &auth, NULL))
			break;

		/* Encryption Type */
		if (wps_get_value(wps, WPS_TYPE_ENCR_TYPE, &encr, NULL))
			break;

		/* Network Key Index (Option) */
		if(wps_get_value(wps, WPS_TYPE_NW_KEY_INDEX, &nwKeyIdx, NULL))
			nwKeyIdx = 1;
		if ((1 > nwKeyIdx) || (4 < nwKeyIdx)) {
			wpa_printf(MSG_WARNING, "Network Key Index is fixed. %d -> 1\n", nwKeyIdx);
			nwKeyIdx = 1;
		}

		/* Network Key */
		nwKey_length = 0;
		(void)wps_get_value(wps, WPS_TYPE_NW_KEY, NULL, &nwKey_length);
		if (nwKey_length) {
			nwKey = (u8 *)calloc(1, nwKey_length + 1);
			if (!nwKey)
				break;
			if (wps_get_value(wps, WPS_TYPE_NW_KEY, nwKey, &nwKey_length)) {
				break;
			}
			nwKey[nwKey_length] = 0;
                        /* Work around for weird Vista bug whereby 
                         * 64-byte WPA key is sent one nibble per byte
                         * instead of one hex char per byte,
                         * e.g. 0x7 instead of '7' and 0xc instead of 'c'
                         */
                        if (nwKey_length == 64 && nwKey[0] < 16) {
                                int i;
                                for (i = 0; i < nwKey_length; i++) {
                                        if (nwKey[i] < 10) {
                                                nwKey[i] += '0';
                                        } else
                                        if (nwKey[i] < 16) {
                                                nwKey[i] += ('A' - 10);
                                        }
                                }
                                wpa_printf(MSG_INFO, 
                                        "Fixed NW Key to: %s", nwKey);
                        }
                        /* Work around to remove null termination 
                         * in passphrase. Decremnt the key length by one
                         * checking for <= 64 - to cover the case of
                         * passphrase of length 63 with null termination
                         * included.
                         */
                        else if ((nwKey_length > 8) && (nwKey_length <= 64) &&
                                 (nwKey[0] > 0xf) && 
                                 (nwKey[nwKey_length-1] == '\0')) {
                            nwKey_length--;
                            wpa_printf(MSG_INFO,"Removed the null termination from passphrase: key_length %d", nwKey_length);
                        }
		}

		/* MAC Address */
		length = sizeof(macAddr);
		if(wps_get_value(wps, WPS_TYPE_MAC_ADDR, macAddr, &length))
			break;

		/* EAP Type (Option) */
		length = 0;
		(void)wps_get_value(wps, WPS_TYPE_EAP_TYPE, NULL, &length);
		if (length) {
			eapType = (char *)calloc(1, length + 1);
			if (!eapType)
				break;
			if (wps_get_value(wps, WPS_TYPE_EAP_TYPE, eapType, &length)) {
				break;
			}
			eapType[length] = 0;
		}

		/* EAP Identity (Option) */
		length = 0;
		(void)wps_get_value(wps, WPS_TYPE_EAP_IDENTITY, NULL, &length);
		if (length) {
			eapIdentity = (char *)calloc(1, length + 1);
			if (!eapIdentity)
				break;
			if (wps_get_value(wps, WPS_TYPE_EAP_IDENTITY, eapIdentity, &length)) {
				break;
			}
			eapIdentity[length] = 0;
		}

		/* Key Provided Automaticaly (Option) */
		if(wps_get_value(wps, WPS_TYPE_KEY_PROVIDED_AUTO, &keyProvideAuto, NULL))
			keyProvideAuto = 0;

		/* 802.1X Enabled (Option) */
		if(wps_get_value(wps, WPS_TYPE_8021X_ENABLED, &enabled8021X, NULL))
			enabled8021X = 0;

		/* Set Configuration */
                #ifdef WPS_WRITE_CHANGES_ONLY   /* the original Sony code */
                /* Original sony code wrote only the new fields,
                 * and not organized by section.
                 */
		f = fopen(filename, "w");
		if (!f)
			break;
                #else /*Atheros change */
                /* Read original config file and write a new one with
                 * some fields modified.
                 */
                rewrite = config_rewrite_create(
                    conf->config_fname   /* original file as input */
                    );
                if (rewrite == NULL) {
                    break;
                }
                #endif

                #ifdef WPS_WRITE_CHANGES_ONLY   /* for original Sony code */
                #define WPS_CHANGE_WRITE(section)       \
                    fwrite(tmpbuf, strlen(tmpbuf), 1, f)
                #else
                #define WPS_CHANGE_WRITE(section)       \
                    config_rewrite_line(rewrite, tmpbuf, section)
                #endif

		/* ssid */
		snprintf(tmpbuf, sizeof(tmpbuf), "ssid=%s\n", str_ssid);
                WPS_CHANGE_WRITE(NULL);
		nvram_set("wlan0_ssid0",str_ssid);
		nvram_set("wifi_wsc_configured","1");
		/* auth_algs */
		if (WPS_AUTHTYPE_SHARED == auth)
			value = 2;
		else
			value = 1;
		snprintf(tmpbuf, sizeof(tmpbuf), "auth_algs=%d\n", value);
                WPS_CHANGE_WRITE(NULL);

		/* wpa */
		switch (auth) {
		case WPS_AUTHTYPE_WPA:
		case WPS_AUTHTYPE_WPAPSK:
		case WPS_AUTHTYPE_WPA2:
		case WPS_AUTHTYPE_WPA2PSK:
                /*Auto Configure must set AP always to WPA AUTO */
		if(conf->wps->autoconfig) {
                    wpa = HOSTAPD_WPA_VERSION_WPA | HOSTAPD_WPA_VERSION_WPA2;
		    encr = WPS_ENCRTYPE_TKIP | WPS_ENCRTYPE_AES;
		} else {
		    if (auth == WPS_AUTHTYPE_WPA || auth == WPS_AUTHTYPE_WPAPSK)
                        wpa = HOSTAPD_WPA_VERSION_WPA;
		    else if (auth == WPS_AUTHTYPE_WPA2 || auth == WPS_AUTHTYPE_WPA2PSK)
                        wpa = HOSTAPD_WPA_VERSION_WPA2;
                    else 
                        wpa = HOSTAPD_WPA_VERSION_WPA | HOSTAPD_WPA_VERSION_WPA2;
                }
			break;
		default:
			wpa = 0;
			break;
		}
		snprintf(tmpbuf, sizeof(tmpbuf), "wpa=%d\n", wpa);
                WPS_CHANGE_WRITE(NULL);

		/* wpa_key_mgmt */
		if (wpa) {
			switch (auth) {
			case WPS_AUTHTYPE_WPAPSK:
				nvram_set("wlan0_ssid0_auth_mode","AUTH_WPAPSK");
				tmp = "WPA-PSK";
				break;
			case WPS_AUTHTYPE_WPA2PSK:
				nvram_set("wlan0_ssid0_auth_mode","AUTH_WPA2PSK");
				tmp = "WPA-PSK";
				break;
			case WPS_AUTHTYPE_WPAPSK | WPS_AUTHTYPE_WPA2PSK:
				nvram_set("wlan0_ssid0_auth_mode","WPA_AUTO_PSK");
				tmp = "WPA-PSK";
				break;
			case WPS_AUTHTYPE_WPA:
			case WPS_AUTHTYPE_WPA2:
				if (nwKey_length)
					tmp = "WPA-PSK WPA-EAP";
				else
					tmp = "WPA-EAP";
				break;
			default:
				tmp = 0;
				break;
			}
			if (tmp) {
				snprintf(tmpbuf, sizeof(tmpbuf), 
                                        "wpa_key_mgmt=%s\n", tmp);
                                WPS_CHANGE_WRITE(NULL);
                        }

			/* wpa_pairwise */
			switch (encr) {
			case WPS_ENCRTYPE_TKIP:
                            if(wps_configured) 
				tmp = "TKIP";
                                /* preserve mixed mode! */
                                else if ((conf->wpa_pairwise & WPA_CIPHER_CCMP) != 0)
                                    tmp = "CCMP TKIP";
				nvram_set("wlan0_ssid0_wpa_cipher","TKIP");
				break;
			case WPS_ENCRTYPE_AES:
                            if(wps_configured)
				tmp = "CCMP";
                                /* preserve mixed mode! */
                                else if ((conf->wpa_pairwise & WPA_CIPHER_TKIP) != 0)
                                    tmp = "CCMP TKIP";
				nvram_set("wlan0_ssid0_wpa_cipher","AES");
				break;
			case WPS_ENCRTYPE_AES | WPS_ENCRTYPE_TKIP:
				nvram_set("wlan0_ssid0_wpa_cipher","AES");
			        tmp = "CCMP TKIP";
				break;
			default:
				tmp = 0;
				break;
			}
			if (tmp) {
				snprintf(tmpbuf, sizeof(tmpbuf),
                                        "wpa_pairwise=%s\n", tmp);
                                WPS_CHANGE_WRITE(NULL);
                        }
		}

		/* wep_tx_keyidx */
		if (encr == WPS_ENCRTYPE_WEP) {
                        int ikey;
			printf("\nnwKeyIdx=%d nwKey_length=%d\n",nwKeyIdx,nwKey_length);                        
			int k;
			for (k = 0; k < nwKey_length; k++)
				{
					printf("%02x",nwKey[k]);
					}

			for (k = 0; k < nwKey_length; k++)
				{
					printf("%c",nwKey[k]);
					}
					
					
			snprintf(tmpbuf, sizeof(tmpbuf), 
                                "wep_default_key=%d\n", nwKeyIdx - 1);
                        WPS_CHANGE_WRITE(NULL);
				char wps_tmp[2]={0};
				char tmp_name[32]={0};
				char tmp_value[64]={0};
				sprintf(wps_tmp,"%d",nwKeyIdx);
				sprintf(tmp_name,"wlan0_ssid0_key%d",nwKeyIdx);
				nvram_set("wlan0_ssid0_default_key",wps_tmp);
			  for (k = 0; k < nwKey_length; k++)
				{
					tmp_value[k]=(char *)(nwKey[k]);
					}
				tmp_value[nwKey_length]=0;
				
				if(nwKey_length == 10)
					{
					nvram_set("wlan0_ssid0_auth_mode","OPEN_SYSTEM");
					nvram_set("wlan0_ssid0_key_length","40");
					nvram_set(tmp_name,tmp_value);						
						}
				else if(nwKey_length == 26)
					{
				 nvram_set("wlan0_ssid0_auth_mode","OPEN_SYSTEM");
				 nvram_set("wlan0_ssid0_key_length","104");		
				 nvram_set(tmp_name,tmp_value);							
						}
							
						
				
                        if (nwKey == NULL) {
				snprintf(tmpbuf, sizeof(tmpbuf), 
                                        "wep_key%d=\n", nwKeyIdx - 1);
                        } else if (is_hex(nwKey, nwKey_length)) {
				int i;
				snprintf(tmpbuf, sizeof(tmpbuf), 
                                        "wep_key%d=", nwKeyIdx - 1);


				for (i = 0; i < nwKey_length; i++)
					snprintf(tmpbuf+strlen(tmpbuf),
                                            sizeof(tmpbuf)-strlen(tmpbuf),
                                            "%02X", nwKey[i]);
				
				snprintf(tmpbuf+strlen(tmpbuf), 
                                        sizeof(tmpbuf)-strlen(tmpbuf), "\n");

			} else if ((5 == nwKey_length) || (13 == nwKey_length)) {
				snprintf(tmpbuf, sizeof(tmpbuf), 
                                        "wep_key%d=\"%s\"\n", 
                                        nwKeyIdx - 1, nwKey);
			} else {
				snprintf(tmpbuf, sizeof(tmpbuf), 
                                        "wep_key%d=%s\n", nwKeyIdx - 1, nwKey);
                        }
                        WPS_CHANGE_WRITE(NULL);

                        /* To be sure: invalidate other keys */
                        for (ikey = 0; ikey < 4; ikey++) {
                                if (ikey != nwKeyIdx-1) {
				        snprintf(tmpbuf, sizeof(tmpbuf), 
                                                "wep_key%d=\n", ikey);
                                WPS_CHANGE_WRITE(NULL);
                        }
                        }
		}

		/* psk */
		switch (auth) {
		case WPS_AUTHTYPE_WPA:
		case WPS_AUTHTYPE_WPA2:
		case WPS_AUTHTYPE_WPAPSK:
		case WPS_AUTHTYPE_WPA2PSK:
			if (64 > nwKey_length) {
				snprintf(tmpbuf, sizeof(tmpbuf), 
                                        "wpa_passphrase=%s\n", nwKey);
                                WPS_CHANGE_WRITE(NULL);
                        } else if (64 == nwKey_length) {
				snprintf(tmpbuf, sizeof(tmpbuf), 
                                        "wpa_psk=%s\n", nwKey);
                                WPS_CHANGE_WRITE(NULL);
                        }
		nvram_set("wlan0_ssid0_passphrase",nwKey);
			break;
		default:
			break;
		}
		nvram_commit();
		
                /* wps_configured -- mark as configured now! */
                /* wps_configured == 1 --> wps_state == WPS_WPSSTATE_CONFIGURED */

                        /* Mark ourselves as configured... this takes effect
                         * when we reread the configuration file.
                         */
		        snprintf(tmpbuf, sizeof(tmpbuf), "wps_configured=1\n");
                        WPS_CHANGE_WRITE(NULL);

                #ifdef WPS_WRITE_CHANGES_ONLY   /* the original Sony code */
                if (fflush(f)) {
                    break;
                }
                #else
                if (config_rewrite_write(rewrite, filename)) {
                    break;
                }

                #ifdef ATH_SINGLE_CFG

                /*
                 * Now that the wps transaction is complete
                 * and wps_configured is set to 1, write the
                 * new wps cfg file to nvram
                 */

                sprintf(tmpbuf,"cfg wpssave %s", filename);
                system(tmpbuf);

                #endif /* #ifdef ATH_SINGLE_CFG */

                #endif /* #ifdef WPS_WRITE_CHANGES_ONLY */

		ret = 0;
	} while (0);

        #ifdef WPS_WRITE_CHANGES_ONLY   /* the original Sony code */
	if (f)
		fclose(f);
        #else
        config_rewrite_free(rewrite);
        #endif

	(void)wps_destroy_wps_data(&wps);

	if (nwKey)
		os_free(nwKey);
	if (eapType)
		os_free(eapType);
	if (eapIdentity)
		os_free(eapIdentity);

	return ret;
}


/* Add Atheros WPS extension to end of WPS message 
 * (M1, M2, M2D or M8-encrypt) or WPS i.e. within beacon or probe response.
 * per Atheros WPS Extensions Specification version 0.2.
 */
int wps_config_add_atheros_wps_ext(
        struct hostapd_data *hapd, 
	struct wps_data *wps)
{
        int ret = 1;
        u8 *buf = NULL;
        u8 *bp;
        int bufsize = 64;
        static const u8 atheros_smi_oui[3] = {
                0x00, 0x24, 0xe2 
        };
        int extended_association = 1;
        int device_type_flags = 0x0001; /* or of: 1=ap, 2=sta, 4=repeater */
        int device_type = 0x0001; /* one of: 1=ap, 2=sta, 4=repeater */
        struct wps_config *conf = hapd->conf->wps;

        if (!conf || conf->wps_disable || !conf->atheros_extension)
                return 0;

        if (conf->atheros_device_type_flags) 
                device_type_flags = conf->atheros_device_type_flags;
        if (conf->atheros_device_type) 
                device_type = conf->atheros_device_type;

        buf = os_zalloc(bufsize);
        if (buf == NULL)
                return 1;
        memcpy(buf, atheros_smi_oui, sizeof(atheros_smi_oui));
        bp = buf + sizeof(atheros_smi_oui);

        /* id 0x6002 length 1 "Extended Association"
         * which is:
         *      1 == EAPOL-START is protected if
         *              it has "WiFi_Protected_Setup"
         *              tacked onto the end of it.
         *      0 = not
         * This is only specified as required for beacons
         * and probe responses... for simplicity, just
         * do it always?
         */
        *bp++ = 0x60;
        *bp++ = 0x02;   /* id 0x6002 */
        *bp++ = 0x00;
        *bp++ = 0x01;   /* length 1 */
        *bp++ = extended_association;

        /* id 0x6000 length 2 "Device Type Flags"
         * This is supported device types:
         *              0x0001 -- access point
         *              0x0002 -- station
         *              0x0004 -- repeater
         * Value for now is 0x0001 == Access Point.
         * TODO: to support repeaters etc.
         * this will have to be variable
         */
        *bp++ = 0x60;
        *bp++ = 0x00;   /* id 0x6000 */
        *bp++ = 0x00;
        *bp++ = 0x02;   /* length 2 */
        *bp++ = (device_type_flags >> 8);
        *bp++ = device_type_flags;

        /* id 0x6001 length 2 "Device Type"
         * This is same as Device Type Flags
         * but with only 1 bit set, giving current
         * state...
         * for now, is 0x0001 == Access Point.
         * TODO: to support repeaters etc.
         * this will have to be variable
         */
        *bp++ = 0x60;
        *bp++ = 0x01;   /* id 0x6001 */
        *bp++ = 0x00;
        *bp++ = 0x02;   /* length 2 */
        *bp++ = (device_type >> 8);
        *bp++ = device_type;

        ret = wps_set_value(wps, WPS_TYPE_VENDOR_EXT, buf, (bp-buf));
        free(buf);
        return ret;
}

#define IE_CON_FOR_WIN_LEGACY_STA "\xdd\x05\x00\x50\xf2\x05\x00"
#define IE_LEN_FOR_WIN_LEGACY_STA 7


int wps_config_create_beacon_ie(void *hapd, u8 **buf, size_t *len)
{
	int ret = -1;
	struct hostapd_bss_config *conf;
	struct wps_config *wps;
	struct wps_data *wps_ie;
	struct hostapd_bss_config *bss = ((struct hostapd_data *)hapd)->conf;
	struct hostapd_ssid *ssid = &bss->ssid;
	u8 u8val;
	size_t length;

	do {
		if (!hapd || !buf || !len)
			break;

		*buf = 0;
		*len = 0;

		conf = ((struct hostapd_data *)hapd)->conf;
		if (!conf)
			break;


		wps = conf->wps;
		if (!wps)
			break;

		if (wps_create_wps_data(&wps_ie))
			break;

		/* Version */
		if (!wps->version)
			u8val = WPS_VERSION;
		else
			u8val = wps->version;
		if (wps_set_value(wps_ie, WPS_TYPE_VERSION, &u8val, 0))
			break;

		/* Wi-Fi Protected Setup State */
		if (wps_set_value(wps_ie, WPS_TYPE_WPSSTATE, &wps->wps_state, 0))
			break;

		if (wps->ap_setup_locked) {
			/* AP Setup Locked */
			if (wps_set_value(wps_ie, WPS_TYPE_AP_SETUP_LOCKED, &wps->ap_setup_locked, 0))
				break;
		}

		if (wps->selreg) {
			/* Selected Registrar */
			if (wps_set_value(wps_ie, WPS_TYPE_SEL_REGISTRAR, &wps->selreg, 0))
				break;

			/* Device Password ID */
			if (wps_set_value(wps_ie, WPS_TYPE_DEVICE_PWD_ID, &wps->dev_pwd_id, 0))
				break;

			/* Selected Registrar Config Methods */
			if (wps_set_value(wps_ie, WPS_TYPE_SEL_REG_CFG_METHODS, &wps->selreg_config_methods, 0))
				break;
		}
		/* UUID-E is optional for single band but required for 
                   dual band in beacon */
		if (!wps->uuid_set) {
                        wpa_printf(MSG_ERROR, "Missing uuid from configuration");
                } else {
		if (wps_set_value(wps_ie, WPS_TYPE_UUID_E, wps->uuid, sizeof(wps->uuid)))
			break;
                }
		/* RF Bands is optional for single band but required for
                   dual band in beacon */
		if (wps_set_value(wps_ie, WPS_TYPE_RF_BANDS, &wps->rf_bands, 0))
			break;

                /* Atheros extensions */
                if (wps_config_add_atheros_wps_ext(hapd, wps_ie)) 
                        break;
		length = 0;
		if (wps_write_wps_ie(wps_ie, buf, &length))
			break;
		*len += length;

		if (!(bss->wpa & (HOSTAPD_WPA_VERSION_WPA|HOSTAPD_WPA_VERSION_WPA2)) &&
			 ssid->wep.keys_set) {
			*buf = (u8 *)os_realloc(*buf, length + IE_LEN_FOR_WIN_LEGACY_STA);
			if (!*buf)
				break;
			os_memcpy(*buf + length, IE_CON_FOR_WIN_LEGACY_STA, IE_LEN_FOR_WIN_LEGACY_STA);
			*len += IE_LEN_FOR_WIN_LEGACY_STA;
		}

		ret = 0;
	} while (0);

	if (ret) {
		if (buf && *buf) {
			os_free(*buf);
			*buf = 0;
		}
		if (len)
			*len = 0;
	}
	return ret;
}


int wps_config_create_probe_resp_ie(void *hapd, u8 **buf, size_t *len)
{
	int ret = -1;
	struct hostapd_bss_config *conf;
	struct wps_config *wps;
	struct wps_data *wps_ie = 0;
	struct hostapd_ssid *ssid;
	u8 u8val;
	size_t length;

	do {
		if (!hapd || !buf || !len)
			break;

		*buf = 0;
		*len = 0;


		conf = ((struct hostapd_data *)hapd)->conf;
		if (!conf)
			break;

	        ssid = &conf->ssid;

		wps = conf->wps;
		if (!wps)
			break;

		if (wps_create_wps_data(&wps_ie))
			break;

		/* Version */
		if (!wps->version)
			u8val = WPS_VERSION;
		else
			u8val = wps->version;
		if (wps_set_value(wps_ie, WPS_TYPE_VERSION, &u8val, 0))
			break;

		/* Wi-Fi Protected Setup State */
		if (wps_set_value(wps_ie, WPS_TYPE_WPSSTATE, &wps->wps_state, 0))
			break;

		if (wps->ap_setup_locked) {
			/* AP Setup Locked */
			if (wps_set_value(wps_ie, WPS_TYPE_AP_SETUP_LOCKED, &wps->ap_setup_locked, 0))
				break;
		}

		if (wps->selreg) {
			/* Selected Registrar */
			if (wps_set_value(wps_ie, WPS_TYPE_SEL_REGISTRAR, &wps->selreg, 0))
				break;

			/* Device Password ID */
			if (wps_set_value(wps_ie, WPS_TYPE_DEVICE_PWD_ID, &wps->dev_pwd_id, 0))
				break;

			/* Selected Registrar Config Methods */
			if (wps_set_value(wps_ie, WPS_TYPE_SEL_REG_CFG_METHODS, &wps->selreg_config_methods, 0))
				break;
		}

		/* Response Type */
                /* Atheros note: the wsccmd code always used WPS_RESTYPE_AP
                 * (not by that name of course).  The Sony code is problematic
                 * because reg_mode is so poorly defined and controlled.
                 * The WPS spec says that in certain
                 * cases the AP should should respond to certain probe 
                 * requests by putting the "info only" value into the
                 * probe response, but this would require putting this
                 * functionality into the driver... and it seems to work
                 * without that.
                 */
                #if 0   /* original from Sony */
		if (WPS_AP_REGMODE_NONE_GET_CONF == wps->reg_mode)
			u8val = WPS_RESTYPE_ENROLLEE_INFO_ONLY;
		else
                #endif  /* end original from sony */
			u8val = WPS_RESTYPE_AP;
		if (wps_set_value(wps_ie, WPS_TYPE_RESP_TYPE, &u8val, 0))
			break;

		/* UUID-E */
		if (!wps->uuid_set) {
                        wpa_printf(MSG_ERROR, "Missing uuid from configuration");
			break;
                } 

		if (wps_set_value(wps_ie, WPS_TYPE_UUID_E, wps->uuid, sizeof(wps->uuid)))
			break;

		/* Manufacturer */
                #if WPS_HACK_PADDING() /* do NOT add padding for probe resp */
		if (wps_set_value(wps_ie, WPS_TYPE_MANUFACTURER, wps->manufacturer, strlen(wps->manufacturer)))
                #else   /* original */
		if (wps_set_value(wps_ie, WPS_TYPE_MANUFACTURER, wps->manufacturer, wps->manufacturer_len))
                #endif  /* WPS_HACK_PADDING */
			break;

		/* Model Name */
                #if WPS_HACK_PADDING() /* do NOT add padding for probe resp */
		if (wps_set_value(wps_ie, WPS_TYPE_MODEL_NAME, wps->model_name, strlen(wps->model_name)))
                #else   /* original */
		if (wps_set_value(wps_ie, WPS_TYPE_MODEL_NAME, wps->model_name, wps->model_name_len))
                #endif  /* WPS_HACK_PADDING */
			break;

		/* Model Number */
                #if WPS_HACK_PADDING() /* do NOT add padding for probe resp */
		if (wps_set_value(wps_ie, WPS_TYPE_MODEL_NUMBER, wps->model_number, strlen(wps->model_number)))
                #else   /* original */
		if (wps_set_value(wps_ie, WPS_TYPE_MODEL_NUMBER, wps->model_number, wps->model_number_len))
                #endif  /* WPS_HACK_PADDING */
			break;

		/* Serial Number */
                #if WPS_HACK_PADDING() /* do NOT add padding for probe resp */
		if (wps_set_value(wps_ie, WPS_TYPE_SERIAL_NUM, wps->serial_number, strlen(wps->serial_number)))
                #else   /* original */
		if (wps_set_value(wps_ie, WPS_TYPE_SERIAL_NUM, wps->serial_number, wps->serial_number_len))
                #endif  /* WPS_HACK_PADDING */
			break;

		/* Primary Device Type */
		if (wps_set_value(wps_ie, WPS_TYPE_PRIM_DEV_TYPE, wps->prim_dev_type, sizeof(wps->prim_dev_type)))
			break;

		/* Device Name */
                #if WPS_HACK_PADDING() /* do NOT add padding for probe resp */
		if (wps_set_value(wps_ie, WPS_TYPE_DEVICE_NAME, wps->dev_name, strlen(wps->dev_name)))
                #else   /* original */
		if (wps_set_value(wps_ie, WPS_TYPE_DEVICE_NAME, wps->dev_name, wps->dev_name_len))
                #endif  /* WPS_HACK_PADDING */
			break;

		/* Config Methods */
		if (wps_set_value(wps_ie, WPS_TYPE_CONFIG_METHODS, &wps->config_methods, 0))
			break;

		/* RF Bands */
		if (wps_set_value(wps_ie, WPS_TYPE_RF_BANDS, &wps->rf_bands, 0))
			break;
                /* Atheros extensions */
                if (wps_config_add_atheros_wps_ext(hapd, wps_ie)) 
                        break;
		length = 0;
		if (wps_write_wps_ie(wps_ie, buf, &length))
			break;
                 
		*len += length;
                /* Bug fix for 58546 */
		if (!(conf->wpa & (HOSTAPD_WPA_VERSION_WPA|HOSTAPD_WPA_VERSION_WPA2)) &&
			 ssid->wep.keys_set) {
			*buf = (u8 *)os_realloc(*buf, length + IE_LEN_FOR_WIN_LEGACY_STA);
			if (!*buf)
				break;
			os_memcpy(*buf + length, IE_CON_FOR_WIN_LEGACY_STA, IE_LEN_FOR_WIN_LEGACY_STA);
			*len += IE_LEN_FOR_WIN_LEGACY_STA;
		}

		ret = 0;
	} while (0);

	if (ret) {
		if (buf && *buf) {
			os_free(*buf);
			*buf = 0;
		}
		if (len)
			*len = 0;
	}

	return ret;
}


int wps_config_create_assoc_resp_ie(void *hapd, u8 **buf, size_t *len)
{
	int ret = -1;
	struct hostapd_bss_config *conf;
	struct wps_config *wps;
	struct wps_data *wps_ie = 0;
	u8 u8val;
	size_t length;

	do {
		if (!hapd || !buf || !len)
			break;

		*buf = 0;
		*len = 0;

		conf = ((struct hostapd_data *)hapd)->conf;
		if (!conf)
			break;

		wps = conf->wps;
		if (!wps)
			break;

		if (wps_create_wps_data(&wps_ie))
			break;

		/* Version */
		if (!wps->version)
			u8val = WPS_VERSION;
		else
			u8val = wps->version;
		if (wps_set_value(wps_ie, WPS_TYPE_VERSION, &u8val, 0))
			break;

		/* Response Type */
                /* Atheros note: the wsccmd code always used WPS_RESTYPE_AP
                 * (not by that name of course).  The Sony code is problematic
                 * because reg_mode is so poorly defined and controlled.
                 * The WPS spec says that in certain
                 * cases the AP should should respond to certain probe 
                 * requests by putting the "info only" value into the
                 * probe response, but this would require putting this
                 * functionality into the driver... and it seems to work
                 * without that.
                 */
                #if 0   /* original from Sony */
		if (WPS_AP_REGMODE_NONE_GET_CONF == wps->reg_mode)
			u8val = WPS_RESTYPE_ENROLLEE_INFO_ONLY;
		else
                #endif  /* end original from soy */
			u8val = WPS_RESTYPE_AP;
		if (wps_set_value(wps_ie, WPS_TYPE_RESP_TYPE, &u8val, 0))
			break;

		length = 0;
		if (wps_write_wps_ie(wps_ie, buf, &length))
			break;
		*len += length;

		ret = 0;
	} while (0);

	if (ret) {
		if (buf && *buf) {
			os_free(*buf);
			*buf = 0;
		}
		if (len)
			*len = 0;
	}

	return ret;
}


int wps_get_wps_ie_txt(void *hapd, u8 *ie, size_t ie_len, char *buf, size_t buf_len)
{
	int ret = -1;
	struct wps_data *data = 0;
	u8 rfbands;
	u8 uuid[SIZE_UUID];
	u8 devtype[SIZE_8_BYTES];
	size_t len;
	size_t written, total = 0;

	do {
		if (!hapd || !ie || !ie_len || !buf || !buf_len)
			break;

		if (wps_create_wps_data(&data))
			break;

		if (wps_parse_wps_ie(ie, ie_len, data))
			break;

		/* UUID-E */
		len = sizeof(uuid);
		if(!wps_get_value(data, WPS_TYPE_UUID_E, uuid, &len)) {
			written = os_snprintf(buf + total, buf_len - total, "UUID-E=");
			if (written >= 0)
				total += written;
			written = wpa_snprintf_hex_uppercase(buf + total, buf_len - total, uuid, len);
			if (written >= 0)
				total += written;
			written = os_snprintf(buf + total, buf_len - total, "\n");
			if (written >= 0)
				total += written;
		}

		/* UUID-R */
		len = sizeof(uuid);
		if(!wps_get_value(data, WPS_TYPE_UUID_R, uuid, &len)) {
			written = os_snprintf(buf + total, buf_len - total, "UUID-R=");
			if (written >= 0)
				total += written;
			written = wpa_snprintf_hex_uppercase(buf + total, buf_len - total, uuid, len);
			if (written >= 0)
				total += written;
			written = os_snprintf(buf + total, buf_len - total, "\n");
			if (written >= 0)
				total += written;
		}

		/* Primary Device Type */
		len = sizeof(devtype);
		if(!wps_get_value(data, WPS_TYPE_PRIM_DEV_TYPE, devtype, &len)) {
			u16 category_id, sub_category_id;
			char *category = 0, *sub_category = 0;

			category_id = WPA_GET_BE16(devtype);
			sub_category_id = WPA_GET_BE16(devtype + 6);
			switch (category_id) {
			case 1:		/* Computer */
				category = "Computer";
				switch (sub_category_id) {
				case 1: sub_category = "PC"; break;
				case 2: sub_category = "Server"; break;
				default: break;
				}
				break;
			case 2:		/* Input Device */
				category = "Input Device";
				break;
			case 3:		/* Printers, Scanners, Faxes and Copiers */
				category = "Printers, Scanners, Faxes and Copiers";
				switch (sub_category_id) {
				case 1: sub_category = "Printer"; break;
				case 2: sub_category = "Scanner"; break;
				default: break;
				}
				break;
			case 4:		/* Camera */
				category = "Camera";
				switch (sub_category_id) {
				case 1: sub_category = "Digital Still Camera"; break;
				default: break;
				}
				break;
			case 5:		/* Storage */
				category = "Storage";
				switch (sub_category_id) {
				case 1: sub_category = "NAS"; break;
				default: break;
				}
				break;
			case 6:		/* Network Infrastructure */
				category = "Network Infrastructure";
				switch (sub_category_id) {
				case 1: sub_category = "AP"; break;
				case 2: sub_category = "Router"; break;
				case 3: sub_category = "Switch"; break;
				default: break;
				}
				break;
			case 7:		/* Displays */
				category = "Displays";
				switch (sub_category_id) {
				case 1: sub_category = "Television"; break;
				case 2: sub_category = "Electronic Picture Frame"; break;
				case 3: sub_category = "Projector"; break;
				default: break;
				}
				break;
			case 8:		/* Multimedia Devices */
				category = "Multimedia Devices";
				switch (sub_category_id) {
				case 1: sub_category = "DAR"; break;
				case 2: sub_category = "PVR"; break;
				case 3: sub_category = "MCX"; break;
				default: break;
				}
				break;
			case 9:		/* Gaming Devices */
				category = "Gaming Devices";
				switch (sub_category_id) {
				case 1: sub_category = "Xbox"; break;
				case 2: sub_category = "Xbox360"; break;
				case 3: sub_category = "Playstation"; break;
				default: break;
				}
				break;
			case 10:	/* Telephone */
				category = "Telephone";
				switch (sub_category_id) {
				case 1: sub_category = "Windows Mobile"; break;
				default: break;
				}
				break;
			default:
				break;
			}

			if (category) {
				written = os_snprintf(buf + total, buf_len - total, "Category=%s\n", category);
				if (written >= 0)
					total += written;
			}
			written = os_snprintf(buf + total, buf_len - total, "Device OUI=");
			if (written >= 0)
				total += written;
			written = wpa_snprintf_hex_uppercase(buf + total, buf_len - total, devtype + 2, 4);
			if (written >= 0)
				total += written;
			written = os_snprintf(buf + total, buf_len - total, "\n");
			if (written >= 0)
				total += written;
			if (sub_category) {
				written = os_snprintf(buf + total, buf_len - total, "Sub Category=%s\n", sub_category);
				if (written >= 0)
					total += written;
			}
		}

		/* RF Bands */
		if(!wps_get_value(data, WPS_TYPE_RF_BANDS, &rfbands, 0)) {
			if (rfbands) {
				written = os_snprintf(buf + total, buf_len - total, "RF Bands=%s%s%sGHz\n",
								   (rfbands & WPS_RFBAND_50GHZ)?"5.0":"",
								   (rfbands & (WPS_RFBAND_50GHZ|WPS_RFBAND_24GHZ))?",":"",
								   (rfbands & WPS_RFBAND_24GHZ)?"2.4":"");
				if (written >= 0)
					total += written;
			}
		}

		ret = 0;
	} while (0);

	(void)wps_destroy_wps_data(&data);

	if (ret)
		return -1;
	return total;
}


