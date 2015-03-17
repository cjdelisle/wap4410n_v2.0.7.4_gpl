/*
 * Copyright (C) 2005 SerComm Corporation. All Rights Reserved.
 *
 * SerComm Corporation reserves the right to make changes to this document
 * without notice. SerComm Corporation makes no warranty, representation
 * or guarantee regarding the suitability of its products for any
 * particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm
 * Corporation specifically disclaims any and all liability, including
 * without limitation consequential or incidental damages; neither does
 * it convey any license under its patent rights, nor the rights of
 * others.
 */
/***************************************************************
*             Copyright (C) 2006 by SerComm Corp.
*                    All Rights Reserved.
*
*      Use of this software is restricted to the terms and
*      conditions of SerComm's software license agreement.
*
*                        www.sercomm.com
****************************************************************/
#define BOOTLOADER	"/dev/mtdblock0"
#define LINUX		"/dev/mtdblock1"
#define ROOTFS		"/dev/mtdblock2"
#define CONFIG		"/dev/mtdblock3"

/*
/dev/mtdblock0 : 0x00000000-0x00060000 : "bootloader"
/dev/mtdblock1 : 0x00060000-0x003b0000 : "linux"
/dev/mtdblock2 : 0x001a0000-0x003b0000 : "rootfs"
/dev/mtdblock3 : 0x007f0000-0x00800000 : "config"
*/

#define PHYS_BOOT_BASE  0xbF000000

#define PID_LEN		70
#define LOADER_LEN	0x20000

#define PID_MTD     BOOTLOADER
//#define PID_OFFSET	(LOADER_LEN-PID_LEN)

#define WPS_PIN_OFFSET 0x1FF70
#define MAC_OFFSET  (LOADER_LEN-0x60)

#define BUF_SIZE 0x10000 /*64k*/
#define PER_WRITE_SIZE 0x4000

int check_pid(char *pidStr);
int fw_upld_init(void);
int fw_upld_done(void);
int fw_upld_upgrade(char *pData, int dataLen);

