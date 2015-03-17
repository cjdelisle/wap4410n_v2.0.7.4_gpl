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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "apply.h"
#include "apcfg.h"
#include "wifiuti.h"

typedef struct APPLY_TABLE_S
{
    unsigned int    id;
    unsigned char   flag;
        #define NO_CHANGE   0x00
        #define CHANGED     0x01
    int             (*handler)(int, int);
    int             unit;
    int             vap;
    unsigned int    parent;
    unsigned int    child;
    unsigned int    rbrother;
}APPLY_TABLE;

#define NO_ACC      -1
#define NON_VAP     -1
#define NON_UNIT    -1

#ifndef DESTROY_ATH_IF
#define DESTROY_ATH_IF      "/tmp/delif"
#endif
#ifndef HOSTAP_CFG
#define HOSTAP_CFG	        "/var/hostap.cfg"
#endif

#define APL_CLOSED          "/tmp/apl_closed"
#define APL_FLAG            "/tmp/apl_flag"
#define APL_RUNNING         "/tmp/apl_running"
#define APL_PARSING         "/tmp/apl_parsing"

static int reboot_handler(int unit, int vap);
static int sysname_handler(int unit, int vap); 
static int login_handler(int unit, int vap); 
static int telnet_handler(int unit, int vap);
static int ssh_handler(int unit, int vap);   
static int httpd_handler(int unit, int vap); 
static int vlan_handler(int unit, int vap);  
static int stp_handler(int unit, int vap);   
static int lltd_handler(int unit, int vap);  
static int ipv4_handler(int unit, int vap);  
static int ipv6_handler(int unit, int vap);  
static int ethsup_handler(int unit, int vap);
static int wlan_handler(int unit, int vap);  
static int vap_handler(int unit, int vap);
static int ntp_handler(int unit, int vap);   
static int snmp_handler(int unit, int vap);
static int log_handler(int unit, int vap);
static int ftp_handler(int unit, int vap);
static int balance_handler(int unit, int vap);
static int redirect_handler(int unit, int vap);
static int rogueap_handler(int unit, int vap);
static int autoreboot_handler(int unit, int vap);
#ifdef _BONJOUR_
static int bonjour_handler(int unit, int vap);
#endif
static int eth_datarate_handler(int unit, int vap);
static int force100m_handler(int unit, int vap);
static int (*APPLYRUN)(const char *format, ...) = NULL;
/****************************************************************************************************************
Apply Tree:
APL_ROOT---APL_REBOOT---APL_SYSNAME
                        APL_LOGIN---APL_TELNET
                                    APL_SSH
                                    APL_HTTPD
                        APL_VLAN ---APL_STP
                                    APL_LLTD
                                    APL_IPv4
                                    APL_IPv6
                                    APL_ETHSUP
                                    APL_BALANCE
                                    APL_WLAN0 ---APL_WLAN0_VAP0
                                                 APL_WLAN0_VAP1
                                                 APL_WLAN0_VAP2
                                                 APL_WLAN0_VAP3
                                                
                                    APL_WLAN1 ---APL_WLAN1_VAP0
                                                 APL_WLAN1_VAP1
                                                 APL_WLAN1_VAP2
                                                 APL_WLAN1_VAP3
                                                 
                        APL_NTP
                        APL_SNMP
                        APL_LOG
                        APL_FTP
                        APL_ETH_DATARATE
                        APL_FORCE100M
                        APL_HTTPREDIRECT
                        APL_ROGUEAP
                        APL_BONJOUR
                         APL_AUTOREBOOT                       
*****************************************************************************************************************/

static APPLY_TABLE applyTable[] = 
{
/*  ID           flag  handler             unit vap parent              child               rbrother          Comment     */
/*=========================================================================================================================*/
{APL_ROOT,          0, NULL,                -1, -1, NO_ACC,             APL_REBOOT,         NO_ACC},
{APL_REBOOT,        0, reboot_handler,      -1, -1, APL_ROOT,           APL_SYSNAME,        NO_ACC},
{APL_SYSNAME,       0, sysname_handler,     -1, -1, APL_REBOOT,         NO_ACC,             APL_LOGIN},
{APL_LOGIN,         0, login_handler,       -1, -1, APL_REBOOT,         APL_TELNET,         APL_VLAN},
{APL_TELNET,        0, telnet_handler,      -1, -1, APL_LOGIN,          NO_ACC,             APL_SSH},
{APL_SSH,           0, ssh_handler,         -1, -1, APL_LOGIN,          NO_ACC,             APL_HTTPD},
{APL_HTTPD,         0, httpd_handler,       -1, -1, APL_LOGIN,          NO_ACC,             NO_ACC},
{APL_VLAN,          0, vlan_handler,        -1, -1, APL_REBOOT,         APL_STP,            APL_NTP},
{APL_STP,           0, stp_handler,         -1, -1, APL_VLAN,           NO_ACC,             APL_LLTD},
{APL_LLTD,          0, lltd_handler,        -1, -1, APL_VLAN,           NO_ACC,             APL_IPv4},
{APL_IPv4,          0, ipv4_handler,        -1, -1, APL_VLAN,           NO_ACC,             APL_IPv6},
{APL_IPv6,          0, ipv6_handler,        -1, -1, APL_VLAN,           NO_ACC,             APL_ETHSUP},
{APL_ETHSUP,        0, ethsup_handler,      -1, -1, APL_VLAN,           NO_ACC,             APL_BALANCE},
{APL_BALANCE,       0, balance_handler,      0, -1, APL_VLAN,           NO_ACC,             APL_WLAN0},
{APL_WLAN0,         0, wlan_handler,         0, -1, APL_VLAN,           APL_WLAN0_VAP0,     APL_WLAN1},
{APL_WLAN0_VAP0,    0, vap_handler,          0,  0, APL_WLAN0,          NO_ACC,             APL_WLAN0_VAP1},
{APL_WLAN0_VAP1,    0, vap_handler,          0,  1, APL_WLAN0,          NO_ACC,             APL_WLAN0_VAP2},
{APL_WLAN0_VAP2,    0, vap_handler,          0,  2, APL_WLAN0,          NO_ACC,             APL_WLAN0_VAP3},
{APL_WLAN0_VAP3,    0, vap_handler,          0,  3, APL_WLAN0,          NO_ACC,             NO_ACC},
{APL_WLAN1,         0, wlan_handler,         1, -1, APL_VLAN,           APL_WLAN1_VAP0,     NO_ACC},
{APL_WLAN1_VAP0,    0, vap_handler,          1,  0, APL_WLAN1,          NO_ACC,             APL_WLAN1_VAP1},
{APL_WLAN1_VAP1,    0, vap_handler,          1,  1, APL_WLAN1,          NO_ACC,             APL_WLAN1_VAP2},
{APL_WLAN1_VAP2,    0, vap_handler,          1,  2, APL_WLAN1,          NO_ACC,             APL_WLAN1_VAP3},
{APL_WLAN1_VAP3,    0, vap_handler,          1,  3, APL_WLAN1,          NO_ACC,             NO_ACC},
{APL_NTP,           0, ntp_handler,         -1, -1, APL_REBOOT,         NO_ACC,             APL_SNMP},
{APL_SNMP,          0, snmp_handler,        -1, -1, APL_REBOOT,         NO_ACC,             APL_LOG},
{APL_LOG,           0, log_handler,         -1, -1, APL_REBOOT,         NO_ACC,             APL_FTP},
{APL_FTP,           0, ftp_handler,         -1, -1, APL_REBOOT,         NO_ACC,             APL_ETH_DATARATE},
{APL_ETH_DATARATE,  0, eth_datarate_handler,-1, -1, APL_REBOOT,         NO_ACC,             APL_FORCE100M},
{APL_FORCE100M,     0, force100m_handler,   -1, -1, APL_REBOOT,         NO_ACC,             APL_HTTPREDIRECT},
{APL_HTTPREDIRECT,  0, redirect_handler,    -1, -1, APL_REBOOT,         NO_ACC,             APL_ROGUEAP},
#ifdef _BONJOUR_
{APL_ROGUEAP,  		0, rogueap_handler,     -1, -1, APL_REBOOT,         NO_ACC,             APL_BONJOUR},
{APL_BONJOUR,		0, bonjour_handler,		-1,	-1,	APL_REBOOT,			NO_ACC,	APL_AUTOREBOOT},
#else
{APL_ROGUEAP,  		0, rogueap_handler,     -1, -1, APL_REBOOT,         NO_ACC,             APL_AUTOREBOOT},
#endif
{APL_AUTOREBOOT,   0, autoreboot_handler,          -1, -1, APL_REBOOT,         NO_ACC,                 NO_ACC}
};


static char *idString[] = 
{   
    "APL_ROOT",      
    "APL_REBOOT",    
    "APL_SYSNAME",  
    "APL_LOGIN",     
    "APL_TELNET",    
    "APL_SSH",       
    "APL_HTTPD",     
    "APL_VLAN",      
    "APL_STP",       
    "APL_LLTD",      
    "APL_IPv4",      
    "APL_IPv6",      
    "APL_ETHSUP", 
    "APL_BALANCE",   
    "APL_WLAN0",
    "APL_WLAN0_VAP0",
    "APL_WLAN0_VAP1",
    "APL_WLAN0_VAP2",
    "APL_WLAN0_VAP3",
    "APL_WLAN1", 
    "APL_WLAN1_VAP0",
    "APL_WLAN1_VAP1",
    "APL_WLAN1_VAP2",
    "APL_WLAN1_VAP3",
    "APL_NTP",       
    "APL_SNMP",      
    "APL_LOG",       
    "APL_FTP",
    "APL_ETH_DATARATE",
    "APL_FORCE100M",
    "APL_HTTPREDIRECT",
    "APL_ROGUEAP",      
#ifdef _BONJOUR_
    "APL_BONJOUR"
#endif
};


#define APL_DEBUG

#ifdef APL_DEBUG

int applyLog(const char *format, ...)
{
    va_list args;
    FILE *fp;

    fp = fopen("/var/log/apply_dbg", "a+");

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
#endif


/*****************************************************************************
 *  handlers
 *****************************************************************************/
static int reboot_handler(int unit, int vap)
{
#ifdef APL_DEBUG    
    applyLog("<%s,%d>: \n",__FUNCTION__,__LINE__);
#endif
    
    APPLYRUN("reboot");
    
    return 0;
}
static int sysname_handler(int unit, int vap)
{
/*
 *  restart syslogd
 */
    //APPLYRUN("rc ip restart"); // Cisco don't like it, badly
    APPLYRUN("rc syslogd restart");
    return 0;
}
static int login_handler(int unit, int vap)
{
/*
 *  restart username/password
 *  restart httpd
 *  restart telnet
 *  restart ssh
 */
#ifdef APL_DEBUG    
    applyLog("<%s,%d>: \n",__FUNCTION__,__LINE__);
#endif
    httpd_handler(unit, vap);
    telnet_handler(unit, vap);
    ssh_handler(unit, vap);
    return 0;  
}

static int telnet_handler(int unit, int vap)
{
/*
*   restart telnet
*/
    APPLYRUN("rc telnetd restart");
    return 0;
}

static int ssh_handler(int unit, int vap)
{
/*
*   restart ssh
*/
    APPLYRUN("rc sshd restart");    
    return 0;    
} 

static int httpd_handler(int unit, int vap)
{
/*
*   restart httpd
*/
    APPLYRUN("rc httpd restart");
    return 0;    
} 

static int vlan_handler(int unit, int vap)
{
/*
*   restart bridge/lan
*   restart wireless
*   restart stp
*   restart ipv4
*   restart ipv6
*   restart lltd
*   restart ethsup
*/

    if (apCfgipv6modeGet() == 1) {
	    system("/bin/sleep 3 && /sbin/reboot&");
	    return 0;
    }

    unit = 0;
    wlanStop(NULL, unit, 0);
    APPLYRUN("rc ip stop");
    APPLYRUN("rc ipv6 stop");
    
    APPLYRUN("rc bridge restart");
    APPLYRUN("rc lan restart");
    wlan_handler(unit,-1);
    stp_handler(-1,-1);
    ipv4_handler(-1,-1);
    ipv6_handler(-1,-1);
    lltd_handler(-1,-1);
    ethsup_handler(-1,-1);

    return 0;    
} 
 
static int stp_handler(int unit, int vap)
{
/*    
 *  rc stp restart
 */
#ifdef APL_DEBUG    
    applyLog("<%s,%d>: \n",__FUNCTION__,__LINE__);
#endif    
    APPLYRUN("rc stp restart");
    return 0;
} 
  
static int lltd_handler(int unit, int vap)
{
/*    
 *  rc lltd restart
 */
#ifdef APL_DEBUG    
    applyLog("<%s,%d>: \n",__FUNCTION__,__LINE__);
#endif 
    APPLYRUN("rc lld2 restart"); 
    return 0;
} 
 
static int ipv4_handler(int unit, int vap)
{
/*    
 *  rc ip restart
 *  rc wscupnp restart
 */
#ifdef APL_DEBUG    
    applyLog("<%s,%d>: \n",__FUNCTION__,__LINE__);
#endif    
    APPLYRUN("rc ip restart");
    APPLYRUN("rc wins restart");
    APPLYRUN("rc wscupnp restart&");
    APPLYRUN("rc httpredirect restart");
    return 0;
} 

static int ipv6_handler(int unit, int vap)
{
/*    
 *  rc ipv6 restart
 */
#ifdef APL_DEBUG    
    applyLog("<%s,%d>: \n",__FUNCTION__,__LINE__);
#endif
    APPLYRUN("rc ipv6 restart");
    httpd_handler(-1, -1);
    return 0;
} 

static int ethsup_handler(int unit, int vap)
{
/*    
 *  rc ethsup restart
 */
#ifdef APL_DEBUG    
    applyLog("<%s,%d>: \n",__FUNCTION__,__LINE__);
#endif    
    APPLYRUN("rc lanDot1xSupp restart");
    return 0;
} 

static int wlan_handler(int unit, int vap)
{
/*
*   restart wlan
*/
    if(unit == -1 || unit >= WLAN_MAX_DEV)
    {
        return 1;
    }
    if(access("/tmp/apl_reload_mod",F_OK)==0)
    {
        wlanStop(NULL, unit, 1);
        unlink("/tmp/apl_reload_mod");
    }else
        wlanStop(NULL, unit, 0);    
    wlanStart(NULL, unit);
    
    return 0;    
} 
static int balance_handler(int unit, int vap)
{
/*
*   Restart load_balance
*/
    APPLYRUN("rc balance restart");
    return 0;
}
#ifdef _BONJOUR_
static int bonjour_handler(int unit, int vap)
{
/*
*   Restart bonjour mDNSResponder
*/
    APPLYRUN("rc mdns restart");
    return 0;
}
#endif
static int redirect_handler(int unit, int vap)
{
/*
*   Restart http redirect
*/
    APPLYRUN("rc httpredirect restart");
    return 0;
}
static int rogueap_handler(int unit, int vap)
{
/*
*   Restart rogueAP
*/
    APPLYRUN("rc rogueap restart");
    return 0;
}
static int autoreboot_handler(int unit, int vap)
{
/*
*   Restart AutoReboot
*/
    APPLYRUN("rc autoreboot restart");
    return 0;
}

static int vap_handler(int unit, int vap)
{
    char scriptfile[64];
    char vapStr[6];
    
    /*valid check*/
    if(unit == -1 || unit >= WLAN_MAX_DEV ||
        vap == -1 || vap >= WLAN_MAX_VAP){
        return 1;
    }
    
    /*
    *   Creat and init script file
    */
    sprintf(scriptfile, "/tmp/%s_%d%d", __FUNCTION__, unit, vap);
    
    if(access(scriptfile,F_OK)==0)
        return 0;
    
#ifdef APL_DEBUG    
    applyLog("<%s,%d>: \n",__FUNCTION__,__LINE__);
#endif    
    
    ATH_SCRIPT_ADD(scriptfile, "#!/bin/sh\n");
    
    sprintf(vapStr,"ath%d%d",unit,vap);
    
    /*Turn off all vaps on this wlan*/
    ATH_SCRIPT_ADD(scriptfile, "/etc/ath/ath_VAP_down %d", unit);
    
    /*Config Current vap*/
    /*If wlan is disabled || vap is disabled , just destroy the vap if existing*/
    if(apCfgWlanStateGet(unit) == 0 || apCfgActiveModeGet(unit,vap) == 0)
    {
        ATH_SCRIPT_ADD(scriptfile, "/etc/ath/ath_VAP_stop %s destroy", vapStr);
    }
    else if(apCfgOpModeGet(unit) != CFG_OP_MODE_ROGAP)
    {
        int apmode = 0, wds = 0, sta = 0;
        
        if((apCfgOpModeGet(unit) != CFG_OP_MODE_PPT) && 
            (apCfgOpModeGet(unit) != CFG_OP_MODE_MPT) &&
            (apCfgOpModeGet(unit) != CFG_OP_MODE_UC) )
            apmode = 1;
        if((apCfgOpModeGet(unit) == CFG_OP_MODE_PPT) ||
            (apCfgOpModeGet(unit) == CFG_OP_MODE_MPT) ||
            (apCfgOpModeGet(unit) == CFG_OP_MODE_AP_PTP) ||
            (apCfgOpModeGet(unit) == CFG_OP_MODE_AP_PTMP))    
            wds = 1;
        if((apCfgOpModeGet(unit) == CFG_OP_MODE_UC ||
            apCfgOpModeGet(unit) == CFG_OP_MODE_UR))
            sta = 1;
            
        /*
        * Normal AP mode
        */
        if(apmode){
            /*If ap is required*/            
            ATH_SCRIPT_ADD(scriptfile, "/etc/ath/ath_VAP_stop %s destroy", vapStr);
            ATH_SCRIPT_ADD(scriptfile, "/etc/ath/ath_VAP_make %s ap", vapStr);
            if(sta) {
                /* If UR mode */
                ATH_SCRIPT_ADD(scriptfile, "/etc/ath/ath_VAP_stop ath%d%d destroy", unit,WLAN_MAX_VAP);
            }
        }
        else if(sta) {
            /* If UC mode */
            ATH_SCRIPT_ADD(scriptfile, "/etc/ath/ath_VAP_stop ath%d%d destroy", unit,WLAN_MAX_VAP);
        }
        else{
            /*No AP need*/
            ATH_SCRIPT_ADD(scriptfile, "/etc/ath/ath_VAP_stop %s destroy", vapStr);
        }
        
        /*
	    *Universal Client/Repeater setup
        */
        if (vap == 0 && sta)
        {
            int sta_vap = WLAN_MAX_VAP;
            
            sprintf(vapStr,"ath%d%d",unit, sta_vap);
            ATH_SCRIPT_ADD(scriptfile, "echo 1 > /proc/us_cfb");
            
            /*
        	*Make sta Vap
        	*/
        	ATH_SCRIPT_ADD(scriptfile, "/etc/ath/ath_VAP_make %s sta", vapStr);    
  
    		wlanBasicApply(scriptfile, unit, sta_vap, 1, 1);
    	    wlanAdvanceApply(scriptfile, unit, sta_vap, 1);
        	
        	/*Config security and Bring the interface up*/
        	wlanNoApSecurityApply(scriptfile, unit, sta_vap);
        	ATH_SCRIPT_ADD(scriptfile, "sleep 2");
            
            /*Configure bridge including stp parameters*/
            wlanBridgeApply(scriptfile, unit, sta_vap, 0);
                
        	ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/rc wpasupplicant restart&");
        }
        
        /* 
        * Pure WDS mode setup: PPT, MPT
        */    
        if(vap == 0 && wds && !apmode)
        {
            sprintf(vapStr,"ath%d%d",unit, vap);
            ATH_SCRIPT_ADD(scriptfile, "echo 1 > /proc/wds_cfb");
    
            /*
            *Make WDS Vap
            */
            ATH_SCRIPT_ADD(scriptfile, "/etc/ath/ath_VAP_make %s wds", vapStr);    
    		
    		wlanBasicApply(scriptfile, unit, vap, 1, 0);
    	    wlanAdvanceApply(scriptfile, unit, vap, 1);
  	        
  	        /*Config wds ap*/
        	wlanWdsApply(scriptfile, unit, vap);
            /*Config security and Bring the interface up*/
        	wlanNoApSecurityApply(scriptfile, unit, vap);
        	ATH_SCRIPT_ADD(scriptfile, "sleep 2");
        	
        	/*Configure bridge including stp parameters*/
            wlanBridgeApply(scriptfile, unit, vap, 1);
        	ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/rc wdspsk start&");
        }
        
        /*
        * Normal AP mode
        */
        if(apmode){
            if(vap!=0)  wds = 0;
                
            if(wds)
            {
                ATH_SCRIPT_ADD(scriptfile, "echo 1 > /proc/wds_cfb");
            }
                   
            /**/            
            wlanBasicApply(scriptfile, unit, vap, 0, 0);
	        wlanAdvanceApply(scriptfile, unit, vap, 0);
            wlanAclApply(scriptfile, unit, vap);

            if(wds)
            {
                /*Config wds ap*/
        	    wlanWdsApply(scriptfile, unit, vap);  
            }
              
            /*Config security and Bring the interface up*/
    		wlanSecurityApply(scriptfile, unit, vap);	
    		
            /*Configure bridge including stp parameters*/
    		wlanBridgeApply(scriptfile, unit, vap, wds?2:0);

        	/*wsc config start*/
        	if(vap == 0){
        	    /*wsc config file creat*/
        	    wlanCreatWscConfig(unit, vap);
        	    
        	    /*wsc upnp start*/
#ifdef WSC_UPNP
                ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/rc wscupnp restart&");
#endif
                if(wds)
                {
    		        ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/rc wdspsk start&");
    		    }
            }
        }
    }
    
    /*Turn on all vaps that should on this wlan*/
    ATH_SCRIPT_ADD(scriptfile, "/etc/ath/ath_VAP_up %d", unit);
    
    /*
    * Run and delete scriptfile
    */
    ATH_SCRIPT_RUN(scriptfile);
    ATH_SCRIPT_DEL(scriptfile);  
    
    return 0;    
}

static int ntp_handler(int unit, int vap)
{
/*    
 *  rc ntp restart
 */
#ifdef APL_DEBUG    
    applyLog("<%s,%d>: \n",__FUNCTION__,__LINE__);
#endif    
    APPLYRUN("rc ntp restart");
    return 0;
} 

static int snmp_handler(int unit, int vap)
{
/*    
 *  rc snmp restart
 */
#ifdef APL_DEBUG    
    applyLog("<%s,%d>: \n",__FUNCTION__,__LINE__);
#endif    
    APPLYRUN("rc snmp restart");
    return 0;
} 

static int log_handler(int unit, int vap)
{
/*
*   restart log including syslog,mail,local log
*/
#ifdef APL_DEBUG    
    applyLog("<%s,%d>: \n",__FUNCTION__,__LINE__);
#endif    
    APPLYRUN("rc syslogd restart");
    return 0;    
} 

static int ftp_handler(int unit, int vap)
{
/*    
 *  rc ftp restart
 */
#ifdef APL_DEBUG    
    applyLog("<%s,%d>: \n",__FUNCTION__,__LINE__);
#endif    
    APPLYRUN("rc ftp restart");
    return 0;
} 

static int eth_datarate_handler(int unit, int vap)
{
    /*
     * *   Restart ethernet data rate
     * */
    int datarate = apCfgEthDataRateGet();
    int force100m = apCfgForce100mGet();
		int auto_nego = apCfgAutonegoGet();
		int duplex_mode = apCfgDuplexmodeGet();
    if (force100m) {
        printf("apply: enable force LAN port speed to 100M.\n");
        APPLYRUN("echo 1 %d 1 > /proc/ethernet_status_cfb", 1);
    }
    else
    	{
    if(datarate!=0 && (auto_nego == 0)){
        printf("apply: setting ethernet data rate to %dMpbs.\n", datarate);
        APPLYRUN("echo 1 %d %d > /proc/ethernet_status_cfb", (datarate==10)? 0:((datarate==100)? 1:2),duplex_mode);
    }else{
        printf("apply: setting ethernet data rate to \"auto detect\".\n");
        APPLYRUN("echo 0 0 0 > /proc/ethernet_status_cfb");
    }

    if(datarate == 1000){
        system("ifconfig eth0 down");
        system("ifconfig eth0 up");
    }
  }
    return 0;
}

static int force100m_handler(int unit, int vap)
{
    int force100m = apCfgForce100mGet();
		int port_speed = apCfgPortspeedGet();
		int auto_nego = apCfgAutonegoGet();
		int duplex_mode = apCfgDuplexmodeGet();
    if (force100m) {
        printf("apply: enable force LAN port speed to 100M.\n");
        APPLYRUN("echo 1 %d 1 > /proc/ethernet_status_cfb", 1);
    } else {
    	
    	if(auto_nego==0) //disabled auto negotiation
    		{
	        APPLYRUN("echo 1 %d %d > /proc/ethernet_status_cfb",port_speed,duplex_mode);						
    			}
			else	
				{
        printf("apply: disable force LAN port speed to 100M.\n");
        eth_datarate_handler(0, 0);
    }
    }

    return 0;
}

int mdns_stopped = 0;

int apl_handleApplyTree(unsigned int id)
{
    int index = id;
    int ret = 0;

    // extra steps for bonjour: stop it first
    if (mdns_stopped == 0 // already stopped before
            && (applyTable[APL_BONJOUR].flag // set BONJOUR flag for static
                || (apCfgDhcpEnableGet() && applyTable[APL_IPv4].flag)))
        // dhcpc mode: set IPv4 flag
    {
#ifdef APL_DEBUG
        applyLog("<%s,%d>: make sure bonjour is stopped.\n", __FUNCTION__, __LINE__);
#endif
        SYSTEM("rc mdns stop");
        mdns_stopped = 1;
    }

    do
    {
        if(applyTable[index].flag)
        {
#ifdef APL_DEBUG    
            applyLog("<%s,%d>: apply item %s, flag=%d\n",__FUNCTION__,__LINE__,idString[index],applyTable[index].flag);
#endif    
            if(applyTable[index].handler)
            {
                ret = applyTable[index].handler(applyTable[index].unit, applyTable[index].vap);
                if(ret != 0)
                {
#ifdef APL_DEBUG    
                        applyLog("<%s,%d>: apply error\n",__FUNCTION__,__LINE__);
#endif    
                    return ret;
                }
            }
        }
        else
        {
            if(applyTable[index].child != NO_ACC)
            {
                ret = apl_handleApplyTree(applyTable[index].child);
            }
        }
        if(applyTable[index].rbrother == NO_ACC)
        {
            break;
        }
        else
        {
            index = applyTable[index].rbrother;
        }
    }while(1);
#ifdef APL_DEBUG    
               applyLog("<%s,%d>:%d apply end\n",__FUNCTION__,__LINE__,ret);
#endif 
    return ret;
}

int apl_do_apply(void)
{
    
    FILE *fp;
    char buffer[8];
    int id;
    
    if(access(APL_FLAG, F_OK)!=0 || access(APL_RUNNING,F_OK)==0)
        return 0;
    
     /*Turn on the applying flag*/
    SYSTEM("echo 1 > %s", APL_PARSING);
    SYSTEM("echo 1 > %s", APL_RUNNING);
    
    if((fp = fopen(APL_FLAG, "r")) == NULL)
    {
        unlink(APL_PARSING);
        unlink(APL_RUNNING);
        return 0;
    }
        
    while(!feof(fp))
	{
	    memset(buffer,0,8);
        fgets(buffer,8,fp);  
        id = atoi(buffer);
        if(id > 0)
        {
            applyTable[id].flag = 1;
#ifdef APL_DEBUG    
            applyLog("<%s,%d>: set item %s\n",__FUNCTION__,__LINE__,idString[atoi(buffer)]);
#endif 
        }
    }
    fclose(fp);
    unlink(APL_FLAG);
    unlink(APL_PARSING);
    
//    APPLYRUN = printf;        //for debug
    APPLYRUN = SYSTEM;
    
    apl_handleApplyTree(APL_ROOT);
    #ifdef APL_DEBUG    
            applyLog("<%s,%d>:apl_handleApplyTree\n",__FUNCTION__,__LINE__);
#endif 
    /*Turn off the applying flag*/
    unlink(APL_RUNNING);
    
    return 0;
}

int apl_close(int close)
{
    if(close)
        SYSTEM("echo 1 > %s", APL_CLOSED);
    else    
        unlink(APL_CLOSED);
    return 0;    
}

void apl_set_flag(unsigned int id)
{
    if(access(APL_CLOSED, F_OK) != 0)
    {
        FILE *fp;
    
        while(access(APL_PARSING,F_OK)==0);
        
        fp = fopen(APL_FLAG, "a+");
        
        if(fp!=NULL)
        {
#ifdef APL_DEBUG    
            applyLog("<%s,%d>: change item %s\n",__FUNCTION__,__LINE__,idString[id]);
#endif
            
            fprintf(fp, "%d\n", id);
            fclose(fp);
        }            
    }
}

void apl_clear_flag(void)
{
    int i;
    int tableSize = sizeof(applyTable)/sizeof(APPLY_TABLE);
    for(i=0; i<tableSize; i++)
    {
        applyTable[i].flag = 0;
    }
}
int apcfgReboot(void)
{
	int reboot= 0;
	
	if(access(APL_FLAG,F_OK) == 0)
    {
        FILE *fp;
        char buffer[8];
        int id;

        if((fp = fopen(APL_FLAG, "r")) != NULL)
        {
        	while(!feof(fp))
        	{
        	    memset(buffer,0,8);
                fgets(buffer,8,fp);  
                id = atoi(buffer);
                if(id == APL_REBOOT)
                {
                	reboot = 1;
                	break;
                }
            }
            fclose(fp);
        }
    }
    return reboot;
}
int apcfgisIphander(void)
{
	int iphander= 0;
	
	if(access(APL_FLAG,F_OK) == 0)
    {
        FILE *fp;
        char buffer[8];
        int id;

        if((fp = fopen(APL_FLAG, "r")) != NULL)
        {
        	while(!feof(fp))
        	{
        	    memset(buffer,0,8);
                fgets(buffer,8,fp);  
                id = atoi(buffer);
                if(id == APL_IPv4 || id == APL_IPv6)
                {
                	iphander = 1;
                	break;
                }
            }
            fclose(fp);
        }
    }
    return iphander;
}

int apcfgHaveChanged(void)
{
    int applySeconds = 0;
    
#define APPLY_COUNTRY       25     
#define APPLY_AUTOCHANNEL   15
#define APPLY_NORMAL        15

    if(access(APL_FLAG,F_OK) == 0)
    {
        FILE *fp;
        char buffer[8];
        int id;
        
        if((fp = fopen(APL_FLAG, "r")) != NULL)
        {
            applySeconds = APPLY_NORMAL;
            while(!feof(fp))
        	{
        	    memset(buffer,0,8);
                fgets(buffer,8,fp);  
                id = atoi(buffer);
//                if(id == COUNTRY || id == WLAN00_OPMODE)
//                {
//                   applySeconds = APPLY_COUNTRY;
//                   break;
//                }
//                if(id == WLAN00_RADIOCHANNEL)
//                {
//                    if(apCfgRadioChannelGet(RADIO_24G)==0)
//                        applySeconds = APPLY_AUTOCHANNEL;
//                }
            }
            fclose(fp);
        }
    }
    
    return applySeconds;
}

/***************************************************************************************
 *  Debug function
 ***************************************************************************************/
#if 1

static int treeDepth = 0;

void printApplyTree(unsigned int id, int back)
{
    int i = 0;
    int index = id;
    int backDepth = 0;
    int length;
    do
    {        
        length = 0;
        length += printf("%s", idString[applyTable[index].id]);
        
        if(applyTable[index].child != NO_ACC)
        {
            length += printf("---");
            treeDepth += length;
            backDepth = length;
            printApplyTree(applyTable[index].child, backDepth);
        }
        else
        {
            printf("\n");
        }
        if(applyTable[index].rbrother == NO_ACC)
        {
            break;
        }
        else
        {
            for(i=0;i<treeDepth;i++)
            {
                printf(" ");
            }
            index = applyTable[index].rbrother;
        }
    }while(1);
    treeDepth -= back;
}
#endif



