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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <stdarg.h>
#include "nvram.h"
#include "utility.h"
#include "apcfg.h"
#include "apply.h"

//#define CHECK_11N
extern int scfgmgr_set(char *,char *);

int apcprint(const char *format, ...)
{
#if T_DEBUG    
    va_list args;
    FILE *fp;

    fp = fopen("/var/log/printbug", "a+");

    if (!fp) {
        fprintf(stderr, "fp is NULL\n");
	    return -1;
    }

    va_start(args, format);
    vfprintf(fp, format, args);
    va_end(args);

    fclose(fp);
#endif    
    return 0;
}

#ifdef T_DEBUG
int apcfgLog(const char *format, ...)
{
    va_list args;
    FILE *fp;

    fp = fopen("/var/log/apcfg_dbg", "a+");

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
#endif /* DEBUG */


#define NVRAM_FREE(x)    if(x!=NULL) free(x); 

struct 	scAclBuf_s scAclBuf[WLAN_MAX_DEV][WLAN_MAX_VAP];
static	A_BOOL	scAclBufUsed[WLAN_MAX_DEV][WLAN_MAX_VAP] = {{FALSE}};

SC_COUNTRY allCountryList[]={
    {"Asia",            CTRY_KOREA_ROC},
    {"Australia",       CTRY_AUSTRALIA},
    {"Canada",          CTRY_CANADA},
    {"Denmark",         CTRY_DENMARK},
    {"Europe",          CTRY_AUSTRIA},
    {"Finland",         CTRY_FINLAND},
    {"France",          CTRY_FRANCE},
    {"Germany",         CTRY_GERMANY},
    {"Ireland",         CTRY_IRELAND},
    {"Italy",           CTRY_ITALY},
    {"Japan",           CTRY_JAPAN},
    {"Mexico",          CTRY_MEXICO},
    {"Netherlands",     CTRY_NETHERLANDS},
    {"NewZealand",      CTRY_NEW_ZEALAND},
    {"Norway",          CTRY_NORWAY},
    {"PuertoRico",      CTRY_PUERTO_RICO},
    {"SouthAmerica",    CTRY_HONDURAS},
    {"Spain",           CTRY_SPAIN},
    {"Sweden",          CTRY_SWEDEN},
    {"Switzerland",     CTRY_SWITZERLAND},
    {"UnitedKingdom",   CTRY_UNITED_KINGDOM},
    {"UnitedStates",    CTRY_UNITED_STATES} 
};

A_STATUS apCfgSsidModeSet(int unit, int bss, int v)
{
    char name[60];
    char *string;
    
    if(apCfgSsidModeGet(unit,bss) != v)
    {
        string = (v==1)?ENABLE:DISABLE;
        sprintf(name,"wlan%d_ssid%d_broabcast",unit,bss);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

int apCfgSsidModeGet(int unit, int bss)
{
    char name[60];
    int value;
    char *p;
    
    sprintf(name,"wlan%d_ssid%d_broabcast",unit,bss);
    p = nvram_safe_get(name);
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
	
}

A_STATUS apCfgSsidSet(int unit, int bss, char *p)
{
    char name[60];
    
    if(strlen(p) > MAX_SSID)
        return A_ERROR;
    
    if(strcmp(p, apCfgSsidGet(unit, bss))!=0)
    {
        sprintf(name,"wlan%d_ssid%d",unit,bss);
        scfgmgr_set(name,p);
        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
        apCfgWscConfiguredSet(1);
    }
    return A_OK;
}

char *
apCfgSsidGet(int unit, int bss)
{
	char name[60];
	static char value[64];
	char *p = NULL;
    sprintf(name,"wlan%d_ssid%d",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}

char *apCfgDevicePinGet(int unit, int bss)
{
    char name[60], *p;
    static char value[8+1];
    sprintf(name,"wlan%d_ssid%d_pin",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS apCfgDevicePinSet(int unit, int bss, char *p)
{
    char name[60];
    
    if(strlen(p) != WPS_PIN_LEN)
        return A_ERROR;
    
    if(strcmp(p, apCfgDevicePinGet(unit, bss))!=0)
    {
        sprintf(name,"wlan%d_ssid%d_pin",unit,bss);
        scfgmgr_set(name,p);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
    }
	
    return A_OK;
}

char *
apCfgVapNameGet(int unit,int bss)
{
    char name[60], *p;
    static char value[MAX_VAPNAME+1];
    sprintf(name,"wlan%d_vap%d",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS apCfgVapNameSet(int unit,int bss,char *p)
{
    char name[60];
    
    if(strlen(p) > MAX_VAPNAME)
        return A_ERROR;
        
    sprintf(name,"wlan%d_vap%d",unit,bss);
    scfgmgr_set(name,p);
    return A_OK;
}

A_STATUS apCfgActiveModeSet(int unit,int bss,int v)
{
    char name[60];  
    char *string;

    /*The first vap could not be disabled*/
    if(bss == 0 && v == 0){
        return A_ERROR;
    }
    /*In ptp or ptmp mode, only the first could be enabled*/
    if(apCfgOpModeGet(unit) != CFG_OP_MODE_AP)
    {
        if(bss !=0 && v==1)
            return A_ERROR;
    }
    if(v!=apCfgActiveModeGet(unit,bss))
    { 
        sprintf(name,"wlan%d_ssid%d_state",unit,bss);
        if(v==1)
        {
    	    string=ENABLE;
        }
        else 
        {
           	string=DISABLE;
        }
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

int apCfgActiveModeGet(int unit, int bss)
{
    int value;
    char *p, name[60];
    
    sprintf(name,"wlan%d_ssid%d_state",unit,bss);
    p = nvram_safe_get(name);
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS
apCfgCTSModeSet(int unit, int v)
{
    char name[60];
    char string[10];
    
    if(v!=apCfgCTSModeGet(unit))
    {
        sprintf(string,"%d",v); 
        sprintf(name,"wlan%d_cts",unit);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

A_BOOL
apCfgCTSModeGet(int unit)
{
    char name[60];
    int value;
    char *p;
    
    sprintf(name,"wlan%d_cts",unit);
    p = nvram_safe_get(name);
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS
apCfgBeaconIntervalSet(int unit, int v)
{
    char name[60];
    char string[10];
    
    if(v < MIN_BEACON_PERIOD || v > MAX_BEACON_PERIOD)
        return A_ERROR;
    
    if(v!=apCfgBeaconIntervalGet(unit))
    {
        sprintf(string,"%d",v); 
        sprintf(name,"wlan%d_bcn",unit);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

int
apCfgBeaconIntervalGet(int unit)
{
    char name[60];
    int value;
    char *p;
    
    sprintf(name,"wlan%d_bcn",unit);
    p = nvram_safe_get(name);
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS apCfgSysNameSet(char *p)
{
    if(strlen(p) < 1 || strlen(p) > MAX_SYSNAME)
        return A_ERROR;
        
    if(strcmp(p,apCfgSysNameGet()))
    {
        scfgmgr_set("sys_name",p);
        apl_set_flag(APL_SYSNAME);
#ifdef _BONJOUR_
        apl_set_flag(APL_BONJOUR);
#endif
    }
    return A_OK;
}	
char *apCfgSysNameGet(void)
{
    char name[60], *p;
    static char value[MAX_SYSNAME+1];
    sprintf(name,"sys_name");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}	

A_STATUS apCfgSysLangSet(char *p)
{
    if(strlen(p) < 0 || strlen(p) > MAX_DESC)
        return A_ERROR;

    if(strcmp(p,apCfgSysLangGet()))
    {
        scfgmgr_set("sys_lang",p);
    }

    return A_OK;
}
char * apCfgSysLangGet(void)
{
    char name[60], *p;
    static char value[MAX_DESC+1];
    sprintf(name,"sys_lang");

    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);

    return value;
}

A_STATUS apCfgDescSet(char *p)
{
    if(strlen(p) < 0 || strlen(p) > MAX_DESC)
        return A_ERROR;

    if(strcmp(p,apCfgDescGet()))
    {
        scfgmgr_set("sys_desc",p);
    }

    return A_OK;
}
char * apCfgDescGet(void)
{
    char name[60], *p;
    static char value[MAX_DESC+1];
    sprintf(name,"sys_desc");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS
apCfgLoginSet(char *p)
{
    if(strlen(p) < 1 || strlen(p) > CFG_MAX_USERNAME)
        return A_ERROR;
        
    if(strcmp(p, apCfgLoginGet()))
    {
        scfgmgr_set("login_username",p);
        apl_set_flag(APL_LOGIN);
    }
    return A_OK;
    
}

char *
apCfgLoginGet(void)
{
    char name[60], *p;
    static char value[CFG_MAX_USERNAME+1];
    sprintf(name,"login_username");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS
apCfgPasswordSet(char *p)
{
    if(strlen(p)<4 || strlen(p) > CFG_MAX_PASSWORD)
        return A_ERROR;
        
    if(strcmp(p, apCfgPasswordGet()))
    {
        scfgmgr_set("login_password",p);
        apl_set_flag(APL_LOGIN);
    }
    return A_OK;
}

char *
apCfgPasswordGet(void)
{
    char name[60], *p;
    static char value[CFG_MAX_PASSWORD+1];
    sprintf(name,"login_password");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS
apCfgVlanListApply(int unit)
{    
    int vap;
    char vlanStr[60];
    int vlanList[MAX_VLAN_LIST];
    int len = 0;
    int i = 0,j=0;  

    vlanList[i++]=apCfgNativeVlanIdGet();
    vlanList[i++] = apCfgManagementVlanIdGet();
           
    for(vap=0; vap<WLAN_MAX_VAP; vap++) {
        vlanList[i++] = apCfgVlanPvidGet(unit, vap);
        
    }
    scCompositor(vlanList, 0, i-1);
    for(j=0; j<i; j++){
        if(j != 0)
        if(vlanList[j] == vlanList[j-1])
            continue;
        len += sprintf(vlanStr+len, "%d,", vlanList[j]);
    }

    apCfgVlanListSet(vlanStr);
    return A_OK;
}

A_STATUS
apCfgVlanListAdd(int v)
{
    char *vlanList ;
    char *delim=",";
    char *pValue;
    char vlan[MAX_VLAN_LIST];
    
    if(v < VLAN_TAG_MIN || v > VLAN_TAG_MAX)
        return A_ERROR;
    
    vlanList = apCfgVlanListGet();
    
    pValue = strtok(vlanList,delim);
    if(pValue != NULL) {
        if(v==atoi(pValue)){
            return A_ERROR;
        }
        while((pValue=strtok(NULL,delim)) != NULL){
            if(v==atoi(pValue))
                return A_ERROR;
        } 
    }
    sprintf(vlan, "%s%d,", apCfgVlanListGet(), v);

    apCfgVlanListSet(vlan);
    
    return A_OK;
}
A_STATUS
apCfgVlanListSet(char *p)
{
    if(strcmp(p,apCfgVlanListGet()))
    {
        scfgmgr_set("vlan_list",p);
        apl_set_flag(APL_VLAN);
    }
    return A_OK;
}
A_STATUS
apCfgVlanListDel(int v)
{
    char *vlanList ;
    char *p;
    char vlan[5];
    int len;
    
    len = sprintf(vlan, "%d,", v);    
    vlanList = apCfgVlanListGet();
    p = vlanList;

    while( *p != '\0') {
        if(memcmp(p, vlan, len) == 0)
        {
            if(p == vlanList)
                vlanList += len;
            else if(*(p+len) == '\0')
                *p = 0;
            else
                memmove(p, p+len, strlen(p)-len);
            break;
        }
        p = strchr(p, ',');
        p++; /* jump ',' */
    }

    apCfgVlanListSet(vlanList);
    
    return A_OK;
}
A_STATUS
apCfgVlanListClear()
{
    char *p = "";
    
    scfgmgr_set("vlan_list",p);
    
    return A_OK;
}
char *
apCfgVlanListGet(void)
{
    char name[60], *p;
    static char value[64+1];
    sprintf(name,"vlan_list");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS apCfgipv6modeSet(int v)
{
    char *string;  

    if(v!=apCfgipv6modeGet())
    {
        string = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("lan_ipv6",string);
        /* if disable IPv6,reboot this devices */
        if(v == 0)
        	apl_set_flag(APL_REBOOT);
        else
        apl_set_flag(APL_IPv6);
    }
    return A_OK;
}

int apCfgipv6modeGet()
{
    int value;
    char *p;
    
    p = nvram_safe_get("lan_ipv6");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}
/*A_STATUS
apCfgipv6wanmodeSet(int v)
{
    char string[5];
    if(v != apCfgipv6wanmodeGet())
    {
        sprintf(string, "%d", v);
  
        scfgmgr_set("lan_fixipv6",string);
        apl_set_flag(APL_IPv6);
    }
    return A_OK;
}
int apCfgipv6wanmodeGet()
{
    int value;
    char *p;
    
    p = nvram_safe_get("lan_fixipv6");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}
*/
A_STATUS apCfgDhcpEnableSet(int v)
{   
    char *string;  
    
    if(v!=apCfgDhcpEnableGet())
    {
        string = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("lan_dhcpc",string);
        apl_set_flag(APL_IPv4);
    }
    return A_OK;
}

int apCfgDhcpEnableGet(void)
{
    int value=0;
    char *p=NULL;
    
    p = nvram_safe_get("lan_dhcpc");
    if(p)
    {
        value = (strcmp(p,ENABLE) == 0)?1:0;
        NVRAM_FREE(p);
    }
    return value;
}

/* add for dhcp client v6 */
/* add begin =>*/
A_STATUS apCfgDhcp6EnableSet(int v)
{
    char *string;  
    
    if(v!=apCfgDhcp6EnableGet())
    {
        if(v==1)
        {
    	    string=ENABLE;
        }
        else 
        {
           	string=DISABLE;
        }
        scfgmgr_set("lan_dhcp6c",string);
        apl_set_flag(APL_IPv6);
    }
    return A_OK;
}

int apCfgDhcp6EnableGet(void)
{    
    int value;
    char *p;
    
    p = nvram_safe_get("lan_dhcp6c");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}
/* <= add end*/
/* Add for Radvd */
A_STATUS apCfgRadvdEnableSet(int v)
{
    char *string;  
    
    if(v!=apCfgRadvdEnableGet())
    {
        if(v==1)
        {
    	    string=ENABLE;
        }
        else 
        {
           	string=DISABLE;
        }
        scfgmgr_set("lan_radvd",string);
        apl_set_flag(APL_IPv6);
    }
    return A_OK;
}

int apCfgRadvdEnableGet(void)
{    
    int value;
    char *p;
    
    p = nvram_safe_get("lan_radvd");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}
/* <= add end */
A_STATUS apCfgDhcpServerEnableSet(int v)
{   
    char *string;  
    if(v!=apCfgDhcpServerEnableGet())
    {
        string = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("lan_dhcps",string);
        apl_set_flag(APL_IPv4);
    }
    return A_OK;
}

int apCfgDhcpServerEnableGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("lan_dhcps");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS
apCfgIpv6AddrSet(char *addr)
{
    if(!scValidIPv6(addr, 1))
        return A_ERROR;
    if(strcmp(addr, apCfgIpv6AddrGet()))
    {
        scfgmgr_set("lan_ipaddrv6", addr);
        apl_set_flag(APL_IPv6);
    }    
    return A_OK;
}

char *
apCfgIpv6AddrGet(void)
{
    char name[60], *p;
    static char value[INET6_ADDRSTRLEN+1];
    sprintf(name,"lan_ipaddrv6");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS 
apCfgGatewayv6AddrSet(char *gw)
{
    if(strlen(gw) && !scValidGWv6(gw))
        return A_ERROR;
    if(strcmp(gw,apCfgGatewayv6AddrGet()))
    {
        scfgmgr_set("lan_gatewayv6",gw);
        apl_set_flag(APL_IPv6);
    }
    return A_OK;
}
char *
apCfgGatewayv6AddrGet(void)
{
    char name[60], *p;
    static char value[INET6_ADDRSTRLEN+1];
    sprintf(name,"lan_gatewayv6");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS 
apCfgNameSrv61Set(char *dns)
{
    if(strlen(dns) && !scValidIPv6(dns, 0))  //change 1 to 0, 20081107
        return A_ERROR;
    if(strcmp(dns,apCfgNameSrv61Get()))
    {
        scfgmgr_set("lan_dnsv61",dns);
        apl_set_flag(APL_IPv6);
    }
    return A_OK;
}
char *
apCfgNameSrv61Get(void)
{
    char name[60], *p;
    static char value[INET6_ADDRSTRLEN+1];
    sprintf(name,"lan_dnsv61");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS 
apCfgNameSrv62Set(char *dns)
{
    if(strlen(dns) && !scValidIPv6(dns, 0))  //change 1 to 0 ,20081107
        return A_ERROR;
    if(strcmp(dns,apCfgNameSrv62Get()))
    {
        scfgmgr_set("lan_dnsv62",dns);
        apl_set_flag(APL_IPv6);
    }
    return A_OK;
}
char *
apCfgNameSrv62Get(void)
{
    char name[60], *p;
    static char value[INET6_ADDRSTRLEN+1];
    sprintf(name,"lan_dnsv62");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}
//ip addr
A_STATUS
apCfgIpAddrSet(int v)
{    
    struct in_addr ipaddress;
    char *string1;
    
    if(v!=apCfgIpAddrGet())
    {
        ipaddress.s_addr=v;
        string1=inet_ntoa(ipaddress);
        scfgmgr_set("lan_ipaddr",string1);
        apl_set_flag(APL_IPv4);
#ifdef _BONJOUR_
        apl_set_flag(APL_BONJOUR);
#endif
    }
    return A_OK;
}

int
apCfgIpAddrGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("lan_ipaddr");
    value =  inet_addr(p);
    NVRAM_FREE(p);
    
    return value;   
}
A_STATUS
apCfgIpMaskSet(int v)
{
    struct in_addr ipaddress;
    char *string1;
    
    if(v!=apCfgIpMaskGet())
    {
        ipaddress.s_addr=v;
        string1=inet_ntoa(ipaddress);
        scfgmgr_set("lan_netmask",string1);
        apl_set_flag(APL_IPv4);
    }
    return A_OK;
}

int
apCfgIpMaskGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("lan_netmask");
    value =  inet_addr(p);
    NVRAM_FREE(p);
    
    return value;   
}
A_STATUS
apCfgGatewayAddrSet(int v)
{
    struct in_addr ipaddress;
    char *string1;
    
    if(v!=apCfgGatewayAddrGet())
    {
        ipaddress.s_addr=v;
        string1=inet_ntoa(ipaddress);
        scfgmgr_set("lan_gateway",string1);
        apl_set_flag(APL_IPv4);
    }
    return A_OK;
}

int
apCfgGatewayAddrGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("lan_gateway");
    value =  inet_addr(p);
    NVRAM_FREE(p);
    
    return value;   
}
A_STATUS
apCfgNameSrvSet(char *p)
{
    if(strlen(p) < 0 || strlen(p) > MAX_NAME_SERVER)
        return A_ERROR;
        
    if(strcmp(p,apCfgNameSrvGet()))
    {
        scfgmgr_set("lan_dns1",p);
        apl_set_flag(APL_IPv4);
    }
    return A_OK;
}
char *
apCfgNameSrvGet(void)
{
    char name[60], *p;
    static char value[MAX_NAME_SERVER+1];
    sprintf(name,"lan_dns1");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS
apCfgNameSrv2Set(char *p)
{
    if(strlen(p) < 0 || strlen(p) > MAX_NAME_SERVER)
        return A_ERROR;
        
    if(strcmp(p,apCfgNameSrv2Get()))
    {
        scfgmgr_set("lan_dns2",p);
        apl_set_flag(APL_IPv4);
    }
    return A_OK;
}
char *
apCfgNameSrv2Get(void)
{
    char name[60], *p;
    static char value[MAX_NAME_SERVER+1];
    sprintf(name,"lan_dns2");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS
apCfgDhcpServerStartSet(char *p)
{
    if(strcmp(p, apCfgDhcpServerStartGet()))
    {
        scfgmgr_set("lan_dhcps_start",p);
        apl_set_flag(APL_IPv4);
    }
    return A_OK;
}
char *
apCfgDhcpServerStartGet(void)
{
    char name[60], *p;
    static char value[32+1];
    sprintf(name,"lan_dhcps_start");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS
apCfgDhcpServerEndSet(char *p)
{
    if(strcmp(p, apCfgDhcpServerEndGet()))
    {
        scfgmgr_set("lan_dhcps_end",p);
        apl_set_flag(APL_IPv4);
    }
    return A_OK;
}
char *
apCfgDhcpServerEndGet(void)
{
    char name[60], *p;
    static char value[32+1];
    sprintf(name,"lan_dhcps_end");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS
apCfgNameDomainSet(char *p)
{
    if(strlen(p) <= 0 || strlen(p) > CFG_MAX_DOMAINNMLEN)
        return A_ERROR;
        
    scfgmgr_set("sys_domain_suffix",p);
    return A_OK;
}

char *
apCfgNameDomainGet(void)
{
    char name[60], *p;
    static char value[CFG_MAX_DOMAINNMLEN+1];
    sprintf(name,"sys_domain_suffix");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS
apCfgCountryCodeSet(int v)
{    
    char string[10];
    
    if(!check_country_domain(v))
        return A_ERROR;
    if(v!=apCfgCountryCodeGet())
    {
        sprintf(string,"%d",v); 
        scfgmgr_set("sys_domain",string);
        apl_set_flag(APL_REBOOT);
        
        if(CTRY_JAPAN == v){
            apCfgChannelWidthModeSet(RADIO_24G, CWM_MODE_20M);            
        }
    }
    return A_OK;
}

int
apCfgCountryCodeGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("sys_domain");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS
apCfgSecMaskSet(int v)
{    
    char string[10];
    if(v!=apCfgSecMaskGet())
    {
        sprintf(string,"%d",v); 
        scfgmgr_set("secret_mask",string);     
    }
    return A_OK;
}

A_BOOL
apCfgSecMaskGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("secret_mask");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}
/*
 *mike added for WINS
 */
A_STATUS
apCfgWinsServerSet(char *p)
{
    if(strlen(p) < 0 || strlen(p) > MAX_WINS_SERVER)
        return A_ERROR;
        
    if(strcmp(p,apCfgWinsServerGet()))
    {
        scfgmgr_set("wins_server",p);
    }
    return A_OK;
}

char *
apCfgWinsServerGet(void)
{
    char name[60], *p;
    static char value[MAX_WINS_SERVER+1];
    sprintf(name,"wins_server");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}
/* 
 *end
 */
A_STATUS
apCfgTimeModeSet(int v)
{
    char string[10];
    
    if(v != apCfgTimeModeGet())
    {
        sprintf(string,"%d",v);
        scfgmgr_set("tod_enable",string); 
        apl_set_flag(APL_NTP);       
    }
    return A_OK;
}

int apCfgTimeModeGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("tod_enable");
    value = (strcmp(p, ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS
apCfgTimeSet(int v)
{
    char string[10];
    
    if(v != apCfgTimeGet())
    {
        sprintf(string,"%d",v);
        /*
        apCfgTimeMonSet(v);
        apCfgTimeDaySet(v;
        apCfgTimeYearSet(v);
        apCfgTimeHourSet(v);
        apCfgTimeMinSet(v);
        apCfgTimeSecSet(v);
        */
    }
    return A_OK;
}
int apCfgTimeGet(void)
{
    int time = 0;
    /*
    apCfgTimeMonGet();
    apCfgTimeDayGet();
    apCfgTimeYearGet();
    apCfgTimeHourGet();
    apCfgTimeMinGet();
    apCfgTimeSecGet();
    */
    return time;
}
A_STATUS
apCfgTimeMonSet(int v)
{
    char string[10];
    if(v != apCfgTimeMonGet())
    {
        sprintf(string, "%d", v);
        scfgmgr_set("tod_mon",string);
        apl_set_flag(APL_NTP);
    }
    return A_OK;
}
int apCfgTimeMonGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("tod_mon");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS
apCfgTimeDaySet(int v)
{
    char string[10];
    if(v != apCfgTimeDayGet())
    {
        sprintf(string, "%d", v);
        scfgmgr_set("tod_day",string);
        apl_set_flag(APL_NTP);
    }
    return A_OK;
}
int apCfgTimeDayGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("tod_day");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS
apCfgTimeYearSet(int v)
{
    char string[10];
    if(v != apCfgTimeYearGet())
    {
        sprintf(string, "%d", v);
        scfgmgr_set("tod_year",string);
        apl_set_flag(APL_NTP);
    }
    return A_OK;
}
int apCfgTimeYearGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("tod_year");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS
apCfgTimeHourSet(int v)
{
    char string[10];
    if(v != apCfgTimeHourGet())
    {
        sprintf(string, "%d", v);
        scfgmgr_set("tod_hour",string);
        apl_set_flag(APL_NTP);
    }
    return A_OK;
}
int apCfgTimeHourGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("tod_hour");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS
apCfgTimeMinSet(int v)
{
    char string[10];
    if(v != apCfgTimeMinGet())
    {
        sprintf(string, "%d", v);
        scfgmgr_set("tod_min",string);
        apl_set_flag(APL_NTP);
    }
    return A_OK;
}
int apCfgTimeMinGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("tod_min");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS
apCfgTimeSecSet(int v)
{
    char string[10];
    if(v != apCfgTimeSecGet())
    {
        sprintf(string, "%d", v);
        scfgmgr_set("tod_sec",string);
        apl_set_flag(APL_NTP);
    }
    return A_OK;
}
int apCfgTimeSecGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("tod_sec");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS apCfgNtpModeSet(int v)
{
    char string[10];
    
    if(v != apCfgNtpModeGet())
    {
        sprintf(string,"%d",v);
        scfgmgr_set("ntp_mode",string);
        apl_set_flag(APL_NTP);        
    }
    return A_OK;
}

int apCfgNtpModeGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("ntp_mode");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS
apCfgNtpServerSet(char *p)
{
    if(strlen(p) < 0 || strlen(p) > MAX_NTP_SERVER)
        return A_ERROR;
        
    if(strcmp(p,apCfgNtpServerGet()))
    {
        scfgmgr_set("ntp_server",p);
        apl_set_flag(APL_NTP);
    }
    return A_OK;
}

char *
apCfgNtpServerGet(void)
{
    char name[60], *p;
    static char value[MAX_NTP_SERVER+1];
    sprintf(name,"ntp_server");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}

char * apCfgTimezoneOffsetGet(void)
{
    char name[60], *p;
    static char value[CFG_TIME_ZONE_LEN+1];
    sprintf(name,"timezone_diff");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS apCfgTimezoneOffsetSet(char *p)
{
    if(strlen(p) < 0 || strlen(p) > CFG_TIME_ZONE_LEN)
        return A_ERROR;
        
    if(strcmp(p, apCfgTimezoneOffsetGet()))
    {
        scfgmgr_set("timezone_diff",p);
        apl_set_flag(APL_NTP);
    }
    return A_OK;
}


A_STATUS
apCfgDaylightSavingSet(int v)
{    
    char *val; 
      
    if(v!=apCfgDaylightSavingGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("timezone_daylightsaving",val);
        apl_set_flag(APL_NTP);
    }
    return A_OK;
}

int apCfgDaylightSavingGet()
{
    int value;
    char *p;
    
    p = nvram_safe_get("timezone_daylightsaving");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}

/*mike add for ftp*/
A_STATUS
apCfgFtpServerSet(char *p)
{
    if(strlen(p) < 0 || strlen(p) > MAX_FTP_SERVER)
        return A_ERROR;   
    if(strcmp(p,apCfgFtpServerGet()))
    {
        scfgmgr_set("ftp_server",p);
        apl_set_flag(APL_FTP);
    }
    return A_OK;
}

char *
apCfgFtpServerGet(void)
{
    char name[60], *p;
    static char value[MAX_FTP_SERVER+1];
    sprintf(name,"ftp_server");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}


A_STATUS apCfgFtpPathSet(char *p)
{
    if(strcmp(p,apCfgFtpPathGet()))
    {
        scfgmgr_set("ftp_path",p);
        apl_set_flag(APL_FTP);
    }
    return A_OK;
}
char *   apCfgFtpPathGet(void)
{
    char name[60], *p;
    static char value[128+1];
    sprintf(name,"ftp_path");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}


A_STATUS apCfgFtpLoginSet(char *p)
{
    if(strlen(p) > CFG_MAX_FTP_LOGINNAME)
        return A_ERROR;
        
    if(strcmp(p, apCfgFtpLoginGet()))
    {
        scfgmgr_set("ftp_login_name",p);
        apl_set_flag(APL_FTP);
    }
    return A_OK;
    
}
char *   apCfgFtpLoginGet(void)
{
    char name[60], *p;
    static char value[CFG_MAX_FTP_LOGINNAME+1];
    sprintf(name,"ftp_login_name");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS apCfgFtpPasswdSet(char *p)
{
    if(strlen(p) > CFG_MAX_FTP_PASSWORD)
        return A_ERROR;
        
    if(strcmp(p, apCfgFtpPasswdGet()))
    {
        scfgmgr_set("ftp_password",p);
        apl_set_flag(APL_FTP);
    }
    return A_OK;
}
char *   apCfgFtpPasswdGet(void)
{
    char name[60], *p;
    static char value[CFG_MAX_FTP_PASSWORD+1];
    sprintf(name,"ftp_password");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}

/*added end*/

A_STATUS apCfglltdSet(int v)
{    
    char *val;
    if(v!=apCfglltdGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("lltd",val);
        apl_set_flag(APL_LLTD);
    }
    return A_OK;
}

A_BOOL apCfglltdGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("lltd");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS apCfgStpSet(int v)
{    
    char *val;
    if(v!=apCfgStpGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("stp",val);
        apl_set_flag(APL_STP);
    }
    return A_OK;
}

A_BOOL apCfgStpGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("stp");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}

#ifdef _BONJOUR_
// add by carole
A_STATUS apCfgBonjourSet(int v)
{
    char *val;
    if(v!=apCfgBonjourGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("bonjour_enable",val);
        apl_set_flag(APL_BONJOUR);
    }
    return A_OK;
}
A_BOOL apCfgBonjourGet(void)
{   
    int value;
    char *p;
    p = nvram_safe_get("bonjour_enable");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    return value;
}
#endif
A_STATUS apCfgForce100mSet(int v)
{
    char *val;
    if(v!=apCfgForce100mGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("lan_force100m",val);

    }
        apl_set_flag(APL_FORCE100M);
    return A_OK;
}
A_BOOL apCfgForce100mGet(void)
{
    int value;
    char *p;
    p = nvram_safe_get("lan_force100m");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    return value;
}

A_STATUS apCfgAutonegoSet(int v)
{
//    char *val;
    if(v!=apCfgAutonegoGet())
    {
//        val = (v==1)?ENABLE:DISABLE;
				if(v==0)
        scfgmgr_set("lan_autonego","0");
				else
        scfgmgr_set("lan_autonego","1");
        apl_set_flag(APL_FORCE100M);
    }
    return A_OK;
}

A_BOOL apCfgAutonegoGet(void)
{
    int value;
    char *p;
    p = nvram_safe_get("lan_autonego");
		if(atoi(p) == 1)
			value=1;
		else
			value=0;	
//    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    return value;
}

A_BOOL apCfgPortspeedSet(int v)
{
//    char *val;

    if(v!=apCfgPortspeedGet())
    {
				if(v == 0)
				{	
        scfgmgr_set("lan_portspeed_define","0");
				scfgmgr_set("eth_data_rate","10Mbps");
				}
				else if(v ==1)
				{	
        scfgmgr_set("lan_portspeed_define","1");
				scfgmgr_set("eth_data_rate","100Mbps");
					}
				else
				{
				scfgmgr_set("eth_data_rate","1000Mbps");
        scfgmgr_set("lan_portspeed_define","2");
					}
        apl_set_flag(APL_FORCE100M);
    }
    return A_OK;
}
A_BOOL apCfgPortspeedGet(void)
{
    int value;
    char *p;
    p = nvram_safe_get("lan_portspeed_define");
		if(atoi(p)==1)
			value =1;
		else if(atoi(p)==2)
			value =2;
		else
			value =0;	
    NVRAM_FREE(p);
    return value;
}

A_BOOL apCfgDuplexmodeSet(int v)
{
 //   char *val;
    if(v!=apCfgDuplexmodeGet())
    {
				if(v == 0)
        scfgmgr_set("lan_duplexmode","0");
				else if(v ==1)
        scfgmgr_set("lan_duplexmode","1");
        apl_set_flag(APL_FORCE100M);
    }
    return A_OK;
}
A_BOOL apCfgDuplexmodeGet(void)
{
    int value;
    char *p;
    p = nvram_safe_get("lan_duplexmode");
		if(atoi(p) == 1)
			value =1;
		else
			value=0;
    NVRAM_FREE(p);
    return value;
}


A_STATUS apCfgRedirectModeSet(int v)
{
    char *val;
    if(v!=apCfgRedirectModeGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("rediret_mode",val);
        apl_set_flag(APL_HTTPREDIRECT);
    }
    return A_OK;
}
A_BOOL apCfgRedirectModeGet(void)
{   
    int value;
    char *p;
    
    p = nvram_safe_get("rediret_mode");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS apCfgRedirectUrlSet(char *url)
{ 
    if(strlen(url) >128)
        return A_ERROR;
    if(strcmp(url, apCfgRedirectUrlGet()) != 0)
    {
        scfgmgr_set("rediret_url",url);
        apl_set_flag(APL_HTTPREDIRECT);
    }
    return A_OK;
}
char *apCfgRedirectUrlGet(void)
{
    char name[60], *p;
    static char value[128+1];
    sprintf(name,"rediret_url");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS apCfgFactoryRestore(void)
{    
    scfgmgr_set("restore_defaults", "1");
    return A_OK;
}

A_STATUS scApCfgRogueDetectSet(int v)
{
	char *val;
	
	if(v!=scApCfgRogueDetectGet())
	{
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("rogue_mode",val);
        apl_set_flag(APL_ROGUEAP);
    }
    return A_OK;
}

int scApCfgRogueDetectGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("rogue_mode");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}

/* unit: minuts */
A_STATUS scApCfgRogueDetectIntSet(char v)
{
	char string[10];
	if (v>=0x03 && v<=0x63){
	    if(v != scApCfgRogueDetectIntGet())
	    {
			sprintf(string,"%d",v); 
		    scfgmgr_set("rogue_interval",string);
	        apl_set_flag(APL_ROGUEAP);
    	}
	    return A_OK;
	}
	return A_EINVAL;
}

char scApCfgRogueDetectIntGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("rogue_interval");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}

/* 0x1 for insecurity, 0x2 for illegal, 0x3 for both */
A_STATUS scApCfgRogueApTypeSet(char v)
{
	char string[10];	
	if (v>=0x00 && v<=0x03){
	    if(v != scApCfgRogueApTypeGet())
	    {
		    sprintf(string,"%d",v);	     
		    scfgmgr_set("rogue_type",string);
	        apl_set_flag(APL_ROGUEAP);
    	}
	    return A_OK;
	}
	return A_EINVAL;
}

char scApCfgRogueApTypeGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("rogue_type");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}

/* 0x3 for AP log and syslog, 0x3|0x4 for AP log, syslog ,snmp trap */
A_STATUS scApCfgRogueSendLogSet(char v)
{
	char string[10];	
	if (v>=0x0 && v<=0x07){
	    if(v != scApCfgRogueSendLogGet())
	    {
		    sprintf(string,"%d",v); 
		    scfgmgr_set("rogue_sendLog",string);
	        apl_set_flag(APL_ROGUEAP);
    	}
	    return A_OK;
	}
	return A_EINVAL;
}

char scApCfgRogueSendLogGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("rogue_sendLog");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}

/* atmost 32*12+1 */
A_STATUS scApCfgLegalApListSet(char *p)
{
    if(strlen(p) < 0 || strlen(p) > MAX_LEGALAPLIST)
        return A_ERROR;
    if(strcmp(p,scApCfgLegalApListGet()))
    {
		scfgmgr_set("legal_ap_list",p);
		apl_set_flag(APL_ROGUEAP);
	}
    return A_OK;
}

char *scApCfgLegalApListGet(void)
{
    char name[60], *p;
    static char value[MAX_LEGALAPLIST+1];
    sprintf(name,"legal_ap_list");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS scApCfgLegalApListAdd(char *pAddr)
{
    char *pList = scApCfgLegalApListGet();
    char *pValue = NULL;
    char newList[MAX_LEGALAPLIST];
    int  n = 0;

    pValue = strstr(pList, pAddr);
    if(pValue)
        return A_ERROR;
   
    n += sprintf(newList, "%s%s,", pList, pAddr);
    if (n<MAX_LEGALAPLIST)
    	scApCfgLegalApListSet(newList);
    else 
    	return A_ERROR;
    return A_OK;
}

A_STATUS scApCfgLegalApListDel(char *pAddr)
{
    char *pList = scApCfgLegalApListGet();
    char *pValue = NULL;
    char newList[MAX_LEGALAPLIST];
    
    pValue = strstr(pList, pAddr);
    if(!pValue)
        return A_ERROR;
   
    bzero(newList, sizeof(newList));
    memcpy(newList, pList, pValue-pList);
    if ((pValue-pList+13)<strlen(pList))	/* delAddr isnot the last one */
        strcpy(newList+(pValue-pList), strchr(pValue, ',')+1);
    else
        newList[pValue-pList] = '\0';
   
    scApCfgLegalApListSet(newList);
    return A_OK;
}
A_STATUS scApCfgLegalApListClear()
{
    char *value="";

	scfgmgr_set("legal_ap_list", value);
	apl_set_flag(APL_ROGUEAP);
    return A_OK;
}
A_STATUS apCfgNativeVlanIdSet(int v)
{     
    char string[10];
    
    if(v < VLAN_TAG_MIN || v > VLAN_TAG_MAX)
        return A_ERROR;
        
    if(v!=apCfgNativeVlanIdGet())
    {
        sprintf(string,"%d",v); 
        scfgmgr_set("vlan_default",string);
        apl_set_flag(APL_VLAN); 
    }
    return A_OK;
}

int apCfgNativeVlanIdGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("vlan_default");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS apCfgNativeVlanTagSet(int v)
{     
    char *string;  

    if(v!=apCfgNativeVlanTagGet())
    {
        string = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("vlan_default_tag",string);
        apl_set_flag(APL_VLAN); 
    }
    return A_OK;
}

int apCfgNativeVlanTagGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("vlan_default_tag");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS apCfgWdsVlanTagSet(int v)
{     
    char *string;  

    if(v!=apCfgWdsVlanTagGet())
    {
        string = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("vlan_wds_tag",string);
        apl_set_flag(APL_VLAN); 
    }
    return A_OK;
}

int apCfgWdsVlanTagGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("vlan_wds_tag");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS apCfgManagementVlanIdSet(int v)
{     
    char string[10];
    
    if(v < VLAN_TAG_MIN || v > VLAN_TAG_MAX)
        return A_ERROR;
        
    if(v!=apCfgManagementVlanIdGet())
    {
        sprintf(string,"%d",v); 
        scfgmgr_set("vlan_management",string);
        apl_set_flag(APL_VLAN);
    }
    return A_OK;
}

int apCfgManagementVlanIdGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("vlan_management");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}

A_UINT32 apCfgsysLogEnabledGet()
{
    int value;
    char *p;
    
    p = nvram_safe_get("syslog_mode");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS apCfgsysLogEnabledSet(A_UINT8 v)
{
    char *val;
    if(v!=apCfgsysLogEnabledGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("syslog_mode",val);
        apl_set_flag(APL_LOG);
    }
    return A_OK;
}

A_UINT32 apCfgsysLogBroadcastGet()
{
    int value;
    char *p;
    
    p = nvram_safe_get("syslog_broadcast");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS apCfgsysLogBroadcastSet(A_UINT8 v)
{
    char *val;
    if(v!=apCfgsysLogBroadcastGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("syslog_broadcast",val);
        apl_set_flag(APL_LOG);
    }
    return A_OK;
}

A_UINT32 apCfgsysLogSeverityGet()
{
    int value;
    char *p;
    
    p = nvram_safe_get("syslog_severity");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS apCfgsysLogSeveritySet(A_UINT8 v)
{
    char value[2];
    
    if(v < 1 || v > 6)
        return A_ERROR;
        
    if(v!=apCfgsysLogSeverityGet())
    {
        memset(value, 0, 2);
        sprintf(value,"%d",v);
        scfgmgr_set("syslog_severity",value);
        apl_set_flag(APL_LOG);
    }
    return A_OK;
}
A_UINT8 *apCfgsysLogServerGet()
{
    char name[60], *p;
    static char value[MAX_SYSLOG_SERVER+1];
    sprintf(name,"syslog_server");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}       
A_STATUS apCfgsysLogServerSet(char *p)
{
    if(strlen(p) < 0 || strlen(p) > MAX_SYSLOG_SERVER)
        return A_ERROR;
  
    if(strcmp(p, apCfgsysLogServerGet()))
    {
        scfgmgr_set("syslog_server",p);
        apl_set_flag(APL_LOG);
    }
    return A_OK;
}     
A_UINT16 apCfgsysLogServerPortGet()
{
    int value;
    char *p;
    
    p = nvram_safe_get("syslog_port");
    value = atoi(p);
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS apCfgsysLogServerPortSet(A_UINT32 v)
{
    char value[6];
    
    if(v<1 || v > 65534)
        return A_ERROR;
        
    memset(value, 0, 6);
    sprintf(value,"%ld",v);
    scfgmgr_set("syslog_port",value);
    return A_OK;
} 

A_BOOL apCfgDeauthGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("log_login_fail");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS apCfgDeauthSet(A_BOOL v)
{
    char *val;
    if(v!= apCfgDeauthGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("log_login_fail",val);
        apl_set_flag(APL_LOG);
    }
    return A_OK;
}

A_BOOL apCfgAuthLoginGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("log_login_success");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS apCfgAuthLoginSet(A_BOOL v)
{
    char *val;
    if(v!= apCfgAuthLoginGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("log_login_success",val);
        apl_set_flag(APL_LOG);
    }
    return A_OK;
}

A_BOOL apCfgChangeSysFucGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("log_system_errors");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS apCfgChangeSysFucSet(A_BOOL v)
{
    char *val;
    if(v!= apCfgChangeSysFucGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("log_system_errors",val);
        apl_set_flag(APL_LOG);
    }
    return A_OK;
}

A_BOOL apCfgChangeCfgGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("log_conf_change");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS apCfgChangeCfgSet(A_BOOL v)
{
    char *val;
    if(v!= apCfgChangeCfgGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("log_conf_change",val);
        apl_set_flag(APL_LOG);
    }
    return A_OK;
}

A_UINT32 apCfgemailAlertsEnabledGet()
{
    int value;
    char *p;
    
    p = nvram_safe_get("email_alert");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS apCfgemailAlertsEnabledSet(A_BOOL v)
{
    char *val;
    if(v!= apCfgemailAlertsEnabledGet())
    {     
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("email_alert",val);
        apl_set_flag(APL_LOG);
    }
    return A_OK;
}

A_STATUS 
scApCfgemailAlertsQlenSet(int v)
{
    char value[6];
    
    if(v<MIN_ALERT_QLEN || v >MAX_ALERT_QLEN)
        return A_ERROR;
    
    if(v!=apCfgemailAlertsQlenGet())
    {
        memset(value, 0, 6);
        sprintf(value,"%d",v);
        scfgmgr_set("email_queue_length",value);
        apl_set_flag(APL_LOG);
    }
    return A_OK;
}	
int 
apCfgemailAlertsQlenGet()
{
    int value;
    char *p;
    
    p = nvram_safe_get("email_queue_length");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS 
scApCfgemailAlertsIntervalSet(int v)
{
    char value[6];
    
    if(v<MIN_ALERT_INTERVAL || v >MAX_ALERT_INTERVAL)
        return A_ERROR;
    
    if(v!=scApCfgemailAlertsIntervalGet())
    {
        memset(value, 0, 6);
        sprintf(value,"%d",v);
        scfgmgr_set("email_send_period",value);
        apl_set_flag(APL_LOG);
    }
    return A_OK;
}	
int 
scApCfgemailAlertsIntervalGet()
{
    int value;
    char *p;
    
    p = nvram_safe_get("email_send_period");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}

A_UINT8 *apCfgsmtpMailServerGet()
{
    char name[60], *p;
    static char value[MAX_SMTP_SERVER+1];
    sprintf(name,"email_alert_server");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}       
A_STATUS apCfgsmtpMailServerSet(char *p)
{
    if(strlen(p) < 0 || strlen(p) > MAX_SMTP_SERVER)
        return A_ERROR;
  
    if(strcmp(p, apCfgsmtpMailServerGet()))
    {
        scfgmgr_set("email_alert_server",p);
        apl_set_flag(APL_LOG);
    }
    return A_OK;
} 

A_UINT8 *apCfgemailAddrForLogGet()
{
    char name[60], *p;
    static char value[MAX_MAIL_SERVER+1];
    sprintf(name,"email_addr");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}       
A_STATUS apCfgemailAddrForLogSet(char *p)
{
    if(strlen(p) < 0 || strlen(p) > MAX_MAIL_SERVER)
        return A_ERROR;
  
    if(strcmp(p, apCfgemailAddrForLogGet()))
    {
        scfgmgr_set("email_addr",p);
        apl_set_flag(APL_LOG);
    }
    return A_OK;
} 

A_UINT8 *apCfgemailAddrReturnGet()
{
    char name[60], *p;
    static char value[MAX_MAIL_SERVER+1];
    sprintf(name,"emailAddrReturn");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}       
A_STATUS apCfgemailAddrReturnSet(char *p)
{
    if(strlen(p) < 0 || strlen(p) > MAX_MAIL_SERVER)
        return A_ERROR;
  
    if(strcmp(p, apCfgemailAddrReturnGet()))
    {
        scfgmgr_set("emailAddrReturn",p);
        apl_set_flag(APL_LOG);
    }
    return A_OK;
} 

A_STATUS scApCfgDot1xSuppEnableSet(A_BOOL v)
{
    char *val;
    if(v!=scApCfgDot1xSuppEnableGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("eth_supp_mode",val);
        apl_set_flag(APL_ETHSUP);
    }
    return A_OK;
}

A_BOOL scApCfgDot1xSuppEnableGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("eth_supp_mode");
    value = (strcmp(p ,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);    
    
    return value;
}

A_STATUS scApCfgDot1xSuppMacEnableSet(A_BOOL v)
{
    char *val;
    if(v!=scApCfgDot1xSuppMacEnableGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("eth_supp_mac",val);
        apl_set_flag(APL_ETHSUP);
    }
    return A_OK;
}

A_BOOL scApCfgDot1xSuppMacEnableGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("eth_supp_mac");
    value = (strcmp(p ,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;  
}

A_STATUS scApCfgDot1xSuppUsernameSet(char *p)
{
    if(strlen(p) < 0 || strlen(p) > MAX_DOT1XSUPP_NAME)
        return A_ERROR;
        
    if(strcmp(p, scApCfgDot1xSuppUsernameGet()))
    {
        scfgmgr_set("eth_supp_user",p);
        apl_set_flag(APL_ETHSUP);
    }
    return A_OK;
}

char * scApCfgDot1xSuppUsernameGet(void)
{
    char name[60], *p;
    static char value[MAX_DOT1XSUPP_NAME+1];
    sprintf(name,"eth_supp_user");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS scApCfgDot1xSuppPasswordSet(char *p)
{
    if(strlen(p) < 4 || strlen(p) > MAX_DOT1XSUPP_PASSWD)
        return A_ERROR;
        
    if(strcmp(p, scApCfgDot1xSuppPasswordGet()))
    {
        scfgmgr_set("eth_supp_pwd",p);
        apl_set_flag(APL_ETHSUP);
    }
    return A_OK;
}

char * scApCfgDot1xSuppPasswordGet(void)
{
    char name[60], *p;
    static char value[MAX_DOT1XSUPP_PASSWD+1];
    sprintf(name,"eth_supp_pwd");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}

A_STATUS apCfgHttpModeSet(A_BOOL v)
{
    char *val;
    if(v!= apCfgHttpModeGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("http_mode",val);
        apl_set_flag(APL_HTTPD);
    }
    return A_OK;
}

A_BOOL apCfgHttpModeGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("http_mode");
    value = (strncmp(p,ENABLE,sizeof(ENABLE)) == 0)?1:0;
    NVRAM_FREE(p);

    return value; 
}

/*
 *mike added for autohttps
 */
A_STATUS apCfgAutohttpsModeSet(A_BOOL v)
{
    char *val;
    if(v!= apCfgAutohttpsModeGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("autohttps",val);
        apl_set_flag(APL_HTTPD);
#ifdef _BONJOUR_
        apl_set_flag(APL_BONJOUR);
#endif
    }
    return A_OK;
} 
 
A_BOOL apCfgAutohttpsModeGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("autohttps");
    value = (strncmp(p,ENABLE,sizeof(ENABLE)) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
} 
/* 
 *end
 */
 

A_STATUS
apCfgHttpPortSet(A_UINT16 v)
{
    char value[6];
    
    if(v<1 || v > 65534)
        return A_ERROR;
        
    if(v!=apCfgHttpPortGet())
    {
        memset(value, 0, 6);
        sprintf(value,"%d",v);
        scfgmgr_set("http_port",value);
        apl_set_flag(APL_HTTPD);
    }
    return A_OK;
}

A_UINT16
apCfgHttpPortGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("http_port");
    value = atoi(p);
    NVRAM_FREE(p);
    
    return value; 
}

A_STATUS apCfgHttpsModeSet(A_BOOL v)
{
    char *val;
    
    if(v!=apCfgHttpsModeGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("https_mode",val);
        apl_set_flag(APL_HTTPD);
    }
    return A_OK;
}

A_BOOL apCfgHttpsModeGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("https_mode");
    value = (strcmp(p, ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value; 
}

A_STATUS
apCfgHttpsPortSet(A_UINT16 v)
{
    char value[6];
    
    if(v<1 || v > 65534)
        return A_ERROR;
        
    if(v!=apCfgHttpsPortGet())
    {
        memset(value, 0, 6);
        sprintf(value,"%d",v);
        scfgmgr_set("https_port",value);
        apl_set_flag(APL_HTTPD);
    }
    return A_OK;
}

A_UINT16
apCfgHttpsPortGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("https_port");
    value = atoi(p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS
apCfgWlanAccessSet(A_BOOL v)
{
    char *string;
    if(v!=apCfgWlanAccessGet())
    {
        string = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("wlan_manage",string);
        apl_set_flag(APL_HTTPD);
    }
    return A_OK;
}

A_BOOL
apCfgWlanAccessGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("wlan_manage");
    value = (strcmp(p, ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS
apCfgSSHSet(A_BOOL v)
{
    char *string;
    if(v!=apCfgSSHGet())
    {
        string = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("SSH",string);
        apl_set_flag(APL_SSH);
#ifdef _BONJOUR_
        apl_set_flag(APL_BONJOUR);
#endif
    }
    return A_OK;
}

A_BOOL
apCfgSSHGet(void)
{
    int value;    
    char *p;
    
    p = nvram_safe_get("SSH");
    value = (strcmp(p, ENABLE) == 0)?1:0;
    NVRAM_FREE(p);

    return value;
}
A_STATUS
apCfgTelnetModeSet(A_BOOL v)
{
    char *val;
    if(v!=apCfgTelnetModeGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("telnet_mode",val);
        apl_set_flag(APL_HTTPD);
    }
    return A_OK;
}

A_BOOL
apCfgTelnetModeGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("telnet_mode");
    value = (strcmp(p, ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS
apCfgTelnetTimeoutSet(A_UINT32 v)
{
    char value[6];
    
    if(v<MIN_TELENT_TIMEOUT || v > MAX_TELNET_TIMEOUT)
        return A_ERROR;
        
    memset(value, 0, 6);
    sprintf(value,"%ld",v);
    scfgmgr_set("telnet_timeout",value);
    return A_OK;
}

A_UINT32
apCfgTelnetTimeoutGet(void)
{
    int value;
    char *p;
    p = nvram_safe_get("telnet_timeout");
    value = atoi(p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS
apCfgSnmpModeSet(A_BOOL v)
{
    char *val;
    if(v!= apCfgSnmpModeGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("snmp_mode",val);
        apl_set_flag(APL_SNMP);
    }
    return A_OK;
}

A_BOOL
apCfgSnmpModeGet(void)
{
    int value;
    char *p;
    p = nvram_safe_get("snmp_mode");
    value = (strcmp(p, ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}
/* snmp for linksys */
A_STATUS apCfgSnmpContactSet(char *p)
{
    if(strlen(p)< 1 || strlen(p) > MAX_SNMP_CONTACT)
        return A_ERROR;
    if(strcmp(p, apCfgSnmpContactGet()))
    {
        scfgmgr_set("snmp_contact",p);
        apl_set_flag(APL_SNMP);
    }
    return A_OK;
}
char *apCfgSnmpContactGet(void)
{
    char name[60], *p;
    static char value[MAX_SNMP_CONTACT+1];
    sprintf(name,"snmp_contact");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;    
}

A_STATUS apCfgSnmpDviceNameSet(char *p)
{
    if(strlen(p)< 1 || strlen(p) > MAX_SNMP_DEVNAME)
        return A_ERROR;
    if(strcmp(p, apCfgSnmpDviceNameGet()))
    {
        scfgmgr_set("snmp_device",p);
        apl_set_flag(APL_SNMP);
    }
    return A_OK;
}
char * apCfgSnmpDviceNameGet(void)
{
    char name[60], *p;
    static char value[MAX_SNMP_DEVNAME+1];
    sprintf(name,"snmp_device");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}

A_STATUS apCfgSnmpLocationSet(char *p)
{
    if(strlen(p)< 1 || strlen(p) > MAX_SNMP_LOCATION)
        return A_ERROR;
    if(strcmp(p, apCfgSnmpLocationGet()))
    {
        scfgmgr_set("snmp_location",p);
        apl_set_flag(APL_SNMP);
    }
    return A_OK;
}
char * apCfgSnmpLocationGet(void)
{
    char name[60], *p;
    static char value[MAX_SNMP_LOCATION+1];
    sprintf(name,"snmp_location");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}

A_STATUS apCfgSnmpTrapCommunitySet(char *p)
{
    if(strlen(p)< 1 || strlen(p) > MAX_SNMP_COMMUNITY)
        return A_ERROR;
    if(strcmp(p, apCfgSnmpTrapCommunityGet()))
    {
        scfgmgr_set("snmp_trap_community",p);
        apl_set_flag(APL_SNMP);
    }
    return A_OK;
}
char * apCfgSnmpTrapCommunityGet(void)
{
    char name[60], *p;
    static char value[MAX_SNMP_COMMUNITY+1];
    sprintf(name,"snmp_trap_community");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}

A_STATUS apCfgSnmpReadCommSet(char *p)
{
    if(strlen(p)<1 || strlen(p) > MAX_SNMP_COMMUNITY)
        return A_ERROR;
    if(strcmp(p, apCfgSnmpReadCommGet()))
    {
        scfgmgr_set("snmp_community_get",p);
        apl_set_flag(APL_SNMP);
    }
    return A_OK;
}

char * apCfgSnmpReadCommGet(void)
{
    char name[60], *p;
    static char value[MAX_SNMP_COMMUNITY+1];
    sprintf(name,"snmp_community_get");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}

A_STATUS apCfgSnmpWriteCommSet(char *p)
{
    if(strlen(p)<1 || strlen(p) > MAX_SNMP_COMMUNITY)
        return A_ERROR;
        
    if(strcmp(p, apCfgSnmpWriteCommGet()))
    {
        scfgmgr_set("snmp_community_set",p);
        apl_set_flag(APL_SNMP);
    }
    return A_OK;
}

char * apCfgSnmpWriteCommGet(void)
{
    char name[60], *p;
    static char value[MAX_SNMP_COMMUNITY+1];
    sprintf(name,"snmp_community_set");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}

A_STATUS apCfgSnmpUserNameSet(char *p)
{
    if(strlen(p)<0 || strlen(p) > MAX_SNMPV3_USERNAME)
        return A_ERROR;
            
    if(strcmp(p, apCfgSnmpUserNameGet()))
    {
        scfgmgr_set("snmp_user",p);
        apl_set_flag(APL_SNMP);
    }
    return A_OK;
}

char * apCfgSnmpUserNameGet(void)
{
    char name[60], *p;
    static char value[MAX_SNMPV3_USERNAME+1];
    sprintf(name,"snmp_user");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}

A_STATUS
apCfgSnmpAuthProtocolSet(int v)
{
    char *string;
    if(v!=apCfgSnmpAuthProtocolGet())
    {
        switch(v)
        {
            case SNMP_AUTH_NONE: 
                string="NONE";
        	    break;
            case SNMP_AUTH_MD5: 
                string="HMAC-MD5";
        	    break;				
            default:
                string="NONE";
        	    break;
        }
        scfgmgr_set("snmp_auth_protocol",string);
        apl_set_flag(APL_SNMP);
    }
    return A_OK;
}


int
apCfgSnmpAuthProtocolGet()
{
    char *p;
    int a = SNMP_AUTH_NONE;
    p=nvram_safe_get("snmp_auth_protocol");
    if (strcmp(p, "NONE") == 0) 
        a=APCFG_AUTH_NONE;
    else if (strcmp(p, "HMAC-MD5") == 0) 
        a=SNMP_AUTH_MD5;
    else 
        a=SNMP_AUTH_NONE;
    NVRAM_FREE(p);
    
    return a;
}

A_STATUS apCfgSnmpAuthKeySet(char *p)
{
    if(strlen(p)<MIN_SNMPV3_AUTHKEY || strlen(p) > MAX_SNMPV3_AUTHKEY)
        return A_ERROR;
            
    if(strcmp(p, apCfgSnmpAuthKeyGet()))
    {
        scfgmgr_set("snmp_auth_key",p);
        apl_set_flag(APL_SNMP);
    }
    return A_OK;
}

char * apCfgSnmpAuthKeyGet(void)
{
    char name[60], *p;
    static char value[32];
    sprintf(name,"snmp_auth_key");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}

A_STATUS
apCfgSnmpPrivProtocolSet(int v)
{
    char *string;
    if(v!=apCfgSnmpPrivProtocolGet())
    {
        switch(v)
        {
            case SNMP_PRIV_NONE: 
                string="NONE";
        	    break;
            case SNMP_PRIV_DES: 
                string="CBC-DES";
        	    break;				
            default:
                string="NONE";
        	    break;
        }
        scfgmgr_set("snmp_priv_protocol",string);
        apl_set_flag(APL_SNMP);
    }
    return A_OK;
}


int
apCfgSnmpPrivProtocolGet()
{
    char *p;
    int a = SNMP_PRIV_NONE;
    p = nvram_safe_get("snmp_priv_protocol");
    if (strcmp(p, "NONE") == 0) 
        a=SNMP_PRIV_NONE;
    else if (strcmp(p, "CBC-DES") == 0) 
        a=SNMP_PRIV_DES;
    else 
        a=SNMP_PRIV_NONE;
    NVRAM_FREE(p);
    
    return a;
}

A_STATUS apCfgSnmpPrivKeySet(char *p)
{
    if(strlen(p)<MIN_SNMPV3_PRIVKEY || strlen(p) > MAX_SNMPV3_PRIVKEY)
        return A_ERROR;    
        
    if(strcmp(p,apCfgSnmpPrivKeyGet()))
    {
        scfgmgr_set("snmp_priv_key",p);
        apl_set_flag(APL_SNMP);
    }
    return A_OK;
}

char * apCfgSnmpPrivKeyGet(void)
{
    char name[60], *p;
    static char value[32];
    sprintf(name,"snmp_priv_key");
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}

A_STATUS
apCfgSnmpAnyManagerSet(A_BOOL v)
{
    char *val;
    if(v!=apCfgSnmpAnyManagerGet())
    {
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("snmp_trusted_any",val);
        apl_set_flag(APL_SNMP);
    }
    return A_OK;
}

A_BOOL
apCfgSnmpAnyManagerGet(void)
{
    char *p;
    int value;
    
    p = nvram_safe_get("snmp_trusted_any");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS
apCfgSnmpManagerIpSet(int v)
{    
    struct in_addr ipaddress;
    char *string1;
    if(v!=apCfgSnmpManagerIpGet())
    {
        ipaddress.s_addr=v;
        string1=inet_ntoa(ipaddress);
        scfgmgr_set("snmp_trusted_host_start",string1);
        apl_set_flag(APL_SNMP);
    }
    return A_OK;
}
int
apCfgSnmpManagerIpGet(void)
{
    char *p;
    long int value;
    
    p = nvram_safe_get("snmp_trusted_host_start");
    value=inet_addr(p);
    NVRAM_FREE(p);
    
    return value;
}
/* mike add for linksys */
A_STATUS
apCfgSnmpManagerIpEndSet(int v)
{    
    struct in_addr ipaddress;
    char *string1;
//    if(v < apCfgSnmpManagerIpGet())
//        v = apCfgSnmpManagerIpGet();
    if(v!=apCfgSnmpManagerIpEndGet())
    {
        ipaddress.s_addr=v;
        string1=inet_ntoa(ipaddress);
        scfgmgr_set("snmp_trusted_host_end",string1);
        apl_set_flag(APL_SNMP);
    }
    return A_OK;
}
int
apCfgSnmpManagerIpEndGet(void)
{
    char *p;
    long int value;
    
    p = nvram_safe_get("snmp_trusted_host_end");
    value=inet_addr(p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS
apCfgSnmpTrapVersionSet(A_UINT32 v)
{
    char value[6];
    
    if(v<1 || v>3)
        return A_ERROR;
    
    if(v!=apCfgSnmpTrapVersionGet())
    {
        memset(value, 0, 6);
        sprintf(value,"%ld",v);
        scfgmgr_set("snmp_trap_version",value);
        apl_set_flag(APL_SNMP);
    }
    return A_OK;
}

A_UINT32
apCfgSnmpTrapVersionGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("snmp_trap_version");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS
apCfgSnmpTrapRecvIpSet(int v)
{    
    struct in_addr ipaddress;
    char *string1;
    if(v!=apCfgSnmpTrapRecvIpGet())
    {
        ipaddress.s_addr=v;
        string1=inet_ntoa(ipaddress);
        scfgmgr_set("snmp_trap_dest",string1);
        apl_set_flag(APL_SNMP);
    }
    return A_OK;
}

int
apCfgSnmpTrapRecvIpGet(void)
{
    char *p;
    p = nvram_safe_get("snmp_trap_dest");
    long int v=inet_addr(p);
    NVRAM_FREE(p);
    return v;
}

A_BOOL  isInvalidSnmpTrapPort(A_UINT32 v)
{
    int i;
    A_UINT32 invalidTrapPortArray[] = 
    {
        1, 2, 3, 5, 7, 9, 20, 21, 
        22, 23, 25, 31, 53, 67, 68, 69, 
        79, 80, 81, 82, 123, 137, 138, 139, 
        161, 168, 169, 443
    };
    
    for(i=0;i<28;i++){
        if( v == invalidTrapPortArray[i])
            return TRUE;
    }    
    return FALSE;
}
/*=============================================================================*/
int
apCfgNumVapsGet(int unit)
{
    char name[60];
    char *p, value[60];
    sprintf(name,"wlan%d_vaps",unit);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}

A_STATUS
apCfgNumVapsSet(int unit, int v)
{
    char name[60];
    char val='0'+v;
    sprintf(name,"wlan%d_vaps",unit);
    scfgmgr_set(name,&val);
    return A_OK;
}

A_STATUS
apCfgWlanStateSet(int unit,int v)
{
    char name[60];
    char *string;
    if(v!=apCfgWlanStateGet(unit))
    {
        if(v==1)
        {
            string=ENABLE;
        }
        else 
        {    
           	string=DISABLE;
        }
        sprintf(name,"wlan%d_state",unit);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

int apCfgWlanStateGet(int unit)
{
    char name[60];
    int value = 0;
    char *p;
    
    sprintf(name,"wlan%d_state",unit);
    p=nvram_safe_get(name);
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}
A_STATUS
apCfgOpModeSet(int unit, int v)
{
    char name[60];
    char *string;
    if(v!=apCfgOpModeGet(unit))
    {
        sprintf(name,"wlan%d_op_mode",unit);
        switch(v)
        {
            case CFG_OP_MODE_AP: 
                string="AccessPoint";
        	    break;
            case CFG_OP_MODE_STA: 
                string="WirelessClient";
        	    break;
            case CFG_OP_MODE_PPT: 
                string="WirelessBridge";
          	    break;
          	case CFG_OP_MODE_MPT: 
          	    string="MultiPointBridge";
          	    break;
          	case CFG_OP_MODE_REPEATER: 
          	    string="WirelessRepeater";
          	    break;
            case CFG_OP_MODE_AP_PTP: 
                string="AccessPointAndPTP";
        	    break;
            case CFG_OP_MODE_AP_PTMP: 
                string="AccessPointAndPTMP";
        	    break;
            case CFG_OP_MODE_UC: 
                string="UnversalClient";
        	    break;
            case CFG_OP_MODE_UR: 
                string="UnversalRepeater";
        	    break;
        	case CFG_OP_MODE_ROGAP:
        	    string="WirelessMonitor";
        	    break;    
            default:
        	    string="AccessPoint";
        	    break;
        }
        
        /*Not Ap mode, then correct the security settings*/
        if(CFG_OP_MODE_AP != v){
            int vap;
            for(vap = 1; vap < WLAN_MAX_VAP; vap++)
                apCfgActiveModeSet(unit, vap, 0);
            
            if(v == CFG_OP_MODE_UC || v == CFG_OP_MODE_UR) {
                int authType = apCfgAuthTypeGet(unit, 0);
                if(authType > APCFG_AUTH_AUTO && authType!=APCFG_AUTH_WPAPSK && authType!=APCFG_AUTH_WPA2PSK && authType!=APCFG_AUTH_WPA_AUTO_PSK){
                    apCfgAuthTypeSet(unit, 0, APCFG_AUTH_NONE);
                }
            }
            if(v == CFG_OP_MODE_ROGAP || v == CFG_OP_MODE_UC || v == CFG_OP_MODE_UR){
                apCfgChannelWidthModeSet(unit, 0);
            }
            if(v == CFG_OP_MODE_UR){
                apCfgInterVapForwardingSet(unit, 0);
            }
        }
        
        scfgmgr_set(name,string);
        apl_set_flag(APL_REBOOT);
        
        /* In ap mode, all disabled ssid will be active */
        if(CFG_OP_MODE_AP == v)
        {
            int vap;
            for(vap = 1; vap < WLAN_MAX_VAP; vap++){
                if(strlen(apCfgSsidGet(unit, vap)) != 0){
                    apCfgActiveModeSet(unit, vap, 1);
                }
            }
        }
    }
    return A_OK;
}

int
apCfgOpModeGet(int unit)
{
    char name[60];
    sprintf(name,"wlan%d_op_mode",unit);
    char *p=nvram_safe_get(name);
    int a = CFG_OP_MODE_AP;
    if (strcmp(p, "AccessPoint") == 0) {
        a=CFG_OP_MODE_AP;
    } else if (strcmp(p, "WirelessClient") == 0) {
        a=CFG_OP_MODE_STA;
    } else if (strcmp(p, "WirelessBridge") == 0) {
        a=CFG_OP_MODE_PPT;
    } else if (strcmp(p, "MultiPointBridge") == 0) {
        a=CFG_OP_MODE_MPT;
    } else if (strcmp(p, "WirelessRepeater") == 0) {
        a=CFG_OP_MODE_REPEATER;
    } else if (strcmp(p, "AccessPointAndPTP") == 0) {
        a=CFG_OP_MODE_AP_PTP;
    } else if (strcmp(p, "AccessPointAndPTMP") == 0) {
        a=CFG_OP_MODE_AP_PTMP;
    } else if (strcmp(p, "UnversalClient") == 0) {
    	a=CFG_OP_MODE_UC;	
    } else if (strcmp(p, "UnversalRepeater") == 0) {
    	a=CFG_OP_MODE_UR;
    } else if (strcmp(p, "WirelessMonitor") == 0) {
        a=CFG_OP_MODE_ROGAP;
    } else {
        a=CFG_OP_MODE_AP;
    }
    NVRAM_FREE(p);
    return a;
}

/* mike add for autoChannel */
A_STATUS
apCfgAutoChannelSet(int unit, int v)
{
    char name[60];
    char *val;
    
    if(v!= apCfgAutoChannelGet(unit))
    {
        val = (v==1)?ENABLE:DISABLE;
        sprintf(name,"wlan%d_auto_channel",unit);
        scfgmgr_set(name,val);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1)); 
        system("echo 1 > /tmp/chan_config");
    }
    return A_OK;
}
int
apCfgAutoChannelGet(int unit)
{
    int value;
    char name[60];
    char *p;
    sprintf(name,"wlan%d_auto_channel",unit);    
    p = nvram_safe_get(name);
    value = (strcmp(p, ENABLE) == 0)?1:0;
    return value; 
}
/* added end */

A_STATUS
apCfgWpsModeSet(int unit, int v)
{
    char name[60];
    char *val;

    if(v!= apCfgWpsModeGet(unit))
    {
        val = (v==1)?ENABLE:DISABLE;
        sprintf(name,"wlan%d_wps",unit);
        scfgmgr_set(name,val);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

A_STATUS
apCfgWpsPinERSet(int unit, int v)
{
    char name[60];
    char *val;

    if(v!= apCfgWpsPinERGet(unit))
    {
        val = (v==1)?ENABLE:DISABLE;
        sprintf(name,"wlan%d_wps_pin_er",unit);
        scfgmgr_set(name,val);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

int
apCfgWpsModeGet(int unit)
{
    int value;
    char name[60];
    char *p;
    sprintf(name,"wlan%d_wps",unit);
    p = nvram_safe_get(name);
    value = (strcmp(p, ENABLE) == 0)?1:0;
    return value;
}

int
apCfgWpsPinERGet(int unit)
{
    int value;
    char name[60];
    char *p;
    sprintf(name,"wlan%d_wps_pin_er",unit);
    p = nvram_safe_get(name);
    if(!strlen(p))
    	value=1;
    else
    	value = (strcmp(p, ENABLE) == 0)?1:0;
    return value;
}

A_STATUS
apCfgRadioChannelSet(int unit,int v)
{
	char name[60];
    char string[10];
    
    if(v<0 || v>14)
        return A_ERROR;
    
    if(v!=apCfgRadioChannelGet(unit))
    {
        sprintf(string,"%d",v); 
        sprintf(name,"wlan%d_channel",unit);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));   
        system("echo 1 > /tmp/chan_config");
        
#ifdef LINKSYS
        if(v >= 5)
            apCfgChannelOffsetSet(unit, "-1");
        else
            apCfgChannelOffsetSet(unit, "1");    
#endif        
    }
    return A_OK;
}

int
apCfgRadioChannelGet(int unit)
{
    char name[60];
    char *p, value[60];
    sprintf(name,"wlan%d_channel",unit);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}

A_STATUS
apCfgChannelOffsetSet(int unit,char *p)
{
    char name[60];
    
    if(strcmp("1", p)!=0 && strcmp("-1", p)!=0)
        return A_ERROR;
        
    if(strcmp(p, apCfgChannelOffsetGet(unit)))
    {
        sprintf(name,"wlan%d_channel_offset",unit);
        scfgmgr_set(name,p);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1)); 
        system("echo 1 > /tmp/apl_reload_mod");
    }
    return A_OK;
}

char *
apCfgChannelOffsetGet(int unit)
{
    char name[60], *p;
    static char value[32];
    sprintf(name,"wlan%d_channel_offset",unit);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}

A_STATUS 
scApCfgAmpduSet(int unit,int v)
{
    char name[60];
    char *string;
    if(v!=scApCfgAmpduGet(unit))
    {
        string = (v==1)?ENABLE:DISABLE;
        sprintf(name,"wlan%d_ampdu",unit);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1)); 
    }
    return A_OK;
}

int  
scApCfgAmpduGet(int unit)
{
	char name[60];
	char *string;
	int a;
    sprintf(name,"wlan%d_ampdu",unit);
    string=nvram_safe_get(name);
    a = (strcmp(string, ENABLE) == 0)? 1:0;
    NVRAM_FREE(string);
    return a;
}

A_STATUS 
scApCfgAmsduSet(int unit,int v)
{
    char name[60];
    char *string;
    if(v!=scApCfgAmsduGet(unit))
    {
        string = (v==1)?ENABLE:DISABLE;
        sprintf(name,"wlan%d_amsdu",unit);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1)); 
    }
    return A_OK;
}

int  
scApCfgAmsduGet(int unit)
{
	char name[60];
    int value;
    char *p;
    
    sprintf(name,"wlan%d_amsdu",unit);
    p = nvram_safe_get(name);
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}

A_STATUS 
scApCfgShortGISet(int unit,int v)
{
    char name[60];
    char *string;

    if(v!=scApCfgShortGIGet(unit))
    {
        switch(v)
        {
            case SHORTGI_SHORT:
                string="SHORTGI_SHORT";
        	    break;
            case SHORTGI_LONG: 
                string="SHORTGI_LONG";
        	    break;
            case SHORTGI_AUTO: 
            default:
                string="SHORTGI_AUTO";
        	    break;
        }
        sprintf(name,"wlan%d_short_GI",unit);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
        system("echo 1 > /tmp/apl_reload_mod"); 
    }
    return A_OK;
}

int  
scApCfgShortGIGet(int unit)
{
	char name[60];
	int a;
	char *string;
    sprintf(name,"wlan%d_short_GI",unit);
    string=nvram_safe_get(name);
    if(strcmp(string, "SHORTGI_LONG") == 0)
        a=SHORTGI_LONG;
    else if(strcmp(string, "SHORTGI_SHORT") == 0)
        a=SHORTGI_SHORT;
    else
        a=SHORTGI_AUTO;
    NVRAM_FREE(string);
    return a;
}

A_STATUS 
scApCfg80211dEnabledSet(int unit,int v)
{
    char name[60];
    char *string;
    if(v!=scApCfg80211dEnabledGet(unit))
    {
        string = (v==1)?ENABLE:DISABLE;
        sprintf(name,"wlan%d_80211d",unit);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

int  
scApCfg80211dEnabledGet(int unit)
{
	char name[60];
	int a = 0;
	char *string;
    sprintf(name,"wlan%d_80211d",unit);
    string=nvram_safe_get(name);
    
    if (strcmp(string, ENABLE) == 0) 
    {
        a=1;
    } 
    else if (strcmp(string, DISABLE)==0)
    {
        a=0;
    }
    NVRAM_FREE(string);
    return a;
}
A_STATUS
apCfgPrioritySet(int unit,int bss,int v)
{
    char name[60];
    char string[5];
    if(v != apCfgPriorityGet(unit,bss))
    {
        sprintf(string,"%d",v);
        sprintf(name,"wlan%d_ssid%d_pri",unit,bss);
        scfgmgr_set(name,string);        
        
       apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}
int
apCfgPriorityGet(int unit,int bss)
{
    char name[60];
    char *string, value[60];
    sprintf(name,"wlan%d_ssid%d_pri",unit,bss);
    string=nvram_safe_get(name);
    strcpy(value, string);
    NVRAM_FREE(string);
    
    return atoi(value);
}

A_STATUS
apCfgWmeSet(int unit,int bss, int v)
{
    char name[60];
    char *string;
    
    if(v!=apCfgWmeGet(unit, bss))
    {
        string = (v==1)?ENABLE:DISABLE;
        sprintf(name,"wlan%d_ssid%d_wmm",unit, bss);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1)); 
    }
    return A_OK;
}

int
apCfgWmeGet(int unit, int bss)
{
    char name[60];
    char *string;
    int a = 0;
    sprintf(name,"wlan%d_ssid%d_wmm",unit, bss);
    string=nvram_safe_get(name);
    if (strcmp(string, ENABLE) == 0) 
    {
        a=1;
    } 
    if (strcmp(string, DISABLE)==0)
    {
        a=0;
    }
    NVRAM_FREE(string);
    return a;
}
A_STATUS
apCfgWmmpsSet(int unit,int bss, int v)
{
    char name[60];
    char *string;
    
    if(v!=apCfgWmmpsGet(unit, bss))
    {
        string = (v==1)?ENABLE:DISABLE;
        sprintf(name,"wlan%d_ssid%d_wmmps",unit, bss);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1)); 
    }
    return A_OK;
}

int
apCfgWmmpsGet(int unit, int bss)
{
    char name[60];
    char *string;
    int a = 0;
    sprintf(name,"wlan%d_ssid%d_wmmps",unit, bss);
    string=nvram_safe_get(name);
    if (strcmp(string, ENABLE) == 0) 
    {
        a=1;
    } 
    if (strcmp(string, DISABLE)==0)
    {
        a=0;
    }
    NVRAM_FREE(string);
    return a;
}
A_STATUS
apCfgNoAckSet(int unit,int bss, int v)
{
    char name[60];
    char *string;
    
    if(v!=apCfgNoAckGet(unit, bss))
    {
        string = (v==1)?ENABLE:DISABLE;
        sprintf(name,"wlan%d_ssid%d_no_ack",unit, bss);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

int
apCfgNoAckGet(int unit, int bss)
{
    char name[60];
    int a = 0;
    char *string;
    sprintf(name,"wlan%d_ssid%d_no_ack",unit, bss);
    string=nvram_safe_get(name);
    if (strcmp(string, ENABLE) == 0) 
    {
        a=1;
    } 
    else if (strcmp(string, DISABLE)==0)
    {
        a=0;
    }
    NVRAM_FREE(string);
    return a;
}
A_STATUS 
scApCfgIdleTimeoutIntervalSet(int unit,int v)
{
    char name[60];
    char string[10];
    
    if(v<SC_MIN_IDLETIMEOUT_INTERVAL || v >SC_MAX_IDLETIMEOUT_INTERVAL)
        return A_ERROR;
    
    if(v!=scApCfgIdleTimeoutIntervalGet(unit))
    {
        sprintf(string,"%d",v); 
        sprintf(name,"wlan%d_idle_timeout",unit);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}	

int 
scApCfgIdleTimeoutIntervalGet(int unit)
{
    char name[60];
    char *p, value[60];
    sprintf(name,"wlan%d_idle_timeout",unit);
        
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}
A_STATUS 
scApCfgDtimIntervalSet(int unit,int v)
{
    char name[60];
    char string[10];
    
    if(v<SC_MIN_DTIM_INTERVAL || v >SC_MAX_DTIM_INTERVAL)
        return A_ERROR;
    
    if(v!=scApCfgDtimIntervalGet(unit))
    {
        sprintf(string,"%d",v); 
        sprintf(name,"wlan%d_dtim",unit);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}	

int 
scApCfgDtimIntervalGet(int unit)
{
    char name[60];
    char *p, value[60];
    sprintf(name,"wlan%d_dtim",unit);
        
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}
A_STATUS
apCfgFragThresholdSet(int unit, int v)
{
    char name[60];
    char string[10];
    
    if(v<MIN_FRAG_THRESHOLD || v>MAX_FRAG_THRESHOLD)
        return A_ERROR;
        
    if(v!=apCfgFragThresholdGet(unit))
    {
        sprintf(string,"%d",v);
        sprintf(name,"wlan%d_frag",unit);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

int
apCfgFragThresholdGet(int unit)
{
    char name[60];
    char *p, value[60];
    sprintf(name,"wlan%d_frag",unit);
        
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}
A_STATUS
apCfgRtsThresholdSet(int unit,int v)
{
    char name[60];
    char string[10];
    
    if(v<MIN_RTS_THRESHOLD || v>MAX_RTS_THRESHOLD)
        return A_ERROR;
            
    if(v!=apCfgRtsThresholdGet(unit))
    {
        sprintf(string,"%d",v);
        sprintf(name,"wlan%d_rts",unit);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

int
apCfgRtsThresholdGet(int unit)
{
    char name[60];
    char *p, value[60];
    sprintf(name,"wlan%d_rts",unit);
        
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}
A_STATUS
apCfgShortPreambleSet(int unit,int v)
{
    char name[60];
    char *string;
    if(v!=apCfgShortPreambleGet(unit))
    {
        string = (v==1)?ENABLE:DISABLE;
        sprintf(name,"wlan%d_prb_type",unit);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

int
apCfgShortPreambleGet(int unit)
{
    char name[60];
    int a = 0;
    char *string;
    sprintf(name,"wlan%d_prb_type",unit);
    string=nvram_safe_get(name);
    if (strcmp(string, ENABLE) == 0) 
    {
        a=1;
    } 
    else if (strcmp(string, DISABLE)==0)
    {
        a=0;
    }
    NVRAM_FREE(string);
    return a;
}

A_STATUS
apCfgPowerSet(int unit,int v)
{
    char name[60];
    char string[10];
    if(v!=apCfgPowerGet(unit))
    {
        sprintf(string,"%d",v);
        sprintf(name,"wlan%d_transmit_power",unit);
  	    scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

int
apCfgPowerGet(int unit)
{
    char name[60];
    char *p, value[60];
    sprintf(name,"wlan%d_transmit_power",unit);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}
A_STATUS
apCfgAntennaSet(int unit,int v)
{
    char name[60];
    char string[10];
    sprintf(string,"%d",v);
    sprintf(name,"wlan%d_antenna",unit);
   	scfgmgr_set(name,string);
    return A_OK;
}

int
apCfgAntennaGet(int unit)
{
    char name[60];
    char *p, value[60];
    sprintf(name,"wlan%d_antenna",unit);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}
A_STATUS
apCfgRateSet(int unit,char *p)
{	
    char name[60];
    if(strcmp(p, apCfgRateGet(unit)))
    {
        sprintf(name,"wlan%d_data_rate",unit);
       	scfgmgr_set(name,p);
       	apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

char *
apCfgRateGet(int unit)
{
    char name[60], *p;
    static char value[32];
    sprintf(name,"wlan%d_data_rate",unit);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}
A_STATUS
apCfgBalanceModeSet(int unit, int v)
{
    char *string = NULL;
    char name[60];
    if(v != apCfgBalanceModeGet(unit))
    {
        string = (v==1)?ENABLE:DISABLE;
        sprintf(name,"wlan%d_balance",unit);
        scfgmgr_set(name,string);
        apl_set_flag(APL_BALANCE);
    }
    return A_OK;
}
int apCfgBalanceModeGet(int unit)
{
    char *string = NULL;
    char name[60];
    int value;
    sprintf(name,"wlan%d_balance",unit);
    string=nvram_safe_get(name);
    
    value = strcmp(ENABLE, string)?0:1;
    NVRAM_FREE(string);
    return value;
}
A_STATUS 
apCfgLoadBalanceSet(int unit, int vap, int v)
{
    char name[60];
    char string[10];
    
    if(v<MIN_BALANCE || v>MAX_BALANCE) return A_ERROR;
    if(v != apCfgLoadBalanceGet(unit, vap))
    {
        sprintf(name, "wlan%d_ssid%d_loadbalance", unit, vap);
        sprintf(string, "%d", v);
        scfgmgr_set(name,string);
        apl_set_flag(APL_BALANCE);
    }
    return A_OK;
}
int apCfgLoadBalanceGet(int unit, int vap)
{
    char name[60];
    char *p, value[60];
    sprintf(name, "wlan%d_ssid%d_loadbalance", unit, vap);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}
A_STATUS
apCfgInterVapForwardingSet(int unit, int v)
{
    char name[60];
    char *string;
    
    if(apCfgOpModeGet(unit) == CFG_OP_MODE_UR)
        v = 0;
        
    if(v!=apCfgInterVapForwardingGet(unit))
    {
        string = (v==1)?ENABLE:DISABLE;
        sprintf(name,"wlan%d_isolation",unit);
       	scfgmgr_set(name,string);
       	apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

int
apCfgInterVapForwardingGet(int unit)
{
    char name[60];
    char *string;
    int a = 0;
    sprintf(name,"wlan%d_isolation",unit);
    string=nvram_safe_get(name);
    if (strcmp(string, ENABLE) == 0) 
    {
        a=1;
    } 
    else if (strcmp(string, DISABLE)==0)
    {
        a=0;
    }
    NVRAM_FREE(string);
    return a;
}
A_STATUS
apCfgRemoteApMacAddrSet(int unit,char *pRemoteApMacAddr)
{
    char name[50];
    char tmpMac[18];
    apCfgRemoteApMacAddrGet(unit,tmpMac);
    if(strcmp(pRemoteApMacAddr, tmpMac))
    {
        sprintf(name,"wlan%d_remote_mac",unit);
        scfgmgr_set(name,pRemoteApMacAddr);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

A_STATUS
apCfgRemoteApMacAddrGet(int unit,char *pRemoteApMacAddr)
{
    char name[50], *p;
    sprintf(name,"wlan%d_remote_mac",unit);
    p = nvram_safe_get(name);
    strcpy(pRemoteApMacAddr, p);
    
    NVRAM_FREE(p);
    return A_OK;
}

/* MD@CPU_AP change for cfg   at 20071203 */
A_STATUS apCfgRemoteWbrMacAddrGet(int unit, int index, A_UCHAR *pMac)
{
    char name[50];
    char *pFullValue;
    char finalValue[8][18];
    sprintf(name,"wlan%d_remote_bridge_mac",unit);
 
    pFullValue = nvram_safe_get(name);
    memcpy(finalValue, pFullValue, 8*18);
  
    finalValue[index][17] = 0;
    strcpy(pMac, finalValue[index]);
    
    NVRAM_FREE(pFullValue);
    return A_OK;
}

A_STATUS apCfgRemoteWbrMacAddrSet(int unit, int index, A_UCHAR *pMac)
{
    char name[50];
    char *pFullValue;
    char finalValue[8][18];
    char valueToSet[8*18+1];
    sprintf(name,"wlan%d_remote_bridge_mac",unit);
    
    pFullValue = nvram_safe_get(name);
    memcpy(finalValue, pFullValue, 8*18);
    
    memcpy(finalValue[index], pMac, 17);
    finalValue[index][17] = ',';
    memcpy(valueToSet, finalValue, 8*18);
    valueToSet[8*18] = 0;
    
    if(strcmp(valueToSet, pFullValue))
    {    
        scfgmgr_set(name,valueToSet);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
    }
    NVRAM_FREE(pFullValue);
    return A_OK;
}
/* change end */

/* mike add for wls_basic */
A_STATUS
apCfgUcrRemoteApMacAddrSet(int unit,char *pRemoteApMacAddr)
{
    char name[50];
    char tmpMac[18];
    apCfgUcrRemoteApMacAddrGet(unit,tmpMac);
    if(strcmp(pRemoteApMacAddr, tmpMac))
    {
        sprintf(name,"wlan%d_remote_mac",unit);
        scfgmgr_set(name,pRemoteApMacAddr);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

A_STATUS
apCfgUcrRemoteApMacAddrGet(int unit, char *pRemoteApMacAddr)
{
    char name[50], *p;
    sprintf(name,"wlan%d_remote_mac",unit);
    p = nvram_safe_get(name);
    strcpy(pRemoteApMacAddr, p);

    NVRAM_FREE(p);
    return A_OK;
}

/* added end */
A_STATUS
apCfgAuthTypeSet(int unit, int bss,int v)
{
    char name[60];
    char *string;
    if(v!=apCfgAuthTypeGet(unit,bss))
    {
        /*UC/UR mode, then only the none and wep are valid for the vap 0*/
        if((apCfgOpModeGet(unit) == CFG_OP_MODE_UC || apCfgOpModeGet(unit) == CFG_OP_MODE_UR) 
            && bss == 0)
        {
            if(v > APCFG_AUTH_AUTO && v!=APCFG_AUTH_WPAPSK && v!=APCFG_AUTH_WPA2PSK && v!=APCFG_AUTH_WPA_AUTO_PSK){    
                return A_ERROR;
            }
        }
#ifdef CHECK_11N
        /* in 11n only mode, only none, wpa2, wpa2-mixed*/
        if(apCfgFreqSpecGet(unit) == MODE_SELECT_11N)
        {
            if(v!=APCFG_AUTH_NONE && v!=APCFG_AUTH_WPA2 && v!=APCFG_AUTH_WPA2PSK && v!=APCFG_AUTH_WPA_AUTO && v!=APCFG_AUTH_WPA_AUTO_PSK){
                return A_ERROR;
            }
        }
#endif        
        sprintf(name,"wlan%d_ssid%d_auth_mode",unit,bss);
        switch(v)
        {
            case APCFG_AUTH_NONE: 
                string="NONE";
        	    break;
            case APCFG_AUTH_OPEN_SYSTEM: 
                string="OPEN_SYSTEM";
        	    break;
          	case APCFG_AUTH_SHARED_KEY: 
          	    string="SHARED_KEY";
          		break;
          	case APCFG_AUTH_AUTO: 
          	    string="AUTH_AUTO";
          	    break;
          	case APCFG_AUTH_WPA: 
          	    string="AUTH_WPA";
          	    break;
            case APCFG_AUTH_WPAPSK: 
                string="AUTH_WPAPSK";
        	    break;
            case APCFG_AUTH_WPA2: 
                string="AUTH_WPA2";
        	    break;
            case APCFG_AUTH_WPA2PSK: 
                string="AUTH_WPA2PSK";
        	    break;
            case APCFG_AUTH_WPA_AUTO: 
                string="WPA_AUTO";
        	    break;	
            case APCFG_AUTH_WPA_AUTO_PSK: 
                string="WPA_AUTO_PSK";
        	    break;	
            case APCFG_AUTH_DOT1X: 
                string="AUTH_DOT1X";
        	    break;						
            default:
                string="NONE";
        	    break;
        }
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
        apCfgWscConfiguredSet(1);
    }
    return A_OK;
}


int
apCfgAuthTypeGet(int unit, int bss)
{
    char name[60];
    char *p;
    int a = APCFG_AUTH_NONE;
    sprintf(name,"wlan%d_ssid%d_auth_mode",unit,bss);
    p=nvram_safe_get(name);
    if (strcmp(p, "NONE") == 0) 
        a=APCFG_AUTH_NONE;
    else if (strcmp(p, "OPEN_SYSTEM") == 0) 
        a=APCFG_AUTH_OPEN_SYSTEM;
    else if (strcmp(p, "SHARED_KEY") == 0)
        a=APCFG_AUTH_SHARED_KEY;
    else if (strcmp(p, "AUTH_AUTO") == 0)
        a=APCFG_AUTH_AUTO;
    else if (strcmp(p, "AUTH_WPA") == 0)
        a=APCFG_AUTH_WPA;
    else if (strcmp(p, "AUTH_WPAPSK") == 0)
        a=APCFG_AUTH_WPAPSK;
    else if (strcmp(p, "AUTH_WPA2") == 0)
        a=APCFG_AUTH_WPA2;
    else if (strcmp(p, "AUTH_WPA2PSK") == 0)
        a=APCFG_AUTH_WPA2PSK;
    else if (strcmp(p, "WPA_AUTO") == 0)
        a=APCFG_AUTH_WPA_AUTO;
    else if (strcmp(p, "WPA_AUTO_PSK") == 0)
        a=APCFG_AUTH_WPA_AUTO_PSK;
    else if (strcmp(p, "AUTH_DOT1X") == 0)
        a=APCFG_AUTH_DOT1X;
    else 
        a=APCFG_AUTH_NONE;
    NVRAM_FREE(p);
    return a;
}

A_STATUS
apCfgWPACipherSet(int unit, int bss,int v)
{
    char name[60];
    char *string;
    if(v!=apCfgWPACipherGet(unit,bss))
    {
        sprintf(name,"wlan%d_ssid%d_wpa_cipher",unit,bss);
        switch(v)
        {
            case WPA_CIPHER_TKIP: 
                string="TKIP";
        	    break;
            case WPA_CIPHER_AES: 
                string="AES";
        	    break;
          	case WPA_CIPHER_AUTO: 
            default:
          	    string="AUTO";
          	    break;					
        }
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
        apCfgWscConfiguredSet(1);
    }
    return A_OK;
}

int
apCfgWPACipherGet(int unit, int bss)
{
    char name[60];
    char *p;
    int a;
    sprintf(name,"wlan%d_ssid%d_wpa_cipher",unit,bss);
    p=nvram_safe_get(name);
    a = WPA_CIPHER_AUTO;
    if (strcmp(p, "TKIP") == 0) 
    {
        a=WPA_CIPHER_TKIP;
    } 
    else if (strcmp(p, "AES") == 0) 
    {
        a=WPA_CIPHER_AES;
    } 
    else if (strcmp(p, "AUTO") == 0) 
    {
        a=WPA_CIPHER_AUTO;
    }
    else 
    {
        //printf("apcfg: unknwon mode %s\n", p);
        a=WPA_CIPHER_AUTO;
    }
    NVRAM_FREE(p);
    return a;
}

A_STATUS
apCfgDefKeySet(int unit, int bss, int v)
{
    /* caller should validate key */
    if (((v >= CFG_MIN_SHARED_KEY) &&
         (v <= CFG_MAX_SHARED_KEY)) ||
         (v == INVALID_SHARED_KEY))
    {
        char name[60];
        char string[10];
        if(v!=apCfgDefKeyGet(unit,bss))
        {
            sprintf(string,"%d",v);
            sprintf(name,"wlan%d_ssid%d_default_key",unit,bss);
   	        scfgmgr_set(name,string);
   	        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
   	        apCfgWscConfiguredSet(1);
   	    }
        return A_OK;
    }
    return A_EINVAL;
}

int
apCfgDefKeyGet(int unit, int bss)
{
    char name[60];
    char *p, value[60];
    sprintf(name,"wlan%d_ssid%d_default_key",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}
A_STATUS
apCfgKey1ValSet(int unit, int bss, char *p)
{
    char name[60];
    int len = strlen(p);
    
    if(len!=0 && len!=5 && len!=10 && len!=13 && len!=26 && len!=16 && len!=32)
        return A_ERROR;
        
    sprintf(name,"wlan%d_ssid%d_key1",unit,bss);
    scfgmgr_set(name,p);
    apCfgWscConfiguredSet(1);
    return A_OK;
}

char *
apCfgKey1ValGet(int unit, int bss)
{
    char name[60], *p;
    static char value[32];
    sprintf(name,"wlan%d_ssid%d_key1",unit,bss);
       
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}
A_STATUS
apCfgKey2ValSet(int unit, int bss, char *p)
{
     char name[60];
     int len = strlen(p);
    if(len!=0 && len!=5 && len!=10 && len!=13 && len!=26 && len!=16 && len!=32)
        return A_ERROR;
     sprintf(name,"wlan%d_ssid%d_key2",unit,bss);
   	 scfgmgr_set(name,p);
   	 apCfgWscConfiguredSet(1);
     return A_OK;
}

char *
apCfgKey2ValGet(int unit, int bss)
{
    char name[60], *p;
    static char value[32];
    sprintf(name,"wlan%d_ssid%d_key2",unit,bss);
       
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}
A_STATUS
apCfgKey3ValSet(int unit, int bss, char *p)
{
     char name[60];
    int len = strlen(p);
    if(len!=0 && len!=5 && len!=10 && len!=13 && len!=26 && len!=16 && len!=32)
        return A_ERROR;
     sprintf(name,"wlan%d_ssid%d_key3",unit,bss);
   	 scfgmgr_set(name,p);
   	 apCfgWscConfiguredSet(1);
     return A_OK;
}

char *
apCfgKey3ValGet(int unit, int bss)
{
    char name[60], *p;
    static char value[32];
    sprintf(name,"wlan%d_ssid%d_key3",unit,bss);
       
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}
A_STATUS
apCfgKey4ValSet(int unit, int bss, char *p)
{
		char name[60];
    int len = strlen(p);
    if(len!=0 && len!=5 && len!=10 && len!=13 && len!=26 && len!=16 && len!=32)
        return A_ERROR;
  	sprintf(name,"wlan%d_ssid%d_key4",unit,bss);
   	scfgmgr_set(name,p);
   	apCfgWscConfiguredSet(1);
    return A_OK;
}

char *
apCfgKey4ValGet(int unit, int bss)
{    
    char name[60], *p;
    static char value[32];
    sprintf(name,"wlan%d_ssid%d_key4",unit,bss);
       
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}

A_STATUS
apCfgKeyValSet(int unit, int bss, int keyId, char *p)
{
    char name[60];
    int len = strlen(p);
    if(len!=0 && len!=5 && len!=10 && len!=13 && len!=26 && len!=16 && len!=32)
        return A_ERROR;
    
    if(strcmp(p, apCfgKeyValGet(unit,bss,keyId)))
    {
        int i;
        
        for(i=0;i<WLAN_MAX_VAP;i++)
        {
      	    sprintf(name,"wlan%d_ssid%d_key%d",unit,i,keyId);
       	    scfgmgr_set(name,p);
        	apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
        }
    }
    return A_OK;
}

char *
apCfgKeyValGet(int unit, int bss, int keyId)
{
    char name[60], *p;
    static char value[32];
    sprintf(name,"wlan%d_ssid%d_key%d", unit, bss, keyId);
       
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}

A_STATUS
apCfgPassphraseSet(int unit, int bss, char *p)
{
    char name[60];
    
    if(strlen(p)<0 || strlen(p)>CFG_MAX_PASSPHRASE )
        return A_ERROR;
        
    if(strcmp(p, apCfgPassphraseGet(unit,bss)))
    {
        sprintf(name,"wlan%d_ssid%d_passphrase",unit,bss);
   	    scfgmgr_set(name,p);
   	    apCfgWscConfiguredSet(1);
   	    apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

char *
apCfgPassphraseGet(int unit, int bss)
{
    char name[60], *p;
    static char value[32];
    sprintf(name,"wlan%d_ssid%d_passphrase",unit,bss);
       
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}
A_STATUS
apCfgRadiusServerSet(int unit, int bss, char *p)
{
    char name[60];
    int vap;
    if(strlen(p)<0 || strlen(p)>CFG_MAX_RADIUSNAME )
        return A_ERROR;
        
    if(strcmp(p, apCfgRadiusServerGet(unit,bss)))
    {
        for(vap=0; vap<WLAN_MAX_VAP; vap++) {
            sprintf(name,"wlan%d_ssid%d_radius_server",unit,vap);
       	    scfgmgr_set(name,p);
   	    }
   	    apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
   	    apCfgWscConfiguredSet(1);
    }
    return A_OK;
}


char *
apCfgRadiusServerGet(int unit, int bss)
{
    char name[60], *p;
    static char value[32];
    sprintf(name,"wlan%d_ssid%d_radius_server",unit,bss);
   
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}

A_STATUS
apCfgBackupRadiusServerSet(int unit, int bss, char *p)
{
    char name[60];
    int vap;
    if(strlen(p)<0 || strlen(p)>CFG_MAX_RADIUSNAME )
        return A_ERROR;
    if(strcmp(p, apCfgBackupRadiusServerGet(unit,bss)))
    {
        for(vap=0; vap<WLAN_MAX_VAP; vap++) {
            sprintf(name,"wlan%d_ssid%d_backup_radius_server",unit,vap);
       	    scfgmgr_set(name,p);
   	    }
   	    apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
   	    apCfgWscConfiguredSet(1);
   	}
    return A_OK;
}

char *
apCfgBackupRadiusServerGet(int unit, int bss)
{
    char name[60], *p;
    static char value[32];
    sprintf(name,"wlan%d_ssid%d_backup_radius_server",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}

int apCfgRadiusPortGet(int unit, int bss)
{
    char name[60];
    char *p, value[60];
	sprintf(name,"wlan%d_ssid%d_radius_port",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}
A_STATUS apCfgRadiusPortSet(int unit,int bss,int v)
{
    char name[60];
    char string[10];
    int vap;
    if(v<1 || v>65534)
        return A_ERROR;
        
    if(v!=apCfgRadiusPortGet(unit,bss))
    {
        sprintf(string,"%d",v);
        for(vap=0; vap<WLAN_MAX_VAP; vap++) {
            sprintf(name,"wlan%d_ssid%d_radius_port",unit,vap);
            scfgmgr_set(name,string);
        }
        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
        apCfgWscConfiguredSet(1);
    }
    return A_OK;
}

int apCfgBackupRadiusPortGet(int unit, int bss)
{
    char name[60];
    char *p, value[60];
	sprintf(name,"wlan%d_ssid%d_backup_radius_port",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}
A_STATUS apCfgBackupRadiusPortSet(int unit,int bss,int v)
{
    char name[60];
    char string[10];
    int vap;
    if(v<1 || v>65534)
        return A_ERROR;
    if(v!=apCfgBackupRadiusPortGet(unit,bss))
    {
        sprintf(string,"%d",v);
        for(vap=0; vap<WLAN_MAX_VAP; vap++) {
            sprintf(name,"wlan%d_ssid%d_backup_radius_port",unit,vap);
            scfgmgr_set(name,string);
        }
        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
        apCfgWscConfiguredSet(1);
    }
    return A_OK;
}

A_STATUS apCfgRadiusSecretSet(int unit, int bss, char *p)
{
    char name[60];
    int vap;
    if(strlen(p)<0 || strlen(p)>CFG_MAX_SECRETLEN)
        return A_ERROR;
        
    if(strcmp(p,apCfgRadiusSecretGet(unit,bss) ))
    {
        for(vap=0; vap<WLAN_MAX_VAP; vap++) {
            sprintf(name,"wlan%d_ssid%d_radius_key",unit,vap);
            scfgmgr_set(name,p);
        }
        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
        apCfgWscConfiguredSet(1);
    }
    return A_OK;
}
char *
apCfgRadiusSecretGet(int unit, int bss)
{
    char name[60], *p;
    static char value[32];
    sprintf(name,"wlan%d_ssid%d_radius_key",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}

A_STATUS apCfgBackupRadiusSecretSet(int unit, int bss, char *p)
{
    char name[60];
    int vap;
    if(strlen(p)<0 || strlen(p)>CFG_MAX_SECRETLEN)
        return A_ERROR;    
    if(strcmp(p, apCfgBackupRadiusSecretGet(unit,bss)))
    {
        for(vap=0; vap<WLAN_MAX_VAP; vap++) {
            sprintf(name,"wlan%d_ssid%d_backup_radius_key",unit,vap);
            scfgmgr_set(name,p);
        }
        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
        apCfgWscConfiguredSet(1);
    }
    return A_OK;
}
char *
apCfgBackupRadiusSecretGet(int unit, int bss)
{
    char name[60], *p;
    static char value[32];
    sprintf(name,"wlan%d_ssid%d_backup_radius_key",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}

int
apCfgGroupKeyUpdateIntervalGet(int unit, int bss)
{
    char name[60];
    char *p, value[60];
    sprintf(name,"wlan%d_ssid%d_rekey_interval",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}
A_STATUS
apCfgGroupKeyUpdateIntervalSet(int unit, int bss, int v)
{
    char name[60];
    char string[10];
    if(v<MIN_GROUP_KEY_UPDATE_INTERVAL || v>MAX_GROUP_KEY_UPDATE_INTERVAL)
        return A_ERROR;
    if(v!=apCfgGroupKeyUpdateIntervalGet(unit,bss))
    {
        sprintf(string,"%d",v);
        sprintf(name,"wlan%d_ssid%d_rekey_interval",unit,bss);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
        apCfgWscConfiguredSet(1);
    }
    return A_OK;
}
int 
apCfgDot1xKeyModeGet(int unit, int bss)
{
    char name[60];
    char *p, value[60];
    sprintf(name,"wlan%d_ssid%d_dot1x_key_mode",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}

A_STATUS apCfgDot1xKeyModeSet(int unit, int bss, int v)
{
    char name[60];
    char val='0'+v;
    
    if(v<1 || v>3)
        return A_ERROR;
            
    if(v!=apCfgDot1xKeyModeGet(unit,bss))
    {
        sprintf(name,"wlan%d_ssid%d_dot1x_key_mode",unit,bss);
        scfgmgr_set(name,&val);
        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
        apCfgWscConfiguredSet(1);
    }
    return A_OK;
}
	
int 
apCfgDot1xKeyLenGet(int unit, int bss)
{
    char name[60];
    char *p, value[60];
    sprintf(name,"wlan%d_ssid%d_dot1x_key_length",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}

A_STATUS
apCfgDot1xKeyLenSet(int unit, int bss, int v)
{
    char name[60];
    char string[10];
    if(v!=5 && v!=13 && v != 16)
        return A_ERROR;
    if(v!=apCfgDot1xKeyLenGet(unit, bss))
    {
        sprintf(string,"%d",v); 
        sprintf(name,"wlan%d_ssid%d_dot1x_key_length",unit,bss);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
        apCfgWscConfiguredSet(1);
    }
    return A_OK;
}	
A_STATUS 
scApCfgGroupKeyUpdateEnabledSet(int unit, int bss, int v)
{
    char name[60];
    char *string;
    
    if(v!=scApCfgGroupKeyUpdateEnabledGet(unit,bss))
    {
        string = (v==1)?ENABLE:DISABLE;
        sprintf(name,"wlan%d_ssid%d_rekey_mode",unit,bss);
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
        apCfgWscConfiguredSet(1);
    }
    return A_OK;
}
int  
scApCfgGroupKeyUpdateEnabledGet(int unit, int bss)
{
    char name[60];
    int a = 0;
    char *string;
    
    sprintf(name,"wlan%d_ssid%d_rekey_mode",unit,bss);
    string=nvram_safe_get(name);
    if (strcmp(string, ENABLE) == 0) 
    {
        a=1;
    } 
    else if (strcmp(string, DISABLE)==0)
    {
        a=0;
    }
    NVRAM_FREE(string);
    return a;
}
A_STATUS 
scApCfgGroupKeyUpdateTerminatedSet(int unit, int bss, int v)
{
    char name[60];
    char val='0'+v;
    if(v!=scApCfgGroupKeyUpdateTerminatedGet(unit,bss))
    {
        sprintf(name,"wlan%d_ssid%d_rekey_terminated",unit,bss);
        scfgmgr_set(name,&val);
        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
        apCfgWscConfiguredSet(1);
    }
    return A_OK;
}	
int  
scApCfgGroupKeyUpdateTerminatedGet(int unit, int bss)
{
    char name[60];
    char *p, value[60];
    sprintf(name,"wlan%d_ssid%d_rekey_terminated",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}	
/* MD@CPU_AP del it for cfg   at 20071203 */
/*
A_STATUS 
scApCfgAcctUpdateIntervalSet(int unit, int bss, int v)
{
    char name[60];
    char string[10];
    
    if(v<MIN_ACCT_UPDATEINTERVAL || v>MAX_ACCT_UPDATEINTERVAL )
        return A_ERROR;
        
    if(v!=scApCfgAcctUpdateIntervalGet(unit,bss))
    {
        sprintf(string,"%d",v); 
        sprintf(name,"wlan%d%d_acctUpdateInterval",unit,bss);
        scfgmgr_set(name,string);
        apl_set_flag(WLAN00_ACCUPDINT+bss);
        apCfgWscConfiguredSet(1);
    }
    return A_OK;
}	
int 
scApCfgAcctUpdateIntervalGet(int unit, int bss)
{
    char name[60];
    char *p, value[60];
    sprintf(name,"wlan%d%d_acctUpdateInterval",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}	
*/
/* del it end */

A_STATUS
scApCfgAcctServerSet(int unit, int bss, char *p)
{
     char name[60];
     int vap;
    if(strlen(p)<1 || strlen(p)>CFG_MAX_SECRETLEN)
        return A_ERROR;     
     if(strcmp(p, scApCfgAcctServerGet(unit,bss)))
     {
        for(vap=0; vap<WLAN_MAX_VAP; vap++) {
            sprintf(name,"wlan%d_ssid%d_acct_server",unit,bss);
       	    scfgmgr_set(name,p);
       	}
   	    apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
   	    apCfgWscConfiguredSet(1);
     }
     return A_OK;
}

char *
scApCfgAcctServerGet(int unit, int bss)
{
    char name[60], *p;
    static char value[32];
    sprintf(name,"wlan%d_ssid%d_acct_server",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}

A_STATUS
scApCfgBackupAcctServerSet(int unit, int bss, char *p)
{
    char name[60];
    int vap;
    if(strlen(p)<1 || strlen(p)>CFG_MAX_SECRETLEN)
        return A_ERROR;     
     if(strcmp(p, scApCfgBackupAcctServerGet(unit,bss)))
     {
        for(vap=0; vap<WLAN_MAX_VAP; vap++) {
            sprintf(name,"wlan%d_ssid%d_backup_acct_server",unit,bss);
       	    scfgmgr_set(name,p);
       	}
   	    apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
   	    apCfgWscConfiguredSet(1);
     }
     return A_OK;
}

char *
scApCfgBackupAcctServerGet(int unit, int bss)
{
    char name[60], *p;
    static char value[32];
    sprintf(name,"wlan%d_ssid%d_backup_acct_server",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    return value;
}

A_STATUS
scApCfgAcctSecretSet(int unit, int bss, char *p)
{
     char name[60];
     int vap;
    if(strlen(p)<0 || strlen(p)>CFG_MAX_SECRETLEN)
        return A_ERROR;     
     if(strcmp(p, scApCfgAcctSecretGet(unit,bss)))
     {
        for(vap=0; vap<WLAN_MAX_VAP; vap++) {
            sprintf(name,"wlan%d_ssid%d_acct_key",unit,vap);
       	    scfgmgr_set(name,p);
       	}
   	    apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
   	    apCfgWscConfiguredSet(1);
    }
    return A_OK;
}

char *
scApCfgAcctSecretGet(int unit, int bss)
{
     char name[60], *p;
     static char value[128];
     sprintf(name,"wlan%d_ssid%d_acct_key",unit,bss);
     
     p = nvram_safe_get(name);
     strcpy(value, p);
     NVRAM_FREE(p);
     return value;
}

A_STATUS
scApCfgBackupAcctSecretSet(int unit, int bss, char *p)
{
     char name[60];
     int vap;
    if(strlen(p)<0 || strlen(p)>CFG_MAX_SECRETLEN)
        return A_ERROR;     
     if(strcmp(p, scApCfgBackupAcctSecretGet(unit,bss)))
     {
        for(vap=0; vap<WLAN_MAX_VAP; vap++) {
            sprintf(name,"wlan%d_ssid%d_backup_acct_key",unit,vap);
       	    scfgmgr_set(name,p);
       	}
   	    apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
   	    apCfgWscConfiguredSet(1);
     }
     return A_OK;
}

char *
scApCfgBackupAcctSecretGet(int unit, int bss)
{
     char name[60], *p;
     static char value[128];
     sprintf(name,"wlan%d_ssid%d_backup_acct_key",unit,bss);
     
     p = nvram_safe_get(name);
     strcpy(value, p);
     NVRAM_FREE(p);
     return value;
}

A_STATUS 
scApCfgAcctPortSet(int unit, int bss, int v)
{
    char name[60];
    char string[10];
    int vap;
    if(v<1 || v>65534)
        return A_ERROR;    
    if(v!=scApCfgAcctPortGet(unit, bss))
    {
        sprintf(string,"%d",v);
        for(vap=0; vap<WLAN_MAX_VAP; vap++) {
            sprintf(name,"wlan%d_ssid%d_acct_port",unit,vap);
       	    scfgmgr_set(name,string);
       	}
        apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
        apCfgWscConfiguredSet(1);
    }
    return A_OK;
}	
int  
scApCfgAcctPortGet(int unit, int bss)
{
    char name[60], value[60], *p;
    
    sprintf(name,"wlan%d_ssid%d_acct_port",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}	

A_STATUS 
scApCfgBackupAcctPortSet(int unit, int bss, int v)
{
    char name[60];
    char string[10];
    int vap;
    if(v<1 || v>65534)
        return A_ERROR;    
    if(v!=scApCfgBackupAcctPortGet(unit, bss))
    {
        sprintf(string,"%d",v);
        for(vap=0; vap<WLAN_MAX_VAP; vap++) {
            sprintf(name,"wlan%d_ssid%d_backup_acct_port",unit,vap);
           	scfgmgr_set(name,string);
        }
       	apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
       	apCfgWscConfiguredSet(1);
    }
    return A_OK;
}	
int  
scApCfgBackupAcctPortGet(int unit, int bss)
{
    char name[60], value[60], *p;
    
    sprintf(name,"wlan%d_ssid%d_backup_acct_port",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}	
/* MD@CPU_AP del it for cfg   at 20071203 */
/*
A_STATUS 
scApCfgAcctEnabledSet(int unit, int bss, int v)
{
	  char name[60];
    char *string;
    if(v==1)
    {
	    string=ENABLE;
    }
    else 
    {
       	string=DISABLE;
    }
    sprintf(name,"wlan%d%d_Acct",unit,bss);
   	scfgmgr_set(name,string);
    return A_OK;
}
int  
scApCfgAcctEnabledGet(int unit, int bss)
{
    char name[60];
    int a=0;
    char *string;
    sprintf(name,"wlan%d%d_Acct",unit,bss);
    string=nvram_safe_get(name);
    if (strcmp(string, ENABLE) == 0) 
    {
        a=1;
    } 
    else if (strcmp(string, DISABLE)==0)
    {
        a=0;
    }
    NVRAM_FREE(string);
    return a;
   
}
A_STATUS 
scApCfgAcctUpdateEnabledSet(int unit, int bss, int v)
{
    char name[60];
    char *string;
    
    if(v!=scApCfgAcctUpdateEnabledGet(unit,bss))
    {
        string=(v==1)?ENABLE:DISABLE;
        sprintf(name,"wlan%d%d_AcctUpdate",unit,bss);
   	    scfgmgr_set(name,string);
   	    apl_set_flag(WLAN00_ACCUPDATE+bss);
    }
    return A_OK;
}	
int  
scApCfgAcctUpdateEnabledGet(int unit, int bss)
{
	char name[60];
	int a = 0;
	char *string;
    sprintf(name,"wlan%d%d_AcctUpdate",unit,bss);
    string=nvram_safe_get(name);
    if (strcmp(string, ENABLE) == 0) 
    {
        a=1;
    } 
    else if (strcmp(string, DISABLE)==0)
    {
        a=0;
    }
    NVRAM_FREE(string);
    return a;
}	
*/
/* del it end */
A_STATUS
apCfgKeyBitLenSet(int unit, int bss, int v)
{
    char name[60];
    char string[10];
    
    if(v!=40 && v!=104 && v != 128)
        return A_ERROR;
        
    if(v!=apCfgKeyBitLenGet(unit,bss))
    {
        int i;
        
        for(i=0;i<WLAN_MAX_VAP;i++)
        {
            sprintf(string,"%d",v);
            sprintf(name,"wlan%d_ssid%d_key_length",unit,i);
            scfgmgr_set(name,string);
            apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
        }
        apCfgWscConfiguredSet(1);
    }
    return A_OK;
}	

int 
apCfgKeyBitLenGet(int unit, int bss)
{
    char name[60];
    char *p, value[60];
    sprintf(name,"wlan%d_ssid%d_key_length",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}
int
apCfgKeyEntryMethodGet(int unit, int bss)
{
    char name[60];
    char *p, value[60];
    sprintf(name,"wlan%d_ssid%d_key_method",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}

A_STATUS
apCfgKeyEntryMethodSet(int unit, int bss, int v)
{
    char name[60];
    char string[10];
    if(v!=apCfgKeyEntryMethodGet(unit,bss))
    {
        int i;
        
        for(i=0;i<WLAN_MAX_VAP;i++)
        {
            sprintf(string,"%d",v);
            sprintf(name,"wlan%d_ssid%d_key_method",unit,i);
       	    scfgmgr_set(name,string);
       	    apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
       	}
       	apCfgWscConfiguredSet(1);
    }
    return A_OK;
}
int
apCfgVlanPvidGet(int unit, int bss)
{
    char name[60];
    char *p, value[32];
    sprintf(name,"wlan%d_ssid%d_vid",unit,bss);
    
    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);
    
    return atoi(value);
}

A_STATUS
apCfgVlanPvidSet(int unit, int bss, int v)
{
    char name[60];
    char string[10];
    if(v < VLAN_TAG_MIN || v > VLAN_TAG_MAX)
        return A_ERROR;     
    
    if(v != apCfgVlanPvidGet(unit, bss))
    {
        sprintf(string,"%d",v);
        sprintf(name,"wlan%d_ssid%d_vid",unit,bss);
       	scfgmgr_set(name,string);
        apl_set_flag(APL_VLAN);
    }
    return A_OK;
}
A_STATUS
apCfgIntraVapForwardingSet(int unit, int bss,int v)
{
    char name[60];
    char *string;
    
    if(v!=apCfgIntraVapForwardingGet(unit,bss))
    {
        string=(v==1)?ENABLE:DISABLE;
        sprintf(name,"wlan%d_ssid%d_isolation",unit,bss);
       	scfgmgr_set(name,string);
       	apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

int
apCfgIntraVapForwardingGet(int unit,int bss)
{
    char name[60];
    char *string;
    int a = 0;
    sprintf(name,"wlan%d_ssid%d_isolation",unit,bss);
    string=nvram_safe_get(name);
    
    if (strcmp(string, ENABLE) == 0) 
    {
        a=1;
    } 
    else if (strcmp(string, DISABLE)==0)
    {
        a=0;
    }
    NVRAM_FREE(string);
    return a;
}
A_STATUS
apCfgAclModeSet(int unit, int bss, int v)
{
    char name[60];
    char *string;
    
    if(v!=apCfgAclModeGet(unit, bss))
    {
        switch(v)
        {
            case APCFG_ACL_DISABLED:
            default:
                string="Disable";
        	    break;
            case APCFG_ACL_LOCAL: 
                string="Local";
        	    break;
            case APCFG_ACL_RADIUS: 
                string="Radius";
        	    break;
        }
        sprintf(name,"wlan%d_ssid%d_access_control",unit,bss);
       	scfgmgr_set(name,string);
       	apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
    }
    return A_OK;
}

int
apCfgAclModeGet(int unit, int bss)
{
    char name[60];
    int a = APCFG_ACL_DISABLED;
    char *string;
    sprintf(name,"wlan%d_ssid%d_access_control",unit,bss);
    string=nvram_safe_get(name);
    
    if (strcmp(string, "Disable") == 0) 
    {
        a=APCFG_ACL_DISABLED;
    } 
    else if (strcmp(string, "Local") == 0) 
    {
        a=APCFG_ACL_LOCAL;
    } 
    else if (strcmp(string, "Radius") == 0) 
    {
        a=APCFG_ACL_RADIUS;
    } 
    NVRAM_FREE(string);
    return a;
}


/*
 * access connection control type
 * 0: Allow
 * 1: Prevent
 */
A_STATUS
apCfgAclTypeSet(int unit, int bss, int v)
{
	char name[60];
	char *string;

	if(v != apCfgAclTypeGet(unit, bss))
	{
		sprintf(name,"wlan%d_ssid%d_acl_type",unit, bss);
		string = (v == APCFG_ACL_ALLOW)?ENABLE:DISABLE;
		
		scfgmgr_set(name,string);
		apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
	}
	
    return A_OK;
}

int
apCfgAclTypeGet(int unit, int bss)
{
    char name[60];
    char *string;
    int value;
    
    sprintf(name,"wlan%d_ssid%d_acl_type",unit, bss);
    string=nvram_safe_get(name);
    
    value = strcmp(string, ENABLE)?APCFG_ACL_PREVENT:APCFG_ACL_ALLOW;
    NVRAM_FREE(string);
    
    return value;
}

/*
 *  ACL List format in configure file should be:
 *      mac,name,1/0 .... .....
 *      separate every unit with ';', so the name can't contain space
 */
A_STATUS
apCfgAclAdd(A_UINT32 unit, int bss, char *pMacAddr, char *pName, int used)
{
    int count=0;
    char name[60];
    char *pFullValue;
    char *pUnit;
    int unitOffset=0;
    char unitValue[sizeof(CFG_ACL_ENTRY)];
    int len;
    char finalValue[CFG_MAX_ACL*sizeof(CFG_ACL_ENTRY)];
    
    if(memcmp(pMacAddr, "00:00:00:00:00:00", 17) == 0)
        return A_ERROR;
    memset(finalValue,0,CFG_MAX_ACL*sizeof(CFG_ACL_ENTRY));
    sprintf(name,"wlan%d_ssid%d_acl_list",unit, bss);
    pFullValue = nvram_safe_get(name);
    pUnit = pFullValue;
    
    scToUppers(pMacAddr);
        
    while(strlen(pUnit) > 0)
    {
        char *pUnitHeader = pUnit;
        char *pUnitEnd = strchr(pUnit,';');
        
        if(memcmp(pUnit, pMacAddr, 17) == 0)
        {
            len = sprintf(unitValue, "%s,%s,%d", pMacAddr, pName, used);
            unitValue[len] = 0;
            
            unitOffset = pUnitHeader-pFullValue;
            memcpy(finalValue, pFullValue, unitOffset);
            memcpy(finalValue+unitOffset, unitValue, len);
            strcpy(finalValue+unitOffset+len,pUnitEnd);
            
            scfgmgr_set(name, finalValue);
            apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
            NVRAM_FREE(pFullValue);
            return A_OK;
        }
        pUnit = pUnitEnd+1;    
        count++;   
    }

#ifdef LINKSYS    
    if(count >= CFG_MAX_ACL_LINKSYS)
#else
    if(count >= CFG_MAX_ACL)
#endif
    {
        NVRAM_FREE(pFullValue);
        return A_NO_MEMORY;
    }

    len = sprintf(unitValue, "%s,%s,%d", pMacAddr, pName, used);
    unitValue[len] = 0;
    if(strlen(pFullValue)>0)
        sprintf(finalValue,"%s%s;", pFullValue, unitValue);
    else
        sprintf(finalValue,"%s;", unitValue);
    scfgmgr_set(name, finalValue);
    apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));

    NVRAM_FREE(pFullValue);
    return A_OK;
}

A_STATUS
apCfgAclDel(A_UINT32 unit, int bss, char *pMac)
{
    char name[60];
    char *pFullValue;
    char *pUnit;
    int unitOffset=0;
    char finalValue[CFG_MAX_ACL*sizeof(CFG_ACL_ENTRY)];

    memset(finalValue,0,CFG_MAX_ACL*sizeof(CFG_ACL_ENTRY));
    sprintf(name,"wlan%d_ssid%d_acl_list",unit, bss);
    pFullValue = nvram_safe_get(name);
    pUnit = pFullValue;
    
    scToUppers(pMac);
    while(strlen(pUnit) > 0)
    {
        char *pUnitHeader = pUnit;
        char *pUnitEnd = strchr(pUnit,';');
        
        if(pUnitEnd == NULL)
            break;
                
        if(memcmp(pUnit, pMac, 17) == 0)
        {
            unitOffset = pUnitHeader-pFullValue;
            memcpy(finalValue, pFullValue, unitOffset);
            strcpy(finalValue+unitOffset,pUnitEnd+1);
            
            scfgmgr_set(name, finalValue);
            apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
            return A_OK;
        }
        pUnit = pUnitEnd+1; 
    }
    
    NVRAM_FREE(pFullValue); 
    return A_ENOENT;
}

void 
apCfgAclClear(A_UINT32 unit, int bss)
{
    char name[60];
    char *finalValue="";
    
    sprintf(name,"wlan%d_ssid%d_acl_list",unit, bss);
	scfgmgr_set(name, finalValue);
	apl_set_flag(APL_WLAN0_VAP0+bss+unit*(WLAN_MAX_VAP+1));
 
    return;   	
}


A_BOOL scAclBufGet(A_UINT32 unit, int bss, struct scAclBuf_s **pScAcl)
{
    struct scAclBuf_s *pScAclCurr = NULL;
    char name[60];
    char *pFullValue;
    char *pUnit;
    
    if (unit < 0 || unit > 1)	return FALSE;
	if (scAclBufUsed[unit][bss])	return FALSE;
	scAclBufUsed[unit][bss] = TRUE;
	(*pScAcl) = NULL;
	
	sprintf(name,"wlan%d_ssid%d_acl_list",unit,bss);
    pFullValue = nvram_safe_get(name);
    pUnit = pFullValue;
    
    while(strlen(pUnit) > 0)
    {
        char *pSep;   
        if((*pScAcl) == NULL)
        {
			(*pScAcl) = &(scAclBuf[unit][bss]);
			(*pScAcl)->next = NULL;
			pScAclCurr = (*pScAcl);
		}
		else
		{
			struct scAclBuf_s *pScAclTmp;
			pScAclTmp = (struct scAclBuf_s *)malloc(sizeof(struct scAclBuf_s));
			if(pScAclTmp == NULL)
			{
				scAclBufFree(unit, bss, *pScAcl);
				return FALSE;
			}	
			pScAclTmp->next  = NULL;
			pScAclCurr->next = pScAclTmp;
			pScAclCurr = pScAclTmp;
		}	      
		memset(pScAclCurr->mac, 0, 18);
		memcpy(pScAclCurr->mac, pUnit, 17);
		pUnit+=18;
		pSep = strchr(pUnit,',');
		memset(pScAclCurr->name, 0, 16);
		memcpy(pScAclCurr->name,pUnit,pSep-pUnit);
		pUnit=pSep+1;
		pScAclCurr->used = ((*pUnit) == '1')?1:0;      
        pUnit+=2;
    } 
    
    NVRAM_FREE(pFullValue);    
	return TRUE;
}	

void	
scAclBufFree(A_UINT32 unit, int bss, struct scAclBuf_s *pScAcl)
{
	struct scAclBuf_s *pScAclTmp;
	
	if(pScAcl){
		pScAcl = pScAcl->next;
		while(pScAcl){
			pScAclTmp = pScAcl;
			pScAcl = pScAcl->next;
			NVRAM_FREE(pScAclTmp);
		}
	}	
	scAclBufUsed[unit][bss] = FALSE;
}

A_STATUS
apCfgFreqSpecSet(int unit, int v)
{
    char name[60];
    char *string;   
    
    if(v!=apCfgFreqSpecGet(unit))
    {
        sprintf(name,"wlan%d_mode",unit);
        switch(v)
        {
            case MODE_SELECT_AUTO:
                string="MODE_SELECT_AUTO";
        	    break;
            case MODE_SELECT_11A: 
                string="MODE_SELECT_11A";
        	    break;
            case MODE_SELECT_11B: 
                string="MODE_SELECT_11B";
        	    break;
          	case MODE_SELECT_11G: 
          	    string="MODE_SELECT_11G";
          	    break;
          	case MODE_SELECT_FH: 
          	    string="MODE_SELECT_FH";
          	    break;
          	case MODE_SELECT_TURBOA: 
          	    string="MODE_SELECT_TURBOA";
          	    break;
            case MODE_SELECT_TURBOG: 
                string="MODE_SELECT_TURBOG";
        	    break;
            case MODE_SELECT_11NA: 
                string="MODE_SELECT_11NA";
        	    break;
            case MODE_SELECT_11NG: 
                string="MODE_SELECT_11NG";
        	    break;
        	case MODE_SELECT_11N: 
                string="MODE_SELECT_11N";
        	    break;
        	/* add cfg but not support now */
        	case MODE_SELECT_11BG:
        	    string="MODE_SELECT_11BG";
        	    break;
        	case MODE_SELECT_11BGN:
        	    string="MODE_SELECT_11BGN";
        	    break;
        	/* add end */
            default:
                string="MODE_SELECT_AUTO";
        	    break;
        }
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
        
        system("echo 1 > /tmp/apl_reload_mod");
        
#ifdef CHECK_11N        
        if(v == MODE_SELECT_11N)
        {
            int vap;
            int authType;
            for(vap = 0; vap < WLAN_MAX_VAP ; vap++)
            {
                authType = apCfgAuthTypeGet(unit, vap);
                if(authType!=APCFG_AUTH_NONE && authType!=APCFG_AUTH_WPA2 && 
                    authType!=APCFG_AUTH_WPA2PSK && authType!=APCFG_AUTH_WPA_AUTO && authType!=APCFG_AUTH_WPA_AUTO_PSK){
                    apCfgAuthTypeSet(unit, vap, APCFG_AUTH_NONE);
                }
            }
        }
#endif
    }
    return A_OK;
}

int
apCfgFreqSpecGet(int unit)
{
    char name[60];
    char *p;
    int a;
    
    sprintf(name,"wlan%d_mode",unit);
    p=nvram_safe_get(name);
    a = MODE_SELECT_AUTO;
    
    if (strcmp(p, "MODE_SELECT_AUTO") == 0) 
    {
        a=MODE_SELECT_AUTO;
    } 
    else if (strcmp(p, "MODE_SELECT_11A") == 0) 
    {
        a=MODE_SELECT_11A;
    } 
    else if (strcmp(p, "MODE_SELECT_11B") == 0) 
    {
        a=MODE_SELECT_11B;
    } 
    else if (strcmp(p, "MODE_SELECT_11G") == 0) 
    {
        a=MODE_SELECT_11G;
    } 
    else if (strcmp(p, "MODE_SELECT_FH") == 0) 
    {
        a=MODE_SELECT_FH;
    } 
    else if (strcmp(p, "MODE_SELECT_TURBOA") == 0) 
    {
        a=MODE_SELECT_TURBOA;
    } 
    else if (strcmp(p, "MODE_SELECT_TURBOG") == 0) 
    {
        a=MODE_SELECT_TURBOG;
    } 
    else if (strcmp(p, "MODE_SELECT_11NA") == 0) 
    {
        a=MODE_SELECT_11NA;
    } 
    else if (strcmp(p, "MODE_SELECT_11NG") == 0) 
    {
        a=MODE_SELECT_11NG;
    }
    else if (strcmp(p, "MODE_SELECT_11N") == 0) 
    {
        a=MODE_SELECT_11N;
    }
    else if (strcmp(p, "MODE_SELECT_11BG") == 0) 
    {
        a=MODE_SELECT_11BG;
    }
    else if (strcmp(p, "MODE_SELECT_11BGN") == 0) 
    {
        a=MODE_SELECT_11BGN;
    }
    else 
    {
        printf("apcfg: unknwon mode %s\n", p);
        return -1;
    }
    NVRAM_FREE(p);
    return a;
}

A_STATUS
apCfgChannelWidthModeSet(int unit, int v)
{
    char name[60];
    char *string;
/*  Raul Commit out     
    if(unit == RADIO_24G){
        if(apCfgCountryCodeGet()==CTRY_JAPAN)
            v = CWM_MODE_20M;
    }
*/    
    if(apCfgOpModeGet(unit) == CFG_OP_MODE_ROGAP || 
        apCfgOpModeGet(unit) == CFG_OP_MODE_UC || 
        apCfgOpModeGet(unit) == CFG_OP_MODE_UR)
        v = CWM_MODE_20M;
          
    if(v!=apCfgChannelWidthModeGet(unit))
    {
        sprintf(name,"wlan%d_channel_width",unit);
        switch(v)
        {
            case CWM_MODE_20M:
                string="CWM_MODE_20M";
        	    break;
            case CWM_MODE_40M: 
                string="CWM_MODE_40M";
        	    break;
            case CWM_MODE_20M_40M: 
            default:
                string="CWM_MODE_20M_40M";
        	    break;
        }
        scfgmgr_set(name,string);
        apl_set_flag(APL_WLAN0+unit*(WLAN_MAX_VAP+1));
        system("echo 1 > /tmp/apl_reload_mod");
    }
    return A_OK;
}

int
apCfgChannelWidthModeGet(int unit)
{
    char name[60];
    char *p;
    int a;
    
    sprintf(name,"wlan%d_channel_width",unit);
    p=nvram_safe_get(name);
    a=CWM_MODE_20M_40M;
    if (strcmp(p, "CWM_MODE_20M") == 0) 
        a=CWM_MODE_20M;
    else if (strcmp(p, "CWM_MODE_20M_40M") == 0) 
        a=CWM_MODE_20M_40M;
    else if(strcmp(p, "CWM_MODE_40M") == 0)
        a=CWM_MODE_40M;
    else
        a=CWM_MODE_20M_40M;
        
    NVRAM_FREE(p);
    return a;
}

A_STATUS
apCfgVlanModeSet(int v)
{
    char *string;
    if(v!=apCfgVlanModeGet())
    {
        string = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("vlan_mode",string);
        apl_set_flag(APL_VLAN);
    }
    return A_OK;
}

int 
apCfgVlanModeGet()
{   
    int value;
    char *p;
    
    p = nvram_safe_get("vlan_mode");
    value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    
    return value;
}

/*
* Set the flag while following item changed
* 1) ssid   2) security issue
*/
A_STATUS
apCfgWscConfiguredSet(int v)
{
    if(v != 1 && v!=0)
        return A_ERROR;
    
    if(*nvram_safe_get("restore_defaults")=='1')
        return A_ERROR;
            
    if(v != apCfgWscConfiguredGet())
    {
        scfgmgr_set("wifi_wsc_configured",v==0?"0":"1");
    }
    
    return A_OK;
}

int 
apCfgWscConfiguredGet(void)
{
    int value;
    char *p;
    
    p = nvram_safe_get("wifi_wsc_configured");
    value =  atoi(p);
    NVRAM_FREE(p);
    
    return value;
}
char* apCfgwdsVlanListGet(void)
{
    static char wdsvlist[21];
    char *p;

    memset(wdsvlist, 0, 21);
   p = nvram_safe_get("wds_vlan_list");
    strcpy(wdsvlist, p);

   NVRAM_FREE(p);
      return wdsvlist;
}

A_STATUS
apCfgwdsVlanListSet(char* list)
{
    if(scValidwdsVlanList(list))
   {
        scfgmgr_set("wds_vlan_list", list);
        apl_set_flag(APL_VLAN); 
        return A_OK;
    }
    return A_ERROR;
}
A_BOOL apCfgMultiEnhanceSet(int v)
{
    if(v!=apCfgMultiEnhanceGet())
    {
        scfgmgr_set("multicast_enbance",v==1?"1":"0");
        apl_set_flag(APL_WLAN0);
    }
    return A_OK;
}

A_BOOL apCfgMultiEnhanceGet(void)
{
    char *p;
    int value =0;

    p = nvram_get("multicast_enbance");
		if(p)
		{
		value = atoi(p);
    NVRAM_FREE(p);
			}
    return value;
	}
A_STATUS apCfgEthDataRateSet(int v)
{
    if(v!=apCfgEthDataRateGet())
    {
        scfgmgr_set("eth_data_rate",v==1000?"1000Mbps":
                (v==100?"100Mbps":
                 (v==10?"10Mbps":"auto")));

			 scfgmgr_set("lan_portspeed_define",v==1000?"2":
                (v==100?"1":
                 (v==10?"0":"0")));
        apl_set_flag(APL_ETH_DATARATE);
    }

    return A_OK;
}
int apCfgEthDataRateGet(void)
{
    char *p;
    int value;

    p = nvram_get("eth_data_rate");
    if(strcmp(p,"auto")==0){
        value = 0;
    }else if(strcmp(p,"10Mbps")==0){
        value = 10;
    }else if(strcmp(p,"100Mbps")==0){
        value = 100;
    }else if(strcmp(p,"1000Mbps")==0){
        value = 1000;
    }else{
        value = 0;
    }

    NVRAM_FREE(p);

    return value;
}

A_STATUS
apCfgCertSet(char *p)
{
    if(strlen(p) < 0 || strlen(p) > MAX_CERT_CONTENT)
        return A_ERROR;

    if(strcmp(p,apCfgCertGet()))
    {
        scfgmgr_set("cert_content",p);
        apl_set_flag(APL_HTTPD);
    }
    return A_OK;
}

char *
apCfgCertGet(void)
{
    char name[60], *p;
    static char value[MAX_CERT_CONTENT+1];
    sprintf(name,"cert_content");

    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);

    return value;
}

A_STATUS
apCfgCertTimeSet(char *p)
{
    if(strlen(p) < 0 || strlen(p) > MAX_CERT_TIME)
        return A_ERROR;

    if(strcmp(p,apCfgCertTimeGet()))
    {
        scfgmgr_set("cert_time",p);
    }
    return A_OK;
}

char *
apCfgCertTimeGet(void)
{
    char name[60], *p;
    static char value[MAX_CERT_TIME+1];
    sprintf(name,"cert_time");

    p = nvram_safe_get(name);
    strcpy(value, p);
    NVRAM_FREE(p);

    return value;
}

void apcfg_submit(void)
{
    nvram_commit();
}

int apCfgAutoRebootModeGet()
{
    char *p;
    int value;

    p = nvram_safe_get("auto_reboot");
    if(!strlen(p))
    	value=1;
    else
    	value = (strcmp(p,ENABLE) == 0)?1:0;
    NVRAM_FREE(p);
    return value;
}
A_STATUS apCfgAutoRebootModeSet(A_BOOL v)
{
    char *val;
    if(v!= apCfgAutoRebootModeGet())
    {     
        val = (v==1)?ENABLE:DISABLE;
        scfgmgr_set("auto_reboot",val);
        apl_set_flag(APL_AUTOREBOOT);
    }
    return A_OK;
}

A_STATUS 
scApCfgAutoRebootIntervalSet(int v)
{
    char value[6];
    
    if(v<CFG_MIN_REBOOT_TIME || v >CFG_MAX_REBOOT_TIME)
        return A_ERROR;
    
    if(v!=apCfgAutoRebootIntervalGet())
    {
        memset(value, 0, 6);
        sprintf(value,"%d",v);
        scfgmgr_set("auto_reboot_interval",value);
        apl_set_flag(APL_AUTOREBOOT);
    }
    return A_OK;
}	

int 
apCfgAutoRebootIntervalGet()
{
    char *p;
    int value;

    p = nvram_safe_get("auto_reboot_interval");//Hour
    value = atoi(p);
    NVRAM_FREE(p);
    return value;

}

A_STATUS 
scApCfgAutoRebootTimeSet(int hour, int min)
{
	int old_hour=0, old_min=0;
	char value[12]={0};
	
	if(hour<0 || hour>23 || min<0 || min>59)
		return A_ERROR;
	apCfgAutoRebootTimeGet(&old_hour, &old_min);
    if(hour!=old_hour || min!=old_min)
    {
        sprintf(value,"%02d:%02d",hour, min);
        scfgmgr_set("auto_reboot_time",value);
        apl_set_flag(APL_AUTOREBOOT);
    }
    return A_OK;
}	

A_STATUS 
apCfgAutoRebootTimeGet(int *hour, int *min)
{
    char *p;
	
	*hour=0;
	*min=0;
    p = nvram_safe_get("auto_reboot_time");//HH:MM
	if(strlen(p))
		sscanf(p,"%02d:%02d",hour, min);
	else{
		hour=2;
		min=0;	
	}
    NVRAM_FREE(p);
    
    return A_OK;
}

