/*
 * WPA Supplicant - ASCII passphrase to WPA PSK tool
 * Copyright (c) 2003-2005, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"

#include "common.h"
#include "sha1.h"

#define WDS_PASSPHRASE "Ralink"

int main(int argc, char *argv[])
{
	unsigned char psk[32];
	unsigned char psphrase[64+8];
	int i;
	/*char *ssid, *passphrase, buf[64], *pos;*/
    char *passphrase;
    
	if (argc != 2) {
		printf("usage: wds_passphrase <passphrase>\n");
		return 1;
	}

	passphrase = argv[1];

#if 1
    if(strlen(passphrase) < 8){
        sprintf(psphrase, "Linux26%s", passphrase);
    }
    else if(strlen(passphrase) > 63)
    {
        sprintf(psphrase, "%s", passphrase);
        psphrase[63] = '\0';
    }
    else{
        sprintf(psphrase, "%s", passphrase);
    }
    passphrase = psphrase;
#endif

	if (strlen(passphrase) < 8 || strlen(passphrase) > 63) {
		printf("Passphrase must be 8..63 characters\n");
		return 1;
	}

	pbkdf2_sha1(passphrase, WDS_PASSPHRASE, sizeof(WDS_PASSPHRASE), 4096, psk, 32);
    memcpy(psk+24, psk+16, 8);
    
	for (i = 0; i < 32; i++)
		printf("%02x", psk[i]);

	return 0;
}
