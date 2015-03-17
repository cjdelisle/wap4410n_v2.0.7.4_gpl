#ifndef __EXPORTS_H__
#define __EXPORTS_H__

#ifndef __ASSEMBLY__

#include <common.h>
#include <net.h>

/* These are declarations of exported functions available in C code */
unsigned long get_version(void);
int  getc(void);
int  tstc(void);
void putc(const char);
void puts(const char*);
void printf(const char* fmt, ...);
void install_hdlr(int, interrupt_handler_t*, void*);
void free_hdlr(int);
void *malloc(size_t);
void free(void*);
void udelay(unsigned long);
unsigned long get_timer(unsigned long);
void vprintf(const char *, va_list);
void do_reset (void);
#if (CONFIG_COMMANDS & CFG_CMD_I2C)
int i2c_write (uchar, uint, int , uchar* , int);
int i2c_read (uchar, uint, int , uchar* , int);
#endif	/* CFG_CMD_I2C */

void app_startup(char **);

#if 1 /*standalone download*/
void dl_GetAddr(unsigned char *node);
int dl_Initialize(void);
int ctrlc (void);
void AssignHWAddress(unsigned char *psBuffer);
int sprintf(char * buf, const char *fmt, ...);
void setenv (char *varname, char *varvalue);
int dl_Transmit(char *buf,int len);
void reset(void);
void * memcpy(void * dest,const void *src,size_t count);
void * memset(void * s,int c,size_t count);
unsigned short sc_xchg( unsigned short dwData);
void dumpData(unsigned char *data, int len);
int FlashDriver(unsigned long dlAddress,unsigned char *dbData,unsigned long dlLength,unsigned long dlFlag);
int memcmp(const void * cs,const void * ct,size_t count);
int saveenv(void);
int NetLoop(proto_t protocol);
void delay_1s(void);
unsigned char * ret_NetTxPacket(void);
#endif

#endif    /* ifndef __ASSEMBLY__ */

enum {
#define EXPORT_FUNC(x) XF_ ## x ,
#include <_exports.h>
#undef EXPORT_FUNC

	XF_MAX
};

#define XF_VERSION	2

#if defined(CONFIG_I386)
extern gd_t *global_data;
#endif

#endif	/* __EXPORTS_H__ */
