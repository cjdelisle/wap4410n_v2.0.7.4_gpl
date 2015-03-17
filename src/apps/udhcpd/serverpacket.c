/* serverpacket.c
 *
 * Constuct and send DHCP server packets
 *
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#include "packet.h"
#include "debug.h"
#include "dhcpd.h"
#include "options.h"
#include "leases.h"
#include "static_leases.h"

/* send a packet to giaddr using the kernel ip stack */
static int send_packet_to_relay(struct dhcpMessage *payload, int ifid)
{
	DEBUG(LOG_INFO, "Forwarding packet to relay");

	return kernel_packet(payload, server_config[ifid].server, SERVER_PORT,
			payload->giaddr, SERVER_PORT);
}


/* send a packet to a specific arp address and ip address by creating our own ip packet */
static int send_packet_to_client(struct dhcpMessage *payload, int force_broadcast, int ifid)
{
	unsigned char *chaddr;
	u_int32_t ciaddr;
	
	if (force_broadcast) {
		DEBUG(LOG_INFO, "broadcasting packet to client (NAK)");
		ciaddr = INADDR_BROADCAST;
		chaddr = MAC_BCAST_ADDR;
	} else if (payload->ciaddr) {
		DEBUG(LOG_INFO, "unicasting packet to client ciaddr");
		ciaddr = payload->ciaddr;
		chaddr = payload->chaddr;
	} else if (ntohs(payload->flags) & BROADCAST_FLAG) {
		DEBUG(LOG_INFO, "broadcasting packet to client (requested)");
		ciaddr = INADDR_BROADCAST;
		chaddr = MAC_BCAST_ADDR;
	} else {
		DEBUG(LOG_INFO, "unicasting packet to client yiaddr");
		ciaddr = payload->yiaddr;
		chaddr = payload->chaddr;
	}
	return raw_packet(payload, server_config[ifid].server, SERVER_PORT, 
			ciaddr, CLIENT_PORT, chaddr, server_config[ifid].ifindex);
}


/* send a dhcp packet, if force broadcast is set, the packet will be broadcast to the client */
static int send_packet(struct dhcpMessage *payload, int force_broadcast, int ifid)
{
	int ret;

	if (payload->giaddr)
		ret = send_packet_to_relay(payload, ifid);
	else ret = send_packet_to_client(payload, force_broadcast, ifid);
	return ret;
}


static void init_packet(struct dhcpMessage *packet, struct dhcpMessage *oldpacket, char type, int ifid)
{
	init_header(packet, type);
	packet->xid = oldpacket->xid;
	memcpy(packet->chaddr, oldpacket->chaddr, 16);
	packet->flags = oldpacket->flags;
	packet->giaddr = oldpacket->giaddr;
	packet->ciaddr = oldpacket->ciaddr;
	add_simple_option(packet->options, DHCP_SERVER_ID, server_config[ifid].server);
}


/* add in the bootp options */
static void add_bootp_options(struct dhcpMessage *packet, int ifid)
{
	packet->siaddr = server_config[ifid].siaddr;
	if (server_config[ifid].sname)
		strncpy(packet->sname, server_config[ifid].sname, sizeof(packet->sname) - 1);
	if (server_config[ifid].boot_file)
		strncpy(packet->file, server_config[ifid].boot_file, sizeof(packet->file) - 1);
}
	

/* send a DHCP OFFER to a DHCP DISCOVER */
int sendOffer(struct dhcpMessage *oldpacket, int ifid)
{
	struct dhcpMessage packet;
	struct dhcpOfferedAddr *lease = NULL;
	u_int32_t req_align, lease_time_align = server_config[ifid].lease;
	unsigned char *req, *lease_time;
	struct option_set *curr;
	struct in_addr addr;

	uint32_t static_lease_ip;
	
#ifdef RONSCODE
	char hostname[16]="";
#endif
	
	init_packet(&packet, oldpacket, DHCPOFFER, ifid);
	
	static_lease_ip=getIpByMac(server_config[ifid].static_leases,oldpacket->chaddr);
	
	/* ADDME: if static, short circuit */
     if (!static_lease_ip)
     {	     
	/* the client is in our lease/offered table */
	/* Ron  add 3 lines */
	if((req = get_option(oldpacket, DHCP_REQUESTED_IP)))
		memcpy(&req_align, req, 4);
	//if ((lease = find_lease_by_chaddr(oldpacket->chaddr, ifid))) {
	if ((lease = find_lease_by_chaddr(oldpacket->chaddr, ifid)) 
			&& (ntohl(req_align)!=ntohl(server_config[ifid].server))) {
		if (!lease_expired(lease)) 
			lease_time_align = lease->expires - time(0);
		packet.yiaddr = lease->yiaddr;
	/* Or the client has a requested ip */
	} else if ((req = get_option(oldpacket, DHCP_REQUESTED_IP)) &&

		   /* Don't look here (ugly hackish thing to do) */
		   memcpy(&req_align, req, 4) &&

		   /* and the ip is in the lease range */
		   ntohl(req_align) >= ntohl(server_config[ifid].start) &&
		   ntohl(req_align) <= ntohl(server_config[ifid].end) &&
		   !static_lease_ip &&
		   /* and its not already taken/offered */ /* ADDME: check that its not a static lease */
		   ((!(lease = find_lease_by_yiaddr(req_align, ifid)) ||
		   
		   /* or its taken, but expired */ /* ADDME: or maybe in here */
		   lease_expired(lease)))) {
		   /* check id addr is not taken by a static ip */
#ifdef RONSCODE	
		   if(!check_ip(req_align, ifid) && (ntohl(req_align)!=ntohl(server_config[ifid].server)) ) 	
#else
		   if(!check_ip(req_align, ifid)) 	
#endif
				packet.yiaddr = req_align; /* FIXME: oh my, is there a host using this IP? */
		   else {
			   /*is it now a static lease, no,beacause find_address skips static lease*/
			   packet.yiaddr = find_address(0, ifid);
			   /* try for an expired lease */
			   if (!packet.yiaddr) packet.yiaddr = find_address(1, ifid);
			   
			}


	/* otherwise, find a free IP */ /*ADDME: is it a static lease? */
	} else {
		packet.yiaddr = find_address(0, ifid);
		
		/* try for an expired lease */
		if (!packet.yiaddr) packet.yiaddr = find_address(1, ifid);

	}
	
	if(!packet.yiaddr) {
		LOG(LOG_WARNING, "no IP addresses to give -- OFFER abandoned");
		return -1;
	}
#ifdef RONSCODE
	{
		char *tmp=get_option(oldpacket,DHCP_HOST_NAME);
		int len=0;
		if(tmp){
			len=*(tmp-1);
			strncpy(hostname,tmp,len);
		}
		//printf("client->name==%s\n",hostname);
	}
	if (!add_lease(packet.chaddr, packet.yiaddr, server_config[ifid].offer_time, ifid,hostname)) {
		LOG(LOG_WARNING, "lease pool is full -- OFFER abandoned");
		return -1;
	}		
#else
	if (!add_lease(packet.chaddr, packet.yiaddr, server_config[ifid].offer_time, ifid)) {
		LOG(LOG_WARNING, "lease pool is full -- OFFER abandoned");
		return -1;
	}		
#endif
	if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
		memcpy(&lease_time_align, lease_time, 4);
		lease_time_align = ntohl(lease_time_align);
		if (lease_time_align > server_config[ifid].lease) 
			lease_time_align = server_config[ifid].lease;
	}

	/* Make sure we aren't just using the lease time from the previous offer */
	if (lease_time_align < server_config[ifid].min_lease) 
		lease_time_align = server_config[ifid].lease;
	/* ADDME: end of short circuit */		
     }
     else
     {
	     /*It is a static lease... use it*/
	     packet.yiaddr=static_lease_ip;
     }
	     
	
	add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(lease_time_align));

	curr = server_config[ifid].options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
		curr = curr->next;
	}

	add_bootp_options(&packet, ifid);
	
	addr.s_addr = packet.yiaddr;
	LOG(LOG_INFO, "sending OFFER of %s", inet_ntoa(addr));
	return send_packet(&packet, 0, ifid);
}


int sendNAK(struct dhcpMessage *oldpacket, int ifid)
{
	struct dhcpMessage packet;

	init_packet(&packet, oldpacket, DHCPNAK, ifid);
	
	DEBUG(LOG_INFO, "sending NAK");
	return send_packet(&packet, 1, ifid);
}


int sendACK(struct dhcpMessage *oldpacket, u_int32_t yiaddr, int ifid)
{
	struct dhcpMessage packet;
	struct option_set *curr;
	unsigned char *lease_time;
	u_int32_t lease_time_align = server_config[ifid].lease;
	struct in_addr addr;
#ifdef RONSCODE	
	char hostname[16]="";
#endif
	init_packet(&packet, oldpacket, DHCPACK, ifid);
	packet.yiaddr = yiaddr;
	
	if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
		memcpy(&lease_time_align, lease_time, 4);
		lease_time_align = ntohl(lease_time_align);
		if (lease_time_align > server_config[ifid].lease) 
			lease_time_align = server_config[ifid].lease;
		else if (lease_time_align < server_config[ifid].min_lease) 
			lease_time_align = server_config[ifid].lease;
	}
	
	add_simple_option(packet.options, DHCP_LEASE_TIME, htonl(lease_time_align));
	
	curr = server_config[ifid].options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
		curr = curr->next;
	}

	add_bootp_options(&packet, ifid);

	addr.s_addr = packet.yiaddr;
	LOG(LOG_INFO, "sending ACK to %s", inet_ntoa(addr));

	if (send_packet(&packet, 0, ifid) < 0) 
		return -1;
#ifdef RONSCODE
	{
		char *tmp=get_option(oldpacket,DHCP_HOST_NAME);
		int len=0;
		if(tmp){
			len=*(tmp-1);
			strncpy(hostname,tmp,len);
		}
	}
		
	add_lease(packet.chaddr, packet.yiaddr, lease_time_align, ifid, hostname);
#else
	add_lease(packet.chaddr, packet.yiaddr, lease_time_align, ifid);
#endif
	return 0;
}


int send_inform(struct dhcpMessage *oldpacket, int ifid)
{
	struct dhcpMessage packet;
	struct option_set *curr;

	init_packet(&packet, oldpacket, DHCPACK, ifid);
	
	curr = server_config[ifid].options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
		curr = curr->next;
	}

	add_bootp_options(&packet, ifid);

	return send_packet(&packet, 0, ifid);
}



