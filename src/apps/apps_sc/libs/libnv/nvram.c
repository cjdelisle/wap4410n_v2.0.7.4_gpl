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
#include "nvram.h"
#include <linux/compiler.h>
#ifdef WAG54GX
#include <linux/mtd/mtd.h>
#else
#include <mtd/mtd-user.h>
#endif

int readFileBin(char *path, char **data) {
	int total;
	int fd=0;
	if((fd=open(path, O_RDONLY)) < 0)
	       	return -1;
	
	lockf(fd,F_LOCK,0);
	
	total=lseek(fd,0,SEEK_END);
	lseek(fd,0,0);
	
	if((*data=malloc(total))==NULL){
		lockf(fd,F_ULOCK,0);
	       	return -1;
	}
	if(read(fd,*data,total)<0){ 
		free(*data);
		lockf(fd,F_ULOCK,0);
		return -1;
	}
	
	lockf(fd,F_ULOCK,0);
	close(fd);
   	return total;
}

void writeFileBin(char *path, char *data, int len) {
   int fd;

	if((fd=open(path, O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR)) < 0)
   		return;
	lockf(fd,F_LOCK,0);
	write(fd, data, len);
	lockf(fd,F_ULOCK,0);
	close(fd);
}
      
static unsigned long crc32(char *data, int length)
{
	unsigned long crc, poly;
	long crcTable[256];
	int i, j;
	poly = 0xEDB88320L;
	for (i=0; i<256; i++) {
		crc = i;
		for (j=8; j>0; j--) {
			if (crc&1) {
				crc = (crc >> 1) ^ poly;
			} else {
				crc >>= 1;
			}
		}
		crcTable[i] = crc;
	}
	crc = 0xFFFFFFFF;

	while( length-- > 0) {
		crc = ((crc>>8) & 0x00FFFFFF) ^ crcTable[ (crc^((char)*(data++))) & 0xFF ];
	}
	
	return crc^0xFFFFFFFF;
}


extern int nvram_load(void)
{
	unsigned long crc;
	int fd1;
	char *data;
	
	nvram_header_t header;

	fd1=open(NVRAM_PATH,O_RDONLY);
	if(fd1<0)
		return NVRAM_FLASH_ERR;
	
	read(fd1, &header,sizeof(nvram_header_t));
	
	/*seek to header end*/
	lseek(fd1,NVRAM_HEADER_SIZE,0);
	if(header.magic!=NVRAM_MAGIC)
	{

		close(fd1);
		return NVRAM_MAGIC_ERR;
	}

	data=malloc(header.len+1);	
	read(fd1, data, header.len+1);		
	close(fd1);	
	
	crc=crc32(data, header.len);
	if(crc!=header.crc)
	{
#ifdef TEST 
		printf("CRC Error!!\n");
		printf("header.crc=%lx\tcrc=%lx\n",header.crc,crc);
#endif
		
		free(data);
		return NVRAM_CRC_ERR;
	}
	writeFileBin(NVRAM_TMP_PATH, data, header.len);
	free(data);
	return NVRAM_SUCCESS;
}

int mtd_erase(const char *mtd)
{
	int mtd_fd;
	mtd_info_t mtd_info;
	erase_info_t erase_info;
	
	/* Open MTD device */
	if ((mtd_fd = open(mtd, O_RDWR)) < 0) {
		return 1;
	}

	/* Get sector size */
	if (ioctl(mtd_fd, MEMGETINFO, &mtd_info) != 0) {
		close(mtd_fd);
		return 1;
	}

	erase_info.length = mtd_info.erasesize;

	for (erase_info.start = 0;
	     erase_info.start < mtd_info.size;
	     erase_info.start += mtd_info.erasesize) {
		if (ioctl(mtd_fd, MEMERASE, &erase_info) != 0) {
			close(mtd_fd);
			return 1;
		}
	}

	close(mtd_fd);
	return 0;
}

extern int nvram_commit(void)
{
	int fd1;
	char *data;
	int len;
#if 0	
	char *cmd[512];
#endif
	nvram_header_t header;
#if 0
	sprintf(cmd,"cp -f %s %s\n",NVRAM_TMP_PATH,NVRAM_JFFS2_PATH);
	printf(cmd);
	system(cmd);
		
return NVRAM_SUCCESS;
#endif

#ifdef TEST	
	if(mtd_erase(NVRAM_PATH))
		return NVRAM_FLASH_ERR;
#endif
        //printf("open %s for read/write\n",NVRAM_PATH);
        
	if((fd1=open(NVRAM_PATH,O_WRONLY))<0)
		return NVRAM_FLASH_ERR;
	if((len=readFileBin(NVRAM_TMP_PATH, &data))<=0)
		return NVRAM_SHADOW_ERR;
	
	system("/bin/echo w1>/proc/led");
	header.magic=NVRAM_MAGIC;
	header.crc=crc32(data, len);
	header.len=len; 
	write(fd1, &header,sizeof(nvram_header_t));
	lseek(fd1,NVRAM_HEADER_SIZE,0);
	write(fd1, data, len);
	
// check data 
	lseek(fd1,NVRAM_HEADER_SIZE,0);
	read(fd1, data,len);
        //printf("check crc\n",NVRAM_PATH);
	if(header.crc!=crc32(data, len)){
            //printf("crc incorrect\n",NVRAM_PATH);
		close(fd1);
		free(data);
		system("/bin/echo w0>/proc/led");
		return NVRAM_FLASH_ERR;
	}			
        //printf("crc correct! done\n",NVRAM_PATH);
	close(fd1);
	free(data);
	system("/bin/echo w0>/proc/led");
	return NVRAM_SUCCESS;
}



extern char* nvram_get_fun(const char *name,char *path)
{
	char *bufspace;
	int size;
	char *s,*sp;
	
	if((size=readFileBin(path, &bufspace))<0) 
		return NULL;

	for (s = bufspace; *s; s++) {
		if (!strncmp(s, name, strlen(name)) && *(s+strlen(name))=='=') {
			sp=malloc(strlen(s)-strlen(name));
			memcpy(sp,(s+strlen(name)+1),(strlen(s)-strlen(name)));
			free(bufspace);
			return sp;
		}
		while(*(++s));
	}
	free(bufspace);
	return NULL;
}
/*
 *   write for called by scfmgr_getall
 * */
extern char*  nvram_getall(char *data,int bytes)
{
 	char *bufspace;
	int size;

 	 if((size=readFileBin(NVRAM_TMP_PATH, &bufspace))<0)
                return (char *)NULL;
	 if (size<bytes)
		 bytes=size;
	 memcpy(data,bufspace,bytes);
         free(bufspace);
        return data;
}

extern char* nvram_get(const char *name)
{	
	char *pt;

	if((pt=nvram_get_fun(name,NVRAM_TMP_PATH))==NULL){
			if((pt=nvram_get_fun(name,NVRAM_DEFAULT))!=NULL)
				nvram_set(name,pt);
			else
				return NULL;
	}
	return pt;
}

static char emp_str[]="";

extern char* nvram_safe_get(const char *name)
{
	char *pt;
	if((pt=nvram_get_fun(name,NVRAM_TMP_PATH))==NULL){
			if((pt=nvram_get_fun(name,NVRAM_DEFAULT))!=NULL)
				nvram_set(name,pt);
			else
				pt=strdup(emp_str);
	}
	return pt;

}

extern int nvram_set(const char* name,const char* value)
{
	char *bufspace, *targetspace;
	int size;
	char *sp, *s;
	int found=0;
	
	if((size=readFileBin(NVRAM_TMP_PATH, &bufspace))>0) {
	    targetspace=malloc(size+strlen(name)+strlen(value)+2);
	}
	else {
	    targetspace=malloc(strlen(name)+strlen(value)+2);
	}

	sp=targetspace;
	if(size > 0) {
	   for (s = bufspace; *s; s++) {
		if (!strncmp(s, name, strlen(name)) && *(s+strlen(name))=='=') {
			found=1;
  			strcpy(sp, name);
			sp+=strlen(name);
        		*(sp++) = '=';
       			strcpy(sp, value);
			sp+=strlen(value);		
			while (*(++s));
		}
		while(*s) *(sp++)=*(s++);
	        *(sp++)=END_SYMBOL;
	    }
	
		free(bufspace);
	}
	if(!found){
		strcpy(sp, name);
		sp+=strlen(name);
        	*(sp++) = '=';
	        strcpy(sp, value);
		sp+=strlen(value);
	        *(sp++) = END_SYMBOL;
	}
        
	*(sp) = END_SYMBOL;

	writeFileBin(NVRAM_TMP_PATH, targetspace, (sp-targetspace)+1);
	free(targetspace);
	
	return NVRAM_SUCCESS;
}

