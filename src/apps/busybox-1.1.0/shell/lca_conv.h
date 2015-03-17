#ifndef _LCA_CONV_H_
#define _LCA_CONV_H_
#define UTF2LAN	0
#define LAN2UTF	1	
size_t do_convert( int type,char *inbuf, size_t inbytesleft,char *outbuf, size_t outbytesleft);
#endif
