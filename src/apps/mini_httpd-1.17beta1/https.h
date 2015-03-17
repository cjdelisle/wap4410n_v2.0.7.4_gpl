#ifndef __https_h__
#define __https_h__

#ifdef HAVE_MATRIXSSL
#include "matrixssl_helper.h"
#else /* HAVE_MATRIXSSL */
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif /* HAVE_MATRIXSSL */

#define HTTPS_SSL_FPRINTF_BUF_SIZ		20480
#ifndef DEFAULT_HTTPS_PORT
#define DEFAULT_HTTPS_PORT 				443
#endif
#define DEFAULT_CERTFILE 						"/etc/mini_httpd.pem"

/* https stuff with openssl function */
int https_init(void);
int https_load(int fd);
int https_free(SSL *p);
int https_fgetc(FILE *stream);
char *https_fgets(char *string , int num , FILE *stream);
int https_fputc(int c, FILE *stream);
int https_fputs(const char *s, FILE *stream);
int https_fread(void *ptr,  size_t  size, size_t nmemb, FILE*stream);
int https_fwrite(void *ptr, size_t  size, size_t nmemb, FILE *stream);
int https_fprintf(FILE *stream, const char *format, ...);

#ifdef HTTPS_TEST
int https_test_main(int argc, char *argv[]);
#endif

#endif /* __https_h__ */
