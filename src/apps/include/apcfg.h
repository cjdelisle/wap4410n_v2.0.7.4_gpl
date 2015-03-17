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

#ifndef __APCFGh
#define __APCFGh
#include <netinet/in.h>
#include <arpa/inet.h>

#include "nvram.h"
#include "utility.h"

int apcprint(const char *format, ...);

#define WLAN_MAX_DEV	1
#define WLAN_MAX_VAP	4

#define RADIO_5G		1
#define RADIO_24G		0

#define ENABLE          "1"
#define DISABLE         "0"
A_STATUS apCfgScPidSet(char *pid);
char *apCfgScPidGet(void);	


#define 	HEX_PIDOFF_HWID 			8
#define     HEX_PIDOFF_PRODUCTID      	76  
#define 	HEX_PIDOFF_SWVERSION		100
#define 	HEX_HWID_LEN				12
#define     HEX_PRODUCTID_LEN        	4

/********************************************************/
/*		Get/Set function on system		 				*/	
/********************************************************/
A_STATUS apCfgSysNameSet(char *p);
char *   apCfgSysNameGet(void);
#define DEFAULT_SYSNAME                 ""
#define MAX_SYSNAME                     39

A_STATUS apCfgDescSet(char *p);
char *   apCfgDescGet(void);
#define DEFAULT_DESC                 ""
#define MAX_DESC                     39

A_STATUS apCfgSysLangSet(char *p);
char *   apCfgSysLangGet(void);

A_STATUS apCfgLoginSet(char *);
char *   apCfgLoginGet(void);
#define DEFAULT_USERNAME                ""
#define CFG_MAX_USERNAME                63

A_STATUS apCfgPasswordSet(char *);
char *   apCfgPasswordGet(void);
#define DEFAULT_PASSWORD                ""
#define CFG_MAX_PASSWORD                63

A_STATUS apCfgVlanListApply(int unit);
A_STATUS apCfgVlanListAdd(int v);
A_STATUS apCfgVlanListSet(char *p);
A_STATUS apCfgVlanListDel(int v);
A_STATUS apCfgVlanListClear(void);
char * apCfgVlanListGet(void);
char * apCfgwdsVlanListGet(void);
int  apCfgwdsVlanListSet(char *);
#define MAX_VLAN_LIST   8

A_STATUS apCfgipv6modeSet(int v);
int apCfgipv6modeGet(void);

//A_STATUS apCfgipv6wanmodeSet(int v);
//int apCfgipv6wanmodeGet();

A_STATUS apCfgDhcpEnableSet(int v);
int apCfgDhcpEnableGet(void);
A_STATUS apCfgDhcp6EnableSet(int v);    /* Add for dhcp client v6 */
int apCfgDhcp6EnableGet(void);          /* Add for dhcp client v6 */
A_STATUS apCfgRadvdEnableSet(int v);    /* Add for Radvd client */
int apCfgRadvdEnableGet(void);          /* Add for Radvd client */
#define DEFAULT_DHCP  	false




A_STATUS apCfgDhcpServerEnableSet(int v);
int apCfgDhcpServerEnableGet(void);

#define ZERO_IPADDRRESS                 "0.0.0.0"
A_STATUS apCfgGatewayAddrSet(int);
int apCfgGatewayAddrGet(void);
#define DEFAULT_GATEWAY					"0.0.0.0"
#define CFG_MAX_IPADDR                  20

A_STATUS apCfgIpAddrSet(int);
int apCfgIpAddrGet(void);
#define DEFAULT_IP_ADDR                 "192.168.1.245"

A_STATUS apCfgIpMaskSet(int);
int apCfgIpMaskGet(void);
#define DEFAULT_SUBNET_MASK             "255.255.255.0"

A_STATUS apCfgNameSrvSet(char *p);
char *apCfgNameSrvGet(void);
A_STATUS apCfgNameSrv2Set(char *p);
char *apCfgNameSrv2Get(void);
#define DEFAULT_DNSADDR              	"0.0.0.0"
#define MAX_NAME_SERVER                 32

A_STATUS apCfgIpv6AddrSet(char *addr);
char *apCfgIpv6AddrGet(void);
#define DEFAULT_IP6_ADDR               "1234::5678/64"

A_STATUS apCfgGatewayv6AddrSet(char *gw);
char *apCfgGatewayv6AddrGet(void);
#define DEFAULT_IP6_GATEWAY            "::/0"

char *apCfgNameSrv61Get(void);
A_STATUS apCfgNameSrv61Set(char *dns);
char *apCfgNameSrv62Get(void);
A_STATUS apCfgNameSrv62Set(char *dns);

A_STATUS apCfgDhcpServerStartSet(char *p);
char *apCfgDhcpServerStartGet(void);
A_STATUS apCfgDhcpServerEndSet(char *p);
char *apCfgDhcpServerEndGet(void);

A_STATUS apCfgNameDomainSet(char *p);
char *apCfgNameDomainGet(void);
#define DEFAULT_DOMAINSUFFIX            ""
#define CFG_MAX_DOMAINNMLEN             64

/*
 * Country/Region Codes from MS WINNLS.H
 * Numbering from ISO 3166
 */
enum CountryCode {
    CTRY_DEFAULT			  = 0,
    CTRY_ALBANIA              = 8,       /* Albania */
    CTRY_ALGERIA              = 12,      /* Algeria */
    CTRY_ARGENTINA            = 32,      /* Argentina */
    CTRY_ARMENIA              = 51,      /* Armenia */
    CTRY_AUSTRALIA            = 36,      /* Australia */
    CTRY_AUSTRIA              = 40,      /* Austria */
    CTRY_AZERBAIJAN           = 31,      /* Azerbaijan */
    CTRY_BAHRAIN              = 48,      /* Bahrain */
    CTRY_BELARUS              = 112,     /* Belarus */
    CTRY_BELGIUM              = 56,      /* Belgium */
    CTRY_BELIZE               = 84,      /* Belize */
    CTRY_BOLIVIA              = 68,      /* Bolivia */
    CTRY_BRAZIL               = 76,      /* Brazil */
    CTRY_BRUNEI_DARUSSALAM    = 96,      /* Brunei Darussalam */
    CTRY_BULGARIA             = 100,     /* Bulgaria */
    CTRY_CANADA               = 124,     /* Canada */
    CTRY_CHILE                = 152,     /* Chile */
    CTRY_CHINA                = 156,     /* People's Republic of China */
    CTRY_COLOMBIA             = 170,     /* Colombia */
    CTRY_COSTA_RICA           = 188,     /* Costa Rica */
    CTRY_CROATIA              = 191,     /* Croatia */
    CTRY_CYPRUS               = 196,
    CTRY_CZECH                = 203,     /* Czech Republic */
    CTRY_DENMARK              = 208,     /* Denmark */
    CTRY_DOMINICAN_REPUBLIC   = 214,     /* Dominican Republic */
    CTRY_ECUADOR              = 218,     /* Ecuador */
    CTRY_EGYPT                = 818,     /* Egypt */
    CTRY_EL_SALVADOR          = 222,     /* El Salvador */
    CTRY_ESTONIA              = 233,     /* Estonia */
    CTRY_FAEROE_ISLANDS       = 234,     /* Faeroe Islands */
    CTRY_FINLAND              = 246,     /* Finland */
    CTRY_FRANCE               = 250,     /* France */
    CTRY_FRANCE2              = 255,     /* France2 */
    CTRY_GEORGIA              = 268,     /* Georgia */
    CTRY_GERMANY              = 276,     /* Germany */
    CTRY_GREECE               = 300,     /* Greece */
    CTRY_GUATEMALA            = 320,     /* Guatemala */
    CTRY_HONDURAS             = 340,     /* Honduras */
    CTRY_HONG_KONG            = 344,     /* Hong Kong S.A.R., P.R.C. */
    CTRY_HUNGARY              = 348,     /* Hungary */
    CTRY_ICELAND              = 352,     /* Iceland */
    CTRY_INDIA                = 356,     /* India */
    CTRY_INDONESIA            = 360,     /* Indonesia */
    CTRY_IRAN                 = 364,     /* Iran */
    CTRY_IRAQ                 = 368,     /* Iraq */
    CTRY_IRELAND              = 372,     /* Ireland */
    CTRY_ISRAEL               = 376,     /* Israel */
    CTRY_ITALY                = 380,     /* Italy */
    CTRY_JAMAICA              = 388,     /* Jamaica */
    CTRY_JAPAN                = 392,     /* Japan */
    CTRY_JAPAN1               = 393,     /* Japan (JP1) */
    CTRY_JAPAN2               = 394,     /* Japan (JP0) */
    CTRY_JAPAN3               = 395,     /* Japan (JP1-1) */
    CTRY_JAPAN4               = 396,     /* Japan (JE1) */
    CTRY_JAPAN5               = 397,     /* Japan (JE2) */
    CTRY_JAPAN6               = 4006,     /* Japan (J6) */

    CTRY_JAPAN7		      = 4007,	 /* Japan (J7) */
    CTRY_JAPAN8		      = 4008,	 /* Japan (J8) */
    CTRY_JAPAN9		      = 4009,	 /* Japan (J9) */

    CTRY_JAPAN10	      = 4010,	 /* Japan (J10) */
    CTRY_JAPAN11	      = 4011,	 /* Japan (J11) */
    CTRY_JAPAN12	      = 4012,	 /* Japan (J12) */

    CTRY_JAPAN13	      = 4013,	 /* Japan (J13) */
    CTRY_JAPAN14	      = 4014,	 /* Japan (J14) */
    CTRY_JAPAN15	      = 4015,	 /* Japan (J15) */

    CTRY_JAPAN16	      = 4016,	 /* Japan (J16) */
    CTRY_JAPAN17	      = 4017,	 /* Japan (J17) */
    CTRY_JAPAN18	      = 4018,	 /* Japan (J18) */

    CTRY_JAPAN19	      = 4019,	 /* Japan (J19) */
    CTRY_JAPAN20	      = 4020,	 /* Japan (J20) */
    CTRY_JAPAN21	      = 4021,	 /* Japan (J21) */

    CTRY_JAPAN22	      = 4022,	 /* Japan (J22) */
    CTRY_JAPAN23	      = 4023,	 /* Japan (J23) */
    CTRY_JAPAN24	      = 4024,	 /* Japan (J24) */
 
    CTRY_JAPAN25	      = 4025,	 /* Japan (J25) */
    CTRY_JAPAN26	      = 4026,	 /* Japan (J26) */
    CTRY_JAPAN27	      = 4027,	 /* Japan (J27) */
    CTRY_JAPAN28	      = 4028,	 /* Japan (J28) */
    CTRY_JAPAN29	      = 4029,	 /* Japan (J29) */
    CTRY_JAPAN30	      = 4030,	 /* Japan (J30) */
    CTRY_JAPAN31	      = 4031,	 /* Japan (J31) */
    CTRY_JAPAN32	      = 4032,	 /* Japan (J32) */
    CTRY_JAPAN33	      = 4033,	 /* Japan (J33) */
    CTRY_JAPAN34	      = 4034,	 /* Japan (J34) */
    CTRY_JAPAN35	      = 4035,	 /* Japan (J35) */

    CTRY_JAPAN36          = 4036,    /* Japan (J36) */
    CTRY_JAPAN37          = 4037,    /* Japan (J37) */
    CTRY_JAPAN38          = 4038,    /* Japan (J38) */
    CTRY_JAPAN39          = 4039,    /* Japan (J39) */
    CTRY_JAPAN40          = 4040,    /* Japan (J40) */
    CTRY_JAPAN41          = 4041,    /* Japan (J41) */
    CTRY_JAPAN42          = 4042,    /* Japan (J42) */
    CTRY_JAPAN43          = 4043,    /* Japan (J43) */
    CTRY_JAPAN44          = 4044,    /* Japan (J44) */
    CTRY_JAPAN45          = 4045,    /* Japan (J45) */
    CTRY_JAPAN46          = 4046,    /* Japan (J46) */
    CTRY_JAPAN47          = 4047,    /* Japan (J47) */
    CTRY_JAPAN48          = 4048,    /* Japan (J48) */
    CTRY_JAPAN49          = 4049,    /* Japan (J49) */
    CTRY_JAPAN50          = 4050,    /* Japan (J50) */
    CTRY_JAPAN51          = 4051,    /* Japan (J51) */
    CTRY_JAPAN52          = 4052,    /* Japan (J52) */
    CTRY_JAPAN53          = 4053,    /* Japan (J53) */
    CTRY_JAPAN54          = 4054,    /* Japan (J54) */
    CTRY_JAPAN55          = 4055,    /* Japan (J55) */
    CTRY_JAPAN56          = 4056,    /* Japan (J56) */

    CTRY_JAPAN57          = 4057,    /* Japan (J57) */
    CTRY_JAPAN58          = 4058,    /* Japan (J58) */
    CTRY_JAPAN59          = 4059,    /* Japan (J59) */

    CTRY_AUSTRALIA2           = 5000,    /* Australia */
    CTRY_CANADA2              = 5001,    /* Canada */

    CTRY_JORDAN               = 400,     /* Jordan */
    CTRY_KAZAKHSTAN           = 398,     /* Kazakhstan */
    CTRY_KENYA                = 404,     /* Kenya */
    CTRY_KOREA_NORTH          = 408,     /* North Korea */
    CTRY_KOREA_ROC            = 410,     /* South Korea */
    CTRY_KOREA_ROC2           = 411,     /* South Korea */
    CTRY_KOREA_ROC3           = 412,     /* South Korea */
    CTRY_KUWAIT               = 414,     /* Kuwait */
    CTRY_LATVIA               = 428,     /* Latvia */
    CTRY_LEBANON              = 422,     /* Lebanon */
    CTRY_LIBYA                = 434,     /* Libya */
    CTRY_LIECHTENSTEIN        = 438,     /* Liechtenstein */
    CTRY_LITHUANIA            = 440,     /* Lithuania */
    CTRY_LUXEMBOURG           = 442,     /* Luxembourg */
    CTRY_MACAU                = 446,     /* Macau */
    CTRY_MACEDONIA            = 807,     /* the Former Yugoslav Republic of Macedonia */
    CTRY_MALAYSIA             = 458,     /* Malaysia */
    CTRY_MALTA		      = 470,	 /* Malta */
    CTRY_MEXICO               = 484,     /* Mexico */
    CTRY_MONACO               = 492,     /* Principality of Monaco */
    CTRY_MOROCCO              = 504,     /* Morocco */
    CTRY_NETHERLANDS          = 528,     /* Netherlands */
    CTRY_NETHERLANDS_ANTILLES = 530,     /* Netherlands Antilles */
    CTRY_NEW_ZEALAND          = 554,     /* New Zealand */
    CTRY_NICARAGUA            = 558,     /* Nicaragua */
    CTRY_NORWAY               = 578,     /* Norway */
    CTRY_OMAN                 = 512,     /* Oman */
    CTRY_PAKISTAN             = 586,     /* Islamic Republic of Pakistan */
    CTRY_PANAMA               = 591,     /* Panama */
    CTRY_PARAGUAY             = 600,     /* Paraguay */
    CTRY_PERU                 = 604,     /* Peru */
    CTRY_PHILIPPINES          = 608,     /* Republic of the Philippines */
    CTRY_POLAND               = 616,     /* Poland */
    CTRY_PORTUGAL             = 620,     /* Portugal */
    CTRY_PUERTO_RICO          = 630,     /* Puerto Rico */
    CTRY_QATAR                = 634,     /* Qatar */
    CTRY_ROMANIA              = 642,     /* Romania */
    CTRY_RUSSIA               = 643,     /* Russia */
    CTRY_SAUDI_ARABIA         = 682,     /* Saudi Arabia */
    CTRY_SERBIA_MONTENEGRO    = 891,     /* Serbia and Montenegro */
    CTRY_SINGAPORE            = 702,     /* Singapore */
    CTRY_SLOVAKIA             = 703,     /* Slovak Republic */
    CTRY_SLOVENIA             = 705,     /* Slovenia */
    CTRY_SOUTH_AFRICA         = 710,     /* South Africa */
    CTRY_SPAIN                = 724,     /* Spain */
    CTRY_SRI_LANKA	      = 144,     /* Sri Lanka */
    CTRY_SWEDEN               = 752,     /* Sweden */
    CTRY_SWITZERLAND          = 756,     /* Switzerland */
    CTRY_SYRIA                = 760,     /* Syria */
    CTRY_TAIWAN               = 158,     /* Taiwan */
    CTRY_THAILAND             = 764,     /* Thailand */
    CTRY_TRINIDAD_Y_TOBAGO    = 780,     /* Trinidad y Tobago */
    CTRY_TUNISIA              = 788,     /* Tunisia */
    CTRY_TURKEY               = 792,     /* Turkey */
    CTRY_UAE                  = 784,     /* U.A.E. */
    CTRY_UKRAINE              = 804,     /* Ukraine */
    CTRY_UNITED_KINGDOM       = 826,     /* United Kingdom */
    CTRY_UNITED_STATES        = 840,     /* United States */
    CTRY_UNITED_STATES_FCC49  = 842,     /* United States (Public Safety)*/
    CTRY_URUGUAY              = 858,     /* Uruguay */
    CTRY_UZBEKISTAN           = 860,     /* Uzbekistan */
    CTRY_VENEZUELA            = 862,     /* Venezuela */
    CTRY_VIET_NAM             = 704,     /* Viet Nam */
    CTRY_YEMEN                = 887,     /* Yemen */
    CTRY_ZIMBABWE             = 716      /* Zimbabwe */
};


typedef struct sc_country_s{
    char *countryName;
    int countryCode;
}SC_COUNTRY;

extern SC_COUNTRY allCountryList[];

A_STATUS apCfgCountryCodeSet(int);
int apCfgCountryCodeGet(void);
#define 	DEFAULT_COUNTRYCODE			CTRY_UNITED_STATES

A_STATUS apCfgSecMaskSet(int v);
A_BOOL apCfgSecMaskGet(void);

A_STATUS apCfgWinsServerSet(char *);
char *   apCfgWinsServerGet(void);
#define 	DEFAULT_WINS_SERVER       	""
#define     MAX_WINS_SERVER          32

A_STATUS apCfgCertSet(char *);
char *   apCfgCertGet(void);
A_STATUS apCfgCertTimeSet(char *);
char *   apCfgCertTimeGet(void);
#define     MAX_CERT_CONTENT    8192
#define     MAX_CERT_TIME       32
#define     DEFAULT_PATH        "/etc/cert/"
#define     DEST_PATH           "/var/"
#define     PEM_NAME            "wap4410n.pem"
#define     PEM_FILE            DEST_PATH PEM_NAME
#define     CERT_FILE           DEST_PATH "certSrv.pem"
#define     KEY_FILE            DEST_PATH  "privkeySrv.pem"
#define     CERT_FLAG           "-----BEGIN CERTIFICATE-----"
#define     KEY_FLAG            "-----BEGIN RSA PRIVATE KEY-----"

#define     IMG_NAME            "wap4410n.img"
#define     IMG_FILE            DEST_PATH IMG_NAME
#define     CFG_NAME            "wap4410n.cfg"
#define     CFG_FILE            DEST_PATH CFG_NAME

A_STATUS apCfgTimeModeSet(int v);
int apCfgTimeModeGet(void);

A_STATUS apCfgTimeSet(int v);
int apCfgTimeGet(void);
A_STATUS apCfgTimeMonSet(int v);
int apCfgTimeMonGet(void); 
A_STATUS apCfgTimeDaySet(int v);
int apCfgTimeDayGet(void);
A_STATUS apCfgTimeYearSet(int v);
int apCfgTimeYearGet(void);
A_STATUS apCfgTimeHourSet(int v);
int apCfgTimeHourGet(void);
A_STATUS apCfgTimeMinSet(int v);
int apCfgTimeMinGet(void);
A_STATUS apCfgTimeSecSet(int v);
int apCfgTimeSecGet(void);


A_STATUS apCfgNtpModeSet(int v);
int apCfgNtpModeGet(void);
A_STATUS apCfgNtpServerSet(char *);
char *   apCfgNtpServerGet(void);
#define 	DEFAULT_NTP_SERVER       	""
#define     MAX_NTP_SERVER          32

A_STATUS apCfgDaylightSavingSet(int v);
int apCfgDaylightSavingGet(void);

A_STATUS apCfgTimezoneOffsetSet(char *);
char *  apCfgTimezoneOffsetGet(void);
#define CFG_TIME_ZONE_LEN               12

A_STATUS apCfgFtpServerSet(char *);
char *   apCfgFtpServerGet(void);
#define 	DEFAULT_FTP_SERVER       	""
#define     MAX_FTP_SERVER          32

A_STATUS apCfgFtpPathSet(char *);
char *   apCfgFtpPathGet(void);

A_STATUS apCfgFtpLoginSet(char *);
char *   apCfgFtpLoginGet(void);
#define DEFAULT_FTP_LOGINNAME                ""
#define CFG_MAX_FTP_LOGINNAME                16

A_STATUS apCfgFtpPasswdSet(char *);
char *   apCfgFtpPasswdGet(void);
#define DEFAULT_FTP_PASSWORD                ""
#define CFG_MAX_FTP_PASSWORD                16

A_STATUS apCfgVlanModeSet(int);
int apCfgVlanModeGet(void);
#define DEFAULT_VLAN_MODE               FALSE

A_STATUS apCfgNativeVlanIdSet(int v);
int apCfgNativeVlanIdGet(void);
A_STATUS apCfgNativeVlanTagSet(int v);
int apCfgNativeVlanTagGet(void);

A_STATUS apCfgWdsVlanTagSet(int v);
int apCfgWdsVlanTagGet(void);

A_STATUS apCfgManagementVlanIdSet(int v);
int apCfgManagementVlanIdGet(void);

A_UINT32 apCfgsysLogEnabledGet(void);
A_STATUS apCfgsysLogEnabledSet(A_UINT8 v);
A_UINT32 apCfgsysLogBroadcastGet(void);
A_STATUS apCfgsysLogBroadcastSet(A_UINT8 v);
A_UINT32 apCfgsysLogSeverityGet(void);
A_STATUS apCfgsysLogSeveritySet(A_UINT8 v);
A_UINT8 *apCfgsysLogServerGet(void);
A_STATUS apCfgsysLogServerSet(char *p); 
#define  MAX_SYSLOG_SERVER      32
A_UINT16 apCfgsysLogServerPortGet(void);
A_STATUS apCfgsysLogServerPortSet(A_UINT32 v);

//SMTP Email Alerts
A_UINT32 apCfgemailAlertsEnabledGet(void);
A_STATUS apCfgemailAlertsEnabledSet(A_BOOL v);
A_STATUS scApCfgemailAlertsQlenSet(int v);
int apCfgemailAlertsQlenGet(void);
#define  MAX_ALERT_QLEN 500
#define  MIN_ALERT_QLEN 1
A_STATUS scApCfgemailAlertsIntervalSet(int v);
int scApCfgemailAlertsIntervalGet(void);
#define  MAX_ALERT_INTERVAL 600
#define  MIN_ALERT_INTERVAL 1
A_UINT8 *apCfgsmtpMailServerGet(void);
A_STATUS apCfgsmtpMailServerSet(char *p);
#define  MAX_SMTP_SERVER      64
A_UINT8 *apCfgemailAddrForLogGet(void);
A_STATUS apCfgemailAddrForLogSet(char *p);
A_UINT8 *apCfgemailAddrReturnGet(void);
A_STATUS apCfgemailAddrReturnSet(char *p);
#define  MAX_MAIL_SERVER      64

//send log type
A_BOOL apCfgDeauthGet(void);
A_STATUS apCfgDeauthSet(A_BOOL v);
A_BOOL apCfgAuthLoginGet(void);
A_STATUS apCfgAuthLoginSet(A_BOOL v);
A_BOOL apCfgChangeSysFucGet(void);
A_STATUS apCfgChangeSysFucSet(A_BOOL v);
A_BOOL apCfgChangeCfgGet(void);
A_STATUS apCfgChangeCfgSet(A_BOOL v);

A_STATUS scApCfgDot1xSuppEnableSet(A_BOOL v);
A_BOOL scApCfgDot1xSuppEnableGet(void);
A_STATUS scApCfgDot1xSuppMacEnableSet(A_BOOL v);
A_BOOL scApCfgDot1xSuppMacEnableGet(void);
A_STATUS scApCfgDot1xSuppUsernameSet(char *p);
char * scApCfgDot1xSuppUsernameGet(void);
#define MAX_DOT1XSUPP_NAME   63

A_STATUS scApCfgDot1xSuppPasswordSet(char *p);
char * scApCfgDot1xSuppPasswordGet(void);
#define MAX_DOT1XSUPP_PASSWD   63

A_STATUS apCfgHttpModeSet(A_BOOL v);
A_BOOL apCfgHttpModeGet(void);
A_STATUS apCfgHttpPortSet(A_UINT16 v);
A_UINT16 apCfgHttpPortGet(void);
A_STATUS apCfgHttpsModeSet(A_BOOL v);
A_BOOL apCfgHttpsModeGet(void);
A_STATUS apCfgHttpsPortSet(A_UINT16 v);
A_UINT16 apCfgHttpsPortGet(void);
A_STATUS apCfgAutohttpsModeSet(A_BOOL v);
A_BOOL apCfgAutohttpsModeGet(void);
A_STATUS apCfgWlanAccessSet(A_BOOL v);
A_BOOL apCfgWlanAccessGet(void);
A_STATUS apCfgSSHSet(A_BOOL v);
A_BOOL apCfgSSHGet(void);
A_STATUS apCfgTelnetModeSet(A_BOOL v);
A_BOOL apCfgTelnetModeGet(void);
A_STATUS apCfgTelnetTimeoutSet(A_UINT32 v);
A_UINT32 apCfgTelnetTimeoutGet(void);
#define MAX_TELNET_TIMEOUT      600  //minutes
#define MIN_TELENT_TIMEOUT      1   //minutes

A_STATUS apCfgSnmpModeSet(A_BOOL v);
A_BOOL apCfgSnmpModeGet(void);
/**/
A_STATUS apCfgSnmpContactSet(char *p);
char * apCfgSnmpContactGet(void);
#define MAX_SNMP_CONTACT    32
A_STATUS apCfgSnmpDviceNameSet(char *p);
char * apCfgSnmpDviceNameGet(void);
#define MAX_SNMP_DEVNAME    32
A_STATUS apCfgSnmpLocationSet(char *p);
char * apCfgSnmpLocationGet(void);
#define MAX_SNMP_LOCATION    32
A_STATUS apCfgSnmpTrapCommunitySet(char *p);
char * apCfgSnmpTrapCommunityGet(void);
/**/
A_STATUS apCfgSnmpReadCommSet(char *p);
char * apCfgSnmpReadCommGet(void);
#define MAX_SNMP_COMMUNITY  32
A_STATUS apCfgSnmpWriteCommSet(char *p);
char * apCfgSnmpWriteCommGet(void);
A_STATUS apCfgSnmpUserNameSet(char *p);
char * apCfgSnmpUserNameGet(void);
#define MAX_SNMPV3_USERNAME  63

A_STATUS apCfgSnmpAuthProtocolSet(int v);
int apCfgSnmpAuthProtocolGet(void);
#define SNMP_AUTH_NONE  0
#define SNMP_AUTH_MD5   1
A_STATUS apCfgSnmpAuthKeySet(char *p);
char * apCfgSnmpAuthKeyGet(void);
#define MIN_SNMPV3_AUTHKEY  8
#define MAX_SNMPV3_AUTHKEY  63


A_STATUS apCfgSnmpPrivProtocolSet(int v);
int apCfgSnmpPrivProtocolGet(void);
    #define SNMP_PRIV_NONE  0
    #define SNMP_PRIV_DES   1
A_STATUS apCfgSnmpPrivKeySet(char *p);
char * apCfgSnmpPrivKeyGet(void);
#define MIN_SNMPV3_PRIVKEY  8
#define MAX_SNMPV3_PRIVKEY  32

A_STATUS apCfgSnmpAnyManagerSet(A_BOOL v);
A_BOOL apCfgSnmpAnyManagerGet(void);
A_STATUS apCfgSnmpManagerIpSet(int v);
int apCfgSnmpManagerIpGet(void);
A_STATUS apCfgSnmpManagerIpEndSet(int v);
int apCfgSnmpManagerIpEndGet(void);
A_STATUS apCfgSnmpTrapVersionSet(A_UINT32 v);
A_UINT32 apCfgSnmpTrapVersionGet(void);
A_STATUS apCfgSnmpTrapRecvIpSet(int v);
int apCfgSnmpTrapRecvIpGet(void);

#define DEFAULT_TRAP_PORT   162
A_STATUS apCfgSnmpTrapPortSet(A_UINT32 v);
A_UINT32 apCfgSnmpTrapPortGet(void);
A_BOOL  isInvalidSnmpTrapPort(A_UINT32 v);

/********************************************************/
/*		Get/Set function on each radio	 				*/	
/********************************************************/
int apCfgNumVapsGet(int unit);
A_STATUS apCfgNumVapsSet(int unit, int v);
#define DEFAULT_VAPS                    WLAN_MAX_VAP

A_STATUS apCfgOpModeSet(int unit, int);
int apCfgOpModeGet(int unit);
#define CFG_OP_MODE_AP                  0
#define CFG_OP_MODE_STA                 1
#define CFG_OP_MODE_PPT                 2
#define CFG_OP_MODE_MPT                 3
#define CFG_OP_MODE_REPEATER            4
#define CFG_OP_MODE_AP_PTP              5
#define CFG_OP_MODE_AP_PTMP             6
#define CFG_OP_MODE_UC                  7
#define CFG_OP_MODE_UR                  8
#define CFG_OP_MODE_ROGAP               9
#define DEFAULT_OP_MODE                 CFG_OP_MODE_AP

A_STATUS apCfgAutoChannelSet(int, int);
int apCfgAutoChannelGet(int);
A_STATUS apCfgWpsModeSet(int, int);
int apCfgWpsModeGet(int);

A_STATUS apCfgWpsPinERSet(int unit, int v);
int apCfgWpsPinERGet(int unit);

A_STATUS apCfgFreqSpecSet(int unit, int);
int apCfgFreqSpecGet(int unit);

#define MODE_SELECT_AUTO      			0
#define MODE_SELECT_11A      			1
#define MODE_SELECT_11B      			2
#define MODE_SELECT_11G      			3
#define MODE_SELECT_FH      			4
#define MODE_SELECT_TURBOA    		    5
#define MODE_SELECT_TURBOG    		    6
#define MODE_SELECT_11NA      			7
#define MODE_SELECT_11NG      			8
#define MODE_SELECT_11N      			9
#define MODE_SELECT_11BG      			10
#define MODE_SELECT_11BGN      			11
#define DEFAULT_WIRELESS_MODE           MODE_SELECT_11BGN

A_STATUS apCfgChannelWidthModeSet(int unit, int v);
int apCfgChannelWidthModeGet(int unit);
#define CWM_MODE_20M                    0
#define CWM_MODE_20M_40M                1
#define CWM_MODE_40M                    2
#define DEFAULT_CWM_MODE                CWM_MODE_20M_40M

A_STATUS apCfgRadioChannelSet(int unit, int);
int apCfgRadioChannelGet(int unit);

A_STATUS apCfgChannelOffsetSet(int unit,char *p);
char *apCfgChannelOffsetGet(int unit);

A_STATUS scApCfgAmpduSet(int unit,int v);
int scApCfgAmpduGet(int unit);

A_STATUS scApCfgAmsduSet(int unit,int v);
int scApCfgAmsduGet(int unit);

A_STATUS  scApCfgShortGISet(int unit,int v);
int scApCfgShortGIGet(int unit);
#define SHORTGI_SHORT                   0
#define SHORTGI_AUTO                    1
#define SHORTGI_LONG                    2

A_STATUS apCfgAclModeSet(int unit, int bss, int);
int apCfgAclModeGet(int unit, int bss);
A_STATUS apCfgAclTypeSet(int unit, int bss, int v);
int apCfgAclTypeGet(int unit, int bss);
#define APCFG_ACL_DISABLED              0
#define APCFG_ACL_LOCAL                 1
#define APCFG_ACL_RADIUS                2
#define DEFAULT_ACL_MODE                APCFG_ACL_DISABLED
#define APCFG_ACL_ALLOW                 0
#define APCFG_ACL_PREVENT               1
#define DEFAULT_APCFG_ACL               APCFG_ACL_ALLOW
#define CFG_MIN_ACL                     1
#define CFG_MAX_ACL                     256
#define CFG_MAX_ACL_LINKSYS             20
typedef struct scAclBuf_s
{
    A_INT8			    name[16];
    char                mac[18];
    char                used;
 	struct scAclBuf_s	*next;
}CFG_ACL_ENTRY;

A_STATUS apCfgAclAdd(A_UINT32 unit, int bss, char *pMacAddr, char *name, int used);
A_STATUS apCfgAclDel(A_UINT32 unit, int bss, char *pMac);
void apCfgAclClear(A_UINT32 unit, int bss);

extern struct 	scAclBuf_s scAclBuf[WLAN_MAX_DEV][WLAN_MAX_VAP];
A_BOOL	scAclBufGet(A_UINT32 unit, int bss, struct scAclBuf_s **pScAcl);
void	scAclBufFree(A_UINT32 unit, int bss, struct scAclBuf_s *pScAcl);


A_STATUS scApCfg80211dEnabledSet(int unit, int v);
int scApCfg80211dEnabledGet(int unit);

A_STATUS apCfgPrioritySet(int unit,int bss,int v);
int apCfgPriorityGet(int unit,int bss);
#define DEFAULT_PRIORITY                 0

A_STATUS apCfgWmeSet(int unit,int bss, int);
int   apCfgWmeGet(int unit,int bss);
#define DEFAULT_WME_MODE                 TRUE

A_STATUS apCfgWmmpsSet(int unit,int bss, int);
int   apCfgWmmpsGet(int unit,int bss);
#define DEFAULT_WMMPS_MODE                 TRUE

A_STATUS apCfgNoAckSet(int unit, int bss, int);
int   apCfgNoAckGet(int unit, int bss);
#define DEFAULT_NO_ACK                 	 FALSE

A_STATUS scApCfgIdleTimeoutIntervalSet(int unit, int v);
int scApCfgIdleTimeoutIntervalGet(int unit);
#define SC_DEFAULT_IDLETIMEOUT_INTERVAL		(5)
#define SC_MIN_IDLETIMEOUT_INTERVAL			(0)
#define SC_MAX_IDLETIMEOUT_INTERVAL			(99)

A_STATUS scApCfgDtimIntervalSet(int unit, int v);
int scApCfgDtimIntervalGet(int unit);
#define SC_DEFAULT_DTIM_INTERVAL		(1)
#define SC_MIN_DTIM_INTERVAL			(1)
#define SC_MAX_DTIM_INTERVAL			(255)

A_STATUS apCfgFragThresholdSet(int unit, int);
int apCfgFragThresholdGet(int unit);
#define DEFAULT_FRAG_THRESHOLD          2346
#define MIN_FRAG_THRESHOLD              256
#define MAX_FRAG_THRESHOLD              2346

A_STATUS apCfgBeaconIntervalSet(int unit, int);
A_BOOL apCfgBeaconIntervalGet(int unit);
A_STATUS apCfgCTSModeSet(int unit, int v);
A_BOOL apCfgCTSModeGet(int unit);

#define DEFAULT_BEACON_PERIOD           100
#define MIN_BEACON_PERIOD               20
#define MAX_BEACON_PERIOD               1000

A_STATUS apCfgRtsThresholdSet(int unit, int);
int apCfgRtsThresholdGet(int unit);
#define MIN_RTS_THRESHOLD               1
#define MAX_RTS_THRESHOLD               2347
#define DEFAULT_RTS_THRESHOLD           MAX_RTS_THRESHOLD

A_STATUS apCfgShortPreambleSet(int, int);
int   apCfgShortPreambleGet(int);

A_STATUS apCfgAutoPowerSet(int unit, int v);
int apCfgAutoPowerGet(int unit);
A_STATUS apCfgPowerSet(int unit, int);
int apCfgPowerGet(int unit);
#define CLI_AUTO_POWER      			100
#define CLI_MAX_POWER       			100
#define CLI_MIN_POWER       			-100
#define DEFAULT_TXPOWER                 100               


A_STATUS apCfgAntennaSet(int unit, int);
int apCfgAntennaGet(int unit);
#define  DEFAULT_ANTENNA                0

A_STATUS apCfgWlanStateSet(int unit, int);
int   apCfgWlanStateGet(int unit);
#define DEFAULT_WLAN_STATE              TRUE

A_STATUS apCfgRateSet(int unit, char *);
char *   apCfgRateGet(int unit);
#define DEFAULT_DATA_RATE               "best"

A_STATUS apCfgBalanceModeSet(int unit, int v);
int apCfgBalanceModeGet(int unit);
A_STATUS apCfgLoadBalanceSet(int unit, int vap, int v);
int apCfgLoadBalanceGet(int unit, int vap);
#define MIN_BALANCE                     0
#define MAX_BALANCE                     10000
#define DEFAULT_BALANCE                 MIN_BALANCE

A_STATUS apCfgInterVapForwardingSet(int, int);
int apCfgInterVapForwardingGet(int);
#define DEFAULT_INTER_VAP_FORWARDING    TRUE

A_STATUS apCfgRemoteApMacAddrSet(int unit, char *pRemoteApMacAddr);
A_STATUS apCfgRemoteApMacAddrGet(int unit, char *pRemoteApMacAddr);
A_STATUS apCfgRemoteWbrMacAddrGet(int unit, int, A_UCHAR *);
A_STATUS apCfgRemoteWbrMacAddrSet(int unit, int, A_UCHAR *);
/* mike add for operation mode ucr */
A_STATUS apCfgUcrRemoteApMacAddrSet(int unit, char *pRemoteApMacAddr);
A_STATUS apCfgUcrRemoteApMacAddrGet(int unit, char *pRemoteApMacAddr);
/* added end */
//A_STATUS apCfgRemoteWbrMacAddrAdd(int unit, A_UCHAR *);
//A_STATUS apCfgRemoteWbrMacAddrDel(int unit, A_UCHAR *);
//A_STATUS apCfgRemoteWbrMacAddrClear(int unit);
#define  MAX_REMOTE_WBR   8
#define DEFAULT_REMOTE_AP_MACADDR   {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}


/********************************************************/
/*		Get/Set function on each vap	 				*/	
/********************************************************/
A_STATUS apCfgVapNameSet(int unit, int bss, char *p);
char *apCfgVapNameGet(int unit, int bss);
#define DEFAULT_VAPNAME					"NETGEAR"
#define MAX_VAPNAME						32

A_STATUS apCfgActiveModeSet(int unit, int bss, int);
int apCfgActiveModeGet(int unit, int bss);
#define DEFAULT_VAP_ACTIVE_MODE         TRUE


A_STATUS apCfgSsidSet(int unit, int bss, char *p);
char *apCfgSsidGet(int unit, int bss);
#define DEFAULT_SSID                    "NETGEAR"
#define MAX_SSID                        32

char *apCfgDevicePinGet(int unit, int bss);
A_STATUS apCfgDevicePinSet(int unit, int bss, char *p);
#define WPS_PIN_LEN         8

A_STATUS apCfgSsidModeSet(int unit, int bss, int);
int apCfgSsidModeGet(int unit, int bss);
#define DEFAULT_SSID_SUPPRESS_MODE      FALSE

A_STATUS apCfgAuthTypeSet(int unit, int bss, int);
int apCfgAuthTypeGet(int unit, int bss);
enum {
    APCFG_AUTH_NONE = 0,
    APCFG_AUTH_OPEN_SYSTEM,
    APCFG_AUTH_SHARED_KEY,
    APCFG_AUTH_AUTO,
    APCFG_AUTH_WPA,
    APCFG_AUTH_WPAPSK,
    APCFG_AUTH_WPA2,
    APCFG_AUTH_WPA2PSK,
    APCFG_AUTH_WPA_AUTO,
    APCFG_AUTH_WPA_AUTO_PSK,
    APCFG_AUTH_DOT1X
};
#define DEFAULT_AUTH_TYPE               APCFG_AUTH_NONE


A_STATUS apCfgWPACipherSet(int unit, int bss,int v);
int apCfgWPACipherGet(int unit, int bss);
enum 
{
    WPA_CIPHER_TKIP = 0,
    WPA_CIPHER_AES,
    WPA_CIPHER_AUTO
};
#define DEFAULT_WPA_CIPHER               WPA_CIPHER_AUTO

A_STATUS apCfgKeyValSet(int unit, int bss, int keyId, char *p);
char *apCfgKeyValGet(int unit, int bss, int keyId);
#define DEFAULT_KEY_VALUE               ""

A_STATUS apCfgKeyBitLenSet(int unit, int bss, int);
int apCfgKeyBitLenGet(int unit, int bss);
#define SC_DEFAULT_KEY_BITLEN        	SC_MIN_KEY_BITLEN
#define SC_MIN_KEY_BITLEN        		40
#define SC_MAX_KEY_BITLEN        		128//104
#define MAX_KEY_LEN_BYTES       		16
#define CFG_MIN_KEY                     1
#define CFG_MIN_SHARED_KEY              1
#define CFG_MAX_SHARED_KEY              4
#define CFG_MAX_KEY                     2048
#define INVALID_SHARED_KEY              (CFG_MAX_SHARED_KEY + 1)

A_STATUS apCfgDefKeySet(int, int bss, int);
int apCfgDefKeyGet(int, int bss);
#define DEFAULT_KEY                     0

A_STATUS apCfgKeyEntryMethodSet(int unit, int bss, int);
int apCfgKeyEntryMethodGet(int unit, int bss);
#define DEFAULT_KEY_ENTRY_METHOD        KEY_ENTRY_METHOD_HEX
#define KEY_ENTRY_METHOD_HEX            0
#define KEY_ENTRY_METHOD_ASCII          1

#define DEFAULT_PASSPHRASE              ""
#define PASSPHRASE_KEY_LEN      		40
#define MIN_PASSPHRASE_SIZE             8
#define CFG_MAX_PASSPHRASE              64
A_STATUS apCfgPassphraseSet(int unit, int bss, char *);
char *   apCfgPassphraseGet(int unit, int bss);

A_STATUS apCfgRadiusServerSet(int unit, int bss, char *p);
char * apCfgRadiusServerGet(int unit, int bss);
#define DEFAULT_RADIUSADDR              "0.0.0.0"
#define CFG_MAX_RADIUSNAME              64

A_STATUS apCfgBackupRadiusServerSet(int unit, int bss, char *p);
char * apCfgBackupRadiusServerGet(int unit, int bss);

A_STATUS apCfgRadiusPortSet(int unit, int bss, int v);
int apCfgRadiusPortGet(int unit, int bss);
#define DEFAULT_RADIUSPORT              1812

A_STATUS apCfgBackupRadiusPortSet(int unit, int bss, int v);
int apCfgBackupRadiusPortGet(int unit, int bss);

A_STATUS apCfgRadiusSecretSet(int unit, int bss, char *p);
char * apCfgRadiusSecretGet(int unit, int bss);
#define DEFAULT_RADIUSSECRET            ""
#define CFG_MAX_SECRETLEN               128

A_STATUS apCfgBackupRadiusSecretSet(int unit, int bss, char *p);
char * apCfgBackupRadiusSecretGet(int unit, int bss);

A_STATUS scApCfgAcctServerSet(int unit, int bss, char *p);
char * scApCfgAcctServerGet(int unit, int bss);

A_STATUS scApCfgBackupAcctServerSet(int unit, int bss, char *p);
char * scApCfgBackupAcctServerGet(int unit, int bss);

A_STATUS scApCfgAcctPortSet(int unit, int bss, int v);
int scApCfgAcctPortGet(int unit, int bss);
#define SC_DEFAULT_ACCTPORT				1813

A_STATUS scApCfgBackupAcctPortSet(int unit, int bss, int v);
int scApCfgBackupAcctPortGet(int unit, int bss);

A_STATUS scApCfgAcctSecretSet(int unit, int bss, char *p);
char * scApCfgAcctSecretGet(int unit,int bss);

A_STATUS scApCfgBackupAcctSecretSet(int unit, int bss, char *p);
char * scApCfgBackupAcctSecretGet(int unit,int bss);

A_STATUS apCfgDot1xKeyLenSet(int unit, int bss, int v);
int apCfgDot1xKeyLenGet(int unit, int bss);

A_STATUS apCfgDot1xKeyModeSet(int unit, int bss, int v);
int apCfgDot1xKeyModeGet(int unit, int bss);
#define DOT1X_MODE_STATIC               0x01
#define DOT1X_MODE_DYNAMIC              0x02

A_STATUS apCfgVlanPvidSet(int unit, int bss, int vlanTag);
int apCfgVlanPvidGet(int unit, int bss);
#define VLAN_TAG_MIN                      0x0001
#define VLAN_TAG_MAX                      0x0FFE
#define DEFAULT_VLAN_TAG                  VLAN_TAG_MIN
 
A_STATUS apCfgGroupKeyUpdateIntervalSet(int, int bss, int);
int apCfgGroupKeyUpdateIntervalGet(int, int bss);
#define DEFAULT_GROUP_KEY_UPDATE_INTERVAL  (60*60)  /* 60 mins */
#define MIN_GROUP_KEY_UPDATE_INTERVAL      (10*60)  /* Min = 30 secs, 0 = Groupkey update turned off */
#define MAX_GROUP_KEY_UPDATE_INTERVAL      (600*60)

A_STATUS apCfgIntraVapForwardingSet(int unit, int vap, int);
int apCfgIntraVapForwardingGet(int unit, int vap);
#define DEFAULT_INTRA_VAP_FORWARDING    TRUE

A_STATUS scApCfgGroupKeyUpdateEnabledSet(int unit, int bss, int v);
int  scApCfgGroupKeyUpdateEnabledGet(int unit, int bss);

A_STATUS scApCfgGroupKeyUpdateTerminatedSet(int unit, int bss, int v);
int  scApCfgGroupKeyUpdateTerminatedGet(int unit, int bss);

A_STATUS scApCfgAcctEnabledSet(int unit, int bss, int v);
int  scApCfgAcctEnabledGet(int unit, int bss);

A_STATUS scApCfgAcctUpdateEnabledSet(int unit, int bss, int v);
int  scApCfgAcctUpdateEnabledGet(int unit, int bss);

A_STATUS scApCfgAcctUpdateIntervalSet(int unit, int bss, int v);
int scApCfgAcctUpdateIntervalGet(int unit, int bss);
#define     MAX_ACCT_UPDATEINTERVAL     60
#define     MIN_ACCT_UPDATEINTERVAL     5    

A_STATUS	scApCfgRogueDetectSet(int v);
int  		scApCfgRogueDetectGet(void);
A_STATUS	scApCfgRogueDetectIntSet(char v);
char		scApCfgRogueDetectIntGet(void);
A_STATUS	scApCfgRogueDetectModeSet(char v);
char		scApCfgRogueDetectModeGet(void);
A_STATUS	scApCfgRogueApTypeSet(char v);
char		scApCfgRogueApTypeGet(void);
A_STATUS	scApCfgRogueSendLogSet(char v);
char		scApCfgRogueSendLogGet(void);
A_STATUS 	scApCfgLegalApListSet(char *p);
char *   	scApCfgLegalApListGet(void);
A_STATUS      scApCfgLegalApListAdd(char *p);
A_STATUS      scApCfgLegalApListDel(char *p);
A_STATUS    scApCfgLegalApListClear(void);
#define     MAX_LEGALAPLIST     (32*13+1)

/* the format of LegalApList: 00c002112344,10fd29332213,00c012******,... 
 * the list separater is ',' and all are uppercase.
 */
#define MAX_TRUST_AP			64
#define ROGUEAP_CONF            "/tmp/rogueap.conf"

A_STATUS 	apCfglltdSet(int v);
A_BOOL 		apCfglltdGet(void);
A_STATUS 	apCfgStpSet(int v);
A_BOOL 		apCfgStpGet(void);

A_STATUS apCfgRedirectModeSet(int v);
A_BOOL apCfgRedirectModeGet(void);
A_STATUS apCfgRedirectUrlSet(char *url);
char *apCfgRedirectUrlGet(void);
#define MAC_CFG_URL_LENGTH          256

A_STATUS apCfgFactoryRestore(void);
A_STATUS apCfgWscConfiguredSet(int v);
int apCfgWscConfiguredGet(void);

void apcfg_submit(void);


#define _BONJOUR_
#ifdef _BONJOUR_
//add by carole
A_STATUS apCfgBonjourSet(int v);
A_BOOL apCfgBonjourGet(void);
#endif

A_STATUS apCfgEthDataRateSet(int v);
A_BOOL apCfgEthDataRateGet(void);
A_STATUS apCfgForce100mSet(int v);
A_BOOL apCfgForce100mGet(void);
A_BOOL apCfgAutonegoSet(int v);
A_BOOL apCfgAutonegoGet(void);
A_BOOL apCfgPortspeedSet(int v);
A_BOOL apCfgPortspeedGet(void);
A_BOOL apCfgDuplexmodeSet(int v);
A_BOOL apCfgDuplexmodeGet(void);

A_BOOL apCfgMultiEnhanceSet(int v);
A_BOOL apCfgMultiEnhanceGet(void);

int apCfgAutoRebootModeGet(void);
A_STATUS apCfgAutoRebootModeSet(A_BOOL v);
A_STATUS scApCfgAutoRebootIntervalSet(int v);
int apCfgAutoRebootIntervalGet(void);

A_STATUS scApCfgAutoRebootTimeSet(int hour, int min);
A_STATUS apCfgAutoRebootTimeGet(int *hour, int *min);

#define CFG_MIN_REBOOT_TIME        1
#define CFG_MAX_REBOOT_TIME        720


#endif


