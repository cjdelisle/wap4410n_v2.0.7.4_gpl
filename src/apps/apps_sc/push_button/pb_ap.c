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


#define timerinterval(tvp, uvp)     \
( ((tvp)->tv_sec - (uvp)->tv_sec)*1000000 + ((tvp)->tv_usec - (uvp)->tv_usec) )

enum{
    STATE_NORMAL,
    STATE_PRESSED
};

static int ButtonIsPushed(void) {
    FILE *fp;
    char cmd[32];

    fp=fopen("/proc/push_button","r");

    if(!fp) {
        return 0;
    }

    fread(cmd,1,sizeof(cmd)-1,fp);
    if(!cmd[0]) {
        fclose(fp);
        return 0;
    }
    if( !memcmp(cmd, "RST ", 4) ) {
        /* RESET BUTTON is pushed */
        fclose(fp);
        return 1;
    }
    fclose(fp);
    return 0;
}


#define SCAN_INTERVAL       400000      /* 0.4 second */
#define TEST_INTERVAL       1000000     /* 1 second   */
#define REBOOT_INTERVAL     1000000     /* 1 second   */
#define RESTORE_INTERVAL    4000000     /* 4 second   */


int main(int argc, char **argv)
{
    struct timeval beginTime, endTime;
    int            state       = STATE_NORMAL;
    time_t         interval;
    int            pid;
    int 		   flag;	    

    if ((pid = fork()) < 0) {
        exit(-1);
    } else if (pid != 0) {
        exit(0);
    }
    setsid();
    chdir("/");
    umask(0);

    /* Process support reset. */
    state = STATE_NORMAL;
    while(1){

        if( ButtonIsPushed() ) {
            if( state == STATE_NORMAL){
                gettimeofday(&beginTime,NULL);
                state = STATE_PRESSED;
                flag = 0;
            } else if(state == STATE_PRESSED) {
            	gettimeofday(&endTime, NULL);
            	interval = timerinterval(&endTime, &beginTime);
            	if (!flag && interval > RESTORE_INTERVAL ) {
            		printf("Please release the button for doing restore.\n");
            		system("echo \"t2\" > /proc/led&");
            		flag = 1;
            	}
            }
        }else {
            /* button is released */
            if(state ==  STATE_PRESSED) {        	
                gettimeofday(&endTime,NULL);
                state = STATE_NORMAL;
                interval = timerinterval(&endTime, &beginTime);

                if (interval > RESTORE_INTERVAL ){
                    /* do restore */
                    printf("Do restore ... \n");
        			system("echo \"d100\" > /proc/led&");
					system("/usr/sbin/nvram set restore_defaults=1");			
					system("echo \"d1\" > /proc/led&");
					system("/sbin/reboot &");
					exit(0);
        		}
        		else if (interval > REBOOT_INTERVAL ) {
        		    printf("Do reboot... \n");
					system("echo \"r100\" > /proc/led&");
					system("/sbin/reboot &");
					exit(0);        			
        		}
        		else {
        		    printf("Do test... \n");
					system("echo \"t\" > /proc/led&");
                }

            }

        }
        usleep(SCAN_INTERVAL);
    }

    return 0;
}

