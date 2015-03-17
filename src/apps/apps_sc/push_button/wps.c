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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int main(void)
{
	FILE *fp;
	char cmd[32];
	
	while(1){
printf("open /proc/wps_button\n");
		fp=fopen("/proc/wps_button","r");
		fread(cmd,32,1,fp);
printf("button status is \"%s\"\n",cmd);
		if(strcmp(cmd,"Enable")==0){
			system("/bin/echo 1 > /proc/wsc_pushbutton&");
		}
		fclose(fp);
	}	
	return 0;
}
