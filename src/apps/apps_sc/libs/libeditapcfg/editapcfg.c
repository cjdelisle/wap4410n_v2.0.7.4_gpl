/*
Copyright (c) 2005 SerComm Corporation.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
* Neither name of Intel Corporation nor the names of its contributors
  may be used to endorse or promote products derived from this software
  without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>
#include "apcfg.h"
#include "utility.h"
#include "editapcfg.h"
#include <time.h>
#define __ICONV_SUPPORT__
#ifdef __ICONV_SUPPORT__
#include "lca_conv.h"
#include "lca_conv.c"
#endif
#define _BONJOUR_
// Vairalbes
//Title
unsigned char CFGTitle[]=";WAP4410N Configuration File - Version: %s\n";
unsigned char CFGMac[]=";MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n"; //;MAC address: xx:xx:xx:xx:xx:xx
unsigned char APmac[6];
unsigned char CFGPin[]=";WPS PIN: %s\n"; //   ;WPS PIN: xxxxxxxx
char APpin[9];
unsigned char CFGChecksum[]=";The checksum: 00000000\n";//      ;The checksum: xxxx 
unsigned char APchecksum[9];

//Basic
unsigned char APName[MAX_SYSNAME+1];
unsigned char APDesc[MAX_DESC+1];
unsigned char APLang[4];
unsigned char dhcpF=1, dhcp6F=1;
unsigned char IPAddress4[4];
unsigned char subnet4[4];
unsigned char gateway4[4];
unsigned char dnsServer1[CFG_MAX_IPADDR+1];
unsigned char dnsServer2[CFG_MAX_IPADDR+1];
unsigned char IPv6;
char IPAddress6[45];
unsigned char prefixlen;
char gateway6[45];
char IPv6dnsServer1[45];
char IPv6dnsServer2[45];
//Time
unsigned char timesetting;
char date_p[15];
char time_p[10];
char timeZone[CFG_TIME_ZONE_LEN+1];
char timeZoneStr[]={"\r\n\
;001-12:00: \"(GMT-12:00) International Date Line West\"\r\n\
;002-11:00: \"(GMT-11:00) Midway Island, Samoa\"\r\n\
;003-10:00: \"(GMT-10:00) Hawaii\"\r\n\
;004-09:00: \"(GMT-09:00) Alaska\"\r\n\
;005-08:00: \"(GMT-08:00) Pacific Time (US & Canada); Tijuana\"\r\n\
;006-07:00: \"(GMT-07:00) Arizona\"\r\n\
;007-07:00: \"(GMT-07:00) Chihuahua, La Paz, Mazatlan\"\r\n\
;008-07:00: \"(GMT-07:00) Mountain Time (US & Canada)\"\r\n\
;009-06:00: \"(GMT-06:00) Central America\"\r\n\
;010-06:00: \"(GMT-06:00) Central Time (US & Canada)\"\r\n\
;011-06:00: \"(GMT-06:00) Guadalajara, Mexico City, Monterrey\"\r\n\
;012-06:00: \"(GMT-06:00) Saskatchewan\"\r\n\
;013-05:00: \"(GMT-05:00) Bogota, Lima, Quito\"\r\n\
;014-05:00: \"(GMT-05:00) Eastern Time (US & Canada)\"\r\n\
;015-05:00: \"(GMT-05:00) Indiana (East)\"\r\n\
;016-04:00: \"(GMT-04:00) Atlantic Time (Canada)\"\r\n\
;017-04:00: \"(GMT-04:00) Caracas, La Paz\"\r\n\
;018-04:00: \"(GMT-04:00) Santiago\"\r\n\
;019-03:00: \"(GMT-03:00) Newfoundland\"\r\n\
;020-03:00: \"(GMT-03:00) Brasilia\"\r\n\
;021-03:00: \"(GMT-03:00) Buenos Aires, Georgetown\"\r\n\
;022-03:00: \"(GMT-03:00) Greenland\"\r\n\
;023-02:00: \"(GMT-02:00) Mid-Atlantic\"\r\n\
;024-01:00: \"(GMT-01:00) Azores\"\r\n\
;025-01:00: \"(GMT-01:00) Cape Verde Is.\"\r\n\
;026+00:00: \"(GMT) Casablanca, Monrovia\"\r\n\
;027+00:00: \"(GMT) Greenwich Mean Time : Dublin, Edinburgh, Lisbon, London\"\r\n\
;028+01:00: \"(GMT+01:00) Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna\"\r\n\
;029+01:00: \"(GMT+01:00) Belgrade, Bratislava, Budapest, Ljubljana, Prague\"\r\n\
;030+01:00: \"(GMT+01:00) Brussels, Copenhagen, Madrid, Paris\"\r\n\
;031+01:00: \"(GMT+01:00) Sarajevo, Skopje, Warsaw, Zagreb\"\r\n\
;032+01:00: \"(GMT+01:00) West Central Africa\"\r\n\
;033+02:00: \"(GMT+02:00) Athens, Beirut, Istanbul, Minsk\"\r\n\
;034+02:00: \"(GMT+02:00) Bucharest\"\r\n\
;035+02:00: \"(GMT+02:00) Cairo\"\r\n\
;036+02:00: \"(GMT+02:00) Harare, Pretoria\"\r\n\
;037+02:00: \"(GMT+02:00) Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius\"\r\n\
;038+02:00: \"(GMT+02:00) Jerusalem\"\r\n\
;039+03:00: \"(GMT+03:00) Baghdad\"\r\n\
;040+03:00: \"(GMT+03:00) Kuwait, Riyadh\"\r\n\
;041+03:00: \"(GMT+03:00) Moscow, St. Petersburg, Volgograd\"\r\n\
;042+03:00: \"(GMT+03:00) Nairobi\"\r\n\
;043+03:30: \"(GMT+03:30) Tehran\"\r\n\
;044+04:00: \"(GMT+04:00) Abu Dhabi, Muscat\"\r\n\
;045+04:00: \"(GMT+04:00) Baku, Tbilisi, Yerevan\"\r\n\
;046+04:30: \"(GMT+04:30) Kabul\"\r\n\
;047+05:00: \"(GMT+05:00) Ekaterinburg\"\r\n\
;048+05:00: \"(GMT+05:00) Islamabad, Karachi, Tashkent\"\r\n\
;049+05:30: \"(GMT+05:30) Chennai, Kolkata, Mumbai, New Delhi\"\r\n\
;050+05:45: \"(GMT+05:45) Kathmandu\"\r\n\
;051+06:00: \"(GMT+06:00) Almaty, Novosibirsk\"\r\n\
;052+06:00: \"(GMT+06:00) Astana, Dhaka\"\r\n\
;053+06:00: \"(GMT+06:00) Sri Jayawardenepura\"\r\n\
;054+06:00: \"(GMT+06:00) Rangoon\"\r\n\
;055+07:00: \"(GMT+07:00) Bangkok, Hanoi, Jakarta\"\r\n\
;056+07:00: \"(GMT+07:00) Krasnoyarsk\"\r\n\
;057+08:00: \"(GMT+08:00) China, Hong Kong, Australia Western\"\r\n\
;058+08:00: \"(GMT+08:00) Irkutsk, Ulaan Bataar\"\r\n\
;059+08:00: \"(GMT+08:00) Kuala Lumpur, Singapore\"\r\n\
;060+08:00: \"(GMT+08:00) Perth\"\r\n\
;061+08:00: \"(GMT+08:00) Taipei\"\r\n\
;062+09:00: \"(GMT+09:00) Osaka, Sapporo, Tokyo\"\r\n\
;063+09:00: \"(GMT+09:00) Seoul\"\r\n\
;064+09:00: \"(GMT+09:00) Yakutsk\"\r\n\
;065+09:30: \"(GMT+09:30) Adelaide\"\r\n\
;066+09:30: \"(GMT+09:30) Darwin\"\r\n\
;067+10:00: \"(GMT+10:00) Brisbane\"\r\n\
;068+10:00: \"(GMT+10:00) Canberra, Melbourne, Sydney\"\r\n\
;069+10:00: \"(GMT+10:00) Guam, Port Moresby\"\r\n\
;070+10:00: \"(GMT+10:00) Hobart\"\r\n\
;071+10:00: \"(GMT+10:00) Vladivostok\"\r\n\
;072+11:00: \"(GMT+11:00) Magadan, Solomon Ls., New Caledonia\"\r\n\
;073+12:00: \"(GMT+12:00) Auckland, Wellington\"\r\n\
;074+12:00: \"(GMT+12:00) Fiji, Kamchatka, Marshall ls.\"\r\n\
;075+13:00: \"(GMT+13:00) Nuku`alofa\"\
"};  

 
/*  
"\r\n;-12: \"(GMT-12:00) Kwajalein\"\r\n\
; -11: \"(GMT-11:00) Midway Island, Samoa\"\r\n\
;-10: \"(GMT-10:00) Hawaii\"\r\n\
; -9: \"(GMT-09:00) Alaska\"\r\n\
; -8: \"(GMT-08:00) Pacific Time (USA; Canada)\"\r\n\
;-7.1: \"(GMT-07:00) Arizona\"\r\n\
;-7.2: \"(GMT-07:00) Mountain Time(USA; Canada)\"\r\n\
; -6.1: \"(GMT-06:00) Mexico\"\r\n\
;-6.2: \"(GMT-06:00) Central Time(USA; Canada)\"\r\n\
;-5.1: \"(GMT-05:00) Indiana East, Colombia, Panama\"\r\n\
;-5.2: \"(GMT-05:00) Eastern Time(USA; Canada)\"\r\n\
; -4.1: \"(GMT-04:00) Bolivia, Venezuela\"\r\n\
;-4.2: \"(GMT-04:00) Atlantic Time(Canada), Brazil West\"\r\n\
;-3.1: \"(GMT-03:00) Guyana\"\r\n\
; -3.2: \"(GMT-03:00) Brazil East, Greenland\"\r\n\
;-2: \"(GMT-02:00) Mid-Atlantic\"\r\n\
;-1: \"(GMT-01:00) Azores\"\r\n\
;0.1: \"(GMT) Gambia, Liberia, Morocco\"\r\n\
;0.2: \"(GMT) England\"\r\n\
;1.1: \"(GMT+01:00) Tunisia\"\r\n\
;1.2: \"(GMT+01:00) France, Germany, Italy\"\r\n\
;2.1: \"(GMT+02:00) South Africa\"\r\n\
;2.2: \"(GMT+02:00) Greece, Ukraine, Romania, Turkey\"\r\n\
;3: \"(GMT+03:00) Iraq, Jordan, Kuwait\"\r\n\
;4: \"(GMT+04:00) Armenia\"\r\n\
;5: \"(GMT+05:00) Pakistan, Russia\"\r\n\
;6: \"(GMT+06:00) Bangladesh, Russia\"\r\n\
;7: \"(GMT+07:00) Thailand, Russia\"\r\n\
;8.1: \"(GMT+08:00) China, Hong Kong, Australia Western\"\r\n\
;8.2\": \"(GMT+08:00) Singapore, Taiwan, Russia\"\r\n\
;9: \"(GMT+09:00) Japan, Korea\"\r\n\
;10.1: \"(GMT+10:00) Guam, Russia\"\r\n\
;10.2: \"(GMT+10:00) Australia\"\r\n\
;11: \"(GMT+11:00) Solomon Islands\"\r\n\
;12.1: \"(GMT+12:00) Fiji\"\r\n\
;12.2: \"(GMT+12:00) New Zealand\"\
"};
*/
unsigned char daylightSaving;
unsigned char usDefntp;
unsigned char ntpServer[CFG_MAX_IPADDR + 1];


//Setup_advanced
#ifdef _BONJOUR_
unsigned char htBonjour; //add by carole
#endif

unsigned char htForce100m;
unsigned char htAutoNegotiation;
unsigned char htPortSpeed;
unsigned char htDuplexMode;
unsigned char htEthDataRate;
unsigned char htRedirect;
unsigned char htRedirectUrl[MAC_CFG_URL_LENGTH + 1];
unsigned char ethDot1xAuth;
unsigned char authViaMac;
unsigned char authName[MAX_DOT1XSUPP_NAME+1];
unsigned char authPasswd[MAX_DOT1XSUPP_PASSWD+1];

//Wireless_Basic
unsigned char wlsMode;
unsigned char channel;
char ssid[WLAN_MAX_VAP][SSID_LEN+1];
unsigned char ssidBroadcast[WLAN_MAX_VAP];


//Radius_Server
unsigned char authSIP1[4];
unsigned short int authSPort1;
char authSSecret1[SECRET_LEN+1];
unsigned char authSIP2[4];
unsigned short int authSPort2;
char authSSecret2[SECRET_LEN+1];

//Wireless_security
unsigned char vapIsolated;
unsigned char secSystem[WLAN_MAX_VAP];
unsigned char wlsSeparation[WLAN_MAX_VAP];
unsigned char authType[WLAN_MAX_VAP];
unsigned char encrypt[WLAN_MAX_VAP];
unsigned char keyNo[WLAN_MAX_VAP];
unsigned char key1[WLAN_MAX_VAP][KEY_LEN+1];
unsigned char key2[WLAN_MAX_VAP][KEY_LEN+1];
unsigned char key3[WLAN_MAX_VAP][KEY_LEN+1];
unsigned char key4[WLAN_MAX_VAP][KEY_LEN+1];
unsigned char preShared[WLAN_MAX_VAP][CFG_MAX_PASSPHRASE + 1];
unsigned short int keyrenew[WLAN_MAX_VAP];

//Wireless_Connection_Control
unsigned char aclFlag[WLAN_MAX_VAP];
unsigned char aclType[WLAN_MAX_VAP];
unsigned char aclMac[WLAN_MAX_VAP][CFG_MAX_ACL_LINKSYS][6];

//VLAN_QoS
unsigned char vlan;
unsigned short int nativeVid;
unsigned short int manageVid;
unsigned char vlanTag;
unsigned char wdsTag;
unsigned char wdsVlanList[21];
unsigned short int vapVlan[4];
unsigned char vapPri[4];
unsigned char vapWmm[4];

//Wireless_Advanced
unsigned char worldwide;
unsigned short int countryCode;
unsigned char channelBand;
unsigned char guardInter;
unsigned char ctsProtect;
unsigned short int beaconInterval;
unsigned char dtimInter;
unsigned short int rtsThreshold;
unsigned short int fragLength;
unsigned char loadBalance;
unsigned short int vapBalance[WLAN_MAX_VAP];

//AP_Mode
unsigned char opMode;
unsigned char pxpMac[4][6];
unsigned char wdsRepeater;
unsigned char ucrRepeater;
char perfssid[SSID_LEN+1];
unsigned char noSecurity;
unsigned char illegalAp;
//unsigned char rogueApNo;
rogueAPMacList_s rogueApMacList;

//Management
unsigned char secShow;
unsigned char loginName[CFG_MAX_USERNAME+1];
unsigned char loginPasswd[CFG_MAX_PASSWORD+1];
unsigned char httpsEnable;
unsigned char webAccess;
unsigned char secureSh;
unsigned char snmp;
unsigned char contact[MAX_SNMP_COMMUNITY+1];
unsigned char snmplocal[MAX_SNMP_COMMUNITY+1];
unsigned char snmpdevice[MAX_SNMP_COMMUNITY+1];
unsigned char getCom[MAX_SNMP_COMMUNITY+1];
unsigned char setCom[MAX_SNMP_COMMUNITY+1];
unsigned char trapCom[MAX_SNMP_COMMUNITY+1];
unsigned char snmpTrust;
unsigned char trapStartIp[4];
unsigned char trapRange;
unsigned char trapServerIp[4];

//Log
unsigned char mailAlert;
char smtpServ[MAX_SMTP_SERVER + 1];
char mailAddr[MAX_MAIL_SERVER + 1];
unsigned short int logLength;
unsigned short int logTime;
unsigned char syslogF;
unsigned char logIPAddr[4];
unsigned char unAthLogin;
unsigned char AthLogin;
unsigned char sysError;
unsigned char cfgChange;

void GetaGroup(char *group,FILE *fp);
int findEQ(char *buff);
int GetaAttr(char *attribute,char *value,FILE *fp);
int matchGroup(char *group);
int matchAttr(char *attr,int id);
int ParseNAssignValue(int groupID,FILE *fp, A_UINT8 type);

char *OutputTitle(char *buff, A_UINT8 type);
char *OutputGroup(char *group,char *buff);
char *OutputAttr(char *attr,char *buff);
char *OutputComment(char *comment,char *buff);
char *OutputCountryComment(char *buff);

#ifndef PC_SIM
void outputCFGGet(A_UINT8 type);
void parseCFGSet(A_UINT8 type);
#endif

// **** Assign Functions
void Assign01(void *var,char *value);
void Assign01PublicAc(void *var,char *value);
void Assign01PublicTc(void *var,char *value);
void Assign01UserAc(void *var,char *value);
void Assign01UserTc(void *var,char *value);
void Assign01EasyAc(void *var,char *value);
void Assign01EasyTc(void *var,char *value);
void Assign0To2(void *var,char *value);
void Assign0To3(void *var,char *value);
void Assign0To4(void *var,char *value);
void Assign0To7(void *var,char *value);
void Assign0To8(void *var,char *value);
void Assign1To3(void *var,char *value);
void Assign1To4(void *var,char *value);
void Assign1To6(void *var,char *value);
void Assign0To14(void *var,char *value);
void Assign0To64(void *var,char *value);
void Assign0To99(void *var,char *value);
void Assign3To99(void *var,char *value);
void Assign0To255(void *var,char *value);
void Assign1To255(void *var,char *value);
void AssignIP(void *var,char *value);
void AssignIPv6(void *var,char *value);
void AssignGWv6(void *var,char *value);
void AssignMask(void *var,char *value);
void AssignSSID(void *var,char *value);
void AssignCountry(void *var,char *value);
void AssignDateRate(void *var,char *value);
void AssignKeyLen(void *var,char *value);
void Assign1To254(void *var,char *value);
void Assign1To500(void *var,char *value);
void Assign10To600(void *var,char *value);
void Assign60To600(void *var,char *value);
void Assign20To1000(void *var,char *value);
void Assign0To2347(void *var,char *value);
void Assign256To2346(void *var,char *value);
void Assign1To4094(void *var,char *value);
void AssignPrefix(void *var,char *value);
void AssignByte(void *var,char *value);
void AssignWord(void *var,char *value);
void AssignPort(void *var,char *value);
void AssignTrapPort(void *var,char *value);
void AssignStr15(void  *var,char *value);
void AssignStr32(void  *var,char *value);
void AssignStr40(void  *var,char *value);
void AssignStr64(void  *var,char *value);
void AssignPassword(void  *var,char *value);
void AssignSecret128(void  *var,char *value);
void AssignEncrypt(void *var,char *value);
void AssignKey(void  *var,char *value);
void AssignPreSharedKey(void  *var,char *value);
void AssignScMacList(void *var,char *value);
void AssignServerStr(void  *var,char *value);
void AssignVersion(void  *var,char *value);
void AssignMac(void *var,char *value);
void AssignTimeZone(void *var,char *value);
void AssignMail(void  *var,char *value);
void AssignURL(void  *var,char *value);
void AssignDate(void  *var,char *value);
void AssignTime(void  *var,char *value);
void AssignPercent(void  *var,char *value);
void Assignwdsvlist(void *var,char *value);
void AssignSMTPServer(void *var,char *value);
// *** Output Functions
char *OutputIP(void *var, char *buff);
char *OutputIPv6(void *var, char *buff);
char *OutputGWv6(void *var, char *buff);
char *OutputByte(void *var,char *buff);
char *OutputWord(void *var,char *buff);
char *OutputMac(void *var,char *buff);
char *OutputDR(void *var,char *buff);
char *OutputStr(void *var,char *buff);
char *OutputPass(void   *var,char *buff);
char *OutputPercent(void *var,char *buff);
char *OutputScMacList(void *var,char  *buff);


// Attribute Defintions
attr_s Basic[]={
    {"Host_Name", (void *)&APName, AssignStr40, OutputStr, "", CFG_TYPE_ALL},
    {"Device_Name", (void *)&APDesc, AssignStr40, OutputStr, "", CFG_TYPE_ALL},
    {"Language", (void *)&APLang, AssignStr40, OutputStr, "", CFG_TYPE_ALL},
    {"IP_settings",(void *)&dhcpF ,Assign01,OutputByte,";0:static; 1:automatic", CFG_TYPE_ALL},
    {"Ipv4_address",(void *)&IPAddress4 ,AssignIP,OutputIP,"", CFG_TYPE_ALL},
    {"Ipv4_subnet_mask",(void *)&subnet4 ,AssignMask,OutputIP,"", CFG_TYPE_ALL},
    {"IPv4_default_gateway",(void *)&gateway4 ,AssignIP,OutputIP,"", CFG_TYPE_ALL},
    {"IPv4_Primary_DNS",(void *)&dnsServer1 ,AssignStr15,OutputStr,"", CFG_TYPE_ALL},
    {"IPv4_Secondary_DNS",(void *)&dnsServer2 ,AssignStr15,OutputStr,"", CFG_TYPE_ALL},
    {"IPv6",(void *)&IPv6 ,Assign01,OutputByte, ";0:disabled; 1:enabled", CFG_TYPE_ALL},
    {"IPv6_settings",(void *)&dhcp6F ,Assign01,OutputByte,";0:static; 1:automatic", CFG_TYPE_ALL},
    {"Ipv6_address",(void *)&IPAddress6 ,AssignIPv6,OutputStr,"", CFG_TYPE_ALL},
    {"Ipv6_prefix_length",(void *)&prefixlen ,AssignPrefix,OutputByte,"", CFG_TYPE_ALL},
    {"IPv6_default_gateway",(void *)&gateway6 ,AssignGWv6,OutputStr,"", CFG_TYPE_ALL},
    {"IPv6_Primary_DNS",(void *)IPv6dnsServer1, AssignIPv6, OutputStr, "", CFG_TYPE_ALL},
    {"IPv6_Secondary_DNS",(void *)IPv6dnsServer2, AssignIPv6, OutputStr, "", CFG_TYPE_ALL},
    {""}
};

attr_s Time[]={
    {"Time_setting",(void *)&timesetting, Assign01, OutputByte, ";0: manually, 1:automatically", CFG_TYPE_ALL},
    {"Date",(void *)&date_p, AssignDate, OutputStr, ";yyyy/mm/dd", CFG_TYPE_ALL},
    {"Time",(void *)&time_p, AssignTime, OutputStr, ";hh:mm:ss", CFG_TYPE_ALL},
    {"Time_zone",(void *)&timeZone, AssignTimeZone, OutputStr, timeZoneStr, CFG_TYPE_ALL},
    {"daylight_saving",(void *)&daylightSaving, Assign01, OutputByte, ";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {"User_Defined_NTP",(void *)&usDefntp, Assign01, OutputByte, ";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {"User_NTP",(void *)&ntpServer, AssignServerStr, OutputStr,"", CFG_TYPE_ALL},
    {""}
};

attr_s Setup_adv[]={
    {"Force_Fast_Ethernet",(void *)&htForce100m, Assign01, OutputByte, ";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {"Ethernet_Auto_Negotiation",(void *)&htAutoNegotiation, Assign01, OutputByte, ";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {"Administrative_Port_Speed",(void *)&htPortSpeed, Assign0To2, OutputByte, ";0:10M, 1:100M, 2:1000M", CFG_TYPE_ALL},
    {"Administrative_Duplex_Mode",(void *)&htDuplexMode, Assign01, OutputByte, ";0:Half Duplex, 1:Full Duplex", CFG_TYPE_ALL},

//	{"Ethernet_Data_Rate",(void *)&htEthDataRate, Assign01, OutputByte, ";0:auto, 1:10 Mb Half, 2:10 Mb Full, 3:100 Mb Half, 4:100 Mb Full, 5:1000 Mb Half, 6:1000 Mb Full", CFG_TYPE_ALL},   
#ifdef _BONJOUR_
    {"Bonjour",(void *)&htBonjour, Assign01, OutputByte, ";0:disabled, 1:enabled", CFG_TYPE_ALL},//add by carole
#endif
    {"HTTP_Redirect",(void *)&htRedirect, Assign01, OutputByte, ";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {"HTTP_Redirect_URL",(void *)&htRedirectUrl, AssignURL, OutputStr, "", CFG_TYPE_ALL},    
    {"802.1x_supplicant",(void *)&ethDot1xAuth ,Assign01,OutputByte,";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {"Via_mac_authentication",(void *)&authViaMac ,Assign01,OutputByte,";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {"Username",(void *)&authName, AssignStr64, OutputStr, "", CFG_TYPE_ALL},
    {"Password",(void *)&authPasswd, AssignPassword, OutputStr, "", CFG_TYPE_ALL},
    {""}
};
  
attr_s Wlan_Basic[]={
    {"Network_mode",(void *)&wlsMode ,Assign0To7, OutputByte,";0:Disable; 1: b only; 2: g only; 3: n only; 4: B/G mixed; 7:B/G/N mixed", CFG_TYPE_ALL},
    {"Channel",(void *)&channel ,Assign0To14, OutputByte,"; 0..14; 0: auto channel", CFG_TYPE_ALL},
    {"SSID1",(void *)&ssid[0], AssignSSID, OutputStr,"", CFG_TYPE_ALL},
    {"SSID1_broadcast",(void *)&ssidBroadcast[0], Assign01,OutputByte,";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {"SSID2",(void *)&ssid[1],AssignSSID,OutputStr,"", CFG_TYPE_ALL},
    {"SSID2_broadcast",(void *)&ssidBroadcast[1], Assign01,OutputByte,";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {"SSID3",(void *)&ssid[2],AssignSSID,OutputStr,"", CFG_TYPE_ALL},
    {"SSID3_broadcast",(void *)&ssidBroadcast[2], Assign01,OutputByte,";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {"SSID4",(void *)&ssid[3],AssignSSID,OutputStr,"", CFG_TYPE_ALL},
    {"SSID4_broadcast",(void *)&ssidBroadcast[3], Assign01,OutputByte,";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {""}
};

attr_s Radius_Server[]={
    {"Primary_radius_server",(void *)&authSIP1 ,AssignIP,OutputIP,"", CFG_TYPE_ALL},
    {"Primary_port_no",(void *)&authSPort1 ,AssignPort,OutputWord,"", CFG_TYPE_ALL},
    {"Primary_shared_secret",(void *)&authSSecret1 ,AssignSecret128,OutputPass,"", CFG_TYPE_ALL},
    {"Secondary_radius_server",(void *)&authSIP2 ,AssignIP,OutputIP,"", CFG_TYPE_ALL},
    {"Secondary_port_no",(void *)&authSPort2 ,AssignPort,OutputWord,"", CFG_TYPE_ALL},
    {"Secondary_shared_secret",(void *)&authSSecret2 ,AssignSecret128,OutputPass,"", CFG_TYPE_ALL},
    {""}
};

attr_s Wlan_sec_1[]={
    {";for SSID1", (void *)NULL, NULL,NULL,"",CFG_TYPE_ALL},
    {"ssid",(void *)&ssid[0], NULL, OutputStr,"", CFG_TYPE_ALL},
    {"isolation_between_SSID",(void *)&vapIsolated ,Assign01,OutputByte,";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {"security_mode",(void *)&secSystem[0],Assign0To8,OutputByte,";1: WEP, 2: WPA-Personal, 3: WPA2-Personal, 4: WPA2-Personal Mixed, 5: WPA-Enterprise,\r\n\
    6: WPA2-Enterprise, 7: WPA2-Enterprise Mixed, 8:RADIUS, 0: Disabled", CFG_TYPE_ALL},
    {"isolation_within_SSID",(void *)&wlsSeparation[0] ,Assign01,OutputByte,";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {"authentication_type",(void *)&authType[0] ,Assign0To2,OutputByte,";0:open system; 1:shared key", CFG_TYPE_ALL},
    {"encryption",(void *)&encrypt[0] ,AssignKeyLen,OutputByte,";64: 64 bit WEP, 128:128 bit WEP, 1: TKP, 2: AES, 3: AES+TKIP", CFG_TYPE_ALL},
    {"Default_Tx_key",(void *)&keyNo[0] ,Assign1To4,OutputByte,"; 1..4", CFG_TYPE_ALL},
    {"key1",(void *)&key1[0] ,AssignKey,OutputPass,";in Hex", CFG_TYPE_ALL},
    {"key2",(void *)&key2[0] ,AssignKey,OutputPass,";in Hex", CFG_TYPE_ALL},
    {"key3",(void *)&key3[0] ,AssignKey,OutputPass,";in Hex", CFG_TYPE_ALL},
    {"key4",(void *)&key4[0] ,AssignKey,OutputPass,";in Hex", CFG_TYPE_ALL},
    {"PSK_key",(void *)&preShared[0] ,AssignPreSharedKey,OutputPass,";in ASCII; The length of the key is from 8 to 64", CFG_TYPE_ALL},
    {"Key_renew",(void *)&keyrenew[0] ,AssignWord,OutputWord,";in seconds", CFG_TYPE_ALL},
    {""}
};

attr_s Wlan_sec_2[]={
    {";for SSID2", (void *)NULL, NULL,NULL,"",CFG_TYPE_ALL},
    {"ssid",(void *)&ssid[1], NULL, OutputStr,"", CFG_TYPE_ALL},
    {"isolation_between_SSID",(void *)&vapIsolated ,Assign01,OutputByte,";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {"security_mode",(void *)&secSystem[1],Assign0To8,OutputByte,";1: WEP, 2: WPA-Personal, 3: WPA2-Personal, 4: WPA2-Personal Mixed, 5: WPA-Enterprise,\r\n\
    6: WPA2-Enterprise, 7: WPA2-Enterprise Mixed, 8:RADIUS, 0: Disabled", CFG_TYPE_ALL},
    {"isolation_within_SSID",(void *)&wlsSeparation[1] ,Assign01,OutputByte,";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {"authentication_type",(void *)&authType[1] ,Assign0To2,OutputByte,";0:open system; 1:shared key", CFG_TYPE_ALL},
    {"encryption",(void *)&encrypt[1] ,AssignKeyLen,OutputByte,";64: 64 bit WEP, 128:128 bit WEP, 1: TKP, 2: AES, 3: AES+TKIP", CFG_TYPE_ALL},
    {"Default_Tx_key",(void *)&keyNo[1] ,Assign1To4,OutputByte,"; 1..4", CFG_TYPE_ALL},
    {"key1",(void *)&key1[1] ,AssignKey,OutputPass,";in Hex", CFG_TYPE_ALL},
    {"key2",(void *)&key2[1] ,AssignKey,OutputPass,";in Hex", CFG_TYPE_ALL},
    {"key3",(void *)&key3[1] ,AssignKey,OutputPass,";in Hex", CFG_TYPE_ALL},
    {"key4",(void *)&key4[1] ,AssignKey,OutputPass,";in Hex", CFG_TYPE_ALL},
    {"PSK_key",(void *)&preShared[1] ,AssignPreSharedKey,OutputPass,";in ASCII; The length of the key is from 8 to 63", CFG_TYPE_ALL},
    {"Key_renew",(void *)&keyrenew[1] ,AssignWord,OutputWord,";in seconds", CFG_TYPE_ALL},
    {""}
};

attr_s Wlan_sec_3[]={
    {";for SSID3", (void *)NULL, NULL,NULL,"",CFG_TYPE_ALL},
    {"ssid",(void *)&ssid[2], NULL, OutputStr,"", CFG_TYPE_ALL},
    {"isolation_between_SSID",(void *)&vapIsolated ,Assign01,OutputByte,";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {"security_mode",(void *)&secSystem[2],Assign0To8,OutputByte,";1: WEP, 2: WPA-Personal, 3: WPA2-Personal, 4: WPA2-Personal Mixed, 5: WPA-Enterprise,\r\n\
    6: WPA2-Enterprise, 7: WPA2-Enterprise Mixed, 8:RADIUS, 0: Disabled", CFG_TYPE_ALL},
    {"isolation_within_SSID",(void *)&wlsSeparation[2] ,Assign01,OutputByte,";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {"authentication_type",(void *)&authType[2] ,Assign0To2,OutputByte,";0:open system; 1:shared key", CFG_TYPE_ALL},
    {"encryption",(void *)&encrypt[2] ,AssignKeyLen,OutputByte,";64: 64 bit WEP, 128:128 bit WEP, 1: TKP, 2: AES, 3: AES+TKIP", CFG_TYPE_ALL},
    {"Default_Tx_key",(void *)&keyNo[2] ,Assign1To4,OutputByte,"; 1..4", CFG_TYPE_ALL},
    {"key1",(void *)&key1[2] ,AssignKey,OutputPass,";in Hex", CFG_TYPE_ALL},
    {"key2",(void *)&key2[2] ,AssignKey,OutputPass,";in Hex", CFG_TYPE_ALL},
    {"key3",(void *)&key3[2] ,AssignKey,OutputPass,";in Hex", CFG_TYPE_ALL},
    {"key4",(void *)&key4[2] ,AssignKey,OutputPass,";in Hex", CFG_TYPE_ALL},
    {"PSK_key",(void *)&preShared[2] ,AssignPreSharedKey,OutputPass,";in ASCII; The length of the key is from 8 to 63", CFG_TYPE_ALL},
    {"Key_renew",(void *)&keyrenew[2] ,AssignWord,OutputWord,";in seconds", CFG_TYPE_ALL},
    {""}
};

attr_s Wlan_sec_4[]={
    {";for SSID4", (void *)NULL, NULL,NULL,"",CFG_TYPE_ALL},
    {"ssid",(void *)&ssid[3], NULL, OutputStr,"", CFG_TYPE_ALL},
    {"isolation_between_SSID",(void *)&vapIsolated ,Assign01,OutputByte,";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {"security_mode",(void *)&secSystem[3],Assign0To8,OutputByte,";1: WEP, 2: WPA-Personal, 3: WPA2-Personal, 4: WPA2-Personal Mixed, 5: WPA-Enterprise,\r\n\
    6: WPA2-Enterprise, 7: WPA2-Enterprise Mixed, 8:RADIUS, 0: Disabled", CFG_TYPE_ALL},
    {"isolation_within_SSID",(void *)&wlsSeparation[3] ,Assign01,OutputByte,";0:disabled, 1:enabled", CFG_TYPE_ALL},
    {"authentication_type",(void *)&authType[3] ,Assign0To2,OutputByte,";0:open system; 1:shared key", CFG_TYPE_ALL},
    {"encryption",(void *)&encrypt[3] ,AssignKeyLen,OutputByte,";64: 64 bit WEP, 128:128 bit WEP, 1: TKP, 2: AES, 3: AES+TKIP", CFG_TYPE_ALL},
    {"Default_Tx_key",(void *)&keyNo[3] ,Assign1To4,OutputByte,"; 1..4", CFG_TYPE_ALL},
    {"key1",(void *)&key1[3] ,AssignKey,OutputPass,";in Hex", CFG_TYPE_ALL},
    {"key2",(void *)&key2[3] ,AssignKey,OutputPass,";in Hex", CFG_TYPE_ALL},
    {"key3",(void *)&key3[3] ,AssignKey,OutputPass,";in Hex", CFG_TYPE_ALL},
    {"key4",(void *)&key4[3] ,AssignKey,OutputPass,";in Hex", CFG_TYPE_ALL},
    {"PSK_key",(void *)&preShared[3] ,AssignPreSharedKey,OutputPass,";in ASCII; The length of the key is from 8 to 63", CFG_TYPE_ALL},
    {"Key_renew",(void *)&keyrenew[3] ,AssignWord,OutputWord,";in seconds", CFG_TYPE_ALL},
    {""}
};

attr_s Wlan_Conn_Control_1[]={
    {";for SSID1", (void *)NULL, NULL,NULL,"",CFG_TYPE_ALL},
    {"Connection_Control",(void *)&aclFlag[0], Assign0To2,OutputByte,";0: disabled, 1:local, 2: Radius", CFG_TYPE_ALL},
    {"Control_type",(void *)&aclType[0], Assign01,OutputByte,";0: allowed the following local mac, 1: prevent the following local mac", CFG_TYPE_ALL},
    {"Mac_01",(void *)&aclMac[0][0] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_02",(void *)&aclMac[0][1] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_03",(void *)&aclMac[0][2] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_04",(void *)&aclMac[0][3] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_05",(void *)&aclMac[0][4] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_06",(void *)&aclMac[0][5] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_07",(void *)&aclMac[0][6] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_08",(void *)&aclMac[0][7] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_09",(void *)&aclMac[0][8] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_10",(void *)&aclMac[0][9] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_11",(void *)&aclMac[0][10] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_12",(void *)&aclMac[0][11] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_13",(void *)&aclMac[0][12] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_14",(void *)&aclMac[0][13] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_15",(void *)&aclMac[0][14] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_16",(void *)&aclMac[0][15] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_17",(void *)&aclMac[0][16] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_18",(void *)&aclMac[0][17] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_19",(void *)&aclMac[0][18] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_20",(void *)&aclMac[0][19] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},    
    {""}
};

attr_s Wlan_Conn_Control_2[]={
    {";for SSID2", (void *)NULL, NULL,NULL,"",CFG_TYPE_ALL},
    {"Connection_Control",(void *)&aclFlag[1], Assign0To2,OutputByte,";0: disabled, 1:local, 2: Radius", CFG_TYPE_ALL},
    {"Control_type",(void *)&aclType[1], Assign01,OutputByte,";0: allowed the following local mac, 1: prevent the following local mac", CFG_TYPE_ALL},
    {"Mac_01",(void *)&aclMac[1][0] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_02",(void *)&aclMac[1][1] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_03",(void *)&aclMac[1][2] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_04",(void *)&aclMac[1][3] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_05",(void *)&aclMac[1][4] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_06",(void *)&aclMac[1][5] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_07",(void *)&aclMac[1][6] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_08",(void *)&aclMac[1][7] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_09",(void *)&aclMac[1][8] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_10",(void *)&aclMac[1][9] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_11",(void *)&aclMac[1][10] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_12",(void *)&aclMac[1][11] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_13",(void *)&aclMac[1][12] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_14",(void *)&aclMac[1][13] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_15",(void *)&aclMac[1][14] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_16",(void *)&aclMac[1][15] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_17",(void *)&aclMac[1][16] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_18",(void *)&aclMac[1][17] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_19",(void *)&aclMac[1][18] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_20",(void *)&aclMac[1][19] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},    
    {""}
};

attr_s Wlan_Conn_Control_3[]={
    {";for SSID3", (void *)NULL, NULL,NULL,"",CFG_TYPE_ALL},
    {"Connection_Control",(void *)&aclFlag[2], Assign0To2,OutputByte,";0: disabled, 1:local, 2: Radius", CFG_TYPE_ALL},
    {"Control_type",(void *)&aclType[2], Assign01,OutputByte,";0: allowed the following local mac, 1: prevent the following local mac", CFG_TYPE_ALL},
    {"Mac_01",(void *)&aclMac[2][0] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_02",(void *)&aclMac[2][1] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_03",(void *)&aclMac[2][2] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_04",(void *)&aclMac[2][3] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_05",(void *)&aclMac[2][4] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_06",(void *)&aclMac[2][5] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_07",(void *)&aclMac[2][6] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_08",(void *)&aclMac[2][7] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_09",(void *)&aclMac[2][8] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_10",(void *)&aclMac[2][9] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_11",(void *)&aclMac[2][10] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_12",(void *)&aclMac[2][11] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_13",(void *)&aclMac[2][12] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_14",(void *)&aclMac[2][13] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_15",(void *)&aclMac[2][14] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_16",(void *)&aclMac[2][15] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_17",(void *)&aclMac[2][16] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_18",(void *)&aclMac[2][17] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_19",(void *)&aclMac[2][18] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_20",(void *)&aclMac[2][19] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},    
    {""}
};

attr_s Wlan_Conn_Control_4[]={
    {";for SSID4", (void *)NULL, NULL,NULL,"",CFG_TYPE_ALL},
    {"Connection_Control",(void *)&aclFlag[3], Assign0To2,OutputByte,";0: disabled, 1:local, 2: Radius", CFG_TYPE_ALL},
    {"Control_type",(void *)&aclType[3], Assign01,OutputByte,";0: allowed the following local mac, 1: prevent the following local mac", CFG_TYPE_ALL},
    {"Mac_01",(void *)&aclMac[3][0] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_02",(void *)&aclMac[3][1] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_03",(void *)&aclMac[3][2] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_04",(void *)&aclMac[3][3] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_05",(void *)&aclMac[3][4] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_06",(void *)&aclMac[3][5] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_07",(void *)&aclMac[3][6] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_08",(void *)&aclMac[3][7] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_09",(void *)&aclMac[3][8] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_10",(void *)&aclMac[3][9] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_11",(void *)&aclMac[3][10] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_12",(void *)&aclMac[3][11] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_13",(void *)&aclMac[3][12] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_14",(void *)&aclMac[3][13] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_15",(void *)&aclMac[3][14] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_16",(void *)&aclMac[3][15] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_17",(void *)&aclMac[3][16] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_18",(void *)&aclMac[3][17] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_19",(void *)&aclMac[3][18] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Mac_20",(void *)&aclMac[3][19] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},    
    {""}
};

attr_s VLAN_QoS[]={
    {"VLAN",(void *)&vlan ,Assign01,OutputByte,";0: disabled, 1:enabled", CFG_TYPE_ALL},
    {"Default_VLAN_ID",(void *)&nativeVid ,Assign1To4094,OutputWord,";1..4094", CFG_TYPE_ALL},
    {"VLAN_Tag",(void *)&vlanTag, Assign0To2,OutputByte,";0:untagged, 1: tagged", CFG_TYPE_ALL},
    {"AP_Management_VLAN",(void *)&manageVid ,Assign1To4094,OutputWord,";1..4094", CFG_TYPE_ALL},
    {"VLAN_tag_over_WDS",(void *)&wdsTag, Assign0To2,OutputByte,";0: disabled, 1:enabled", CFG_TYPE_ALL},
   {"WDS_VLAN_List",(void *)&wdsVlanList, Assignwdsvlist, OutputStr,";a,b,...,\"a\" is a VLAN ID. Different IDs divided by \",\". 4 VLAN IDs should be set at most", CFG_TYPE_ALL},
    {"VLAN_ID_4_SSID1",(void *)&vapVlan[0] ,Assign1To4094,OutputWord,";1..4094", CFG_TYPE_ALL},
    {"Priority_4_SSID1",(void *)&vapPri[0] ,Assign0To7,OutputByte,";0..7", CFG_TYPE_ALL},
    {"WMM_4_SSID1",(void *)&vapWmm[0], Assign0To2,OutputByte,";0: disabled, 1:enabled", CFG_TYPE_ALL},
    {"VLAN_ID_4_SSID2",(void *)&vapVlan[1] ,Assign1To4094,OutputWord,";1..4094", CFG_TYPE_ALL},
    {"Priority_4_SSID2",(void *)&vapPri[1] ,Assign0To7,OutputByte,";0..7", CFG_TYPE_ALL},
    {"WMM_4_SSID2",(void *)&vapWmm[1], Assign0To2,OutputByte,";0: disabled, 1:enabled", CFG_TYPE_ALL},
    {"VLAN_ID_4_SSID3",(void *)&vapVlan[2] ,Assign1To4094,OutputWord,";1..4094", CFG_TYPE_ALL},
    {"Priority_4_SSID3",(void *)&vapPri[2] ,Assign0To7,OutputByte,";0..7", CFG_TYPE_ALL},
    {"WMM_4_SSID3",(void *)&vapWmm[2], Assign0To2,OutputByte,";0: disabled, 1:enabled", CFG_TYPE_ALL},
    {"VLAN_ID_4_SSID4",(void *)&vapVlan[3] ,Assign1To4094,OutputWord,";1..4094", CFG_TYPE_ALL},
    {"Priority_4_SSID4",(void *)&vapPri[3] ,Assign0To7,OutputByte,";0..7", CFG_TYPE_ALL},
    {"WMM_4_SSID4",(void *)&vapWmm[3], Assign0To2,OutputByte,";0: disabled, 1:enabled", CFG_TYPE_ALL},
    {""}
};

attr_s Wlan_Adv[]={
    {"802.11d",(void *)&worldwide ,Assign01,OutputByte,";0: disabled, 1:enabled", CFG_TYPE_ALL},
    {"Country_Region",(void *)&countryCode ,AssignCountry,OutputWord,"", CFG_TYPE_ALL},
    {"Channel_bandwidth",(void *)&channelBand ,Assign0To2,OutputByte,";0: auto, 1: 20MHz, 2: 40MHz", CFG_TYPE_ALL},
    {"Guard_Interval",(void *)&guardInter ,Assign0To2,OutputByte,";0: auto, 1: 400ns, 2: 800ns", CFG_TYPE_ALL},
    {"CTS_Protection_Mode",(void *)&ctsProtect ,Assign01,OutputByte,";0: disable, 1: auto", CFG_TYPE_ALL}, 
    {"beacon_interval",(void *)&beaconInterval ,Assign20To1000,OutputWord,";20..1000", CFG_TYPE_ALL},
    {"DTIM_interval",(void *)&dtimInter ,Assign1To255,OutputByte,";1..255", CFG_TYPE_ALL},
    {"rts_threshold",(void *)&rtsThreshold ,Assign0To2347,OutputWord,";1..2347", CFG_TYPE_ALL},
    {"fragmentation_threshold",(void *)&fragLength ,Assign256To2346,OutputWord,";256..2346", CFG_TYPE_ALL},
    {"Load_balance",(void *)&loadBalance ,Assign01,OutputByte,";0: disabled, 1:enabled", CFG_TYPE_ALL},
/* balance */
    {"Load_threshold_4_SSD1",(void *)&vapBalance[0] ,AssignPercent,OutputPercent,"", CFG_TYPE_ALL},
    {"Load_threshold_4_SSD2",(void *)&vapBalance[1] ,AssignPercent,OutputPercent,"", CFG_TYPE_ALL},
    {"Load_threshold_4_SSD3",(void *)&vapBalance[2] ,AssignPercent,OutputPercent,"", CFG_TYPE_ALL},
    {"Load_threshold_4_SSD4",(void *)&vapBalance[3] ,AssignPercent,OutputPercent,"", CFG_TYPE_ALL},
    {""}
};

attr_s AP_Mode[]={
    {"AP_mode",(void *)&opMode ,Assign0To4,OutputByte,";0: Access Point, 1: WDS repeater, 2: WDS bridge, 3: Universal bridge, 4: Monitor", CFG_TYPE_ALL},
    {"MAC1",(void *)&pxpMac[0] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"MAC2",(void *)&pxpMac[1] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"MAC3",(void *)&pxpMac[2] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"MAC4",(void *)&pxpMac[3] ,AssignMac,OutputMac,"", CFG_TYPE_ALL},
    {"Allow_WDS_repeater",(void *)&wdsRepeater ,Assign01,OutputByte,";0:disabled, 1: enabled", CFG_TYPE_ALL},
    {"Universal_repeater",(void *)&ucrRepeater ,Assign01,OutputByte,";0:disabled, 1: enabled", CFG_TYPE_ALL},
    {"Preferred_SSID",(void *)&perfssid,AssignSSID,OutputStr,";for universal bridge", CFG_TYPE_ALL},
    {"No_Security",(void *)&noSecurity ,Assign01,OutputByte,";1: view it as a rogue AP. 0: don¡¯t", CFG_TYPE_ALL},
    {"No_in_Legal_AP_List", (void *)&illegalAp ,Assign01,OutputByte,";1: view it as a rogue AP. 0: don¡¯t", CFG_TYPE_ALL},
    {"Legal_AP_List_entry_no",(void *)&(rogueApMacList.no), Assign0To255, OutputWord,";0..255", CFG_TYPE_ALL},
    {"Legal_AP",(void *)&rogueApMacList, AssignScMacList, OutputScMacList,"", CFG_TYPE_ALL},
    {""}
};

attr_s Management[]={
    {"secret_shown",(void *)&secShow ,Assign01,OutputByte,";0: show clear text, 1: just show ********", CFG_TYPE_ALL},
    {"username",(void *)&loginName ,AssignStr64, OutputStr, "", CFG_TYPE_ALL},
    {"AP_password",(void *)&loginPasswd ,AssignPassword, OutputPass, "", CFG_TYPE_ALL},
    {"HTTPS_Access",(void *)&httpsEnable ,Assign01,OutputByte,";0:disabled, 1: enabled", CFG_TYPE_ALL},
    {"Wireless_Web_Access",(void *)&webAccess ,Assign01,OutputByte,";0:disabled, 1: enabled", CFG_TYPE_ALL},
    {"Secure_Shell",(void *)&secureSh ,Assign01,OutputByte,";0:disabled, 1: enabled", CFG_TYPE_ALL},
    {"SNMP",(void *)&snmp ,Assign01,OutputByte,";0:disabled, 1: enabled", CFG_TYPE_ALL},
    {"Contact",(void *)&contact, AssignStr32, OutputStr,"", CFG_TYPE_ALL},
    {"Device_Name",(void *)&snmpdevice, AssignStr32, OutputStr,"", CFG_TYPE_ALL},    
    {"Location",(void *)&snmplocal, AssignStr32, OutputStr,"", CFG_TYPE_ALL},
    {"Get_Community",(void *)&getCom ,AssignStr32, OutputStr,"", CFG_TYPE_ALL},
    {"Set_Community",(void *)&setCom ,AssignStr32, OutputStr,"", CFG_TYPE_ALL},
    {"Trap_Community",(void *)&trapCom ,AssignStr32, OutputStr,"", CFG_TYPE_ALL},    
    {"SNMP_Trusted_Any_Host",(void *)&snmpTrust ,Assign01,OutputByte,";0:disabled, 1: enabled", CFG_TYPE_ALL},
    {"SNMP_Trusted_Host_start_IP",(void *)&trapStartIp, AssignIP, OutputIP,"", CFG_TYPE_ALL},
    {"SNMP_Trusted_Host_range",(void *)&trapRange, Assign1To254, OutputByte,";0..255", CFG_TYPE_ALL},
    {"Trap_Destination",(void *)&trapServerIp, AssignIP, OutputIP,"", CFG_TYPE_ALL},
    {""}    
};

attr_s Log[]={
    {"E-mail_alert",(void *)&mailAlert ,Assign01,OutputByte,";0:disabled, 1: enabled", CFG_TYPE_ALL},
    {"SMTP_server",(void *)&smtpServ ,AssignSMTPServer,OutputStr,"", CFG_TYPE_ALL},
    {"Email_address_for_logs",(void *)&mailAddr ,AssignMail,OutputStr,"", CFG_TYPE_ALL},
    {"Log_queue_length",(void *)&logLength ,Assign1To500,OutputWord,"", CFG_TYPE_ALL},
    {"Log_time_threshold",(void *)&logTime ,Assign60To600,OutputWord,"", CFG_TYPE_ALL},
    {"Syslog",(void *)&syslogF ,Assign01,OutputByte,";0:disabled, 1: enabled", CFG_TYPE_ALL},
    {"Syslog_server_IP",(void *)&logIPAddr ,AssignIP,OutputIP,"", CFG_TYPE_ALL},
    {"Unathorized_login_attempt",(void *)&unAthLogin, Assign01,OutputByte,";0:no log, 1: log", CFG_TYPE_ALL},
    {"Authorized_Login",(void *)&AthLogin, Assign01,OutputByte,";0:no log, 1: log", CFG_TYPE_ALL},
    {"System_error_message",(void *)&sysError, Assign01,OutputByte,";0:no log, 1: log", CFG_TYPE_ALL},
    {"Configuration_changes",(void *)&cfgChange, Assign01,OutputByte,";0:no log, 1: log", CFG_TYPE_ALL},
    {""}
};

/*
 * BASIC
 * Time
 * Setup_advanced
 *
 * Wireless_Basic
 * Radius_Server
 * Wireless_security_1
 * Wireless_security_2
 * Wireless_security_3
 * Wireless_security_4
 * Wireless_Connection_Control_1
 * Wireless_Connection_Control_2
 * Wireless_Connection_Control_3
 * Wireless_Connection_Control_4
 * VLAN_QoS
 * Wireless_Advanced
 * 
 * AP_Mode
 * 
 * Management
 * Log
 *
*/
CFG_s CFGData[]={
    {"",(void *)NULL, 0},
    {"BASIC",(void *)&Basic, CFG_TYPE_ALL},
    {"Time",(void *)&Time, CFG_TYPE_ALL},
    {"Setup_advanced",(void *)&Setup_adv, CFG_TYPE_ALL},
    {"Wireless_Basic",(void *)&Wlan_Basic, CFG_TYPE_ALL},
    {"Radius_Server",(void *)&Radius_Server, CFG_TYPE_ALL},
    {"Wireless_security_1",(void *)&Wlan_sec_1, CFG_TYPE_ALL},
    {"Wireless_security_2",(void *)&Wlan_sec_2, CFG_TYPE_ALL},
    {"Wireless_security_3",(void *)&Wlan_sec_3, CFG_TYPE_ALL},
    {"Wireless_security_4",(void *)&Wlan_sec_4, CFG_TYPE_ALL},
    {"Wireless_Connection_Control_1",(void *)&Wlan_Conn_Control_1, CFG_TYPE_ALL},
    {"Wireless_Connection_Control_2",(void *)&Wlan_Conn_Control_2, CFG_TYPE_ALL},
    {"Wireless_Connection_Control_3",(void *)&Wlan_Conn_Control_3, CFG_TYPE_ALL},
    {"Wireless_Connection_Control_4",(void *)&Wlan_Conn_Control_4, CFG_TYPE_ALL},
    {"VLAN_QoS",(void *)&VLAN_QoS, CFG_TYPE_ALL},
    {"Wireless_Advanced",(void *)&Wlan_Adv, CFG_TYPE_ALL},
    {"AP_Mode",(void *)&AP_Mode, CFG_TYPE_ALL},
    {"Management",(void *)&Management, CFG_TYPE_ALL},
    {"Log",(void *)&Log, CFG_TYPE_ALL},
};

//#define EDIT_DEBUG
#ifdef EDIT_DEBUG
int mylog(const char *format, ...)
{
    va_list args;
    FILE *fp;

    fp = fopen("/dev/ttyS0", "a+");

    if (!fp) {
        fprintf(stderr, "fp is NULL\n");
	    return -1;
    }

    va_start(args, format);
    vfprintf(fp, format, args);
    va_end(args);

    fclose(fp);
    return 0;
}
#else
#define mylog(fmt, ...)
#endif

#define 	SSID_ATTR 	"SSID"

void (*nvAssignFuncTbl[])() ={
	AssignKey,
	AssignMac,
	AssignSSID,
	NULL
};
A_BOOL isNvAssignFunc(void (*assignfunc)())
{
	int index = 0;
	
	while(nvAssignFuncTbl[index]){
		if(nvAssignFuncTbl[index] == assignfunc){
			return TRUE;
		}	
		index++;	
	}	
	return FALSE;
}	

void correctVariables()
{
#define AP_OPMODE   0	    
	// channel
	int maxChannel = 0;
	char buff[32];
	FILE *fp = fopen("/tmp/chan_list","r");
	if(fp!=NULL)
	{
	    while(fgets(buff, 31, fp)){	        
	        if(strcmp(buff,"current:\n") == 0)
	            break;
	        else
	            maxChannel = atoi(buff);
	    }
	    fclose(fp);
	}
	if(channel > maxChannel) channel=0;
	if(opMode!= AP_OPMODE)  //if WDS bridge is enable
	{ 
		if(channel==0) channel=maxChannel;
	}
    
    // rogue Ap
    if(opMode!= AP_OPMODE)  //rogue ap could be enabled in ap mode only
    {
        //rogueAP = 0;
    }
    
#if 0	
	// Date Rate
#define ONLY11B_WMODE 2
#define ONLY11G_WMODE 3
#define ONLY11N_WMODE 4
	if(wlsMode== ONLY11B_WMODE){
		if (dataRate>11 || dataRate==6 || dataRate==9) 
			dataRate=0;
	}else if(wlsMode== ONLY11G_WMODE || wlsMode== ONLY11N_WMODE)
	{
		if(dataRate <6 || dataRate==11)
			dataRate=0;
	}
#endif	
}

//return 0:     discard this input
//return non-0: this input is OK, need reboot
int parseCFG(char *cfgfp, A_UINT8 type, A_BOOL setup)
{

    FILE *fp;
    char group[128];
    int groupID;
    int goodGroup=0;
    
    if((fp = fopen(cfgfp, "r")) == NULL)
    {
        mylog("open %s failed\n", cfgfp);
        return goodGroup;
    } 
#ifdef EDIT_DEBUG       
    mylog("<%s,%d>\n",__FUNCTION__,__LINE__);
#endif

#ifndef PC_SIM    
    outputCFGGet(type);  //backup global values
#endif

    while(!feof(fp)){
        GetaGroup(group,fp);
        if ((groupID=matchGroup(group))){
#ifdef EDIT_DEBUG              
            mylog("parseCFG get a group=%s (groupID=%d)\n",group, groupID);
#endif
			if(CFGData[groupID].type&type){
	            goodGroup+=ParseNAssignValue(groupID,fp, type);
            }
        }
    }
    fclose(fp);
	correctVariables();

#ifndef PC_SIM  
    if(goodGroup && setup)
        parseCFGSet(type);
#endif
    return goodGroup;    
}

void GetaGroup(char *group,FILE *fp)
{
    char buffer[130];
    int i,j,l=0;
   
    while(l==0){
        if(feof(fp)) break;
        buffer[0]='\0';
        fgets(buffer,129,fp);
        buffer[129]='\0';
//mylog("GetaGroup buffer=%s\n",buffer);          
        for(i=0;buffer[i];i++){
            if(buffer[i]=='['){
                for(j=i+1;buffer[j];j++){
                    if(buffer[j]==']'){
                        l=j-i-1;
                        memcpy(group,buffer+i+1,l);
                        break;
                    }
                }
                break;
            }  
        }
    }
    group[l]='\0';
}

int findEQ(char *buff)
// return '=''s location;
{
    int i;
    char c;
    for(i=0;(c=buff[i]);i++){
        if(c==' ') break;
        if(c=='='){
            return (i);
        }
    }
    return 0;
}
         
   

int GetaAttr(char *attribute,char *value,FILE *fp)
{
    char buffer[130];
    int i,j,l=0;  
    while(l==0){
        long pos;
        if(feof(fp)) return 0;
        attribute[0]=value[0]=buffer[0]='\0';
        pos=ftell(fp);
        fgets(buffer,129,fp);
        buffer[129]='\0';
        if(buffer[0]=='['){
            pos=fseek(fp,pos,SEEK_SET);
            return 0;
        }
        if(buffer[0]!='\0' && buffer[0]!=' '){
            for(i=0;buffer[i];i++){
                if((j=findEQ(buffer))){
                    memcpy(attribute,buffer,j);
                    attribute[j]='\0';
                    for(i=j+1;buffer[i];i++){
                        if(buffer[i]>' ' && buffer[i]<0x7F){
                            value[l]=buffer[i];
                            l++;
                        }
                        else if(buffer[i] == ' ' && (strncmp(attribute, SSID_ATTR, 4)==0 && attribute[5] == '=')){
                        	value[l]=buffer[i];
                            l++;
                    	}
			else if((strcmp(attribute, "SSID1")==0 )||(strcmp(attribute, "SSID2")==0)||(strcmp(attribute, "SSID3")==0)||(strcmp(attribute, "SSID4")==0 ))
			{
				if(buffer[i]!=0x0D){
					value[l]=buffer[i];
					l++;
				}
				else{
					mylog("982===\n");
					break;
				}
			}
			else{
                        	break;
                            }
                    }
                    value[l]='\0';
                    if(!l)	l++; //just for walk out the while loop
                    break;
                }
            }  
        }
    }
    return 1;
}

int matchGroup(char *group)
// return gruopID. 0 means not matched
{
    int i;
    for(i=1;i<sizeof(CFGData)/sizeof(CFG_s);i++){
        if(!strcmp(CFGData[i].group,group)) 
            return i;
    }
    return 0;
}  

int matchAttr(char *attr,int id)
// return attributeID. 0 means not matched
{
    int i;
    attr_s *p=(attr_s *)CFGData[id].attrs;
    for(i=0; *((p+i)->attr)!='\0';i++){
        if(!strcmp((p+i)->attr,attr)) 
            return i+1;
    }
    return 0;
}

int ParseNAssignValue(int groupID,FILE *fp, A_UINT8 type)
{
    char attr[128],value[128];
    attr_s *p;
    void (*assignfunc)();
    int attrID;
    int goodAttr=0;
    while(!feof(fp)){
        mylog("%s:%d:<attr,value> = <%s,%s>\n", __FUNCTION__, __LINE__, attr, value);
        if(GetaAttr(attr,value,fp)){
            if((attrID=matchAttr(attr,groupID))){
                p=(attr_s *)CFGData[groupID].attrs;
                if((p+attrID-1)->type&type)
                {
                    assignfunc=(p+attrID-1)->assignfunc;
                    if(assignfunc){
                    	if(value[0] || ((!value[0]) && isNvAssignFunc(assignfunc))){
                        	assignfunc((p+attrID-1)->var,value);
                        	goodAttr++; 
                        }
                    }   
                }         
            }
        mylog("%s:%d:<attr,value> = <%s,%s>, attrID=%d\n", __FUNCTION__, __LINE__, attr, value, attrID);
        }
        else
            break;
    }
    return (goodAttr) ? 1 : 0;
}

//Output editable configuration function
//return length of editable configuration
int OutputCFG(char *buff, A_UINT8 type)
{
    int i,j,l;
    attr_s *p;
    char *(*outputfunc)();
    char *pchsum;
    unsigned char *point;
    char *temBuff;
    unsigned long chSumDate = 0;
    
#ifdef EDIT_DEBUG
    mylog("<%s,%d>\n",__FUNCTION__,__LINE__);
#endif

#ifndef PC_SIM
    outputCFGGet(type);
#endif

#ifdef EDIT_DEBUG
    mylog("<%s,%d>\n",__FUNCTION__,__LINE__);
#endif
    temBuff=buff;
    buff=OutputTitle(buff, type);
    pchsum = buff;    
    point = (unsigned char *)buff;    
#ifdef EDIT_DEBUG
    mylog("<%s,%d>\n",__FUNCTION__,__LINE__);
#endif
    for(i=1;i<sizeof(CFGData)/sizeof(CFG_s);i++){
    	if(!(CFGData[i].type&type))
    	    continue;
        buff=OutputGroup(CFGData[i].group,buff);
#ifdef EDIT_DEBUG
        mylog("group=%s\n",CFGData[i].group);
#endif
        p=(attr_s *)CFGData[i].attrs;
        for(j=0;(p+j)->attr[0]!='\0';j++){
            if(!((p+j)->type&type))
                continue;
            buff=OutputAttr((p+j)->attr,buff);  /* write subType */
            outputfunc=(p+j)->outputfunc;
            if(outputfunc)                       
                buff=outputfunc((p+j)->var,buff); /* content */
            if(memcmp((p+j)->attr, "Country_Region", 14)) {
                buff=OutputComment((p+j)->comment,buff); /* note */
            }
            else{
                buff=OutputCountryComment(buff); /* country note */
            }
        }     
    }
    *buff='\0';
    while(*point != 0)
        chSumDate += *point++;
    l=sprintf(APchecksum, "%-8lu", chSumDate);
    APchecksum[8] = '\n';
    memcpy(pchsum-9 ,APchecksum, 9);
    return (buff-temBuff);
}

char *OutputTitle(char *buff, A_UINT8 type)
{
    int l = 0;
    char temp[20];
    
    getVersion(temp);
    l += sprintf(buff+l, CFGTitle, temp);
    l += sprintf(buff+l, CFGMac, APmac[0],APmac[1],APmac[2],APmac[3],APmac[4],APmac[5]);
    l += sprintf(buff+l, CFGPin, APpin);
    l += sprintf(buff+l, CFGChecksum);

    return(buff+l);
}

char *OutputGroup(char *group,char *buff)
{
    int l;
    *buff='\r';
    buff++;
    *buff='\n';
    buff++;
    *buff='[';
    buff++;
    l=strlen(group);
    memcpy(buff,group,l);
    buff[l]=']';
    buff[l+1]='\r';
    buff[l+2]='\n';
    return(buff+l+3);
}

char *OutputAttr(char *attr,char *buff)
{
    int l;
    l=strlen(attr);
    memcpy(buff,attr,l);
    if(attr[0] != ';'){
        buff[l++] = '=';
    }
    return(buff+l);
}

char *OutputComment(char *comment,char *buff)
{
    int l;
    if(comment){
        l=strlen(comment);
        if(l){
            *buff=' ';
            buff++;
            memcpy(buff,comment,l);
            buff+=l;
        }
    }
    *buff ='\r';
    buff++;
    *buff ='\n';
    buff++;
    return buff;
}   
char *OutputCountryComment(char *buff)
{
    int coLine;
    
    char *comment[]={
        ";840: United States. It¡¯s read only",
        ";36: Australia. It¡¯s read only",
        ";76: Brazil. It¡¯s read only",
        ";156: China. It¡¯s read only",
        ";1702: Asia, 208: Denmark, 1276: Europe, 246: Finland, 250: France, 276: Germany,\r\n\
         372: Ireland, 380: Italy, 392: Japan, 528: Netherlands, 554: New Zealand, 578: Norway, \r\n\
         724: Spain, 752: Sweden, 756: Switzerland, 826: United Kingdom",
        ";484: Mexico, 630: Puerto Rico, 1076: South America",
       ";410: South Korea. It¡¯s read only",

    };
    
    switch(sc_get_default_country()){
        case CTRY_UNITED_STATES:
            coLine = 0;
            break;
        case CTRY_AUSTRALIA:
            coLine = 1;
            break;
        case CTRY_BRAZIL:
            coLine = 2;
            break;
        case CTRY_CHINA:
            coLine = 3;
            break;
        case CTRY_GERMANY:
            coLine = 4;
            break;
        case CTRY_MEXICO:
            coLine = 5;
            break;
        case CTRY_KOREA_ROC:
	    coLine = 6;
	    break;
        default:
            coLine = 0;
    }
    
    return OutputComment(comment[coLine], buff);
}
// assign function: It has to to check if its value is valid
//                  If not, do nothing.
//
void Assign01(void *var,char *value)
{
    unsigned char t=*((unsigned char *)value)-'0';
    if (/*t>=0 && */t<=1)
        *((unsigned char *)var)= t;
}

void Assign0To2(void *var,char *value)
{
    unsigned char t=*((unsigned char *)value)-'0';
    if (/*t>=0 && */ t<=2)
        *((unsigned char *)var)= t;
}

void Assign0To3(void *var,char *value)
{
    unsigned char t=*((unsigned char *)value)-'0';
    if (/*t>=0 && */ t<=3)
        *((unsigned char *)var)= t;
}	        

void Assign0To4(void *var,char *value)
{
    unsigned char t=*((unsigned char *)value)-'0';
    if (/*t>=0 && */ t<=4)
        *((unsigned char *)var)= t;
}

void Assign1To3(void *var,char *value)
{
    unsigned char t=*((unsigned char *)value)-'0';
    if (t>=1 && t<=3)
        *((unsigned char *)var)= t;
}

void Assign1To4(void *var,char *value)
{
    unsigned char t=*((unsigned char *)value)-'0';
    if (t>=1 && t<=4)
        *((unsigned char *)var)= t;
}

void Assign1To6(void *var,char *value)
{
    int t;
    char c=*(char *)value;
    if(c<'0' || c>'9') return;
    t=atoi(value);
    if (t>=1 && t<=6)
        *((unsigned short int *)var)= (unsigned short int)t;
}
void Assign0To7(void *var,char *value)
{
	unsigned char t=*((unsigned char *)value)-'0';
    if (t<=7)
        *((unsigned char *)var)= t;
}
	
void Assign0To8(void *var,char *value)
{
	unsigned char t=*((unsigned char *)value)-'0';
    if (/*t>=0 && */ t<=8)
        *((unsigned char *)var)= t;
}
	
void Assign0To14(void *var,char *value)
{
	int t;
    char c=*(char *)value;
    if(c<'0' || c>'9') return;
    t=atoi(value);
    if (t>=0 && t<=14)
        *((unsigned char *)var)= (unsigned char)t;
}

void Assign0To64(void *var,char *value)
{
	int t;
    char c=*(char *)value;
    if(c<'0' || c>'9') return;
    t=atoi(value);
    if (t>=0 && t<=64)
        *((unsigned char *)var)= (unsigned char)t;
}

void Assign0To99(void *var,char *value)
{
	int t;
    char c=*(char *)value;
    if(c<'0' || c>'9') return;
    t=atoi(value);
    if (t>=0 && t<=99)
        *((unsigned short int *)var)= (unsigned short int)t;
}

void Assign3To99(void *var,char *value)
{
	int t;
    char c=*(char *)value;
    if(c<'3' || c>'9') return;
    t=atoi(value);
    if (t>=3 && t<=99)
        *((unsigned short int *)var)= (unsigned short int)t;
}

void Assign1To254(void *var,char *value)
{
    int t=atoi(value);
    if (t>=1 && t<=254)
        *((unsigned char *)var)= (unsigned char)t;
}
void Assign0To255(void *var,char *value)
{
    int t=atoi(value);
    if (t<=255)
        *((unsigned short int *)var)= (unsigned short int)t;
    rogueApMacList.outNo = 0;
}
void Assign1To255(void *var,char *value)
{
    int t=atoi(value);
    if (t>=1 && t<=255)
        *((unsigned char *)var)= (unsigned char)t;
}
void Assign1To500(void *var,char *value)
{
    int t=atoi(value);
    if (t>=1 && t<=500)
        *((unsigned short int *)var)= (unsigned short int)t;
}
void Assign10To600(void *var,char *value)
{
    int t=atoi(value);
    if (t>=10 && t<=600)
        *((unsigned short int *)var)= (unsigned short int)t;
} 
void Assign60To600(void *var,char *value)
{
    int t=atoi(value);
    if (t>=60 && t<=600)
        *((unsigned short int *)var)= (unsigned short int)t;
} 
void Assign20To1000(void *var,char *value)
{
    int t=atoi(value);
    if (t>=20 && t<=1000)
        *((unsigned short int *)var)= (unsigned short int)t;
}

void Assign0To2347(void *var,char *value)
{
    int t;
    char c=*(char *)value;
    if(c<'0' || c>'9') return;
    t=atoi(value);
    if (t>=0 && t<=2347)
        *((unsigned short int *)var)= (unsigned short int)t;
}
	
void Assign256To2346(void *var,char *value)
{
    int t;
    char c=*(char *)value;
    if(c<'0' || c>'9') return;
    t=atoi(value);
    if (t>=256 && t<=2346)
        *((unsigned short int *)var)= (unsigned short int)t;
}

void Assign1To4094(void *var,char *value)
{
	int t;
    char c=*(char *)value;
    if(c<'0' || c>'9') return;
    t=atoi(value);
    if (t>=1 && t<4095)
        *((unsigned short int *)var)= (unsigned short int)t;
}	
void AssignPrefix(void *var,char *value)
{
    int t;
    char c=*(char *)value;
    if(c<'0' || c>'9') return;
    t=atoi(value);
    if (t>=0 && t<=128)
        *((unsigned char *)var)=   (unsigned char)t;
}
void AssignByte(void *var,char *value)
{
    int t;
    char c=*(char *)value;
    if(c<'0' || c>'9') return;
    t=atoi(value);
    if (t>=0 && t<=255)
        *((unsigned char *)var)=   (unsigned char)t;
}
	
void AssignWord(void *var,char *value)
{
    int t;
    char c=*(char *)value;
    if(c<'0' || c>'9') return;
    t=atoi(value);
    if (t>=0 && t<65536)
        *((unsigned short int *)var)= (unsigned short int)t;
}

void AssignPort(void *var,char *value)
{
    int t;
    char c=*(char *)value;
    if(c<'0' || c>'9') return;
    t=atoi(value);
    if (t>0 && t<65535)
        *((unsigned short int *)var)= (unsigned short int)t;
}

void AssignTrapPort(void *var,char *value)
{
    int t;
    char c=*(char *)value;
    
    if(c<'0' || c>'9') return;
    t=atoi(value);
    if (t>0 && t<65535)
    {
        if(isInvalidSnmpTrapPort((A_UINT32)t)){
            return;
        }
        *((unsigned short int *)var)= (unsigned short int)t;
    }
}

void AssignTimeZone(void *var,char *value)
{
    int l,i,h,m;
    l=strlen(value);
    if(l>CFG_TIME_ZONE_LEN) return;
    sscanf(value, "%d%d:%d",&i,&h,&m);
    if(i<1 || i>75) return;
    if(h<-12 || h>13) return;    
    if(m!=0 && m!=30 && m!=45) return;
    if(m==45 && h!=5) return;
    if(m==30 && (h!=3 || h!=4 || h!=5 || h!=9)) return;
    
    strcpy(var,value);
}

void AssignIP(void *var,char *value)
{
    int l;
    unsigned char *a=(unsigned char *)var;
    unsigned int i[4];
    l=sscanf(value,"%d.%d.%d.%d",&i[0],&i[1],&i[2],&i[3]);
    if(l!=4) return;
    /*
#ifdef _EASY_N9_
    if(!(i[0]==0 && i[1]==0 && i[2]==0 && i[3]==0))
#endif
    */
    if((i[0]<1 || i[0]>254) || (i[1]<0 || i[1]>255) || (i[2]<0 || i[2]>255) ||(i[3]<1 || i[3]>254)) 
        return;
    for(l=3;l>=0;l--) a[l]=(unsigned char)i[l];
}
void AssignIPv6(void *var,char *value)
{
    if(!scValidIPv6(value, 0)) return;

    strcpy(var,value);
}
void AssignGWv6(void *var,char *value)
{
    if(!scValidGWv6(value)) return;    
  
    strcpy(var, value);
}
void AssignMask(void *var,char *value)
{
    int l;
    unsigned char *a=(unsigned char *)var;
    unsigned int i[4];
    l=sscanf(value,"%d.%d.%d.%d",&i[0],&i[1],&i[2],&i[3]);
    if(l!=4) return;
    
    if(i[0] != 255)
    {
        if(i[1] != 0 || i[2] != 0 || i[3] != 0)
		        return ;
        switch(i[0])
        {
            case 192:
            case 224:
            case 240:
            case 248:
            case 252:
            case 254:
                break;
            default:
		        return ;
        }
    }
    else if(i[1] != 255)
    {
        if( i[2] != 0 || i[3] != 0)
		        return ;
        //0, 128, 192, 224, 240, 248, 252, 254
        switch(i[1])
        {
            case 0:
            case 128:
            case 192:
            case 224:
            case 240:
            case 248:
            case 252:
            case 254:
                break;
            default:
		        return ;
        }
    }
    else if(i[2] != 255)
    {
        if(i[3] != 0)
		    return ;

        //0, 128, 192, 224, 240, 248, 252, 254
        switch(i[2])
        {
            case 0:
            case 128:
            case 192:
            case 224:
            case 240:
            case 248:
            case 252:
            case 254:
                break;
            default:
		        return ;
        }   
    }
    else
    {
        //0, 128, 192, 224, 240, 248, 252
        switch(i[3])
        {            
            case 0:
            case 128:
            case 192:
            case 224:
            case 240:
            case 248:
            case 252:
                break;
            default:
		        return ;
        }       
    }
        
    for(l=3;l>=0;l--) a[l]=(unsigned char)i[l];
}

void AssignSSID(void *var,char *value)
{
    int l;
    l=strlen(value);
    if(l>SSID_LEN) return;
    strncpy(var,value,SSID_LEN);
}

void AssignCountry(void *var,char *value)
{
    unsigned short int l;
    char c=*(char *)value;
    unsigned short int de_country;
    FILE *fp = NULL;
    char buff[64];
    char *p;
    char *tok = "-";
    int ret = 0;
    if(c<'0' || c>'9') return;  
    de_country = (unsigned short int)sc_get_default_country();
    switch(de_country){
        case CTRY_UNITED_STATES:
        case CTRY_AUSTRALIA:
        case CTRY_BRAZIL:
        case CTRY_CHINA:
        case CTRY_GERMANY:
 	case CTRY_MEXICO:
 	case CTRY_KOREA_ROC:
            *((unsigned short int*)var) = de_country;
            return;
        default:
            break;
    }
    
    l=(unsigned short int )atoi(value);
    
    fp = fopen("/tmp/country_list", "r");
    if(fp != NULL){
        while(fgets(buff, 63, fp)){
            p = strtok(buff, tok);
            p = strtok(NULL, tok);
            if(p != NULL)
                if(memcmp(value, p, sizeof(unsigned short int)) == 0){
                    ret = 1;
                    break;
                }
        }
        fclose(fp);
    }

    if(ret)
        *((unsigned short int*)var) = (unsigned short int)atoi(value);
    else
        *((unsigned short int*)var) = de_country;
}

void AssignDateRate(void *var,char *value)
{
    int t;
    char c=*(char *)value;
    if(c<'0' || c>'9') return;
    t=atoi(value);
    if (t==0 || t==1 || t==2 || t==5 || t==6 || t==9 || t==11 || 
        t==12 || t==18 || t==24 || t==36 || t==48 || t==54) 
        *((unsigned char *)var)= (unsigned char)t;
}

void AssignKeyLen(void *var,char *value)
{
    int t=atoi(value);
    switch(t)
    {
        case 1:
            t = WPA_CIPHER_TKIP;
            break;
        case 2:
            t = WPA_CIPHER_AES;
            break;
        case 3:
            t = WPA_CIPHER_AUTO;
            break;
        case 64:
            t = 40;
            break;
        case 128:
            t = 104;
            break;
        default:
            t = 40;
    }

    *((unsigned char *)var)= (unsigned char)t;
}
int IsStarStr(char *buff,int length)
{
	int i;
	for(i=0;i<length;i++){
		if(*(buff+i)!='*') return 0;
	}
	return 1;
}

void AssignStr15(void  *var,char *value)
{
    int l;
    l=strlen(value);
    memset((unsigned char *)var, 0, l+1);    
	if(l>15){
		memcpy(var,value,15);
		*((unsigned char *)var+15)='\0';
	}
	else
		strcpy(var,value);
}

void AssignStr32(void  *var,char *value)
{
    int l;
    l=strlen(value);
    memset((unsigned char *)var, 0, l+1);    
	if(l>32){
		memcpy(var,value,32);
		*((unsigned char *)var+32)='\0';
	}
	else
		strcpy(var,value);
}	
void AssignStr40(void  *var,char *value)
{
    int l;
    l=strlen(value);
    memset((unsigned char *)var, 0, l+1);    
	if(l>39){
		memcpy(var,value,40);
		*((unsigned char *)var+40)='\0';
	}
	else
		strcpy(var,value);
}
void AssignStr64(void  *var,char *value)
{
    int l;
    l=strlen(value);
    memset((unsigned char *)var, 0, l+1);    
	if(l>63){
		memcpy(var,value,64);
		*((unsigned char *)var+64)='\0';
	}
	else
		strcpy(var,value);
}
void AssignPassword(void  *var,char *value)
{
    int l;
    l=strlen(value);
    if(l<4) return;
    memset((unsigned char *)var, 0, l+1);    
    if(IsStarStr(value,l)) return;
	if(l>63){
		memcpy(var,value,64);
		*((unsigned char *)var+64)='\0';
	}
	else
		strcpy(var,value);
}	
void AssignSecret128(void  *var,char *value)
{
    int l;
    l=strlen(value);
    memset((unsigned char *)var, 0, l+1);    
    if(IsStarStr(value,l)) return;
	if(l>SECRET_LEN){
		memcpy(var,value,SECRET_LEN);
		*((unsigned char *)var+SECRET_LEN)='\0';
	}
	else
		strcpy(var,value);
}
int IsHexStr(char *str,int l,int flag)
{
    char c;
    int i;
    for(i=0;i<l;i+=1+flag){
        c=*(str+i);
        if(c>='0' && c<='9') goto nextcheck;
        if(c>='A' && c<='F') goto nextcheck;
        if(c>='a' && c<='f') goto nextcheck;
	if(i == (l+1)/2 && flag?(memcmp(str+i, "**:**:**", 8)==0):(memcmp(str+i, "******", 6)==0))   
		return 1;
	    return 0;
nextcheck:
	    i++;
	    if(i>=l) break;
	    c=*(str+i);
        if(c>='0' && c<='9') continue;
        if(c>='A' && c<='F') continue;
        if(c>='a' && c<='f') continue;
        return 0;
    }
    return 1;
}


void hexcpy(unsigned char *var, char * value,int l,int flag)
{
    int i,j;
    unsigned char c1,c2;
    for(i=0,j=0;j<l;i++,j+=2+flag){
        c1=*(value+j)-'0';
        c1=c1>10?(c1&0x0F)+9:c1;
        c2=*(value+j+1)-'0';
        c2=c2>10?(c2&0x0F)+9:c2;
        *(var+i)=(c1<<4)|c2;
    }
}
   

void AssignKey(void  *var,char *value)
{
	if(value[0]){
	    int l,k=KEY_LEN;
	    l=strlen(value);
	    if(l&0x01) return;
		if(l>k) l=k;
		if(l!=10 && l!=26) return;
	    if(!IsHexStr(value,l,0)) return;
	    
	    memset((unsigned char *)var, 0, l+1);	    
	    if(IsStarStr(value,l)) return;
	    memcpy((unsigned char *)var, value, l);    
	}else{
		memset((unsigned char *)var, 0, KEY_LEN+1);
	} 
}
void AssignPreSharedKey(void  *var,char *value)
{
    int l;
    l=strlen(value);
    if(l<8 || l>63) return;
    memset((unsigned char *)var, 0, l+1);    
    if(IsStarStr(value,l)) return;
    if(l>63){
    	memcpy((unsigned char *)var,value,63);
    	*((unsigned char *)var+63)='\0';
    }
    else
    	strcpy((unsigned char *)var,value);
}

int IsInScMacList(unsigned char *d,rogueAPMacList_s *L)
{
    unsigned short int i,n;
    n= L->outNo;
    for(i=0;i<n;i++){
        if(!memcmp(L->mac[i],d,17)) return 1;
    }
    return 0;
}

void AssignMac(void *var,char *value)
{
	if(value[0]){
	    int l,flag;
	    l=strlen(value);
	    if(l!=12 && l!=17) return;
	    flag=(l==17)?1:0;
	    if(!IsHexStr(value,l,flag)) return;
	    hexcpy((unsigned char *)var,value,l,flag);
	}else{
		memset((unsigned char *)var, 0, 6);
	}	    
}
   
void AssignScMacList(void *var,char *value)
{
    rogueAPMacList_s *p;
    int l,flag;
    unsigned char mac[18];
    p=(rogueAPMacList_s *)var;
    l=strlen(value);
	memset(mac, 0, 18);
    
    if(p->no<=p->outNo) return;
    if(l!=12 && l!=17) return;
    flag=(l==17)?1:0;
    if(!IsHexStr(value,l,flag)) return;
	if(l == 12)
		scMacStr12ToStr17((A_UINT8 *)value, (A_UINT8 *)mac, ":" );
	else
		memcpy((unsigned char *)&mac, value, l);
    if(IsInScMacList((unsigned char *)&mac, p)) {
        p->no--;
        return;
    }
    memcpy(&(p->mac[p->outNo++]),&mac,17);
}

void AssignServerStr(void  *var,char *value)
{
    int l;
    l=strlen(value);
    memset((unsigned char *)var, 0, l+1);    

	if(l>CFG_MAX_IPADDR){
		memcpy(var,value,CFG_MAX_IPADDR);
		*((unsigned char *)var+CFG_MAX_IPADDR)='\0';
	}
	else
		strcpy(var,value);
}

void AssignMail(void  *var,char *value)
{
    char buff[65];
    if(strlen(value) > 64)
        return;
    memcpy(buff, value, strlen(value)+1);
    
/*    char *p = strchr(buff, '@');
    if( p == NULL) return;
    *p = 0;
    if(!scValidStr(buff)) return;
    if(!scValidUrl(p+1)) return;
	*/
	if(!scValidEmailAddr(buff, (A_UINT8)strlen(buff)))
		return ;
    strcpy(var, value);
}

void AssignURL(void  *var,char *value)
{
    char *p = value;
    
    scToLows(p);
/*    if(strncmp(p, "http://", 7) == 0)
        p += 7;
    if(strncmp(p, "https://", 7) == 0)
        p += 8;
*/
    if(!scValidUrl(p)) return;
    strcpy(var, value);
}
void AssignDate(void  *var,char *value)
{
    int t;
    char *spare = "/";
    char buff[15];
    strcpy(buff, value);
    char *p = strtok(buff, spare);
    char c = *p;
    if(c<'0' || c>'9') return;
    t=atoi(p);
    if(t<1970 && t>2100) return;
    p = strtok(NULL, spare);
    t=atoi(p);
    if(t<0 && t>12) return;
    p = strtok(NULL, spare);
    t=atoi(p);
    if(t<0 && t>31) return ;
    
    strcpy(var, value);
}
void AssignTime(void  *var,char *value)
{
    int t;
    char buff[128];
    strcpy(buff, value);
    char *spare = ":";
    char *p = strtok(buff, spare);
    char c = *p;
    if(c<'0' || c>'9') return;
    t=atoi(p);
    if(t<0 && t>59) return;
        strcpy(var, p);
    p = strtok(NULL, spare);
    t=atoi(p);
    if(t<0 && t>59) return;
        strcat(var, p);
    p = strtok(NULL, spare);
    t=atoi(p);
    if(t<0 && t>59) return;
    strcpy(var, value);
}
void AssignPercent(void  *var,char *value)
{
    float t; 
    unsigned short int temp;
    sscanf(value, "%f",&t);
    if(t < 0.0 || t >100.0) return;
    temp = t * 100;
    memcpy(var, &temp, 2);
}

void Assignwdsvlist(void *var, char *value)
{
    if(scValidwdsVlanList(value))
        strcpy(var, value);
}
void AssignSMTPServer(void *var, char *value)
{
	if(strlen(value) < 65 && scValidUrl(value))
	{
             if(strncasecmp(value,"http://",7)!=0 && strncasecmp(value,"https://",8)!=0)
	         strcpy(var, value);
	}
}
// Output Function: char *Outputxxx(void *var, char *buff)
//                   return buff contains var's display content and
//                          its buff pointer to available

char *OutputIP(void *var, char *buff)
{
    int l;
    unsigned char *address=(char *)var;
    if(address[0]||address[1]||address[2]||address[3]
/*
#ifdef _EASY_N9_
        || 1
#endif
*/
    ){
   	    sprintf(buff,"%u.%u.%u.%u",address[0],address[1],address[2],address[3]);
   	    l=strlen(buff);
   	    return (buff+l);
    }
    else return (buff);
}
char *OutputIPv6(void *var, char *buff)
{
    return buff;
}
char *OutputGWv6(void *var, char *buff)
{
    return buff;
}
char *OutputByte(void *var,char *buff)
{
    int l;
    unsigned char a=*(char *)var;
    sprintf(buff,"%u",a);
    l=strlen(buff);
    return (buff+l);
}

char *OutputWord(void *var,char *buff)
{
    int l;
    unsigned short int a=*(unsigned short int *)var;
    sprintf(buff,"%u",a);
    l=strlen(buff);
    return (buff+l);
}

char *OutputMac(void *var,char *buff)
{
    unsigned char *a=(char *)var;
    if(a[0]||a[1]||a[2]||a[3]||a[4]||a[5]){
   	    sprintf(buff,"%02X:%02X:%02X:%02X:%02X:%02X",a[0],a[1],a[2],a[3],a[4],a[5]);
   	    return (buff+17);
    }
    else
   	    return buff;
}

char *OutputStr(void *var,char *buff)
{
    int l;
    l=strlen(var);
    memcpy(buff,var,l);
    return (buff+l);
}

char *OutputPass(void   *var,char *buff)
{
    if(secShow) 
        memset(var, '*', strlen(var));
   
    return OutputStr(var, buff);
}

char *OutputDR(void *var,char *buff)
{
    int l;
    unsigned char a=*(char *)var;
    if (a!=5)
        sprintf(buff,"%u",a);
    else
        sprintf(buff,"%u.5",a);
    l=strlen(buff);
    return (buff+l);
}
char *OutputPercent(void *var,char *buff)
{
    int l = 0;
    unsigned short int a=*(unsigned short int *)var;
    if(a != 0){
        sprintf(buff, "%.2f %%", a/100.0);
        l=strlen(buff);
    }
    return (buff+l);
}
char *OutputScMacList(void *var,char  *buff)
{
    rogueAPMacList_s *p;
    char temp[80],c;
    int l;
    p=(rogueAPMacList_s *)var;
    if(!p->no) return buff;
    for(l=2;(c=*(buff-l));l++){
	    if(c=='\n'){
		    memcpy(temp,buff-(l+1),(l+1));
		    break;
	    }
    }
    while(1){
	    memcpy(buff, (void *)&(p->mac[p->outNo]),17);
		buff+=17;
		p->outNo++;
	    if(p->no==p->outNo) return buff;
	    memcpy(buff,temp,(l+1));
	    buff+=(l+1);
    }
}

void acpcfg_edit_setip(char *ipAddr, char *ipStr)
{   
    char    ipstrtemp[20],*p=NULL;
    
    strcpy(ipstrtemp, ipStr);
    p = ipstrtemp;
    for ( ; *p; p++)
        if (*p == '.')
            *p = 0;
    p = ipstrtemp;        
    ipAddr[0]=atoi(p);
    p+=strlen(p) + 1;
    ipAddr[1]=atoi(p);
    p+=strlen(p) + 1;
    ipAddr[2]=atoi(p);
    p+=strlen(p) + 1;
    ipAddr[3]=atoi(p);
}



#ifndef PC_SIM
void outputCFGGet(A_UINT8 type)
{
    A_UINT32 ipAddress;
    A_UINT8 unit=RADIO_24G, vap = 0; 
    struct scAclBuf_s *pScAclCurr = NULL, *pScAcl = NULL;
    int count;
    if_info_t iface_status;
    char ipv6Buf[45];
    char *separ="/";
    char *pt;
    if_infov6_t iface6_status;

     //Management
    secShow = apCfgSecMaskGet();
	getMgtBrInfo(&iface_status);
	
	scHexs2Chars(iface_status.mac, APmac, 6, 1);
	strcpy(APpin, apCfgDevicePinGet(unit, vap));
	//BASIC 
	strcpy(APName, apCfgSysNameGet());   
    strcpy(APDesc, apCfgDescGet());
    strcpy(APLang, apCfgSysLangGet());
    dhcpF=apCfgDhcpEnableGet();
    dhcp6F=apCfgDhcp6EnableGet();
    ipAddress=apCfgIpAddrGet();
    memcpy(IPAddress4, &(ipAddress),4);
    ipAddress=apCfgIpMaskGet();
    memcpy(subnet4, &(ipAddress),4);
    ipAddress=apCfgGatewayAddrGet();
    memcpy(gateway4, &(ipAddress),4);
    strcpy(dnsServer1, apCfgNameSrvGet());
    strcpy(dnsServer2, apCfgNameSrv2Get());
    
    IPv6 = apCfgipv6modeGet();
    if(IPv6) {
        if(apCfgDhcp6EnableGet()) {
            getMgtBrv6Info(&iface6_status, 1);
            if(strlen(iface6_status.ipaddr) == 0)
                getMgtBrv6Info(&iface6_status, 2);
            if(strlen(iface6_status.ipaddr) > 0)
                sprintf(ipv6Buf, "%s", iface6_status.ipaddr);
            if(strlen(iface6_status.gw) > 0)
                sprintf(gateway6, "%s", iface6_status.gw);
	}
        else {
            sprintf(ipv6Buf, "%s", apCfgIpv6AddrGet());
            sprintf(gateway6, "%s", apCfgGatewayv6AddrGet());
        }
        strcpy(IPv6dnsServer1, apCfgNameSrv61Get());
        strcpy(IPv6dnsServer2, apCfgNameSrv62Get());
        pt = strtok(ipv6Buf, separ);
        if(pt != NULL)
            strcpy(IPAddress6, pt);
        pt = strtok(NULL, separ);
	if(pt != NULL)
            prefixlen = atoi(pt);
    }

    //Time
    timesetting = apCfgTimeModeGet();
    timesetting = timesetting?0:1;

    {
        char tpbuff[128];
        separ = " ";
        
        getTimeofDay(tpbuff, TIME_FORMAT_NORMAL);
        strcpy(date_p, strtok(tpbuff, separ));
        strtok(NULL, separ); //eat weekday
        strcpy(time_p, strtok(NULL, separ));
        strcpy(timeZone, apCfgTimezoneOffsetGet());        
    }
    
    usDefntp = apCfgNtpModeGet();
    daylightSaving=apCfgDaylightSavingGet();
    strcpy(ntpServer, apCfgNtpServerGet());
    
    //Setup_advanced
#ifdef _BONJOUR_
    htBonjour = apCfgBonjourGet(); //add by carole
#endif
    htForce100m = apCfgForce100mGet();
    htAutoNegotiation = apCfgAutonegoGet();
    htPortSpeed=apCfgPortspeedGet();
    htDuplexMode=apCfgDuplexmodeGet();
    //htEthDataRate = 
    htRedirect = apCfgRedirectModeGet();
    if(htRedirect)
        strcpy(htRedirectUrl, apCfgRedirectUrlGet());
    ethDot1xAuth = scApCfgDot1xSuppEnableGet();
    if(ethDot1xAuth)
    {
        authViaMac = scApCfgDot1xSuppMacEnableGet();
        strcpy(authName, scApCfgDot1xSuppUsernameGet());
        strcpy(authPasswd, scApCfgDot1xSuppPasswordGet());

    }

    //Wireless_Basic
    wlsMode = (apCfgWlanStateGet(unit) == 0)? 0 : 
              ((apCfgFreqSpecGet(unit) == MODE_SELECT_11B)? 1: 
                ((apCfgFreqSpecGet(unit) == MODE_SELECT_11G)?2:
                  ((apCfgFreqSpecGet(unit) == MODE_SELECT_11N)?3:  
                    ((apCfgFreqSpecGet(unit) == MODE_SELECT_11BG)? 4:7))));
    if(apCfgAutoChannelGet(unit))
        channel = 0;
    else
        channel=apCfgRadioChannelGet(unit);
    
    vapIsolated=apCfgInterVapForwardingGet(unit);    

    for(vap = 0; vap<WLAN_MAX_VAP; vap++)
    {
        if(!apCfgActiveModeGet(unit, vap))
            continue;
        sprintf(ssid[vap], "%s", apCfgSsidGet(unit, vap));     
	    ssidBroadcast[vap] = !apCfgSsidModeGet(unit, vap);

	    //Wireless_security
	    wlsSeparation[vap]=apCfgIntraVapForwardingGet(unit, vap);
	    secSystem[vap] = (apCfgAuthTypeGet(unit, vap) == APCFG_AUTH_NONE) ? 0 :
    	                 ((apCfgAuthTypeGet(unit, vap) == APCFG_AUTH_WPAPSK) ? 2 :
    	                  ((apCfgAuthTypeGet(unit, vap) == APCFG_AUTH_WPA2PSK) ? 3 :
    	                   ((apCfgAuthTypeGet(unit, vap) == APCFG_AUTH_WPA_AUTO_PSK) ? 4:
    	                     ((apCfgAuthTypeGet(unit, vap) == APCFG_AUTH_WPA) ? 5 :
    	                      ((apCfgAuthTypeGet(unit, vap) == APCFG_AUTH_WPA2) ? 6: 
    	                       ((apCfgAuthTypeGet(unit, vap) == APCFG_AUTH_WPA_AUTO) ? 7:
	                           ((apCfgAuthTypeGet(unit, vap) == APCFG_AUTH_DOT1X) ? 8:1)))))));
	    authType[vap] = (apCfgAuthTypeGet(unit, vap) == APCFG_AUTH_AUTO) ? 0 : 
	                    ((apCfgAuthTypeGet(unit, vap) == APCFG_AUTH_SHARED_KEY) ? 2 : 1);
	    if(secSystem[vap] == 1)
	   	{    
	   	    encrypt[vap] = (apCfgKeyBitLenGet(unit, vap) == 40)?64:128;
	   	}
	   	else
	   	{
	   	    encrypt[vap] = (apCfgWPACipherGet(unit, vap) == WPA_CIPHER_TKIP)?1:
	   	                    ((apCfgWPACipherGet(unit, vap) == WPA_CIPHER_AES)?2:3);
	   	}
	    keyNo[vap] = apCfgDefKeyGet(unit, vap) - CFG_MIN_KEY+1;
	    memset(key1[vap], 0, sizeof(key1[vap]));
	    memset(key2[vap], 0, sizeof(key2[vap]));
	    memset(key3[vap], 0, sizeof(key3[vap]));
	    memset(key4[vap], 0, sizeof(key4[vap]));
	    memset(preShared[vap],0,sizeof(preShared[vap]));
    	sprintf(key1[vap], "%s", apCfgKeyValGet(unit, vap, 1));
    	sprintf(key2[vap], "%s", apCfgKeyValGet(unit, vap, 2));
    	sprintf(key3[vap], "%s", apCfgKeyValGet(unit, vap, 3));
    	sprintf(key4[vap], "%s", apCfgKeyValGet(unit, vap, 4));
    	strcpy(preShared[vap], apCfgPassphraseGet(unit, vap));
	    keyrenew[vap] = apCfgGroupKeyUpdateIntervalGet(unit, vap);
	    
	    //Wireless_Connection_Control
	    aclFlag[vap] = apCfgAclModeGet(unit, vap);
        aclType[vap] = apCfgAclTypeGet(unit, vap);

        
        while (!scAclBufGet(unit, vap, &pScAcl)) ;
    	if(pScAcl){
    	    for(count = 0,pScAclCurr = pScAcl; pScAclCurr&&count<20; pScAclCurr = pScAclCurr->next){
    	        scHexs2Chars(pScAclCurr->mac, aclMac[vap][count], 6, 1);
    	    	count++;
    		}
        } 
        scAclBufFree(unit, vap, pScAcl);
        
        //VLAN_Qos
        if(apCfgVlanModeGet()) {
            vapVlan[vap] = apCfgVlanPvidGet(unit, vap);
            vapPri[vap] = apCfgPriorityGet(unit, vap);
        }
        vapWmm[vap] = apCfgWmeGet(unit, vap);
        //Wireless_Advanced
        if(apCfgBalanceModeGet(unit))
            vapBalance[vap] = apCfgLoadBalanceGet(unit, vap);
    }
    vap = 0;
    //Radius_Server
    acpcfg_edit_setip(authSIP1, apCfgRadiusServerGet(unit, vap));    
    authSPort1=apCfgRadiusPortGet(unit, vap);
    sprintf(authSSecret1,"%s",apCfgRadiusSecretGet(unit, vap)); 
    acpcfg_edit_setip(authSIP2, apCfgBackupRadiusServerGet(unit, vap));    
    authSPort2=apCfgBackupRadiusPortGet(unit, vap);    
    sprintf(authSSecret2,"%s",apCfgBackupRadiusSecretGet(unit, vap));        

    //VLAN_Qos
    vlan = apCfgVlanModeGet();
    if(vlan){
        nativeVid = apCfgNativeVlanIdGet();
        manageVid = apCfgManagementVlanIdGet();
        vlanTag = apCfgNativeVlanTagGet();
        wdsTag = apCfgWdsVlanTagGet();
        strcpy(wdsVlanList, apCfgwdsVlanListGet());
    }

    //Wireless_Advanced
    worldwide = scApCfg80211dEnabledGet(unit);
    countryCode = apCfgCountryCodeGet();
    channelBand = (apCfgChannelWidthModeGet(unit)==CWM_MODE_20M)?1:
                    ((apCfgChannelWidthModeGet(unit)==CWM_MODE_40M)?2:0); 
    guardInter = (scApCfgShortGIGet(unit)==SHORTGI_SHORT)?1:
                    ((scApCfgShortGIGet(unit)==SHORTGI_LONG)?2:0); 
    ctsProtect = apCfgCTSModeGet(unit); 
    beaconInterval=apCfgBeaconIntervalGet(unit);
    dtimInter = scApCfgDtimIntervalGet(unit); 
    rtsThreshold=apCfgRtsThresholdGet(unit);
    fragLength = apCfgFragThresholdGet(unit);
    loadBalance = apCfgBalanceModeGet(unit); 

    //AP_Mode

        opMode = (apCfgOpModeGet(unit)== CFG_OP_MODE_AP_PTP)? 1:
                    ((apCfgOpModeGet(unit)== CFG_OP_MODE_MPT)? 2:
                     ((apCfgOpModeGet(unit)== CFG_OP_MODE_UC || 
                    apCfgOpModeGet(unit)== CFG_OP_MODE_UR)?3:
                      (apCfgOpModeGet(unit)== CFG_OP_MODE_ROGAP)?4:0));
                        
    wdsRepeater = (apCfgOpModeGet(unit) == CFG_OP_MODE_AP_PTMP)?1:0;
    ucrRepeater = (apCfgOpModeGet(unit) == CFG_OP_MODE_UR)?1:0;
    //Remote MAC
    char remotemac[20];
    if(wdsRepeater || opMode == 2)
    {
        int i;

        for(i=0; i<4; i++)
        {
            memset(pxpMac[i],0,sizeof(pxpMac[i]));
            apCfgRemoteWbrMacAddrGet(unit, i, remotemac);
            if(strcmp("00:00:00:00:00:00", remotemac) != 0)
                scHexs2Chars(remotemac, pxpMac[i], 6, 1);
        }
    }
    else if(opMode == 1)
    {
        apCfgRemoteApMacAddrGet(unit, remotemac);
        if(strcmp("00:00:00:00:00:00", remotemac) != 0)
            scHexs2Chars(remotemac, pxpMac[0], 6, 1);
    }
    else if(opMode == 3)
    {
       #ifdef __ICONV_SUPPORT__ 
	char converted_text[128] = {0};
	char pDest[128];
	char *pSrc=NULL;
	int ret=0;
	pSrc=apCfgSsidGet(unit, vap);
	 ret=do_convert(LAN2UTF, pSrc, strlen(pSrc), converted_text, 128);
	if(ret!=-1){
		if(strlen(converted_text)<128)
			strcpy(pDest, converted_text);
		else
			strcpy(pDest, pSrc);
		sprintf(perfssid, "%s",pDest);
	}
	else{
		 sprintf(perfssid, "%s",pSrc);
	}
#else
        sprintf(perfssid, "%s", apCfgSsidGet(unit, vap));   
#endif
        apCfgUcrRemoteApMacAddrGet(unit, remotemac);
        if(strcmp("00:00:00:00:00:00", remotemac) != 0)
            scHexs2Chars(remotemac, pxpMac[0], 6, 1);
    }
    {
        char flag;
        flag = scApCfgRogueApTypeGet();
        noSecurity = (0x01 & flag)?1:0;
        illegalAp = (0x02 & flag)?1:0;
    }
    if(opMode == 4)
    {
        char *pList = scApCfgLegalApListGet();
        char *delim= ",";
        char *pValue = NULL;
        count = 0 ;
        
        rogueApMacList.no = rogueApMacList.outNo = 0;
        if (strlen(pList)){
            pValue = strtok(pList, delim);
            rogueApMacList.no++;
            scMacStr12ToStr17(pValue, rogueApMacList.mac[count++], ":");
            while((pValue = strtok(NULL, delim)) != NULL)
            {
                rogueApMacList.no++;
                scMacStr12ToStr17(pValue, rogueApMacList.mac[count++], ":");
            }
        }
        //rogueApNo = rogueApMacList.no;

        //rogueAPMacList_s rogueApMacList;
    }
    //Management
    strcpy(loginName, apCfgLoginGet());
    strcpy(loginPasswd, apCfgPasswordGet());
    httpsEnable = apCfgHttpsModeGet();
    webAccess = apCfgWlanAccessGet();
    secureSh = apCfgSSHGet();
    snmp = apCfgSnmpModeGet();
    if(snmp) {
        strcpy(contact, apCfgSnmpContactGet());
        strcpy(snmplocal, apCfgSnmpLocationGet());
        strcpy(snmpdevice, apCfgDescGet());
        strcpy(getCom, apCfgSnmpReadCommGet());
        strcpy(setCom, apCfgSnmpWriteCommGet());
        strcpy(trapCom, apCfgSnmpTrapCommunityGet());
        snmpTrust = apCfgSnmpAnyManagerGet();
        if(snmpTrust != 1);
        {
            unsigned char trapendIp[4];
            ipAddress = apCfgSnmpManagerIpGet();
            memcpy(trapStartIp, &(ipAddress), 4);
       
            ipAddress = apCfgSnmpManagerIpEndGet();
            memcpy(trapendIp, &(ipAddress), 4);
            trapRange = trapendIp[3];
        }
        
        ipAddress = apCfgSnmpTrapRecvIpGet();
        memcpy(trapServerIp, &(ipAddress), 4);
    }
    //Log
    
    mailAlert = apCfgemailAlertsEnabledGet();
    if(mailAlert) {
        strcpy(smtpServ, apCfgsmtpMailServerGet());
        strcpy(mailAddr, apCfgemailAddrForLogGet());
        logLength = apCfgemailAlertsQlenGet();
        logTime = scApCfgemailAlertsIntervalGet();
    }
    syslogF = apCfgsysLogEnabledGet();
    ipAddress = inet_addr(apCfgsysLogServerGet());
    memcpy(logIPAddr, &(ipAddress), 4);
    unAthLogin = apCfgDeauthGet();
    AthLogin = apCfgAuthLoginGet();
    sysError = apCfgChangeSysFucGet();
    cfgChange = apCfgChangeCfgGet();
}

void parseCFGSet(A_UINT8 type)
{
    char *separ;
    A_UINT32 ipAddress;
    A_UINT8 unit=RADIO_24G, vap=0;
    int i;
    char ipstrtemp[20], macAddrStr[20], zeroMac[6], zeroMac17[17];
    char ipv6Buf[45];
    
    memset(zeroMac,0,sizeof(zeroMac));
	memset(zeroMac17,0,sizeof(zeroMac17));
    //BASIC
    if(strlen(APName)!=0){
    	if(scValidHostName(APName, strlen(APName)))
        	apCfgSysNameSet(APName);
    }
    if(strlen(APDesc)!=0){
        apCfgDescSet(APDesc);
    } 
    if(strlen(APLang)!=0){
        apCfgSysLangSet(APLang);
    } 
    apCfgDhcpEnableSet(dhcpF);
    apCfgDhcp6EnableSet(dhcp6F);
    memcpy(&(ipAddress),IPAddress4,4);
    if(ipAddress!=0)
        apCfgIpAddrSet(ipAddress);
        
    memcpy(&(ipAddress),subnet4,4);
    if(ipAddress!=0)
        apCfgIpMaskSet(ipAddress);
        
    memcpy(&(ipAddress),gateway4, 4);
    apCfgGatewayAddrSet(ipAddress);  
      
    apCfgNameSrvSet(dnsServer1);
    apCfgNameSrv2Set(dnsServer2);
  	apCfgipv6modeSet(IPv6);
    sprintf(ipv6Buf, "%s/%d", IPAddress6, prefixlen);
    if(apCfgIpv6AddrSet(ipv6Buf)!=0)
    {
    		if((!apCfgIpv6AddrGet())||(strcmp(apCfgIpv6AddrGet(),"")==0)||(strcmp(apCfgIpv6AddrGet(),"\0")==0))
    			apCfgipv6modeSet(0);
    }
    
    apCfgGatewayv6AddrSet(gateway6);
    apCfgNameSrv61Set(IPv6dnsServer1);
    apCfgNameSrv62Set(IPv6dnsServer2);

    //Time
    apCfgTimeModeSet(!timesetting);
    if(!timesetting)
    {
        separ = "/";
        apCfgTimeYearSet(atoi(strtok(date_p, separ)));
        apCfgTimeMonSet(atoi(strtok(NULL, separ)));
        apCfgTimeDaySet(atoi(strtok(NULL, separ)));
        
        separ = ":";
        apCfgTimeHourSet(atoi(strtok(time_p, separ)));
        apCfgTimeMinSet(atoi(strtok(NULL, separ)));
        apCfgTimeSecSet(atoi(strtok(NULL, separ)));
    }
    apCfgTimezoneOffsetSet(timeZone);
    apCfgNtpModeSet(usDefntp);
    apCfgNtpServerSet(ntpServer);
    apCfgDaylightSavingSet(daylightSaving);
    
    //Setup_advanced
#ifdef _BONJOUR_
    apCfgBonjourSet(htBonjour);//add by carole
#endif
    apCfgForce100mSet(htForce100m);
    apCfgAutonegoSet(htAutoNegotiation);
    apCfgPortspeedSet(htPortSpeed);
    apCfgDuplexmodeSet(htDuplexMode);
    apCfgRedirectModeSet(htRedirect);
    apCfgRedirectUrlSet(htRedirectUrl);
    scApCfgDot1xSuppEnableSet(ethDot1xAuth);;
    scApCfgDot1xSuppMacEnableSet(authViaMac);
    if(strlen(authName))
        scApCfgDot1xSuppUsernameSet(authName);
    if(strlen(authPasswd))
        scApCfgDot1xSuppPasswordSet(authPasswd);
    
    //VLAN_QoS
    apCfgVlanModeSet(vlan);
    apCfgVlanListClear();
    if(vlan) {
        apCfgNativeVlanIdSet(nativeVid);
        apCfgManagementVlanIdSet(manageVid);
        apCfgNativeVlanTagSet(vlanTag);
        apCfgWdsVlanTagSet(wdsTag);
        apCfgwdsVlanListSet(wdsVlanList);
    }
    
    //Wireless_Basic
    if(wlsMode)
        apCfgWlanStateSet(unit,1);
    switch(wlsMode)
    {
        default:
            apCfgFreqSpecSet(unit,MODE_SELECT_11BGN);     
        case 0:
            apCfgWlanStateSet(unit,0);
            break;
        case 1:
            apCfgFreqSpecSet(unit,MODE_SELECT_11B);
            break;
        case 2:
            apCfgFreqSpecSet(unit,MODE_SELECT_11G);
            break;
        case 3:
            apCfgFreqSpecSet(unit,MODE_SELECT_11N);
            break;     
        case 4:
            apCfgFreqSpecSet(unit,MODE_SELECT_11BG);
            break; 
        case 7:
            apCfgFreqSpecSet(unit,MODE_SELECT_11BGN);
            break; 
    }
    apCfgAutoChannelSet(unit, !channel);  
    if(channel != 0)
        apCfgRadioChannelSet(unit, channel);
    
    apCfgInterVapForwardingSet(unit, vapIsolated);
    
    /* AP Mode will affect ssid number, so we parse ap mode first */
    //AP_Mode
    switch(opMode)     
    {
        default:
        case 0:
            if(wdsRepeater)
                apCfgOpModeSet(unit, CFG_OP_MODE_AP_PTMP);
            else
                apCfgOpModeSet(unit, CFG_OP_MODE_AP);
            break;
        case 1:
            apCfgOpModeSet(unit, CFG_OP_MODE_AP_PTP);
            break;
        case 2:
            apCfgOpModeSet(unit, CFG_OP_MODE_MPT);
            break;    
        case 3:
            if(ucrRepeater)
                apCfgOpModeSet(unit, CFG_OP_MODE_UR);
            else
                apCfgOpModeSet(unit, CFG_OP_MODE_UC);
            break;     
        case 4:
            apCfgOpModeSet(unit, CFG_OP_MODE_ROGAP);
            {
                char flag;
                flag = noSecurity | (illegalAp << 1);    
                scApCfgRogueApTypeSet(flag);
            }
            break;   
    }
    for(vap = 0; vap < WLAN_MAX_VAP; vap++)
    {     
        if(strlen(ssid[vap]) == 0){
            scSetDefaultVap(unit, vap);
            continue;            
        }
        apCfgActiveModeSet(unit, vap, 1);
        apCfgSsidSet(unit, vap, ssid[vap]);
        apCfgSsidModeSet(unit, vap, !ssidBroadcast[vap]);
        
        //Wireless_security
        apCfgIntraVapForwardingSet(unit, vap, wlsSeparation[vap]);
        switch(secSystem[vap])
        {
        case 0:     //None
            apCfgAuthTypeSet(unit, vap,APCFG_AUTH_NONE);
            break;
        case 1:     //WEP
        {
            int len;
            len = encrypt[vap]/8;
            apCfgKeyBitLenSet(unit, vap, encrypt[vap]);
            if(authType[vap]==0)
                apCfgAuthTypeSet(unit, vap, APCFG_AUTH_AUTO);
            else if(authType[vap]==2)
                apCfgAuthTypeSet(unit, vap, APCFG_AUTH_SHARED_KEY);
            else
                apCfgAuthTypeSet(unit, vap, APCFG_AUTH_OPEN_SYSTEM);         
            apCfgDefKeySet(unit, vap, keyNo[vap] + CFG_MIN_KEY-1);
            
            if(strlen(key1[vap]) && (strlen(key1[vap]) == len*2))
                apCfgKeyValSet(unit, vap, 1, key1[vap]);
            if(strlen(key2[vap]) && (strlen(key2[vap]) == len*2))
                apCfgKeyValSet(unit, vap, 2, key2[vap]);
            if(strlen(key3[vap]) && (strlen(key3[vap]) == len*2))
                apCfgKeyValSet(unit, vap, 3, key3[vap]);
            if(strlen(key4[vap]) && (strlen(key4[vap]) == len*2))
                apCfgKeyValSet(unit, vap, 4, key4[vap]);        

            break;
        }    
        case 2:     //WPA PSK
            apCfgAuthTypeSet(unit, vap, APCFG_AUTH_WPAPSK);
            apCfgWPACipherSet(unit, vap, encrypt[vap]);
            if(strlen(preShared[vap]))
                apCfgPassphraseSet(unit, vap,preShared[vap]);
            apCfgGroupKeyUpdateIntervalSet(unit, vap,keyrenew[vap]); 
            break;
        case 3:     //WPA2 PSK
            apCfgAuthTypeSet(unit, vap,APCFG_AUTH_WPA2PSK);
            apCfgWPACipherSet(unit, vap,WPA_CIPHER_AES);
            if(strlen(preShared[vap]))
                apCfgPassphraseSet(unit, vap,preShared[vap]);
            apCfgGroupKeyUpdateIntervalSet(unit, vap,keyrenew[vap]); 
            break;
        case 4:     //WPA/WPA2 PSK
            apCfgAuthTypeSet(unit, vap,APCFG_AUTH_WPA_AUTO_PSK);
            apCfgWPACipherSet(unit, vap,WPA_CIPHER_AUTO);
            if(strlen(preShared[vap]))
                apCfgPassphraseSet(unit, vap,preShared[vap]);
            apCfgGroupKeyUpdateIntervalSet(unit, vap,keyrenew[vap]); 
            break;
        case 5:     //WPA Radius
            apCfgAuthTypeSet(unit, vap,APCFG_AUTH_WPA);
            apCfgWPACipherSet(unit, vap,encrypt[vap]);
            apCfgGroupKeyUpdateIntervalSet(unit, vap,keyrenew[vap]); 
            break;
        case 6:     //WPA2 Radius
            apCfgAuthTypeSet(unit, vap,APCFG_AUTH_WPA2);
            apCfgWPACipherSet(unit, vap,WPA_CIPHER_AES);
            apCfgGroupKeyUpdateIntervalSet(unit, vap,keyrenew[vap]); 
            break;
        case 7:     //WPA/WAP2 Radius
            apCfgAuthTypeSet(unit, vap,APCFG_AUTH_WPA_AUTO);
            apCfgWPACipherSet(unit, vap,WPA_CIPHER_AUTO);
            apCfgGroupKeyUpdateIntervalSet(unit, vap,keyrenew[vap]); 
            break;
        case 8:     //802.1x
        {    
            apCfgAuthTypeSet(unit, vap,APCFG_AUTH_DOT1X);
            break;
        }    
        default:
            break;
        }
        
        //Wireless_alc
        apCfgAclModeSet(unit, vap, aclFlag[vap]);
        apCfgAclTypeSet(unit, vap, aclType[vap]);
        
        apCfgAclClear(unit, vap); 
        for(i=0; i<CFG_MAX_ACL_LINKSYS; i++) {
            if(memcmp(aclMac[vap][i], zeroMac, 6)){
                macAddrToString(aclMac[vap][i], macAddrStr, ":");
                apCfgAclAdd(unit, vap, macAddrStr, "unknown", 1);
            }
        }
        
        //VLAN&Qos
        apCfgVlanPvidSet(unit, vap, vapVlan[vap]);
        apCfgWmeSet(unit, vap, vapWmm[vap]);
        apCfgPrioritySet(unit, vap, vapPri[vap]); 
        
        //Wireless_Advanced
        apCfgLoadBalanceSet(unit, vap, vapBalance[vap]);
	}
	//Add VLAN ID end then it will apply Vlan List
    apCfgVlanListApply(unit);
    
	vap = 0;
    //Radius_Server
    sprintf(ipstrtemp, "%u.%u.%u.%u", authSIP1[0], authSIP1[1], authSIP1[2], authSIP1[3]);
	apCfgRadiusServerSet(unit, vap, ipstrtemp);
	apCfgRadiusPortSet(unit, vap, authSPort1);
	if(strlen(authSSecret1))
	    apCfgRadiusSecretSet(unit, vap, authSSecret1);  
    sprintf(ipstrtemp, "%u.%u.%u.%u", authSIP2[0], authSIP2[1], authSIP2[2], authSIP2[3]);
	apCfgBackupRadiusServerSet(unit, vap, ipstrtemp);
	apCfgBackupRadiusPortSet(unit, vap, authSPort2);
	if(strlen(authSSecret2))
	    apCfgBackupRadiusSecretSet(unit, vap, authSSecret2);  
    
    //Wireless_Advanced
    scApCfg80211dEnabledSet(unit, worldwide);
    apCfgCountryCodeSet(countryCode);
    (channelBand==CWM_MODE_40M)?apCfgChannelWidthModeSet(unit, channelBand):apCfgChannelWidthModeSet(unit, !channelBand);
    (guardInter==SHORTGI_LONG)?scApCfgShortGISet(unit, guardInter):scApCfgShortGISet(unit, !guardInter);
    
    apCfgCTSModeSet(unit, ctsProtect); 
    apCfgBeaconIntervalSet(unit, beaconInterval);
    scApCfgDtimIntervalSet(unit, dtimInter);
    apCfgRtsThresholdSet(unit, rtsThreshold);
    apCfgFragThresholdSet(unit, fragLength);
    apCfgBalanceModeSet(unit, loadBalance); 
    
    //Remote AP
    {
        if(opMode == 1 || opMode == 3) {
            if(memcmp(pxpMac[0], zeroMac, 6) == 0){
                strcpy(macAddrStr, "00:00:00:00:00:00");
            }else{
                macAddrToString(pxpMac[0], macAddrStr, ":");
            }
            apCfgRemoteApMacAddrSet(unit, macAddrStr);
            if(opMode == 3)
                apCfgSsidSet(unit, vap, perfssid);
        }
        else if(opMode == 0 || opMode == 2) {
            for(i=0; i<4; i++)
            {
                 if(memcmp(pxpMac[i], zeroMac, 6) == 0){
                    strcpy(macAddrStr, "00:00:00:00:00:00");
                }else{
                    macAddrToString(pxpMac[i], macAddrStr, ":");
                }
                apCfgRemoteWbrMacAddrSet(unit, i, macAddrStr);
            }
        }
    }
    //Rogue AP mac
    {
        scApCfgLegalApListClear();
        for(i=0; i<rogueApMacList.no; i++) {
            if(memcmp(rogueApMacList.mac[i], zeroMac17, 17)){
                scMacStr17ToStr12((A_UINT8*)rogueApMacList.mac[i], (A_UINT8*)macAddrStr);
                scApCfgLegalApListAdd(macAddrStr);
            }
        }
    }
    
    //Management
    apCfgSecMaskSet(secShow);
    apCfgLoginSet(loginName);
    if(strlen(loginPasswd))
        apCfgPasswordSet(loginPasswd); 
    
    apCfgAutohttpsModeSet(httpsEnable);
    apCfgHttpsModeSet(httpsEnable);
    apCfgWlanAccessSet(webAccess);
    apCfgSSHSet(secureSh);
    apCfgSnmpModeSet(snmp);
    if(snmp) {
        unsigned char trapendIp[4];
        
        apCfgSnmpContactSet(contact);
        apCfgSnmpLocationSet(snmplocal);
        apCfgDescSet(snmpdevice);
        apCfgSnmpReadCommSet(getCom);
        apCfgSnmpWriteCommSet(setCom);
        apCfgSnmpTrapCommunitySet(trapCom);
        apCfgSnmpAnyManagerSet(snmpTrust);
        
        memcpy(&(ipAddress),trapStartIp,4);
        if(ipAddress!=0)
            apCfgSnmpManagerIpSet(ipAddress);
        
        trapendIp[3] = trapRange;
        memcpy(&(ipAddress),trapendIp,4);
        memcpy(&(ipAddress),trapStartIp,3);
        if(ipAddress!=0 && ipAddress >= (A_UINT32)apCfgSnmpManagerIpGet())
            apCfgSnmpManagerIpEndSet(ipAddress);
        else
            apCfgSnmpManagerIpEndSet((int)apCfgSnmpManagerIpGet());
            
        memcpy(&(ipAddress),trapServerIp,4);
        if(ipAddress!=0)
            apCfgSnmpTrapRecvIpSet(ipAddress);
    }
    
    //Log
    apCfgemailAlertsEnabledSet(mailAlert);
    if(mailAlert) {
        apCfgsmtpMailServerSet(smtpServ);
        apCfgemailAddrForLogSet(mailAddr);
        scApCfgemailAlertsQlenSet(logLength);
        scApCfgemailAlertsIntervalSet(logTime);
    }
    apCfgsysLogEnabledSet(syslogF);
    
    sprintf(ipstrtemp, "%u.%u.%u.%u", logIPAddr[0], logIPAddr[1], logIPAddr[2], logIPAddr[3]);
    apCfgsysLogServerSet(ipstrtemp);
    
    apCfgDeauthSet(unAthLogin);
    apCfgAuthLoginSet(AthLogin);
    apCfgChangeSysFucSet(sysError);
    apCfgChangeCfgSet(cfgChange);    
}    
#endif    

#if 0
int main(void)
{
    parseCFG("/var/wap4410n.cfg", 0xFF, 1);
    return 0;
}
#endif
