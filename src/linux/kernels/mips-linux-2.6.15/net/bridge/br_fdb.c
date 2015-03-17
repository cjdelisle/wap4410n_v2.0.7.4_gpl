/*
 *	Forwarding database
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	$Id: //depot/sw/releases/7.3_AP/linux/kernels/mips-linux-2.6.15/net/bridge/br_fdb.c#8 $
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/times.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/jhash.h>
#include <asm/atomic.h>
#include "br_private.h"

#ifndef CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT

static int br_no_entries = 0;

#define BR_INCR_ENTRIES() br_no_entries += 1
#define BR_DECR_ENTRIES() br_no_entries -= 1
#define BR_MAX_TABLE_ENTRIES    1024

#endif

static kmem_cache_t *br_fdb_cache __read_mostly;
#ifdef  CONFIG_ATHR_IGMPV2_SNOOPING
static kmem_cache_t *br_mdb_cache __read_mostly;
void del_phy_from_mdb(struct net_bridge * br, struct net_bridge_mdb_entry * mdb, struct net_device * dev,struct mcast_phy * brphy);
#endif
static void athr_print_mac(struct mac_addr *macaddr);
static int fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		      const unsigned char *addr);

static void fdb_rcu_free(struct rcu_head *head);

/* Set entry up for deletion with RCU  */
void br_fdb_put(struct net_bridge_fdb_entry *ent)
{
	if (atomic_dec_and_test(&ent->use_count))
		call_rcu(&ent->rcu, fdb_rcu_free);
}

static __inline__ void fdb_delete(struct net_bridge_fdb_entry *f)
{
	hlist_del_rcu(&f->hlist);
	br_fdb_put(f);
}

#ifdef  CONFIG_ATHR_IGMPV2_SNOOPING
static void mdb_rcu_free(struct rcu_head *head);


static void mdb_rcu_free(struct rcu_head *head)
{
	struct net_bridge_mdb_entry *ent
		= container_of(head, struct net_bridge_mdb_entry, rcu);
	kmem_cache_free(br_mdb_cache, ent);
}

/* Set entry up for deletion with RCU  */
void br_mdb_put(struct net_bridge_mdb_entry *ent)
{
	if (atomic_dec_and_test(&ent->use_count))
		call_rcu(&ent->rcu, mdb_rcu_free);
}

static __inline__ void mdb_delete(struct net_bridge_mdb_entry *m)
{
	hlist_del_rcu(&m->hlist);
	br_mdb_put(m);
}

static void br_mdb_expiry(unsigned long __data)
{
	struct net_bridge * br;
	int i;

	br = (struct net_bridge *) __data;
	spin_lock_bh(&br->mdb_hash_lock);
	for (i=0;i < BR_HASH_SIZE; i++)
	{
		struct hlist_node *h , *next;
		struct net_bridge_mdb_entry * mdb;
		struct mcast_phy *brphy, * next_brphy;

		hlist_for_each_safe(h, next, &br->br_mdb_hash[i]) 
		{
			mdb  = hlist_entry(h, struct net_bridge_mdb_entry, hlist);
			if (time_after(jiffies, mdb->query_timeout_jiffies)) {
				spin_lock_bh(&(mdb->list_lock));

				list_for_each_entry_safe(brphy,next_brphy,&(mdb->br_phylist),list)
				{
					if (time_after(mdb->query_jiffies,brphy->last_heard_jiffies))
					{
					del_phy_from_mdb(br, mdb, brphy->dev,brphy);
					}
				} /* end of brphy loop */
				spin_unlock_bh(&(mdb->list_lock));
			}
		}
	}
	spin_unlock_bh(&br->mdb_hash_lock);
	mod_timer(&(br->mdb_aging_timer),jiffies + msecs_to_jiffies(120*1000) );
}
#endif /* snooping */

#if defined(CONFIG_ATHR_IGMPV2_SNOOPING) || defined(CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT)
static void athr_print_mac(struct mac_addr *macaddr)
{
        unsigned char mac[ETH_ALEN];
        unsigned char mac1[40], *ptr;
        int i, cnt;

        memcpy(mac, macaddr->addr, ETH_ALEN);

        ptr = mac1;

        for (i = 0; i < ETH_ALEN; i++)  {
                cnt = sprintf(ptr, "%02x%c", mac[i], i == ETH_ALEN - 1 ? ' ':':');
                ptr += cnt;
                //printk("%02x%c", mac[i], i == ETH_ALEN - 1 ? ' ':':');
        }

        *(ptr + 1)='\0';

        printk("%s\n", mac1);
}
#endif
#ifdef CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT

/*
 * Add a virtual phy port for wifi/wan interface
 */
static struct athr_eth_physical_port athr_phy_ports[ATHR_MAX_PHY_PORTS + 1] ;



static void __init athr_port_mac_limit_init(void)
{

        int i=0;

        for (i=0; i < ATHR_MAX_PHY_PORTS; i++) {
            athr_phy_ports[i].total_mac_allowed = ATHR_DEFAULT_MAX_MAC_ADDRS_PER_PORT;
            athr_phy_ports[i].mac_addr_count = 0;
        }

        printk("%s: athr_port_mac_limit_init init complete ...\n", __func__);
}

struct athr_eth_physical_port * athr_get_phy_port_from_num(int phy_port_num)
{
        if (phy_port_num < 0 || phy_port_num > (ATHR_MAX_PHY_PORTS + 1)) {
                printk("%s: invalid phy port no %d specified\n", __func__, phy_port_num);
                return NULL;
        }
        else  {
                /*
                 * For wan/wifi interface, port no can be zero, so
                 * assign the max+1 phy port to them
                 */
                if (phy_port_num == 0) {
                        return &athr_phy_ports[ATHR_MAX_PHY_PORTS];
                }
                else
                {
                        return &athr_phy_ports[phy_port_num - 1];
                }
        }

}

void athr_fdb_delete_by_phy_port(struct net_bridge *br, struct athr_eth_physical_port *phy_port)
{
	int i;
	struct net_bridge_fdb_entry *f;

	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct hlist_node *h, *g;

		hlist_for_each_safe(h, g, &br->hash[i]) {
			f = hlist_entry(h, struct net_bridge_fdb_entry, hlist);
			if (!f->is_local && f->phy_port == phy_port) {
                                fdb_delete(f);
                        }
		}
	}
}

int athr_set_phy_port_mac_limit(struct net_bridge *br, int phy_port_num, int new_total_mac_addrs)
{

        struct athr_eth_physical_port *phy_port;

        phy_port = athr_get_phy_port_from_num(phy_port_num);

        if (phy_port == NULL)
                return -EINVAL;

	spin_lock_bh(&br->hash_lock);

        if (new_total_mac_addrs < phy_port->total_mac_allowed) {
            /* clean the mac table */
            printk("%s: phy port %d cleaning the mac table entries\n", __func__, phy_port_num);
            athr_fdb_delete_by_phy_port(br, phy_port);
        }

        phy_port->total_mac_allowed = new_total_mac_addrs;

	spin_unlock_bh(&br->hash_lock);

        return 0;
}


static int athr_phy_port_mac_ageing_timer_op(struct net_bridge *br, unsigned char *mac_addr,
                                      uint16_t new_ageing_interval, int op)
{
	int i, age = -ENODATA;
	struct net_bridge_fdb_entry *f;

	spin_lock_bh(&br->hash_lock);

	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct hlist_node *h, *g;

		hlist_for_each_safe(h, g, &br->hash[i]) {
			f = hlist_entry(h, struct net_bridge_fdb_entry, hlist);
			if (!f->is_local && !compare_ether_addr(mac_addr, f->addr.addr)) {
                                if (op == 1) {
                                        f->max_age = (new_ageing_interval * HZ);
                                        if (f->max_age == 0) {
                                                f->ageing_timer = 0;
                                        }
                                        else
                                        {
                                                f->ageing_timer = jiffies;
                                        }

                                        age = 0;
                                }
                                else if (op == 0) {
                                        age = (f->max_age > 0) ? (f->max_age / HZ) : f->max_age;
                                }
                                break;
                        }
		}
	}


	spin_unlock_bh(&br->hash_lock);

        return age;
}

int athr_set_phy_port_mac_ageing_interval(struct net_bridge *br, unsigned char *mac_addr, int16_t new_ageing_interval)
{
	int ret = 0;

        if (new_ageing_interval < 0) {
                printk("%s: invalid ageing interval %d specified\n", 
                       __func__, new_ageing_interval);
                return -EINVAL;
        }

        ret = athr_phy_port_mac_ageing_timer_op(br, mac_addr, new_ageing_interval, 1);

        return ret;

}

int athr_get_phy_port_mac_ageing_interval(struct net_bridge *br, unsigned char *mac_addr)
{

        int ret = 0;

        ret = athr_phy_port_mac_ageing_timer_op(br, mac_addr, 0, 0);

        return ret;

}

int athr_get_phy_port_mac_addr_limit(int phy_port_num)
{
        struct athr_eth_physical_port *phy_port;

        phy_port = athr_get_phy_port_from_num(phy_port_num);

        if (phy_port == NULL)
                return -EINVAL;

        return phy_port->total_mac_allowed;

}

int athr_get_mac_table_from_phy_port(struct net_bridge *br, int phy_port_num, void __user *userbuf)
{
	int i, cnt = 0, total_mac_count;
	struct net_bridge_fdb_entry *f;
        struct athr_eth_physical_port *phy_port;
        struct athr_phy_port_mac_table *mac_table;
	struct hlist_node *h;
        void *buf;
        int ret=0;
        size_t size =0;
        unsigned long maxnum = 0;

        phy_port = athr_get_phy_port_from_num(phy_port_num);

        if (phy_port == NULL)
                return -EINVAL;

	rcu_read_lock();

        total_mac_count = phy_port->mac_addr_count;

        if (total_mac_count < 1) {
                printk("%s: mac table is empty for port %d\n", __func__, phy_port_num);
                rcu_read_unlock();
                return 0;
        }

        size = phy_port->mac_addr_count * sizeof(struct athr_phy_port_mac_table);

        if (size > PAGE_SIZE) {
                size = PAGE_SIZE;
                maxnum = PAGE_SIZE/sizeof(struct athr_phy_port_mac_table);
        }

        buf = kmalloc(size, GFP_USER);

        if (!buf) {
                printk("%s: kmalloc failed\n", __func__);
                rcu_read_unlock();
                return -ENOMEM;
        }

        mac_table = buf;

	for (i = 0; i < BR_HASH_SIZE; i++) {
		hlist_for_each_entry_rcu(f, h, &br->hash[i], hlist) {
			if (!f->is_local && f->phy_port == phy_port) {

                                memcpy(mac_table->addr, f->addr.addr, ETH_ALEN);
                                mac_table->addr[ETH_ALEN + 1] = '\0';

                                mac_table->max_ageing_interval = (f->max_age > 0) ? \
                                                (f->max_age / HZ) : f->max_age;

                                mac_table->cur_ageing_interval = (f->ageing_timer > 0) ? \
                                                ((jiffies - f->ageing_timer) / HZ) : 
                                                f->ageing_timer;
                                ++mac_table;
                                ++cnt;

                                if (maxnum != 0 && cnt == maxnum) {
                                        break;
                                }
                        }
		}
	}

        if (cnt > 0) {
            if (copy_to_user(userbuf, buf, (cnt * sizeof(struct athr_phy_port_mac_table))))
                     ret = -EFAULT;
            else
                     ret = cnt;
        }
        else
                ret = 0;

        kfree(buf);

        rcu_read_unlock();

        return ret;
}

#endif

void __init br_fdb_init(void)
{
	br_fdb_cache = kmem_cache_create("bridge_fdb_cache",
					 sizeof(struct net_bridge_fdb_entry),
					 0,
					 SLAB_HWCACHE_ALIGN, NULL, NULL);
#ifdef CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT
        athr_port_mac_limit_init();
#endif
}

void __exit br_fdb_fini(void)
{
	kmem_cache_destroy(br_fdb_cache);
}


/* if topology_changing then use forward_delay (default 15 sec)
 * otherwise keep longer (default 5 minutes)
 */
static __inline__ unsigned long hold_time(const struct net_bridge *br)
{
	return br->topology_change ? br->forward_delay : br->ageing_time;
}

static __inline__ int has_expired(const struct net_bridge *br,
				  const struct net_bridge_fdb_entry *fdb)
{
	return !fdb->is_static 
		&& time_before_eq(fdb->ageing_timer + hold_time(br), jiffies);
}

static __inline__ int br_mac_hash(const unsigned char *mac)
{
	return jhash(mac, ETH_ALEN, 0) & (BR_HASH_SIZE - 1);
}


void br_fdb_changeaddr(struct net_bridge_port *p, const unsigned char *newaddr)
{
	struct net_bridge *br = p->br;
	int i;

	spin_lock_bh(&br->hash_lock);

	/* Search all chains since old address/hash is unknown */
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct hlist_node *h;
		hlist_for_each(h, &br->hash[i]) {
			struct net_bridge_fdb_entry *f;

			f = hlist_entry(h, struct net_bridge_fdb_entry, hlist);
			if (f->dst == p && f->is_local) {
				/* maybe another port has same hw addr? */
				struct net_bridge_port *op;
				list_for_each_entry(op, &br->port_list, list) {
					if (op != p && 
					    !compare_ether_addr(op->dev->dev_addr,
								f->addr.addr)) {
						f->dst = op;
						goto insert;
					}
				}

				/* delete old one */
				fdb_delete(f);
				goto insert;
			}
		}
	}
 insert:
	/* insert new address,  may fail if invalid address or dup. */
	fdb_insert(br, p, newaddr);

	spin_unlock_bh(&br->hash_lock);
}

void br_fdb_cleanup(unsigned long _data)
{
	struct net_bridge *br = (struct net_bridge *)_data;
	unsigned long delay = hold_time(br);
	struct net_bridge_fdb_entry *f;
	int i;

	spin_lock_bh(&br->hash_lock);
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct hlist_node *h, *n;

		hlist_for_each_entry_safe(f, h, n, &br->hash[i], hlist) {
#ifdef CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT
			if (!f->is_static && (f->phy_port != NULL)) {
                                /*
                                 * Delete the entry only if the mac ageing
                                 * interval is set to non-zero
                                 */
                                delay = f->max_age;
                                if (delay > 0) {
                                        if (time_before_eq(f->ageing_timer +
                                                           delay,
                                                           jiffies)) {
                                                fdb_delete(f);
                                        }
                                }
                        }
#else
			if (!f->is_static &&
			    time_before_eq(f->ageing_timer + delay, jiffies))
				fdb_delete(f);
#endif
		}
	}
	spin_unlock_bh(&br->hash_lock);

	mod_timer(&br->gc_timer, jiffies + HZ/10);
}

void br_fdb_delete_by_port(struct net_bridge *br, struct net_bridge_port *p)
{
	int i;

	spin_lock_bh(&br->hash_lock);
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct hlist_node *h, *g;

		hlist_for_each_safe(h, g, &br->hash[i]) {
			struct net_bridge_fdb_entry *f
				= hlist_entry(h, struct net_bridge_fdb_entry, hlist);
			if (f->dst != p) 
				continue;

			/*
			 * if multiple ports all have the same device address
			 * then when one port is deleted, assign
			 * the local entry to other port
			 */
			if (f->is_local) {
				struct net_bridge_port *op;
				list_for_each_entry(op, &br->port_list, list) {
					if (op != p && 
					    !compare_ether_addr(op->dev->dev_addr,
								f->addr.addr)) {
						f->dst = op;
						goto skip_delete;
					}
				}
			}

			fdb_delete(f);
		skip_delete: ;
		}
	}
	spin_unlock_bh(&br->hash_lock);
}

/* No locking or refcounting, assumes caller has no preempt (rcu_read_lock) */
struct net_bridge_fdb_entry *__br_fdb_get(struct net_bridge *br,
					  const unsigned char *addr)
{
	struct hlist_node *h;
	struct net_bridge_fdb_entry *fdb;

	hlist_for_each_entry_rcu(fdb, h, &br->hash[br_mac_hash(addr)], hlist) {
		if (!compare_ether_addr(fdb->addr.addr, addr)) {
			if (unlikely(has_expired(br, fdb)))
				break;
			return fdb;
		}
	}

	return NULL;
}

/* Interface used by ATM hook that keeps a ref count */
struct net_bridge_fdb_entry *br_fdb_get(struct net_bridge *br, 
					unsigned char *addr)
{
	struct net_bridge_fdb_entry *fdb;

	rcu_read_lock();
	fdb = __br_fdb_get(br, addr);
	if (fdb) 
		atomic_inc(&fdb->use_count);
	rcu_read_unlock();
	return fdb;
}

static void fdb_rcu_free(struct rcu_head *head)
{
	struct net_bridge_fdb_entry *ent
		= container_of(head, struct net_bridge_fdb_entry, rcu);
#ifdef CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT
        if (ent->phy_port != NULL) {
                if (ent->phy_port->mac_addr_count > 0) {
                        --(ent->phy_port->mac_addr_count);
                        ent->phy_port = NULL;
                        printk("%s: Deleting entry for mac addr : ", __func__);
                        athr_print_mac(&ent->addr);
                }
        }
        else
                printk("%s: Danger will robinson, fdb entry with no phy_port...\n", __func__);
	kmem_cache_free(br_fdb_cache, ent);
#else
	kmem_cache_free(br_fdb_cache, ent);
        BR_DECR_ENTRIES();
#endif
}


/*
 * Fill buffer with forwarding table records in 
 * the API format.
 */
int br_fdb_fillbuf(struct net_bridge *br, void *buf,
		   unsigned long maxnum, unsigned long skip)
{
	struct __fdb_entry *fe = buf;
	int i, num = 0;
	struct hlist_node *h;
	struct net_bridge_fdb_entry *f;

	memset(buf, 0, maxnum*sizeof(struct __fdb_entry));

	rcu_read_lock();
	for (i = 0; i < BR_HASH_SIZE; i++) {
		hlist_for_each_entry_rcu(f, h, &br->hash[i], hlist) {
			if (num >= maxnum)
				goto out;

			if (has_expired(br, f)) 
				continue;

			if (skip) {
				--skip;
				continue;
			}

			/* convert from internal format to API */
			memcpy(fe->mac_addr, f->addr.addr, ETH_ALEN);
			fe->port_no = f->dst->port_no;
			fe->is_local = f->is_local;
			if (!f->is_static)
				fe->ageing_timer_value = jiffies_to_clock_t(jiffies - f->ageing_timer);
			++fe;
			++num;
		}
	}

 out:
	rcu_read_unlock();

	return num;
}

static inline struct net_bridge_fdb_entry *fdb_find(struct hlist_head *head,
						    const unsigned char *addr)
{
	struct hlist_node *h;
	struct net_bridge_fdb_entry *fdb;

	hlist_for_each_entry_rcu(fdb, h, head, hlist) {
		if (!compare_ether_addr(fdb->addr.addr, addr))
			return fdb;
	}
	return NULL;
}
#ifdef CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT
static struct net_bridge_fdb_entry *fdb_create(struct hlist_head *head,
					       struct net_bridge_port *source,
					       const unsigned char *addr, 
					       int is_local,
                                               int phy_port_num)
#else
static struct net_bridge_fdb_entry *fdb_create(struct hlist_head *head,
					       struct net_bridge_port *source,
					       const unsigned char *addr,
					       int is_local)

#endif
{
	struct net_bridge_fdb_entry *fdb;
#ifdef CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT
        struct athr_eth_physical_port *phy_port;
        struct mac_addr mac;

        phy_port =  athr_get_phy_port_from_num(phy_port_num);

        if (phy_port == NULL) {
            printk("%s: packet from mac addr %s arrived on invalid phy port no %d\n",
                   __func__, addr, phy_port_num);
            return NULL;
        }

        /*
         * check the mac limit only if the total mac allowed
         */
        if (phy_port->total_mac_allowed > 0) {
                if (phy_port->mac_addr_count == phy_port->total_mac_allowed) {
                    printk("%s: phy port %d mac limit %d reached, dropping mac addr: ",
                           __func__, phy_port_num, phy_port->total_mac_allowed);
                    memcpy(mac.addr, addr, ETH_ALEN);
                    athr_print_mac(&mac);
                    return NULL;
                }
        }

#else
	if (br_no_entries == BR_MAX_TABLE_ENTRIES) {
                return NULL;
        }
#endif
	fdb = kmem_cache_alloc(br_fdb_cache, GFP_ATOMIC);
	if (fdb) {
		memcpy(fdb->addr.addr, addr, ETH_ALEN);
		atomic_set(&fdb->use_count, 1);
		hlist_add_head_rcu(&fdb->hlist, head);

		fdb->dst = source;
		fdb->is_local = is_local;
		fdb->is_static = is_local;
        fdb->ageing_timer = jiffies;
#ifdef CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT
                fdb->max_age = (ATHR_DEFAULT_PORT_MAC_AGEING_INTERVAL * HZ);
                fdb->phy_port = phy_port;

                /* Don't count the physical port lan address */

                if (!is_local) {
                        ++(phy_port->mac_addr_count);
                }
#else
		BR_INCR_ENTRIES();
#endif
	}
	return fdb;
}

static int fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		  const unsigned char *addr)
{
	struct hlist_head *head = &br->hash[br_mac_hash(addr)];
	struct net_bridge_fdb_entry *fdb;

	if (!is_valid_ether_addr(addr))
		return -EINVAL;

	fdb = fdb_find(head, addr);
	if (fdb) {
		/* it is okay to have multiple ports with same 
		 * address, just use the first one.
		 */
		if (fdb->is_local) 
			return 0;

		printk(KERN_WARNING "%s adding interface with same address "
		       "as a received packet\n",
		       source->dev->name);
		fdb_delete(fdb);
 	}

#ifdef CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT
        /* Pass port no 1 by default for the local device address */
	if (!fdb_create(head, source, addr, 1, 1))
#else
	if (!fdb_create(head, source, addr, 1))
#endif
		return -ENOMEM;

	return 0;
}

int br_fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		  const unsigned char *addr)
{
	int ret;

	spin_lock_bh(&br->hash_lock);
	ret = fdb_insert(br, source, addr);
	spin_unlock_bh(&br->hash_lock);
	return ret;
}

#ifdef CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT
int br_fdb_update(struct net_bridge *br, struct net_bridge_port *source,
		   const unsigned char *addr, int phy_port)
#else
int br_fdb_update(struct net_bridge *br, struct net_bridge_port *source,
		   const unsigned char *addr)
#endif
{
	struct hlist_head *head = &br->hash[br_mac_hash(addr)];
	struct net_bridge_fdb_entry *fdb;
        int ret=0;

	/* some users want to always flood. */
	if (hold_time(br) == 0) {
		return -1;
        }

	rcu_read_lock();

	fdb = fdb_find(head, addr);

	if (likely(fdb)) {
		/* attempt to update an entry for a local interface */
		if (unlikely(fdb->is_local)) {
			if (net_ratelimit())
				printk(KERN_WARNING "%s: received packet with "
				       " own address as source address\n",
				       source->dev->name);
		} else {
			/* fastpath: update of existing entry */
			fdb->dst = source;
#ifdef CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT
                        if (fdb->max_age > 0) {
                                fdb->ageing_timer = jiffies;
                        }
#else
                        fdb->ageing_timer = jiffies;
#endif
		}
	} else {
		spin_lock_bh(&br->hash_lock);
		if (!fdb_find(head, addr))
#ifdef CONFIG_ATHR_ETHERNET_PORT_MAC_LIMIT
                        if (fdb_create(head, source, addr, 0, phy_port) == NULL) {
                                ret = -1;
                        }
#else
			if (fdb_create(head, source, addr, 0) == NULL) {
                                ret = -1;
                        }
#endif
		/* else  we lose race and someone else inserts
		 * it first, don't bother updating
		 */
		spin_unlock_bh(&br->hash_lock);
	}
	rcu_read_unlock();

        return ret;
}

#ifdef  CONFIG_ATHR_IGMPV2_SNOOPING


typedef struct {
	u_int8_t uc[6];
} mac_addr_t;
struct arl_struct {
    mac_addr_t mac_addr;
    int port_map;
    int sa_drop; 
};

int
br_issue_ioctl(struct net_device  * dev, char * grp_mac,int bitmap)
{
	
	struct ifreq ifrr;
	struct arl_struct  arl1;
	int err = -EOPNOTSUPP;

	strncpy(ifrr.ifr_name, dev->name, IFNAMSIZ);
	ifrr.ifr_name[IFNAMSIZ]='\0';

  
	
	memcpy(arl1.mac_addr.uc, grp_mac, 6);
	arl1.port_map = bitmap;
	arl1.sa_drop = 0;
   if (dev->do_ioctl){
			if (bitmap != 0)
				{
				err = dev->do_ioctl(dev, &arl1, SIOCDEVPRIVATE | 0x06);
				}
			else
			{
			err= dev->do_ioctl(dev,&arl1, SIOCDEVPRIVATE | 0x07);
			}
		}

	return(err);
}


void __init br_mdb_init(void)
{
	br_mdb_cache = kmem_cache_create("bridge_mdb_cache",
					 sizeof(struct net_bridge_mdb_entry),
					 0,
					 SLAB_HWCACHE_ALIGN, NULL, NULL);
  if (br_mdb_cache == NULL) return 0;
}

static inline void __br_mdb_init_timer(struct timer_list *timer,
			  void (*_function)(unsigned long),
			  unsigned long _data)
{
	init_timer(timer);
	timer->function = _function;
	timer->data = _data;
}

void br_mdb_init_timer(struct net_bridge *br)
{
	__br_mdb_init_timer(&br->mdb_aging_timer, br_mdb_expiry, (unsigned long) br);
	mod_timer(&(br->mdb_aging_timer), jiffies + msecs_to_jiffies(120*1000));
}
struct net_bridge_mdb_entry  *
mdb_find(struct net_bridge *br, struct hlist_head * head, char *grp_mac)
{
	struct net_bridge_mdb_entry  * mdb = NULL;
	struct hlist_node *h;
	int found = 0;
	rcu_read_lock();
	hlist_for_each_entry_rcu(mdb, h, head, hlist) {
		if (!compare_ether_addr(mdb->multicast_mac.addr, grp_mac)) {
			found = 1;
			break;
		}
	}
	rcu_read_unlock();
	if (found == 0) mdb = NULL;
	return (mdb);
}


struct mcast_phy *
new_mdb_brphy(struct net_bridge_mdb_entry * mdb,
  struct net_device * dev, int athr_phy_port_num)
{

    struct mcast_phy * brphy;
    brphy  = kmalloc(sizeof (struct mcast_phy), GFP_KERNEL);
    if (brphy != NULL){
        brphy->dev = dev;
        brphy->athr_phy_port_num = athr_phy_port_num;
        brphy->no_of_subscribers = 0;
				brphy->last_heard_jiffies = jiffies;
	    spin_lock_bh(&mdb->list_lock);
        list_add(&(brphy->list), &(mdb->br_phylist));
	    spin_unlock_bh(&mdb->list_lock);
    }
    return brphy;
}


struct mcast_port *
new_mdb_brport(struct net_device *dev, struct net_bridge_mdb_entry * mdb)
{

    struct mcast_port * brport;
    brport   = kzalloc(sizeof (struct mcast_port ), GFP_KERNEL);
    if (brport  != NULL){
        brport ->num_phy_ports = 1;
        brport->nbr = dev->br_port;
				brport->portbitmap = 0;
	      spin_lock_bh(&mdb->list_lock);
        list_add(&(brport->list) , &(mdb->br_portlist));
	spin_unlock_bh(&mdb->list_lock);
    }
		mdb->num_brports ++;
    return brport;
}



void
del_phy_from_mdb(struct net_bridge * br, struct net_bridge_mdb_entry * mdb, struct net_device * dev,struct mcast_phy * brphy)
{
	struct mcast_port * brport;
	int found =0;
	int leaving_port;
	int bitmap =0;

	struct net_device * leaving_dev= NULL;
	/* list lock already acquired first delete this brphy from mdb */
	leaving_dev  = brphy->dev;
  leaving_port = brphy->athr_phy_port_num;
	list_del(&(brphy->list));
	kfree(brphy);

	/* find if brport already exists */
	list_for_each_entry(brport,&(mdb->br_portlist),list)
	{
		if (brport->nbr == dev->br_port ) 
		{   
			found = 1;
			break;
		}
	}   
	if(found == 1) {
		brport ->num_phy_ports --;
		bitmap = brport->portbitmap & (~(1<< leaving_port));
		printk("Deleting bitmap %x portmap %x\n", bitmap, brport->portbitmap);
		br_issue_ioctl(leaving_dev, mdb->multicast_mac.addr, bitmap);
		brport->portbitmap = bitmap & 0x3f;
		if (brport->num_phy_ports == 0)
		{
			/* reconfigure s26 hardrware */
			list_del(&(brport->list));
			kfree(brport);
		  mdb->num_brports --;
			/* remove multicast entry also if required */
			if (mdb->num_brports == 0) {
				athr_print_mac(&(mdb->multicast_mac.addr));
				mdb_delete(mdb);
			}
		}
		return;
	}	
	else /* found = 0 */{
		printk("%s.. WARNING -- Del from non-exieting port \n", __func__);

	}	
	return;
}

void
add_phy_to_brport(struct net_device * dev,struct net_bridge_mdb_entry *mdb, int athr_phy_port_num)
{
    struct mcast_port * brport;
		int bitmap = 0;
		int ret = 0;
		int found = 0;
    /* find if brport already exists */
		rcu_read_lock();
    list_for_each_entry_rcu(brport,&(mdb->br_portlist),list)
    {
        if (brport->nbr == dev->br_port ) 
        {   
            brport ->num_phy_ports ++;
						found = 1;
						break;
        }
    }   
		if (found == 0) {
      brport = new_mdb_brport(dev,mdb);
		}
		rcu_read_unlock();
		/* add this port to bitmap if required */
		bitmap = (1<<athr_phy_port_num) & 0x3f;
		if ((bitmap & brport->portbitmap)!= bitmap)
		{
			/* configure hardware for adding ARL */
			bitmap |= brport->portbitmap;
			ret = br_issue_ioctl(dev, mdb->multicast_mac.addr, bitmap);
			if (ret == 0) 
			{
				  brport->portbitmap = bitmap & 0x3f;
		      printk("%p.. Final bitmap %0x \n", brport , brport->portbitmap);
			} else {
				printk("Failed to program hardware port %d error %d  ", athr_phy_port_num, ret);
				athr_print_mac(&(mdb->multicast_mac));

			}
		}
		return;
}


void
del_phy_to_brport(struct net_device * dev,struct net_bridge_mdb_entry *mdb)
{
    /* find if brport already exists */
    struct mcast_port * brport;
    list_for_each_entry(brport,&(mdb->br_portlist),list)
    {
        if (brport->nbr == dev->br_port ) 
        {   
            brport ->num_phy_ports --;
            return;
        }
    }   
    printk("Error - unable to find brport \n");
    return;
}

static struct net_bridge_mdb_entry  *mdb_create(struct net_bridge *br, struct hlist_head *head,
					       const unsigned char *addr)
{
	struct net_bridge_mdb_entry *mdb;

	mdb = kmem_cache_alloc(br_mdb_cache, GFP_ATOMIC);
	
		atomic_set(&mdb->use_count, 1);
	if (mdb) {
		memcpy(mdb->multicast_mac.addr, addr, ETH_ALEN);
        INIT_LIST_HEAD(&mdb->br_portlist);
        INIT_LIST_HEAD(&mdb->br_phylist);
			mdb->query_jiffies = jiffies;
			mdb->query_timeout_jiffies = jiffies + msecs_to_jiffies(200*1000);
			mdb->num_brports = 0;
	    spin_lock_init(&mdb->list_lock);
        spin_lock_bh(&br->mdb_hash_lock);
        hlist_add_head(&mdb->hlist, head);
	    spin_unlock_bh(&br->mdb_hash_lock);
	} else
{	
printk("%s.. Cache alloc returns NULL \n",__func__);
}
	return mdb;
}

struct net_bridge_mdb_entry  *
new_mdb(struct net_bridge *br, struct hlist_head *head,char *grp_mac)
{

    struct net_bridge_mdb_entry  *mdb;

    mdb = mdb_find(br,head,grp_mac);
    if (mdb == NULL) {
        mdb =  mdb_create(br,head,  grp_mac);
        return (mdb);
        }
    return (mdb);
}

void br_mdb_update(struct net_bridge *br, struct net_device *dev, char *grp_mac,  int igmp_cmd, int athr_phy_port_num) 
{

	struct net_bridge_mdb_entry *mdb;
	struct hlist_head *head = &br->br_mdb_hash[br_mac_hash(grp_mac)];
	struct mcast_phy  *brphy = NULL;

	int found = 0;
	/* check if the multi cast mac is already present */
	mdb = mdb_find(br,head, grp_mac);

	if ((mdb == NULL) && (igmp_cmd == 1)) /* JOIN or REPORT */ {
		/* add new entry in the mdb */
		/* and proceed */
		mdb = new_mdb(br,head,grp_mac);
	}
	else if ((mdb == NULL) && (igmp_cmd == 2) ) /* LEAVE*/ {
		printk(" WARNING mdb is NULL Unknown Multi Mac Leaves \n");
		return ;
	}
	if (igmp_cmd == 2) 
	{
		printk("LEAVE cmd ignored - just fwding ..\n");
		return ;
	}
	if (mdb == NULL)
	{
		printk("WARNING --- mdb is NULL.. returning \n");
		return ;
	}
	/* new or old, mdb contains the correct mdb entry */
	/* loop and check for same device /athr_phynum combination */
	rcu_read_lock();
	list_for_each_entry(brphy, &mdb->br_phylist, list) {
		if ((brphy->dev == dev) && 
				(brphy->athr_phy_port_num == athr_phy_port_num)) {
			found = 1;
			break;
		}
	}
	rcu_read_unlock();

	if ((found  == 0) && (igmp_cmd == 1)) /* JOIN*/{
		brphy = new_mdb_brphy(mdb,dev,athr_phy_port_num);
		if (brphy == NULL) /* default to bridge flooding , free mdb entry */
		{
			printk("%s:%d.Cannot allocate the strcutures\n", __func__, __LINE__);
			return;
		}
	}
	if (igmp_cmd == 1 ) {
		brphy->no_of_subscribers ++;
		brphy->last_heard_jiffies = jiffies;
		if (found == 0){
			add_phy_to_brport(dev,mdb, athr_phy_port_num );
		}
	}

	//printk("Printing tables at the end of update \n");
	//br_mdb_print(br);
}
void 
br_mdb_query(struct net_bridge *br,char * grp_mac)
{
	struct net_bridge_mdb_entry *mdb;
	if (0) { /* non 0 grp id */
	  struct hlist_head *head = &br->br_mdb_hash[br_mac_hash(grp_mac)];
		/* get the mdb corresponding to grpmac */
    spin_lock_bh(&br->mdb_hash_lock);
		mdb = mdb_find(br,head,grp_mac);
		spin_unlock_bh(&br->mdb_hash_lock);
		/* if null ERROR */
		if (mdb == NULL) {
			printk("Warning - Query on non existent grp - returning \n");
			return;
		}
		mdb -> query_jiffies = jiffies;
		mdb ->query_timeout_jiffies = jiffies + msecs_to_jiffies(200*1000);
		return;
	}
	else /* 0 grp id */ {
		int i;
		spin_lock_bh(&br->br_mdb_hash);
		for (i = 0; i < BR_HASH_SIZE; i++)
		{
			struct hlist_node *h;
			hlist_for_each (h, &br->br_mdb_hash[i])
			{
				mdb = hlist_entry (h, struct net_bridge_mdb_entry, hlist);
				mdb->query_jiffies = jiffies;
				mdb->query_timeout_jiffies = jiffies + msecs_to_jiffies(200);
			}
		}
		spin_unlock_bh(&br->br_mdb_hash);
	}
}



void
br_mdb_print(struct net_bridge *br)
{
int i;
struct mcast_port  * brport;
struct mcast_phy   * brphy;
for (i = 0; i < BR_HASH_SIZE; i++)
  {
    struct hlist_node *h;
    hlist_for_each (h, &br->br_mdb_hash[i])
    {
      struct net_bridge_mdb_entry *m;
      m = hlist_entry (h, struct net_bridge_mdb_entry, hlist);
      athr_print_mac (&(m->multicast_mac.addr));
      spin_lock_bh(&(m->list_lock));
      list_for_each_entry (brport, &(m->br_portlist), list)
      {
	printk("brport %p -- numPorts %d bitmap %0x \n", brport, brport->num_phy_ports, brport->portbitmap);
      }
      list_for_each_entry (brphy, &(m->br_phylist), list)
      {
	printk(" numSub %d , dev %p -- athr_phy %d\n", brphy->no_of_subscribers, brphy->dev, brphy->athr_phy_port_num);
      }
	spin_unlock_bh(&m->list_lock);
    }
	}
}
#endif
