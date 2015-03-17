/*
 * This is a module which is used for setting the MSS option in TCP packets.
 *
 * Copyright (C) 2000 Marc Boucher <marc@mbsi.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/skbuff.h>

#include <linux/ip.h>
#include <net/tcp.h>

#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_TCPMSS.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marc Boucher <marc@mbsi.ca>");
MODULE_DESCRIPTION("iptables TCP MSS modification module");

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

static u_int16_t
cheat_check(u_int32_t oldvalinv, u_int32_t newval, u_int16_t oldcheck)
{
	u_int32_t diffs[] = { oldvalinv, newval };
	return csum_fold(csum_partial((char *)diffs, sizeof(diffs),
                                      oldcheck^0xFFFF));
}

static inline unsigned int
optlen(const u_int8_t *opt, unsigned int offset)
{
	/* Beware zero-length options: make finite progress */
	if (opt[offset] <= TCPOPT_NOP || opt[offset+1] == 0) return 1;
	else return opt[offset+1];
}

static unsigned int
ipt_tcpmss_target(struct sk_buff **pskb,
		  const struct net_device *in,
		  const struct net_device *out,
		  unsigned int hooknum,
		  const void *targinfo,
		  void *userinfo)
{
	const struct ipt_tcpmss_info *tcpmssinfo = targinfo;
	struct tcphdr *tcph;
	struct iphdr *iph;
	u_int16_t tcplen, newtotlen, oldval, newmss;
	unsigned int i;
	u_int8_t *opt;

	if (!skb_make_writable(pskb, (*pskb)->len))
		return NF_DROP;

	if ((*pskb)->ip_summed == CHECKSUM_HW &&
	    skb_checksum_help(*pskb, out == NULL))
		return NF_DROP;

	iph = (*pskb)->nh.iph;
	tcplen = (*pskb)->len - iph->ihl*4;

	tcph = (void *)iph + iph->ihl*4;

	/* Since it passed flags test in tcp match, we know it is is
	   not a fragment, and has data >= tcp header length.  SYN
	   packets should not contain data: if they did, then we risk
	   running over MTU, sending Frag Needed and breaking things
	   badly. --RR */
	if (tcplen != tcph->doff*4) {
		if (net_ratelimit())
			printk(KERN_ERR
			       "ipt_tcpmss_target: bad length (%d bytes)\n",
			       (*pskb)->len);
		return NF_DROP;
	}

	if(tcpmssinfo->mss == IPT_TCPMSS_CLAMP_PMTU) {
		if(!(*pskb)->dst) {
			if (net_ratelimit())
				printk(KERN_ERR
			       		"ipt_tcpmss_target: no dst?! can't determine path-MTU\n");
			return NF_DROP; /* or IPT_CONTINUE ?? */
		}

		if(dst_mtu((*pskb)->dst) <= (sizeof(struct iphdr) + sizeof(struct tcphdr))) {
			if (net_ratelimit())
				printk(KERN_ERR
		       			"ipt_tcpmss_target: unknown or invalid path-MTU (%d)\n", dst_mtu((*pskb)->dst));
			return NF_DROP; /* or IPT_CONTINUE ?? */
		}

		newmss = dst_mtu((*pskb)->dst) - sizeof(struc