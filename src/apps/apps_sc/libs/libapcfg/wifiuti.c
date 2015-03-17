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
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include "apcfg.h"
#include "utility.h"
//#define __ICONV_SUPPORT__ 
#ifdef __ICONV_SUPPORT__
#include "lca_conv.h"
#include "lca_conv.c"

#endif

#ifndef HOSTAP_CFG
#define HOSTAP_CFG	        "/var/hostap.cfg"
#endif

#define     ATH_UP          "/tmp/wlan%d_up%d"
enum{
    ATH_UP_PRE=0,           //for module and channel issue
    ATH_UP_MAKE,            //for make 
    ATH_UP_STA,             //for active sta
    ATH_UP_AP,              //for active ap/wds vap
    ATH_UP_SUF,             //for action after wifi started
    ATH_UP_LAST,            //last                 
    ATH_UP_MAX    
};

#define     ATH_DOWN        "/tmp/wlan%d_down"
#define     ATH_DELIF       "/tmp/wlan_delif"

#define VAP_STR_LEN		5
#define PVID                apCfgVlanPvidGet(unit,vap)
#define PVID_WDS            apCfgVlanPvidGet(unit,0)
#define PRI_WDS             apCfgPriorityGet(unit,0)


/*
* Wireless config functions
*/

void ATH_SCRIPT_DEL(char *scriptFile)
{
    unlink(scriptFile);
}

int ATH_SCRIPT_ADD(char *scriptFile, const char *format, ...)
{
    FILE *fscript;
    va_list arg;

    fscript=fopen(scriptFile, "a+");
    if(fscript==NULL) return -1;
	va_start(arg, format);
	vfprintf(fscript, format, arg);
	va_end(arg);
	fprintf(fscript, "\n");
    fclose(fscript);
    return 0;
}

void ATH_SCRIPT_RUN(char *scriptFile)
{
    chmod(scriptFile, 0x777);
    system(scriptFile);
}

#ifdef LINUX_WSC

// Authentication types
#define WSC_AUTHTYPE_OPEN        0x0001
#define WSC_AUTHTYPE_WPAPSK      0x0002
#define WSC_AUTHTYPE_SHARED      0x0004
#define WSC_AUTHTYPE_WPA         0x0008
#define WSC_AUTHTYPE_WPA2        0x0010
#define WSC_AUTHTYPE_WPA2PSK     0x0020
//This isn't in spec, just for implement, Terry Yang 2007-4-10 17:01:12
#define WSC_AUTHTYPE_1X          0x0040

// Encryption type
#define WSC_ENCRTYPE_NONE       0x0001
#define WSC_ENCRTYPE_WEP        0x0002
#define WSC_ENCRTYPE_TKIP       0x0004
#define WSC_ENCRTYPE_AES        0x0008

#ifndef SIZE_32_BYTES
#define SIZE_32_BYTES           32
#define SIZE_128_BYTES          128
#endif

typedef struct 
{
	char ssid[SIZE_32_BYTES+1];
	unsigned char authtype;
	unsigned char encrytype;
	unsigned char keyindex;
	char networkkey[SIZE_128_BYTES];
}userconfig;

void wlanCreatWscConfig(int unit, int vap)
{
    char fileName[32];
	userconfig   uconfig;
	int authType;
	FILE *ucfp=NULL;
	
    memset(&uconfig,0,sizeof(userconfig));
	strcpy(uconfig.ssid,apCfgSsidGet(unit,vap));
	uconfig.ssid[32]=0;
	authType = apCfgAuthTypeGet(unit,vap);
	switch(authType)
	{
	    default:
		case APCFG_AUTH_NONE:
	        uconfig.encrytype = WSC_ENCRTYPE_NONE;
			uconfig.authtype = WSC_AUTHTYPE_OPEN;
			uconfig.keyindex = 1;
			strcpy(uconfig.networkkey,"");
			break;
	    case APCFG_AUTH_OPEN_SYSTEM:
	    case APCFG_AUTH_SHARED_KEY:
    	case APCFG_AUTH_AUTO:
	        uconfig.encrytype = WSC_ENCRTYPE_WEP;	        
	        if(authType == APCFG_AUTH_OPEN_SYSTEM)
				uconfig.authtype  =WSC_AUTHTYPE_OPEN;
			else if(authType == APCFG_AUTH_SHARED_KEY)
				uconfig.authtype  =WSC_AUTHTYPE_SHARED;
			else
				uconfig.authtype  =WSC_AUTHTYPE_SHARED|WSC_AUTHTYPE_OPEN;
	        uconfig.keyindex = apCfgDefKeyGet(unit,vap);
	        strcpy(uconfig.networkkey,apCfgKeyValGet(unit,vap, apCfgDefKeyGet(unit,vap)));
	        break;
	    
	    
   		case APCFG_AUTH_WPAPSK:
    	case APCFG_AUTH_WPA2PSK:
    	case APCFG_AUTH_WPA_AUTO_PSK:
   		    if(authType == APCFG_AUTH_WPAPSK)
				uconfig.authtype  =WSC_AUTHTYPE_WPAPSK;
			else if(authType == APCFG_AUTH_WPA2PSK)
				uconfig.authtype  =WSC_AUTHTYPE_WPA2PSK;
			else
				uconfig.authtype  =WSC_AUTHTYPE_WPAPSK;
			if(apCfgWPACipherGet(unit,vap) == WPA_CIPHER_AES)
				uconfig.encrytype= WSC_ENCRTYPE_AES;
			else if(apCfgWPACipherGet(unit,vap) == WPA_CIPHER_TKIP)
				uconfig.encrytype= WSC_ENCRTYPE_TKIP;
		    else
		        uconfig.encrytype= WSC_ENCRTYPE_TKIP;
			uconfig.keyindex = 1;
			strcpy(uconfig.networkkey,apCfgPassphraseGet(unit,vap));
			break;
	    case APCFG_AUTH_WPA:
    	case APCFG_AUTH_WPA2:
    	case APCFG_AUTH_WPA_AUTO:
    	case APCFG_AUTH_DOT1X:
    	    #ifdef T_DEBUG    
    	        printf("<%s,%d>: Current security mode didn't support WPS\n",__FUNCTION__,__LINE__);
    	    #endif    
    	    if(authType == APCFG_AUTH_WPA)
				uconfig.authtype  =WSC_AUTHTYPE_WPA;
			else if(authType == APCFG_AUTH_WPA2)
				uconfig.authtype  =WSC_AUTHTYPE_WPA2;
			else if(authType == APCFG_AUTH_WPA_AUTO)
				uconfig.authtype  =WSC_AUTHTYPE_WPA;
		    else
		        uconfig.authtype  =WSC_AUTHTYPE_1X;
		        
			if(apCfgWPACipherGet(unit,vap) == WPA_CIPHER_AES)
				uconfig.encrytype= WSC_ENCRTYPE_AES;
			else if(apCfgWPACipherGet(unit,vap) == WPA_CIPHER_TKIP)
				uconfig.encrytype= WSC_ENCRTYPE_TKIP;
		    else
		        uconfig.encrytype= WSC_ENCRTYPE_TKIP;
			uconfig.keyindex = 1;
			
	        break;
	}
	sprintf(fileName,"/tmp/wscdata_%d%d.bin",unit,vap);
	ucfp=fopen(fileName,"w");
	if(ucfp)
	{
		fwrite(&uconfig,sizeof(userconfig),1,ucfp);
		fclose(ucfp);
	}
}
#endif

void wlanCorrectChannelApply(int unit)
{
    int curChan = 0, maxChan = 0;
    FILE *fp;
    char line[32];
    
    /*
    * Correc the channel and the Extension Sub-Channel 
    */
    if(apCfgChannelWidthModeGet(unit) == 0)  
        return;
        
    if(access("/tmp/chan_list",F_OK)!=0 || access("/tmp/chan_curr",F_OK)!=0)
    {
        printf(" ERROR: No file chan_list&chan_curr existing.\n");
        syslog(LOG_ERR,"[SC][systme_error] Channel error.");
        return;
    }      
    
    //Get current channel and max valid channel 
	fp = fopen("/tmp/chan_list", "r");
	while(fgets(line,sizeof(line),fp))
	{
	    if(strcmp(line,"available frequencies :\n") == 0)
	        continue;
	    
	    if(strcmp(line,"current:\n") == 0){
	        fgets(line,sizeof(line),fp);
	        curChan = atoi(line);
	        break;   
	    }else{
	        maxChan = atoi(line);
	    }
	}
	fclose(fp);

    //Correct Channel
    if( apCfgAutoChannelGet(unit)==0 && apCfgRadioChannelGet(unit) != curChan){
        system("scapply close");
        apCfgRadioChannelSet(unit, curChan);
        system("scapply open");
    }
    
    /*
    * For 11ng, 0--static 20; 1--dyn 2040; 2-- static 40
    */
    //for static 20, we do not need the Extension Sub-Channel
    if(apCfgChannelWidthModeGet(unit) == 0)  
        return;
        
    //Correct the offset    
    system("scapply close");
    
    if(curChan-1 < 4 && strcmp(apCfgChannelOffsetGet(unit), "-1") == 0)
        apCfgChannelOffsetSet(unit, "1");
        
    if(maxChan-curChan < 4 &&  strcmp(apCfgChannelOffsetGet(unit), "1") == 0)
        apCfgChannelOffsetSet(unit, "-1");
    
    system("scapply open");
    
    return;
}

void wlanBasicApply(char *scriptfile, int unit, int bss, int all, int sta)
{
    char vapStr[32];
    //int txpower;
    int vap, wirelessMode = apCfgFreqSpecGet(unit);
	
	vap = bss;
    if (bss == WLAN_MAX_VAP)
        vap = 0;
    sprintf(vapStr, "ath%d%d", unit, bss);
    
    //Disable Background Scan
    ATH_SCRIPT_ADD(scriptfile, "iwpriv %s bgscan 0 > /dev/null 2>&1", vapStr);
    ATH_SCRIPT_ADD(scriptfile, "iwpriv %s doth 0 > /dev/null 2>&1", vapStr);
    ATH_SCRIPT_ADD(scriptfile, "iwpriv %s ff 0 > /dev/null 2>&1", vapStr);

    //Check for RF command. If vap0, set the RF parameters, else do the
    //simple cofiguration.
    if(vap == 0 || all)
    {
    
        /*
        * 11n configuration section
        */
        //increase queue length
        ATH_SCRIPT_ADD(scriptfile, "/sbin/ifconfig %s txqueuelen %d", vapStr, 1000);
        //turn on halfgi
        ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/iwpriv %s shortgi %d > /dev/null 2>&1", vapStr, (scApCfgShortGIGet(unit)==SHORTGI_LONG)?0:1);
        //wireless mode
        ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/iwconfig %s channel 0",vapStr);
        //11b
        if(wirelessMode == MODE_SELECT_11B)
        {
            ATH_SCRIPT_ADD(scriptfile, "iwpriv %s mode 11B > /dev/null 2>&1", vapStr);
        }
        //11g only/11bg
        else if(wirelessMode == MODE_SELECT_11G || wirelessMode == MODE_SELECT_11BG)
        {
            ATH_SCRIPT_ADD(scriptfile, "iwpriv %s mode 11G > /dev/null 2>&1", vapStr);
            ATH_SCRIPT_ADD(scriptfile, "iwpriv %s pureg %d > /dev/null 2>&1", vapStr, (wirelessMode == MODE_SELECT_11G)?1:0);
        }
        //11n
        else
        {   
            //Channel Width Mode
            //cwmmode 0 is static 20; cwmmode 1 is dyn 2040; cwmmode 2 is static 40
            if(apCfgChannelWidthModeGet(unit) == 0)
            {
                ATH_SCRIPT_ADD(scriptfile, "iwpriv %s mode 11NGHT20 > /dev/null 2>&1", vapStr);
                ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/iwpriv %s cwmmode 0", vapStr);
            }else{
                ATH_SCRIPT_ADD(scriptfile, "iwpriv %s extoffset %s > /dev/null 2>&1", vapStr, apCfgChannelOffsetGet(unit));
                if(strcmp(apCfgChannelOffsetGet(unit), "1")==0)
                    ATH_SCRIPT_ADD(scriptfile, "iwpriv %s mode 11NGHT40PLUS > /dev/null 2>&1", vapStr);
                else
                    ATH_SCRIPT_ADD(scriptfile, "iwpriv %s mode 11NGHT40MINUS > /dev/null 2>&1", vapStr);    
                ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/iwpriv %s cwmmode %d", vapStr, apCfgChannelWidthModeGet(unit));
            }
            
            ATH_SCRIPT_ADD(scriptfile, "iwpriv wifi%d ForBiasAuto 1", unit);
            ATH_SCRIPT_ADD(scriptfile, "iwpriv %s puren %d > /dev/null 2>&1", vapStr,(wirelessMode == MODE_SELECT_11N)?1:0);
            ATH_SCRIPT_ADD(scriptfile, "iwpriv %s pureg 0 > /dev/null 2>&1", vapStr);
        }
        
        ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/iwpriv wifi%d HALDbg 0",unit);
        //Set Aggregation State
        //set mpdu aggregation
        ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/iwpriv wifi%d AMPDU %d",unit, scApCfgAmpduGet(unit));
        //set number of sub-frames in an ampdu
        ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/iwpriv wifi%d AMPDUFrames 32",unit);
        //set ampdu limit to 50000
        ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/iwpriv wifi%d AMPDULim 50000",unit);
        //Disable ANI as this is causing Rx issues at Low RSSI
        ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/iwpriv wifi%d ANIEna 0",unit);
        
        //Set the chain masks
        ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/iwpriv wifi%d txchainmask 7",unit);
        ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/iwpriv wifi%d rxchainmask 7",unit);
 #if 0
        //set tx power
		txpower = scMaxTxPowerGet(unit)-apCfgPowerGet(unit);
		ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/iwpriv wifi%d TXPowLim %d > /dev/null 2>&1", 0, 2*((txpower>1)?txpower:1));
 #endif     
    	//An extra IE is provided for Intel interop
        ATH_SCRIPT_ADD(scriptfile, "echo 1 > /proc/sys/dev/ath/htdupieenable");
    }else{
        //Pure G
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s pureg %d > /dev/null 2>&1", vapStr, (wirelessMode == MODE_SELECT_11G)?1:0);
        //Pure N
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s puren %d > /dev/null 2>&1", vapStr,(wirelessMode == MODE_SELECT_11N)?1:0);
    }
    
    //if sta, Config remote AP address, and needn't config the channel
    if (sta){
        if(sta == 1) {
            char ucr_tmp[20];
            ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/iwconfig %s mode managed", vapStr);
            //remote AP address
            apCfgUcrRemoteApMacAddrGet(RADIO_24G, ucr_tmp);
            if(strcmp(ucr_tmp,"00:00:00:00:00:00") != 0){
                ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/iwconfig %s ap %s", vapStr, ucr_tmp);    
            }
        }
    } else //if(vap == 0 || all) //set frequency only
    {
        FILE *fp = NULL;
        char buffer[130];
        
    	fp = fopen("/tmp/chan_curr", "r");
        
        if(fp != NULL)
    	{
        	if(fgets(buffer,129,fp)!=NULL)
        	{
                ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/iwconfig %s channel %s",vapStr, buffer);
            }
            fclose(fp);
        }
    }

}

void wlanAdvanceApply(char *scriptfile, int unit, int bss, int all)
{
    char vapStr[32];
    int vap;
    int vap_num=1;
    int i=0;
    
    vap = bss;
    if (bss == WLAN_MAX_VAP)
        vap = 0;
    sprintf(vapStr,"ath%d%d",unit,bss);

    /*set WMM*/
    if(all) {
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s wmm %d > /dev/null 2>&1", vapStr, apCfgWmeGet(unit,0)?1:0);
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s uapsd %d > /dev/null 2>&1", vapStr, apCfgWmmpsGet(unit,0)?1:0);
    } else {
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s wmm %d > /dev/null 2>&1", vapStr, apCfgWmeGet(unit,vap)?1:0);
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s uapsd %d > /dev/null 2>&1", vapStr, apCfgWmmpsGet(unit,vap)?1:0);
    }
    
    if(vap == 0 || all)
    {    
        /* set Worldwide Mode (802.11d) */
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s countryie %d > /dev/null 2>&1", vapStr, scApCfg80211dEnabledGet(unit)?1:0);
            
        //Set beacon interval
        		for(i=1;i<4;i++)
        		{
        			if(apCfgActiveModeGet(unit,i))
        			vap_num++;
        			}
            ATH_SCRIPT_ADD(scriptfile, "iwpriv %s bintval %d > /dev/null 2>&1", vapStr, vap_num * apCfgBeaconIntervalGet(unit));
/*        if(apCfgBeaconIntervalGet(unit) < 400 && 
            (apCfgActiveModeGet(unit,1) || apCfgActiveModeGet(unit,2) || apCfgActiveModeGet(unit,3)))
        {
            ATH_SCRIPT_ADD(scriptfile, "iwpriv %s bintval 400 > /dev/null 2>&1", vapStr);
        }else{
            ATH_SCRIPT_ADD(scriptfile, "iwpriv %s bintval %d > /dev/null 2>&1", vapStr, apCfgBeaconIntervalGet(unit));
        }
        */
        //Set DTIM Interval
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s dtim_period %d > /dev/null 2>&1", vapStr, scApCfgDtimIntervalGet(unit));
				if(apCfgMultiEnhanceGet() == 1)
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s mcastenhance 2 > /dev/null 2>&1", vapStr);  // Enable Multicast translate to Unicast function
    }
    
    /* aging timeout */
    /*
    if(scApCfgIdleTimeoutIntervalGet(unit) == 0)
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s inact 86400 > /dev/null 2>&1", vapStr);   //one day
    else
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s inact %d > /dev/null 2>&1", vapStr, scApCfgIdleTimeoutIntervalGet(unit)*60);
    */
    
    //Set RTS/CTS threshold
    ATH_SCRIPT_ADD(scriptfile, "iwconfig %s rts %d > /dev/null 2>&1", vapStr, apCfgRtsThresholdGet(unit));
    //Set fragment length
    ATH_SCRIPT_ADD(scriptfile, "iwconfig %s frag %d > /dev/null 2>&1", vapStr, apCfgFragThresholdGet(unit));
    //Protect Mode
    ATH_SCRIPT_ADD(scriptfile, "iwpriv %s protmode %d > /dev/null 2>&1", vapStr, apCfgCTSModeGet(unit));
    
    //Preamble Type
    //ATH_SCRIPT_ADD(scriptfile, "iwpriv %s shpreamble %d > /dev/null 2>&1", vapStr, apCfgShortPreambleGet(unit));
}

void wlanAclApply(char *scriptfile, int unit, int vap)
{
	char vapStr[32];
	
	sprintf(vapStr,"ath%d%d",unit,vap);
	
    if(apCfgAclModeGet(unit, vap) == APCFG_ACL_LOCAL)    
    {
        struct scAclBuf_s *pScAclCurr = NULL, *pScAcl = NULL;
        
        /*Enable local ACL*/
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s maccmd 3  > /dev/null 2>&1", vapStr);
        if(apCfgAclTypeGet(unit, vap) == 0)
            ATH_SCRIPT_ADD(scriptfile, "iwpriv %s maccmd 1  > /dev/null 2>&1", vapStr);
	    else
	    	ATH_SCRIPT_ADD(scriptfile, "iwpriv %s maccmd 2  > /dev/null 2>&1", vapStr);
        /*Disable Radius ACL*/
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s maccmd 6  > /dev/null 2>&1", vapStr);    
          
    	while (!scAclBufGet(unit, vap, &pScAcl)) ;
    	if(pScAcl)
    	{
    	    for(pScAclCurr = pScAcl; pScAclCurr; pScAclCurr = pScAclCurr->next)
    	    {
    	        if(pScAclCurr->used)
    	    		ATH_SCRIPT_ADD(scriptfile, "iwpriv %s addmac %s  > /dev/null 2>&1", vapStr,pScAclCurr->mac);
    	    }
    	}	
        scAclBufFree(unit, vap, pScAcl); 
    }
    else
    {
        /*Disable local ACL*/
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s maccmd 3  > /dev/null 2>&1", vapStr);
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s maccmd 0  > /dev/null 2>&1", vapStr);
        
        /*If Radius ACL is enabled, then enable it; else disable it*/
        if(apCfgAclModeGet(unit, vap) == APCFG_ACL_RADIUS)
        {
            ATH_SCRIPT_ADD(scriptfile, "iwpriv %s maccmd 5  > /dev/null 2>&1", vapStr);
        }else{
            ATH_SCRIPT_ADD(scriptfile, "iwpriv %s maccmd 6  > /dev/null 2>&1", vapStr);  
        }
    }
}

void wlanAclListApply(char *scriptfile)
{
    FILE *fp;
    char line[64+1];
    int unit = RADIO_24G;
    
    /*If not local, then needn't apply the acl list*/
    if(apCfgAclModeGet(unit, 0) != APCFG_ACL_LOCAL)  
        return ;
               
    /*
    * List All vaps
    */
    system("iwconfig | grep ath | cut -b 1-5 > /tmp/vaplistx.tmp");  
    while((fp=fopen("/tmp/vaplistx.tmp","r"))==NULL);
    while(fgets(line, 64, fp)!=NULL){
        
        struct scAclBuf_s *pScAclCurr = NULL, *pScAcl = NULL;
        
        line[5] = 0;
        
        /*Enable local ACL*/
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s maccmd 3  > /dev/null 2>&1", line);
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s maccmd 1  > /dev/null 2>&1", line);
        /*Disable Radius ACL*/
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s maccmd 6  > /dev/null 2>&1", line);    
          
        while (!scAclBufGet(unit, line[4]-'0', &pScAcl)) ;
        if(pScAcl)
        {
            for(pScAclCurr = pScAcl; pScAclCurr; pScAclCurr = pScAclCurr->next)
            {
                if(pScAclCurr->used)
            		ATH_SCRIPT_ADD(scriptfile, "iwpriv %s addmac %s  > /dev/null 2>&1", line, pScAclCurr->mac);
            }
        }	
        scAclBufFree(unit, line[4]-'0', pScAcl); 
    }   
    fclose(fp);
    unlink("/tmp/vaplistx.tmp");
}

void wlanWdsApply(char *scriptfile, int unit, int vap_id)
{
    char vapStr[32];
	unsigned char remoteMac[18];

	sprintf(vapStr,"ath%d%d", unit, vap_id);
	
    /*
    * Config WDS
    */
    ATH_SCRIPT_ADD(scriptfile, "iwpriv %s wdstype 0 > /dev/null 2>&1", vapStr);    
    ATH_SCRIPT_ADD(scriptfile, "iwpriv %s wds_del FF:FF:FF:FF:FF:FF > /dev/null 2>&1",vapStr);
    
    if((apCfgOpModeGet(unit) == CFG_OP_MODE_PPT) || 
        (apCfgOpModeGet(unit) == CFG_OP_MODE_MPT))
    {
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s wdstype 1 > /dev/null 2>&1", vapStr);
    }else if((apCfgOpModeGet(unit) == CFG_OP_MODE_AP_PTP) ||
            (apCfgOpModeGet(unit) == CFG_OP_MODE_AP_PTMP))
    {
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s wdstype 2 > /dev/null 2>&1", vapStr);
    }
    
    if((apCfgOpModeGet(unit) == CFG_OP_MODE_PPT) || (apCfgOpModeGet(unit) == CFG_OP_MODE_AP_PTP))
    {
        memset(remoteMac, 0, 18);
        apCfgRemoteApMacAddrGet(unit, remoteMac);
        ATH_SCRIPT_ADD(scriptfile, "iwpriv %s wds_add %s > /dev/null 2>&1",vapStr,remoteMac);
    }
    else
    {
        int i;
        for(i=0;i<8;i++)
        {
            memset(remoteMac,0,18);
            apCfgRemoteWbrMacAddrGet(unit, i, remoteMac); 
            if(strcmp(remoteMac,"00:00:00:00:00:00")!=0)
                ATH_SCRIPT_ADD(scriptfile, "iwpriv %s wds_add %s > /dev/null 2>&1",vapStr,remoteMac);
        }
    }
    ATH_SCRIPT_ADD(scriptfile, "iwpriv %s wds 1 > /dev/null 2>&1",vapStr);
}
void wlanSecurityApply(char *scriptfile, int unit, int vap)
{
	int authType, authmode;
	char vapStr[32], hostApCfgName[32];
	char wps_model_number[32];
    char vapSsid[64+1];
#ifdef   __ICONV_SUPPORT__    
	char converted_text[128] = {0};
	char *pSrc=NULL;
	int ret=0;
#endif
	getVersion(wps_model_number);
	sprintf(vapStr,"ath%d%d",unit,vap);
	sprintf(hostApCfgName,"%s.%s",HOSTAP_CFG, vapStr);

	authType = apCfgAuthTypeGet(unit,vap);                
    
    //Set SSID
    scConversionSSID(apCfgSsidGet(unit,vap), vapSsid);
#ifdef   __ICONV_SUPPORT__     
    //add carole
	 ret=do_convert(UTF2LAN, vapSsid, strlen(vapSsid), converted_text, 128);
	if(ret!=-1){
		if(strlen(converted_text)<128)		
			strncpy(vapSsid, converted_text, MAX_SSID);
	}
#endif    

	ATH_SCRIPT_ADD(scriptfile, "iwconfig %s essid \"%s\" > /dev/null 2>&1", vapStr, vapSsid);
    //Broadcast SSID
	ATH_SCRIPT_ADD(scriptfile, "iwpriv %s hide_ssid %d > /dev/null 2>&1", vapStr, apCfgSsidModeGet(unit,vap));
	//Wireless separate
	ATH_SCRIPT_ADD(scriptfile, "iwpriv %s ap_bridge %d > /dev/null 2>&1", vapStr, apCfgIntraVapForwardingGet(unit,vap)?0:1);
    
	switch(authType)
	{
		default:
		case APCFG_AUTH_NONE:
			authmode = 1;
			ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/iwconfig %s key off",vapStr);
			ATH_SCRIPT_ADD(scriptfile, "iwpriv %s authmode %d > /dev/null 2>&1",vapStr, authmode);
		    ATH_SCRIPT_ADD(scriptfile, "iwpriv %s privacy 0 > /dev/null 2>&1",vapStr); 
			break;
		case APCFG_AUTH_OPEN_SYSTEM:
    	case APCFG_AUTH_SHARED_KEY:
    	case APCFG_AUTH_AUTO:
    		authmode = (authType==APCFG_AUTH_AUTO)? 4 : authType;

		    ATH_SCRIPT_ADD(scriptfile, "iwpriv %s authmode %d > /dev/null 2>&1",vapStr, authmode);
		    ATH_SCRIPT_ADD(scriptfile, "iwpriv %s privacy 1 > /dev/null 2>&1",vapStr);
			
			if(apCfgKeyEntryMethodGet(unit,vap) == KEY_ENTRY_METHOD_HEX)
			{
			    if(strlen(apCfgKeyValGet(unit,vap, apCfgDefKeyGet(unit,vap)))==0){
			        syslog(LOG_ERR,"[SC][systme_error][%s] No WEP Key.", apCfgSsidGet(unit,vap));
			    }
			    ATH_SCRIPT_ADD(scriptfile, "iwconfig %s key \"[%d]\" %s > /dev/null 2>&1",vapStr,apCfgDefKeyGet(unit,vap),apCfgKeyValGet(unit,vap, apCfgDefKeyGet(unit,vap)));
			}
			else
			{
			    if(strlen(apCfgKeyValGet(unit,vap, apCfgDefKeyGet(unit,vap)))==0){
			        syslog(LOG_ERR,"[SC][systme_error][%s] No WEP Key.", apCfgSsidGet(unit,vap));
			    }
			    ATH_SCRIPT_ADD(scriptfile, "iwconfig %s key \"[%d]\" s:%s > /dev/null 2>&1",vapStr,apCfgDefKeyGet(unit,vap),apCfgKeyValGet(unit,vap, apCfgDefKeyGet(unit,vap)));
			}
            ATH_SCRIPT_ADD(scriptfile, "iwconfig %s key \"[%d]\" > /dev/null 2>&1",vapStr,apCfgDefKeyGet(unit,vap));
			break;
		case APCFG_AUTH_WPA:
   		case APCFG_AUTH_WPAPSK:
    	case APCFG_AUTH_WPA2:
    	case APCFG_AUTH_WPA2PSK:
    	case APCFG_AUTH_WPA_AUTO:
    	case APCFG_AUTH_WPA_AUTO_PSK:
    	case APCFG_AUTH_DOT1X:	
    	{
    		FILE *fHostAPCfg;
    		
			int needRadius=0, wpaMode, psk;
			char topology_file[32]={0};
			FILE *topo_file;
			sprintf(topology_file,"/var/topology.conf.%s",vapStr);
			if((topo_file=fopen(topology_file,"w")))
				{
					fprintf(topo_file,"bridge none\n");					
					fprintf(topo_file,"{\n");
					fprintf(topo_file,"}\n");
			if(apCfgVlanModeGet())
					fprintf(topo_file,"bridge br%d\n",apCfgVlanPvidGet(unit,vap));
			else
					fprintf(topo_file,"bridge br0\n");
					fprintf(topo_file,"{\n");
					fprintf(topo_file,"    interface %s\n",vapStr);
					fprintf(topo_file,"    interface eth0\n");
					fprintf(topo_file,"}\n");
					fprintf(topo_file,"radio wifi0\n");
					fprintf(topo_file,"{\n");
					fprintf(topo_file,"    ap \n");
					fprintf(topo_file,"				{\n");
					fprintf(topo_file,"        config /etc/wpa2/80211g.ap_radio\n");
					fprintf(topo_file,"        bss %s\n",vapStr);
					fprintf(topo_file,"        {\n");
					fprintf(topo_file,"        config /var/hostap.cfg.%s\n",vapStr);
					fprintf(topo_file,"        }\n");
					fprintf(topo_file,"            #couldbe# sta ath1\n");
					fprintf(topo_file,"        {\n");
					fprintf(topo_file,"        config /etc/wpa2/config1.sta\n");
					fprintf(topo_file,"        }\n");
					fprintf(topo_file,"}\n");
					fclose(topo_file);
					}
    		/*Build HostAPd configure file*/
			fHostAPCfg = fopen(hostApCfgName,"w+");
/*  Raul commit out
			fprintf(fHostAPCfg,"interface=%s\n",vapStr);
			
			if(apCfgVlanModeGet())
			    fprintf(fHostAPCfg, "bridge=br%d\n", apCfgVlanPvidGet(unit,vap));
			else    
			    fprintf(fHostAPCfg,"bridge=br0\n");
			fprintf(fHostAPCfg,"driver=madwifi\n");
*/ 
#ifdef   __ICONV_SUPPORT__     
    //add carole
   			 pSrc=apCfgSsidGet(unit,vap);
			 ret=do_convert(UTF2LAN, pSrc, strlen(pSrc), converted_text, 128);
			if(ret!=-1){
				if(strlen(converted_text)<128)
					strcpy(pSrc, converted_text);
		
			}
			fprintf(fHostAPCfg,"ssid=%s\n",pSrc);
#else   	
	
			fprintf(fHostAPCfg,"ssid=%s\n",apCfgSsidGet(unit,vap));
#endif		
#ifdef T_DEBUG
			fprintf(fHostAPCfg,"logger_syslog=-1\n");
			fprintf(fHostAPCfg,"logger_syslog_level=0\n");
			fprintf(fHostAPCfg,"logger_stdout=-1\n");
			fprintf(fHostAPCfg,"logger_stdout_level=0\n");
			fprintf(fHostAPCfg,"debug=4\n");
#else
			fprintf(fHostAPCfg,"logger_syslog=0\n");
			fprintf(fHostAPCfg,"logger_syslog_level=4\n");
			fprintf(fHostAPCfg,"logger_stdout=0\n");
			fprintf(fHostAPCfg,"logger_stdout_level=4\n");
			fprintf(fHostAPCfg,"debug=0\n");
#endif
			fprintf(fHostAPCfg,"ctrl_interface=/var/run/hostapd.%s\n",vapStr); 
			fprintf(fHostAPCfg,"ctrl_interface_group=0\n");
			if(apCfgVlanModeGet())
				fprintf(fHostAPCfg,"iapp_interface=br%d\n",apCfgManagementVlanIdGet());
			else
				fprintf(fHostAPCfg,"iapp_interface=br0\n");

			switch(authType)
			{
			    case APCFG_AUTH_WPA:
			    	needRadius = 1;
			    	wpaMode = 1;
			    	psk = 0;
			    	break;
			    case APCFG_AUTH_WPAPSK:
			    	needRadius = 0;
			    	wpaMode = 1;
			    	psk = 1;
			    	break;
			    case APCFG_AUTH_WPA2:
			    	needRadius = 1;
			    	wpaMode = 2;
			    	psk = 0;
			    	break;
			    case APCFG_AUTH_WPA2PSK:
			    	needRadius = 0;
			    	wpaMode = 2;
			    	psk = 1;
			    	break;
			    case APCFG_AUTH_WPA_AUTO:
			    	needRadius = 1;
			    	wpaMode = 3;
			    	psk = 0;
			    	break;
			    case APCFG_AUTH_WPA_AUTO_PSK:
			    	needRadius = 0;
			    	wpaMode = 3;
			    	psk = 1;
			    	break;
			    case APCFG_AUTH_DOT1X:
			    	needRadius = 1;
			    	wpaMode = -1;
			    	psk = 0;
			    	break;
			    default:
			    	needRadius = 0;
			    	wpaMode = -1;
			    	psk = 0;
			        break;
			}
			if(wpaMode != -1)
			{
				fprintf(fHostAPCfg,"wpa=%d\n",wpaMode);
    			if (psk)
    			{
    			    if(strlen(apCfgPassphraseGet(unit,vap))== 0){
    				    syslog(LOG_ERR,"[SC][systme_error][%s]No WPA PSK.", apCfgSsidGet(unit,vap));
    				}
    				
    				if(strlen(apCfgPassphraseGet(unit,vap))<64)
    				fprintf(fHostAPCfg,"wpa_passphrase=%s\n",apCfgPassphraseGet(unit,vap));
						else
    				fprintf(fHostAPCfg,"wpa_psk=%s\n",apCfgPassphraseGet(unit,vap));
    				fprintf(fHostAPCfg,"wpa_key_mgmt=WPA-PSK\n");    						
    		    }
    		    else
    		    {
    		    	fprintf(fHostAPCfg,"wpa_key_mgmt=WPA-EAP\n");
    		    }
    		    		
    		    if(apCfgWPACipherGet(unit,vap) == WPA_CIPHER_TKIP)
    		    {
    		        fprintf(fHostAPCfg,"wpa_pairwise=TKIP\n");
    		    }
    		    else if(apCfgWPACipherGet(unit,vap) == WPA_CIPHER_AES)
    		    {
    		        fprintf(fHostAPCfg,"wpa_pairwise=CCMP\n");
    		    }
    		    else
    		    {
    		        fprintf(fHostAPCfg,"wpa_pairwise=TKIP CCMP\n");
    		    }
    		    
    		    if(scApCfgGroupKeyUpdateEnabledGet(unit, vap))
    		    	fprintf(fHostAPCfg,"wpa_group_rekey=%d\n", apCfgGroupKeyUpdateIntervalGet(unit, vap));
    		    else
    		        fprintf(fHostAPCfg,"wpa_group_rekey=0\n");
    		    fprintf(fHostAPCfg,"wpa_strict_rekey=%d\n", scApCfgGroupKeyUpdateTerminatedGet(unit, vap)? 1 : 0);
    		    
    		    fprintf(fHostAPCfg,"wpa_gmk_rekey=0\n");
    		    
    		    /* IEEE 802.11i/RSN/WPA2 pre-authentication */
    		    if(wpaMode > 1)
    		    {
        			fprintf(fHostAPCfg,"rsn_preauth=1\n");
        			if(apCfgVlanModeGet())
        			    fprintf(fHostAPCfg, "rsn_preauth_interfaces=br%d\n", apCfgManagementVlanIdGet());
        			else    
        			    fprintf(fHostAPCfg, "rsn_preauth_interfaces=br0\n");
        		}
			}
			
			if(apCfgAclModeGet(unit, vap) == APCFG_ACL_RADIUS)
    		{
    		    fprintf(fHostAPCfg,"macaddr_acl=2\n");
    		    needRadius = 1;
    		}
					
			if (needRadius)
			{
				char dst[32];
				unsigned long ipAddr;
				char *pRadiusServer = apCfgRadiusServerGet(unit,vap);
				char *pAccServer = scApCfgAcctServerGet(unit,vap);
				char *pSecRadiusServer = apCfgBackupRadiusServerGet(unit,vap);
				char *pAccRadiusServer = scApCfgBackupAcctServerGet(unit,vap);
				ipAddr = apCfgIpAddrGet();
				inet_ntop(AF_INET, &ipAddr, dst, 32);
				fprintf(fHostAPCfg,"own_ip_addr=%s\n",dst);
				
				/*Primary servers*/
				if(strlen(pRadiusServer) && strcmp(pRadiusServer,"0.0.0.0")){
				fprintf(fHostAPCfg,"auth_server_addr=%s\n",apCfgRadiusServerGet(unit,vap));
				fprintf(fHostAPCfg,"auth_server_port=%d\n",apCfgRadiusPortGet(unit,vap));
				fprintf(fHostAPCfg,"auth_server_shared_secret=%s\n",apCfgRadiusSecretGet(unit,vap));
				}
				else
				{
				    syslog(LOG_ERR,"[SC][systme_error]No Radius Server.");
				}
				if(strlen(pAccServer) && strcmp(pAccServer,"0.0.0.0")){
				    fprintf(fHostAPCfg,"acct_server_addr=%s\n",scApCfgAcctServerGet(unit,vap));
				    fprintf(fHostAPCfg,"acct_server_port=%d\n",scApCfgAcctPortGet(unit,vap));
				    fprintf(fHostAPCfg,"acct_server_shared_secret=%s\n",scApCfgAcctSecretGet(unit,vap));
				}
				/*backup*/
				if(strlen(pSecRadiusServer) && strcmp(pSecRadiusServer,"0.0.0.0")){
				    fprintf(fHostAPCfg,"auth_server_addr=%s\n",apCfgBackupRadiusServerGet(unit,vap));
				    fprintf(fHostAPCfg,"auth_server_port=%d\n",apCfgBackupRadiusPortGet(unit,vap));
				    fprintf(fHostAPCfg,"auth_server_shared_secret=%s\n",apCfgBackupRadiusSecretGet(unit,vap));
				}
				if(strlen(pAccRadiusServer) && strcmp(pAccRadiusServer,"0.0.0.0")){
				    fprintf(fHostAPCfg,"acct_server_addr=%s\n",scApCfgBackupAcctServerGet(unit,vap));
				    fprintf(fHostAPCfg,"acct_server_port=%d\n",scApCfgBackupAcctPortGet(unit,vap));
				    fprintf(fHostAPCfg,"acct_server_shared_secret=%s\n",scApCfgBackupAcctSecretGet(unit,vap));
				}
				fprintf(fHostAPCfg,"ieee8021x=1\n");

			    if (wpaMode == -1)
			    {
			        if(apCfgDot1xKeyModeGet(unit,vap) == DOT1X_MODE_DYNAMIC)
			        {
    			    	fprintf(fHostAPCfg,"wep_key_len_unicast=%d\n",apCfgDot1xKeyLenGet(unit,vap));
    					fprintf(fHostAPCfg,"wep_key_len_broadcast=%d\n",apCfgDot1xKeyLenGet(unit,vap));
    				    fprintf(fHostAPCfg,"wep_rekey_period=0\n");
    				}
    				else if(apCfgDot1xKeyModeGet(unit,vap) == DOT1X_MODE_STATIC)
    				{
        			    ATH_SCRIPT_ADD(scriptfile, "iwconfig %s key \"[%d]\" %s > /dev/null 2>&1",vapStr,apCfgDefKeyGet(unit,vap),apCfgKeyValGet(unit,vap, apCfgDefKeyGet(unit,vap)));
        			    ATH_SCRIPT_ADD(scriptfile, "iwconfig %s key \"[%d]\" > /dev/null 2>&1",vapStr,apCfgDefKeyGet(unit,vap));
    				    fprintf(fHostAPCfg,"wep_key_len_unicast=0\n");
    					fprintf(fHostAPCfg,"wep_key_len_broadcast=0\n");
    					fprintf(fHostAPCfg,"wep_rekey_period=0\n");
    				}
			    }
			    fprintf(fHostAPCfg,"eap_reauth_period=0\n");
			}
			else
			{
				fprintf(fHostAPCfg,"eapol_version=2\n");
				fprintf(fHostAPCfg,"eapol_key_index_workaround=0\n");
				fprintf(fHostAPCfg,"eap_server=1\n");
				fprintf(fHostAPCfg,"eap_user_file=/etc/wpa2/hostapd.eap_user\n");
			}
			/*****WPS Start*****/
			if(unit==0 && vap == 0)
			{			
				if(needRadius == 0)
				{
					if((apCfgWpsModeGet(unit)==0)){
				    	fprintf(fHostAPCfg,"wps_disable=1\n");
				    	fprintf(fHostAPCfg,"wps_upnp_disable=1\n");						
					}
					else{
				    	fprintf(fHostAPCfg,"wps_disable=0\n");
				    	fprintf(fHostAPCfg,"wps_upnp_disable=0\n");
					}
				    fprintf(fHostAPCfg,"wps_version=0x10\n");
				    fprintf(fHostAPCfg,"wps_auth_type_flags=0x0023\n");
				    fprintf(fHostAPCfg,"wps_encr_type_flags=0x000f\n");
				    fprintf(fHostAPCfg,"wps_conn_type_flags=0x01\n");
				    if(apCfgWpsPinERGet(unit))
				    	fprintf(fHostAPCfg,"wps_config_methods=0x0086\n");//External Registar Enabled
				   	else
				    	fprintf(fHostAPCfg,"wps_config_methods=0x0082\n");			    
					if(apCfgWscConfiguredGet() == 0)//authType == APCFG_AUTH_NONE)
				    	fprintf(fHostAPCfg,"wps_configured=0\n");
					else
				    	fprintf(fHostAPCfg,"wps_configured=1\n");
				    fprintf(fHostAPCfg,"wps_rf_bands=0x03\n");
				    fprintf(fHostAPCfg,"wps_manufacturer=Cisco Small Business\n");
				    fprintf(fHostAPCfg,"wps_model_name=WAP4410N\n");
				    fprintf(fHostAPCfg,"wps_model_number=%s\n",wps_model_number);
				    fprintf(fHostAPCfg,"wps_serial_number=123456789012\n");
				    fprintf(fHostAPCfg,"wps_friendly_name=Wireless-N Access Point with PoE\n");
				    fprintf(fHostAPCfg,"wps_manufacturer_url=http://www.cisco.com/en/US/products/ps10047/index.html\n");
				    fprintf(fHostAPCfg,"wps_model_description=Wireless-N Access Point with PoE\n");
				    fprintf(fHostAPCfg,"wps_model_url=http://%s/index.htm\n",nvram_safe_get("lan_ipaddr"));
				    fprintf(fHostAPCfg,"wps_upc_string=upc string here\n");
				    fprintf(fHostAPCfg,"wps_default_pin=%s\n",apCfgDevicePinGet(unit,vap));
				    fprintf(fHostAPCfg,"wps_dev_category=6\n");
				    fprintf(fHostAPCfg,"wps_dev_sub_category=1\n");
				    fprintf(fHostAPCfg,"wps_dev_oui=0050f204\n");
				    fprintf(fHostAPCfg,"wps_dev_name=WAP4410N\n");
				    fprintf(fHostAPCfg,"wps_os_version=0x00000001\n");
				    fprintf(fHostAPCfg,"wps_atheros_extension=1\n");
			}
			else
			{
			    fprintf(fHostAPCfg,"wps_disable=1\n");
			    fprintf(fHostAPCfg,"wps_upnp_disable=1\n");						
			}
		}
		else
		{
			fprintf(fHostAPCfg,"wps_disable=1\n");
		    fprintf(fHostAPCfg,"wps_upnp_disable=1\n");						
		}

		fclose(fHostAPCfg);

//		    ATH_SCRIPT_ADD(scriptfile, "/sbin/hostapd -u %d -v %d %s &", unit,vap, hostApCfgName);
//        ATH_SCRIPT_ADD(scriptfile, "/bin/ln -sf /tmp/hostapd.%s /sbin/hostapd &",vapStr);
				if(unit == 0 && vap == 0)
				{
        ATH_SCRIPT_ADD(scriptfile, "/usr/bin/killall hostapd");
		    ATH_SCRIPT_ADD(scriptfile, "sleep 2");
        ATH_SCRIPT_ADD(scriptfile, "/sbin/hostapd /var/topology.conf.%s -B -P /var/run/hostapd_pid.%s &",vapStr,vapStr);
				}
				else
				{
        ATH_SCRIPT_ADD(scriptfile, "/usr/bin/killall hostapd.%s",vapStr);
		    ATH_SCRIPT_ADD(scriptfile, "sleep 2");
        ATH_SCRIPT_ADD(scriptfile, "/bin/ln -sf /sbin/hostapd /tmp/hostapd.%s",vapStr);
        ATH_SCRIPT_ADD(scriptfile, "/tmp/hostapd.%s /var/topology.conf.%s -B -P /var/run/hostapd_pid.%s &",vapStr,vapStr,vapStr);        					
					}				

		    ATH_SCRIPT_ADD(scriptfile, "sleep 1");
    		break;
    	}	
	}
#if 1   //def LINUX_WSC    
    if((authType <= APCFG_AUTH_AUTO) && ((vap == 0) || (apCfgAclModeGet(unit, vap) == APCFG_ACL_RADIUS)))
    {
        FILE *fHostAPCfg;
			FILE *topo_file;
			char topology_file[32]={0};			
			sprintf(topology_file,"/var/topology.conf.%s",vapStr);
			if((topo_file=fopen(topology_file,"w")))
				{
					fprintf(topo_file,"bridge none\n");					
					fprintf(topo_file,"{\n");
					fprintf(topo_file,"}\n");
			if(apCfgVlanModeGet())
					fprintf(topo_file,"bridge br%d\n",apCfgVlanPvidGet(unit,vap));
			else
					fprintf(topo_file,"bridge br0\n");
					fprintf(topo_file,"{\n");
					fprintf(topo_file,"    interface %s\n",vapStr);
					fprintf(topo_file,"    interface eth0\n");
					fprintf(topo_file,"}\n");
					fprintf(topo_file,"radio wifi0\n");
					fprintf(topo_file,"{\n");
					fprintf(topo_file,"    ap \n");
					fprintf(topo_file,"				{\n");
					fprintf(topo_file,"        config /etc/wpa2/80211g.ap_radio\n");
					fprintf(topo_file,"        bss %s\n",vapStr);
					fprintf(topo_file,"        {\n");
					fprintf(topo_file,"        config /var/hostap.cfg.%s\n",vapStr);
					fprintf(topo_file,"        }\n");
					fprintf(topo_file,"            #couldbe# sta ath1\n");
					fprintf(topo_file,"        {\n");
					fprintf(topo_file,"        config /etc/wpa2/config1.sta\n");
					fprintf(topo_file,"        }\n");
					fprintf(topo_file,"}\n");
					fclose(topo_file);
					}
        /*Build HostAPd configure file*/
		fHostAPCfg = fopen(hostApCfgName,"w+");
/*  Raul Commit out
		fprintf(fHostAPCfg,"interface=%s\n",vapStr);
		if(apCfgVlanModeGet())
		    fprintf(fHostAPCfg, "bridge=br%d\n", apCfgVlanPvidGet(unit,vap));
		else   
		    fprintf(fHostAPCfg,"bridge=br0\n");
		fprintf(fHostAPCfg,"driver=madwifi\n");
*/		
	#ifdef T_DEBUG
        fprintf(fHostAPCfg,"logger_syslog=-1\n");
        fprintf(fHostAPCfg,"logger_syslog_level=0\n");
        fprintf(fHostAPCfg,"logger_stdout=-1\n");
        fprintf(fHostAPCfg,"logger_stdout_level=0\n");
        fprintf(fHostAPCfg,"debug=4\n");
    #else
        fprintf(fHostAPCfg,"logger_syslog=0\n");
        fprintf(fHostAPCfg,"logger_syslog_level=4\n");
        fprintf(fHostAPCfg,"logger_stdout=0\n");
        fprintf(fHostAPCfg,"logger_stdout_level=4\n");
        fprintf(fHostAPCfg,"debug=0\n");
    #endif
			fprintf(fHostAPCfg,"ctrl_interface=/var/run/hostapd.%s\n",vapStr);
			fprintf(fHostAPCfg,"ctrl_interface_group=0\n");       
			
			if(apCfgVlanModeGet())
				fprintf(fHostAPCfg,"iapp_interface=br%d\n",apCfgManagementVlanIdGet());
			else
				fprintf(fHostAPCfg,"iapp_interface=br0\n");			 
	#ifdef   __ICONV_SUPPORT__     
    //add carole
   			 pSrc=apCfgSsidGet(unit,vap);
			 ret=do_convert(UTF2LAN, pSrc, strlen(pSrc), converted_text, 128);
			if(ret!=-1){
				if(strlen(converted_text)<128)
					strcpy(pSrc, converted_text);
		
			}
			fprintf(fHostAPCfg,"ssid=%s\n",pSrc);
#else   			
			fprintf(fHostAPCfg,"ssid=%s\n",apCfgSsidGet(unit,vap));
#endif
		if(apCfgAclModeGet(unit, vap) == APCFG_ACL_RADIUS)
		{
		    char dst[32];
			unsigned long ipAddr;
			char *pRadiusServer = apCfgRadiusServerGet(unit,vap);
			char *pAccServer = scApCfgAcctServerGet(unit,vap);
			char *pSecRadiusServer = apCfgBackupRadiusServerGet(unit,vap);
			char *pAccRadiusServer = scApCfgBackupAcctServerGet(unit,vap);
			
		    fprintf(fHostAPCfg,"macaddr_acl=2\n");
			ipAddr = apCfgIpAddrGet();
			inet_ntop(AF_INET, &ipAddr, dst, 32);
			fprintf(fHostAPCfg,"own_ip_addr=%s\n",dst);
			
			/*Primary servers*/
			if(strlen(pRadiusServer) && strcmp(pRadiusServer,"0.0.0.0")){
			fprintf(fHostAPCfg,"auth_server_addr=%s\n",apCfgRadiusServerGet(unit,vap));
			fprintf(fHostAPCfg,"auth_server_port=%d\n",apCfgRadiusPortGet(unit,vap));
			fprintf(fHostAPCfg,"auth_server_shared_secret=%s\n",apCfgRadiusSecretGet(unit,vap));
			}
			if(strlen(pAccServer) && strcmp(pAccServer,"0.0.0.0")){
			    fprintf(fHostAPCfg,"acct_server_addr=%s\n",scApCfgAcctServerGet(unit,vap));
			    fprintf(fHostAPCfg,"acct_server_port=%d\n",scApCfgAcctPortGet(unit,vap));
			    fprintf(fHostAPCfg,"acct_server_shared_secret=%s\n",scApCfgAcctSecretGet(unit,vap));
			}
			/*backup*/
			if(strlen(pSecRadiusServer) && strcmp(pSecRadiusServer,"0.0.0.0")){
			    fprintf(fHostAPCfg,"auth_server_addr=%s\n",apCfgBackupRadiusServerGet(unit,vap));
			    fprintf(fHostAPCfg,"auth_server_port=%d\n",apCfgBackupRadiusPortGet(unit,vap));
			    fprintf(fHostAPCfg,"auth_server_shared_secret=%s\n",apCfgBackupRadiusSecretGet(unit,vap));
			}
			if(strlen(pAccRadiusServer) && strcmp(pAccRadiusServer,"0.0.0.0")){
			    fprintf(fHostAPCfg,"acct_server_addr=%s\n",scApCfgBackupAcctServerGet(unit,vap));
			    fprintf(fHostAPCfg,"acct_server_port=%d\n",scApCfgBackupAcctPortGet(unit,vap));
			    fprintf(fHostAPCfg,"acct_server_shared_secret=%s\n",scApCfgBackupAcctSecretGet(unit,vap));
			}
		}
		if(authType == APCFG_AUTH_OPEN_SYSTEM)
		    fprintf(fHostAPCfg,"auth_algs=1\n");
		else if(authType == APCFG_AUTH_SHARED_KEY)
		    fprintf(fHostAPCfg,"auth_algs=2\n");
		else if(authType == APCFG_AUTH_AUTO)
		    fprintf(fHostAPCfg,"auth_algs=3\n");
		else
		    fprintf(fHostAPCfg,"auth_algs=1\n");
		            
        fprintf(fHostAPCfg,"wpa=0\n");
		if(apCfgAclModeGet(unit, vap) == APCFG_ACL_RADIUS)
				fprintf(fHostAPCfg,"ieee8021x=1\n");
		else{
				fprintf(fHostAPCfg,"eapol_version=2\n");
				fprintf(fHostAPCfg,"eapol_key_index_workaround=0\n");
				fprintf(fHostAPCfg,"eap_server=1\n");
				fprintf(fHostAPCfg,"eap_user_file=/etc/wpa2/hostapd.eap_user\n");
		}
		/*****WPS Start*****/
		if(unit==0 && vap == 0){
			if(apCfgAclModeGet(unit, vap) == APCFG_ACL_RADIUS || apCfgWpsModeGet(unit)==0)
			{
			    fprintf(fHostAPCfg,"wps_disable=1\n");
			    fprintf(fHostAPCfg,"wps_upnp_disable=1\n");				
			}
			else{
			    fprintf(fHostAPCfg,"wps_disable=0\n");
			    fprintf(fHostAPCfg,"wps_upnp_disable=0\n");
			    fprintf(fHostAPCfg,"wps_version=0x10\n");
			    fprintf(fHostAPCfg,"wps_auth_type_flags=0x0023\n");
			    fprintf(fHostAPCfg,"wps_encr_type_flags=0x000f\n");
			    fprintf(fHostAPCfg,"wps_conn_type_flags=0x01\n");
			    if(apCfgWpsPinERGet(unit))
			    	fprintf(fHostAPCfg,"wps_config_methods=0x0086\n");//External Registar Enabled
			   	else
			    	fprintf(fHostAPCfg,"wps_config_methods=0x0082\n");
				if(apCfgWscConfiguredGet() == 0)//authType == APCFG_AUTH_NONE)
					fprintf(fHostAPCfg,"wps_configured=0\n");
				else			    
			    	fprintf(fHostAPCfg,"wps_configured=1\n");
			    fprintf(fHostAPCfg,"wps_rf_bands=0x03\n");
			    fprintf(fHostAPCfg,"wps_manufacturer=Cisco Small Business\n");
			    fprintf(fHostAPCfg,"wps_model_name=WAP4410N\n");
			    fprintf(fHostAPCfg,"wps_model_number=%s\n",wps_model_number);
			    fprintf(fHostAPCfg,"wps_serial_number=123456789012\n");
			    fprintf(fHostAPCfg,"wps_friendly_name=Wireless-N Access Point with PoE\n");
			    fprintf(fHostAPCfg,"wps_manufacturer_url=http://www.cisco.com/en/US/products/ps10047/index.html\n");
			    fprintf(fHostAPCfg,"wps_model_description=Wireless-N Access Point with PoE\n");
			    fprintf(fHostAPCfg,"wps_model_url=http://%s/index.htm\n",nvram_safe_get("lan_ipaddr"));
			    fprintf(fHostAPCfg,"wps_upc_string=upc string here\n");
			    fprintf(fHostAPCfg,"wps_default_pin=%s\n",apCfgDevicePinGet(unit,vap));
			    fprintf(fHostAPCfg,"wps_dev_category=6\n");
			    fprintf(fHostAPCfg,"wps_dev_sub_category=1\n");
			    fprintf(fHostAPCfg,"wps_dev_oui=0050f204\n");
			    fprintf(fHostAPCfg,"wps_dev_name=WAP4410N\n");
			    fprintf(fHostAPCfg,"wps_os_version=0x00000001\n");
			    fprintf(fHostAPCfg,"wps_atheros_extension=1\n");
			}
		}
		else{
			fprintf(fHostAPCfg,"wps_disable=1\n");
			fprintf(fHostAPCfg,"wps_upnp_disable=1\n");							
		}		

        fclose(fHostAPCfg);
//        ATH_SCRIPT_ADD(scriptfile, "/sbin/hostapd -u %d -v %d %s &",unit,vap ,hostApCfgName);
//        ATH_SCRIPT_ADD(scriptfile, "/sbin/hostapd /etc/wpa2/topology.conf.%s &",vapStr);

		if(unit == 0 && vap == 0){
        	ATH_SCRIPT_ADD(scriptfile, "/usr/bin/killall hostapd");
		    ATH_SCRIPT_ADD(scriptfile, "sleep 2");
        	ATH_SCRIPT_ADD(scriptfile, "/sbin/hostapd /var/topology.conf.%s -B -P /var/run/hostapd_pid.%s &",vapStr,vapStr);
		}
		else{
        	ATH_SCRIPT_ADD(scriptfile, "/usr/bin/killall hostapd.%s",vapStr);
		    ATH_SCRIPT_ADD(scriptfile, "sleep 2");
        	ATH_SCRIPT_ADD(scriptfile, "/bin/ln -sf /sbin/hostapd /tmp/hostapd.%s",vapStr);
        	ATH_SCRIPT_ADD(scriptfile, "/tmp/hostapd.%s /var/topology.conf.%s -B -P /var/run/hostapd_pid.%s &",vapStr,vapStr,vapStr);        					
		}
					
					
        ATH_SCRIPT_ADD(scriptfile, "sleep 1");
	    if(authType != APCFG_AUTH_NONE)
	    {
    		authmode = (authType==APCFG_AUTH_AUTO)? 4 : authType;
    
    	    ATH_SCRIPT_ADD(scriptfile, "iwpriv %s authmode %d > /dev/null 2>&1",vapStr, authmode);
    	    ATH_SCRIPT_ADD(scriptfile, "iwpriv %s privacy 1 > /dev/null 2>&1",vapStr);
    
    	    if(apCfgKeyEntryMethodGet(unit,vap) == KEY_ENTRY_METHOD_HEX)
    		{
    		    ATH_SCRIPT_ADD(scriptfile, "iwconfig %s key \"[%d]\" %s > /dev/null 2>&1",vapStr,apCfgDefKeyGet(unit,vap),apCfgKeyValGet(unit,vap, apCfgDefKeyGet(unit,vap)));
    		}
    		else
    		{
    		    ATH_SCRIPT_ADD(scriptfile, "iwconfig %s key \"[%d]\" s:%s > /dev/null 2>&1",vapStr,apCfgDefKeyGet(unit,vap),apCfgKeyValGet(unit,vap, apCfgDefKeyGet(unit,vap)));
    		}
            ATH_SCRIPT_ADD(scriptfile, "iwconfig %s key \"[%d]\" > /dev/null 2>&1",vapStr,apCfgDefKeyGet(unit,vap));
    	}

    }
#endif
    ATH_SCRIPT_ADD(scriptfile, "ifconfig %s up > /dev/null 2>&1", vapStr);
}


void wlanNoApSecurityApply(char *scriptfile, int unit, int vap_id)
{
	int authType, authmode, vap;
	char vapStr[32];
    char vapSsid[64+1];
    /*We use the first vap's security for the WDS connection*/
#ifdef   __ICONV_SUPPORT__    
	 char converted_text[128] = {0};
	 int ret=0;
#endif  
  vap = 0;
        
	sprintf(vapStr,"ath%d%d", unit, vap_id);

    //Set SSID
	scConversionSSID(apCfgSsidGet(unit,vap), vapSsid);
#ifdef   __ICONV_SUPPORT__   
	//carole
	 ret=do_convert(UTF2LAN, vapSsid, strlen(vapSsid), converted_text, 128);
	if(ret!=-1){
		if(strlen(converted_text)<128)		
			strncpy(vapSsid, converted_text, MAX_SSID);
	}
#endif

	ATH_SCRIPT_ADD(scriptfile, "iwconfig %s essid \"%s\" > /dev/null 2>&1", vapStr, vapSsid);
    //Broadcast SSID
	ATH_SCRIPT_ADD(scriptfile, "iwpriv %s hide_ssid %d > /dev/null 2>&1", vapStr, apCfgSsidModeGet(unit,vap));
    
    /*
    * Config security
    */
	authType = apCfgAuthTypeGet(unit,vap);

	switch(authType)
	{
		case APCFG_AUTH_NONE:
			authmode = 1;
			ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/iwconfig %s key off",vapStr);
			ATH_SCRIPT_ADD(scriptfile, "iwpriv %s authmode %d > /dev/null 2>&1",vapStr, authmode);
		    ATH_SCRIPT_ADD(scriptfile, "iwpriv %s privacy 0 > /dev/null 2>&1",vapStr); 
			break;
		case APCFG_AUTH_OPEN_SYSTEM:
    	case APCFG_AUTH_SHARED_KEY:
    	case APCFG_AUTH_AUTO:
    		authmode = (authType==APCFG_AUTH_AUTO)? 4 : authType;

		    ATH_SCRIPT_ADD(scriptfile, "iwpriv %s authmode %d > /dev/null 2>&1",vapStr, authmode);
		    ATH_SCRIPT_ADD(scriptfile, "iwpriv %s privacy 1 > /dev/null 2>&1",vapStr);
			
			if(apCfgKeyEntryMethodGet(unit,vap) == KEY_ENTRY_METHOD_HEX)
			{
			    if(strlen(apCfgKeyValGet(unit,vap, apCfgDefKeyGet(unit,vap)))==0){
			        syslog(LOG_ERR,"[SC][systme_error] [%s] No WEP Key.", apCfgSsidGet(unit,vap));
			    }
			    ATH_SCRIPT_ADD(scriptfile, "iwconfig %s key \"[%d]\" %s > /dev/null 2>&1",vapStr,apCfgDefKeyGet(unit,vap),apCfgKeyValGet(unit,vap, apCfgDefKeyGet(unit,vap)));
			}
			else
			{
			    if(strlen(apCfgKeyValGet(unit,vap, apCfgDefKeyGet(unit,vap)))==0){
			        syslog(LOG_ERR,"[SC][systme_error] [%s] No WEP Key.", apCfgSsidGet(unit,vap));
			    }
			    ATH_SCRIPT_ADD(scriptfile, "iwconfig %s key \"[%d]\" s:%s > /dev/null 2>&1",vapStr,apCfgDefKeyGet(unit,vap),apCfgKeyValGet(unit,vap, apCfgDefKeyGet(unit,vap)));
			}
            ATH_SCRIPT_ADD(scriptfile, "iwconfig %s key \"[%d]\" > /dev/null 2>&1",vapStr,apCfgDefKeyGet(unit,vap));
			break;
   		case APCFG_AUTH_WPAPSK:
    	case APCFG_AUTH_WPA2PSK:
		case APCFG_AUTH_WPA:
    	case APCFG_AUTH_WPA2:
    	case APCFG_AUTH_WPA_AUTO:
    	case APCFG_AUTH_WPA_AUTO_PSK:
    	case APCFG_AUTH_DOT1X:	
    	    //Client mode didn't support WPA auto mode or 802.1x
    	default:
    	    authmode = 1;
			ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/iwconfig %s key off",vapStr);
			ATH_SCRIPT_ADD(scriptfile, "iwpriv %s authmode %d > /dev/null 2>&1",vapStr, authmode);
		    ATH_SCRIPT_ADD(scriptfile, "iwpriv %s privacy 0 > /dev/null 2>&1",vapStr); 
		    
    	    break;	
	}
	ATH_SCRIPT_ADD(scriptfile, "ifconfig %s up > /dev/null 2>&1", vapStr);
	
}

void wlanKillVap(char *scriptfile, int unit, int vap)
{
    FILE *fp;
    char vapStr[32], fileName[32], hostApCfgName[32], hostPidFileName[32];
    char line[64+1];
    
	sprintf(vapStr,"ath%d%d",unit,vap);
    
    /*Remove from Bridge*/
    sprintf(fileName, "%s.%s", ATH_DELIF, vapStr);
    if(access(fileName,F_OK)==0)
    {    
        ATH_SCRIPT_ADD(scriptfile, "chmod +x %s", fileName);
        ATH_SCRIPT_ADD(scriptfile, "%s", fileName);
        ATH_SCRIPT_ADD(scriptfile, "rm -f %s", fileName);
        ATH_SCRIPT_ADD(scriptfile, "sleep 1");
         
    }
    /*Bring the interface down*/    
    ATH_SCRIPT_ADD(scriptfile, "ifconfig %s down > /dev/null 2>&1", vapStr);
    ATH_SCRIPT_ADD(scriptfile, "sleep 1");
	ATH_SCRIPT_ADD(scriptfile, "wlanconfig %s destroy > /dev/null 2>&1", vapStr);
	
	/*
	* Added BZ@SC_CPUAP  at 2007-8-10
	* Must sleep 3 or 4 seconds, or the remained actived vap may not work normally.
	*/
	ATH_SCRIPT_ADD(scriptfile, "sleep 3");
	
	/*Kill hostapd*/
	sprintf(hostApCfgName,"%s.%s",HOSTAP_CFG, vapStr);
	sprintf(hostPidFileName, "/tmp/hostapd%s", vapStr);
	SYSTEM("ps | grep %s | cut -b 1-5 > %s", hostApCfgName, hostPidFileName);
	while((fp=fopen(hostPidFileName, "r"))==NULL);
    while(fgets(line, 64, fp)!=NULL){
        line[5] = 0;
		ATH_SCRIPT_ADD(scriptfile, "kill -9 %s > /dev/null 2>&1", line);     
    }   
    fclose(fp);
    unlink(hostPidFileName);
	
	/*Kill wsc upnp*/
#ifdef WSC_UPNP
    if(vap == 0){
        ATH_SCRIPT_ADD(scriptfile, "/usr/bin/killall wscupnpd > /dev/null 2>&1");
    	ATH_SCRIPT_ADD(scriptfile, "/usr/bin/killall -9 wscupnpd > /dev/null 2>&1");
    }
#endif

}

void wlanBridgeApply(char *scriptfile, int unit, int bss, int wds)
{
    char vapStr[32];
	int vap;
	int pri;
	FILE *fp;
	
	vap = bss;    
    if (bss == WLAN_MAX_VAP)
        vap = 0;
    
    sprintf(vapStr,"ath%d%d", unit, bss);
	
	fp=fopen("/tmp/vlan.conf","r");
	
    if(fp != NULL)
    {
        if(wds)
        {
            char line[64+1];
            int vlanId = 0;
            
            /*get out the native vlan*/
            fgets(line, 64, fp);
            sscanf(line,"%d %*d", &vlanId);
            
            ATH_SCRIPT_ADD(scriptfile, "echo 1 %d %d %d > /proc/wds_vlan_cfb", vlanId, apCfgWdsVlanTagGet(), apCfgVlanPvidGet(unit, vap));
        		    
        	fgets(line, 64, fp); /* eat management vlan id */
            if(apCfgWdsVlanTagGet()){
        		        
                while(fgets(line, 64, fp)!=NULL){
                    
                    vlanId = atoi(line);
                    
                    if(wds == 2 && apCfgVlanPvidGet(unit, vap) == vlanId)
                        continue;
                    ATH_SCRIPT_ADD(scriptfile, "vconfig add %s %d",vapStr, vlanId);
                    for (pri=0; pri<8; pri++){
                        ATH_SCRIPT_ADD(scriptfile, "vconfig set_ingress_map %s.%d %d %d",vapStr,vlanId,pri,pri);
                	    ATH_SCRIPT_ADD(scriptfile, "vconfig set_egress_map %s.%d %d %d",vapStr, vlanId,pri,pri);
                    }
        	        ATH_SCRIPT_ADD(scriptfile, "ifconfig %s.%d up",vapStr, vlanId);
        	        ATH_SCRIPT_ADD(scriptfile, "brctl addif br%d %s.%d",vlanId, vapStr, vlanId);
                
                    ATH_SCRIPT_ADD(scriptfile, "echo brctl delif br%d %s.%d >> %s.%s", vlanId, vapStr, vlanId, 
                                                                                    ATH_DELIF, vapStr);
                    ATH_SCRIPT_ADD(scriptfile, "echo vconfig rem %s.%d >> %s.%s", vapStr, vlanId, 
                                                                                    ATH_DELIF, vapStr);
                    ATH_SCRIPT_ADD(scriptfile, "brctl setpathcost br%d %s.%d %d > /dev/null 2>&1", vlanId, vapStr, vlanId, 50);                                                                
    		        ATH_SCRIPT_ADD(scriptfile, "brctl setportprio br%d %s.%d %d > /dev/null 2>&1", vlanId, vapStr, vlanId, 100);
    		        if(apCfgInterVapForwardingGet(RADIO_24G))
                        ATH_SCRIPT_ADD(scriptfile, "brctl intervf br%d enable", vlanId);
                    else
                        ATH_SCRIPT_ADD(scriptfile, "brctl intervf br%d disable", vlanId);
        	    }
            }
        }
        
        if((wds == 0 || wds == 2) || !apCfgWdsVlanTagGet())
        {
            
            ATH_SCRIPT_ADD(scriptfile, "vconfig add %s %d", vapStr, apCfgVlanPvidGet(unit, vap));
            /* Add by archer for Vlan Priority */
            ATH_SCRIPT_ADD(scriptfile, "iwpriv %s priority %d",vapStr, apCfgPriorityGet(unit, vap));
            /* Add end */
            for (pri=0; pri<8; pri++){
                ATH_SCRIPT_ADD(scriptfile, "vconfig set_ingress_map %s.%d %d %d",vapStr,apCfgVlanPvidGet(unit, vap),pri,pri);
                ATH_SCRIPT_ADD(scriptfile, "vconfig set_egress_map %s.%d %d %d",vapStr, apCfgVlanPvidGet(unit, vap),pri,pri);
            }
            ATH_SCRIPT_ADD(scriptfile, "ifconfig %s.%d up",vapStr, apCfgVlanPvidGet(unit, vap));
            ATH_SCRIPT_ADD(scriptfile, "brctl addif br%d %s.%d",apCfgVlanPvidGet(unit, vap), vapStr, apCfgVlanPvidGet(unit, vap));
        	
        	ATH_SCRIPT_ADD(scriptfile, "echo brctl delif br%d %s.%d >> %s.%s", apCfgVlanPvidGet(unit, vap), vapStr, apCfgVlanPvidGet(unit, vap), ATH_DELIF, vapStr);
        	ATH_SCRIPT_ADD(scriptfile, "echo vconfig rem %s.%d >> %s.%s", vapStr, apCfgVlanPvidGet(unit, vap), ATH_DELIF, vapStr);
        	
        	ATH_SCRIPT_ADD(scriptfile, "brctl setpathcost br%d %s.%d %d > /dev/null 2>&1", apCfgVlanPvidGet(unit, vap), vapStr, apCfgVlanPvidGet(unit, vap), 50);
    		ATH_SCRIPT_ADD(scriptfile, "brctl setportprio br%d %s.%d %d > /dev/null 2>&1", apCfgVlanPvidGet(unit, vap), vapStr, apCfgVlanPvidGet(unit, vap), 100);
    		
    		if(apCfgInterVapForwardingGet(RADIO_24G))
                ATH_SCRIPT_ADD(scriptfile, "brctl intervf br%d enable", apCfgVlanPvidGet(unit, vap));
            else
                ATH_SCRIPT_ADD(scriptfile, "brctl intervf br%d disable", apCfgVlanPvidGet(unit, vap));
        }
        fclose(fp);
    }
    else
    {
        ATH_SCRIPT_ADD(scriptfile, "brctl addif br0 %s > /dev/null 2>&1",vapStr);
        ATH_SCRIPT_ADD(scriptfile, "echo brctl delif br0 %s >> %s.%s", vapStr, ATH_DELIF,vapStr);
        ATH_SCRIPT_ADD(scriptfile, "brctl setpathcost br0 %s %d > /dev/null 2>&1", vapStr, 50);
	    ATH_SCRIPT_ADD(scriptfile, "brctl setportprio br0 %s %d > /dev/null 2>&1", vapStr, 100);
	    if(apCfgInterVapForwardingGet(RADIO_24G))
            ATH_SCRIPT_ADD(scriptfile, "brctl intervf br0 enable");
        else
            ATH_SCRIPT_ADD(scriptfile, "brctl intervf br0 disable");
    }
}
void wlanKillAllVap(char *scriptfile)
{
    FILE *fp;
    char line[64+1];
    char fileName[32];
            
    /*
    * List All vaps
    */
    system("iwconfig | grep ath | cut -b 1-5 > /tmp/vaplist.tmp");  
    while((fp=fopen("/tmp/vaplist.tmp","r"))==NULL);
    while(fgets(line, 64, fp)!=NULL){
        line[5] = 0;
        /*Remove from Bridge*/
        sprintf(fileName, "%s.%s", ATH_DELIF, line);
        if(access(fileName,F_OK)==0)
        {    
            ATH_SCRIPT_ADD(scriptfile, "chmod +x %s", fileName);
            ATH_SCRIPT_ADD(scriptfile, "%s", fileName);
            ATH_SCRIPT_ADD(scriptfile, "rm -f %s", fileName);
            ATH_SCRIPT_ADD(scriptfile, "sleep 1");
             
        }
        /*Bring the interface down*/    
        ATH_SCRIPT_ADD(scriptfile, "ifconfig %s down > /dev/null 2>&1", line);
        ATH_SCRIPT_ADD(scriptfile, "sleep 2");
		ATH_SCRIPT_ADD(scriptfile, "wlanconfig %s destroy > /dev/null 2>&1", line);     
    }   
    fclose(fp);
    unlink("/tmp/vaplist.tmp");
	
	/*Killall instances of hostapd and wsc*/
	ATH_SCRIPT_ADD(scriptfile, "killall hostapd > /dev/null 2>&1");
	ATH_SCRIPT_ADD(scriptfile, "killall hostapd.ath01 > /dev/null 2>&1");
	ATH_SCRIPT_ADD(scriptfile, "killall hostapd.ath02 > /dev/null 2>&1");
	ATH_SCRIPT_ADD(scriptfile, "killall hostapd.ath03 > /dev/null 2>&1");
	ATH_SCRIPT_ADD(scriptfile, "killall -9 hostapd > /dev/null 2>&1");
	ATH_SCRIPT_ADD(scriptfile, "killall -9 hostapd.ath01 > /dev/null 2>&1");
	ATH_SCRIPT_ADD(scriptfile, "killall -9 hostapd.ath02 > /dev/null 2>&1");
	ATH_SCRIPT_ADD(scriptfile, "killall -9 hostapd.ath03 > /dev/null 2>&1");
#ifdef WSC_UPNP
    ATH_SCRIPT_ADD(scriptfile, "/usr/bin/killall wscupnpd > /dev/null 2>&1");
	ATH_SCRIPT_ADD(scriptfile, "/usr/bin/killall -9 wscupnpd > /dev/null 2>&1");
#endif

}

int wlanModuleLoad(char *scriptfile, int unit)
{
    unsigned int countryid;


    
    countryid = apCfgCountryCodeGet();
    countryid = countryid > 1000?countryid-1000:countryid;
		if(countryid == 392)
    ATH_SCRIPT_ADD(scriptfile, "/etc/ath/ath_MOD_load_JP");
		else
    ATH_SCRIPT_ADD(scriptfile, "/etc/ath/ath_MOD_load");
    
    
    if(countryid != CTRY_DEFAULT )
    {
        ATH_SCRIPT_ADD(scriptfile, "iwpriv wifi%d setCountryID %d",unit, countryid);
    }
    
    /*Config HW mac etc.*/
{    
    if_info_t if_info;
    unsigned char macStr[18];
    int i;
    
    getMgtBrInfo(&if_info);

#ifdef LINUX_WSC
    scMacStr17ToStr12(if_info.mac, macStr);
    SYSTEM("echo %s > /tmp/wsc_mac", macStr);
#endif
    
#ifdef LINKSYS
    /*plus the mac by one*/
     scHexs2Chars(if_info.mac, macStr, 6, 1);
     for(i=5; i>=0 && !++macStr[i]; i--);
     macAddrToString(macStr, if_info.mac, NULL);
     
#endif    
    ATH_SCRIPT_ADD(scriptfile, "/sbin/ifconfig wifi0 hw ether %s",  if_info.mac);
    ATH_SCRIPT_ADD(scriptfile, "/sbin/ifconfig wifi0 txqueuelen %d", 1000);

}

    /*Start Load balance*/
    ATH_SCRIPT_ADD(scriptfile, "/usr/sbin/rc balance restart&");
    
    return 0;
}

int wlanStart(char *scriptfile, int unit)
{
    char WLAN_UP[ATH_UP_MAX][30];
    char vapStr[VAP_STR_LEN];
    int step;
    int vap;
    
    /*Init and Creat wlan up script file*/
    for(step = 0; step < ATH_UP_MAX; step++){
        sprintf(WLAN_UP[step], ATH_UP, unit, step);
        ATH_SCRIPT_DEL(WLAN_UP[step]);
	    ATH_SCRIPT_ADD(WLAN_UP[step], "#!/bin/sh\n");
    }
    ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_LAST], "sleep 1");
    
    /*
    * insert the wifi modules and config it
    */     
    wlanModuleLoad(WLAN_UP[ATH_UP_PRE], unit);
    
    /*
    * Channel configuration: creat a vap for channel, then remove it
    */
    if(access("/tmp/chan_config",F_OK)==0 || access("/tmp/chan_list",F_OK)!=0)
    {
        int wirelessMode = apCfgFreqSpecGet(unit);
        
        /*Turn off the channel config flag*/
        system("rm -f /tmp/chan_config");
        
        /*Make vap to config channel*/
        sprintf(vapStr,"ath09");
        if((apCfgOpModeGet(unit) == CFG_OP_MODE_PPT) || (apCfgOpModeGet(unit) == CFG_OP_MODE_MPT))
            ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "wlanconfig %s create wlandev wifi%d wlanmode ap nosbeacon > /dev/null 2>&1", vapStr, unit);
        else
            ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "wlanconfig %s create wlandev wifi%d wlanmode ap > /dev/null 2>&1", vapStr, unit);
        
        /*Simple Config for this vap*/
        ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "/usr/sbin/iwconfig %s channel 0",vapStr);
        
        /*wireless mode*/
        //11b
        if(wirelessMode == MODE_SELECT_11B)
        {
            ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "iwpriv %s mode 11b > /dev/null 2>&1", vapStr);
        }
        //11g only/11bg
        else if(wirelessMode == MODE_SELECT_11G || wirelessMode == MODE_SELECT_11BG)
        {
            ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "iwpriv %s mode 11g > /dev/null 2>&1", vapStr);
            ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "iwpriv %s pureg %d > /dev/null 2>&1", vapStr, (wirelessMode == MODE_SELECT_11G)?1:0);
        }
        //11n
        else
        {   
            //Channel Width Mode
            //cwmmode 0 is static 20; cwmmode 1 is dyn 2040; cwmmode 2 is static 40
            if(apCfgChannelWidthModeGet(unit) == 0)
            {
                ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "iwpriv %s mode 11NGHT20 > /dev/null 2>&1", vapStr);
                ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "/usr/sbin/iwpriv %s cwmmode 0", vapStr);
            }else{
                if(apCfgAutoChannelGet(unit) == 0)
                {
                    ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "iwpriv %s extoffset %s > /dev/null 2>&1", vapStr, apCfgChannelOffsetGet(unit));
                    if(strcmp(apCfgChannelOffsetGet(unit), "1")==0)
                        ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "iwpriv %s mode 11NGHT40PLUS > /dev/null 2>&1", vapStr);
                    else
                        ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "iwpriv %s mode 11NGHT40MINUS > /dev/null 2>&1", vapStr);    
                }else{
                    ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "iwpriv %s mode 11NGHT40 > /dev/null 2>&1", vapStr);
                }       
                ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "/usr/sbin/iwpriv %s cwmmode %d", vapStr, apCfgChannelWidthModeGet(unit));
            }
            
            ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "iwpriv wifi%d ForBiasAuto 1", unit);
            ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "iwpriv %s puren %d > /dev/null 2>&1", vapStr,(wirelessMode == MODE_SELECT_11N)?1:0);
            ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "iwpriv %s pureg 0 > /dev/null 2>&1", vapStr);
        }
        
    	ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "iwconfig %s essid \"........\" > /dev/null 2>&1", vapStr);
    	ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "iwpriv %s hide_ssid 1 > /dev/null 2>&1", vapStr);
    	
    	/*If fixed channel, then config the channel*/
        if(apCfgAutoChannelGet(unit) == 0)
        {
            ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "/usr/sbin/iwconfig %s channel 6",vapStr);
            ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "/usr/sbin/iwconfig %s channel %d",vapStr, apCfgRadioChannelGet(unit));
        }
	    /*Bring the interface up at this point*/
    	ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "ifconfig %s up > /dev/null 2>&1",vapStr); 
    	ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "sleep %d", apCfgAutoChannelGet(unit)? 6:2); 
        ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "/usr/sbin/iwconfig %s getCurrentChannel > /tmp/chan_list",vapStr);
        ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "/usr/sbin/iwconfig %s getChannel > /tmp/chan_curr",vapStr);
        ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "/usr/sbin/iwconfig %s getCurrTxPower > /tmp/maxpwr_curr", vapStr);

        ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "ifconfig %s down > /dev/null 2>&1",vapStr);  
        ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "sleep 1"); 
        ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_PRE], "wlanconfig %s destroy > /dev/null 2>&1", vapStr);
    }
    
    if(scriptfile==NULL){
	    ATH_SCRIPT_RUN(WLAN_UP[ATH_UP_PRE]);
	}else{
	    ATH_SCRIPT_ADD(scriptfile, "chmod 777 %s", WLAN_UP[ATH_UP_PRE]);
        ATH_SCRIPT_ADD(scriptfile, "%s", WLAN_UP[ATH_UP_PRE]);
	}
    
    /*
    * Correct Something
    */
    //Correct the channel
    wlanCorrectChannelApply(unit);
    
    /*
    *  Rogue AP mode
    */
    if(apCfgOpModeGet(unit) == CFG_OP_MODE_ROGAP)
    {
        sprintf(vapStr,"ath00");
        ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_MAKE], "wlanconfig %s create wlandev wifi%d wlanmode sta nosbeacon > /dev/null 2>&1", vapStr, unit);
        ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_MAKE], "iwpriv %s bgscan 0 > /dev/null 2>&1", vapStr);
        ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_MAKE], "iwpriv %s doth 0 > /dev/null 2>&1", vapStr);
        ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_MAKE], "iwpriv %s ff 0 > /dev/null 2>&1", vapStr);
        ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_MAKE], "iwconfig %s essid \".......  ........  ......\" > /dev/null 2>&1", vapStr);
        ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_MAKE], "ifconfig %s up > /dev/null 2>&1", vapStr);
        ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_LAST], "/usr/sbin/rc rogueap start&");
    }
    else if(apCfgWlanStateGet(unit) == 1) 
    {  
        /* 
        * Normal AP mode setup 
        */
        if((apCfgOpModeGet(unit) == CFG_OP_MODE_AP) || 
            (apCfgOpModeGet(unit) == CFG_OP_MODE_UR) )
        {
            for(vap=0; vap < WLAN_MAX_VAP; vap++)
            {    
                
                if(apCfgActiveModeGet(unit,vap))
                {   
                    sprintf(vapStr,"ath%d%d",unit,vap);
                    
        		    /*
        		    *Make Vap
        		    */
        		    ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_MAKE], "wlanconfig %s create wlandev wifi%d wlanmode ap > /dev/null 2>&1", vapStr, unit);
	                wlanBasicApply(WLAN_UP[ATH_UP_MAKE], unit, vap, 0, (apCfgOpModeGet(unit) == CFG_OP_MODE_UR)?2:0);
	                wlanAdvanceApply(WLAN_UP[ATH_UP_MAKE], unit, vap, 0);
    	            wlanAclApply(WLAN_UP[ATH_UP_MAKE], unit, vap);
    				
    				/*Configure bridge including stp parameters*/
    				wlanBridgeApply(WLAN_UP[ATH_UP_SUF], unit, vap, 0);
        			
        			/*Config security and Bring the interface up*/
    				wlanSecurityApply(WLAN_UP[ATH_UP_AP], unit, vap);	
    				
        		    /*wsc config start*/
        		    if(vap == 0){
        		        /*wsc config file creat*/
#ifdef LINUX_WSC
        		        wlanCreatWscConfig(unit, vap);
#endif        		        
        		        /*wsc upnp start*/
#ifdef WSC_UPNP
                        ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_LAST], "/usr/sbin/rc wscupnp restart&");
#endif
        		    }
        		}
            }
        }
        /*
        * WDS+AP mode setup: AP_PPT, AP_MPT
        */
        if( (apCfgOpModeGet(unit) == CFG_OP_MODE_AP_PTP) ||
            (apCfgOpModeGet(unit) == CFG_OP_MODE_AP_PTMP))
        {
            vap = 0;
            sprintf(vapStr,"ath%d%d",unit, vap);     
           
            ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_MAKE], "echo 1 > /proc/wds_cfb");

            /*
        	*Make Vap 
        	*/
        	ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_MAKE], "wlanconfig %s create wlandev wifi%d wlanmode ap > /dev/null 2>&1", vapStr, unit);   
            wlanBasicApply(WLAN_UP[ATH_UP_MAKE], unit, vap, 0, 0);
	        wlanAdvanceApply(WLAN_UP[ATH_UP_MAKE], unit, vap, 0);
    	    wlanAclApply(WLAN_UP[ATH_UP_MAKE], unit, vap);
    				
    		/*Configure bridge including stp parameters*/
    		wlanBridgeApply(WLAN_UP[ATH_UP_SUF], unit, vap, 2);
        	
        	/*Config wds ap*/
        	wlanWdsApply(WLAN_UP[ATH_UP_AP], unit, vap);
        			
            /*Config security and Bring the interface up*/
            wlanSecurityApply(WLAN_UP[ATH_UP_AP], unit, vap);	
    				
            /*wsc config start*/
            if(vap == 0){
                /*wsc config file creat*/
#ifdef LINUX_WSC
                wlanCreatWscConfig(unit, vap);
#endif        		        
                /*wsc upnp start*/
#ifdef WSC_UPNP
                ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_LAST], "/usr/sbin/rc wscupnp restart&");
#endif
            }
    		
    		ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_LAST], "/usr/sbin/rc wdspsk start&");
        }
            
        /* 
        * Pure WDS mode setup: PPT, MPT
        */
        if((apCfgOpModeGet(unit) == CFG_OP_MODE_PPT) || 
            (apCfgOpModeGet(unit) == CFG_OP_MODE_MPT))
        {
           
            vap = 0;
            sprintf(vapStr,"ath%d%d",unit, vap);     
           
            ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_MAKE], "echo 1 > /proc/wds_cfb");

            /*
        	*Make Vap 
        	*/   
            ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_MAKE], "wlanconfig %s create wlandev wifi%d wlanmode ap nosbeacon > /dev/null 2>&1", vapStr, unit);
		    
			wlanBasicApply(WLAN_UP[ATH_UP_MAKE], unit, vap, 1, 0);
	        wlanAdvanceApply(WLAN_UP[ATH_UP_MAKE], unit, vap, 1);
    		
    		/*Configure bridge including stp parameters*/
            wlanBridgeApply(WLAN_UP[ATH_UP_SUF], unit, vap, 1);
        	
        	/*Config wds ap*/
        	wlanWdsApply(WLAN_UP[ATH_UP_AP], unit, vap);
        	
        	/*Config security and Bring the interface up*/
    		wlanNoApSecurityApply(WLAN_UP[ATH_UP_AP], unit, vap);
    		ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_AP], "sleep 2");
    		
    		ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_LAST], "/usr/sbin/rc wdspsk start&");
        }
	
	    /*
	    *Universal Client/Repeater setup
        */
        if ( apCfgOpModeGet(unit) == CFG_OP_MODE_UC || apCfgOpModeGet(unit) == CFG_OP_MODE_UR)
        {
            vap = WLAN_MAX_VAP;
            sprintf(vapStr,"ath%d%d",unit, WLAN_MAX_VAP);       //Use the last VAP for the sta vap
            /*
            * insmod the required module
            */
            ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_MAKE], "echo 1 > /proc/us_cfb");
            
            /*
        	*Make sta Vap
        	*/
        	ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_MAKE], "wlanconfig %s create wlandev wifi%d wlanmode sta nosbeacon > /dev/null 2>&1", vapStr, unit);
		        
			wlanBasicApply(WLAN_UP[ATH_UP_MAKE], unit, vap, 1, 1);
	        wlanAdvanceApply(WLAN_UP[ATH_UP_MAKE], unit, vap, 1);
    		
    		/*Configure bridge including stp parameters*/
            wlanBridgeApply(WLAN_UP[ATH_UP_SUF], unit, vap, 0);
    				
        	/*Config security and Bring the interface up*/
    		wlanNoApSecurityApply(WLAN_UP[ATH_UP_STA], unit, vap);
    		ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_STA], "sleep 2");

    		ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_LAST], "/usr/sbin/rc wpasupplicant start&");
        }
    
        /*SNNP trap*/
	    ATH_SCRIPT_ADD(WLAN_UP[ATH_UP_LAST], "/usr/sbin/snmptrap -t 3");
    }

	/*
	* Run the script file
	*/
	for(step = 1; step < ATH_UP_MAX; step++){
	    if(scriptfile==NULL){
	        ATH_SCRIPT_RUN(WLAN_UP[step]);
	    }else{
	        ATH_SCRIPT_ADD(scriptfile, "chmod 777 %s", WLAN_UP[step]);
            ATH_SCRIPT_ADD(scriptfile, "%s", WLAN_UP[step]);
	    }
    }
	//SYSTEM("/usr/sbin/wlan_check &");		
    
    return 0;
}

int wlanStop(char *scriptfile, int unit, int unload_module/*1,0*/)
{
    char WLAN_DOWN[30];
    
 	//system("killall wlan_check > /dev/null 2>&1");    	   
    sprintf(WLAN_DOWN, ATH_DOWN, unit);
    
    ATH_SCRIPT_DEL(WLAN_DOWN);
	ATH_SCRIPT_ADD(WLAN_DOWN, "#!/bin/sh\n");
    
    ATH_SCRIPT_ADD(WLAN_DOWN, "/etc/ath/ath_WLAN_stop %d %s", unit, unload_module?"unload":"");
    
    /*SNNP trap and other*/
	ATH_SCRIPT_ADD(WLAN_DOWN, "/usr/sbin/snmptrap -t 2");
	ATH_SCRIPT_ADD(WLAN_DOWN, "/usr/bin/killall -9 hostapd");	
	ATH_SCRIPT_ADD(scriptfile, "killall -9 hostapd.ath01 > /dev/null 2>&1");
	ATH_SCRIPT_ADD(scriptfile, "killall -9 hostapd.ath02 > /dev/null 2>&1");
	ATH_SCRIPT_ADD(scriptfile, "killall -9 hostapd.ath03 > /dev/null 2>&1");	
	/*
	* Run the script file
	*/
	if(scriptfile==NULL){
	    ATH_SCRIPT_RUN(WLAN_DOWN);
	}else{
	    ATH_SCRIPT_ADD(scriptfile, "chmod 777 %s", WLAN_DOWN);
        ATH_SCRIPT_ADD(scriptfile, "%s", WLAN_DOWN);
	}
	
    return 0;
}

