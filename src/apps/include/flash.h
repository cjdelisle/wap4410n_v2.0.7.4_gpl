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
#ifndef _FLASH_H_
#define _FLASH_H_


#define FLASH_SIZE        		0x00800000
#define BIN_FILE_SIZE           FLASH_SIZE

#define	PARTITION_NUM	        6

#define	MTD_BOOT                0x01
#define MTD_BENV                0x02
#define	MTD_RTFS                0x04
#define	MTD_KERN                0x08
#define	MTD_CONF                0x10
#define MTD_CALI                0x20

#define BOOT_SIZE               0x00040000
#define BENV_SIZE               0x00010000
#define RTFS_SIZE               0x00650000
#define KERN_SIZE               0x00140000
#define CONF_SIZE               0x00010000
#define CALI_SIZE				(FLASH_SIZE - BOOT_SIZE - BENV_SIZE - KERN_SIZE - RTFS_SIZE - CONF_SIZE)

#define BOOT_MTD_DEV            "/dev/mtdblock0"
#define BENV_MTD_DEV            "/dev/mtdblock1"
#define RTFS_MTD_DEV            "/dev/mtdblock2"
#define KERN_MTD_DEV            "/dev/mtdblock3"
#define CONF_MTD_DEV            "/dev/mtdblock4"
#define CALI_MTD_DEV            "/dev/mtdblock5"

/* offset in bin file */
#define BOOT_BIN_OFF            0
#define BENV_BIN_OFF            (BOOT_BIN_OFF+BOOT_SIZE)
#define RTFS_BIN_OFF            (BENV_BIN_OFF+BENV_SIZE)
#define KERN_BIN_OFF            (RTFS_BIN_OFF+RTFS_SIZE)
#define CONF_BIN_OFF            (KERN_BIN_OFF+KERN_SIZE)
#define CALI_BIN_OFF            (CONF_BIN_OFF+CONF_SIZE)

/* offset in FLASH */
#define BOOT_FLASH_OFF          BOOT_BIN_OFF
#define BENV_FLASH_OFF          BOOT_BIN_OFF
#define KERN_FLASH_OFF          KERN_BIN_OFF
#define RTFS_FLASH_OFF          RTFS_BIN_OFF
#define CONF_FLASH_OFF          CONF_BIN_OFF
#define CALI_FLASH_OFF          CALI_BIN_OFF

#define NO_PROTECT              0
#define PROTECT_BOOT            1
#define PROTECT_KERN            2
#define PROTECT_RTFS            3


#define PID_MTDOFFSET         	(BOOT_SIZE - 0x50 + 1)
#define PRODID_MTDOFFSET     	(KERN_SIZE - 0x10)

#define FLASH_ADDR_BASE      0xbf000000

#ifndef NVRAM_SIZE
#define NVRAM_SIZE           0x10000
#endif

#ifndef NODE_ADDRESS
#define NODE_ADDRESS        (BOOT_SIZE - 0x50 - 0x10)
#endif
#ifndef PID_OFFSET
#define PID_OFFSET          (BOOT_SIZE - 0x50 + 1)
#endif
#ifndef DOMAIN_OFFSET       
#define DOMAIN_OFFSET       (BOOT_SIZE - 0x80)
#endif
#ifndef COUNTRY_OFFSET       
#define COUNTRY_OFFSET      (BOOT_SIZE - 0x80 + 1)
#endif
#ifndef HWVER_OFFSET       
#define HWVER_OFFSET        (BOOT_SIZE - 0x80 + 2)
#endif
#ifndef WPS_OFFSET       
#define WPS_OFFSET          (BOOT_SIZE - 0x80 + 8)
#endif

#ifndef SN_OFFSET       
#define SN_OFFSET           (BOOT_SIZE - 0x90)
#ifdef LINKSYS
#define SN_LENGTH           12
#else
#define SN_LENGTH           13 
#endif
#endif

int fwKernFlashErase();
int fwKernFlashWrite(unsigned long offset, char *buf, unsigned long size);
int fwRtfsFlashErase();
int fwRtfsFlashWrite(unsigned long offset, char *buf, unsigned long size);
int fwVersionFlashUpdate(char *fwVersionChar);
int downldPidInfoFlashGet(unsigned char *buf);
int hw_info_read(void);

#define	FL_OPEN_DEV_ERR		1
#define	FL_READ_DEV_ERR		2
#define	FL_WRITE_DEV_ERR	3
#define	FL_ERASE_ERR		4
#define	FL_NOT_CONFIG_DEV	5
#define	FL_TOO_LONG_SIZE	6
#define	FL_BAD_CONFIG		7
#define	FL_NO_ENOUGH_MEM	8
#define	FL_OPEN_FILE_ERR	9
#define	FL_WRITE_FILE_ERR	10
#define	FL_BUF_EMPTY		11

#define HW_ID_LEN   32

typedef  struct  VCI  {
    unsigned short    VerControl;
    unsigned short    DownControl;
    unsigned char     Hid[HW_ID_LEN];
    unsigned short    Hver;
    unsigned short    ProdID;
    unsigned short    ProdIDmask;
    unsigned short    ProtID;
    unsigned short    ProtIDmask;
    unsigned short    FuncID;
    unsigned short    FuncIDmask;
    unsigned short    Fver;
    unsigned short    Cseg;
    unsigned short    Csize;
} __attribute__((packed)) VCI_TABLE;

#define  VCI_LEN         56

#define 	HEX_PIDOFF_HWID 			8
#define     HEX_PIDOFF_HWVER            72
#define     HEX_PIDOFF_PRODUCTID      	76  
#define 	HEX_PIDOFF_SWVERSION		100
#define 	HEX_HWID_LEN				12
#define     HEX_PRODUCTID_LEN        	4

/*
*  GPIO Define
*/
#define GPIO_WIRELESS   0
#define GPIO_STATUS     1
#define GPIO_POWER      1
#define GPIO_POE        3
#define GPIO_POE_DETECT 4
#define GPIO_COLD_START 7
#define GPIO_WARM_START 15
#define GPIO_PUSHBUTTON 21

/*
*  Domain Mapping
*/
#define SKU_US  0x10
#define SKU_AU  0x35
#define SKU_JP  0x40
#define SKU_LA  0x50
#define SKU_BR  0x51
#define SKU_CN  0x70
#define SKU_KR  0x75
#define SKU_G5  0x80

#endif
