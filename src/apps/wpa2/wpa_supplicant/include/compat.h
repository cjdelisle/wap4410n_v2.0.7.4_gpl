/*-
 * Copyright (c) 2002-2004 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 *
 * $Id: //depot/sw/releases/7.3_AP/wlan/include/compat.h#3 $
 */
#ifndef _ATH_COMPAT_H_
#define _ATH_COMPAT_H_
/*
 * BSD/Linux compatibility shims.  These are used mainly to
 * minimize differences when importing necesary BSD code.
 */
#define NBBY    8           /* number of bits/byte */

#ifdef __linux__
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
#define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))  /* to any y */
#endif
#elif WIN32
#define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))  /* to any y */
#endif

#define howmany(x, y)   (((x)+((y)-1))/(y))

/* Bit map related macros. */
#define setbit(a,i) ((a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define clrbit(a,i) ((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#define isset(a,i)  ((a)[(i)/NBBY] & (1<<((i)%NBBY)))
#define isclr(a,i)  (((a)[(i)/NBBY] & (1<<((i)%NBBY))) == 0)

#ifndef WIN32
#define __printflike(_a,_b) \
    __attribute__ ((__format__ (__printf__, _a, _b)))
#endif

#define __offsetof(t,m) offsetof(t,m)

#ifndef ALIGNED_POINTER
/*
 * ALIGNED_POINTER is a boolean macro that checks whether an address
 * is valid to fetch data elements of type t from on this architecture.
 * This does not reflect the optimal alignment, just the possibility
 * (within reasonable limits). 
 *
 */
#define ALIGNED_POINTER(p,t)    1
#endif

/*
**  For non Linux (gcc compiled) drivers, define the likely() and 
**  unlikely() macros to be simply the argument.  This sould fix build
**  issues for NetBSD and Vista
*/

#ifndef __linux__

#define unlikely(_a)    _a
#define likely(_b)      _b

#endif

/*
** Assert for Linux kernel mode.  This assumes unlikely is defined,
** so it assumes a Linux OS
*/

#ifdef __KERNEL__
#include <asm/page.h>
extern void ar7xxx_misc_wakeup_process(void);
extern void ar7xxx_set_kassert(unsigned char enable);
extern unsigned char ar7xxx_is_kassert_enabled(void);

#if 0
#define SET_KASSERT(_en)  \
    do {                  \
    ar7xxx_set_kassert(_en); \
}while(0)

#define GET_KASSERT()  ar7xxx_is_kassert_enabled()
#endif
#if 0
#define KASSERT(exp, msg) do {          \
    if (unlikely(!(exp))) {         \
        if(ar7xxx_is_kassert_enabled()) { \
            printk msg;         \
            BUG();              \
        }                       \
        else {                  \
            ar7xxx_misc_wakeup_process(); \
        }                       \
    }                   \
} while (0)
#else
#define KASSERT(exp, msg) do {          \
    if (unlikely(!(exp))) {         \
        printk msg;         \
        BUG();              \
    }                   \
} while (0)
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
#define CTL_AUTO -2
#define DEV_ATH 9
#else
#define CTL_AUTO CTL_UNNUMBERED
#define DEV_ATH CTL_UNNUMBERED
#endif  /* sysctl */

#endif /* __KERNEL__ */

#ifndef WIN32
#ifndef ATH_SUPPORT_STA
#ifndef __packed
#define __packed   __attribute__((__packed__))
#endif
#endif
#endif

/*
 * NetBSD/FreeBSD defines for file version.
 */
#define __FBSDID(_s)
#define __KERNEL_RCSID(_n,_s)
#endif /* _ATH_COMPAT_H_ */
