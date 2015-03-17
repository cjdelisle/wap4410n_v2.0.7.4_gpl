#include <linux/stddef.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/bitops.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <net/sch_generic.h>

#include "ag7100.h"
#include "ag7100_phy.h"
#include "ag7100_trc.h"


#define SC_ETH_CTRL

/*
*   Added by MD@SC_CPUAP For ethernet connection status
*/

#ifdef SC_ETH_CTRL
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/socket.h>
#include <net/sock.h>

int ether_status = 0;
int ether_speed = 1;
int ether_duplex = 0;
int ether_ctrl = 0;  /*0x01 for speed ctrl; 0x02 for duplex ctrl*/

static struct proc_dir_entry *ethernet_status_file = NULL;
static int ethernet_status_read_proc(struct file *filp,char *buf,size_t count , loff_t *offp);
static int ethernet_status_write_proc(struct file *filp,const char *buf,size_t count , loff_t *offp);
void ag7100_mii_set_10(void);
void ag7100_mii_set_10_half(void);
void ag7100_mii_set_100(void);
void ag7100_mii_set_100_half(void);
void ag7100_mii_set_1000(void);
void ag7100_mii_set_auto(void);

static struct file_operations ethernet_status_fops=
{
	read: ethernet_status_read_proc,
    write: ethernet_status_write_proc,
};
#endif // SC_ETH_CTRL

static ag7100_mac_t *ag7100_macs[2];
static void ag7100_hw_setup(ag7100_mac_t *mac);
static void ag7100_hw_stop(ag7100_mac_t *mac);
static void ag7100_oom_timer(unsigned long data);
static int  ag7100_check_link(ag7100_mac_t *mac);
#ifdef DMA_DEBUG
static int  check_for_dma_hang(ag7100_mac_t *mac);
#endif
static int  ag7100_tx_alloc(ag7100_mac_t *mac);
static int  ag7100_rx_alloc(ag7100_mac_t *mac);
static void ag7100_rx_free(ag7100_mac_t *mac);
static void ag7100_tx_free(ag7100_mac_t *mac);
static int  ag7100_ring_alloc(ag7100_ring_t *r, int count);
static int  ag7100_rx_replenish(ag7100_mac_t *mac);
static int  ag7100_tx_reap(ag7100_mac_t *mac);
static void ag7100_ring_release(ag7100_mac_t *mac, ag7100_ring_t  *r);
static void ag7100_ring_free(ag7100_ring_t *r);
static void ag7100_tx_timeout_task(ag7100_mac_t *mac);
static void ag7100_get_default_macaddr(ag7100_mac_t *mac, u8 *mac_addr);
static int  ag7100_poll(struct net_device *dev, int *budget);
static void ag7100_buffer_free(struct sk_buff *skb);
#ifdef CONFIG_AR9100
void ag7100_dma_reset(ag7100_mac_t *mac);
int board_version;
#endif
static int  ag7100_recv_packets(struct net_device *dev, ag7100_mac_t *mac,
    int max_work, int *work_done);
static irqreturn_t ag7100_intr(int cpl, void *dev_id, struct pt_regs *regs);
static struct sk_buff * ag7100_buffer_alloc(void);

char *mii_str[2][4] = {
    {"GMii", "Mii", "RGMii", "RMii"},
    {"RGMii", "RMii", "INVL1", "INVL2"}
};
char *spd_str[] = {"10Mbps", "100Mbps", "1000Mbps"};
char *dup_str[] = {"half duplex", "full duplex"};

#define MODULE_NAME "AG7100"

/* if 0 compute in init */
int tx_len_per_ds = 0;
#if defined(CONFIG_AR9100) && defined(CONFIG_AG7100_GE1_RMII)
void  ag7100_tx_flush(ag7100_mac_t *mac);
void howl_10baset_war(ag7100_mac_t *mac);
#endif
module_param(tx_len_per_ds, int, 0);
MODULE_PARM_DESC(tx_len_per_ds, "Size of DMA chunk");

/* if 0 compute in init */
int tx_max_desc_per_ds_pkt=0;

/* if 0 compute in init */
#ifdef CONFIG_AR9100
int fifo_3 = 0x780008;
#else
int fifo_3 = 0;
#endif
module_param(fifo_3, int, 0);
MODULE_PARM_DESC(fifo_3, "fifo cfg 3 settings");

int mii0_if = AG7100_MII0_INTERFACE;
module_param(mii0_if, int, 0);
MODULE_PARM_DESC(mii0_if, "mii0 connect");

int mii1_if = AG7100_MII1_INTERFACE;
module_param(mii1_if, int, 0);
MODULE_PARM_DESC(mii1_if, "mii1 connect");
#ifndef CONFIG_AR9100
int gige_pll = 0x0110000;
#else
#define SW_PLL 0x1f000000ul
int gige_pll = 0x1a000000;
#endif
module_param(gige_pll, int, 0);
MODULE_PARM_DESC(gige_pll, "Pll for (R)GMII if");

/*
* Cfg 5 settings
* Weed out junk frames (CRC errored, short collision'ed frames etc.)
*/
int fifo_5 = 0x7ffef;
module_param(fifo_5, int, 0);
MODULE_PARM_DESC(fifo_5, "fifo cfg 5 settings");

#define addr_to_words(addr, w1, w2)  {                                 \
    w1 = (addr[0] << 24) | (addr[1] << 16) | (addr[2] << 8) | addr[3]; \
    w2 = (addr[4] << 24) | (addr[5] << 16) | 0;                        \
}

/*
 * Defines specific to this implemention
 */

#ifndef CONFIG_AG7100_LEN_PER_TX_DS
#error Please run menuconfig and define CONFIG_AG7100_LEN_PER_TX_DS
#endif

#ifndef CONFIG_AG7100_NUMBER_TX_PKTS
#error Please run menuconfig and define CONFIG_AG7100_NUMBER_TX_PKTS
#endif

#ifndef CONFIG_AG7100_NUMBER_RX_PKTS
#error Please run menuconfig and define CONFIG_AG7100_NUMBER_RX_PKTS
#endif
#define AG7100_TX_FIFO_LEN          2048
#define AG7100_TX_MIN_DS_LEN        128
#define AG7100_TX_MAX_DS_LEN        AG7100_TX_FIFO_LEN

#define AG7100_TX_MTU_LEN           1536

#define AG7100_TX_DESC_CNT           CONFIG_AG7100_NUMBER_TX_PKTS*tx_max_desc_per_ds_pkt
#define AG7100_TX_REAP_THRESH        AG7100_TX_DESC_CNT/2
#define AG7100_TX_QSTART_THRESH      4*tx_max_desc_per_ds_pkt

#define AG7100_RX_DESC_CNT           CONFIG_AG7100_NUMBER_RX_PKTS

#define AG7100_NAPI_WEIGHT           64
#define AG7100_PHY_POLL_SECONDS      2
int dma_flag = 0;
static inline int ag7100_tx_reap_thresh(ag7100_mac_t *mac)
{
    ag7100_ring_t *r = &mac->mac_txring;
#if defined(CONFIG_AR9100) && defined(CONFIG_AG7100_GE1_RMII)
    if(mac->speed_10t) 
        return (ag7100_ndesc_unused(mac, r) < 2);
    else 
#endif
    return (ag7100_ndesc_unused(mac, r) < AG7100_TX_REAP_THRESH);
}

static inline int ag7100_tx_ring_full(ag7100_mac_t *mac)
{
    ag7100_ring_t *r = &mac->mac_txring;

    ag7100_trc_new(ag7100_ndesc_unused(mac, r),"tx ring full");
    return (ag7100_ndesc_unused(mac, r) < tx_max_desc_per_ds_pkt + 2);
}

static int
ag7100_open(struct net_device *dev)
{
    unsigned int w1 = 0, w2 = 0;
    ag7100_mac_t *mac = (ag7100_mac_t *)dev->priv;
    int st;
#if defined(CONFIG_AR9100) && defined(SWITCH_AHB_FREQ)
    u32 tmp_pll, pll;
#endif

    assert(mac);

    st = request_irq(mac->mac_irq, ag7100_intr, 0, dev->name, dev);
    if (st < 0)
    {
        printk(MODULE_NAME ": request irq %d failed %d\n", mac->mac_irq, st);
        return 1;
    }
    if (ag7100_tx_alloc(mac)) goto tx_failed;
    if (ag7100_rx_alloc(mac)) goto rx_failed;

    ag7100_hw_setup(mac);
#if defined(CONFIG_AR9100) && defined(SWITCH_AHB_FREQ)
    /* 
    * Reduce the AHB frequency to 100MHz while setting up the 
    * S26 phy. 
    */
    pll= ar7100_reg_rd(AR7100_PLL_CONFIG);
    tmp_pll = pll& ~((PLL_DIV_MASK << PLL_DIV_SHIFT) | (PLL_REF_DIV_MASK << PLL_REF_DIV_SHIFT));
    tmp_pll = tmp_pll | (0x64 << PLL_DIV_SHIFT) |
        (0x5 << PLL_REF_DIV_SHIFT) | (1 << AHB_DIV_SHIFT);

    ar7100_reg_wr_nf(AR7100_PLL_CONFIG, tmp_pll);
    udelay(100*1000);
#endif

#if defined(CONFIG_ATHRS26_PHY)
    /* if using header for register configuration, we have to     */
    /* configure s26 register after frame transmission is enabled */
    if (mac->mac_unit == 1) /* wan phy */
        athrs26_reg_init();
#elif defined(CONFIG_ATHRS16_PHY)
    if (mac->mac_unit == 1) 
        athrs16_reg_init();
#endif

#ifdef CONFIG_MACH_AR7100_PB47
    if (mac->mac_unit == 0) /* wan phy */
        athr_vir_phy_setup();
    else
    	ag7100_phy_setup(mac->mac_unit);
#else
    ag7100_phy_setup(mac->mac_unit);
#endif

#if defined(CONFIG_AR9100) && defined(SWITCH_AHB_FREQ)
    ar7100_reg_wr_nf(AR7100_PLL_CONFIG, pll);
    udelay(100*1000);
#endif
    /*
    * set the mac addr
    */
    addr_to_words(dev->dev_addr, w1, w2);
    ag7100_reg_wr(mac, AG7100_GE_MAC_ADDR1, w1);
    ag7100_reg_wr(mac, AG7100_GE_MAC_ADDR2, w2);

    /*
    * phy link mgmt
    */
    init_timer(&mac->mac_phy_timer);
    mac->mac_phy_timer.data     = (unsigned long)mac;
    mac->mac_phy_timer.function = ag7100_check_link;
    ag7100_check_link(mac);
#ifdef DMA_DEBUG
    init_timer(&mac->mac_dbg_timer);
    mac->mac_dbg_timer.data     = (unsigned long)mac;
    mac->mac_dbg_timer.function = (void *)check_for_dma_hang;
    mod_timer(&mac->mac_dbg_timer, jiffies + AG7100_PHY_POLL_SECONDS*HZ);
#endif

    dev->trans_start = jiffies;

    ag7100_int_enable(mac);
    ag7100_rx_start(mac);

    ag7100_start_rx_count(mac);



    return 0;

rx_failed:
    ag7100_tx_free(mac);
tx_failed:
    free_irq(mac->mac_irq, dev);
    return 1;
}

static int
ag7100_stop(struct net_device *dev)
{
    ag7100_mac_t *mac = (ag7100_mac_t *)dev->priv;
    int flags;

    spin_lock_irqsave(&mac->mac_lock, flags);
    netif_stop_queue(dev);
    netif_carrier_off(dev);

    ag7100_hw_stop(mac);
    free_irq(mac->mac_irq, dev);

   /* 
    *  WAR for bug:32681 reduces the no of TX buffers to five from the
    *  actual number  of allocated buffers. Revert the value before freeing 
    *  them to avoid memory leak
    */
#if defined(CONFIG_AR9100) && defined(CONFIG_AG7100_GE1_RMII)
    mac->mac_txring.ring_nelem = AG7100_TX_DESC_CNT;
    mac->speed_10t = 0;
#endif

    ag7100_tx_free(mac);
    ag7100_rx_free(mac);

    del_timer(&mac->mac_phy_timer);
#ifdef DMA_DEBUG
    del_timer(&mac->mac_dbg_timer);
#endif

    spin_unlock_irqrestore(&mac->mac_lock, flags);
    /*ag7100_trc_dump();*/
    return 0;
}

static void
ag7100_hw_setup(ag7100_mac_t *mac)
{
    ag7100_ring_t *tx = &mac->mac_txring, *rx = &mac->mac_rxring;
    ag7100_desc_t *r0, *t0;
#ifdef CONFIG_AR9100 
#ifndef CONFIG_PORT0_AS_SWITCH
    if(mac->mac_unit) {
#ifdef CONFIG_DUAL_F1E_PHY
    ag7100_reg_wr(mac, AG7100_MAC_CFG1, (AG7100_MAC_CFG1_RX_EN |
        AG7100_MAC_CFG1_TX_EN|AG7100_MAC_CFG1_RX_FCTL));
#else
	 ag7100_reg_wr(mac, AG7100_MAC_CFG1, (AG7100_MAC_CFG1_RX_EN |
        AG7100_MAC_CFG1_TX_EN|AG7100_MAC_CFG1_RX_FCTL|AG7100_MAC_CFG1_TX_FCTL));
#endif
    }
    else {
	 ag7100_reg_wr(mac, AG7100_MAC_CFG1, (AG7100_MAC_CFG1_RX_EN |
        AG7100_MAC_CFG1_TX_EN|AG7100_MAC_CFG1_RX_FCTL));
   }
#else
   if(mac->mac_unit) {
    ag7100_reg_wr(mac, AG7100_MAC_CFG1, (AG7100_MAC_CFG1_RX_EN |
        AG7100_MAC_CFG1_TX_EN|AG7100_MAC_CFG1_RX_FCTL));
    }
    else {
         ag7100_reg_wr(mac, AG7100_MAC_CFG1, (AG7100_MAC_CFG1_RX_EN |
        AG7100_MAC_CFG1_TX_EN |AG7100_MAC_CFG1_RX_FCTL|AG7100_MAC_CFG1_TX_FCTL));
   }
#endif
#else
#ifdef CONFIG_MACH_AR7100_PB47
    if (mac->mac_unit == 0){ /* wan phy */ 
	ag7100_reg_wr(mac, AG7100_MAC_CFG1, (AG7100_MAC_CFG1_RX_EN |
        	AG7100_MAC_CFG1_TX_EN | AG7100_MAC_CFG1_RX_FCTL));
    } else {
        ag7100_reg_wr(mac, AG7100_MAC_CFG1, (AG7100_MAC_CFG1_RX_EN |
        AG7100_MAC_CFG1_TX_EN));
   }
#else
	ag7100_reg_wr(mac, AG7100_MAC_CFG1, (AG7100_MAC_CFG1_RX_EN |
        AG7100_MAC_CFG1_TX_EN));
#endif	
#endif

    ag7100_reg_rmw_set(mac, AG7100_MAC_CFG2, ( AG7100_MAC_CFG2_PAD_CRC_EN |
        AG7100_MAC_CFG2_LEN_CHECK));

    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_0, 0x1f00);
    /*
    * set the mii if type - NB reg not in the gigE space
    */
    ar7100_reg_wr(mii_reg(mac), mii_if(mac));
    ag7100_reg_wr(mac, AG7100_MAC_MII_MGMT_CFG, AG7100_MGMT_CFG_CLK_DIV_20);

#ifdef CONFIG_AR7100_EMULATION
    ag7100_reg_rmw_set(mac, AG7100_MAC_FIFO_CFG_4, 0x3ffff);
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_1, 0xfff0000);
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_2, 0x1fff);
#else
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_1, 0xfff0000);
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_2, 0x1fff);
    /*
    * Weed out junk frames (CRC errored, short collision'ed frames etc.)
    */
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_4, 0xffff);
#ifdef CONFIG_AR9100
    /* Drop CRC Errors and Pause Frames */
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_5, 0x7efef);
#else
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_5, 0x7ffef); 
#endif
#endif

    t0  =  &tx->ring_desc[0];
    r0  =  &rx->ring_desc[0];

    ag7100_reg_wr(mac, AG7100_DMA_TX_DESC, ag7100_desc_dma_addr(tx, t0));
    ag7100_reg_wr(mac, AG7100_DMA_RX_DESC, ag7100_desc_dma_addr(rx, r0));

    printk(MODULE_NAME ": cfg1 %#x cfg2 %#x\n", ag7100_reg_rd(mac, AG7100_MAC_CFG1),
        ag7100_reg_rd(mac, AG7100_MAC_CFG2));
}

static void
ag7100_hw_stop(ag7100_mac_t *mac)
{
    ag7100_rx_stop(mac);
    ag7100_tx_stop(mac);
    ag7100_int_disable(mac);
    /*
    * put everything into reset.
    */
#if defined(CONFIG_DUAL_F1E_PHY) || defined(CONFIG_MACH_AR7100_PB47)
    /* JK: on pb47, it appears that mac0's MDC/MDIO is routed to mac1's phy instead of mac1's MDC/MDIO.
       setting mac0's soft reset will break MDC/MDIO channel. hence, we only do it if the mac being
       stopped is mac1. */
    if(mac->mac_unit == 1)
#endif
    	ag7100_reg_rmw_set(mac, AG7100_MAC_CFG1, AG7100_MAC_CFG1_SOFT_RST);
}

/*
 * program the usb pll (misnomer) to genrate appropriate clock
 * Write 2 into control field
 * Write pll value 
 * Write 3 into control field 
 * Write 0 into control field 
 */
#ifdef CONFIG_AR9100
#define ag7100_pll_shift(_mac)      (((_mac)->mac_unit) ? 22: 20)
#define ag7100_pll_offset(_mac)     \
    (((_mac)->mac_unit) ? AR9100_ETH_INT1_CLK : \
                          AR9100_ETH_INT0_CLK)
#else
#define ag7100_pll_shift(_mac)      (((_mac)->mac_unit) ? 19: 17)
#define ag7100_pll_offset(_mac)     \
    (((_mac)->mac_unit) ? AR7100_USB_PLL_GE1_OFFSET : \
                          AR7100_USB_PLL_GE0_OFFSET)
#endif
static void
ag7100_set_pll(ag7100_mac_t *mac, unsigned int pll)
{
#ifdef CONFIG_AR9100
#define ETH_PLL_CONFIG AR9100_ETH_PLL_CONFIG
#else
#define ETH_PLL_CONFIG AR7100_USB_PLL_CONFIG
#endif 
    uint32_t shift, reg, val;

    shift = ag7100_pll_shift(mac);
    reg   = ag7100_pll_offset(mac);

    val  = ar7100_reg_rd(ETH_PLL_CONFIG);
    val &= ~(3 << shift);
    val |=  (2 << shift);
    ar7100_reg_wr(ETH_PLL_CONFIG, val);
    udelay(100);

    ar7100_reg_wr(reg, pll);

    val |=  (3 << shift);
    ar7100_reg_wr(ETH_PLL_CONFIG, val);
    udelay(100);

    val &= ~(3 << shift);
    ar7100_reg_wr(ETH_PLL_CONFIG, val);
    udelay(100);

    printk(MODULE_NAME ": pll reg %#x: %#x  ", reg, ar7100_reg_rd(reg));
}

#if defined(CONFIG_AR9100) && defined(CONFIG_AG7100_GE1_RMII)


/* 
 * Flush from tail till the head and free all the socket buffers even if owned by DMA
 * before we change the size of the ring buffer to avoid memory leaks and reset the ring buffer.
 * 
 * WAR for Bug: 32681 
 */

void
ag7100_tx_flush(ag7100_mac_t *mac)
{
    ag7100_ring_t   *r     = &mac->mac_txring;
    int              head  = r->ring_nelem , tail = 0, flushed = 0, i;
    ag7100_desc_t   *ds;
    ag7100_buffer_t *bf;
    uint32_t    flags;


    ar7100_flush_ge(mac->mac_unit);

    while(flushed != head)
    {
        ds   = &r->ring_desc[tail];

        bf      = &r->ring_buffer[tail];
        if(bf->buf_pkt) {
            for(i = 0; i < bf->buf_nds; i++)
            {
                ag7100_intr_ack_tx(mac);
                ag7100_ring_incr(tail);
            }
        
            ag7100_buffer_free(bf->buf_pkt);
            bf->buf_pkt = NULL;
        } 
        else
            ag7100_ring_incr(tail);

        ag7100_tx_own(ds);
        flushed ++;
    }
    r->ring_head = r->ring_tail = 0;

    return;
}

/*
 * Work around to recover from Tx failure when connected to 10BASET.
 * Bug: 32681. 
 *
 * After AutoNeg to 10Mbps Half Duplex, under some un-identified circumstances
 * during the init sequence, the MAC is in some illegal state
 * that stops the TX and hence no TXCTL to the PHY. 
 * On Tx Timeout from the software, the reset sequence is done again which recovers the 
 * MAC and Tx goes through without any problem. 
 * Instead of waiting for the application to transmit and recover, we transmit 
 * 40 dummy Tx pkts on negogiating as 10BASET.
 * Reduce the number of TX buffers from 40 to 5 so that in case of TX failures we do
 * a immediate reset and retrasmit again till we successfully transmit all of them. 
 */

void
howl_10baset_war(ag7100_mac_t *mac)
{

    struct sk_buff *dummy_pkt;
    struct net_device *dev = mac->mac_dev;
    ag7100_desc_t *ds;
    ag7100_ring_t *r;
    int i=6;
   
    /*
     * Create dummy packet 
     */ 
    dummy_pkt = dev_alloc_skb(64);
    skb_put(dummy_pkt, 60);
    atomic_dec(&dummy_pkt->users);
    while(--i >= 0) {
        dummy_pkt->data[i] = 0xff;
    }
    ag7100_get_default_macaddr(mac,(dummy_pkt->data + 6));
    dummy_pkt->dev = dev;
    i = 40;

   /* 
    *  Reduce the no of TX buffers to five from the actual number
    *  of allocated buffers and link the fifth descriptor to first.
    *  WAR for Bug:32681 to cause early Tx Timeout in 10BASET.
    */
    ag7100_tx_flush(mac);
    ds = mac->mac_txring.ring_desc;
    r = &mac->mac_txring;
    r->ring_nelem = 5;
    ds[r->ring_nelem - 1].next_desc = ag7100_desc_dma_addr(r, &ds[0]);
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_3, 0x300020);

    mac->speed_10t = 1;
    while(i-- && mac->speed_10t) {
        netif_carrier_on(dev);
        netif_start_queue(dev);

        mdelay(100);
        ag7100_hard_start(dummy_pkt,dev); 

        netif_carrier_off(dev);
        netif_stop_queue(dev);
    }
    return ;
}
#endif
	
/*
 * Several fields need to be programmed based on what the PHY negotiated
 * Ideally we should quiesce everything before touching the pll, but:
 * 1. If its a linkup/linkdown, we dont care about quiescing the traffic.
 * 2. If its a single gigE PHY, this can only happen on lup/ldown.
 * 3. If its a 100Mpbs switch, the link will always remain at 100 (or nothing)
 * 4. If its a gigE switch then the speed should always be set at 1000Mpbs, 
 *    and the switch should provide buffering for slower devices.
 *
 * XXX Only gigE PLL can be changed as a parameter for now. 100/10 is hardcoded.
 * XXX Need defines for them -
 * XXX FIFO settings based on the mode
 */
#ifdef CONFIG_ATHRS16_PHY
static int is_setup_done = 0;
#endif
static void
ag7100_set_mac_from_link(ag7100_mac_t *mac, ag7100_phy_speed_t speed, int fdx)
{
#ifdef CONFIG_ATHRS26_PHY
    int change_flag = 0;

    if(mac->mac_speed !=  speed)
        change_flag = 1;

    if(change_flag)
    {
        athrs26_phy_off(mac);
        athrs26_mac_speed_set(mac, speed);
    }
#endif
#ifdef CONFIG_ATHRS16_PHY 
    if(!is_setup_done && 
#ifndef CONFIG_PORT0_AS_SWITCH
        mac->mac_unit == 0 && 
#else
        mac->mac_unit == 1 && 
#endif
        (mac->mac_speed !=  speed || mac->mac_fdx !=  fdx)) 
    {   
       /* workaround for WAN port thru RGMII */
       phy_mode_setup();
       is_setup_done = 1;
    }
#endif
   /*
    *  Flush TX descriptors , reset the MAC and relink all descriptors.
    *  WAR for Bug:32681 
    */

#if defined(CONFIG_AR9100) && defined(CONFIG_AG7100_GE1_RMII)
    if(mac->speed_10t && (speed != AG7100_PHY_SPEED_10T)) {
        mac->speed_10t = 0;
        ag7100_tx_flush(mac);
        mdelay(500);
	ag7100_dma_reset(mac);
    }
#endif

    mac->mac_speed =  speed;
    mac->mac_fdx   =  fdx;
    ag7100_set_mii_ctrl_speed(mac, speed);
    ag7100_set_mac_duplex(mac, fdx);
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_3, fifo_3);
#ifndef CONFIG_AR9100
    ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_5, fifo_5);
#endif

    switch (speed)
    {
    case AG7100_PHY_SPEED_1000T:
#ifdef CONFIG_AR9100
        ag7100_reg_wr(mac, AG7100_MAC_FIFO_CFG_3, 0x780fff);
#endif
        ag7100_set_mac_if(mac, 1);
#ifdef CONFIG_AR9100
        if (mac->mac_unit == 0)
        { /* eth0 */
            ag7100_set_pll(mac, gige_pll);
        }
        else
        {
#ifdef CONFIG_DUAL_F1E_PHY
            ag7100_set_pll(mac, gige_pll);
#else
            ag7100_set_pll(mac, SW_PLL);
#endif
        }
#else
        ag7100_set_pll(mac, gige_pll);
#endif
        ag7100_reg_rmw_set(mac, AG7100_MAC_FIFO_CFG_5, (1 << 19));
        break;

    case AG7100_PHY_SPEED_100TX:
        ag7100_set_mac_if(mac, 0);
        ag7100_set_mac_speed(mac, 1);
#ifndef CONFIG_AR7100_EMULATION
#ifdef CONFIG_AR9100
        if (mac->mac_unit == 0)
        { /* eth0 */
            ag7100_set_pll(mac, 0x13000a44);
        }
        else
        {
#ifdef CONFIG_DUAL_F1E_PHY
            ag7100_set_pll(mac, 0x13000a44);
#else
            ag7100_set_pll(mac, SW_PLL);
#endif
        }
#else
        ag7100_set_pll(mac, 0x0001099);
#endif
#endif
        ag7100_reg_rmw_clear(mac, AG7100_MAC_FIFO_CFG_5, (1 << 19));
        break;

    case AG7100_PHY_SPEED_10T:
        ag7100_set_mac_if(mac, 0);
        ag7100_set_mac_speed(mac, 0);
#ifdef CONFIG_AR9100
        if (mac->mac_unit == 0)
        { /* eth0 */
            ag7100_set_pll(mac, 0x00441099);
        }
        else
        {
#ifdef CONFIG_DUAL_F1E_PHY
            ag7100_set_pll(mac, 0x00441099);
#else
            ag7100_set_pll(mac, SW_PLL);
#endif
        }
#else
        ag7100_set_pll(mac, 0x00991099);
#endif
#if defined(CONFIG_AR9100) && defined(CONFIG_AG7100_GE1_RMII)
        if((speed == AG7100_PHY_SPEED_10T) && !mac->speed_10t) {
           howl_10baset_war(mac);
        }
#endif
        ag7100_reg_rmw_clear(mac, AG7100_MAC_FIFO_CFG_5, (1 << 19));
        break;

    default:
        assert(0);
    }

#ifdef CONFIG_ATHRS26_PHY
    if(change_flag) 
        athrs26_phy_on(mac);
#endif

    printk(MODULE_NAME ": cfg_1: %#x\n", ag7100_reg_rd(mac, AG7100_MAC_FIFO_CFG_1));
    printk(MODULE_NAME ": cfg_2: %#x\n", ag7100_reg_rd(mac, AG7100_MAC_FIFO_CFG_2));
    printk(MODULE_NAME ": cfg_3: %#x\n", ag7100_reg_rd(mac, AG7100_MAC_FIFO_CFG_3));
    printk(MODULE_NAME ": cfg_4: %#x\n", ag7100_reg_rd(mac, AG7100_MAC_FIFO_CFG_4));
    printk(MODULE_NAME ": cfg_5: %#x\n", ag7100_reg_rd(mac, AG7100_MAC_FIFO_CFG_5));
}

#ifdef DMA_DEBUG
static void dump_tx_descs(ag7100_mac_t *mac, int tail) {
    ag7100_ring_t   *r     = &mac->mac_txring;
    ag7100_ring_t   *cr     = &mac->mac_txring_cache;
    int              head  = r->ring_head, i = tail;
    ag7100_desc_t   *ds;
    unsigned int *pds;

    while (tail != head)
    {
        ds   = &r->ring_desc[tail];
	pds = (unsigned int *)ds;
        printk(MODULE_NAME ": tx (%d) - addr 0x%x size 0x%x next addr 0x%x\n", tail, *pds, *(pds+1), *(pds+2));
        ag7100_ring_incr(tail);
    }
    printk("XXX 1 XXX\n"); 
    tail = i;
    i = 0;
    while (tail != head)
    {
        ds   = &r->ring_desc[tail];
	pds = (unsigned int *)ds;
        printk(MODULE_NAME ": tx  (%d) - addr 0x%x size 0x%x next addr 0x%x\n", tail, *pds, *(pds+1), *(pds+2));
        ds   = &cr->ring_desc[tail];
        pds = (unsigned int *)ds;
        printk(MODULE_NAME ": txc (%d) - addr 0x%x size 0x%x next addr 0x%x\n", tail, *pds, *(pds+1), *(pds+2));
        ag7100_ring_decr(tail);
	i++;
        if (i >= 40) break;
    }
    printk("XXX 2 XXX\n"); 
    return;
}

static void copy_txdescs(ag7100_mac_t *mac, int start, int end)
{
    ag7100_ring_t      *r   = &mac->mac_txring;
    ag7100_ring_t      *tr   = &mac->mac_txring_cache;
    ag7100_desc_t      *tds, *fds;

    if (end >= r->ring_nelem) end -= r->ring_nelem;
    while (start != end)
    {
        fds = &r->ring_desc[start];
        tds = &tr->ring_desc[start];
        memcpy(tds, fds, 8); /* just the first two words of the desc */
        ag7100_ring_incr(start);
    }
}

static int check_for_dma_hang(ag7100_mac_t *mac) {

    ag7100_ring_t   *r     = &mac->mac_txring;
    int              head  = r->ring_head, tail = r->ring_tail;
    ag7100_desc_t   *ds;
    ag7100_buffer_t *bp;

    ar7100_flush_ge(mac->mac_unit);

    while (tail != head)
    {
        ds   = &r->ring_desc[tail];
        bp   =  &r->ring_buffer[tail];

        if(ag7100_tx_owned_by_dma(ds)) {
            if ((jiffies - bp->trans_start) > ((1 * HZ/10))) {
                printk(MODULE_NAME ": Tx Dma status : %s\n",
                ag7100_tx_stopped(mac) ? "inactive" : "active");
#if 0
                printk(MODULE_NAME ": timestamp:%u jiffies:%u diff:%d\n",bp->trans_start,jiffies,
                             (jiffies - bp->trans_start));
#endif
                printk(MODULE_NAME ": head %d tail %d\n",head, tail);
		printk(MODULE_NAME ": tx status = 0x%x tx desc 0x%x\n", ag7100_reg_rd(mac, AG7100_DMA_TX_STATUS),
		ag7100_reg_rd(mac, AG7100_DMA_TX_DESC)); 
		dump_tx_descs(mac, tail);
		/* recover from hang by updating the tx desc pointer to next vaild one */
		ag7100_reg_wr(mac, AG7100_DMA_TX_DESC, ag7100_desc_dma_addr(r, ds));
		printk(MODULE_NAME ": tx status = 0x%x tx desc 0x%x\n", ag7100_reg_rd(mac, AG7100_DMA_TX_STATUS),
		ag7100_reg_rd(mac, AG7100_DMA_TX_DESC)); 
		ag7100_intr_ack_txurn(mac);
		ag7100_tx_start(mac);
           }
           break;
        }
        ag7100_ring_incr(tail);
    }
    mod_timer(&mac->mac_dbg_timer, jiffies + AG7100_PHY_POLL_SECONDS*HZ);
    return 0;
}
#endif

/*
 * phy link state management
 */
static int
ag7100_check_link(ag7100_mac_t *mac)
{
    struct net_device  *dev     = mac->mac_dev;
    int                 carrier = netif_carrier_ok(dev), fdx, phy_up;
    ag7100_phy_speed_t  speed;
    int                 rc;

    /* The vitesse switch uses an indirect method to communicate phy status
    * so it is best to limit the number of calls to what is necessary.
    * However a single call returns all three pieces of status information.
    * 
    * This is a trivial change to the other PHYs ergo this change.
    *
    */
    
    /*
    ** If this is not connected, let's just jump out
    */
    if(mii_if(mac) > 3)
        goto done;

    rc = ag7100_get_link_status(mac->mac_unit, &phy_up, &fdx, &speed);
    if (rc < 0)
        goto done;

    if (!phy_up)
    {
        if (carrier)
        {
            printk(MODULE_NAME ": unit %d: phy not up carrier %d\n", mac->mac_unit, carrier);
/*
* Added by MD@SC_CPUAP For ethernet connection status
*/
#ifdef SC_ETH_CTRL
			ether_status = 0;
#endif			
/* Added end */

            netif_carrier_off(dev);
            netif_stop_queue(dev);
        }
        goto done;
    }

    /*
    * phy is up. Either nothing changed or phy setttings changed while we 
    * were sleeping.
    */

    if ((fdx < 0) || (speed < 0))
    {
        printk(MODULE_NAME ": phy not connected?\n");
        return 0;
    }

#ifdef SC_ETH_CTRL
    if(ether_ctrl == 0)
    {
#endif
    if (carrier && (speed == mac->mac_speed) && (fdx == mac->mac_fdx)) 
        goto done;

/*
*   Added by MD@SC_CPUAP For ethernet connection status
*/
#ifdef SC_ETH_CTRL
        ether_status = 1;
        ether_speed = speed;
        ether_duplex = fdx;
    }
    else
    {
        if (carrier &&
            ( !(ether_ctrl&0x01) || ether_speed == mac->mac_speed) &&
            ( !(ether_ctrl&0x02) || ether_duplex == mac->mac_fdx)){
            goto done;
        }
        ether_status = 1;
        speed = ether_speed;
        fdx = ether_duplex;
    }
#endif // SC_ETH_CTRL

    printk(MODULE_NAME ": unit %d phy is up...%d", mac->mac_unit, mii_if(mac));
    printk("%s %s %s\n", mii_str[mac->mac_unit][mii_if(mac)], 
        spd_str[speed], dup_str[fdx]);

    ag7100_set_mac_from_link(mac, speed, fdx);

    printk(MODULE_NAME ": done cfg2 %#x ifctl %#x miictrl %#x \n", 
        ag7100_reg_rd(mac, AG7100_MAC_CFG2), 
        ag7100_reg_rd(mac, AG7100_MAC_IFCTL),
        ar7100_reg_rd(mii_reg(mac)));
    /*
    * in business
    */
    netif_carrier_on(dev);
    netif_start_queue(dev);

done:
    mod_timer(&mac->mac_phy_timer, jiffies + AG7100_PHY_POLL_SECONDS*HZ);

    return 0;
}

static void
ag7100_choose_phy(uint32_t phy_addr)
{
#ifdef CONFIG_AR7100_EMULATION
    if (phy_addr == 0x10)
    {
        ar7100_reg_rmw_set(AR7100_MII0_CTRL, (1 << 6));
    }
    else
    {
        ar7100_reg_rmw_clear(AR7100_MII0_CTRL, (1 << 6));
    }
#endif
}

uint16_t
ag7100_mii_read(int unit, uint32_t phy_addr, uint8_t reg)
{
    ag7100_mac_t *mac   = ag7100_unit2mac(0);
    uint16_t      addr  = (phy_addr << AG7100_ADDR_SHIFT) | reg, val;
    volatile int           rddata;
    uint16_t      ii = 0x1000;

    ag7100_choose_phy(phy_addr);

    ag7100_reg_wr(mac, AG7100_MII_MGMT_CMD, 0x0);
    ag7100_reg_wr(mac, AG7100_MII_MGMT_ADDRESS, addr);
    ag7100_reg_wr(mac, AG7100_MII_MGMT_CMD, AG7100_MGMT_CMD_READ);

    do
    {
        udelay(5);
        rddata = ag7100_reg_rd(mac, AG7100_MII_MGMT_IND) & 0x1;
    }while(rddata && --ii);

    val = ag7100_reg_rd(mac, AG7100_MII_MGMT_STATUS);
    ag7100_reg_wr(mac, AG7100_MII_MGMT_CMD, 0x0);

    return val;
}

void
ag7100_mii_write(int unit, uint32_t phy_addr, uint8_t reg, uint16_t data)
{
    ag7100_mac_t *mac   = ag7100_unit2mac(0);
    uint16_t      addr  = (phy_addr << AG7100_ADDR_SHIFT) | reg;
    volatile int rddata;
    uint16_t      ii = 0x1000;

    ag7100_choose_phy(phy_addr);

    ag7100_reg_wr(mac, AG7100_MII_MGMT_ADDRESS, addr);
    ag7100_reg_wr(mac, AG7100_MII_MGMT_CTRL, data);

    do
    {
        rddata = ag7100_reg_rd(mac, AG7100_MII_MGMT_IND) & 0x1;
    }while(rddata && --ii);
}

#ifdef SC_ETH_CTRL
#define BIT_ENABLE(reg, off) (reg | (1<<off))
#define BIT_DISABLE(reg, off) (reg & ~(1<<off))
void ag7100_mii_set_10(void)
{
    unsigned int reg = 0;

    /* disable 1000full advertise */
    reg = phy_reg_read(0, 0, ATHR_1000BASET_CONTROL);
    reg = BIT_DISABLE(reg, 9); // Disable 1000BASE-T FULL DUPLEX
    phy_reg_write(0, 0, ATHR_1000BASET_CONTROL,reg);

    /* enable 10full only */
    reg = phy_reg_read(0, 0, ATHR_AUTONEG_ADVERT);
    reg = BIT_DISABLE(reg, 5); // Disable 10BASE-TX HALF DUPLEX
    reg = BIT_ENABLE(reg, 6); // Enable 10BASE-TX FULL DUPLEX
    reg = BIT_DISABLE(reg, 7); // Disable 100BASE-TX HALF DUPLEX
    reg = BIT_DISABLE(reg, 8); // Disable 100BASE-TX FULL DUPLEX
	phy_reg_write(0, 0, ATHR_AUTONEG_ADVERT, reg);

}

void ag7100_mii_set_10_half(void)
{
    unsigned int reg = 0;

    /* disable 1000full advertise */
    reg = phy_reg_read(0, 0, ATHR_1000BASET_CONTROL);
    reg = BIT_DISABLE(reg, 9); // Disable 1000BASE-T FULL DUPLEX
    phy_reg_write(0, 0, ATHR_1000BASET_CONTROL,reg);

    /* enable 10full only */
    reg = phy_reg_read(0, 0, ATHR_AUTONEG_ADVERT);
    reg = BIT_ENABLE(reg, 5); // Disable 10BASE-TX HALF DUPLEX
    reg = BIT_DISABLE(reg, 6); // Enable 10BASE-TX FULL DUPLEX
    reg = BIT_DISABLE(reg, 7); // Disable 100BASE-TX HALF DUPLEX
    reg = BIT_DISABLE(reg, 8); // Disable 100BASE-TX FULL DUPLEX
	phy_reg_write(0, 0, ATHR_AUTONEG_ADVERT, reg);

}

void ag7100_mii_set_100(void)
{
    unsigned int reg = 0;

    /* disable 1000full advertise */
    reg = phy_reg_read(0, 0, ATHR_1000BASET_CONTROL);
    reg = BIT_DISABLE(reg, 9); // Disable 1000BASE-T FULL DUPLEX
    phy_reg_write(0, 0, ATHR_1000BASET_CONTROL,reg);

    /* enable 100full only */
    reg = phy_reg_read(0, 0, ATHR_AUTONEG_ADVERT);
    reg = BIT_DISABLE(reg, 5); // Disable 10BASE-TX HALF DUPLEX
    reg = BIT_DISABLE(reg, 6); // Disable 10BASE-TX FULL DUPLEX
    reg = BIT_DISABLE(reg, 7); // Disable 100BASE-TX HALF DUPLEX
    reg = BIT_ENABLE(reg, 8); // Enable 100BASE-TX FULL DUPLEX
	phy_reg_write(0, 0, ATHR_AUTONEG_ADVERT, reg);

}

void ag7100_mii_set_100_half(void)
{
    unsigned int reg = 0;

    /* disable 1000full advertise */
    reg = phy_reg_read(0, 0, ATHR_1000BASET_CONTROL);
    reg = BIT_DISABLE(reg, 9); // Disable 1000BASE-T FULL DUPLEX
    phy_reg_write(0, 0, ATHR_1000BASET_CONTROL,reg);

    /* enable 100full only */
    reg = phy_reg_read(0, 0, ATHR_AUTONEG_ADVERT);
    reg = BIT_DISABLE(reg, 5); // Disable 10BASE-TX HALF DUPLEX
    reg = BIT_DISABLE(reg, 6); // Disable 10BASE-TX FULL DUPLEX
    reg = BIT_ENABLE(reg, 7); // Disable 100BASE-TX HALF DUPLEX
    reg = BIT_DISABLE(reg, 8); // Enable 100BASE-TX FULL DUPLEX
	phy_reg_write(0, 0, ATHR_AUTONEG_ADVERT, reg);

}

void ag7100_mii_set_1000(void)
{
    unsigned int reg = 0;

    /* enable 1000full advertise */
    reg = phy_reg_read(0, 0, ATHR_1000BASET_CONTROL);
    reg = BIT_ENABLE(reg, 9); // Enable 1000BASE-T FULL DUPLEX
    phy_reg_write(0, 0, ATHR_1000BASET_CONTROL,reg);

    /* disable 10/100 */
    reg = phy_reg_read(0, 0, ATHR_AUTONEG_ADVERT);
    reg = BIT_DISABLE(reg, 5); // Disable 10BASE-TX HALF DUPLEX
    reg = BIT_DISABLE(reg, 6); // Disable 10BASE-TX FULL DUPLEX
    reg = BIT_DISABLE(reg, 7); // Disable 100BASE-TX HALF DUPLEX
    reg = BIT_DISABLE(reg, 8); // Disable 100BASE-TX FULL DUPLEX
	phy_reg_write(0, 0, ATHR_AUTONEG_ADVERT, reg);

}

void ag7100_mii_set_auto(void)
{
    /* enable 10/100 advertise */
    phy_reg_write(0, 0, ATHR_AUTONEG_ADVERT,
                  ATHR_ADVERTISE_ALL);

    /* enable 1000full advertise */
    phy_reg_write(0, 0, ATHR_1000BASET_CONTROL,
                  ATHR_ADVERTISE_1000FULL);

}

/*
*   Added by MD@SC_CPUAP For ethernet connection status
*/
static int ethernet_status_read_proc(struct file *filp,char *buf,size_t count , loff_t *offp)
{
	int len=0;

	if(*offp!=0) return 0;
	len=sprintf(buf, "Status=%d\nSpeed=%s\n%s\n", ether_status, spd_str[ether_speed], dup_str[ether_duplex]);
	*offp=len;
	return len;
}
static int ethernet_status_write_proc(struct file *filp,const char *buf,size_t count , loff_t *offp)
{
    printk("start to write\n");
    sscanf(buf, "%d %d %d\n", &ether_ctrl, &ether_speed, &ether_duplex);
    printk("ether_ctrl=%d\nether_speed=%s\nether_duplex=%s\n", ether_ctrl, spd_str[ether_speed], dup_str[ether_duplex]);

    if(ether_ctrl&0x01){
        switch(ether_speed){
            case 0:
            	  if(ether_duplex == 0)
                ag7100_mii_set_10_half();
								else
                ag7100_mii_set_10();
                break;
            case 1:
            	  if(ether_duplex == 0)
                ag7100_mii_set_100_half();
								else
                ag7100_mii_set_100();
                break;
            case 2:
                ag7100_mii_set_1000();
                break;
            default:
                break;
        }
    }else{
        ag7100_mii_set_auto();
    }

    /* Reset PHYs*/
    phy_reg_write(0, 0, ATHR_PHY_CONTROL,
                  ATHR_CTRL_AUTONEGOTIATION_ENABLE
                  | ATHR_CTRL_SOFTWARE_RESET);
	return count;
}
#endif // SC_ETH_CTRL


/*
 * Tx operation:
 * We do lazy reaping - only when the ring is "thresh" full. If the ring is 
 * full and the hardware is not even done with the first pkt we q'd, we turn
 * on the tx interrupt, stop all q's and wait for h/w to
 * tell us when its done with a "few" pkts, and then turn the Qs on again.
 *
 * Locking:
 * The interrupt only touches the ring when Q's stopped  => Tx is lockless, 
 * except when handling ring full.
 *
 * Desc Flushing: Flushing needs to be handled at various levels, broadly:
 * - The DDr FIFOs for desc reads.
 * - WB's for desc writes.
 */
static void
ag7100_handle_tx_full(ag7100_mac_t *mac)
{
    u32         flags;
#if defined(CONFIG_AR9100) && defined(CONFIG_AG7100_GE1_RMII)
    if(!mac->speed_10t)
#endif
    assert(!netif_queue_stopped(mac->mac_dev));

    mac->mac_net_stats.tx_fifo_errors ++;

    netif_stop_queue(mac->mac_dev);

    spin_lock_irqsave(&mac->mac_lock, flags);
    ag7100_intr_enable_tx(mac);
    spin_unlock_irqrestore(&mac->mac_lock, flags);
}

/* ******************************
 * 
 * Code under test - do not use
 *
 * ******************************
 */

static ag7100_desc_t *
ag7100_get_tx_ds(ag7100_mac_t *mac, int *len, unsigned char **start)
{
    ag7100_desc_t      *ds;
    int                len_this_ds;
    ag7100_ring_t      *r   = &mac->mac_txring;
#ifdef DMA_DEBUG
    ag7100_buffer_t    *bp;
#endif

    /* force extra pkt if remainder less than 4 bytes */
    if (*len > tx_len_per_ds)
        if (*len <= (tx_len_per_ds + 4))
            len_this_ds = tx_len_per_ds - 4;
        else
            len_this_ds = tx_len_per_ds;
    else
        len_this_ds    = *len;

    ds = &r->ring_desc[r->ring_head];

    ag7100_trc_new(ds,"ds addr");
    ag7100_trc_new(ds,"ds len");
#ifdef CONFIG_AR9100
    if(ag7100_tx_owned_by_dma(ds))
        ag7100_dma_reset(mac);
#else
    assert(!ag7100_tx_owned_by_dma(ds));
#endif

    ds->pkt_size       = len_this_ds;
    ds->pkt_start_addr = virt_to_phys(*start);
    ds->more           = 1;

    *len   -= len_this_ds;
    *start += len_this_ds;

#ifdef DMA_DEBUG
     bp = &r->ring_buffer[r->ring_head];
     bp->trans_start = jiffies; /*Time stamp each packet */
#endif

    ag7100_ring_incr(r->ring_head);

    return ds;
}

#if defined(CONFIG_ATHRS26_PHY)
int
#else
static int
#endif
ag7100_hard_start(struct sk_buff *skb, struct net_device *dev)
{
    ag7100_mac_t       *mac = (ag7100_mac_t *)dev->priv;
    ag7100_ring_t      *r   = &mac->mac_txring;
    ag7100_buffer_t    *bp;
    ag7100_desc_t      *ds, *fds;
    unsigned char      *start;
    int                len;
    int                nds_this_pkt;

#ifdef VSC73XX_DEBUG
    {
        static int vsc73xx_dbg;
        if (vsc73xx_dbg == 0) {
            vsc73xx_get_link_status_dbg();
            vsc73xx_dbg = 1;
        }
        vsc73xx_dbg = (vsc73xx_dbg + 1) % 10;
    }
#endif

#if defined(CONFIG_ATHRS26_PHY) && defined(HEADER_EN)
    /* add header to normal frames */
    /* check if normal frames */
    if ((mac->mac_unit == 0) && (!((skb->cb[0] == 0x7f) && (skb->cb[1] == 0x5d))))
    {
        skb_push(skb, HEADER_LEN);
        skb->data[0] = 0x10; /* broadcast = 0; from_cpu = 0; reserved = 1; port_num = 0 */
        skb->data[1] = 0x80; /* reserved = 0b10; priority = 0; type = 0 (normal) */
    }

#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
    if(unlikely((skb->len <= 0) 
        || (skb->len > (dev->mtu + ETH_HLEN + HEADER_LEN + 4))))
    { /*vlan tag length = 4*/
        printk(MODULE_NAME ": [%d] bad skb, dev->mtu=%d,ETH_HLEN=%d len %d\n", mac->mac_unit, dev->mtu, ETH_HLEN,  skb->len);
        goto dropit;
    }
#else
    if(unlikely((skb->len <= 0) 
        || (skb->len > (dev->mtu + ETH_HLEN + HEADER_LEN))))
    {
        printk(MODULE_NAME ": [%d] bad skb, dev->mtu=%d,ETH_HLEN=%d len %d\n", mac->mac_unit, dev->mtu, ETH_HLEN,  skb->len);
        goto dropit;
    }
#endif  

#else
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
    if(unlikely((skb->len <= 0) || (skb->len > (dev->mtu + ETH_HLEN + 4))))
    {  /*vlan tag length = 4*/
        printk(MODULE_NAME ": bad skb, len %d\n", skb->len);
        goto dropit;
    }
#else
    if(unlikely((skb->len <= 0) || (skb->len > (dev->mtu + ETH_HLEN))))
    {
        printk(MODULE_NAME ": bad skb, len %d\n", skb->len);
        goto dropit;
    }
#endif    
#endif

    if (ag7100_tx_reap_thresh(mac)) 
        ag7100_tx_reap(mac);

    ag7100_trc_new(r->ring_head,"hard-stop hd");
    ag7100_trc_new(r->ring_tail,"hard-stop tl");

    ag7100_trc_new(skb->len,    "len this pkt");
    ag7100_trc_new(skb->data,   "ptr 2 pkt");

    dma_cache_wback((unsigned long)skb->data, skb->len);

    bp          = &r->ring_buffer[r->ring_head];
    bp->buf_pkt = skb;
    len         = skb->len;
    start       = skb->data;

    assert(len>4);

    nds_this_pkt = 1;
    fds = ds = ag7100_get_tx_ds(mac, &len, &start);

    while (len>0)
    {
        ds = ag7100_get_tx_ds(mac, &len, &start);
        nds_this_pkt++;
        ag7100_tx_give_to_dma(ds);
    }

    ds->more        = 0;
    ag7100_tx_give_to_dma(fds);

    bp->buf_lastds  = ds;
    bp->buf_nds     = nds_this_pkt;

#ifdef DMA_DEBUG
    copy_txdescs(mac, (int)((unsigned int)fds - (unsigned int)r->ring_desc)/sizeof(ag7100_desc_t), (int)((unsigned int)ds - 
(unsigned int)r->ring_desc)/sizeof(ag7100_desc_t)+1);
#endif

    ag7100_trc_new(ds,"last ds");
    ag7100_trc_new(nds_this_pkt,"nmbr ds for this pkt");

    wmb();

    mac->net_tx_packets ++;
    mac->net_tx_bytes += skb->len;

    ag7100_trc(ag7100_reg_rd(mac, AG7100_DMA_TX_CTRL),"dma idle");

    ag7100_tx_start(mac);

    if (unlikely(ag7100_tx_ring_full(mac)))
        ag7100_handle_tx_full(mac);

    dev->trans_start = jiffies;

    return NETDEV_TX_OK;

dropit:
    printk(MODULE_NAME ": dropping skb %08x\n", skb);
    kfree_skb(skb);
    return NETDEV_TX_OK;
}

/*
 * Interrupt handling:
 * - Recv NAPI style (refer to Documentation/networking/NAPI)
 *
 *   2 Rx interrupts: RX and Overflow (OVF).
 *   - If we get RX and/or OVF, schedule a poll. Turn off _both_ interurpts. 
 *
 *   - When our poll's called, we
 *     a) Have one or more packets to process and replenish
 *     b) The hardware may have stopped because of an OVF.
 *
 *   - We process and replenish as much as we can. For every rcvd pkt 
 *     indicated up the stack, the head moves. For every such slot that we
 *     replenish with an skb, the tail moves. If head catches up with the tail
 *     we're OOM. When all's done, we consider where we're at:
 *
 *      if no OOM:
 *      - if we're out of quota, let the ints be disabled and poll scheduled.
 *      - If we've processed everything, enable ints and cancel poll.
 *
 *      If OOM:
 *      - Start a timer. Cancel poll. Ints still disabled. 
 *        If the hardware's stopped, no point in restarting yet. 
 *
 *      Note that in general, whether we're OOM or not, we still try to
 *      indicate everything recvd, up.
 *
 * Locking: 
 * The interrupt doesnt touch the ring => Rx is lockless
 *
 */
static irqreturn_t
ag7100_intr(int cpl, void *dev_id, struct pt_regs *regs)
{
    struct net_device *dev  = (struct net_device *)dev_id;
    ag7100_mac_t      *mac  = (ag7100_mac_t *)dev->priv;
    int   isr, imr, handled = 0;

    isr   = ag7100_get_isr(mac);
    imr   = ag7100_reg_rd(mac, AG7100_DMA_INTR_MASK);

    ag7100_trc(isr,"isr");
    ag7100_trc(imr,"imr");

    assert(isr == (isr & imr));

    if (likely(isr & (AG7100_INTR_RX | AG7100_INTR_RX_OVF)))
    {
        handled = 1;
        if (likely(netif_rx_schedule_prep(dev)))
        {
            ag7100_intr_disable_recv(mac);
            __netif_rx_schedule(dev);
        }
        else
        {
            printk(MODULE_NAME ": driver bug! interrupt while in poll\n");
            assert(0);
            ag7100_intr_disable_recv(mac);
        }
        /*ag7100_recv_packets(dev, mac, 200, &budget);*/
    }
    if (likely(isr & AG7100_INTR_TX))
    {
        handled = 1;
        ag7100_intr_ack_tx(mac);
        ag7100_tx_reap(mac);
    }
    if (unlikely(isr & AG7100_INTR_RX_BUS_ERROR))
    {
        assert(0);
        handled = 1;
        ag7100_intr_ack_rxbe(mac);
    }
    if (unlikely(isr & AG7100_INTR_TX_BUS_ERROR))
    {
        assert(0);
        handled = 1;
        ag7100_intr_ack_txbe(mac);
    }

    if (!handled)
    {
        assert(0);
        printk(MODULE_NAME ": unhandled intr isr %#x\n", isr);
    }

    return IRQ_HANDLED;
}

 /*
  * Rx and Tx DMA hangs and goes to an invalid state in HOWL boards 
  * when the link partner is forced to 10/100 Mode.By resetting the MAC
  * we are able to recover from this state.This is a software  WAR and
  * will be removed once we have a hardware fix. 
  */

#ifdef CONFIG_AR9100

void ag7100_dma_reset(ag7100_mac_t *mac)
{
    uint32_t mask;

    if(mac->mac_unit)
        mask = AR7100_RESET_GE1_MAC;
    else
        mask = AR7100_RESET_GE0_MAC;

    ar7100_reg_rmw_set(AR7100_RESET, mask);
    mdelay(100);
    ar7100_reg_rmw_clear(AR7100_RESET, mask);
    mdelay(100);

    ag7100_intr_disable_recv(mac);
#if defined(CONFIG_AR9100) && defined(CONFIG_AG7100_GE1_RMII)
    mac->speed_10t = 0;
#endif
    schedule_work(&mac->mac_tx_timeout);
}

#endif

static int
ag7100_poll(struct net_device *dev, int *budget)
{
    ag7100_mac_t       *mac       = (ag7100_mac_t *)dev->priv;
    int work_done,      max_work  = min(*budget, dev->quota), status = 0;
    ag7100_rx_status_t  ret;
    u32                 flags;

    ret = ag7100_recv_packets(dev, mac, max_work, &work_done);

    dev->quota  -= work_done;
    *budget     -= work_done;

#ifdef CONFIG_AR9100
    if(ret == AG7100_RX_DMA_HANG)
    {
        status = 0;
        netif_rx_complete(dev);
        ag7100_dma_reset(mac);
        return status;
    }
#endif
    if (likely(ret == AG7100_RX_STATUS_DONE))
    {
        netif_rx_complete(dev);
        spin_lock_irqsave(&mac->mac_lock, flags);
        ag7100_intr_enable_recv(mac);
        spin_unlock_irqrestore(&mac->mac_lock, flags);
    }
    else if (likely(ret == AG7100_RX_STATUS_NOT_DONE))
    {
        /*
        * We have work left
        */
        status = 1;
    }
    else if (ret == AG7100_RX_STATUS_OOM)
    {
        printk(MODULE_NAME ": oom..?\n");
        /* 
        * Start timer, stop polling, but do not enable rx interrupts.
        */
        mod_timer(&mac->mac_oom_timer, jiffies+1);
        netif_rx_complete(dev);
    }

    return status;
}

static int
ag7100_recv_packets(struct net_device *dev, ag7100_mac_t *mac, 
    int quota, int *work_done)
{
    ag7100_ring_t       *r     = &mac->mac_rxring;
    ag7100_desc_t       *ds;
    ag7100_buffer_t     *bp;
    struct sk_buff      *skb;
    ag7100_rx_status_t   ret   = AG7100_RX_STATUS_DONE;
    int head = r->ring_head, len, status, iquota = quota, more_pkts, rep;

    ag7100_trc(iquota,"iquota");
#if !defined(CONFIG_AR9100)
    status = ag7100_reg_rd(mac, AG7100_DMA_RX_STATUS);
#endif

process_pkts:
    ag7100_trc(status,"status");
#if !defined(CONFIG_AR9100)
    /*
    * Under stress, the following assertion fails.
    *
    * On investigation, the following `appears' to happen.
    *   - pkts received
    *   - rx intr
    *   - poll invoked
    *   - process received pkts
    *   - replenish buffers
    *   - pkts received
    *
    *   - NO RX INTR & STATUS REG NOT UPDATED <---
    *
    *   - s/w doesn't process pkts since no intr
    *   - eventually, no more buffers for h/w to put
    *     future rx pkts
    *   - RX overflow intr
    *   - poll invoked
    *   - since status reg is not getting updated
    *     following assertion fails..
    *
    * Ignore the status register.  Regardless of this
    * being a rx or rx overflow, we have packets to process.
    * So, we go ahead and receive the packets..
    */
    assert((status & AG7100_RX_STATUS_PKT_RCVD));
    assert((status >> 16));
#endif
    /*
    * Flush the DDR FIFOs for our gmac
    */
    ar7100_flush_ge(mac->mac_unit);

    assert(quota > 0); /* WCL */

    while(quota)
    {
        ds    = &r->ring_desc[head];

        ag7100_trc(head,"hd");
        ag7100_trc(ds,  "ds");

        if (ag7100_rx_owned_by_dma(ds))
        {
#ifdef CONFIG_AR9100
            if(quota == iquota)
            {
                *work_done = quota = 0;
                return AG7100_RX_DMA_HANG;
            }
#else
            assert(quota != iquota); /* WCL */
#endif
            break;
        }
        ag7100_intr_ack_rx(mac);

        bp                  = &r->ring_buffer[head];
        len                 = ds->pkt_size;
        skb                 = bp->buf_pkt;
        assert(skb);
        skb_put(skb, len - ETHERNET_FCS_SIZE);

#if defined(CONFIG_ATHRS26_PHY) && defined(HEADER_EN)
        uint8_t type;
        uint16_t def_vid;

        if(mac->mac_unit == 0)
        {
            type = (skb->data[1]) & 0xf;

            if (type == NORMAL_PACKET)
            {
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)            	
                /*cpu egress tagged*/
                if (is_cpu_egress_tagged())
                {
                    if ((skb->data[12 + HEADER_LEN] != 0x81) || (skb->data[13 + HEADER_LEN] != 0x00))
                    {
                        def_vid = athrs26_defvid_get(skb->data[0] & 0xf);
                        skb_push(skb, 2); /* vid lenghth - header length */
                        memmove(&skb->data[0], &skb->data[4], 12); /*remove header and add vlan tag*/

                        skb->data[12] = 0x81;
                        skb->data[13] = 0x00;
                        skb->data[14] = (def_vid >> 8) & 0xf;
                        skb->data[15] = def_vid & 0xff;
                    }
                }
                else
#endif                
                    skb_pull(skb, 2); /* remove attansic header */

                mac->net_rx_packets ++;
                mac->net_rx_bytes += skb->len;
                /*
                * also pulls the ether header
                */
                skb->protocol       = eth_type_trans(skb, dev);
                skb->dev            = dev;
                bp->buf_pkt         = NULL;
                dev->last_rx        = jiffies;
                quota--;

                netif_receive_skb(skb);
            }
            else
            {
                mac->net_rx_packets ++;
                mac->net_rx_bytes += skb->len;
                bp->buf_pkt         = NULL;
                dev->last_rx        = jiffies;
                quota--;

                if (type == READ_WRITE_REG_ACK)
                {
                    header_receive_skb(skb);
                }
                else
                {
                    kfree_skb(skb);
                }
            }
        }else
        {
            mac->net_rx_packets ++;
            mac->net_rx_bytes += skb->len;
            /*
            * also pulls the ether header
            */
            skb->protocol       = eth_type_trans(skb, dev);
            skb->dev            = dev;
            bp->buf_pkt         = NULL;
            dev->last_rx        = jiffies;
            quota--;

            netif_receive_skb(skb);
        }

#else
        mac->net_rx_packets ++;
        mac->net_rx_bytes += skb->len;
        /*
        * also pulls the ether header
        */
        skb->protocol       = eth_type_trans(skb, dev);
        skb->dev            = dev;
        bp->buf_pkt         = NULL;
        dev->last_rx        = jiffies;

        quota--;

        netif_receive_skb(skb);
#endif

        ag7100_ring_incr(head);
    }

    assert(iquota != quota);
    r->ring_head   =  head;

    rep = ag7100_rx_replenish(mac);
#ifdef CONFIG_AR9100
    if(rep < 0)
    {
        *work_done =0 ;
        return AG7100_RX_DMA_HANG;
    }
#endif
    /*
    * let's see what changed while we were slogging.
    * ack Rx in the loop above is no flush version. It will get flushed now.
    */
    status       =  ag7100_reg_rd(mac, AG7100_DMA_RX_STATUS);
    more_pkts    =  (status & AG7100_RX_STATUS_PKT_RCVD);

    ag7100_trc(more_pkts,"more_pkts");

    if (!more_pkts) goto done;
    /*
    * more pkts arrived; if we have quota left, get rolling again
    */
    if (quota)      goto process_pkts;
    /*
    * out of quota
    */
    ret = AG7100_RX_STATUS_NOT_DONE;

done:
    *work_done   = (iquota - quota);

    if (unlikely(ag7100_rx_ring_full(mac))) 
        return AG7100_RX_STATUS_OOM;
    /*
    * !oom; if stopped, restart h/w
    */

    if (unlikely(status & AG7100_RX_STATUS_OVF))
    {
        mac->net_rx_over_errors ++;
        ag7100_intr_ack_rxovf(mac);
        ag7100_rx_start(mac);
    }

    return ret;
}

static struct sk_buff *
    ag7100_buffer_alloc(void)
{
    struct sk_buff *skb;

    skb = dev_alloc_skb(AG7100_RX_BUF_SIZE);
    if (unlikely(!skb))
        return NULL;
    skb_reserve(skb, AG7100_RX_RESERVE);

    return skb;
}

static void
ag7100_buffer_free(struct sk_buff *skb)
{
    if (in_irq())
        dev_kfree_skb_irq(skb);
    else
        dev_kfree_skb(skb);
}

/*
 * Head is the first slot with a valid buffer. Tail is the last slot 
 * replenished. Tries to refill buffers from tail to head.
 */
static int
ag7100_rx_replenish(ag7100_mac_t *mac)
{
    ag7100_ring_t   *r     = &mac->mac_rxring;
    int              head  = r->ring_head, tail = r->ring_tail, refilled = 0;
    ag7100_desc_t   *ds;
    ag7100_buffer_t *bf;

    ag7100_trc(head,"hd");
    ag7100_trc(tail,"tl");

    do
    {
        bf                  = &r->ring_buffer[tail];
        ds                  = &r->ring_desc[tail];

        ag7100_trc(ds,"ds");
#ifdef CONFIG_AR9100
        if(ag7100_rx_owned_by_dma(ds))
        {
            return -1;
        }
#else		
        assert(!ag7100_rx_owned_by_dma(ds));
#endif
        assert(!bf->buf_pkt);

        bf->buf_pkt         = ag7100_buffer_alloc();
        if (!bf->buf_pkt)
        {
            printk(MODULE_NAME ": outta skbs!\n");
            break;
        }
        dma_cache_inv((unsigned long)bf->buf_pkt->data, AG7100_RX_BUF_SIZE);
        ds->pkt_start_addr  = virt_to_phys(bf->buf_pkt->data);

        ag7100_rx_give_to_dma(ds);
        refilled ++;

        ag7100_ring_incr(tail);

    } while(tail != head);
    /*
    * Flush descriptors
    */
    wmb();

    r->ring_tail = tail;
    ag7100_trc(refilled,"refilled");

    return refilled;
}

/* 
 * Reap from tail till the head or whenever we encounter an unxmited packet.
 */
static int
ag7100_tx_reap(ag7100_mac_t *mac)
{
    ag7100_ring_t   *r     = &mac->mac_txring;
    int              head  = r->ring_head, tail = r->ring_tail, reaped = 0, i;
    ag7100_desc_t   *ds;
    ag7100_buffer_t *bf;
    uint32_t    flags;

    ag7100_trc_new(head,"hd");
    ag7100_trc_new(tail,"tl");

    ar7100_flush_ge(mac->mac_unit);

    spin_lock_irqsave(&mac->mac_lock, flags);
    while(tail != head)
    {
        ds   = &r->ring_desc[tail];

        ag7100_trc_new(ds,"ds");

        if(ag7100_tx_owned_by_dma(ds))
            break;

        bf      = &r->ring_buffer[tail];
        assert(bf->buf_pkt);

        ag7100_trc_new(bf->buf_lastds,"lastds");

        if(ag7100_tx_owned_by_dma(bf->buf_lastds))
            break;

        for(i = 0; i < bf->buf_nds; i++)
        {
            ag7100_intr_ack_tx(mac);
            ag7100_ring_incr(tail);
        }

        ag7100_buffer_free(bf->buf_pkt);
        bf->buf_pkt = NULL;

        reaped ++;
    }

    r->ring_tail = tail;
    spin_unlock_irqrestore(&mac->mac_lock, flags);

    if (netif_queue_stopped(mac->mac_dev) &&
        (ag7100_ndesc_unused(mac, r) >= AG7100_TX_QSTART_THRESH) &&
        netif_carrier_ok(mac->mac_dev))
    {
        if (ag7100_reg_rd(mac, AG7100_DMA_INTR_MASK) & AG7100_INTR_TX)
        {
            spin_lock_irqsave(&mac->mac_lock, flags);
            ag7100_intr_disable_tx(mac);
            spin_unlock_irqrestore(&mac->mac_lock, flags);
        }
        netif_wake_queue(mac->mac_dev);
    }

    return reaped;
}

/*
 * allocate and init rings, descriptors etc.
 */
static int
ag7100_tx_alloc(ag7100_mac_t *mac)
{
    ag7100_ring_t *r = &mac->mac_txring;
    ag7100_desc_t *ds;
    int i, next;

    if (ag7100_ring_alloc(r, AG7100_TX_DESC_CNT))
        return 1;

    ag7100_trc(r->ring_desc,"ring_desc");

    ds = r->ring_desc;
    for(i = 0; i < r->ring_nelem; i++ )
    {
        ag7100_trc_new(ds,"tx alloc ds");
        next                =   (i == (r->ring_nelem - 1)) ? 0 : (i + 1);
        ds[i].next_desc     =   ag7100_desc_dma_addr(r, &ds[next]);
        ag7100_tx_own(&ds[i]);
    }

#ifdef DMA_DEBUG
    r = &mac->mac_txring_cache;
    if (ag7100_ring_alloc(r, AG7100_TX_DESC_CNT))
        return 1;
    ds = r->ring_desc;
    for(i = 0; i < r->ring_nelem; i++ )
    {
        next                =   (i == (r->ring_nelem - 1)) ? 0 : (i + 1);
        ds[i].next_desc     =   0xdeadfeed;
        ag7100_tx_own(&ds[i]);
    }
#endif
    return 0;
}

static int
ag7100_rx_alloc(ag7100_mac_t *mac)
{
    ag7100_ring_t *r  = &mac->mac_rxring;
    ag7100_desc_t *ds;
    int i, next, tail = r->ring_tail;
    ag7100_buffer_t *bf;

    if (ag7100_ring_alloc(r, AG7100_RX_DESC_CNT))
        return 1;

    ag7100_trc(r->ring_desc,"ring_desc");

    ds = r->ring_desc;
    for(i = 0; i < r->ring_nelem; i++ )
    {
        next                =   (i == (r->ring_nelem - 1)) ? 0 : (i + 1);
        ds[i].next_desc     =   ag7100_desc_dma_addr(r, &ds[next]);
    }

    for (i = 0; i < AG7100_RX_DESC_CNT; i++)
    {
        bf                  = &r->ring_buffer[tail];
        ds                  = &r->ring_desc[tail];

        bf->buf_pkt         = ag7100_buffer_alloc();
        if (!bf->buf_pkt) 
            goto error;

        dma_cache_inv((unsigned long)bf->buf_pkt->data, AG7100_RX_BUF_SIZE);
        ds->pkt_start_addr  = virt_to_phys(bf->buf_pkt->data);

        ag7100_rx_give_to_dma(ds);
        ag7100_ring_incr(tail);
    }

    return 0;
error:
    printk(MODULE_NAME ": unable to allocate rx\n");
    ag7100_rx_free(mac);
    return 1;
}

static void
ag7100_tx_free(ag7100_mac_t *mac)
{
    ag7100_ring_release(mac, &mac->mac_txring);
    ag7100_ring_free(&mac->mac_txring);
}

static void
ag7100_rx_free(ag7100_mac_t *mac)
{
    ag7100_ring_release(mac, &mac->mac_rxring);
    ag7100_ring_free(&mac->mac_rxring);
}

static int
ag7100_ring_alloc(ag7100_ring_t *r, int count)
{
    int desc_alloc_size, buf_alloc_size;

    desc_alloc_size = sizeof(ag7100_desc_t)   * count;
    buf_alloc_size  = sizeof(ag7100_buffer_t) * count;

    memset(r, 0, sizeof(ag7100_ring_t));

    r->ring_buffer = (ag7100_buffer_t *)kmalloc(buf_alloc_size, GFP_KERNEL);
    printk("%s Allocated %d at 0x%lx\n",__func__,buf_alloc_size,(unsigned long) r->ring_buffer);
    if (!r->ring_buffer)
    {
        printk(MODULE_NAME ": unable to allocate buffers\n");
        return 1;
    }

    r->ring_desc  =  (ag7100_desc_t *)dma_alloc_coherent(NULL, 
        desc_alloc_size,
        &r->ring_desc_dma, 
        GFP_DMA);
    if (! r->ring_desc)
    {
        printk(MODULE_NAME ": unable to allocate coherent descs\n");
        kfree(r->ring_buffer);
        printk("%s Freeing at 0x%lx\n",__func__,(unsigned long) r->ring_buffer);
        return 1;
    }

    memset(r->ring_buffer, 0, buf_alloc_size);
    memset(r->ring_desc,   0, desc_alloc_size);
    r->ring_nelem   = count;

    return 0;
}

static void
ag7100_ring_release(ag7100_mac_t *mac, ag7100_ring_t  *r)
{
    int i;

    for(i = 0; i < r->ring_nelem; i++)
        if (r->ring_buffer[i].buf_pkt)
            ag7100_buffer_free(r->ring_buffer[i].buf_pkt);
}

static void
ag7100_ring_free(ag7100_ring_t *r)
{
    dma_free_coherent(NULL, sizeof(ag7100_desc_t)*r->ring_nelem, r->ring_desc,
        r->ring_desc_dma);
    kfree(r->ring_buffer);
    printk("%s Freeing at 0x%lx\n",__func__,(unsigned long) r->ring_buffer);
}

/*
 * Error timers
 */
static void
ag7100_oom_timer(unsigned long data)
{
    ag7100_mac_t *mac = (ag7100_mac_t *)data;
    int val;

    ag7100_trc(data,"data");
    ag7100_rx_replenish(mac);
    if (ag7100_rx_ring_full(mac))
    {
        val = mod_timer(&mac->mac_oom_timer, jiffies+1);
        assert(!val);
    }
    else
        netif_rx_schedule(mac->mac_dev);
}

static void
ag7100_tx_timeout(struct net_device *dev)
{
    ag7100_mac_t *mac = (ag7100_mac_t *)dev->priv;
    ag7100_trc(dev,"dev");
    printk("%s\n",__func__);
    /* 
    * Do the reset outside of interrupt context 
    */
    schedule_work(&mac->mac_tx_timeout);
}

static void
ag7100_tx_timeout_task(ag7100_mac_t *mac)
{
    ag7100_trc(mac,"mac");
    ag7100_stop(mac->mac_dev);
    ag7100_open(mac->mac_dev);
}

/*
* Changed by BZ@SC_CPUAP
* For ethernet Mac Address
*/
#if 1
static void
ag7100_get_default_macaddr(ag7100_mac_t *mac, u8 *mac_addr)
{
#define MAC_ADDR_IN_FLASH 0xbf03ffa0    
    /* Use MAC address stored in Flash */
    u8 *eep_mac_addr = (mac->mac_unit) ? AR7100_EEPROM_GE1_MAC_ADDR:
        MAC_ADDR_IN_FLASH;    

    printk(MODULE_NAME "CHH: Mac address for unit %d\n",mac->mac_unit);
    printk(MODULE_NAME "CHH: %02x:%02x:%02x:%02x:%02x:%02x \n",
        eep_mac_addr[0],eep_mac_addr[1],eep_mac_addr[2],
        eep_mac_addr[3],eep_mac_addr[4],eep_mac_addr[5]);

    memcpy(mac_addr, eep_mac_addr, 6);
}
#else
static void
ag7100_get_default_macaddr(ag7100_mac_t *mac, u8 *mac_addr)
{
    /*
    ** Use MAC address stored in Flash.  If CONFIG_AG7100_MAC_LOCATION is defined,
    ** it provides the physical address of where the MAC addresses are located.
    ** This can be a board specific location, so it's best to be part of the
    ** defconfig for that board.
    **
    ** The default locations assume the last sector in flash.
    */
    
#ifdef CONFIG_AG7100_MAC_LOCATION
    u8 *eep_mac_addr = (u8 *)( CONFIG_AG7100_MAC_LOCATION + (mac->mac_unit)*6);
#else
    u8 *eep_mac_addr = (mac->mac_unit) ? AR7100_EEPROM_GE1_MAC_ADDR:
        AR7100_EEPROM_GE0_MAC_ADDR;
#endif

    printk(MODULE_NAME "CHH: Mac address for unit %d base = %x \n",mac->mac_unit, mac->mac_base);
    printk(MODULE_NAME "CHH: %02x:%02x:%02x:%02x:%02x:%02x \n",
        eep_mac_addr[0],eep_mac_addr[1],eep_mac_addr[2],
        eep_mac_addr[3],eep_mac_addr[4],eep_mac_addr[5]);
        
    /*
    ** Check for a valid manufacturer prefix.  If not, then use the defaults
    */
    
    if(eep_mac_addr[0] == 0x00 && 
       eep_mac_addr[1] == 0x03 && 
       eep_mac_addr[2] == 0x7f)
    {
        memcpy(mac_addr, eep_mac_addr, 6);
    }
    else
    {
        /* Use Default address at top of range */
        mac_addr[0] = 0x00;
        mac_addr[1] = 0x03;
        mac_addr[2] = 0x7F;
        mac_addr[3] = 0xFF;
        mac_addr[4] = 0xFF;
        mac_addr[5] = 0xFF - mac->mac_unit;
    }
}
#endif
static int
ag7100_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
#if !defined(CONFIG_ATHRS26_PHY) && !defined(CONFIG_ATHRS16_PHY)
    printk(MODULE_NAME ": unsupported ioctl\n");
    return -EOPNOTSUPP;
#else
    return athr_ioctl(ifr->ifr_data, cmd);
#endif
}

static struct net_device_stats 
    *ag7100_get_stats(struct net_device *dev)
{
    ag7100_mac_t *mac = dev->priv;
    struct Qdisc *sch;
    int i;

    sch = rcu_dereference(dev->qdisc);
    mac->mac_net_stats.tx_dropped = sch->qstats.drops;

    i = ag7100_get_rx_count(mac) - mac->net_rx_packets;
    if (i<0)
        i=0;

    mac->mac_net_stats.rx_missed_errors = i;

    return &mac->mac_net_stats;
}

static void
ag7100_vet_tx_len_per_pkt(unsigned int *len)
{
    unsigned int l;

    /* make it into words */
    l = *len & ~3;

    /* 
    * Not too small 
    */
    if (l < AG7100_TX_MIN_DS_LEN)
        l = AG7100_TX_MIN_DS_LEN;
    else
    /* Avoid len where we know we will deadlock, that
    * is the range between fif_len/2 and the MTU size
    */
    if (l > AG7100_TX_FIFO_LEN/2)
        if (l < AG7100_TX_MTU_LEN)
            l = AG7100_TX_MTU_LEN;
        else if (l > AG7100_TX_MAX_DS_LEN)
            l = AG7100_TX_MAX_DS_LEN;
        *len = l;
}

/*
 * All allocations (except irq and rings).
 */
static int __init
ag7100_init(void)
{
    int i;
    struct net_device *dev;
    ag7100_mac_t      *mac;
    uint32_t mask;

    /* 
* Added by MD@SC_CPUAP
*	For ethernet connection status
*/
	ethernet_status_file=create_proc_entry("ethernet_status_cfb",0666,&proc_root);
	ethernet_status_file->owner = THIS_MODULE;
	ethernet_status_file->proc_fops = &ethernet_status_fops;
/* Add end */

    /* 
    * tx_len_per_ds is the number of bytes per data transfer in word increments.
    * 
    * If the value is 0 than we default the value to a known good value based
    * on benchmarks. Otherwise we use the value specified - within some 
    * cosntraints of course.
    *
    * Tested working values are 256, 512, 768, 1024 & 1536.
    *
    * A value of 256 worked best in all benchmarks. That is the default.
    *
    */

    /* Tested 256, 512, 768, 1024, 1536 OK, 1152 and 1280 failed*/
    if (0 == tx_len_per_ds)
        tx_len_per_ds = CONFIG_AG7100_LEN_PER_TX_DS;

    ag7100_vet_tx_len_per_pkt( &tx_len_per_ds);

    printk(MODULE_NAME ": Length per segment %d\n", tx_len_per_ds);

    /* 
    * Compute the number of descriptors for an MTU 
    */
#ifndef CONFIG_AR9100
    tx_max_desc_per_ds_pkt = AG7100_TX_MAX_DS_LEN / tx_len_per_ds;
    if (AG7100_TX_MAX_DS_LEN % tx_len_per_ds) tx_max_desc_per_ds_pkt++;
#else
    tx_max_desc_per_ds_pkt =1;
#endif

    printk(MODULE_NAME ": Max segments per packet %d\n", tx_max_desc_per_ds_pkt);
    printk(MODULE_NAME ": Max tx descriptor count    %d\n", AG7100_TX_DESC_CNT);
    printk(MODULE_NAME ": Max rx descriptor count    %d\n", AG7100_RX_DESC_CNT);

    /* 
    * Let hydra know how much to put into the fifo in words (for tx) 
    */
    if (0 == fifo_3)
        fifo_3 = 0x000001ff | ((AG7100_TX_FIFO_LEN-tx_len_per_ds)/4)<<16;

    printk(MODULE_NAME ": fifo cfg 3 %08x\n", fifo_3);

    /* 
    ** Do the rest of the initializations 
    */

    for(i = 0; i < AG7100_NMACS; i++)
    {
        mac = kmalloc(sizeof(ag7100_mac_t), GFP_KERNEL);
        if (!mac)
        {
            printk(MODULE_NAME ": unable to allocate mac\n");
            return 1;
        }
        memset(mac, 0, sizeof(ag7100_mac_t));

        mac->mac_unit               =  i;
        mac->mac_base               =  ag7100_mac_base(i);
        mac->mac_irq                =  ag7100_mac_irq(i);
        ag7100_macs[i]              =  mac;
        spin_lock_init(&mac->mac_lock);
        /*
        * out of memory timer
        */
        init_timer(&mac->mac_oom_timer);
        mac->mac_oom_timer.data     = (unsigned long)mac;
        mac->mac_oom_timer.function = ag7100_oom_timer;
        /*
        * watchdog task
        */
        INIT_WORK(&mac->mac_tx_timeout, ag7100_tx_timeout_task, mac);

        dev = alloc_etherdev(0);
        if (!dev)
        {
            kfree(mac);
            printk("%s Freeing at 0x%lx\n",__func__,(unsigned long) mac);
            printk(MODULE_NAME ": unable to allocate etherdev\n");
            return 1;
        }

        mac->mac_dev         =  dev;
        dev->get_stats       =  ag7100_get_stats;
        dev->open            =  ag7100_open;
        dev->stop            =  ag7100_stop;
        dev->hard_start_xmit =  ag7100_hard_start;
#if defined(CONFIG_ATHRS26_PHY) || defined(CONFIG_ATHRS16_PHY) 
        dev->do_ioctl        =  ag7100_do_ioctl;
#else
        dev->do_ioctl        =  NULL;
#endif
        dev->poll            =  ag7100_poll;
        dev->weight          =  AG7100_NAPI_WEIGHT;
        dev->tx_timeout      =  ag7100_tx_timeout;
        dev->priv            =  mac;

        ag7100_get_default_macaddr(mac, dev->dev_addr);

        if (register_netdev(dev))
        {
            printk(MODULE_NAME ": register netdev failed\n");
            goto failed;
        }

#ifdef CONFIG_AR9100
        ag7100_reg_rmw_set(mac, AG7100_MAC_CFG1, AG7100_MAC_CFG1_SOFT_RST 
				| AG7100_MAC_CFG1_RX_RST | AG7100_MAC_CFG1_TX_RST);
#else
        ag7100_reg_rmw_set(mac, AG7100_MAC_CFG1, AG7100_MAC_CFG1_SOFT_RST);
#endif
        udelay(20);
        mask = ag7100_reset_mask(mac->mac_unit);

        /*
        * put into reset, hold, pull out.
        */
        ar7100_reg_rmw_set(AR7100_RESET, mask);
        mdelay(100);
        ar7100_reg_rmw_clear(AR7100_RESET, mask);
        mdelay(100);
    }

    ag7100_trc_init();

#ifdef CONFIG_AR9100
#define AP83_BOARD_NUM_ADDR ((char *)0xbf7f1244)

	board_version = (AP83_BOARD_NUM_ADDR[0] - '0') +
			((AP83_BOARD_NUM_ADDR[1] - '0') * 10);
#endif

#if defined(CONFIG_ATHRS26_PHY)
    athrs26_reg_dev(ag7100_macs);
#endif

    return 0;

failed:
    for(i = 0; i < AG7100_NMACS; i++)
    {
        if (!ag7100_macs[i]) 
            continue;
        if (ag7100_macs[i]->mac_dev) 
            free_netdev(ag7100_macs[i]->mac_dev);
        kfree(ag7100_macs[i]);
        printk("%s Freeing at 0x%lx\n",__func__,(unsigned long) ag7100_macs[i]);
    }
    return 1;
}

static void __exit
ag7100_cleanup(void)
{
    int i;

/*
* Added by MD@SC_CPUAP
*	For ethernet connection status
*/
	remove_proc_entry("ethernet_status_cfb",&proc_root);
/* Add end */

    for(i = 0; i < AG7100_NMACS; i++)
    {
        unregister_netdev(ag7100_macs[i]->mac_dev);
        free_netdev(ag7100_macs[i]->mac_dev);
        kfree(ag7100_macs[i]);
        printk("%s Freeing at 0x%lx\n",__func__,(unsigned long) ag7100_macs[i]);
    }
    printk(MODULE_NAME ": cleanup done\n");
}

module_init(ag7100_init);
module_exit(ag7100_cleanup);
