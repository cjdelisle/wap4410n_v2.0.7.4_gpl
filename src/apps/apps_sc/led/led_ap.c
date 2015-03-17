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

#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdarg.h>
#include <string.h>

#define WLS_SCAN_INTERVAL       400000      /* 0.4 second */

#define WLS_ERROR       -1
#define WLS_OFF         0
#define WLS_ON          1
#define WLS_TRANSMIT    2

#define POE_ERROR       -1
#define POE_OFF         0
#define POE_ON          1

static unsigned long long wls_packets = 0;
static int wls_status = 0;
static int poe_status = 0;

static char isspace(unsigned char c)
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

static int get_wls_status(int unit) 
{
    FILE *fp;
    char line[1024];
    int is_on = 0;
    unsigned long long packets_tmp=0, cur_packets = 0;
    
    fp=fopen("/proc/net/dev", "r");
	
    if(fp == NULL) {
        return WLS_ERROR;
    }
    
    fgets(line, sizeof(line), fp);	/* eat line */
	fgets(line, sizeof(line), fp);
	
    while (fgets(line, sizeof(line), fp)) {
        char ifname[32];
	    char *s = line;
	    int i = 0;

	    while(*s != ':')
	    {
	        if(!isspace(*s))
	            ifname[i++] = *s;
	        s++;
	    }
	    ifname[i] = 0;
	   
	    if(strncmp(ifname, "ath", 3) != 0)
	        continue;
	        
	    is_on = 1;
	    
	    sscanf(++s, "%*Lu%*Lu%*lu%*lu%*lu%*lu%*lu%*lu%*Lu%Lu%*lu%*lu%*lu%*lu%*lu%*lu",&packets_tmp);
        cur_packets+=packets_tmp;
    }        	   
    fclose(fp);
    
    /*Wireless is off*/
    if(is_on == 0)
        return WLS_OFF;
    
    /*Wireless transmit*/
    if(cur_packets!=wls_packets){
        wls_packets = cur_packets;
        return WLS_TRANSMIT;
    }    
     
    return WLS_ON;
}
#if 0
static int get_poe_status(void) 
{
    FILE *fp = NULL;
    char line[512];
    int poe=POE_ERROR;
    
    fp=fopen("/proc/led","r");

    if(!fp) {
        return POE_ERROR;
    }

    fgets(line, sizeof(line), fp);
    if(!line[0]) {
        fclose(fp);
        return 0;
    }
    sscanf(line, "POE=%d",&poe);
    fclose(fp);
    
    return (poe?POE_ON:POE_OFF);
}
#endif

int main(int argc, char **argv)
{
    int pid;
    int cur_status;
    
    if ((pid = fork()) < 0) {
        exit(-1);
    } else if (pid != 0) {
        exit(0);
    }
    setsid();
    chdir("/");
    umask(0);
    
    while(1){
        
        /*
        * Wireless Led issue
        */
        cur_status = get_wls_status(0);
        
        if(cur_status == WLS_TRANSMIT)
        {
            /*Turn Wireless Blinking*/
            system("echo w4 > /proc/led&");
        }
        else if(cur_status != wls_status)
        {
            switch(cur_status)
            {
                case WLS_OFF:
                    /*Turn wireles off*/
                    system("echo w0 > /proc/led&");
                    break;
                    
                case WLS_ON:
                    system("echo w1 > /proc/led&");
                    break;
                    
                default:
                    break;
            }
        }
        wls_status = cur_status;
        
#if 0        
        /*
        * POE led issue
        */
        cur_status = get_poe_status();
        if(1) //cur_status != poe_status
        {
            switch(cur_status)
            {
                case POE_OFF:
                    /*Turn wireles off*/
                    system("echo p0 > /proc/led&");
                    break;
                    
                case POE_ON:
                    system("echo p1 > /proc/led&");
                    break;
                    
                default:
                    break;
            }
            poe_status = cur_status;
        }
#endif        
        usleep(WLS_SCAN_INTERVAL);
    }

    return 0;
}

