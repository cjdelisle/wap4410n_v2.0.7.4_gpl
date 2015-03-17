#ifndef _UPGRADE_H_
#define _UPGRADE_H_

#include <config.h>
#include "sc_config.h"

/* define for download */
#define FLASH_ADDR_BASE		(CFG_FLASH_BASE)
#define BOOT_ADDR_BASE		(CFG_FLASH_BASE)

#ifdef _WAP4410N_ 
#define FLASH_SIZE			(0x800000)
#define BOOT_SIZE			(0x40000)	//256k
#define NVRAM_SIZE			(0x10000)	//64k  
#define RF_SIZE				(0x10000)	//RF parameters (calibration data)
#define ENV_SIZE			(0x10000)
#endif

#ifdef _AP101NA_ 
#define FLASH_SIZE			(0x800000)
#define BOOT_SIZE			(0x40000)	//256k
#define NVRAM_SIZE			(0x10000)	//64k  
#define RF_SIZE				(0x10000)	//RF parameters (calibration data)
#define ENV_SIZE			(0x10000)
#endif

#define PID_OFFSET			(BOOT_SIZE - 0x56)
#define NODE_ADDRESS		(BOOT_SIZE - 0x60)


/* This part is defined by SPEC */
#ifdef _WAP4410N_ /* fit wap4410n spec */
#define SN_ADDRESS			(NODE_ADDRESS - 0x30) /* length : 16 bytes */
#define DOMAIN_ADDRESS		(NODE_ADDRESS - 0x20) /* length : 1  byte  */
#define COUNTRY_ADDRESS		(DOMAIN_ADDRESS + 0x01)  /* length : 1  byte  */
#define HW_VERSION_ADDR		(DOMAIN_ADDRESS + 0x02)  /* length : 1  byte  */
#define WPS_PIN_ADDRESS		(DOMAIN_ADDRESS + 0x08)	 /* length : 8  bytes */

#define SN_LEN		(16)
#define WPS_PIN_LEN	(8)

#define _KEEP_SN_
#define _KEEP_WPS_PIN_
#define _KEEP_DOMAIN_
#define _KEEP_COUNTRY_
#define _KEEP_HW_

#endif
#ifdef _AP101NA_ /* fit wap4410n spec */
#define SN_ADDRESS			(NODE_ADDRESS - 0x30) /* length : 16 bytes */
#define DOMAIN_ADDRESS		(NODE_ADDRESS - 0x20) /* length : 1  byte  */
#define COUNTRY_ADDRESS		(DOMAIN_ADDRESS + 0x01)  /* length : 1  byte  */
#define HW_VERSION_ADDR		(DOMAIN_ADDRESS + 0x02)  /* length : 1  byte  */
#define WPS_PIN_ADDRESS		(DOMAIN_ADDRESS + 0x08)	 /* length : 8  bytes */

#define SN_LEN		(16)
#define WPS_PIN_LEN	(8)

#define _KEEP_SN_
#define _KEEP_WPS_PIN_
#define _KEEP_DOMAIN_
#define _KEEP_COUNTRY_
#define _KEEP_HW_

#endif

/* This part should be cared - Follow Flash map */
#define RF_ADDRESS			(FLASH_SIZE - RF_SIZE)
#define NVRAM_ADDR			(RF_ADDRESS - NVRAM_SIZE)

#define PRODUCT_ID_OFFSET	(FLASH_SIZE - 0x10 - RF_SIZE - NVRAM_SIZE)
#define PROTOCOL_ID_OFFSET	(PRODUCT_ID_OFFSET + 0x02)
#define FW_VERSION_OFFSET	(PRODUCT_ID_OFFSET + 0x04)
#define SIGN_OFFSET			(PRODUCT_ID_OFFSET + 0x08)   /* eRcOmM */

/**********************************************/
/* This part is related to Download Function  */
/**********************************************/
/* For normal upgrade use */
#define UPGRADE_START_OFFSET (BOOT_SIZE + ENV_SIZE)
#define UPGRADE_END_OFFSET	 (FLASH_SIZE - RF_SIZE - NVRAM_SIZE)
/* protect area */

#define _REMEMBER_MAC_
/* for debug */
#define ASSIGN_TEST 0

/* gpio define */
#define HIGH 	1
#define LOW		0

#define WPS_BUTTON	0
#define WPS_BUTTON_ACTIVE	LOW	
#define INTERNET_LED	13
#define INTERNET_LED_ACTIVE	LOW
#define TEST_LED	8
#define TEST_LED_ACTIVE	HIGH
#define POWER_LED	12
#define POWER_LED_ACTIVE	HIGH

/* move from .c file, some macros needed by some other files */
typedef struct {
	unsigned char      DestAddress[6];
	unsigned char      SrcAddress[6];
	unsigned short     sap;
	unsigned short     wcmd;
	unsigned short     wsequence;
	unsigned short     woffset;
	unsigned short     wsegment;
	unsigned short     wLen;
	unsigned char      bData[600];
} __attribute__((packed)) DLC;
/* sap */
#define UPGRADE_HW_ETHER 0x8888
#define ASSIGN_HW_ETHER 0x15
/* wcmd */
#define  GET_VERSION_INFO 0x0000
#define  DOWN_REQUEST     0x0001
#define  DOWN_DATA        0x0002
#define  DOWN_RESET       0x0003
#define  DOWN_VERIFY      0x0004   
#define  DOWN_EALL        0x0005   

#define DLC_LEN      10

typedef struct  VCI  {
	unsigned char     Prifix[7];
	unsigned short    VerControl;
	unsigned short    DownControl;
	unsigned char     Hid[32];
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
	unsigned char     Postfix[7];
}  __attribute__((packed)) VCI_TABLE;

#define  VCI_LEN         56 //sizeof(VCI_TABLE)-14

typedef  struct  DCB_TYPE {
	unsigned short   sequn;
	unsigned short   dest_seg;
	unsigned short   dest_off;
	unsigned short   src_seg;
	unsigned short   src_off;
	unsigned short   count;
	unsigned         erase;
	unsigned         no;
	unsigned short   tme_of_day;
	unsigned long    state;
}   DCB_BUF;
#define  IDLE            0x00
#define  PROG            0x01
#define  PROGERR         0x02
#define  COM_SEQ_ERR     0x05
#define  SEQ_NUM_ERR     0x06
#define  PROG_ERR        0x07
#define  VERIFY_ERR      0x09

typedef struct belkin_aspack {
	unsigned char	mac[6];
	unsigned int	unused;		/* unused 4 bytes */
	unsigned char	password[6];	/* PMTIAZ for wireless. PMTI for non-wireless */
	unsigned short	padd;		/* unused 2 bytes */
	unsigned char	domain;
	unsigned char	country_code;
	unsigned char	padding[6];	/* unused */
	unsigned char	serial[16];
	unsigned int	pad[4];		/* unused 16 bytes */
	unsigned char	wps_pin[8];
} __attribute__((packed)) multiple_assign;

#define ENET_MAX_PKT 1514

#define REG_WRITE(addr,data) (*((volatile unsigned long *)addr)) = data
#define REG_READ(addr)  (*((volatile unsigned long *) addr))


void AssignStart(void);
void DownloadStart(void);
void AssignHandler (uchar * pkt, unsigned dest, unsigned src, unsigned len);
void DownloadHandler (uchar * pkt, unsigned dest, unsigned src, unsigned len);
void do_boot(void);
void AssignHWAddress(unsigned char *psBuffer);

unsigned long flash_get_sector_addr(unsigned long addr);
unsigned long flash_get_sector_size(unsigned long addr);

#endif
