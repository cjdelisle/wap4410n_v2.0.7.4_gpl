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

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <syslog.h>
#include <wait.h>	
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "nvram.h"
#include "scfgmgr.h"
#include "socket_tools.h"

int console_mode=1;

int myPipe(char *command, char **output)
{
  FILE *fp;
  char buf[NVRAM_SIZE];
  int len=0;
  
  *output=malloc(1);
  strcpy(*output, "");
  if((fp=popen(command, "r"))==NULL)
     return(-1);
  while((fgets(buf, NVRAM_SIZE, fp)) != NULL){
     len=strlen(*output)+strlen(buf);	  
     if((*output=realloc(*output, (sizeof(char) * (len+1))))==NULL)
     return(-1);
      strcat(*output, buf);
  }
  pclose(fp);
  return len;
}

/* 
 * scfg_init()
 * 
 * Check nvram is exist or not
 * 
 */

int scfg_init()
{
	if(access(NVRAM_TMP_PATH,F_OK)<0)
	{
		puts("No nvram!!\n");
		return -1;
	}
	return 0;
}

/* 
 * scfg_run()
 * 
 * Do command 
 * 
 * Support multi_get if MULTI_GET is defined
 * Support multi_set if MULTI_SET is defined
 *  
 */
void scfg_run(int infd)
{
	scfgmgr_header *header=NULL;	//header get from user
	char *payload=NULL; 		//payload get from user
	char buf[NVRAM_SIZE];		//write buffer
	unsigned int buf_len=0;		//buffer length
	
	console_mode=0;
	
	if(socket_read(&header,&payload,infd)<0){
		goto error;
	}
	switch(header->cmd){
		case SCFG_GETALL:
		{	
			int fd;	
			if((fd=open(NVRAM_TMP_PATH, O_RDONLY))<0){
				goto error;
			}
			buf_len=lseek(fd,0,SEEK_END);
			lseek(fd,0,0);
			if(read(fd,buf,buf_len)<0){
				goto error;
   			}
   			close(fd);
			
			break;
		}
		case SCFG_GET:
		{
#ifdef MULTI_GET
			char *pt,token=END_SYMBOL;
			int v_len,n_len;
#else
			char *pt;
			int v_len;
#endif
			char *value=NULL;
			pt=payload;
#ifdef MULTI_GET
			while(pt && *pt){				
#endif
				value=nvram_get(pt);
				if(value==NULL)
					goto error;
				v_len=strlen(value);
			
#ifdef MULTI_GET
				n_len=strlen(pt);
				
				if(buf_len==0)
					sprintf(buf,"%s=%s",pt,value);		
				else
					sprintf(buf,"%s%c%s=%s%c",buf,token,pt,value,token);
				buf_len+=(v_len+n_len+2);
				free(value);
				pt+=n_len+1;			
			}
			buf[buf_len++]=END_SYMBOL;
#else
			strcpy(buf,value);
			free(value);
			buf_len=v_len+1;
			
#endif
			break;
		}
		case SCFG_SET:
		{
			char *pt;
			char *name;
			char *value;
			pt=payload;
			if(pt && strchr(pt,'=')==NULL){
				goto error;
			}
#ifdef MULTI_SET
			while(*pt){
#endif
				strcpy(buf,pt);
				name=strtok(buf,"=");
				value=(strtok(NULL,"")?:"");
/* Mike@CPU_AP add for send syslog */
extern int cfgmylog(const char *format, ...);
/* add end */
				if(name!=NULL)
/* Mike@CPU_AP add for send syslog */
    			{			
                    cfgmylog("<%s,%d>\n", __FUNCTION__, __LINE__);
    			    syslog(LOG_INFO,"[SC][CFG_Change]%s is change to %s",name,value);   			
/* add end */
				    nvram_set(name,value);
/* Mike@CPU_AP add for send syslog */
				}
/* add end */				
				else 
					goto error;

#ifdef MULTI_SET
				while(*pt++);
			}
#endif
			break;
		}
		case SCFG_COMMIT:
		{
			if(nvram_commit()<0)
				goto error;
			
			break;
		}
		case SCFG_TEST:
		{
			/* just support wan */
			nvram_set("wan_mode","bridgedonly");
			nvram_set("wan_encap","0");
			nvram_set("wan_vpi","8");
			nvram_set("wan_vci","81");
			system("/usr/bin/killall br2684ctl");
			system("/usr/sbin/rc wan restart&");
			system("/usr/bin/killall udhcpd");
			buf_len=0;
	
			break;
		}	
		case SCFG_ADSL_STATUS:
		{
			int fd;
			char *pt;
			int downstream=0;
			int upstream=0;
			
			console_mode=1;
			if((fd=open("/proc/avalanche/avsar_modem_stats", O_RDONLY))<0){
				goto error;
			}
			buf_len=lseek(fd,0,SEEK_END);
			lseek(fd,0,0);
			if(read(fd,buf,buf_len)<0){
				goto error;
   			}
   			close(fd);
			if((pt=strstr(buf,"US"))){
				sscanf(pt,"%*[^:]: %d %*[^:]: %d",&upstream,&downstream);
			}
			sprintf(buf,"%dKbps/%dKbps",downstream,upstream);
			buf_len=strlen(buf)+1;
			
			break;
		}
		case SCFG_CONSOLE:
		{
			char *data;
			if(strcmp(payload,"exit")==0 ||
				strcmp(payload,"quit")==0 ||
				strcmp(payload,"bye")==0 ){
				console_mode=0;
			}else{
				console_mode=1;
				if(strncmp(payload,"cd ",3)==0){
					char mytmp[1024];
					chdir(payload+3);
					getcwd(mytmp,1024);
					sprintf(buf,"cd %s",mytmp);
					buf_len=strlen(buf)+1;
					break;	
				}
				myPipe(payload,&data);
				if(data!=NULL){
					strcpy(buf,data);
				        buf_len=strlen(buf)+1;
					free(data);
				}
			}
				
			break;	
		}
		case SCFG_RECEIVE:
		{
			char tmp[128];
			char filename[128];
			char *pt;
			int i=0;
			int fd;
			pt=payload;
			/* parser file name */
			while(*pt!='\0' && i <128) tmp[i++]=*pt++;
			tmp[i]='\0';
			sprintf(filename,"/tmp/%s",tmp);
			fd=open(filename,O_CREAT | O_TRUNC| O_WRONLY,S_IRWXU|S_IRWXG|S_IRWXO);
			if(fd<0) goto error;
			write(fd,pt+1,header->len-strlen(tmp)-1);
			close(fd);
			break;
		}
		case SCFG_VERSION:
		{
			FILE *fp;
			char annex;
			int a=0;
			int b=0;
			int c=0;
			
			console_mode=1;
			fp=fopen("/etc/version","r");
			fread(buf,128,1,fp);
			fclose(fp);
			sscanf(buf,"%c%d.%d.%d",&annex,&a,&b,&c);
			sprintf(buf,"%c.%02x%02x%02x",annex,a,b,c);
			buf_len=strlen(buf)+1;			
			break;
		}
		case SCFG_LOCAL_IP:
		{
			console_mode=1;
			strcpy(buf,nvram_get("lan_ipaddr"));
			buf_len=strlen(buf)+1;			
			break;
		}
		
		case SCFG_RESTORE:
		{
			console_mode=1;
			buf_len=0;
			nvram_set("restore_defaults","1");
			nvram_commit();
			break;
		}
		case SCFG_CHECKSUM:
		{	
			FILE *fp;
			unsigned short checksum;
			fp=fopen("/dev/mtdblock/0","r");
			if(fp==NULL)
				goto error;
			fseek(fp,-4,SEEK_END);
			fread(&checksum,sizeof(unsigned short),1,fp);
			fclose(fp);
			sprintf(buf,"%x",checksum);
			buf_len=strlen(buf)+1;
			console_mode=1;
			break;
		}
		case SCFG_CFG_INIT:
		{
			nvram_load();
			nvram_commit();
			break;
		}

		default:
		{
error:	
			header->magic=SCM_MAGIC;	
			header->cmd=SCFG_ERR;
			header->len=buf_len;
			if(header->len==0)
				socket_write(header,NULL,infd);
			else
				socket_write(header,buf,infd);
			if(!header)     free(header);
		        if(!payload)    free(payload);
			return; 
		}
	}
	/* OK */	
	header->magic=SCM_MAGIC;	
	header->cmd=SCFG_OK;
	header->len=buf_len;
	socket_write(header,buf,infd);
	
	if(!header)	free(header);	
	if(!payload)	free(payload);	
}

int main(int argc,char *argv[])
{
	int sockfd;
	int infd;
	pid_t pid;
	struct sockaddr_in srv;
	socklen_t socklen;
	if(fork()!=0) exit(0);

	chdir("/");
	umask(0);
	
	if(scfg_init()<0)
	{
		exit(1);
	}

	/* creat socket */
	if((sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
	{
		perror("socket");
		exit(1);
	}
	memset(&srv,0,sizeof(srv));
	srv.sin_family = AF_INET;
	srv.sin_port = htons(DEFAULT_REMOTE_PORT);
	if((bind(sockfd,(struct sockaddr *)&srv,sizeof(srv)))<0)
	{
		perror("bind");
		exit(1);
	}
	
	if((listen(sockfd,5))<0)
	{
		perror("listen");
		exit(1);
	}
	
	while((infd=accept(sockfd,(struct sockaddr *)&srv,&socklen))>=0)
	{
		if((pid=fork())<0)
		{
			perror("fork");
			exit(1);
		}
		else if(pid==0)
		{	
			/* only allow connect SOCKET_WAIT_TIME (sec) */
			alarm(SOCKET_WAIT_TIME);			
			/* do command */
			while(console_mode)
			{
				scfg_run(infd);
				alarm(0);
			}
			exit(0);
		}
		else
		{
			int status;
			/*only allow one connection ,change it for multi-connect */
			waitpid(pid,&status,WUNTRACED);
		}		
		close(infd);
	}
	exit(0);
}
