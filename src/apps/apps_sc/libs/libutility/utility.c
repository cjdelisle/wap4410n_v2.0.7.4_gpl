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
#include "apcfg.h"
#include "nvram.h"
#include "utility.h"
#include "flash.h"

#ifndef HOSTAP_CFG
#define HOSTAP_CFG	        "/var/hostap.cfg"
#endif

#ifndef DESTROY_ATH_IF
#define DESTROY_ATH_IF      "/tmp/delif"
#endif

extern int scfgmgr_set(char *,char *);

int sc_dbg(const char *format, ...)
{
    va_list args;
    FILE *fp;

    if (access("/tmp/sc_dbg", F_OK) != 0) {
        return 0;
    }

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

int SYSTEM(const char *format, ...) 
{
    char buf[1024]="";
    va_list arg;

    va_start(arg, format);
    vsnprintf(buf,1024, format, arg);
    va_end(arg);

    sc_dbg("cmd:{%s}\n", buf);
    system(buf);
    usleep(1);
    return 0;
}

int COMMAND(const char *format, ...)
{
    char buf[1024]="";
    va_list arg;
    FILE *fp;
    
    va_start(arg, format);
    vsnprintf(buf,1024, format, arg);
    va_end(arg);
    fp=fopen("/tmp/cmd_agent","w");
    if(fp==NULL)
        return 1;
    fwrite(buf,1024,1,fp);
    fclose(fp);
    usleep(1);
    return 0;
}

void scToLows(A_UINT8 *charStr)
{
	int i,len = strlen((char*)charStr);
	for(i=0;i<len;i++){
		if(charStr[i]>='A' && charStr[i]<='Z')
			charStr[i]+= 'a'-'A';
	}	
}
void scToUppers(A_UINT8 *charStr)
{
	int i,len = strlen((char*)charStr);
	for(i=0;i<len;i++){
		if(charStr[i]>='a' && charStr[i]<='z')
			charStr[i]-= 'a'-'A';
	}	
} 	
int scIsIpAddress(A_UINT8 *ipAddr)
{
    char *mixIp = "1.0.0.1";
    char *maxIp = "254.255.255.254";
    A_UINT32 ip = inet_addr(ipAddr);
    if(ip < inet_addr(mixIp) || ip > inet_addr(maxIp)){
        return 0;
    }

    return 1;
}

int scIsAllnumber(A_UINT8 *str)
{
    char *numStr="0123456789.";
    int i;
    int len = strlen((char*)str);
    char *p = (char*)str;
    for(i=0; i<len; i++) {
        if(strchr(numStr, *p++) == NULL) {
            return 0;
        }
    }
    return 1;
}
char *str = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.";
int scValidStr(A_UINT8 *charStr)
{
    char *p = (char*)charStr;
    for(; *p != '\0'; p++)
    {
        if(strchr(str, *p) == NULL)
            return 0;
    }
    return 1;
}
char *validstr = "\"\\[<]>:;,|=+*? ~!@#$%^&()`{}";
int scValidUrl(A_UINT8 *charStr)
{
    char tempUrl[129];
    char *p = NULL, *pt = NULL;
    int count = 0;
    
    strcpy(tempUrl, charStr);
    p = tempUrl;
    if(!strncasecmp(p,"http://",7))
        p = p + 7;
    else if(!strncasecmp(p,"https://",8))
        p = p + 8;
    pt = p;
    while((p = strchr(p, '.')) != NULL) {
        p++;
        count++;
    }
    if (count == 0)
        return 0;
    if(count == 3 && scIsIpAddress((unsigned char*)tempUrl))
        return 1;
    p = pt;
    for(; *p != '\0'; p++){
        if(strchr(validstr, *p) != NULL)
            return 0;    
    }
    if(scIsAllnumber(pt))
        return 0;
    return 1;
}
int scValidIPv6(A_UINT8 *ipStr, A_UINT8 flag)
{
    int size = strlen(ipStr);
    char *p = ipStr;
    int i;
    int flagdotNo = 0;//=3
    int flagcolNo = 0;//<=7
    int flagcolCo =0;//<=1
    int flaghex = 0;//<=4
    int flagdec = 0;//<=3
    int flagslash = 0; // 1; '/'
    char* start_p = ipStr;
    
    if(size > (flag?46:40)) return 0;

    for(i=0; i<size; p++,i++)
    {
        if( *p == '/')
        {
            flagslash++;
            if(flag)
            {
                if(strchr(p+1, '/') != NULL)
                    return 0;
                if(atoi(p+1) < 0 || atoi(p+1) > 128)
                    return 0;
                else
                    break;
            }
            else
                return 0;
        }
        else if( *p == ':')
        {
            if(memcmp(p, ":::", 3) == 0)   //value.substring(i, i+3) == ":::" )
                return 0;
            if( memcmp(p, "::", 2) == 0 )
            {
                if( flagcolCo >= 1 ) 
                    return 0;
                else
                {
                    flagcolCo++;
                    flaghex = 0;
                    flagdec = 0;
                    p++;i++;
                    start_p=p+1;
                }
            }
            else if( i == 0 )  //some error
                return 0;
            else if( flag?(*(p+1) == '/'):(i==size-1) )
                return 0;
            if( memcmp(p-1, "::", 2) != 0 && *(p+1) == '.') //don't know
                return 0;
            if( flagdotNo == 0 && flagcolNo < 7 )
            {
                flagcolNo++;
                flaghex = 0;
                flagdec = 0;
                start_p = p+1;
            }
            else
                return 0;
        }
        else if( scValidHex(*p) )
        {
            flaghex ++;

            if( (*p -'0') >=0 && (*p -'0') <=9)
            {
                flagdec ++;
            }
            else
            {
                flagdec = 0;
            }
            if( flaghex > 4 )
                return 0;
            else
                continue;
        }
        else if( *p == '.' )
        {
            if((1<=flaghex && flaghex<=3) && (flagdec == flaghex))
            {
                if( 0 <= (*(p+1) - '0') && (*(p+1) - '0')<= 9 )
                {
                    flagdotNo ++;

                    flagdec = 0;
                    flaghex = 0;
                }
                else
                    return 0;
                if(flagdotNo == 1)
                {
                      if(inet_addr(start_p)==INADDR_NONE)
                          return 0;
                 }

                if( flagdotNo > 3 )
                    return 0;
            }
            else
                return 0;
        }
        else
            return 0;
    }
    
    if( ( flagcolNo==1 && flagcolCo>7 ) || ( flagcolCo==0 && flagcolNo!=7 ) || ( flagdotNo!=0 && flagdotNo!=3 ) || (flag?(flagslash !=1):0))
        return 0;
    return 1;
}
int scValidGWv6(A_UINT8 *ipStr)
{
    int flagcolCo = 0;
    int flaghex = 0;
    int flagcolNo = 0;
    char *separ = "/";
    int prefix = 0;
    char buff[46];
    strcpy(buff, (char*)ipStr);
    char *p = strtok(buff, separ);
    
    if(p == NULL) {
        prefix = 128;
        p = buff;
    }
    if(*p != ':' && p[strlen(p)-1] != ':')
    {    
        if(!scValidIPv6((unsigned char*)p, 0)) return 0;
    }
    else
    {
        for(; *p!='\0'; p++) {
            if( *p == ':'){
                if(memcmp(p, ":::", 3) == 0)   //value.substring(i, i+3) == ":::" )
                    return 0;
                if( memcmp(p, "::", 2) == 0 )
                {
                    if( flagcolCo >= 1 ) 
                        return 0;
                    else
                    {
                        flagcolCo++;
                        flaghex = 0;
                        p++;
                    }
                }
                if(*(p+1) == '\0' && *(p-1) != ':')
                {
                    return 0;
                }    
                if(flagcolNo < 7 )
                {
                    flagcolNo++;
                    flaghex = 0;
                }
                else
                    return 0;
            }
            else if(scValidHex(*p))
            {
                flaghex++;
                if( flaghex > 4 )
                    return 0;
                else
                    continue;
            }
            else
                return 0;
        }       
    }
    if(prefix != 128){
        prefix = atoi(strtok(NULL, separ));
        if(prefix<0 || prefix >128)
            return 0;
    }
    return 1;
}

int scValidEmailAddr(A_UINT8 *email, A_UINT8 len)
{
    char *InvalidStr="`~!@#$%^&*-+=\\\"'()[]{};:,<>?/ "; //except '_', '.', '|'
    char *sign, *dot, *lastdot;
    char *start = NULL, *end = NULL;    
    
    sign = strchr((char *)email, '@');
    if(sign == NULL)
        return 0;
    
    start = (char *)email;
    end = start + len;
    lastdot = start;
    if(*lastdot == '.')
        return 0;
    while((dot = strchr(lastdot+1, '.')) != NULL){
        lastdot = dot;
    }
    if(lastdot == start || (end-lastdot!=3 && end-lastdot!=4))
        return 0;
    
    for(; start < end; start++){
        if(start == sign)
            continue;
        if(strchr(InvalidStr, *start))
            return 0;
    }
    
    return 1;
}
/*
 * mac_plus_one()
 */
void mac_plus_one(unsigned char *mac1, unsigned char *mac) 
{
    int i;

    memcpy(mac1, mac, 6);
    for(i=5; i>=0 && !++mac1[i]; i--);
    return;
}

int 
scChars2Hexs(unsigned char *charStr, int strLen, char *hexBuf , char *separator)
{
    int i;
    int cnt = 0;
    char *p = hexBuf;

    for (i = 0; i < strLen; i++) {
    	if(separator){
        	cnt += sprintf(p, "%02X%s", charStr[i],separator);
        	p += 3;
        }	
        else{
        	cnt += sprintf(p, "%02X", charStr[i]);
        	p += 2;
        }
    }
    if(separator){
    	cnt -= 1;
    	p   -= 1;
    }	
    *p = 0;	
    return cnt;
}

int
macAddrToString(char *macAddress, char *buf , char *separator)
{
	return scChars2Hexs((unsigned char *)macAddress, 6, buf, separator);
}

A_INT16 
scHex2Char(A_UINT8 * str, A_UINT8 len, A_INT32 * value)
{
	A_UINT8 ch;
	A_UINT8 i;
	A_INT32 result=0;
	
	for(i=0;(i<len)&&(str[i]);i++) {
		if(str[i]==' ') continue;
		result *= 16;
		ch = str[i];
		if((ch<='9') && (ch>='0')) result += (A_INT32)ch - '0';
		else if((ch<='F') && (ch>='A')) result += (A_INT32)(ch - 'A') + 10;
		else if((ch<='f') && (ch>='a')) result += (A_INT32)(ch - 'a') + 10;
		else return -1;
	}
	*value = result;
	
	return 0;
}
A_INT16 
scHexs2Chars(A_UINT8 * hexs, A_UINT8 * str, A_UINT8 len, A_INT16 interval)
{
    A_INT16 i;
	A_INT32 lTemp;
	A_UINT8 *p=hexs;
	
    for(i=0;i<len;i++)
    {
		scHex2Char(p,2,&lTemp);
		str[i] = lTemp;
        p += 2;
        if(interval) p++; /* skip ":" */
    }
    return 0;
} 
void
scMacStr17ToStr12(A_UINT8 *str17, A_UINT8 *str12)
{
    int i, j;
    for(i=j=0; i<17; i++){
        if((i+1)%3 == 0)
            continue;
        str12[j] = str17[i];
        j++;
    }
    str12[j] = '\0';
}

void
scMacStr12ToStr17(A_UINT8 *str12, A_UINT8 *str17, char *separator)
{
    int i, j;
    for(i=j=0; i<17; i++){
        if((i+1)%3 == 0)
            str17[i] = *separator;
        else{    
            str17[i] = str12[j];
            j++;
        }
    }
    str17[i] = '\0';
}

A_BOOL  scValidHex(char ch)
{
	char validStr[] = {"0123456789ABCDEFabcdef"};
	char name[2]={0};
	
	name[0]=ch;
	if(strstr(validStr,name))
		return 1;
	return 0;
}

A_BOOL  scValidHexs(char *str, int len)
{
	int i;
	for(i=0; i<len; i++)
	{
	    if(!scValidHex(str[i]))
	        return 0;
	}
    return 1;
}

/*
 * hostname:
 *  - contains only letters, digits and hyphens.
 *  - can't begin or end with a hyphen.
 *  - no other symbols, punctuation character or blank spaces are permitted.
 */
int scHostnameIsAllNumber(A_UINT8 *str)
{
    char *numStr="0123456789";
    int i;
    int len = strlen(str);
    char *p = str;
    for(i=0; i<len; i++) {
        if(strchr(numStr, *p++) == NULL) {
            return 0;
        }
    }
    return 1;
}
char *HostnameStr = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-";
int scValidHostnameStr(A_UINT8 *charStr)
{
    char *p = charStr;
    for(; *p != '\0'; p++)
    {
        if(strchr(HostnameStr, *p) == NULL)
            return 0;
    }
    return 1;
}
A_BOOL	scValidHostName(char *pName, A_UINT16 len)
{

    if (len < 1)
        return FALSE;
    if (pName[0] == '-' || pName[len-1] == '-')
        return FALSE;
    if (scValidHostnameStr(pName) == 0)
        return FALSE;
    if (scHostnameIsAllNumber(pName) == 1) {
        return FALSE;
    }

	return TRUE;
}

A_BOOL 	scValidNetbiosName(char *pName, A_UINT16 len)
{
	int i;
	char invalidMSNameStr[] = "\"/\\[<]>.:;,|=+*? `";
	char name[2]={0};

    if (len < 1)
        return FALSE;
    if (pName[0] == '-' || pName[len-1] == '-')
        return FALSE;
	for(i=0; i<len; i++){
		name[0]=pName[i];
		if(strstr(invalidMSNameStr,name))
			return FALSE;
	}
	for(i=0; i<len; i++){
		if(pName[i]<'0' || pName[i]>'9')
			return TRUE;
	}
	return FALSE;
}


A_BOOL scIpSeparateTo4(A_UINT32 ipAddress, int *ip1, int *ip2, int *ip3, int *ip4)
{
	struct in_addr addr;
    char netstr[20]; 	
    int l;
    	
    addr.s_addr = ipAddress;
    strcpy(netstr, inet_ntoa(addr));
    
    l=sscanf(netstr,"%d.%d.%d.%d",ip1,ip2,ip3,ip4);

    if(l!=4) return FALSE;	
    
    return TRUE;
}	
A_BOOL scValidIpAddress(A_UINT32 ipAddress)
{
	int ip[4];
	
	if(!scIpSeparateTo4(ipAddress, &ip[0], &ip[1], &ip[2], &ip[3]))
		return FALSE;
    
    if (ip[0] < 1 || ip[0] > 223 || 127 == ip[0])
    	return FALSE;
    	
    if (ip[1] < 0 || ip[1] > 255)
		return FALSE;
		
	if (ip[2] < 0 || ip[2] > 255)
		return FALSE;

	if (ip[3] < 1 || ip[3] > 254)
		return FALSE;

	return TRUE;
}	

static int scIpMaskTable[9] = {0, 128, 192, 224, 240, 248, 252, 254, 255};
static int scIpMaskRange[4][2] = { {2,8},{0,8},{0,8},{0,7}};

A_BOOL scValidIpMask(A_UINT32 ipMask, A_UINT32 *pValidIpMask)
{
	int mask[4];
	int i, j;
	int start, end;
	A_BOOL found;

	if(!scIpSeparateTo4(ipMask, &mask[0], &mask[1], &mask[2], &mask[3]))
		return FALSE;

	for(i=0; i<4; i++){
		found = FALSE;
		start = scIpMaskRange[i][0];
		end = scIpMaskRange[i][1];

		for(j = start; j<=end; j++){
			if((mask[i]||i==3) &&  mask[i] == scIpMaskTable[j])
				found = TRUE;
		}
		if(!found){
			A_BOOL returnValue = TRUE;
			
			for(;i<4;i++){
				if(mask[i] != 0)
					returnValue = FALSE;
				mask[i] = 0;
			}	
			if(pValidIpMask){
				char netstr[20];	
				sprintf(netstr, "%d.%d.%d.%d", mask[0], mask[1], mask[2], mask[3]);
	        	*pValidIpMask = inet_addr(netstr);
        	}
			return returnValue;	
		}	
	}	
	if(pValidIpMask){
		*pValidIpMask = ipMask;	
	}	
    return TRUE;
}

A_BOOL scValidIpGateWay(A_UINT32 gateway)
{
	return ((gateway == 0) || scValidIpAddress(gateway));
}	

A_BOOL scValidIpMaskGateWay(A_UINT32 ipaddr, A_UINT32 netmask, A_UINT32 gateway)
{
	if (0 == (ipaddr&~netmask) || 0 == ~(ipaddr|netmask))
	{
		return FALSE;
	}
	
	if (0 == gateway)
		return TRUE;
		
	if (0 == (gateway&~netmask) || 0 == ~(gateway|netmask))
	{
		return FALSE;
	}
	if ((ipaddr&netmask) != (gateway&netmask))
	{
		return FALSE;
	}
	return TRUE;
}

A_BOOL  scInvalidHttpPort(A_UINT32 port, A_UINT32 flag)
{
    int i;
    A_UINT32 invalidHttpPortArray[] = 
    {
        1,   2,   3,   5,   7,   9,   20,  21, 
        22,  23,  25,  31,  53,  67,  68,  69, 
        79,  81,  82,  123, 137, 138, 139, 161,
        168, 169, 80,  443
    };
    
    for(i=0; i<28; i++){
        if(port == invalidHttpPortArray[i]){
        	if(flag==1 && port==80)
        		;
        	else if(flag==2 && port==443)
        		;
            else
            	return TRUE;
        }
    }    
    return FALSE;
}
int scMaxTxPowerGet(int unit)
{
    FILE *fp;
    char filename[32];
    int maxtxpower=0;

    sprintf(filename, "/tmp/maxpwr_curr");
    
    if((access(filename, F_OK) == 0) && (fp = fopen(filename, "r")))
    {
        fscanf(fp, "%d dBm", &maxtxpower);
        fclose(fp);
    }
    return  maxtxpower;
}
A_BOOL
asciiToPassphraseKey(A_UINT8 *pstr, A_UINT8 *pPpKey, int encryptedKeyLen)
{
    int     keyIndex   = 0;
    int     asciiIndex = 0;
    int     count      = 0;
    A_UINT8 tval       = 0;
    
    int     alen       = strlen((char*)pstr); // Length os ascii string
    
    /* Convert the ascii value read from file to Hex value and store in cfg struct */
    while (asciiIndex < alen && keyIndex < encryptedKeyLen) {
        if (pstr[asciiIndex] >= '0' && pstr[asciiIndex] <= '9') {
            tval = (tval << 4) | (pstr[asciiIndex] - '0');
            count++;
        } else if (pstr[asciiIndex] >= 'a' && pstr[asciiIndex] <= 'f') {
            tval = (tval << 4) | (pstr[asciiIndex] - 'a' + 10);
            count++;
        } else if (pstr[asciiIndex] >= 'A' && pstr[asciiIndex] <= 'F') {
            tval = (tval << 4) | (pstr[asciiIndex] - 'A' + 10);
            count++;
        } else {
            /* bad character */
            return A_EINVAL;
        }
        /* 2 ascii chars make one hex char */
        if (count == 2) {
            /* store hex string in apcfg struct */
            pPpKey[keyIndex++] = tval;
            count = 0;
            tval = 0;
        }
        asciiIndex++;
    }
    
    return A_OK;
}

void sgml_encode(char *pStrDest, char *pStrSrc, A_BOOL readOnly)
{
	char *pSrc;
	
	int len = 0;
	if(pStrDest==NULL || pStrSrc==NULL){
	    return;
	}
	
	for(pSrc = pStrSrc; *pSrc; pSrc++){
		switch(*pSrc){
			case '&':
				len += sprintf(pStrDest+len, "&#38;"); 
				break;
			case '"':
				len += sprintf(pStrDest+len, "&#34;"); 
				break;
			case '<':
				len += sprintf(pStrDest+len, "&#60;");
				break;
			case '>':
				len += sprintf(pStrDest+len, "&#62;");
				break;
			case '(':
				len += sprintf(pStrDest+len, "&#40;");
				break;
			case ')':
				len += sprintf(pStrDest+len, "&#41;");
				break;		
			case 39:   //'
			    len += sprintf(pStrDest+len, "&#39;");
			    break;	
			case ' ':
			    if(readOnly)
			        len += sprintf(pStrDest+len, "&nbsp;");
			    else
			        len += sprintf(pStrDest+len, "&#32;");    
			    break;        
			default:
				pStrDest[len++] = *pSrc;
				break;					
		}
	}
	pStrDest[len] = 0;
}

void scSecretHide(char *secret)
{
	while(*secret){
		*secret = '*';
		secret++;
	}	
}	

A_BOOL scSecretHidden(char *secret)
{
	if(!(*secret))
		return  FALSE;
	while(*secret){
		if(*secret != '*')
			return FALSE;
		secret++;	
	}	
	return TRUE;
}

#define scSetByte(dest,source,len) memcpy((dest),(source),(len))
A_UINT16 scGetWord(A_UINT8 * buf)
{
    A_UINT16 wResult;
    scSetByte(&wResult, buf, 2);
    return wResult;
}
void scSetWord(A_UINT8 * buf, A_UINT16 wValue)
{
    scSetByte(buf, &wValue, 2);
}

A_UINT32 scGetDword(A_UINT8 * buf)
{
    A_UINT32 dwResult;
    scSetByte(&dwResult, buf, 4);
    return dwResult;
}

void scSetDword(A_UINT8 * buf, A_UINT32 dwValue)
{
    scSetByte(buf, &dwValue, 4);
}
A_BOOL scValidwdsVlanList(char *list)
{
    char testlist[21];
    char *pa = NULL;
    int i = -1, count = 1, t = -1;
    int value[5];
    int len = strlen(list);

    memset(testlist, 0, 21);
    if(len < 21)
        strcpy(testlist, list);
    else
        return FALSE;
    i = -1;
    while(testlist[++i] != '\0')
    {
        if(testlist[i] < '0' || testlist[i] > '9')
        {
            if(testlist[i] != ',')
            {
                return FALSE;
            }
            else
            {
                count++;
                if(count > 1)
                {
                    return FALSE;
                }
            }
        }
        else
            count = 0;
    }
    if(i == 0)
        return TRUE;
    
    i = -1;
    pa = strtok(testlist,",");
    while(pa != NULL)
    {
        value[++i] = atoi(pa);
        if(value[i] < 1 || value[i] > 4094)
        {
            return FALSE;
        }
        t = -1;
        while(++t < i)
            if(value[t] == value[i])
                return FALSE;
        if(i > 3)
        {
            return FALSE;
        }
        pa = strtok(NULL,",");
    }
    return TRUE;
}



int get_sockfd()
{
	int sockfd = -1;
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("user: socket creating failed");
		return (-1);
	}
	return sockfd;
}

int getMgtBrInfo(if_info_t *if_info)
{
    char ifName[16];
    FILE *fp;
    char line[64+1];
    int vlanId = 0;
    
    /*If vlan disabled*/
    if((fp=fopen("/tmp/vlan.conf","r"))==NULL)
    {
        sprintf(ifName,"br0");
    }
    else
    {
        fgets(line, 64, fp); /* eat native vlan id */
        fgets(line, 64, fp); /* management vlan id */
        vlanId = atoi(line);
        sprintf(ifName,"br%d", vlanId);
        fclose(fp);
    }
    return getIFInfo(ifName, if_info);
}

int getIFInfo(char *if_name, if_info_t *if_info)
{
	unsigned char *pt;
	struct ifreq ifr;
	struct sockaddr_in *saddr;
	int fd;
	int ret=0;

	strcpy(if_info->ifname,if_name);
	if ((fd=get_sockfd())>=0)
	{
		strcpy(ifr.ifr_name, if_info->ifname);
		ifr.ifr_addr.sa_family = AF_INET;
		/* get ip address */
		if (ioctl(fd, SIOCGIFADDR, &ifr)==0){
			saddr = (struct sockaddr_in *)&ifr.ifr_addr;
			strcpy(if_info->ipaddr,(char *)inet_ntoa(saddr->sin_addr));
			/* for hide on demand ip */
			if(strcmp(if_info->ipaddr,"10.64.64.64")==0)
				ret=-2;			
		}else
			ret=-1;
		/* get mac address */
		if (ioctl(fd, SIOCGIFHWADDR, &ifr)==0){
			pt=ifr.ifr_hwaddr.sa_data;
			sprintf(if_info->mac,"%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX"
					,*pt,*(pt+1),*(pt+2),*(pt+3),*(pt+4),*(pt+5));
			if_info->mac[17]='\0';
		}else 
			ret=-1;
		/* get netmask */
		if (ioctl(fd,SIOCGIFNETMASK , &ifr)==0){
			saddr = (struct sockaddr_in *)&ifr.ifr_addr;
			strcpy(if_info->mask,(char *)inet_ntoa(saddr->sin_addr));
		}else
			ret=-1;
		
		/* get mtu */
		if (ioctl(fd,SIOCGIFMTU, &ifr)==0){
			if_info->mtu=ifr.ifr_mtu;	
		}else
			ret=-1;	
		close(fd);
		
		/*Try to get gateway*/
		if(apCfgDhcpEnableGet() == FALSE && apCfgGatewayAddrGet()==0)
		{
		    if_info->gw.s_addr = 0;
	    }
	    else
    	{
    		FILE *fp;
    		
    		if_info->gw.s_addr = 0;
    		
    		fp = fopen("/proc/net/route","r");
    		if(fp)
    		{
    			char line[256];
    			while(fgets(line, sizeof(line), fp)){
    				if(!strstr(line, if_name))
    					continue;
    				if(sscanf(line, "%*s\t00000000\t%lx", (long int *)&if_info->gw.s_addr))
    					break;	
    			}
    			fclose(fp);
    		}	
    	}
		
		return ret;

	}
	return -1;
}
char isspace (unsigned char c)
{
  if ( c == ' '
    || c == '\f'
    || c == '\n'
    || c == '\r'
    || c == '\t'
    || c == '\v' )
      return 1;

  return 0;
}
int getIFAdvInfo(char *if_name, if_adv_info_t *if_info)
{
    FILE *fh;
	char buf[1024];
	struct net_device_info stats;
		
	fh = fopen("/proc/net/dev", "r");
	if (!fh) {
		return -1;
	}
	
	fgets(buf, sizeof(buf), fh);	/* eat line */
	fgets(buf, sizeof(buf), fh);
	
	while (fgets(buf, sizeof(buf), fh)) {
	    char name[128];
	    int i = 0;
	    char *s = buf;
	    while(*s != ':')
	    {
	        if(!isspace(*s))
	            name[i++] = *s;
	        s++;
	    }
	    name[i] = 0;
	    if(strcmp(if_name, name) == 0)
	    {
	        memset(&stats, 0, sizeof(struct net_device_info));
	        sscanf(++s, "%Lu%Lu%lu%lu%lu%lu%lu%lu%Lu%Lu%lu%lu%lu%lu%lu%lu",
        	   &stats.rx_bytes, /* missing for 0 */
        	   &stats.rx_packets,
        	   &stats.rx_errors,
        	   &stats.rx_dropped,
        	   &stats.rx_fifo_errors,
        	   &stats.rx_frame_errors,
        	   &stats.rx_compressed, /* missing for <= 1 */
        	   &stats.rx_multicast, /* missing for <= 1 */
        	   &stats.tx_bytes, /* missing for 0 */
        	   &stats.tx_packets,
        	   &stats.tx_errors,
        	   &stats.tx_dropped,
        	   &stats.tx_fifo_errors,
        	   &stats.collisions,
        	   &stats.tx_carrier_errors,
        	   &stats.tx_compressed /* missing for <= 1 */
        	   );
        	   
	        strcpy(if_info->ifname, name);
	        if_info->rx_packets = stats.rx_packets;
	        if_info->tx_packets = stats.tx_packets;
	        if_info->rx_bytes = stats.rx_bytes;
	        if_info->tx_bytes = stats.tx_bytes;	        
	        if_info->errors = stats.rx_errors;
	        if_info->dropped = stats.rx_dropped;
	        fclose(fh);
	        return 0;
	    }
	}/* end while */
	fclose(fh);
    return 1;
}

int getEthernetStatus(void)
{
    //postil 2:fail or error; 1:connect; 0:disconnect
    FILE *fp;
    char buf[16];
 
    fp=fopen("/proc/ethernet_status_cfb","r");
    if(fp==NULL)
        return 2;

    fgets(buf,16,fp);
    fclose(fp);
	if(strstr(buf, "Status=0") == NULL)
	{
		if(strstr(buf, "Status=1") == NULL)
			return 2;
		else
			return 1;		
	}else 
		return 0;
}

void libGetAPMacAddress(char *pMac)
{
    unsigned char *pt;
	struct ifreq ifr;
	int fd;
	int ret=0;
	if ((fd=get_sockfd())>=0)
	{
		strcpy(ifr.ifr_name, "wifi0");
		ifr.ifr_addr.sa_family = AF_INET;

		/* get mac address */
		if (ioctl(fd, SIOCGIFHWADDR, &ifr)==0){
			pt=(unsigned char *)ifr.ifr_hwaddr.sa_data;
			sprintf(pMac,"%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX"
					,*pt,*(pt+1),*(pt+2),*(pt+3),*(pt+4),*(pt+5));
		}else 
			ret=-1;

		close(fd);

	}
}

void getProductName(char *buffer)
{
#ifdef WAP4410N
    sprintf(buffer,"WAP4410N"); 
#else    
    sprintf(buffer,"AP101nA");
#endif
}

void getProductDesc(char *buffer)
{
#ifdef WAP4410N
    sprintf(buffer,"Wireless-N Access Point with PoE"); 
#else    
    sprintf(buffer,"WAP Pre-N Wireless AP");
#endif
}

enum{
    NEEDNOT_DST,    	/*need not daylight saving time*/
    NA_DST,         	/*North America need DST*/
    EU_DST,         	/*Europe DST*/
    CHILE_DST,		/*Chile dst*/
    SA_DST,		/*sourth america,for example: Brazil*/
    IRAQ_DST,		/*raq and Iran */
    AU2_DST,		/*Australia - Tasmania*/
    AU3_DST,		/*New Zealand, Chatham */
    AF_DST,		/*Egypt*/
    AU_DST         	/*Australia DST*/
};

/*
 * type: 0 - normal; 1 - multi-language for WebGUI
 */
void getTimeofDay(char *buffer, int type)
{
    time_t t;
    struct tm *st;
    int len = 0;
    int add_year=1900;
    
    
    time(&t);
    st=localtime(&t);
    if(st->tm_year == 108)
    	add_year=1903;
#ifdef LINKSYS
{
    char wday[128];
    char timezone[20];
    strcpy(timezone, apCfgTimezoneOffsetGet());
    switch(st->tm_wday)
    {
        case 0:
            if (type == TIME_FORMAT_MULTILANG)
                strcpy(wday, "<script language=\"javascript\" type=\"text/javascript\">dw(st_week_7);</script>");
            else
                strcpy(wday, "Sun");
            break;
        case 1:
            if (type == TIME_FORMAT_MULTILANG)
                strcpy(wday, "<script language=\"javascript\" type=\"text/javascript\">dw(st_week_1);</script>");
            else
                strcpy(wday, "Mon");
            break;
        case 2:
            if (type == TIME_FORMAT_MULTILANG)
                strcpy(wday, "<script language=\"javascript\" type=\"text/javascript\">dw(st_week_2);</script>");
            else
                strcpy(wday, "Tue");
            break;
        case 3:
            if (type == TIME_FORMAT_MULTILANG)
                strcpy(wday, "<script language=\"javascript\" type=\"text/javascript\">dw(st_week_3);</script>");
            else
                strcpy(wday, "Wed");
            break;
        case 4:
            if (type == TIME_FORMAT_MULTILANG)
                strcpy(wday, "<script language=\"javascript\" type=\"text/javascript\">dw(st_week_4);</script>");
            else
                strcpy(wday, "Thu");
            break;
        case 5:
            if (type == TIME_FORMAT_MULTILANG)
                strcpy(wday, "<script language=\"javascript\" type=\"text/javascript\">dw(st_week_5);</script>");
            else
                strcpy(wday, "Fri");
            break;
        case 6:
            if (type == TIME_FORMAT_MULTILANG)
                strcpy(wday, "<script language=\"javascript\" type=\"text/javascript\">dw(st_week_6);</script>");
            else
                strcpy(wday, "Sta");
            break;
        default:
            strcpy(wday, "\0");
    }
    if(apCfgTimeModeGet() == 0)
    len = sprintf(buffer,"%d/%02d/%02d %s %02d:%02d:%02d (%s)"
                ,st->tm_year+add_year
                ,st->tm_mon+1
                ,st->tm_mday
                ,wday
                ,st->tm_hour
                ,st->tm_min
                ,st->tm_sec
                ,timezone+3);
    else
    len = sprintf(buffer,"%d/%02d/%02d %s %02d:%02d:%02d"
                ,st->tm_year+add_year
                ,st->tm_mon+1
                ,st->tm_mday
                ,wday
                ,st->tm_hour
                ,st->tm_min
                ,st->tm_sec
                );
}                
#else    
    len = sprintf(buffer,"%d-%02d-%02d %02d:%02d:%02d"
                ,st->tm_year+add_year
                ,st->tm_mon+1
                ,st->tm_mday
                ,st->tm_hour
                ,st->tm_min
                ,st->tm_sec);
#endif
    buffer[len] = 0;
}

void getUpTime(char *buffer)
{
    FILE *fUptime;
    char uptime[20];
    struct timespec ltime;
    struct tm ltm;
    int days;
    
    fUptime = fopen("/proc/uptime","r");
    fscanf(fUptime,"%s ",uptime);
    fclose(fUptime);
    
    ltime.tv_sec = atol(uptime);
    
    days = ltime.tv_sec / (60*60*24);
    bcopy((char *)localtime (&ltime.tv_sec), (char *)&ltm, sizeof(struct tm));
    if(days > 0)
        sprintf(buffer, "Day %d, %2d:%02d:%02d", days, ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
    else
        sprintf(buffer, "%2d:%02d:%02d", ltm.tm_hour, ltm.tm_min, ltm.tm_sec);

}

/*
 * Follow New PID VID from Cisco, example:
 *
 *      PID VID: WAP4410N-A V02
 */
void getPid(char *buffer)
{
    FILE *fp = NULL;
    char line[16] = {0};
    unsigned int domain = 0;

    /* show model name */
    sprintf(buffer, "%s", "WAP4410N-");

    /*
     * get domain to show correct PID
     */
    if((access("/tmp/hw_info/domain", F_OK) == 0) && (fp = fopen("/tmp/hw_info/domain", "r")))
    {
        fgets(line, sizeof(line), fp);
        fclose(fp);

        sscanf(line, "%x", &domain);
    }
    switch(domain) {
        /*
        case SKU_G5:
        case SKU_AU:
        case SKU_LA:
        case SKU_CN:
        case SKU_KR:
        case SKU_BR:
            sprintf(buffer, "%s%s", buffer, "E");
            break;
            */
        case SKU_US:
            sprintf(buffer, "%s%s", buffer, "A");
            break;
        default:
            sprintf(buffer, "%s%s", buffer, "E");
            break;
    }
}
void getHwVersion(char *buffer)
{
    FILE *fp = NULL;
    char line[16] = {0};
    unsigned int hwv = 0;

    /* get PID */
    getPid(buffer);

    /*
     * get hardwave version to show correct VID
     */
    if ((access("/tmp/hw_info/hwv", F_OK) == 0) && (fp = fopen("/tmp/hw_info/hwv", "r"))) {
        fgets(line, sizeof(line), fp);
        fclose(fp);

        sscanf(line, "%x", &hwv);
    }
    if (hwv == 0x13)
        sprintf(buffer, "%s%s", buffer, " V02");
    else
        sprintf(buffer, "%s%s", buffer, " V01");

}

void getVersion(char *buffer)
{
    FILE *fp;
    char buf[16];
    char ext[32] = {0};
    int len;
 
    fp=fopen("/etc/fwversion","r");
    if(fp==NULL)
        return ;

    fgets(buf,16,fp);
    fclose(fp);
    buf[4] = 0; /* drop '\n'*/

    fp = fopen("/etc/extra_version", "r");
    if (fp != NULL) {
	    fread(ext,1,31,fp);
	    fclose(fp);
    }
    len = strlen(ext);
    if (ext[len-1] == '\n')
	    ext[len-1] = 0;
#ifdef LINKSYS
    sprintf(buffer,"%c.%c",buf[0],buf[1]);

    if (buf[2] == '0')
	    sprintf(buffer, "%s.%c", buffer, buf[3]);
    else
	    sprintf(buffer, "%s.%s", buffer, &buf[2]);

    if (strlen(ext))
	    sprintf(buffer, "%s.%s", buffer, ext);
#else
    sprintf(buffer,"Version %c.%c Release %s",buf[0],buf[1],&buf[2]);
#endif
}

unsigned short getVersion2(void)
{
    FILE *fp;
    unsigned char buf[16];
 
    fp=fopen("/etc/fwversion","r");
    if(fp==NULL)
        return 0;

    fgets(buf,16,fp);
    fclose(fp);
    buf[4] = 0;
    return (unsigned short)strtol(buf, NULL, 16);
}

/*  Add for IPv6 address 
*   AC@CPU_AP
*   2007.11.21
*/
#ifdef AF_INET6
#define PATH_PROCNET_IFINET6    "/proc/net/if_inet6"
#define PATH_PROCNET_ROUTE6     "/proc/net/ipv6_route"
#define IPV6_ADDR_LINKLOCAL     0x0020U
#endif

int getMgtBrv6Info(if_infov6_t *if_info, int status)
{
    char ifName[16];
    FILE *fp;
    char line[64+1];
    int vlanId = 0;
    int ret = 0;
    
    /*If vlan disabled*/
    if((fp=fopen("/tmp/vlan.conf","r"))==NULL)
    {
        sprintf(ifName,"br0");
    }
    else
    {
        fgets(line, 64, fp); /* eat native vlan id */
        fgets(line, 64, fp); /* management vlan id */
        vlanId = atoi(line);
        sprintf(ifName,"br%d", vlanId);
        fclose(fp);
    }
    memset(if_info, 0, sizeof(if_infov6_t));
    strcpy(if_info->ifname, ifName);

    if(status == 0 && !apCfgDhcp6EnableGet()) {
        ret = getIP6Info(ifName, if_info->ipaddr);
        getIP6GwInfo(ifName, if_info->gw);
    }
    
    if(status == 1 && apCfgDhcp6EnableGet()){
        
        getIP6dhcpInfo(ifName, if_info);
        /*
        strcpy(if_info->ipaddr, apCfgIpv6AddrGet());
        strcpy(if_info->gw, apCfgGatewayv6AddrGet());
        */
    }
    if(status == 2 && apCfgRadvdEnableGet()){
        getIP6RadvdInfo(ifName, if_info);
    }
    return ret;
}
int getIP6Info(char *ifname, char *address )
{    
#if defined(AF_INET6) && defined(IN6_IS_ADDR_V4MAPPED)

	FILE *fp;
	char addr6[40], devname[20];
    struct in6_addr inaddr6;
	int plen, scope, dad_status, if_idx;
	char addr6p[8][5];
    
    if ((fp = fopen(PATH_PROCNET_IFINET6, "r")) != NULL) {
		while ( fscanf(fp, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %20s\n",
				addr6p[0], addr6p[1], addr6p[2], addr6p[3], addr6p[4],
				addr6p[5], addr6p[6], addr6p[7], &if_idx, &plen, &scope,
				&dad_status, devname) != EOF) {
		    if (!strcmp(devname, ifname) && (scope != IPV6_ADDR_LINKLOCAL)){
		        
		        sprintf(addr6, "%s:%s:%s:%s:%s:%s:%s:%s",
						addr6p[0], addr6p[1], addr6p[2], addr6p[3],
						addr6p[4], addr6p[5], addr6p[6], addr6p[7]);
				inet_pton(AF_INET6, addr6, &inaddr6);				
				inet_ntop(AF_INET6, (void *)&inaddr6, address, INET6_ADDRSTRLEN);
				sprintf(address,"%s/%d",address,plen);
				
				fclose(fp);				
				return 0;
		    }
		    else
		        continue;
	    }
	fclose(fp);
	}
	return -1;	
#else
    return -1;
#endif
}

int getIP6GwInfo(char *ifname, char *gw )
{    
#if defined(AF_INET6) && defined(IN6_IS_ADDR_V4MAPPED)
	FILE *fp;
	char addr6[40],addr6x[80];
    struct in6_addr inaddr6;
	char iface[16];
	int iflags, metric, refcnt, use, prefix_len, slen;
    
    if ((fp = fopen(PATH_PROCNET_ROUTE6, "r")) != NULL) {
		while ( 1 ) {
		    int r;
		    r = fscanf(fp, "%32s%x%*s%x%32s%x%x%x%x%s\n",
				   addr6x, &prefix_len, &slen, addr6x+40,
				   &metric, &use, &refcnt, &iflags, iface);
		    if (r != 9) {
			    if ((r < 0) && feof(fp)) { /* EOF with no (nonspace) chars read. */
				    break;
			    }
                return -1;
		    }
		    else
		    {
		        if(strncmp(iface, ifname, strlen(ifname)) == 0 && (metric == 1)){
		            int i = 0;
		            char *p = addr6x;	        
		            do{		                
				        addr6[i++] = *p++;
				        if (!((i+1)%5)) {
					        addr6[i++] = ':';
				        }
		            }while(i < 40);
		            inet_pton(AF_INET6, addr6,	&inaddr6);
		            inet_ntop(AF_INET6, (void *)&inaddr6, gw, INET6_ADDRSTRLEN);
		            sprintf(gw,"%s/%d",gw,prefix_len);
		            
		            fclose(fp);
				    return 0;
		        }
		    }
	    }
	fclose(fp);
	}
	return -1;	
#else
    return -1;
#endif
}

int getIP6RadvdInfo(char *ifname, if_infov6_t *if_info )
{
    FILE *fp = NULL;
    char buff[64];
    struct in6_addr inaddr6;
    char addr[INET6_ADDRSTRLEN];
    char route[INET6_ADDRSTRLEN];
    char temp[INET6_ADDRSTRLEN];
    char *p = "/";
    
    int prelen;
    
    if((fp = fopen("/proc/ipv6_info_cfb", "r")) != NULL)
    {
        fgets(buff, 64, fp);
        fgets(buff, 64, fp);
        if(strncmp(buff, "Radvd=1", 7) != 0){
            fclose(fp);
            return -1;
        }
        fgets(buff, 64, fp);
        memset(addr, 0, sizeof(addr));
        sscanf(buff, "Ip=%s", addr);
        
        /* After put the address to if_info, remove overstaffed zero */
        if(strlen(addr) != 0) {
            strtok(addr, p);
            prelen= atoi(strtok(NULL, p));
            inet_pton(AF_INET6, addr,	&inaddr6);
            memset(temp, 0, sizeof(temp));
            inet_ntop(AF_INET6, (void *)&inaddr6, temp, INET6_ADDRSTRLEN);
            
            sprintf(if_info->ipaddr, "%s/%d",temp, prelen);
        }
        fgets(buff, 64, fp);
        memset(route, 0, sizeof(route));
        sscanf(buff, "Route=%s", route);
        if(strlen(route) != 0) {
	        strtok(route, p);
	        prelen= atoi(strtok(NULL, p));
	        inet_pton(AF_INET6, route,	&inaddr6);
	        memset(temp, 0, sizeof(temp));
	        inet_ntop(AF_INET6, (void *)&inaddr6, temp, INET6_ADDRSTRLEN);
	        sprintf(if_info->gw, "%s/%d",temp, prelen);
      	}
        fclose(fp);
    }
    
    return 1;
}
int getIP6dhcpInfo(char *ifname, if_infov6_t *if_info )
{
    FILE *fp = NULL;
    char buff[32];
    
    if((fp = fopen("/tmp/dhcp6c.lease", "r")) != NULL){
        fgets(buff, 32, fp);
        if(strncmp(buff, "1", 1) == 0){
            strcpy(if_info->ipaddr, apCfgIpv6AddrGet());
            strcpy(if_info->gw, apCfgGatewayv6AddrGet());
        }            
        fclose(fp);
    }
    return 0;
}
/*
*   For domain & country issue
*/
void country_list_generate(void)
{
    FILE *fp;
    char line[64];
    unsigned int domain = 0;
    
    unlink("/tmp/country_list");
    
    if((access("/tmp/hw_info/domain", F_OK) == 0) && (fp = fopen("/tmp/hw_info/domain", "r")))
    {
        fgets(line, sizeof(line), fp);
        fclose(fp);
        
        sscanf(line, "%x", &domain);
    }

    switch(domain)
    {
        default:
        case SKU_US:
            SYSTEM("echo %s-%d >> /tmp/country_list", "United States", CTRY_UNITED_STATES);
            break;
        case SKU_AU:
            SYSTEM("echo %s-%d >> /tmp/country_list", "Australia", CTRY_AUSTRALIA);
            break;
        case SKU_BR:
            SYSTEM("echo %s-%d >> /tmp/country_list", "Brazil", CTRY_BRAZIL);
            break;
        case SKU_CN:
            SYSTEM("echo %s-%d >> /tmp/country_list", "China", CTRY_CHINA);
            break;
        case SKU_G5:
            SYSTEM("echo %s-%d >> /tmp/country_list", "Asia", CTRY_SINGAPORE+1000);
            SYSTEM("echo %s-%d >> /tmp/country_list", "Denmark", CTRY_DENMARK);
            SYSTEM("echo %s-%d >> /tmp/country_list", "Europe", CTRY_GERMANY+1000);
            SYSTEM("echo %s-%d >> /tmp/country_list", "Finland", CTRY_FINLAND);
            SYSTEM("echo %s-%d >> /tmp/country_list", "France", CTRY_FRANCE);
            SYSTEM("echo %s-%d >> /tmp/country_list", "Germany", CTRY_GERMANY);
            SYSTEM("echo %s-%d >> /tmp/country_list", "Ireland", CTRY_IRELAND);
            SYSTEM("echo %s-%d >> /tmp/country_list", "Italy", CTRY_ITALY);
            SYSTEM("echo %s-%d >> /tmp/country_list", "Netherlands", CTRY_NETHERLANDS);
            SYSTEM("echo %s-%d >> /tmp/country_list", "New Zealand", CTRY_NEW_ZEALAND);
            SYSTEM("echo %s-%d >> /tmp/country_list", "Norway", CTRY_NORWAY);
            SYSTEM("echo %s-%d >> /tmp/country_list", "Spain", CTRY_SPAIN);
            SYSTEM("echo %s-%d >> /tmp/country_list", "Sweden", CTRY_SWEDEN);
            SYSTEM("echo %s-%d >> /tmp/country_list", "Switzerland", CTRY_SWITZERLAND);
            SYSTEM("echo %s-%d >> /tmp/country_list", "United Kingdom", CTRY_UNITED_KINGDOM);
            
            break;
        case SKU_LA:
            SYSTEM("echo %s-%d >> /tmp/country_list", "Mexico", CTRY_MEXICO);
            SYSTEM("echo %s-%d >> /tmp/country_list", "Puerto Rico", CTRY_PUERTO_RICO);
            SYSTEM("echo %s-%d >> /tmp/country_list", "South America", CTRY_BRAZIL+1000);
            break;                 
        case SKU_KR:
            SYSTEM("echo %s-%d >> /tmp/country_list", "South Korea", CTRY_KOREA_ROC);
            break;
        case SKU_JP:
            SYSTEM("echo %s-%d >> /tmp/country_list", "Japan", CTRY_JAPAN);
            break;
    }
    if(!check_country_domain(apCfgCountryCodeGet()))
        set_default_country();
}

void set_default_country(void)
{    
    char str_domain[32];
    int domain;
    
    domain = sc_get_default_country();
    sprintf(str_domain, "%d", domain);
    nvram_set("sys_domain", str_domain);

    /* set language for SKU_JP */
    if (domain == CTRY_JAPAN) {
        nvram_set("sys_lang", "jp");
    }
}

A_BOOL check_country_domain(int countryCode)
{
    int ret = 0;
    FILE *fp = NULL;
    char buffer[128];
    char *line = "-";
    char *p = NULL;
    
    fp = fopen("/tmp/country_list", "r");
    if(fp) {
        while(fgets(buffer, 128, fp)){
            p = strtok(buffer, line);
            p = strtok(NULL, line);
            if(p != NULL)
                if(atoi(p) == countryCode)
                {
                    ret = 1;
                    break;
                }
        }
        fclose(fp);
    }
    
    return ret;
}
A_INT16 sc_get_default_country(void)
{
    FILE *fp;
    char line[64];
    unsigned int domain = 0;
    
    if((access("/tmp/hw_info/domain", F_OK) == 0) && (fp = fopen("/tmp/hw_info/domain", "r")))
    {
        fgets(line, sizeof(line), fp);
        fclose(fp);
        
        sscanf(line, "%x", &domain);
    }
    
    switch(domain)
    {
        default:
        case SKU_US:
            return CTRY_UNITED_STATES;
            break;
        case SKU_AU:
            return CTRY_AUSTRALIA;
            break;
        case SKU_BR:
            return CTRY_BRAZIL;
            break;
        case SKU_CN:
            return CTRY_CHINA;
            break;
        case SKU_G5:
            return CTRY_GERMANY;
            break;
        case SKU_LA:
            return CTRY_MEXICO;
            break;
        case SKU_JP:
            return CTRY_JAPAN;
            break;
	case SKU_KR:
	    return CTRY_KOREA_ROC;
	    break;
    }
    
    return -1;
}

void sc_swap(int v[], int i, int j)
{
   int temp;

   temp = v[i];
   v[i] = v[j];
   v[j] = temp;
}

A_BOOL  scCompositor(int v[], int left, int right)
{
    int i, last;
    
    if (left >= right) 
       return FALSE;
    sc_swap(v, left, (left + right)/2);
    last = left;                       
    for (i = left + 1; i <= right; i++)
       if (v[i] < v[left])
           sc_swap(v, ++last, i);
    sc_swap(v, left, last);  
    scCompositor(v, left, last-1);
    scCompositor(v, last+1, right);
    return TRUE;
}

A_BOOL scConversionSSID(A_UINT8 *src, A_UINT8 *dest)
{
    char *p = (char *)src;

    while(*p != 0)
    {
        if(*p == '\"' || *p == '\\' || *p == '`')
            *dest++ = '\\';
        *dest++ = *p++;
    }
    *dest = 0;

    return TRUE;
}

A_STATUS scSetDefaultVap(int unit, int bss)
{
    /* Set basic default */
    apCfgActiveModeSet(unit, bss, 0);
    apCfgSsidSet(unit, bss, "");
    apCfgSsidModeSet(unit, bss, DEFAULT_SSID_SUPPRESS_MODE);
    
    /* Set Security default */
    scSetDefaultSecurity(unit, bss);
    
    /* Set ACL default */
    apCfgAclModeSet(unit, bss, DEFAULT_ACL_MODE);
    apCfgAclTypeSet(unit, bss, DEFAULT_APCFG_ACL);
    apCfgAclClear(unit, bss); 
    
    /* Set Vlan default*/
    apCfgVlanPvidSet(unit, bss, DEFAULT_VLAN_TAG);
    apCfgWmeSet(unit, bss, DEFAULT_WME_MODE);
    apCfgWmmpsSet(unit, bss, DEFAULT_WMMPS_MODE);
    apCfgPrioritySet(unit, bss, DEFAULT_PRIORITY); 
   
    /* Set Balance default */
    apCfgLoadBalanceSet(unit, bss, DEFAULT_BALANCE);
    
    return A_OK;
}

A_STATUS scSetDefaultSecurity(int unit, int bss)
{
    int i;

    apCfgIntraVapForwardingSet(unit, bss, DEFAULT_INTRA_VAP_FORWARDING);
    apCfgAuthTypeSet(unit, bss,APCFG_AUTH_NONE);
    apCfgDefKeySet(unit, bss, DEFAULT_KEY);

    apCfgWPACipherSet(unit, bss, DEFAULT_WPA_CIPHER);
    apCfgPassphraseSet(unit, bss,DEFAULT_PASSPHRASE);
    apCfgGroupKeyUpdateIntervalSet(unit, bss, DEFAULT_GROUP_KEY_UPDATE_INTERVAL);

    /* Do not reset the following settings to default
     * if any vap using wep method, all vaps are sharing one settings */
    for (i = 0; i < WLAN_MAX_VAP; i++) {
        if (apCfgAuthTypeGet(unit, i) == APCFG_AUTH_OPEN_SYSTEM
                || apCfgAuthTypeGet(unit, i) == APCFG_AUTH_SHARED_KEY) {
            return A_OK;
        }
    }
    apCfgKeyBitLenSet(unit, bss, SC_DEFAULT_KEY_BITLEN);
    apCfgKeyValSet(unit, bss, 1, DEFAULT_KEY_VALUE);
    apCfgKeyValSet(unit, bss, 2, DEFAULT_KEY_VALUE);
    apCfgKeyValSet(unit, bss, 3, DEFAULT_KEY_VALUE);
    apCfgKeyValSet(unit, bss, 4, DEFAULT_KEY_VALUE);

    return A_OK;
}
