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
#ifndef _SC_APPLY_H_
#define _SC_APPLY_H_
#define _BONJOUR_
enum
{
    APL_ROOT=0,      
    APL_REBOOT,    
    APL_SYSNAME,
    APL_LOGIN,     
    APL_TELNET,    
    APL_SSH,       
    APL_HTTPD,     
    APL_VLAN,      
    APL_STP,       
    APL_LLTD,      
    APL_IPv4,      
    APL_IPv6,      
    APL_ETHSUP, 
    APL_BALANCE,  
    APL_WLAN0,  
    APL_WLAN0_VAP0,
    APL_WLAN0_VAP1,
    APL_WLAN0_VAP2,
    APL_WLAN0_VAP3,
    APL_WLAN1, 
    APL_WLAN1_VAP0,
    APL_WLAN1_VAP1,
    APL_WLAN1_VAP2,
    APL_WLAN1_VAP3,
    APL_NTP,       
    APL_SNMP,      
    APL_LOG,       
    APL_FTP,
    APL_ETH_DATARATE,
    APL_FORCE100M,
    APL_HTTPREDIRECT,
    APL_ROGUEAP,     
#ifdef _BONJOUR_
    APL_BONJOUR,
#endif
    APL_AUTOREBOOT
};

void apl_set_flag(unsigned int id);
void apl_clear_flag(void);
int apl_status_get(int status);
int apcfgReboot(void);
int apcfgisIphander(void);
int apcfgHaveChanged(void);

#endif
