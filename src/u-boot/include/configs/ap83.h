/*
 * This file contains the configuration parameters for the dbau1x00 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/ar7100.h>
#include "sc_config.h"
/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS     1	    /* max number of memory banks */
//#define CFG_MAX_FLASH_SECT      128    /* max number of sectors on one chip */
#define CFG_MAX_FLASH_SECT      256    /* max number of sectors on one chip */
#define CFG_FLASH_SECTOR_SIZE   (64*1024)
#ifdef _SC_CODE_
#ifdef _WAP4410N_
#define CFG_FLASH_SIZE          0x00800000 /* Total flash size */
#endif
#ifdef _AP101NA_
#define CFG_FLASH_SIZE          0x00800000 /* Total flash size */
#endif
#else
#define CFG_FLASH_SIZE          0x00800000 /* Total flash size */
#endif

#define CFG_FLASH_WORD_SIZE     unsigned short 
#define CFG_FLASH_ADDR0         (0x5555)  		
#define CFG_FLASH_ADDR1         (0x2AAA)

#define CFG_HOWL_1_2 1

#ifdef CFG_HOWL_1_2
#define AR9100_FLASH_CONFIG  0xb80f0004
#endif



/* 
 * We boot from this flash
 */
#define CFG_FLASH_BASE		    0xbf000000

#undef CONFIG_ROOTFS_RD
#undef CONFIG_ROOTFS_FLASH
#undef CONFIG_BOOTARGS_FL
#undef CONFIG_BOOTARGS_RD
#undef CONFIG_BOOTARGS
#undef  MTDPARTS_DEFAULT
#undef  MTDIDS_DEFAULT

#define CONFIG_ROOTFS_FLASH
#define CONFIG_BOOTARGS CONFIG_BOOTARGS_FL

#define CONFIG_BOOTARGS_RD     "console=ttyS0,115200 root=01:00 rd_start=0x80600000 rd_size=5242880 init=/sbin/init mtdparts=ar9100-nor0:256k(u-boot),64k(u-boot-env),4096k(rootfs),2048k(uImage)"

/* XXX - putting rootfs in last partition results in jffs errors */

#ifndef CFG_HOWL_1_2
#define CONFIG_BOOTARGS_FL     "console=ttyS0,115200 root=31:00 rootfstype=jffs2 init=/sbin/init mtdparts=ar9100-nor0:4096k(rootfs),256k(u-boot),128k(u-boot-env),1024k(uImage)"
#else
#if 0 /* art not ok */
#define CONFIG_BOOTARGS_FL     "console=ttyS0,115200 root=31:02 rootfstype=jffs2 init=/sbin/init mtdparts=ar9100-nor0:256k(u-boot),64k(u-boot-env),2560k(rootfs),1152k(uImage),64k(calibration)"
#else

#ifdef _WAP4410N_
#define CONFIG_BOOTARGS_FL     "console=ttyS0,115200 root=31:02 rootfstype=jffs2 init=/sbin/init mtdparts=ar9100-nor0:256k(u-boot),64k(u-boot-env),6464k(rootfs),1280k(uImage),64k(nvram),64k(calibration)"
#endif

#ifdef _AP101NA_
#define CONFIG_BOOTARGS_FL     "console=ttyS0,115200 root=31:02 rootfstype=jffs2 init=/sbin/init mtdparts=ar9100-nor0:256k(u-boot),64k(u-boot-env),6464k(rootfs),1280k(uImage),64k(nvram),64k(calibration)"
#endif

#endif
#endif

#define MTDPARTS_DEFAULT    "mtdparts=ar9100-nor0:4096k(rootfs),256k(u-boot),128k(u-boot-env),1024k(uImage)"
#define MTDIDS_DEFAULT      "nor0=ar9100-nor0"


//#define CFG_FLASH_BASE		    0xbfc00000 /* Temp WAR as remap is not on by default */

/* 
 * The following #defines are needed to get flash environment right 
 */
#define	CFG_MONITOR_BASE	TEXT_BASE
#define	CFG_MONITOR_LEN		(192 << 10)

#undef CFG_HZ
/*
 * MIPS32 24K Processor Core Family Software User's Manual
 *
 * 6.2.9 Count Register (CP0 Register 9, Select 0)
 * The Count register acts as a timer, incrementing at a constant
 * rate, whether or not an instruction is executed, retired, or
 * any forward progress is made through the pipeline.  The counter
 * increments every other clock, if the DC bit in the Cause register
 * is 0.
 */
/* Since the count is incremented every other tick, divide by 2 */
/* XXX derive this from CFG_PLL_FREQ */
#if (CFG_PLL_FREQ == CFG_PLL_200_200_100)
#	define CFG_HZ          (200000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_300_300_150)
#	define CFG_HZ          (200000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_333_333_166)
#	define CFG_HZ          (333000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_266_266_133)
#	define CFG_HZ          (266000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_266_266_66)
#	define CFG_HZ          (266000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_400_400_200)
#	define CFG_HZ          (400000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_360_360_180)
#	define CFG_HZ          (360000000/2)
#endif


/* 
 * timeout values are in ticks 
 */
#define CFG_FLASH_ERASE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Write */

/*
 * Cache lock for stack
 */
#define CFG_INIT_SP_OFFSET	0x1000

#define	CFG_ENV_IS_IN_FLASH    1
#undef CFG_ENV_IS_NOWHERE  

/* Address and size of Primary Environment Sector	*/
#ifdef _SC_CODE_
#define CFG_ENV_ADDR		0xbf040000
#define CFG_ENV_SIZE		0x10000
#define CONFIG_BOOTCOMMAND 	"bootm 0xbf6A0000"
#else
#define CFG_ENV_ADDR		0xbf040000
#define CFG_ENV_SIZE		0x20000
#define CONFIG_BOOTCOMMAND 	"bootm 0xbf460000"
#endif

//#define CONFIG_FLASH_16BIT

#define CONFIG_NR_DRAM_BANKS	2

#if (CFG_PLL_FREQ == CFG_PLL_200_200_100)
#define CFG_DDR_REFRESH_VAL     0x4c00
#define CFG_DDR_CONFIG_VAL      0x67bc8cd0
#define CFG_DDR_MODE_VAL_INIT   0x161
#define CFG_DDR_EXT_MODE_VAL    0x2
#define CFG_DDR_MODE_VAL        0x61
#elif (CFG_PLL_FREQ == CFG_PLL_400_400_200) || (CFG_PLL_FREQ == CFG_PLL_360_360_180)
#define CFG_DDR_REFRESH_VAL     0x5f00
#define CFG_DDR_CONFIG_VAL      0x77bc8cd0
#define CFG_DDR_MODE_VAL_INIT   0x131
#define CFG_DDR_EXT_MODE_VAL    0x0
#define CFG_DDR_MODE_VAL        0x31
#endif

#define CFG_DDR_TRTW_VAL        0x1f
#define CFG_DDR_TWTR_VAL        0x1e

#define CFG_DDR_CONFIG2_VAL			    0x83d1f6a2
#define CFG_DDR_RD_DATA_THIS_CYCLE_VAL  0xffff


#define CONFIG_NET_MULTI

#define CONFIG_MEMSIZE_IN_BYTES


/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_COMMANDS	(( CONFIG_CMD_DFL | CFG_CMD_DHCP | CFG_CMD_ELF | \
            CFG_CMD_MII | CFG_CMD_PING | CFG_CMD_NET | CFG_CMD_JFFS2 |\
   CFG_CMD_ENV | CFG_CMD_FLASH | CFG_CMD_LOADS | CFG_CMD_RUN | CFG_CMD_LOADB \
   | CFG_CMD_ELF ))


#define CONFIG_IPADDR		192.168.1.10
#define CONFIG_SERVERIP		192.168.1.11
#define CONFIG_ETHADDR		00:00:00:00:00:00
#define CFG_FAULT_ECHO_LINK_DOWN    1
#define CONFIG_PHY_GIGE       1              /* GbE speed/duplex detect */

//#define CFG_VSC8601_PHY
//#define CFG_VITESSE_8601_7395_PHY 1
#define CFG_ATHR_PHY 1
#ifdef _SC_CODE_
#define CFG_AG7100_NMACS 1
#else
#define CFG_AG7100_NMACS 2
#endif
#define CONFIG_AR9100 1
//#define CFG_PHY_ADDR 0x14  /* Port 4 */
#define CFG_PHY_ADDR 0  /* Port 4 */
#define CFG_GMII     0
#define CFG_MII0_RGMII             1

#define CFG_BOOTM_LEN	(16 << 20) /* 16 MB */
#define DEBUG
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "hush>"

#include <cmd_confdefs.h>

#endif	/* __CONFIG_H */
