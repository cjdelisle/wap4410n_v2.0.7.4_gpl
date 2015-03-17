/*
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: //depot/sw/releases/7.3_AP/linux/kernels/mips-linux-2.6.15/net/bridge/br_private.h#4 $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#ifndef _BR_PRIVATE_H
#define _BR_PRIVATE_H

#include <linux/netdevice.h>
#include <linux/miscdevice.h>
#include <linux/if_bridge.h>

#define BR_HASH_BITS 8
#define BR_HASH_SIZE (1 << BR_HASH_BITS)

#define BR_HOLD_TIME (1*HZ)

#define BR_PORT_BITS	10
#define BR_MAX_PORTS	(1<<BR_PORT_BITS)

typedef struct bridge_id bridge_id;
typedef struct mac_addr mac_addr;
typedef __u16 port_id;

struct bridge_id
{
	unsigned char	prio[2];
	unsigned char	addr[6];
};

struct mac_addr
{
	unsigned char	addr[6];
};

#ifdef CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT

#define ATHR_MAX_PHY_PORTS                      CONFIG_ATHR_MAX_PHY_PORTS
#define ATHR_DEFAULT_MAX_MAC_ADDRS_PER_PORT     30
#define ATHR_DEFAULT_PORT_MAC_AGEING_INTERVAL   (30) /* 30 seconds */

struct athr_eth_physical_port
{
        uint16_t                        mac_addr_count;
        uint16_t                        total_mac_allowed;
};

struct athr_phy_port_mac_table
{
	unsigned char	                addr[8];
        unsigned long                   max_ageing_interval;
        unsigned long                   cur_ageing_interval;
};

#endif

struct net_bridge_fdb_entry
{
	struct hlist_node		hlist;
	struct net_bridge_port		*dst;
#ifdef CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT
        struct athr_eth_physical_port    *phy_port;
        unsigned long                   max_age;
#endif
	struct rcu_head			rcu;
	atomic_t			use_count;
	unsigned long			ageing_timer;
	mac_addr			addr;
	unsigned char			is_local;
	unsigned char			is_static;
};

struct net_bridge_port
{
	struct net_bridge		*br;
	struct net_device		*dev;
	struct list_head		list;

	/* STP */
	u8				priority;
	u8				state;
	u16				port_no;
	unsigned char			topology_change_ack;
	unsigned char			config_pending;
	port_id				port_id;
	port_id				designated_port;
	bridge_id			designated_root;
	bridge_id			designated_bridge;
	u32				path_cost;
	u32				designated_cost;

	struct timer_list		forward_delay_timer;
	struct timer_list		hold_timer;
	struct timer_list		message_age_timer;
	struct kobject			kobj;
	struct rcu_head			rcu;
};

/* Multicast DB entry - Indexed by Multicast MAC address */
#ifdef  CONFIG_ATHR_IGMPV2_SNOOPING
struct net_bridge_mdb_entry {
    struct hlist_node    hlist;
	spinlock_t			list_lock;
    mac_addr            multicast_mac;
		int                 num_brports;
    struct list_head    br_portlist;
    struct list_head    br_phylist;
		unsigned long       query_jiffies;
		unsigned long       query_timeout_jiffies;

	  struct rcu_head			rcu;
	  atomic_t			use_count;
};
/* br_portlist is a linked list of the following structures
 * that indicate the br- ports to which packets with this Multicast
 * mac are to be fwded.
 */
struct mcast_port {
    struct net_bridge_port * nbr;
    int    num_phy_ports;
		int    portbitmap;
    struct list_head       list;
};
/* br_phylist is a lined lst of the following structures that
 * keep the record of number of clients for a particular MCAST Mac
 * wrt the physical port 
 */
struct mcast_phy {
    struct net_device * dev;
    int athr_phy_port_num;
    int no_of_subscribers;
		unsigned long last_heard_jiffies;
    struct list_head       list;
};
#endif
struct net_bridge
{
	spinlock_t			lock;
	struct list_head		port_list;
	struct net_device		*dev;
	struct net_device_stats		statistics;
	spinlock_t			hash_lock;
	struct hlist_head		hash[BR_HASH_SIZE];
	struct list_head		age_list;

#ifdef  CONFIG_ATHR_IGMPV2_SNOOPING
    spinlock_t         mdb_hash_lock;
	struct hlist_head	br_mdb_hash[BR_HASH_SIZE];
	struct timer_list   mdb_aging_timer;

#endif
	/* STP */
	bridge_id			designated_root;
	bridge_id			bridge_id;
	u32				root_path_cost;
	unsigned long			max_age;
	unsigned long			hello_time;
	unsigned long			forward_delay;
	unsigned long			bridge_max_age;
	unsigned long			ageing_time;
	unsigned long			bridge_hello_time;
	unsigned long			bridge_forward_delay;

	u16				root_port;
	unsigned char			stp_enabled;
	unsigned char			topology_change;
	unsigned char			topology_change_detected;
/*
* For wireless intervf support
* By TY@SC_CPUAP, At 2007-7-12
* Start of Change
*/
	unsigned char			intervf;
/*
 * End of Change
 */
	struct timer_list		hello_timer;
	struct timer_list		tcn_timer;
	struct timer_list		topology_change_timer;
	struct timer_list		gc_timer;
	struct kobject			ifobj;
};

#ifdef CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT

int athr_set_phy_port_mac_limit(struct net_bridge *br,
                                int phy_port_num,
                                int new_total_mac_addrs);
int athr_set_phy_port_mac_ageing_interval(struct net_bridge *br,
                                      unsigned char *mac_addr,
                                      int16_t new_ageing_interval);
int athr_get_phy_port_mac_ageing_interval(struct net_bridge *br, unsigned char *mac_addr);
int athr_get_phy_port_mac_addr_limit(int phy_port_num);
int athr_get_mac_table_from_phy_port(struct net_bridge *br, int phy_port_num,
                                     void __user *userbuf);

#endif
extern struct notifier_block br_device_notifier;
extern const unsigned char bridge_ula[6];

/* called under bridge lock */
static inline int br_is_root_bridge(const struct net_bridge *br)
{
	return !memcmp(&br->bridge_id, &br->designated_root, 8);
}


/* br_device.c */
extern void br_dev_setup(struct net_device *dev);
extern int br_dev_xmit(struct sk_buff *skb, struct net_device *dev);

/* br_fdb.c */
extern void br_fdb_init(void);
#ifdef  CONFIG_ATHR_IGMPV2_SNOOPING
extern void br_mdb_print(struct net_bridge *);
extern void br_mdb_init(void);
extern void br_mdb_timer_init(struct timer_list *timer, void (*_function)(unsigned long), unsigned long _data);
#endif /* snooping */
extern void br_fdb_fini(void);
extern void br_fdb_changeaddr(struct net_bridge_port *p,
			      const unsigned char *newaddr);
extern void br_fdb_cleanup(unsigned long arg);
extern void br_fdb_delete_by_port(struct net_bridge *br,
			   struct net_bridge_port *p);
extern struct net_bridge_fdb_entry *__br_fdb_get(struct net_bridge *br,
						 const unsigned char *addr);
extern struct net_bridge_fdb_entry *br_fdb_get(struct net_bridge *br,
					       unsigned char *addr);
extern void br_fdb_put(struct net_bridge_fdb_entry *ent);
extern int br_fdb_fillbuf(struct net_bridge *br, void *buf, 
			  unsigned long count, unsigned long off);
extern int br_fdb_insert(struct net_bridge *br,
			 struct net_bridge_port *source,
			 const unsigned char *addr);

#ifdef CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT
extern int br_fdb_update(struct net_bridge *br, struct net_bridge_port *source,
                                      const unsigned char *addr, int phy_port);
#else
extern int br_fdb_update(struct net_bridge *br, struct net_bridge_port *source,
                                      const unsigned char *addr);
#endif

/* br_forward.c */
extern void br_deliver(const struct net_bridge_port *to,
		struct sk_buff *skb);
extern int br_dev_queue_push_xmit(struct sk_buff *skb);
extern void br_forward(const struct net_bridge_port *to,
		struct sk_buff *skb);
extern int br_forward_finish(struct sk_buff *skb);
extern void br_flood_deliver(struct net_bridge *br,
		      struct sk_buff *skb,
		      int clone);
extern void br_flood_forward(struct net_bridge *br,
		      struct sk_buff *skb,
		      int clone);

/* br_if.c */
extern int br_add_bridge(const char *name);
extern int br_del_bridge(const char *name);
extern void br_cleanup_bridges(void);
extern int br_add_if(struct net_bridge *br,
	      struct net_device *dev);
extern int br_del_if(struct net_bridge *br,
	      struct net_device *dev);
extern int br_min_mtu(const struct net_bridge *br);
extern void br_features_recompute(struct net_bridge *br);

/* br_input.c */
extern int br_handle_frame_finish(struct sk_buff *skb);
extern int br_handle_frame(struct net_bridge_port *p, struct sk_buff **pskb);

/* br_ioctl.c */
extern int br_dev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
extern int br_ioctl_deviceless_stub(unsigned int cmd, void __user *arg);

/* br_netfilter.c */
extern int br_netfilter_init(void);
extern void br_netfilter_fini(void);

/* br_stp.c */
extern void br_log_state(const struct net_bridge_port *p);
extern struct net_bridge_port *br_get_port(struct net_bridge *br,
				    	   u16 port_no);
extern void br_init_port(struct net_bridge_port *p);
extern void br_become_designated_port(struct net_bridge_port *p);

/* br_stp_if.c */
extern void br_stp_enable_bridge(struct net_bridge *br);
extern void br_stp_disable_bridge(struct net_bridge *br);
extern void br_stp_enable_port(struct net_bridge_port *p);
extern void br_stp_disable_port(struct net_bridge_port *p);
extern void br_stp_recalculate_bridge_id(struct net_bridge *br);
extern void br_stp_set_bridge_priority(struct net_bridge *br,
				       u16 newprio);
extern void br_stp_set_port_priority(struct net_bridge_port *p,
				     u8 newprio);
extern void br_stp_set_path_cost(struct net_bridge_port *p,
				 u32 path_cost);
extern ssize_t br_show_bridge_id(char *buf, const struct bridge_id *id);

/* br_stp_bpdu.c */
extern int br_stp_handle_bpdu(struct sk_buff *skb);

/* br_stp_timer.c */
extern void br_stp_timer_init(struct net_bridge *br);
extern void br_stp_port_timer_init(struct net_bridge_port *p);
extern unsigned long br_timer_value(const struct timer_list *timer);

/* br.c */
extern struct net_bridge_fdb_entry *(*br_fdb_get_hook)(struct net_bridge *br,
						       unsigned char *addr);
extern void (*br_fdb_put_hook)(struct net_bridge_fdb_entry *ent);


#ifdef CONFIG_SYSFS
/* br_sysfs_if.c */
extern int br_sysfs_addif(struct net_bridge_port *p);
extern void br_sysfs_removeif(struct net_bridge_port *p);
extern void br_sysfs_freeif(struct net_bridge_port *p);

/* br_sysfs_br.c */
extern int br_sysfs_addbr(struct net_device *dev);
extern void br_sysfs_delbr(struct net_device *dev);

#else

#define br_sysfs_addif(p)	(0)
#define br_sysfs_removeif(p)	do { } while(0)
#define br_sysfs_freeif(p)	kfree(p)
#define br_sysfs_addbr(dev)	(0)
#define br_sysfs_delbr(dev)	do { } while(0)
#endif /* CONFIG_SYSFS */

#endif
