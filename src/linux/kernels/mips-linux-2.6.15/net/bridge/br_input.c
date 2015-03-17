/*
 *	Handle incoming frames
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: //depot/sw/releases/7.3_AP/linux/kernels/mips-linux-2.6.15/net/bridge/br_input.c#4 $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/netfilter_bridge.h>
#include "br_private.h"

#ifdef  CONFIG_ATHR_IGMPV2_SNOOPING
#include <linux/ip.h>
#include <linux/igmp.h>
void igmp_snoop_pkt(struct sk_buff *skb, int *cmd, char * groupAddrL2);
#endif
const unsigned char bridge_ula[6] = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };

static int br_pass_frame_up_finish(struct sk_buff *skb)
{
	netif_receive_skb(skb);
	return 0;
}

static void br_pass_frame_up(struct net_bridge *br, struct sk_buff *skb)
{
	struct net_device *indev;

	br->statistics.rx_packets++;
	br->statistics.rx_bytes += skb->len;

	indev = skb->dev;
	skb->dev = br->dev;

	NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_IN, skb, indev, NULL,
			br_pass_frame_up_finish);
}

/* note: already called with rcu_read_lock (preempt_disabled) */
int br_handle_frame_finish(struct sk_buff *skb)
{
	const unsigned char *dest = eth_hdr(skb)->h_dest;
/*
* For wireless intervf support
* By TY@SC_CPUAP, At 2007-7-12
* Start of Change
*/
	const unsigned char *source = eth_hdr(skb)->h_source;
	struct net_bridge_fdb_entry *src;
/*
 * End of Change
 */
	struct net_bridge_port *p = skb->dev->br_port;
	struct net_bridge *br = p->br;
	struct net_bridge_fdb_entry *dst;
	int passedup = 0;

  //printk("from_dev is %p \n" , skb->dev);
	/* insert into forwarding database after filtering to avoid spoofing */
#ifdef CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT
	if (br_fdb_update(p->br, p, eth_hdr(skb)->h_source, skb->cb[0]) < 0) {
		kfree_skb(skb);
		goto out;
	}
#else
	if (br_fdb_update(p->br, p, eth_hdr(skb)->h_source) < 0) {
		kfree_skb(skb);
		goto out;
	}
#endif

	if (br->dev->flags & IFF_PROMISC) {
		struct sk_buff *skb2;

		skb2 = skb_clone(skb, GFP_ATOMIC);
		if (skb2 != NULL) {
			passedup = 1;
			br_pass_frame_up(br, skb2);
		}
	}

#ifdef  CONFIG_ATHR_IGMPV2_SNOOPING
	if (dest[0] & 1)  {
		int igmp_cmd = 0;
		char grp_mac[6];

		if (!compare_ether_addr(br->dev->broadcast, dest)) {
			br_flood_forward(br, skb,!passedup);
			if (!passedup)
				br_pass_frame_up(br, skb);
			goto out;
		}
		igmp_snoop_pkt(skb, &igmp_cmd, &(grp_mac[0]));
		if ((igmp_cmd == 1)  || (igmp_cmd == 2))
		{ 
			/* we got a IGMP packet
			 * process igmp based on igmp_cmd 
			 * and continue the flow 
			 */
			br_mdb_update(br, skb->dev,grp_mac, igmp_cmd, skb->cb[0]);
		}else if (igmp_cmd == 3) {
			br_mdb_query(br, grp_mac);
		}
		
		if ((igmp_cmd != 0) && (igmp_cmd !=1)) {
			br_flood_forward(br,skb,!passedup);
			if (!passedup)
				br_pass_frame_up(br, skb);
		}else  if (igmp_cmd == 1) 
		{
			if (!passedup)
				br_pass_frame_up(br, skb);
			
		} else {
		  br_mcast_fwd(br, skb,dest);
		}
		goto out;
	}
#else
	if (dest[0] & 1) {
		br_flood_forward(br, skb, !passedup);
		if (!passedup)
			br_pass_frame_up(br, skb);
		goto out;
	}

#endif
	dst = __br_fdb_get(br, dest);
	if (dst != NULL && dst->is_local) {
		if (!passedup)
			br_pass_frame_up(br, skb);
		else
			kfree_skb(skb);
		goto out;
	}

	if (dst != NULL) {
/*
* For wireless intervf support
* By TY@SC_CPUAP, At 2007-7-12
* Start of Change
*/
	    if(br->intervf)
	    {
	        #ifdef T_DEBUG    
	            printk("<0><%s,%d>: block inter vap forward enabled\n",__FUNCTION__,__LINE__);
	        #endif
                src = __br_fdb_get(br, source);
    	        if(src==NULL || ((strncmp(src->dst->dev->name, "ath", 3) == 0) &&
    	           (strncmp(dst->dst->dev->name, "ath", 3) == 0)))
    	        {
    	            #ifdef T_DEBUG    
	                printk("<0><%s,%d>: block a packet from vap to vap\n",__FUNCTION__,__LINE__);
	            #endif
    	            kfree_skb(skb);
    	            goto out;
    	        }
	    }
/*
 * End of Change
 */
		br_forward(dst->dst, skb);
		goto out;
	}

	br_flood_forward(br, skb, 0);

out:
	return 0;
}

/*
 * Called via br_handle_frame_hook.
 * Return 0 if *pskb should be processed furthur
 *	  1 if *pskb is handled
 * note: already called with rcu_read_lock (preempt_disabled) 
 */
int br_handle_frame(struct net_bridge_port *p, struct sk_buff **pskb)
{
	struct sk_buff *skb = *pskb;
	const unsigned char *dest = eth_hdr(skb)->h_dest;

	if (p->state == BR_STATE_DISABLED)
		goto err;

	if (!is_valid_ether_addr(eth_hdr(skb)->h_source))
		goto err;

	if (p->state == BR_STATE_LEARNING)
#ifdef CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT
                if (br_fdb_update(p->br, p, eth_hdr(skb)->h_source, skb->cb[0]) < 0) {
                        goto err;
                }
#else
                if (br_fdb_update(p->br, p, eth_hdr(skb)->h_source) < 0) {
                        goto err;
                }
#endif

	if (p->br->stp_enabled &&
	    !memcmp(dest, bridge_ula, 5) &&
	    !(dest[5] & 0xF0)) {
		if (!dest[5]) {
			NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_IN, skb, skb->dev, 
				NULL, br_stp_handle_bpdu);
			return 1;
		}
	}

	else if (p->state == BR_STATE_FORWARDING) {
		if (br_should_route_hook) {
			if (br_should_route_hook(pskb)) 
				return 0;
			skb = *pskb;
			dest = eth_hdr(skb)->h_dest;
		}

		if (!compare_ether_addr(p->br->dev->dev_addr, dest))
			skb->pkt_type = PACKET_HOST;

		NF_HOOK(PF_BRIDGE, NF_BR_PRE_ROUTING, skb, skb->dev, NULL,
			br_handle_frame_finish);
		return 1;
	}

err:
	kfree_skb(skb);
	return 1;
}
#ifdef  CONFIG_ATHR_IGMPV2_SNOOPING
#define IGMP_SNOOP_CMD_JOIN 1
#define IGMP_SNOOP_CMD_LEAVE  2
#define IGMP_SNOOP_CMD_QUERY  3
#define IGMP_SNOOP_CMD_OTHER 0

void
igmp_snoop_pkt(struct sk_buff *skb, int *cmd, char * groupAddrL2)
{

	int proto = eth_hdr(skb)->h_proto;
	if (proto == htons(ETH_P_IP)) {
		const struct iphdr *ip = (struct iphdr *)
			(skb->data);

		if (ip->protocol == 2) {
			/* ver1, ver2 */
			const struct igmphdr *igmp = (struct igmphdr *)
				(skb->data +  (4 * ip->ihl));
			/* ver 3*/
			const struct igmpv3_report *igmpr3 = (struct igmpv3_report *) igmp;
			u_int32_t	groupAddr = 0;
			*cmd = IGMP_SNOOP_CMD_OTHER;
			u_int8_t	*srcAddr = eth_hdr(skb)->h_source;

			switch (igmp->type) {
				case IGMP_HOST_MEMBERSHIP_QUERY:
					/* query */
					groupAddr = igmp->group;
					*cmd = 3;
					break;
				case IGMP_HOST_MEMBERSHIP_REPORT :
					/* V1 report */
					groupAddr = igmp->group;
					*cmd = IGMP_SNOOP_CMD_JOIN;
					break;
				case IGMPV2_HOST_MEMBERSHIP_REPORT	:
					/* V2 report */
					groupAddr = igmp->group;
					*cmd = IGMP_SNOOP_CMD_JOIN;
					break;
				case IGMP_HOST_LEAVE_MESSAGE :
					/* V2 leave */
					groupAddr = igmp->group;
					*cmd = IGMP_SNOOP_CMD_LEAVE;
					break;
				case IGMPV3_HOST_MEMBERSHIP_REPORT:
					/* V3 report */
					groupAddr = igmpr3->grec[0].grec_mca;
					if (igmpr3->grec[0].grec_type == IGMPV3_CHANGE_TO_EXCLUDE ||
							igmpr3->grec[0].grec_type == IGMPV3_MODE_IS_EXCLUDE)
					{
						*	cmd = IGMP_SNOOP_CMD_JOIN;
					}
					else if (igmpr3->grec[0].grec_type == IGMPV3_CHANGE_TO_INCLUDE ||
							igmpr3->grec[0].grec_type == IGMPV3_MODE_IS_INCLUDE)
					{
						*	cmd = IGMP_SNOOP_CMD_LEAVE;
					}
					break;
				default:
					printk("%s.. encountered bad igmp type %d\n", __func__, igmp->type);
					break;
			}

			printk( "IGMP %s %2.2x [%2.2x] %8.8x - %02x:%02x:%02x:%02x:%02x:%02x\n",
					*cmd == 2 ? "Leave" : (*cmd == 1 ? "Join " : "other"),
					igmp->type,
					igmp->type == 0x22 ? igmpr3->grec[0].grec_type : 0,
					groupAddr,
					srcAddr[0],srcAddr[1],srcAddr[2],
					srcAddr[3],srcAddr[4],srcAddr[5]);


			groupAddrL2[0] = 0x01;
			groupAddrL2[1] = 0x00;
			groupAddrL2[2] = 0x5e;
			groupAddrL2[3] = (groupAddr >> 16) & 0x7f;
			groupAddrL2[4] = (groupAddr >>  8) & 0xff;
			groupAddrL2[5] = (groupAddr >>  0) & 0xff;

		} 
	}

}


#endif /* snooping */
