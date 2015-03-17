#include <config.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <command.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include "ar7100_soc.h"
#include "ag7100.h"
#include "ag7100_phy.h"

#include "sc_config.h"
#ifdef _SC_CODE_
#define ATHR_STATUS_LINK_PASS                 0x0400 
#define ATHR_PHY_SPEC_STATUS             17
#endif
#if (CONFIG_COMMANDS & CFG_CMD_MII)
#include <miiphy.h>
#endif
#define ag7100_unit2mac(_unit)     ag7100_macs[(_unit)]
#define ag7100_name2mac(name)	   (strcmp(name,"eth0") ? ag7100_unit2mac(1) : ag7100_unit2mac(0))

int ag7100_miiphy_read(char *devname, unsigned char phaddr,
	       unsigned char reg, unsigned short *value);
int ag7100_miiphy_write(char *devname, unsigned char phaddr,
	        unsigned char reg, unsigned short data);

ag7100_mac_t *ag7100_macs[CFG_AG7100_NMACS];

static int
ag7100_send(struct eth_device *dev, volatile void *packet, int length)
{
    int i;
    ag7100_mac_t *mac = (ag7100_mac_t *)dev->priv;

    ag7100_desc_t *f = mac->fifo_tx[mac->next_tx];

#if defined(CFG_ATHRS26_PHY) && defined(CFG_ATHRHDR_EN)
    uint8_t *pkt_buf;

    pkt_buf = (uint8_t *) packet;

    if ((pkt_buf[1] & 0xf) != 0x5) {
        length = length + ATHRHDR_LEN;
        pkt_buf = (uint8_t *) packet - ATHRHDR_LEN;
        pkt_buf[0] = 0x10;  /* broadcast = 0; from_cpu = 0; reserved = 1; port_num = 0 */
        pkt_buf[1] = 0x80;  /* reserved = 0b10; priority = 0; type = 0 (normal) */
    }
    f->pkt_size = length;
    f->pkt_start_addr = virt_to_phys(pkt_buf);
#else
    f->pkt_size = length;
    f->pkt_start_addr = virt_to_phys(packet);
#endif
    ag7100_tx_give_to_dma(f);
    flush_cache((u32) packet, length);
    ag7100_reg_wr(mac, AG7100_DMA_TX_DESC, virt_to_phys(f));
    ag7100_reg_wr(mac, AG7100_DMA_TX_CTRL, AG7100_TXE);

    for (i = 0; i < MAX_WAIT; i++) {
        udelay(10);
        if (!ag7100_tx_owned_by_dma(f))
            break;
    }
    if (i == MAX_WAIT)
        printf("Tx Timed out\n");

    f->pkt_start_addr = 0;
    f->pkt_size = 0;

    if (++mac->next_tx >= NO_OF_TX_FIFOS)
        mac->next_tx = 0;

    return (0);
}

#ifdef _SC_LED_BLINK_SUPPOER_
static int time_compare = 0;
static int flag = 0;
static int power_led_status = LED_ON;
extern int upgrade_action;
#endif
static int ag7100_recv(struct eth_device *dev)
{
    int length;
    ag7100_desc_t *f;
    ag7100_mac_t *mac;

#ifdef _SC_LED_BLINK_SUPPOER_
	int time_now;
	int freq = CFG_HZ/2;
	
	if(!flag)
	{
		time_compare = get_ticks();
		flag = 1;
	}
	else
	{
		time_now = get_ticks();
		
		if((time_now - time_compare)/freq == 1)
		{
			if(power_led_status == LED_ON)
			{
				
				Led_Power(LED_OFF);
				if(upgrade_action)
					Led_Wlan(LED_ON);
				else
					Led_Wlan(LED_OFF);
				power_led_status = LED_OFF;
			}
			else
			{
				Led_Power(LED_ON);
				if(upgrade_action)
					Led_Wlan(LED_OFF);
				else
					Led_Wlan(LED_ON);
				power_led_status = LED_ON;
			}
			time_compare = time_now;			
		}
	}
#endif    

    mac = (ag7100_mac_t *)dev->priv;

    for (;;) {
        f = mac->fifo_rx[mac->next_rx];
        if (ag7100_rx_owned_by_dma(f))
            break;

        length = f->pkt_size;

        NetReceive(NetRxPackets[mac->next_rx] , length - 4);
        flush_cache((u32) NetRxPackets[mac->next_rx] , PKTSIZE_ALIGN);

        ag7100_rx_give_to_dma(f);

        if (++mac->next_rx >= NO_OF_RX_FIFOS)
            mac->next_rx = 0;
    }

    if (!(ag7100_reg_rd(mac, AG7100_DMA_RX_CTRL))) {
        ag7100_reg_wr(mac, AG7100_DMA_RX_DESC, virt_to_phys(f));
        ag7100_reg_wr(mac, AG7100_DMA_RX_CTRL, 1);
    }

    return (0);
}

static void ag7100_hw_start(ag7100_mac_t *mac)
{
    u32 mii_ctrl_val, isXGMII = CFG_GMII;

#ifdef CFG_MII0_RGMII
    mii_ctrl_val = 0x12;
#else
#ifdef CFG_MII0_MII
    mii_ctrl_val = 0x11;
#else
#ifdef CFG_MII0_RMII
    mii_ctrl_val = 0x13;
#endif
#endif
#endif

    ag7100_reg_wr(mac, AG7100_MAC_CFG1, (AG7100_MAC_CFG1_RX_EN |
		    AG7100_MAC_CFG1_TX_EN));

    ag7100_reg_rmw_set(mac, AG7100_MAC_CFG2, (AG7100_MAC_CFG2_PAD_CRC_EN |
		         AG7100_MAC_CFG2_LEN_CHECK));

#if 1
    ag7100_set_mac_if(mac, isXGMII);
#endif

    //ag7100_reg_wr(mac, AG7100_MAC_CFG2, 0x7135);
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_0, 0x1f00);

#if 0
    ag7100_reg_wr(mac, AR7100_MII0_CTRL, ag7100_get_mii_if());
#else
    if(mac->mac_unit == 0) {
        ar7100_reg_wr(AR7100_MII0_CTRL, mii_ctrl_val);
    } else {
        ar7100_reg_wr(AR7100_MII1_CTRL, mii_ctrl_val);
    }
#endif

    //ag7100_reg_wr(mac, AG7100_MAC_IFCTL, 0x10000);
    //ag7100_reg_wr(mac, AG7100_MAC_CFG1, 0x005);
    ag7100_reg_wr(mac, AG7100_MAC_MII_MGMT_CFG, AG7100_MGMT_CFG_CLK_DIV_20);

    ag7100_reg_rmw_set(mac, AG7100_MAC_FIFO_CFG_4, 0x3ffff);
    ag7100_reg_rmw_clear(mac, AG7100_MAC_FIFO_CFG_5, (1 << 19));
#ifdef AR9100
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_3, 0x780008);
#else
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_3, 0x400fff);
#endif
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_1, 0xfff0000);
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_2, 0x1fff);

#ifdef _SC_ENET_DEBUG_
    printf(": cfg1 %#x cfg2 %#x\n", ag7100_reg_rd(mac, AG7100_MAC_CFG1),
        ag7100_reg_rd(mac, AG7100_MAC_CFG2));
#endif
}

static void ag7100_set_mac_from_link(ag7100_mac_t *mac, int speed, int fdx)
{
    int is1000 = (speed == _1000BASET);
    int is100 = (speed == _100BASET);

    mac->speed = speed;
    mac->duplex = fdx;

    ag7100_set_mii_ctrl_speed(mac, speed);
    ag7100_set_mac_if(mac, is1000);
    ag7100_set_mac_duplex(mac, fdx);

    if (!is1000)
        ag7100_set_mac_speed(mac, is100);
    /*
     * XXX program PLL
     */
    mac->link = 1;
}

static int ag7100_check_link(ag7100_mac_t *mac)
{
    u32 link, duplex, speed, fdx, i;

#if !defined(CFG_ATHRS26_PHY) && !defined(CFG_ATHRHDR_EN)
    ag7100_phy_link(mac->mac_unit, link, duplex, speed);
    ag7100_phy_duplex(mac->mac_unit,duplex);
    ag7100_phy_speed(mac->mac_unit,speed);

    mac->link = link;
#ifdef _SC_ENET_DEBUG_
    printf("ag7100_check_link : link = %x.\n", link);
#endif
    if(!mac->link) {
        printf("%s link down\n",mac->dev->name);
        return 0;
    }
#else
     duplex = FULL;
     speed = _100BASET;
#endif

      if (speed == _1000BASET) {

#ifdef AR9100
	uint32_t shift, reg, val;
#endif

#ifndef AR9100
        i = *(volatile int *) 0xb8050004;
        i = i | (0x6 << 19);
        *(volatile int *) 0xb8050004 = i;
        udelay(100);
        *(volatile int *) 0xb8050014 = 0x1a000000;

        i = *(volatile int *) 0xb8050004;
        i = i & (~(0x3b << 19));
        *(volatile int *) 0xb8050004 = i;
        udelay(100);

        i = *(volatile int *) 0xb8050004;
        i = i | (0x3 << 20);
        *(volatile int *) 0xb8050004 = i;
        udelay(100);

        i = *(volatile int *) 0xb8050004;
        i = i & (~(0x3 << 20));
        *(volatile int *) 0xb8050004 = i;
        udelay(100);
#endif
        if(!mac->mac_unit){
            ar7100_reg_wr(AR7100_MII0_CTRL, 0x22);
	} else {
	    ar7100_reg_wr(AR7100_MII1_CTRL, 0x22);
	}

        ag7100_reg_rmw_clear(mac, AG7100_MAC_CFG2, 0xffff);
        ag7100_reg_rmw_set(mac, AG7100_MAC_CFG2, 0x7215);
        ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_3, 0x780fff);

#ifdef AR9100
#define ag7100_pll_shift(_mac)	(((_mac)->mac_unit) ? 22: 20)
#define ag7100_pll_offset(_mac)	(((_mac)->mac_unit) ? 0xb8050018 : 0xb8050014)
#define ETH_PLL_CONFIG		0xb8050004

	shift = ag7100_pll_shift(mac);
	reg = ag7100_pll_offset(mac);

	val  = ar7100_reg_rd(ETH_PLL_CONFIG);
	val &= ~(3 << shift);
	val |=  (2 << shift);
	ar7100_reg_wr(ETH_PLL_CONFIG, val);
	udelay(100);

	if (mac->mac_unit) {
        	*(volatile int *) 0xb8050018 = 0x1f000000;
	} else {
        	*(volatile int *) 0xb8050014 = 0x1a000000;
	}

	val |=  (3 << shift);
	ar7100_reg_wr(ETH_PLL_CONFIG, val);
	udelay(100);

	val &= ~(3 << shift);
	ar7100_reg_wr(ETH_PLL_CONFIG, val);
	udelay(100);

	//printf("pll reg %#x: %#x  ", reg, ar7100_reg_rd(reg));
#endif

        ag7100_reg_rmw_set(mac, AG7100_MAC_FIFO_CFG_5, (1 << 19));

        if(mac->mac_unit == 0) {
            miiphy_write(mac->dev->name, CFG_PHY_ADDR, 0x1f, 0x1);
            miiphy_write(mac->dev->name, CFG_PHY_ADDR, 0x1c, 0x3000);
            miiphy_write(mac->dev->name, CFG_PHY_ADDR, 0x1f, 0x0);
       }
    } else if (speed == _100BASET) {
#ifdef AR9100
#define ag7100_pll_shift(_mac)	(((_mac)->mac_unit) ? 22: 20)
#define ag7100_pll_offset(_mac)	(((_mac)->mac_unit) ? 0xb8050018 : 0xb8050014)
#define ETH_PLL_CONFIG		0xb8050004
	uint32_t shift, reg, val;

	shift = ag7100_pll_shift(mac);
	reg = ag7100_pll_offset(mac);

	val  = ar7100_reg_rd(ETH_PLL_CONFIG);
	val &= ~(3 << shift);
	val |=  (2 << shift);
	ar7100_reg_wr(ETH_PLL_CONFIG, val);
	udelay(100);

	if (mac->mac_unit) {
        	*(volatile int *) 0xb8050018 = 0x1f000000;
	} else {
        	*(volatile int *) 0xb8050014 = 0x13000a44;
	}

	val |=  (3 << shift);
	ar7100_reg_wr(ETH_PLL_CONFIG, val);
	udelay(100);

	val &= ~(3 << shift);
	ar7100_reg_wr(ETH_PLL_CONFIG, val);
	udelay(100);

	//printf(": pll reg %#x: %#x  ", reg, ar7100_reg_rd(reg));

#else

        i = *(volatile int *) 0xb8050004;
        i = i | (0x3 << 20);
        *(volatile int *) 0xb8050004 = i;
        udelay(100);

        *(volatile int *) 0xb8050014 = 0x13000a44;

        *(volatile int *) 0xb805001c = 0x00000909;
        udelay(100);

        i = *(volatile int *) 0xb8050004;
        i = i & (~(0x1 << 20));
        *(volatile int *) 0xb8050004 = i;
        udelay(100);

        i = *(volatile int *) 0xb8050004;
        i = i | (0x3 << 20);
        *(volatile int *) 0xb8050004 = i;
        udelay(100);

        i = *(volatile int *) 0xb8050004;
        i = i & (~(0x3 << 20));
        *(volatile int *) 0xb8050004 = i;
        udelay(100);
#endif
        ag7100_reg_rmw_clear(mac, AG7100_MAC_CFG2, 0xffff);
        ag7100_reg_rmw_set(mac, AG7100_MAC_CFG2, 0x7115);
    }
    if (mac->link && (duplex == mac->duplex) && (speed == mac->speed))
                    return 1;
    mac->duplex = duplex;
    mac->speed = speed;
    mac->link = 1;

    fdx = (duplex == FULL) ? 1 : 0;
    printf("dup %d speed %d\n", fdx, speed);

    ag7100_set_mac_duplex(mac,fdx);

    if (speed == _100BASET)
        ag7100_set_mac_speed(mac, 1);
    else if (speed == _10BASET)
        ag7100_set_mac_speed(mac, 0);

    return 1;
}

/*
 * For every command we re-setup the ring and start with clean h/w rx state
 */
static int ag7100_clean_rx(struct eth_device *dev, bd_t * bd)
{

    int i;
    ag7100_desc_t *fr;
    ag7100_mac_t *mac = (ag7100_mac_t*)dev->priv;

    if (!ag7100_check_link(mac))
        return 0;

    mac->next_rx = 0;
    for (i = 0; i < NO_OF_RX_FIFOS; i++) {
        fr = mac->fifo_rx[i];
        fr->pkt_start_addr = virt_to_phys(NetRxPackets[i]);
        flush_cache((u32) NetRxPackets[i], PKTSIZE_ALIGN);
        ag7100_rx_give_to_dma(fr);
    }

    ag7100_reg_wr(mac, AG7100_DMA_RX_DESC, virt_to_phys(mac->fifo_rx[0]));
    ag7100_reg_wr(mac, AG7100_DMA_RX_CTRL, AG7100_RXE);	/* rx start */
    udelay(1000 * 1000);


    return 1;

}

static int ag7100_alloc_fifo(int ndesc, ag7100_desc_t ** fifo)
{
    int i;
    u32 size;
    uchar *p = NULL;

    size = sizeof(ag7100_desc_t) * ndesc;
    size += CFG_CACHELINE_SIZE - 1;

    if ((p = malloc(size)) == NULL) {
        printf("Cant allocate fifos\n");
        return -1;
    }

    p = (uchar *) (((u32) p + CFG_CACHELINE_SIZE - 1) &
	   ~(CFG_CACHELINE_SIZE - 1));
    p = UNCACHED_SDRAM(p);

    for (i = 0; i < ndesc; i++)
        fifo[i] = (ag7100_desc_t *) p + i;

    return 0;
}

static int ag7100_setup_fifos(ag7100_mac_t *mac)
{
    int i;

    if (ag7100_alloc_fifo(NO_OF_TX_FIFOS, mac->fifo_tx))
        return 1;

    for (i = 0; i < NO_OF_TX_FIFOS; i++) {
        mac->fifo_tx[i]->next_desc = (i == NO_OF_TX_FIFOS - 1) ?
            virt_to_phys(mac->fifo_tx[0]) : virt_to_phys(mac->fifo_tx[i + 1]);
        ag7100_tx_own(mac->fifo_tx[i]);
    }

    if (ag7100_alloc_fifo(NO_OF_RX_FIFOS, mac->fifo_rx))
        return 1;

    for (i = 0; i < NO_OF_RX_FIFOS; i++) {
        mac->fifo_rx[i]->next_desc = (i == NO_OF_RX_FIFOS - 1) ?
            virt_to_phys(mac->fifo_rx[0]) : virt_to_phys(mac->fifo_rx[i + 1]);
    }

    return (1);
}

static void ag7100_halt(struct eth_device *dev)
{
    ag7100_mac_t *mac = (ag7100_mac_t *)dev->priv;
    ag7100_reg_wr(mac, AG7100_DMA_RX_CTRL, 0);
    while (ag7100_reg_rd(mac, AG7100_DMA_RX_CTRL));
}

unsigned char *
ag7100_mac_addr_loc(void)
{
	extern flash_info_t flash_info[];

	/* MAC address is store in the 2nd 4k of last sector */
	return ((unsigned char *)
		(KSEG1ADDR(AR7100_SPI_BASE) + (4 * 1024) +
		flash_info[0].size - (64 * 1024) /* sector_size */ ));

}

#ifdef _SC_CODE_
static void ag7100_get_ethaddr(struct eth_device *dev)
{
    unsigned char *mac = dev->enetaddr;

    int x, temp;
    char *s, *e, buf[64];

    temp = getenv_r("ethaddr", buf, sizeof(buf));
    s = (temp > 0) ? buf : NULL;

    for (x = 0; x < 6; ++x) {
        dev->enetaddr[x] = s ? simple_strtoul(s, &e, 16) : 0;
        if (s)
            s = (*e) ? e + 1 : e;
    }
    /* Use fixed address if the above address is invalid */
    if ((mac[0] == 0xff && mac[5] == 0xff) ||
    	(mac[0] == 0x00 && mac[1] == 0x00 && mac[2] == 0x00 && mac[3] == 0x00 && mac[4] == 0x00 && mac[5] == 0x00))
    {
        mac[0] = 0x00;
        mac[1] = 0x03;
        mac[2] = 0x7f;
        mac[3] = 0x09;
        mac[4] = 0x0b;
        mac[5] = 0xad;
        printf("No valid address in Flash. Using fixed address\n");
    } 
}
#else
static void ag7100_get_ethaddr(struct eth_device *dev)
{
#if 0
    int x, temp;
    char *s, *e, buf[64];

    temp = getenv_r("ethaddr", buf, sizeof(buf));
    s = (temp > 0) ? buf : NULL;

    for (x = 0; x < 6; ++x) {
        dev->enetaddr[x] = s ? simple_strtoul(s, &e, 16) : 0;
        if (s)
            s = (*e) ? e + 1 : e;
    }
#else
    unsigned char *eeprom;
    unsigned char *mac = dev->enetaddr;

    eeprom = ag7100_mac_addr_loc();

    if (strcmp(dev->name, "eth0") == 0) {
        memcpy(mac, eeprom, 6);
    } else if (strcmp(dev->name, "eth1") == 0) {
        eeprom += 6;
        memcpy(mac, eeprom, 6);
    } else {
        printf("%s: unknown ethernet device %s\n", __func__, dev->name);
        return;
    }

    /* Use fixed address if the above address is invalid */
    if (mac[0] == 0xff && mac[5] == 0xff) {
        mac[0] = 0x00;
        mac[1] = 0x03;
        mac[2] = 0x7f;
        mac[3] = 0x09;
        mac[4] = 0x0b;
        mac[5] = 0xad;
        printf("No valid address in Flash. Using fixed address\n");
    } else {
        printf("Fetching MAC Address from 0x%p\n", __func__, eeprom);
    }
#endif
}
#endif
#ifdef CONFIG_AR9100_MDIO_DEBUG
int
ag7100_dump_vsc_regs(ag7100_mac_t *mac)
{

	unsigned i;
	unsigned short v;
	char *fmt[] = {"\t", "\n"};

	printf("IEEE & Standard registers\n");
	for (i = 0; i < 0x20; i++) {
		v = 0;
		ag7100_miiphy_read(mac->dev->name, 0, i, &v);
		printf("0x%02x: 0x%04x%s", i, v, fmt[i & 1]);
	}

	printf("Extended registers\n");

	/* Enable extended register set access */
	ag7100_miiphy_write(mac->dev->name, 0, 0x1f,  0x1);
	for (i = 16; i <= 30; i++) {
		v = 0;
		ag7100_miiphy_read(mac->dev->name, 0, i, &v);
		printf("0x%02x: 0x%04x%s", i, v, fmt[i & 1]);
	}
	ag7100_miiphy_write(mac->dev->name, 0, 0x1f,  0x0);
	printf("\n");
}
#endif

#ifdef _SC_CODE_
#define TIME_OUT_VAL 0x4000
static int ag7100_check_link_real_time(ag7100_mac_t *mac)
{
    u16 link;
    int i = 0;
    
    do{
    	ag7100_miiphy_read(mac->dev->name, CFG_PHY_ADDR, ATHR_PHY_SPEC_STATUS, &link);
#ifdef _SC_ENET_DEBUG_
    	printf("ag7100_check_link_real_time : link = %x.\n", link);
#endif
    	udelay(500);
    	i++;
    }
    while((!(link&ATHR_STATUS_LINK_PASS))&&(i<TIME_OUT_VAL));	
    
    if(i==TIME_OUT_VAL)
    	return 0;
	
    return 1;
}
#endif

int ag7100_enet_initialize(bd_t * bis)
{
    struct eth_device *dev[CFG_AG7100_NMACS];
    u32 mask, mac_h, mac_l;
    int i;
#if defined(CFG_ATHRS26_PHY) && defined(CFG_SWITCH_FREQ)
    u32 pll_value;
#endif

#ifdef _SC_ENET_DEBUG_
    printf("ag7100_enet_initialize...\n");
#endif
   /* Workaround to bring the TX_EN to low */

     i = *(volatile int *) 0xb806001c ;
    *(volatile int *) 0xb806001c = (i | 0x3300);
    udelay(10 * 1000);
     i = *(volatile int *) 0xb806001c ;
    *(volatile int *) 0xb806001c = (i & 0xffffccff);
    udelay(10 * 1000);
    *(volatile int *) 0xb8070000 = 0x13;
    *(volatile int *) 0xb8070004 = 0x11;
    udelay(10 * 1000);
    *(volatile int *) 0xb9000000 = 0x0;
    *(volatile int *) 0xba000000 = 0x0;
     i = *(volatile int *) 0xb806001c ;
    *(volatile int *) 0xb806001c = (i | 0x3300);
    udelay(10 * 1000);

    for (i = 0;i < CFG_AG7100_NMACS;i++) {

        if ((dev[i] = (struct eth_device *) malloc(sizeof (struct eth_device))) == NULL) {
            puts("malloc failed\n");
            return 0;
        }

        if ((ag7100_macs[i] = (ag7100_mac_t *) malloc(sizeof (ag7100_mac_t))) == NULL) {
            puts("malloc failed\n");
            return 0;
        }

        memset(ag7100_macs[i], 0, sizeof(ag7100_macs[i]));
        memset(dev[i], 0, sizeof(dev[i]));

        sprintf(dev[i]->name, "eth%d", i);
        ag7100_get_ethaddr(dev[i]);

        ag7100_macs[i]->mac_unit = i;
        ag7100_macs[i]->mac_base = i ? AR7100_GE1_BASE : AR7100_GE0_BASE ;
        ag7100_macs[i]->dev = dev[i];

        dev[i]->iobase = 0;
        dev[i]->init = ag7100_clean_rx;
        dev[i]->halt = ag7100_halt;
        dev[i]->send = ag7100_send;
        dev[i]->recv = ag7100_recv;
        dev[i]->priv = (void *)ag7100_macs[i];

        eth_register(dev[i]);

#if defined(CFG_ATHRS26_PHY) && defined(CFG_ATHRHDR_EN)
        athrs26_reg_dev(dev[i]);
#endif
#if (CONFIG_COMMANDS & CFG_CMD_MII)
        miiphy_register(dev[i]->name, ag7100_miiphy_read, ag7100_miiphy_write);
#endif
        if(!i) {
            mask = (AR7100_RESET_GE0_MAC | AR7100_RESET_GE0_PHY |
                    AR7100_RESET_GE1_MAC | AR7100_RESET_GE1_PHY);

            ar7100_reg_rmw_set(AR7100_RESET, mask);
            udelay(1000 * 100);

            ar7100_reg_rmw_clear(AR7100_RESET, mask);
            udelay(1000 * 100);

            udelay(10 * 1000);
        }
        ag7100_hw_start(ag7100_macs[i]);
        ag7100_setup_fifos(ag7100_macs[i]);

#if defined(CFG_ATHRS26_PHY) && defined(CFG_SWITCH_FREQ)
        pll_value = ar7100_reg_rd(AR7100_CPU_PLL_CONFIG);
        mask = pll_value & ~(PLL_CONFIG_PLL_FB_MASK | PLL_CONFIG_REF_DIV_MASK);
        mask = mask | (0x64 << PLL_CONFIG_PLL_FB_SHIFT) |
            (0x5 << PLL_CONFIG_REF_DIV_SHIFT) | (1 << PLL_CONFIG_AHB_DIV_SHIFT);

        ar7100_reg_wr_nf(AR7100_CPU_PLL_CONFIG, mask);
        udelay(100 * 1000);
#endif

        ag7100_phy_setup(ag7100_macs[i]->mac_unit);
        udelay(100 * 1000);

#if defined(CFG_ATHRS26_PHY) && defined(CFG_SWITCH_FREQ)
        ar7100_reg_wr_nf(AR7100_CPU_PLL_CONFIG, pll_value);
        udelay(100 * 1000);
#endif

#ifdef _SC_ENET_DEBUG_
        {
            unsigned char *mac = dev[i]->enetaddr;

            printf("%s: %02x:%02x:%02x:%02x:%02x:%02x\n", dev[i]->name,
                   mac[0] & 0xff, mac[1] & 0xff, mac[2] & 0xff,
                   mac[3] & 0xff, mac[4] & 0xff, mac[5] & 0xff);
        }
#endif
        mac_l = (dev[i]->enetaddr[4] << 8) | (dev[i]->enetaddr[5]);
        mac_h = (dev[i]->enetaddr[0] << 24) | (dev[i]->enetaddr[1] << 16) |
            (dev[i]->enetaddr[2] << 8) | (dev[i]->enetaddr[3] << 0);

        ag7100_reg_wr(ag7100_macs[i], AG7100_GE_MAC_ADDR1, mac_l);
        ag7100_reg_wr(ag7100_macs[i], AG7100_GE_MAC_ADDR2, mac_h);

#if defined(CFG_ATHRS26_PHY) && defined(CFG_ATHRHDR_EN)
        /* if using header for register configuration, we have to     */
        /* configure s26 register after frame transmission is enabled */
    	athrs26_reg_init();
#endif

#ifdef _SC_ENET_DEBUG_
        printf("%s up\n",dev[i]->name);

		/* try to read PHY ID for ensuring we could access PHY */
		{
			unsigned long id = 0;
			unsigned short id1 = 0, id2 = 0;
			int j = 0;
			for(j;j<32;j++)
			{
				printf("phy_addr[%d] : ", j);
				ag7100_miiphy_read(dev[i]->name, j, PHY_PHYIDR1, &id1);
				ag7100_miiphy_read(dev[i]->name, j, PHY_PHYIDR2, &id2);
				
				id = (id1<<16) | id2;
				printf("phyid = %08x.\n", id);
				
				if(id == 0x4dd04e)
					break;
			}
		}
#endif

#ifdef _SC_CODE_
		if(ag7100_check_link_real_time(ag7100_macs[i]))
			printf("Link is really up.\n");
		else
			printf("Link is Down !!! \n");
#endif

#ifdef CONFIG_AR9100_MDIO_DEBUG
    ag7100_dump_vsc_regs(ag7100_macs[i]);
#endif
    }

    return 1;
}

#if (CONFIG_COMMANDS & CFG_CMD_MII)
int ag7100_miiphy_read(char *devname, unsigned char phaddr,
	       unsigned char reg, unsigned short *value)
{
    uint16_t addr = (phaddr << AG7100_ADDR_SHIFT) | reg;
    volatile int rddata;
#ifdef _SC_CODE_
	uint16_t      ii = 0;
#endif
    ag7100_mac_t *mac = ag7100_name2mac(devname);

    ag7100_reg_wr(mac, AG7100_MII_MGMT_CMD, 0x0);
    ag7100_reg_wr(mac, AG7100_MII_MGMT_ADDRESS, addr);
    ag7100_reg_wr(mac, AG7100_MII_MGMT_CMD, AG7100_MGMT_CMD_READ);

    rddata = ag7100_reg_rd(mac, AG7100_MII_MGMT_IND) & 0x1;
#ifdef _SC_CODE_
    do
    {
        rddata = ag7100_reg_rd(mac, AG7100_MII_MGMT_IND) & 0x1;
    	ii++;
    }while(rddata && (ii<5000));
    
    if(ii==5000)
    {
    	printf("time-out.....\n");    	
    }
    *value = ag7100_reg_rd(mac, AG7100_MII_MGMT_STATUS);
    ag7100_reg_wr(mac, AG7100_MII_MGMT_CMD, 0x0);    	
#else
    while (rddata) {
        rddata = ag7100_reg_rd(mac, AG7100_MII_MGMT_IND) & 0x1;
    }
    *value = ag7100_reg_rd(mac, AG7100_MII_MGMT_STATUS);
    ag7100_reg_wr(mac, AG7100_MII_MGMT_CMD, 0x0);
#endif
    return 0;
}

int ag7100_miiphy_write(char *devname, unsigned char phaddr,
	        unsigned char reg, unsigned short data)
{
    uint16_t addr = (phaddr << AG7100_ADDR_SHIFT) | reg;
    volatile int rddata;
    ag7100_mac_t *mac = ag7100_name2mac(devname);

    ag7100_reg_wr(mac, AG7100_MII_MGMT_ADDRESS, addr);
    ag7100_reg_wr(mac, AG7100_MII_MGMT_CTRL, data);

    rddata = ag7100_reg_rd(mac, AG7100_MII_MGMT_IND) & 0x1;
    while (rddata) {
        rddata = ag7100_reg_rd(mac, AG7100_MII_MGMT_IND) & 0x1;
    }
    return 0;
}

#endif		/* CONFIG_COMMANDS & CFG_CMD_MII */
