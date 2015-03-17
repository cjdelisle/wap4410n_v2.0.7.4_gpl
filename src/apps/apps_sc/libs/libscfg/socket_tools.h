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
#ifndef _SOCKETTOOLS_
#define _SOCKETTOOLS_

/* path of socket log */
#ifndef SOCKET_LOG
#define SOCKET_LOG  "log"
#endif
#include "socket_header.h"
int socket_connect();
/*
 * Read data from socket
 * @param	header	like packet's header
 * @param	data	save data in here
 * @param	infd	write data to here
 * @return	0 success -1 error
 */
int socket_read(scfgmgr_header **header,char **data,int infd);

/*
 * Write data to socket
 * @param	header	like packet's header
 * @param	data	data that you want write to socket
 * @param	infd	read data from here
 * @return	0 success -1 error
 */
int socket_write(scfgmgr_header *header,char *data,int infd);

#ifdef TEST
/*
 * Save log
 * @param	msg	message 
 * @param	len	message length
 */
void socket_log(char *msg,int len);
#endif

/*
 *  The fllowing funtions are designed for socket client AP , ex: CGI  
 */
#ifdef SCFG_CLIENT
/*
 * Communication with scfgmgr   
 * @param	shd	header for write 
 * @param	sdata	data for write
 * @param	rhd	header for read   
 * @param	rdata	data for read
 * @return	0 success -1 error
 */
int scfgmgr_connect(scfgmgr_header *shd,char *sdata,scfgmgr_header **rhd,char **rdata);

/*
 * Send command to scfgmgr
 * @param       cmd   command  
 * @return      0 success -1 error
 */
int scfgmgr_cmd(int cmd,char **rdata);
#define scfgmgr_commit() {char *tmp;scfgmgr_cmd(SCFG_COMMIT,&tmp);}
/*
 * Get all configuration data from scfgmgr
 * @param       rdata   save data in this point
 * @return      0 success -1 error
 */
int scfgmgr_getall(char **rdata);	


int scfgmgr_get(char *data,char **rdata);	

/*
 * Save configuration data to scfgmgr
 * @param       data     data ,you want save
 * @param       value    value
 * @return      0 success -1 error
 */
int scfgmgr_set(char *name,char *value);

int scfgmgr_sendfile(char *data,int len);
int scfgmgr_console(char *data,char **rdata);
/*
 * Parse value form data
 * @param       name     parse this name's value
 * @return      value 
 */
char* value_parser(char *name);
#endif

#endif
