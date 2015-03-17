/*
 *	Forwarding decision
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: //depot/sw/releases/7.3_AP/linux/kernels/mips-linux-2.6.15/net/bridge/br_forward.c#3 $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/netfilter_bridge.h>
#include <linux/etherdevice.h>
#include <linux/jhash.h>

#include "br_private.h"

static inline int should_deliver(const struct net_bridge_port *p, 
				 const struct sk_buff *skb)
{
	if (skb->dev == p->dev ||
	    p->state != BR_STATE_FORWARDING)
		return 0;

	return 1;
}

int br_dev_queue_push_xmit(struct sk_buff *skb)
{
	/* drop mtu oversized packets except tso */
	if (skb->len > (skb->dev->mtu + ((skb->protocol == ETH_P_8021Q) ? 4 : 0) ) && !skb_shinfo(skb)->tso_size)
		kfree_skb(skb);
	else {
#ifdef CONFIG_BRIDGE_NETFILTER
		/* ip_refrag calls ip_fragment, doesn't copy the MAC header. */
		nf_bridge_maybe_copy_header(skb);
#endif
		skb_push(skb, ETH_HLEN);

		dev_queue_xmit(skb);
	}

	return 0;
}

int br_forward_finish(struct sk_buff *skb)
{
	NF_HOOK(PF_BRIDGE, NF_BR_POST_ROUTING, skb, NULL, skb->dev,
			br_dev_queue_push_xmit);

	return 0;
}

static void __br_deliver(const struct net_bridge_port *to, struct sk_buff *skb)
{
	skb->dev = to->dev;
	NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_OUT, skb, NULL, skb->dev,
			br_forward_finish);
}

static void __br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
	struct net_device *indev;

	indev = skb->dev;
	skb->dev = to->dev;
	skb->ip_summed = CHECKSUM_NONE;

	NF_HOOK(PF_BRIDGE, NF_BR_FORWARD, skb, indev, skb->dev,
			br_forward_finish);
}

/* called with rcu_read_lock */
void br_deliver(const struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_deliver(to, skb);
		return;
	}

	kfree_skb(skb);
}

/* called with rcu_read_lock */
void br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_forward(to, skb);
		return;
	}

	kfree_skb(skb);
}

/* called under bridge lock */
static void br_flood(struct net_bridge *br, struct sk_buff *skb, int clone,
	void (*__packet_hook)(const struct net_bridge_port *p, 
			      struct sk_buff *skb))
{
	struct net_bridge_port *p;
	struct net_bridge_port *prev;

	if (clone) {
		struct sk_buff *skb2;

		if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
			br->statistics.tx_dropped++;
			return;
		}

		skb = skb2;
	}

	prev = NULL;

	list_for_each_entry_rcu(p, &br->port_list, list) {
/*
* For ****
* By BZ@SC_CPUAP, At 2007-7-12
* Start of Change
*/
		if (memcmp((char *)(p->dev->name),(char *)(skb->dev->name),5)){
/*
 * End of Change
 */
		if (should_deliver(p, skb)) {
			if (prev != NULL) {
				struct sk_buff *skb2;

				if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
					br->statistics.tx_dropped++;
					kfree_skb(skb);
					return;
				}

				__packet_hook(prev, skb2);
			}

			prev = p;
		}
/*
* For ****
* By BZ@SC_CPUAP, At 2007-7-12
* Start of Change
*/
		}else{
                     //printk("[%s],dev prefix is same, should be eth0.x, needn't deliver\n");
		}
/*
 * End of Change
 */
	}

	if (prev != NULL) {
		__packet_hook(prev, skb);
		return;
	}

	kfree_skb(skb);
}


/* called with rcu_read_lock */
void br_flood_deliver(struct net_bridge *br, struct sk_buff *skb, int clone)
{
	br_flood(br, skb, clone, __br_deliver);
}

/* called under bridge lock */
void br_flood_forward(struct net_bridge *br, struct sk_buff *skb, int clone)
{
	br_flood(br, skb, clone, __br_forward);
}
#ifdef  CONFIG_ATHR_IGMPV2_SNOOPING
static __inline__ int br_mdb_hash(const unsigned char *mac)
{
	return jhash(mac, ETH_ALEN, 0) & (BR_HASH_SIZE - 1);
}

void br_mcforward(const struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_forward(to, skb);
		return;
	}

	kfree_skb(skb);
}

void  br_mcast_fwd(struct net_bridge *br, struct sk_buff *skb,const char * dest)
{
	struct hlist_head *head = &br->br_mdb_hash[br_mdb_hash(dest)];
	struct net_bridge_mdb_entry *mdb;
	struct mcast_port * brport;
	struct net_bridge_port  *nbr;
	struct sk_buff  *skb1;
	struct hlist_node *h;
	int found = 0;
	int i = 0;

	rcu_read_lock();

	/* find the multicast mac in net_bridge */
	hlist_for_each_entry_rcu(mdb, h, head, hlist) {
		if (!compare_ether_addr(mdb->multicast_mac.addr, dest))
		{
			found = 1 ;
			break;
		}
	}
	rcu_read_unlock();
	if (found  == 0) 
	{
		/* skb is to be dropped */
		kfree_skb(skb);
		return;
	}
	/* for every brport list in the mdb, clone and call br_forward */

	//`spin_lock_bh(&mdb->list_lock);
	rcu_read_lock();

	list_for_each_entry_rcu(brport, &mdb->br_portlist, list) {
		i++;
		if ((skb1 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
		  kfree_skb(skb);
			return;
		}
		br_mcforward(brport->nbr,skb1);
	}
	rcu_read_unlock();
	//spin_unlock_bh(&mdb->list_lock);
	kfree_skb(skb);

}
    
    
#endif
