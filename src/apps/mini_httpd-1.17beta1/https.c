/*
 * HTTPS stuff
 *
 * Copyright 2006, Gemtek Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND GEMTEK GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. GEMTEK
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: https.c,v 1.1 2008-02-27 13:24:33 breeze_zhang Exp $
 */
#ifndef __https_c__
#define __https_c__

#include <stdio.h>
#include <stdarg.h>
#include "matrixssl_helper.h"
#define HTTPS_SSL_FPRINTF_BUF_SIZ		20480


#define SAFE_FREE(p) {\
	if( p != NULL )\
	{\
		free(p);\
		p = NULL;\
	}\
}

static char *ssl_fprintf_buf = NULL ;


#if 0
int do_ssl = 0 ;
static char       *certfile = "/usr/sbin/certSrv.pem" ;
#ifndef MATRIX_SSL
static char       *cipher = 0 ;
#endif /* !MATRIX_SSL */
#ifdef MATRIX_SSL
static sslKeys_t *keys ;
#else /* !MATRIX_SSL */
static SSL_CTX *ssl_ctx = NULL ;
#endif /* MATRIX_SSL */
static SSL        *ssl = NULL ;
static char *ssl_fprintf_buf = NULL ;


int https_init(void)
{
	if( !do_ssl )	/* ssl is disabled */
		return 0 ;
	
	printf("SSL init !!!");
		
#ifdef 	MATRIX_SSL
	if (matrixSslOpen() < 0) {
		printf("matrixSslOpen failed, exiting...\n");
		return -1;
	}
	if( matrixSslReadKeys( &keys, certfile, certfile, NULL, NULL ) < 0 ) {
	    printf("Can't load certificate and/or private key\n");
	    return -1 ;
	}
#else /* !MATRIX_SSL */
	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();
	ssl_ctx = SSL_CTX_new( SSLv23_server_method() );

	if ( certfile[0] != '\0' )
	    if ( SSL_CTX_use_certificate_file( ssl_ctx, certfile, SSL_FILETYPE_PEM ) == 0 ||
		 SSL_CTX_use_PrivateKey_file( ssl_ctx, certfile, SSL_FILETYPE_PEM ) == 0 ||
		 SSL_CTX_check_private_key( ssl_ctx ) == 0 ) {
		printf("SSL_CTX_use_certificate_file fail !!!");
#ifdef HAVE_OPENSSL		
		ERR_print_errors_fp( stderr );
#endif		
		return -1 ;
	}
	printf("certfile=<%s>", certfile);
	
	if ( cipher != (char*) 0 ) {
	    if ( SSL_CTX_set_cipher_list( ssl_ctx, cipher ) == 0 ) {
			printf("SSL_CTX_set_cipher_list fail !!!");
#ifdef HAVE_OPENSSL			
			ERR_print_errors_fp( stderr );
#endif			
			return -1 ;
		}
	}	
	printf("cipher=<%s>", cipher);
#endif /* MATRIX_SSL */	
	
	return 0 ;
}


int https_load(int fd)
{
	printf("do_ssl=%d", do_ssl);
	if( !do_ssl )	/* ssl is disabled */
		return 0 ;	
	
#ifdef MATRIX_SSL
	ssl = SSL_new(keys);
#else /* !MATRIX_SSL */	
	ssl = SSL_new( ssl_ctx );
#endif /* MATRIX_SSL */

	if( NULL == ssl ) return -1 ;
		
	SSL_set_fd( ssl, fd );
	
#ifdef MATRIX_SSL
	if ( SSL_accept( ssl ) <= 0 ) {
#else
	if ( SSL_accept( ssl ) == 0 ) {
#endif /* MATRIX_SSL */		
		printf("SSL_accept fail !!!\n");
#ifdef HAVE_OPENSSL		
	   ERR_print_errors_fp( stderr );
#endif	   
	   return -1 ;
	}
	    	
	return 0 ;
}

#endif

int https_free(SSL *p)
{
	printf("p=%s", (p == NULL)?"NULL":"Not NULL");
	if(p) {	
		SSL_free(p) ;
		p = (SSL *)0 ;
	}
		
	SAFE_FREE(ssl_fprintf_buf);
		
	return 0 ;
}

int https_fgetc(FILE *stream)
{
	if(do_ssl) {	
		char c;
		int r;
		
		r = SSL_read(ssl, &c, 1);
		if( r <= 0 )
			return EOF ;
		else	
			return c ;
	}
	else {
		if(!feof(stream))	
			return fgetc(stream);
		else
			return EOF ;	
	}			
}

char *https_fgets(char *s , int size , FILE *stream)
{
	if(do_ssl) {	
		char *p = s ;
		char c ;
		int r, len =0 ;
		
		while(1) {
			r = SSL_read(ssl, &c, 1);
			if( r <= 0 )
				break;
				
			*p++ = c ;
			len++;
			if( c == '\n' || len == size )
				break;
		}
	
		*p='\0';

		if( !len )
			return NULL;
		else
			return s ;
	}
	else {	
		if(!feof(stream))	
			return fgets(s, size, stream);
		else	
			return NULL ;	
	}	
}

int https_fputc(int c, FILE *stream)
{
	if(do_ssl) {	
		char w ;
		int r;
	
		w = c ;
		r = SSL_write(ssl, &w, 1);
		if( r <= 0 )
				return EOF;
		else
			return c ;
	}
	else {	
		if(!feof(stream))	
			return fputc(c, stream);
		else
			return EOF;	
	}		
}

int https_fputs(const char *s, FILE *stream)
{
	if(do_ssl) {	
		int r;
#ifdef MATRIX_SSL		
		r = SSL_write(ssl, (char *)s, (int)strlen(s));
#else
		r = SSL_write(ssl, s, strlen(s));
#endif /* MATRIX_SSL */
		if( r <= 0 )
			return EOF;	
		else		
			return r;
	}
	else {
		if(!feof(stream))	
			return fputs(s, stream);
		else
			return EOF;			
	}		
}

int https_fread(void *ptr, size_t  size, size_t nmemb, FILE*stream)
{
	if(do_ssl) {	
		int r;
		
		r = SSL_read(ssl, ptr, size*nmemb);
		if( r <= 0 )
			return 0 ;	
		else
			return (r/size) ;
	}
	else {
		if(!feof(stream))	
			return fread(ptr, size, nmemb, stream);
		else
			return 0 ;		
	}	
}

int https_fwrite(void *ptr, size_t  size, size_t nmemb, FILE *stream)
{
	if(do_ssl) {	
		int r;
		
		r = SSL_write(ssl, ptr, size*nmemb);
		if( r <= 0 )
			return 0 ;		
		else	
			return (r/size) ;
	}
	else {
		if(!feof(stream))	
			return fwrite(ptr, size, nmemb, stream);
		else
			return 0 ;			
	}	 			
}

int https_fprintf(FILE *stream, const char *format, ...)
{
	char *buf;
	va_list ap;
	int ret, r ;
                      
   if( ssl_fprintf_buf == NULL )
   	{
   		ssl_fprintf_buf = (char *)malloc(HTTPS_SSL_FPRINTF_BUF_SIZ);
   		if( ssl_fprintf_buf == NULL )
   		{
   			printf("%s", "ssl_fprintf_buf malloc fail !!!");
   			return 0 ;
   		}	
   	}
   	
   buf = ssl_fprintf_buf ;
   	                     
   va_start(ap, format);
   ret = vsprintf(buf, format, ap);
   va_end(ap);

	if( ret < 0 ) {
		return 0 ;
	}
	
	if(do_ssl) {	
		r = SSL_write(ssl, buf, ret);
		if( r <= 0 )
			return 0 ;	
		else	
			return r ;
	}
	else {
		if(!feof(stream))	{
			ret = fprintf(stream, "%s", buf);
			fflush(stream);
			return ret ; 
		}	
		else
			return 0 ;					
	}		
}
#endif /* __https_c__ */
