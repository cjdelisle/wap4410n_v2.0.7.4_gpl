/* files.h */
#ifndef _FILES_H
#define _FILES_H

#define MAX_INTERFACES	2
/* Ron */
//#define MAX_KEYWORDS	18   
#define MAX_KEYWORDS	19+1   // add static release
#include "packet.h"
struct config_keyword {
	char keyword[14];
	int (*handler)(char *line, void *var);
	void *var;
	char def[30];
};

struct config_kw_arr {
	struct config_keyword keywords[MAX_KEYWORDS];
};

void reconfig_dhcpd(int ifid);
int read_config(char *file);
void write_leases(int ifid);
void read_leases(char *file, int ifid);
#endif
