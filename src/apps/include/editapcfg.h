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
#ifndef _SC_EDITAPCFG_H_
#define _SC_EDITAPCFG_H_
#include "utility.h"
#include "apcfg.h"

#define SSID_LEN        MAX_SSID
#define SECRET_LEN      CFG_MAX_SECRETLEN
#define KEY_LEN         (MAX_KEY_LEN_BYTES*2)
#define VLANLIST_NO     MAX_VLAN_LIST
#define AC_NO           CFG_MAX_ACL
#define PSKKEY_MAXLEN	CFG_MAX_PASSPHRASE

struct scMac_s{
   unsigned short int no;
   unsigned short int curNo;  // for output index
   unsigned char mac[AC_NO][17];
};
typedef struct scMac_s scMacList_s;

typedef struct rogueAPMac_s{
   unsigned short int no;
   unsigned short int outNo;  // for output index
   unsigned char mac[AC_NO][17+1];
}rogueAPMacList_s;

#define SELECTONE           0
#define ASIA                410
#define Australia           36
#define Canada              124
#define DENMARK             208
#define EUROPE              40
#define FINLAND             246
#define FRANCE              250
#define GERMANY             276
#define IRELAND             372
#define ITALY               380
#define JAPAN               392
#define MEXICO              484
#define NETHERLANDS         528
#define NEW_ZEALAND         554
#define NORWAY              578
#define PUERTO_RICO         630
#define SOUTH_AMERICA       340
#define SPAIN               724
#define SWEDEN              752
#define SWITZERLAND         756
#define UNITED_KINGDOM      826
#define UNITED_STATES       840

// CFG Structure
struct sAttr{
   char *attr;
   void *var;
   void (*assignfunc)();
   char *(*outputfunc)(); 
   void *comment;
   char type;
};
typedef struct sAttr attr_s;

struct sCFG{
   char *group;
   void *attrs;
   char type;
};
typedef struct sCFG CFG_s;

int parseCFG(char *cfgfp, A_UINT8 type, A_BOOL setup);
int OutputCFG(char *buff, A_UINT8 type);

enum{
	EC_GROUP_ALL=0,
	EC_GROUP_IP,
	EC_GROUP_SYS,	
	EC_GROUP_WLAN,
	EC_GROUP_VAP1,
	EC_GROUP_VAP2,
	EC_GROUP_VAP3,
	EC_GROUP_VAP4,
    EC_GROUP_RS,
	EC_GROUP_ACL,
	EC_GROUP_REAP,
	EC_GROUP_ADMIN,
	EC_GROUP_ROGUE,
	EC_GROUP_SNMP,
	EC_GROUP_LOG
};

#define     CFG_TYPE_ALL    0xFF

#endif
