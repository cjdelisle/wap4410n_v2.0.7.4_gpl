/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright © 2003 Atheros Communications, Inc.,  All Rights Reserved.
 */

/*
 * Manage the atheros ethernet PHY.
 *
 * All definitions in this file are operating system independent!
 */
#ifdef __BDI
#include "bdi.h"
#endif

#include <config.h>
#include <linux/types.h>
#include <common.h>
#include <miiphy.h>
#include "phy.h"
#include "ar7100_soc.h"

#include "athr_phy.h"
#define MODULE_NAME "ATHRF1E"

typedef struct {
  int              is_enet_port;
  int              mac_unit;
  unsigned int     phy_addr;
}athr_phy_t;

athr_phy_t phy_info[] = {
    {is_enet_port: 1,
     mac_unit    : 0,
     phy_addr    : 0x00}
};

/* Add by Ken */
static uint16_t
ag7100_mii_read(uint32_t phybase, uint16_t phyaddr, uint16_t reg)
{
    uint16_t val;

    phy_reg_read(phybase, phyaddr, reg, &val);
    return val;
}

static void
ag7100_mii_write(uint32_t phybase, uint16_t phyaddr, uint16_t reg, uint32_t val)
{
    phy_reg_write(phybase, phyaddr, reg, val);
}
//end of Ken

static athr_phy_t *
athr_phy_find(int unit)
{
    int i;
    athr_phy_t *phy;

    for(i = 0; i < sizeof(phy_info)/sizeof(athr_phy_t); i++) {
        phy = &phy_info[i];
        
        if (phy->is_enet_port && (phy->mac_unit == unit)) 
            return phy;
    }
    
    return NULL;
}

int
athr_phy_setup(int unit)
{
    athr_phy_t *phy = athr_phy_find(unit);
    uint16_t  phyHwStatus;
    uint16_t  timeout;

    if (!phy) {
        printf(MODULE_NAME": \nNo phy found for unit %d\n", unit);
        return;
    }
    
     /*
     * After the phy is reset, it takes a little while before
     * it can respond properly.
     */

    phy_reg_write(unit, phy->phy_addr, ATHR_AUTONEG_ADVERT,
                  ATHR_ADVERTISE_ALL);

    phy_reg_write(unit, phy->phy_addr, ATHR_1000BASET_CONTROL,
                  ATHR_ADVERTISE_1000FULL);

    /* delay tx_clk */
    
#if 0 /* close this delay because CPU RGMII has done this! */
	phy_reg_write(unit, phy->phy_addr, 0x1D, 0x5);
    phy_reg_write(unit, phy->phy_addr, 0x1E, 0x100);
#else
	phy_reg_write(unit, phy->phy_addr, 0x1D, 0x5);
    phy_reg_write(unit, phy->phy_addr, 0x1E, 0x3c47);
#endif
#ifdef _LOOP_BACK_
    /* power saving mode */
    phy_reg_write(0, phy->phy_addr, 0x1D, 0x29); 
    phy_reg_write(0, phy->phy_addr, 0x1E, 0x0);
    /* Reset PHYs*/
    phy_reg_write(0, phy->phy_addr, ATHR_PHY_CONTROL, ATHR_CTRL_SOFTWARE_RESET);
#ifdef _LOOP_BACK_100M_
    phy_reg_write(0, phy->phy_addr, ATHR_PHY_CONTROL, 0xa100); 
#endif
#ifdef _LOOP_BACK_10M_
    phy_reg_write(0, phy->phy_addr, ATHR_PHY_CONTROL, 0x8100); 
#endif
#else
    /* Reset PHYs*/
    phy_reg_write(unit, phy->phy_addr, ATHR_PHY_CONTROL,
                  ATHR_CTRL_AUTONEGOTIATION_ENABLE 
                  | ATHR_CTRL_SOFTWARE_RESET);
#endif
    //KK
    //mdelay(500);

    /*
     * Wait up to 3 seconds for ALL associated PHYs to finish
     * autonegotiation.  The only way we get out of here sooner is
     * if ALL PHYs are connected AND finish autonegotiation.
     */
    timeout=20;
    for (;;) {
	phy_reg_read(unit, phy->phy_addr, ATHR_PHY_CONTROL,&phyHwStatus);

        if (ATHR_RESET_DONE(phyHwStatus)) {
	    printf(MODULE_NAME": Port %d, Neg Success\n", unit);
            break;
        }
        if (timeout == 0) {
	    printf(MODULE_NAME": Port %d, Negogiation timeout\n", unit);
            break;
        }
        if (--timeout == 0) {
	    printf(MODULE_NAME": Port %d, Negogiation timeout\n", unit);
            break;
        }
	//KK
        //mdelay(150);
    }
#ifdef _SC_ENET_DEBUG_
    printf(MODULE_NAME": unit %d phy addr %x ", unit, phy->phy_addr);
    printf(MODULE_NAME": reg0 %x\n", ag7100_mii_read(0, phy->phy_addr, 0));
#endif
}

int
athr_phy_is_up(int unit)
{
    int status;
    athr_phy_t *phy = athr_phy_find(unit);

    if (!phy) 
        return 0;
    status = ag7100_mii_read(0, phy->phy_addr, ATHR_PHY_SPEC_STATUS);

    if (status & ATHR_STATUS_LINK_PASS)
        return 1;

    return 0;
}

int
athr_phy_is_fdx(int unit)
{
    int status;
    athr_phy_t *phy = athr_phy_find(unit);
    int ii = 200;

    if (!phy) 
        return 0;
    do {
    status = ag7100_mii_read(0, phy->phy_addr, ATHR_PHY_SPEC_STATUS);
	     //KK
             //mdelay(10);
    } while((!(status & ATHR_STATUS_RESOVLED)) && --ii);
    status = !(!(status & ATHER_STATUS_FULL_DEPLEX));

    return (status);
}
int
athr_phy_speed(int unit)
{
    int status;
    athr_phy_t *phy = athr_phy_find(unit);
    int ii = 200;

    if (!phy) 
        return 0;
    do {
        status = ag7100_mii_read(0, phy->phy_addr, ATHR_PHY_SPEC_STATUS);
	    //mdelay(10);
    }while((!(status & ATHR_STATUS_RESOVLED)) && --ii);

    status = ((status & ATHER_STATUS_LINK_MASK) >> ATHER_STATUS_LINK_SHIFT);

    switch(status) {
    case 0:
        return AG7100_PHY_SPEED_10T;
    case 1:
        return AG7100_PHY_SPEED_100TX;
    case 2:
        return AG7100_PHY_SPEED_1000T;
    default:
        printf(MODULE_NAME": Unkown speed read!\n");
    }
    return -1;
}

#ifdef _LOOP_BACK_ /* loopback */
void dump_Register1(void)
{
    int status;
    athr_phy_t *phy = athr_phy_find(0);

    if (!phy) 
        return 0;
    status = ag7100_mii_read(0, phy->phy_addr, ATHR_PHY_STATUS);

	printf("status = %08x.\n", status);
	
    status = ag7100_mii_read(0, phy->phy_addr, ATHR_PHY_SPEC_STATUS);
	printf("realtime status = %08x.\n", status);
}
#endif
