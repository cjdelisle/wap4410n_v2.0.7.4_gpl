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

/* ScMM */
#include <string.h>
#define SCM_MAGIC 0x53634d4d

#define DEFAULT_REMOTE_IP   "192.168.0.1"
#define DEFAULT_REMOTE_PORT 32764 
//#define DEFAULT_REMOTE_PORT 12345 

/* header struct*/
typedef struct scfgmgr_header_s{
	unsigned long   magic;
	int   cmd;
	unsigned long   len;
} scfgmgr_header;

enum {
	SCFG_WARNING=-2,
	SCFG_ERR,
	SCFG_OK,
	SCFG_GETALL,
	SCFG_GET,
	SCFG_SET,
	SCFG_COMMIT,
	SCFG_TEST,
	SCFG_ADSL_STATUS,
	SCFG_CONSOLE,
	SCFG_RECEIVE,
	SCFG_VERSION,
	SCFG_LOCAL_IP,
	SCFG_RESTORE,
	SCFG_CHECKSUM,
	SCFG_CFG_INIT,
}cmd_type;
