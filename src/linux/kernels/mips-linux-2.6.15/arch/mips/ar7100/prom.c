/*
 * Copyright (c) 2009, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 */


/*
 * Prom setup file for ar7100
 */

#include <linux/init.h>
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/bootmem.h>

#include <asm/bootinfo.h>
#include <asm/addrspace.h>

#include "ar7100.h"

int __ath_flash_size;

void __init prom_init(void)
{
    int memsz = 0x2000000, argc = fw_arg0, i;
	char **arg = (char**) fw_arg1;

    printk ("flash_size passed from bootloader = %ld\n", fw_arg3);
    __ath_flash_size = fw_arg3;

	/* 
     * if user passes kernel args, ignore the default one 
     */
/* For 32M RAM. We Only add  "mem=32M" into the default command line
* By TBZ@SC_CPUAP, At 2007-7-12
* Start of Change
*/
#if 1
/*
 * End of Change
 */
	if (argc > 1) {
		arcs_cmdline[0] = '\0';

        for (i = 1; i < argc; i++) 
            printk("arg %d: %s\n", i, arg[i]);

        /* 
         * arg[0] is "g", the rest is boot parameters 
         */
        for (i = 1; i < argc; i++) {
            if (strlen(arcs_cmdline) + strlen(arg[i] + 1)
                >= sizeof(arcs_cmdline))
                break;
            if(i==3){
            strcat(arcs_cmdline, "rootfstype=squashfs");    
            }else{    
            strcat(arcs_cmdline, arg[i]);
            }
            strcat(arcs_cmdline, " ");
        }
    }
/*
* For 32M RAM. We Only add  "mem=32M" into the default command line
* By TBZ@SC_CPUAP, At 2007-7-12
* Start of Change
*/
#endif
/*
 * End of Change
 */
    mips_machgroup = MACH_GROUP_AR7100;
    mips_machtype  = MACH_ATHEROS_AP81;

    /*
     * By default, use all available memory.  You can override this
     * to use, say, 8MB by specifying "mem=8M" as an argument on the
     * linux bootup command line.
     */
    add_memory_region(0, memsz, BOOT_MEM_RAM);
}

void __init prom_free_prom_memory(void)
{
}




