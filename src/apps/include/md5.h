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
#ifndef	__INCmd5h
#define	__INCmd5h

#ifndef A_UINT32
#define A_UINT32 	unsigned long
#endif

struct MD5Context {
        A_UINT32 buf[4];
        A_UINT32 bits[2];
        unsigned char in[64];
};

void MD5Init();
void MD5Update();
void MD5Final();
void MD5Transform();
void md5_calc(unsigned char *output, unsigned char *input, unsigned int inlen);

void generate_wepkey(unsigned char type, char *s, unsigned char *key);

typedef struct MD5Context MD5_CTX;

#endif /* __INCmd5h */
