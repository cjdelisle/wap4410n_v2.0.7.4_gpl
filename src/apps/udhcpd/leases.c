/* 
 * leases.c -- tools to manage DHCP leases 
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 */

#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "debug.h"
#include "dhcpd.h"
#include "files.h"
#include "options.h"
#include "leases.h"
#include "arpping.h"
#include "static_leases.h"

unsigned char blank_chaddr[] = {[0 ... 15] = 0};
static	time_t time_bef_NTP=0;/* record the old time,use for currect time after ntp get new time */

/* clear every lease out that chaddr OR yiaddr matches and is nonzero */
void clear_lease(u_int8_t *chaddr, u_int32_t yiaddr, int ifid)
{
	unsigned int i, j;
	for (j = 0; j < 16 && !chaddr[j]; j++);
	
	for (i = 0; i < server_config[ifid].max_leases; i++)
		if ((j != 16 && !memcmp(leases[i].chaddr, chaddr, 16)) ||
		    (yiaddr && leases[i].yiaddr == yiaddr)) {
			memset(&(leases[i]), 0, sizeof(struct dhcpOfferedAddr));
		}
}

#ifdef RONSCODE
/* add a lease into the table, clearing out any old ones */
struct dhcpOfferedAddr *add_lease(u_int8_t *chaddr, u_int32_t yiaddr, unsigned long lease, int ifid,char *hostname)
#else
struct dhcpOfferedAddr *add_lease(u_int8_t *chaddr, u_int32_t yiaddr, unsigned long lease, int ifid)
#endif
{
	struct dhcpOfferedAddr *oldest;
	/* clean out any old ones */
	clear_lease(chaddr, yiaddr, ifid);
		
	oldest = oldest_expired_lease(ifid);
	
	if (oldest) {
		memcpy(oldest->chaddr, chaddr, 16);
		oldest->yiaddr = yiaddr;
		oldest->expires = time(0) + lease;
#ifdef RONSCODE
		memcpy(oldest->hostname,hostname,16);
#endif
	}
	
	return oldest;
}

/* -- Jeff Sun -- Apr.23.2005 -- Modify for update expire time after ntp get correct time */
/* true if a lease has expired */
int lease_expired(struct dhcpOfferedAddr *lease)
{
    time_t now = time(0);
    int c;
    unsigned int i;
    
    /* init time_bef_NTP */
    if(time_bef_NTP==0) time_bef_NTP=now;

    /* ntp get new time,orig time is in year 2000,new time is after 2005.01.01, do update */
    if( now - time_bef_NTP > ((2005-2000)*365*24*60*60) )
    {
    	for (c = 0; c < no_of_ifaces; c++)
		{
			if (server_config[c].active == FALSE)
					continue;
            
        	for (i = 0; i < server_config[c].max_leases; i++)
        		if (leases[i].yiaddr != 0 && server_config[c].remaining)
       				leases[i].expires += (now-time_bef_NTP);
	    }    
    }
    time_bef_NTP=now;
    
	return (lease->expires < (unsigned long) now);
}	


/* Find the oldest expired lease, NULL if there are no expired leases */
struct dhcpOfferedAddr *oldest_expired_lease(int ifid)
{
	struct dhcpOfferedAddr *oldest = NULL;
	unsigned long oldest_lease = time(0);
	unsigned int i;

	
	for (i = 0; i < server_config[ifid].max_leases; i++)
		if (oldest_lease > leases[i].expires) {
			oldest_lease = leases[i].expires;
			oldest = &(leases[i]);
		}
	return oldest;
		
}


/* Find the first lease that matches chaddr, NULL if no match */
struct dhcpOfferedAddr *find_lease_by_chaddr(u_int8_t *chaddr, int ifid)
{
	unsigned int i;

	for (i = 0; i < server_config[ifid].max_leases; i++)
		if (!memcmp(leases[i].chaddr, chaddr, 16)) return &(leases[i]);
	
	return NULL;
}


/* Find the first lease that matches yiaddr, NULL is no match */
struct dhcpOfferedAddr *find_lease_by_yiaddr(u_int32_t yiaddr, int ifid)
{
	unsigned int i;

	for (i = 0; i < server_config[ifid].max_leases; i++)
		if (leases[i].yiaddr == yiaddr) return &(leases[i]);
	
	return NULL;
}


/* find an assignable address, it check_expired is true, we check all the expired leases as well.
 * Maybe this should try expired leases by age... */
u_int32_t find_address(int check_expired, int ifid) 
{
	u_int32_t addr, ret;
	struct dhcpOfferedAddr *lease = NULL;		
	addr = ntohl(server_config[ifid].start); /* addr is in host order here */
	for (;addr <= ntohl(server_config[ifid].end); addr++) {
		/* ie, 192.168.55.0 */
		if (!(addr & 0xFF)) continue;

		/* ie, 192.168.55.255 */
		if ((addr & 0xFF) == 0xFF) continue;
		/* Ron */
		/* ie, this ip is server ip */
		if (addr == ntohl(server_config[ifid].server)) continue;
		/* Ron */
	
	   /* only do if it is not a static lease address */
	   if (!reservedIp(server_config[ifid].static_leases,htonl(addr)))
	   {
		/* lease is not taken */
		ret = htonl(addr);
		if ((!(lease = find_lease_by_yiaddr(ret, ifid)) ||

		     /* or it expired and we are checking for expired leases */
		     (check_expired  && lease_expired(lease))) &&

		     /* and it isn't on the network */
	    	     !check_ip(ret, ifid)) {
			return ret;
			break;
		}
	   }
	}
	return 0;
}

/* check is an IP is taken, if it is, add it to the lease table */
int check_ip(u_int32_t addr, int ifid)
{
	struct in_addr temp;
#ifdef RONSCODE
	char hostname[16]="";
#endif	
	
	if (arpping(addr, server_config[ifid].server, server_config[ifid].arp, server_config[ifid].interface) == 0) {
		temp.s_addr = addr;
	 	LOG(LOG_INFO, "%s belongs to someone, reserving it for %ld seconds", 
	 		inet_ntoa(temp), server_config[ifid].conflict_time);
#ifdef RONSCODE
		add_lease(blank_chaddr, addr, server_config[ifid].conflict_time, ifid,hostname);
#else
		add_lease(blank_chaddr, addr, server_config[ifid].conflict_time, ifid);
#endif
		return 1;
	} else 
	{
		return 0;
	}
}

