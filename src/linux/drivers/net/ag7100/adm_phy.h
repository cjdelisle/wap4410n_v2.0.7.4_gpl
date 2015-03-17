/*
 * Copyright (c) 2009, Atheros Communications Inc.
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
 * 
 */

#ifndef _ADM_PHY_H
#define _ADM_PHY_H

#define ADM_PHY0_ADDR   0x10
#define ADM_PHY1_ADDR   0x11
#define ADM_PHY2_ADDR   0x12
#define ADM_PHY3_ADDR   0x13
#define ADM_PHY4_ADDR   0x14

#define ADM_VLAN_TAG_VALID                   0x81
#define ADM_VLAN_TAG_SIZE                    4
#define ADM_VLAN_TAG_OFFSET                  12   /* After DA & SA */

/*****************/
/* PHY Registers */
/*****************/
#define ADM_PHY_CONTROL                 0
#define ADM_PHY_STATUS                  1
#define ADM_PHY_ID1                     2
#define ADM_PHY_ID2                     3
#define ADM_AUTONEG_ADVERT              4
#define ADM_LINK_PARTNER_ABILITY        5
#define ADM_AUTONEG_EXPANSION           6


/* ADM_PHY_CONTROL fields */
#define ADM_CTRL_SOFTWARE_RESET                    0x8000
#define ADM_CTRL_SPEED_100                         0x2000
#define ADM_CTRL_AUTONEGOTIATION_ENABLE            0x1000
#define ADM_CTRL_START_AUTONEGOTIATION             0x0200
#define ADM_CTRL_SPEED_FULL_DUPLEX                 0x0100

/* Phy status fields */
#define ADM_STATUS_AUTO_NEG_DONE                   0x0020
#define ADM_STATUS_LINK_PASS                       0x0004

#define ADM_AUTONEG_DONE(ip_phy_status)                   \
    (((ip_phy_status) &                                  \
        (ADM_STATUS_AUTO_NEG_DONE)) ==                    \
        (ADM_STATUS_AUTO_NEG_DONE))

/* Link Partner ability */
#define ADM_LINK_100BASETX_FULL_DUPLEX       0x0100
#define ADM_LINK_100BASETX                   0x0080
#define ADM_LINK_10BASETX_FULL_DUPLEX        0x0040
#define ADM_LINK_10BASETX                    0x0020

/* Advertisement register. */
#define ADM_ADVERTISE_100FULL                0x0100
#define ADM_ADVERTISE_100HALF                0x0080  
#define ADM_ADVERTISE_10FULL                 0x0040  
#define ADM_ADVERTISE_10HALF                 0x0020  

#define ADM_ADVERTISE_ALL (ADM_ADVERTISE_10HALF | ADM_ADVERTISE_10FULL | \
                       ADM_ADVERTISE_100HALF | ADM_ADVERTISE_100FULL)
               
#define BOOL    uint32_t

BOOL adm_phySetup(int ethUnit);
int adm_phyIsUp(int ethUnit);
int adm_phyIsFullDuplex(int ethUnit);
BOOL adm_phySpeed(int ethUnit);
               
#endif
