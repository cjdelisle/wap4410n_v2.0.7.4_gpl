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
#ifndef _UTILITY_H_
#define _UTILITY_H_

typedef enum {
    A_ERROR = -1,               /* Generic error return */
    A_OK = 0,                   /* success */
    A_EINVAL,                   /* Invalid parameter */     
    A_NO_MEMORY,  
	A_ENOENT, 
	A_EXIST    
} A_STATUS;

typedef char                    A_CHAR;
typedef A_CHAR                  A_INT8;
typedef short                   A_INT16;
typedef long                    A_INT32;
typedef unsigned char           A_UCHAR;
typedef A_UCHAR                 A_UINT8;
typedef unsigned short          A_UINT16;
typedef unsigned long           A_UINT32;
typedef unsigned long           A_UINT;
typedef int                     A_BOOL;
typedef long long               A_INT64;
typedef unsigned long long      A_UINT64;
typedef A_UINT64                A_LONGSTATS;

typedef A_UINT32                UINT32;
typedef A_INT16                 INT16;
typedef A_INT32			        INT32;
typedef char			        CHAR;
typedef unsigned char		    BYTE;
typedef unsigned short 		    WORD;
typedef unsigned long		    DWORD;
typedef void 			        VOID;


#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

#define LOCAL   static

#define WLAN_MAC_ADDR_SIZE      6
union wlanMACAddr {
    A_UINT8  octets[WLAN_MAC_ADDR_SIZE];
    A_UINT16 words[WLAN_MAC_ADDR_SIZE/2];
};
typedef union wlanMACAddr WLAN_MACADDR;

#define A_MACADDR_COPY(from, to)              \
    do {                                      \
        (to)->words[0] = (from)->words[0];    \
        (to)->words[1] = (from)->words[1];    \
        (to)->words[2] = (from)->words[2];    \
    } while (0)

#define A_MACADDR_COMP(m1, m2)                \
    ((((m1)->words[2] == (m2)->words[2]) &&   \
      ((m1)->words[1] == (m2)->words[1]) &&   \
      ((m1)->words[0] == (m2)->words[0])) == 0)
      
/*utility functions*/
int sc_dbg(const char *format, ...);
int SYSTEM(const char *format, ...) ;
int COMMAND(const char *format, ...);

void scToLows(A_UINT8 *charStr);
void scToUppers(A_UINT8 *charStr); 	
int scValidStr(A_UINT8 *charStr); 	
int scValidUrl(A_UINT8 *charStr);
int scValidIPv6(A_UINT8 *ipStr, A_UINT8 flag);
int scValidGWv6(A_UINT8 *ipStr);
int scValidEmailAddr(A_UINT8 *email, A_UINT8 len);
int scChars2Hexs(unsigned char *charStr, int strLen, char *hexBuf , char *separator);
int macAddrToString(char *macAddress, char *buf , char *separator);
A_INT16 scHex2Char(A_UINT8 * str, A_UINT8 len, A_INT32 * value);
A_INT16 scHexs2Chars(A_UINT8 * hexs, A_UINT8 * str, A_UINT8 len, A_INT16 interval);
void scMacStr12ToStr6(A_UINT8 *str12, A_UINT8 *str6);
void scMacStr17ToStr12(A_UINT8 *str17, A_UINT8 *str12);
void scMacStr12ToStr17(A_UINT8 *str12, A_UINT8 *str17, char *separator);
A_BOOL  scValidHex(char ch);
A_BOOL  scValidHexs(char *str, int len);
A_BOOL 	scValidNetbiosName(char *pName, A_UINT16 len);
A_BOOL 	scValidHostName(char *pName, A_UINT16 len);
A_BOOL scValidIpAddress(A_UINT32 ipAddress);
A_BOOL scValidIpMask(A_UINT32 ipMask, A_UINT32 *pValidIpMask);
A_BOOL scValidIpGateWay(A_UINT32 gateway);
A_BOOL scValidIpMaskGateWay(A_UINT32 ipaddr, A_UINT32 netmask, A_UINT32 gateway);
A_BOOL asciiToPassphraseKey(A_UINT8 *pstr, A_UINT8 *pPpKey, int encryptedKeyLen);
int scMaxTxPowerGet(int unit);
void sgml_encode(char *pStrDest, char *pStrSrc, A_BOOL readOnly);
void scSecretHide(char *secret);
A_BOOL scSecretHidden(char *secret);
A_UINT16 scGetWord(A_UINT8 * buf);
void scSetWord(A_UINT8 * buf, A_UINT16 wValue);
A_UINT32 scGetDword(A_UINT8 * buf);
void scSetDword(A_UINT8 * buf, A_UINT32 dwValue);	
A_BOOL scValidwdsVlanList(char *list);
typedef struct if_info_s{
	char ifname[16];
	char ipaddr[16];
	char mac[18];
	char mask[16];
	struct in_addr gw;
	int  mtu;
	
}if_info_t;
/* add by archer for statistical */
typedef struct if_adv_info_s{
    char ifname[16];
    unsigned long long rx_packets;	/* total packets received       */
	unsigned long long tx_packets;	/* total packets transmitted    */
	unsigned long long rx_bytes;	/* total bytes received         */
	unsigned long long tx_bytes;	/* total bytes transmitted      */
	unsigned long errors;	/* bad packets received         */
	unsigned long dropped;	/* no space in linux buffers    */
}if_adv_info_t;

struct net_device_info {
	unsigned long long rx_packets;	/* total packets received       */
	unsigned long long tx_packets;	/* total packets transmitted    */
	unsigned long long rx_bytes;	/* total bytes received         */
	unsigned long long tx_bytes;	/* total bytes transmitted      */
	unsigned long rx_errors;	/* bad packets received         */
	unsigned long tx_errors;	/* packet transmit problems     */
	unsigned long rx_dropped;	/* no space in linux buffers    */
	unsigned long tx_dropped;	/* no space available in linux  */
	unsigned long rx_multicast;	/* multicast packets received   */
	unsigned long rx_compressed;
	unsigned long tx_compressed;
	unsigned long collisions;
	
    unsigned long rx_fifo_errors;	/* recv'r fifo overrun          */
    unsigned long tx_fifo_errors;
    unsigned long rx_frame_errors;	/* recv'd frame alignment error */
    unsigned long tx_carrier_errors;
};
/* add end */

int get_sockfd(void);
int getMgtBrInfo(if_info_t *if_info);
int getIFInfo(char *if_name, if_info_t *if_info);
int getIFAdvInfo(char *if_name, if_adv_info_t *if_info);
void libGetAPMacAddress(char *pMac);
void getProductName(char *prodName);
void getProductDesc(char *buffer);
void getTimeofDay(char *buffer, int type);
#define TIME_FORMAT_NORMAL  0
#define TIME_FORMAT_MULTILANG   1
void getUpTime(char *buffer);
void getHwVersion(char *buffer);
void getVersion(char *buffer);
unsigned short getVersion2(void);

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN            46
#endif
typedef struct if_infov6_s{
	char ifname[16];
	char ipaddr[INET6_ADDRSTRLEN];
    char gw[INET6_ADDRSTRLEN];
	
}if_infov6_t;
int getMgtBrv6Info(if_infov6_t *if_info, int status);
int getIP6Info(char *ifname, char *address);
int getIP6GwInfo(char *ifname, char *gw );
int getIP6RadvdInfo(char *ifname, if_infov6_t *if_info );
int getIP6dhcpInfo(char *ifname, if_infov6_t *if_info );
int getEthernetStatus(void);

void country_list_generate(void);
void set_default_country(void);
A_BOOL check_country_domain(int);
A_INT16 sc_get_default_country(void);

A_BOOL  scCompositor(int v[], int left, int right);
A_BOOL scConversionSSID(A_UINT8 *src, A_UINT8 *dest);

A_STATUS scSetDefaultVap(int unit, int bss);
A_STATUS scSetDefaultSecurity(int unit, int bss);

#endif
