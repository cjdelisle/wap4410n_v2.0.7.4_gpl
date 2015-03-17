#define _SC_CODE_


#if 1
#define _WAP4410N_
#else
#define _AP101NA_
#endif


#define _SC_LED_BLINK_SUPPOER_

#ifdef _WAP4410N_
#define BOOT_VERSION_STRING "WAP4410N - Loader Version 1.08"

#define _STANDALONE_

#ifdef _STANDALONE_
#define _STANDALONE_OFFSET			0x38000
#define _CODE_RAM_ADDR			0x80200000
#define _MAX_STANDALONE_SIZE	0x8000
#endif

#endif

#ifdef _AP101NA_
#define BOOT_VERSION_STRING "AP101NA - Loader Version 1.00"
#endif
//#define CONFIG_AR9100_MDIO_DEBUG
//#define _FOR_DDR_ISSUE_
//#define _SC_ENET_DEBUG_
#define _KEEP_CALIBRATION_
#define LED_ON	0
#define LED_OFF	1

#define _MULTIPLE_ASSIGN_

//#define _LOOP_BACK_
#ifdef _LOOP_BACK_
#define _LOOP_BACK_100M_  /* Loopback under 100M */
#endif
#ifdef _LOOP_BACK_
//#define _LOOP_BACK_10M_  /* Loopback under 10M */
#endif
