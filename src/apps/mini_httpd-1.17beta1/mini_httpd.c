/* mini_httpd - small HTTP server
**
** Copyright ?1999,2000 by Jef Poskanzer <jef@acme.com>.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/


#include "version.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/sysinfo.h>
#include "port.h"
#include "match.h"
#include "tdate_parse.h"
#include "nvram.h"
//#include "utility.h"

#define LOG_ENANLE "/tmp/log_enable"

#define _USE_COOKIES_ 1 //add by carole


#ifdef HAVE_SENDFILE
# ifdef HAVE_LINUX_SENDFILE
#  include <sys/sendfile.h>
# else /* HAVE_LINUX_SENDFILE */
#  include <sys/uio.h>
# endif /* HAVE_LINUX_SENDFILE */
#endif /* HAVE_SENDFILE */

#ifdef USE_SSL
#ifdef MATRIX_SSL
#include "matrixssl_helper.h"	
#endif
#ifdef OPEN_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif
#endif /* USE_SSL */

//extern char* crypt( const char* key, const char* setting );


#if defined(AF_INET6) && defined(IN6_IS_ADDR_V4MAPPED)
#define USE_IPV6
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

#ifndef SHUT_WR
#define SHUT_WR 1
#endif

#ifndef SIZE_T_MAX
#define SIZE_T_MAX 2147483647L
#endif

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif


#ifndef ERR_DIR
#define ERR_DIR "."
#endif /* ERR_DIR */
#ifndef DEFAULT_HTTP_PORT
#define DEFAULT_HTTP_PORT 80
#endif /* DEFAULT_HTTP_PORT */
#ifdef USE_SSL
#ifdef MATRIX_SSL
#ifndef DEFAULT_HTTPS_PORT
#define DEFAULT_HTTPS_PORT 443
#endif /* DEFAULT_HTTPS_PORT */
#ifndef DEFAULT_CERTFILE
#define DEFAULT_CERTFILE "/var/certSrv.pem"
#endif /* DEFAULT_CERTFILE */
#ifndef DEFAULT_KEYFILE
#define DEFAULT_KEYFILE "/var/privkeySrv.pem"
#endif /* DEFAULT_KEYFILE */
#elif defined (OPEN_SSL)
#ifndef DEFAULT_HTTPS_PORT
#define DEFAULT_HTTPS_PORT 443
#endif /* DEFAULT_HTTPS_PORT */
#ifndef DEFAULT_CERTFILE
#define DEFAULT_CERTFILE "/usr/sbin/mini_httpd.pem"
#endif /* DEFAULT_CERTFILE */
#endif
#endif /* USE_SSL */
#ifndef DEFAULT_USER
#define DEFAULT_USER "root"
#endif /* DEFAULT_USER */
#ifndef CGI_NICE
#define CGI_NICE 10
#endif /* CGI_NICE */
#ifndef CGI_PATH
#define CGI_PATH "./"
#endif /* CGI_PATH */
#ifndef CGI_LD_LIBRARY_PATH
#define CGI_LD_LIBRARY_PATH "/lib:/usr/local/lib:/usr/lib"
#endif /* CGI_LD_LIBRARY_PATH */
#ifndef AUTH_FILE
#define AUTH_FILE ".htpasswd"
#endif /* AUTH_FILE */
#ifndef TIMEOUT
#define TIMEOUT 60
#endif /* TIMEOUT */
#ifndef DEFAULT_CHARSET
#define DEFAULT_CHARSET "iso-8859-1"
#endif /* DEFAULT_CHARSET */


#define METHOD_UNKNOWN 0
#define METHOD_GET 1
#define METHOD_HEAD 2
#define METHOD_POST 3

#define SYSLOG 1				     

/* A multi-family sockaddr. */
typedef union {
    struct sockaddr sa;
    struct sockaddr_in sa_in;
#ifdef USE_IPV6
    struct sockaddr_in6 sa_in6;
    struct sockaddr_storage sa_stor;
#endif /* USE_IPV6 */
    } usockaddr;


static char* argv0;
static int debug;
static int port;
static int ports;
static char* dir;
static int do_chroot;
static int vhost;
static char* user;
static char* cgi_pattern;
static char* url_pattern;
static int no_empty_referers;
static char* local_pattern;
static char* hostname;
static char hostname_buf[500];
static char* logfile;
static char* pidfile;
static char* charset;
static char* p3p;
static int max_age;

/* 
* By MD@SC_CPUAP for autohttps  2007-7-31
* Added Begin
*/
static int autohttps;
/* 
* Added End
*/
static int autohttp;
static int httpport;
static FILE* logfp;
static int listen4_fd, listen6_fd;
#ifdef USE_SSL
	static int do_ssl;
	static int do_both;
#ifdef MATRIX_SSL
    static int listen4s_fd, listen6s_fd, fd, fd6;
    static char* certfile;
	static char *cipher;
	sslKeys_t *keys;
	sslConn_t *cpssl;
	unsigned char *c;
	int acceptAgain, flags;
	int	quit, again, rc, err;
#elif defined (OPEN_SSL)
	static char* certfile;
	static char* cipher;
	static SSL_CTX* ssl_ctx;
#endif
#endif /* USE_SSL */

static char cwd[MAXPATHLEN];

/* Ron */
#define NEEDCHECKPW kill(getppid(),SIGINT)
#define LOGOUT kill(getppid(),SIGHUP)
#define SAVETIME kill(getppid(),SIGUSR2)
static char *http_realm=NULL; /* Auth title */
static int  web_timeout=30*60; /* timeout */
static time_t last_access_time=0; /* last access time*/
static int someone_in_use=0;  /*1=someone in use, 0=nobody*/
static char remote_ip[256]=""; /* current user's ip */
static char last_remote_ip[256]=""; /*lastest user's ip*/
static char *rondir;
static int still_timeout=0;
static int not_auth=0;
#if 0
#define NVRAM_GET_TEMP  "/tmp/nvram_get"

static char emp_str[]="";
char *nvram_safe_get(char *name)
{
    char cmd_str[128]={0}, result[256]={0};
    char *pt, *p=NULL;
    FILE *fp=NULL;

    sprintf(cmd_str, "/usr/sbin/nvram get %s >%s 2>/dev/null", NVRAM_GET_TEMP);
    system(cmd_str);
    fp=fopen(NVRAM_GET_TEMP, "rt");
    if(!fp){
        pt=strdup(emp_str);
        return pt;
    }
    fgets(result, sizeof(result), fp);
    fclose(fp);
    p=strchr(result, '\n');
    if(p)
        *p=0;
    p=strchr(result, '=');
    if(!p){
        pt=strdup(emp_str);
        return pt;
    }
    *p++=0;
    pt=strdup(emp_str);
    return pt;        
}
#endif
int get_sockfd()
{
	int sockfd = -1;
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("user: socket creating failed");
		return (-1);
	}
	return sockfd;
}
int SYSTEM(const char *format, ...) 
{
    char buf[1024]="";
    va_list arg;

    va_start(arg, format);
    vsnprintf(buf,1024, format, arg);
    va_end(arg);

    system(buf);
    usleep(1);
    return 0;
}

void scToLows(unsigned char *charStr)
{
	int i,len = strlen((char*)charStr);
	for(i=0;i<len;i++){
		if(charStr[i]>='A' && charStr[i]<='Z')
			charStr[i]+= 'a'-'A';
	}	
}

int sc_dbg(const char *format, ...)
{
    va_list args;
    FILE *fp;

    if (access("/tmp/sc_dbg", F_OK) != 0) {
        return 0;
    }

    fp = fopen("/dev/ttyS0", "a+");

    if (!fp) {
        fprintf(stderr, "fp is NULL\n");
	    return -1;
    }

    va_start(args, format);
    vfprintf(fp, format, args);
    va_end(args);

    fclose(fp);
    return 0;
}
static void handle_chdir( int sig )
{
    chdir(rondir?:"/www");    
    (void) getcwd( cwd, sizeof(cwd) - 1 );
    if ( cwd[strlen( cwd ) - 1] != '/' )
	(void) strcat( cwd, "/" );
}

static void handle_logout( int sig )
{
	/* logout */
	last_access_time=0;
	still_timeout=0;
	if(access(LOG_ENANLE, F_OK) == 0){
			 unlink(LOG_ENANLE);
	}
}
  
static void handle_web_time( int sig )
{
	struct sysinfo info;
	sysinfo(&info);
	last_access_time=info.uptime;
}
static int check_timeout(void)
{
	struct sysinfo info;
	sysinfo(&info);
	if(strcmp(last_remote_ip,remote_ip)==0 && still_timeout==1){
		usleep(1);
		return 1;
	}

	if(last_access_time==0){
		usleep(1);
		return 0;
	}
	else if(((info.uptime-last_access_time)>web_timeout) && web_timeout!=0)
	{
		usleep(1);
		return 1;
	}
	return 0;
}
static void handle_timeout_stat(int sig)
{
	/* enter timeout stat */
	still_timeout=1;
}

/* Ron */

/* Request variables. */
static int conn_fd;
#ifdef USE_SSL
#ifdef OPEN_SSL
static SSL* ssl;
#endif
#endif /* USE_SSL */
static usockaddr client_addr;
static char* request;
static size_t request_size, request_len, request_idx;
static int method;
static char* path;
/* 
* By MD@SC_CPUAP for autohttps  2007-7-31
* Added Begin
*/
static char scpath[100];
/* 
* Added End
*/
static char fakepath[128]="";
static char* file;
static char* pathinfo;
struct stat sb;
static char* query;
static char* protocol;
static int status;
static off_t bytes;
static char* req_hostname;

static char* authorization;
static size_t content_length;
static char* content_type;
static char* cookie;
static char* host;
static time_t if_modified_since;
static char* referer;
static char* useragent;

static char* remoteuser;
/* Forwards. */
//static void usage( void );
static void read_config( char* filename );
static void value_required( char* name, char* value );
static void no_value_required( char* name, char* value );
static int initialize_listen_socket( usockaddr* usaP );
static void handle_request( void );
static void de_dotdot( char* file );
static int get_pathinfo( void );
static void do_file( void );
//static void do_dir( void );
#ifdef HAVE_SCANDIR
static char* file_details( const char* dir, const char* name );
static void strencode( char* to, size_t tosize, const char* from );
#endif /* HAVE_SCANDIR */
static void do_cgi( void );
static void cgi_interpose_input( int wfd );
static void post_post_garbage_hack( void );
static void cgi_interpose_output( int rfd, int parse_headers );
static char** make_argp( void );
static char** make_envp( void );
static char* build_env( char* fmt, char* arg );
static void auth_check( char* dirname );
static void send_authenticate( char* realm );
static void send_authenticateTimeout( char* realm );
static void send_authenticateWarning( char* realm );
static void sendAutoHttps(char* path);
static void sendAutoHttp(char* path);
static char* virtual_file( char* file );
static void send_error( int s, char* title, char* extra_header, char* text );
static void send_error_body( int s, char* title, char* text );
static int send_error_file( char* filename );
static void send_return(int type);//carole
static void send_error_tail( void );
static void add_headers( int s, char* title, char* extra_header, char* me, char* mt, off_t b, time_t mod );
static void start_request( void );
static void add_to_request( char* str, size_t len );
static char* get_request_line( void );
static void start_response( void );
static void add_to_response( char* str, size_t len );
static void send_response( void );
static void send_via_write( int fd, off_t size );
static ssize_t my_read( char* buf, size_t size );
static ssize_t my_write( char* buf, size_t size );
#ifdef HAVE_SENDFILE
static int my_sendfile( int fd, int socket, off_t offset, size_t nbytes );
#endif /* HAVE_SENDFILE */
static void add_to_buf( char** bufP, size_t* bufsizeP, size_t* buflenP, char* str, size_t len );
static void make_log_entry( void );
static void check_referer( void );
static int really_check_referer( void );
static char* get_method_str( int m );
static void init_mime( void );
static char* figure_mime( char* name, char* me, size_t me_size );
static void handle_sigterm( int sig );
static void handle_sigchld( int sig );
static void handle_sigalrm( int sig );
static void lookup_hostname( usockaddr* usa4P, size_t sa4_len, int* gotv4P, usockaddr* usa6P, size_t sa6_len, int* gotv6P );
static char* ntoa( usockaddr* usaP );
static int sockaddr_check( usockaddr* usaP );
static size_t sockaddr_len( usockaddr* usaP );
static void strdecode( char* to, char* from );
static int hexit( char c );
static int b64_decode( const char* str, unsigned char* space, int size );
static void set_ndelay( int fd );
static void clear_ndelay( int fd );
static void* e_malloc( size_t size );
static void* e_realloc( void* optr, size_t size );
static char* e_strdup( char* ostr );
#ifdef NO_SNPRINTF
static int snprintf( char* str, size_t size, const char* format, ... );
#endif /* NO_SNPRINTF */

int
main( int argc, char** argv )
{
    int argn;
    uid_t uid = 32767;
    usockaddr host_addr4;
    usockaddr host_addr6;
    usockaddr host_addr4s;    
    usockaddr host_addr6s;    
    int gotv4, gotv6, gotv4s ,gotv6s;
    fd_set lfdset;
    int maxfd;
    usockaddr usa,usa80;
    int sz, r ;
    char* cp;
    FILE *wl_p;
    char *p;
    int wlan_mgr;
    char delifPath[256];
    /*int remote_manage;*/
    int unit,vap;
    char wl_tmp[256];
    char wl_ipaddr[17];
    static char wl_mac[19];
    int wl_mac_match=0;
    struct sysinfo info;

    /* Parse args. */
    argv0 = argv[0];
    debug = 0;
    port = -1;
    ports = -1;
    dir = (char*) 0;
    do_chroot = 0;
    vhost = 0;
    cgi_pattern = (char*) 0;
    url_pattern = (char*) 0;
    no_empty_referers = 0;
    local_pattern = (char*) 0;
    charset = DEFAULT_CHARSET;
    p3p = (char*) 0;
    max_age = -1;
    user = DEFAULT_USER;
    hostname = (char*) 0;
    logfile = (char*) 0;
    pidfile = (char*) 0;
    logfp = (FILE*) 0;
#ifdef USE_SSL
    do_ssl = 0;
    do_both = 0;
#if defined(OPEN_SSL) || defined(MATRIX_SSL)       
    certfile = DEFAULT_CERTFILE;
    cipher = (char*) 0;
#endif  
#endif /* USE_SSL */
    argn = 1;
    while ( argn < argc && argv[argn][0] == '-' )
	{
	if ( strcmp( argv[argn], "-V" ) == 0 )
	    {
	    (void) printf( "%s\n", SERVER_SOFTWARE );
	    exit( 0 );
	    }
	else if ( strcmp( argv[argn], "-C" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    read_config( argv[argn] );
	    }
	else if ( strcmp( argv[argn], "-D" ) == 0 )
	    debug = 1;
#ifdef USE_SSL
	else if ( strcmp( argv[argn], "-S" ) == 0 )
	    do_ssl = 1;
/* 
* By MD@SC_CPUAP for autohttps  2007-7-31
* Added Begin
*/
	else if ( strcmp( argv[argn], "-A" ) == 0 )
	    autohttps = 1;	     
/* 
* Added End
*/	    
	else if ( strcmp( argv[argn], "-B" ) == 0 )
	    {
	    do_both = 1;
	    ++argn;
	    ports = atoi(argv[argn]);
	    }
	else if ( strcmp( argv[argn], "-E" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
#if defined(OPEN_SSL) || defined(MATRIX_SSL)   
	    certfile = argv[argn];
#endif  
        }
	else if ( strcmp( argv[argn], "-Y" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    cipher = argv[argn];
	    }
#endif /* USE_SSL */
	else if ( strcmp( argv[argn], "-p" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    port = atoi( argv[argn] );
	    }
/* Add by archer for autohttp */
	else if ( strcmp( argv[argn], "-a" ) == 0 )
	    autohttp = 1;	
/* Add end */
	else if ( strcmp( argv[argn], "-d" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    dir = argv[argn];
	    rondir = strdup(argv[argn]);
	    }
	else if ( strcmp( argv[argn], "-c" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    cgi_pattern = argv[argn];
	    }
	else if ( strcmp( argv[argn], "-u" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    user = argv[argn];
	    }
	else if ( strcmp( argv[argn], "-h" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    hostname = argv[argn];
	    }
	else if ( strcmp( argv[argn], "-r" ) == 0 ){
	    //do_chroot = 1;
	    ++argn;
	    if(argv[argn]!=NULL)
	 	   http_realm=argv[argn];	
	}
	else if ( strcmp( argv[argn], "-v" ) == 0 )
	    vhost = 1;
	else if ( strcmp( argv[argn], "-l" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    logfile = argv[argn];
	    }
	else if ( strcmp( argv[argn], "-i" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    pidfile = argv[argn];
	    }
	else if ( strcmp( argv[argn], "-t" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    web_timeout = atoi(argv[argn]);
	    }
	else if ( strcmp( argv[argn], "-T" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    charset = argv[argn];
	    }
	else if ( strcmp( argv[argn], "-P" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    p3p = argv[argn];
	    }
	else if ( strcmp( argv[argn], "-M" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    max_age = atoi( argv[argn] );
	    }
	else
	    exit(1);//usage();
	++argn;
	}
    sc_dbg("%s:%s():%d\n", __FILE__, __FUNCTION__, __LINE__);
/* 
* By MD@SC_CPUAP for autohttps  2007-7-31
* Added Begin
*/	
		if(autohttps || autohttp)
		{
			do_both = 1;
		}
/* 
* Added End
*/		
    if ( argn != argc )
	exit(1);//usage();

    cp = strrchr( argv0, '/' );
    if ( cp != (char*) 0 ) 
	++cp;
    else
	cp = argv0;
//    openlog( cp, LOG_NDELAY|LOG_PID, LOG_DAEMON );
    openlog( "auth", 0 , LOG_AUTH );

    sc_dbg("%s:%s():%d\n", __FILE__, __FUNCTION__, __LINE__);
    if ( port == -1 )
	{
#ifdef USE_SSL
#if defined(OPEN_SSL) || defined(MATRIX_SSL)   
	if ( do_ssl && !do_both)
	    port = DEFAULT_HTTPS_PORT;
	else
	    port = DEFAULT_HTTP_PORT;
#endif
#else /* USE_SSL */
	port = DEFAULT_HTTP_PORT;
#endif /* USE_SSL */
	}
    sc_dbg("%s:%s():%d\n", __FILE__, __FUNCTION__, __LINE__);

    /* Log file. */
    if ( logfile != (char*) 0 )
	{
	logfp = fopen( logfile, "a" );
	if ( logfp == (FILE*) 0 )
	    {
#ifdef SYSLOG
	    syslog( LOG_CRIT, "%s - %m", logfile );
	    perror( logfile );
#endif
	    exit( 1 );
	    }
	}

    /* Look up hostname. */
    sc_dbg("%s:%s():%d\n", __FILE__, __FUNCTION__, __LINE__);
#ifdef USE_SSL
    if(do_both)
    {
        lookup_hostname(
    	&host_addr4, sizeof(host_addr4), &gotv4,
    	&host_addr6, sizeof(host_addr6), &gotv6 );
    	
    	httpport = port;
        port = ports;
        lookup_hostname(
    	&host_addr4s, sizeof(host_addr4s), &gotv4s,
    	&host_addr6s, sizeof(host_addr6s), &gotv6s );
    }else if(do_ssl)
    {
        lookup_hostname(
    	&host_addr4s, sizeof(host_addr4s), &gotv4s,
    	&host_addr6s, sizeof(host_addr6s), &gotv6s );
    }else
#endif
    lookup_hostname(
	&host_addr4, sizeof(host_addr4), &gotv4,
	&host_addr6, sizeof(host_addr6), &gotv6 );
    
    if ( hostname == (char*) 0 )
	{
	(void) gethostname( hostname_buf, sizeof(hostname_buf) );
	hostname = hostname_buf;
	}
    if ( ! ( gotv4 || gotv6 || gotv4s ||gotv6s ) )
	{
#ifdef SYSLOG
	syslog( LOG_CRIT, "can't find any valid address" );
#endif
//	(void) fprintf( stderr, "%s: can't find any valid address\n", argv0 );
	exit( 1 );
	}
    sc_dbg("%s:%s():%d\n", __FILE__, __FUNCTION__, __LINE__);

    /* Initialize listen sockets.  Try v6 first because of a Linux peculiarity;
    ** like some other systems, it has magical v6 sockets that also listen for
    ** v4, but in Linux if you bind a v4 socket first then the v6 bind fails.
    */
    if ( gotv6 )
#ifdef USE_SSL
	listen6_fd = initialize_listen_socket( &host_addr6 );
    else
	listen6_fd = -1;

	if ( gotv6s)
    	listen6s_fd = initialize_listen_socket( &host_addr6s );
    else
    	listen6s_fd=-1;
#else
	listen6_fd = initialize_listen_socket( &host_addr6 );
    else
	listen6_fd = -1;
#endif
    if ( gotv4 )
#ifdef USE_SSL    
	listen4_fd = initialize_listen_socket( &host_addr4 );
    else
	listen4_fd = -1;
    /* If we didn't get any valid sockets, fail. */
    if ( gotv4s)
 		listen4s_fd = initialize_listen_socket( &host_addr4s );
	else
		listen4s_fd=-1;
    if ( listen4s_fd == -1 && listen6s_fd == -1 && listen6_fd == -1 && listen4_fd == -1)
#else
	listen4_fd = initialize_listen_socket( &host_addr4 );
    else
	listen4_fd = -1;
    /* If we didn't get any valid sockets, fail. */
    if ( listen4_fd == -1 && listen6_fd == -1 )
#endif    
	{
#ifdef SYSLOG
	syslog( LOG_CRIT, "can't bind to any address" );
#endif
//	(void) fprintf( stderr, "%s: can't bind to any address\n", argv0 );
	exit( 1 );
	}

#ifdef USE_SSL
#ifdef MATRIX_SSL
    if ( do_ssl )
	{
        int ret = 0;
		if (matrixSslOpen() < 0) {
		   exit(1);
    	}
        ret = matrixSslReadKeys(&keys, certfile, DEFAULT_KEYFILE, NULL, NULL);
        sc_dbg("%s:%d:certfile=%s keyfile=%s ret=%d\n", __FUNCTION__, __LINE__, certfile, DEFAULT_KEYFILE, ret);
        if ( ret < 0)  
        {
            cpssl = NULL;
			if ((listen6s_fd = socketListen(ports, &err)) == INVALID_SOCKET) 
    		{
    		    ;
    		}
    		setSocketBlock(listen6s_fd);
    		setSocketBlock(listen6_fd);
    		if ((listen4s_fd = socketListen(ports, &err)) == INVALID_SOCKET) 
    		{
    		    ;
    		}
    			
            /*
            	Set blocking or not on the listen socket
            */
    	    setSocketBlock(listen4s_fd);
    	    setSocketBlock(listen4_fd);
    	    quit = 0;
    	    again = 0;		
    	    flags = 0;
    	    acceptAgain = 1;
    		if (acceptAgain) 
    		{
    			fd6 = socketAccept(listen6s_fd, &err);
    			fd = socketAccept(listen4s_fd, &err);
    			if(fd6 == INVALID_SOCKET && fd == INVALID_SOCKET ){
    		    exit( 1 );
    		    }
				if ((rc = sslAccept(&cpssl, fd6, keys, NULL, flags)) == 0);
				else if(( rc = sslAccept(&cpssl, fd, keys, NULL, flags)) == 0);
				else{
    	        exit( 1 );
    			}
    			flags = 0;
    			//acceptAgain = 0;
    		}   
    	}
	}
#endif	/* MATRIX_SSL */

#ifdef OPEN_SSL 
    if ( do_ssl )
	{
	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();
	ssl_ctx = SSL_CTX_new( SSLv23_server_method() );
	if ( certfile[0] != '\0' )
	    if ( SSL_CTX_use_certificate_file( ssl_ctx, certfile, SSL_FILETYPE_PEM ) == 0 ||
		 SSL_CTX_use_PrivateKey_file( ssl_ctx, certfile, SSL_FILETYPE_PEM ) == 0 ||
		 SSL_CTX_check_private_key( ssl_ctx ) == 0 )
		{
		ERR_print_errors_fp( stderr );
		exit( 1 );
		}
	if ( cipher != (char*) 0 )
	    {
	    if ( SSL_CTX_set_cipher_list( ssl_ctx, cipher ) == 0 )
		{
		ERR_print_errors_fp( stderr );
		exit( 1 );
		}
	    }
	}
#endif /* OPEN_SSL */
#endif /* USE_SSL */

    if ( ! debug )
	{
	/* Make ourselves a daemon. */
#ifdef HAVE_DAEMON
	if ( daemon( 1, 1 ) < 0 )
	    {
#ifdef SYSLOG
	    syslog( LOG_CRIT, "daemon - %m" );
	    perror( "daemon" );
#endif
	    exit( 1 );
	    }
#else
	switch ( fork() )
	    {
	    case 0:
	    break;
	    case -1:
#ifdef SYSLOG
	    syslog( LOG_CRIT, "fork - %m" );
	    perror( "fork" );
#endif
	    exit( 1 );
	    default:
	    exit( 0 );
	    }
#ifdef HAVE_SETSID
	(void) setsid();
#endif
#endif
	}
    else
	{
	/* Even if we don't daemonize, we still want to disown our parent
	** process.
	*/
#ifdef HAVE_SETSID
	(void) setsid();
#endif /* HAVE_SETSID */
	}

    if ( pidfile != (char*) 0 )
        {
	/* Write the PID file. */
	FILE* pidfp = fopen( pidfile, "w" );
        if ( pidfp == (FILE*) 0 )
            {
#ifdef SYSLOG
	    syslog( LOG_CRIT, "%s - %m", pidfile );
	    perror( pidfile );
#endif
            exit( 1 );
            }
        (void) fprintf( pidfp, "%d\n", (int) getpid() );
        (void) fclose( pidfp );
        }

    /* Read zone info now, in case we chroot(). */
    tzset();

    /* If we're root, start becoming someone else. */
    if ( getuid() == 0 )
	{
	struct passwd* pwd;
	pwd = getpwnam( user );
	if ( pwd == (struct passwd*) 0 )
	    {
#ifdef SYSLOG
	    syslog( LOG_CRIT, "unknown user - '%s'", user );
#endif
//	    (void) fprintf( stderr, "%s: unknown user - '%s'\n", argv0, user );
	    exit( 1 );
	    }
	/* Set aux groups to null. */
	if ( setgroups( 0, (gid_t*) 0 ) < 0 )
	    {
#ifdef SYSLOG
	    syslog( LOG_CRIT, "setgroups - %m" );
	    perror( "setgroups" );
#endif
	    exit( 1 );
	    }
	/* Set primary group. */
	if ( setgid( pwd->pw_gid ) < 0 )
	    {
#ifdef SYSLOG
	    syslog( LOG_CRIT, "setgid - %m" );
	    perror( "setgid" );
#endif
	    exit( 1 );
	    }
	/* Try setting aux groups correctly - not critical if this fails. */
	if ( initgroups( user, pwd->pw_gid ) < 0 )
	    {
#ifdef SYSLOG
	    syslog( LOG_ERR, "initgroups - %m" );
	    perror( "initgroups" );
#endif
	    }
#ifdef HAVE_SETLOGIN
	/* Set login name. */
	(void) setlogin( user );
#endif /* HAVE_SETLOGIN */
	/* Save the new uid for setting after we chroot(). */
	uid = pwd->pw_uid;
	}

    /* Switch directories if requested. */
    if ( dir != (char*) 0 )
	{
	if ( chdir( dir ) < 0 )
	    {
#ifdef SYSLOG
	    syslog( LOG_CRIT, "chdir - %m" );
	    perror( "chdir" );
#endif
	    exit( 1 );
	    }
	}
/* Ron Get current directory */
    /* Get current directory. */
    (void) getcwd( cwd, sizeof(cwd) - 1 );
    if ( cwd[strlen( cwd ) - 1] != '/' )
	(void) strcat( cwd, "/" );

    /* Chroot if requested. */
    if ( do_chroot )
	{
	if ( chroot( cwd ) < 0 )
	    {
#ifdef SYSLOG
	    syslog( LOG_CRIT, "chroot - %m" );
	    perror( "chroot" );
#endif
	    exit( 1 );
	    }
	(void) strcpy( cwd, "/" );
	/* Always chdir to / after a chroot. */
	if ( chdir( cwd ) < 0 )
	    {
#ifdef SYSLOG
	    syslog( LOG_CRIT, "chroot chdir - %m" );
	    perror( "chroot chdir" );
#endif
	    exit( 1 );
	    }

	}

    /* If we're root, become someone else. */
    if ( getuid() == 0 )
	{
	/* Set uid. */
	if ( setuid( uid ) < 0 )
	    {
#ifdef SYSLOG
	    syslog( LOG_CRIT, "setuid - %m" );
	    perror( "setuid" );
#endif
	    exit( 1 );
	    }
	/* Check for unnecessary security exposure. */
	if ( ! do_chroot )
	    {
#ifdef SYSLOG
	    syslog( LOG_NOTICE,
		"started as root without requesting chroot(), warning only" );
#endif
//	    (void) fprintf( stderr,
//		"%s: started as root without requesting chroot(), warning only\n", argv0 );
	    }
	}

    /* Catch various signals. */
#ifdef HAVE_SIGSET
    (void) sigset( SIGTERM, handle_sigterm );
    //(void) sigset( SIGINT, handle_sigterm );
    //(void) sigset( SIGHUP, handle_sigterm );
    //(void) sigset( SIGUSR1, handle_sigterm );
    (void) sigset( SIGCHLD, handle_sigchld );
    (void) sigset( SIGPIPE, SIG_IGN );
#else /* HAVE_SIGSET */
    (void) signal( SIGTERM, handle_sigterm );
    //(void) signal( SIGINT, handle_sigterm );
    //(void) signal( SIGHUP, handle_sigterm );
    (void) signal( SIGCHLD, handle_sigchld );
    (void) signal( SIGPIPE, SIG_IGN );
#endif /* HAVE_SIGSET */
    
/* Ron */
    /* change www dir */
    (void) signal( SIGUSR1, handle_chdir);
    /* record last access */
    (void) signal( SIGUSR2, handle_web_time );
    /* logout */
    (void) signal( SIGHUP, handle_logout);
    (void) signal( SIGINT, handle_timeout_stat);
/* Ron */    
    
    init_mime();

#ifdef SYSLOG
    if ( hostname == (char*) 0 )
	syslog(
	    LOG_NOTICE, "%.80s starting on port %d", SERVER_SOFTWARE, port );
    else
	syslog(
	    LOG_NOTICE, "%.80s starting on %.80s, port %d", SERVER_SOFTWARE,
	    hostname, port );
#endif
    /* Main loop. */
    for (;;)
	{
	/* Possibly do a select() on the possible two listen fds. */
	FD_ZERO( &lfdset );
	maxfd = -1;
	if ( listen4_fd != -1 )
	    {
	    FD_SET( listen4_fd, &lfdset );
	    if ( listen4_fd > maxfd )
		maxfd = listen4_fd;
	    }
	if ( listen6_fd != -1 )
	    {
	    FD_SET( listen6_fd, &lfdset );
	    if ( listen6_fd > maxfd )
		maxfd = listen6_fd;
	    }
#ifdef USE_SSL	    
	 if(listen4s_fd != -1)
	  {
	    FD_SET( listen4s_fd, &lfdset );
	    if ( listen4s_fd > maxfd )
		maxfd = listen4s_fd;
	  } 
	  if(listen6s_fd != -1)
	  {
	    FD_SET( listen6s_fd, &lfdset );
	    if ( listen6s_fd > maxfd )
		maxfd = listen6s_fd;	  	
	  }  
#endif	  
	if ( listen4_fd != -1 && listen6_fd != -1 )
	{
	    if ( select( maxfd + 1, &lfdset, (fd_set*) 0, (fd_set*) 0, (struct timeval*) 0 ) < 0 )
		{
		if ( errno == EINTR )
		    continue;	/* try again */
#ifdef SYSLOG
		syslog( LOG_CRIT, "select - %m" );
		perror( "select" );
#endif
		exit( 1 );
		}
	}	
	/* (If we don't have two listen fds, we can just skip the select()
	** and fall through.  Whichever listen fd we do have will do a
	** blocking accept() instead.)
	*/

	/* Accept the new connection. */
	sz = sizeof(usa);
#ifndef USE_SSL	
	if ( listen4_fd != -1 && FD_ISSET( listen4_fd, &lfdset ) )
	    conn_fd = accept( listen4_fd, &usa.sa, &sz );
	else if ( listen6_fd != -1 && FD_ISSET( listen6_fd, &lfdset ) )
	    conn_fd = accept( listen6_fd, &usa.sa, &sz );
#else
	int retval;

	if ( listen4_fd != -1 || listen6_fd != -1 || listen4s_fd != -1 || listen6s_fd != -1)
	{ 
		retval=select(maxfd+1,&lfdset,(fd_set*) 0, (fd_set*) 0,(struct timeval*) 0 );
	}

	if(retval<=0)
	{
	 continue;
	}

    if(listen4s_fd != -1 && FD_ISSET(listen4s_fd,&lfdset))
    {
        do_ssl = 1;
	    conn_fd = accept( listen4s_fd, &usa.sa, &sz );
    }
    else if(listen6s_fd != -1 && FD_ISSET(listen6s_fd,&lfdset))
    {
        do_ssl = 1;
        conn_fd = accept( listen6s_fd, &usa.sa, &sz );
    }
    else if(listen4_fd != -1 && FD_ISSET(listen4_fd,&lfdset))
    {
        do_ssl = 0;
        conn_fd = accept( listen4_fd, &usa.sa, &sz );
    }
    else if ( listen6_fd != -1 && FD_ISSET( listen6_fd, &lfdset ) )
	{
		do_ssl = 0;
		conn_fd = accept( listen6_fd, &usa.sa, &sz );
	}
#endif
	else
	    {
#ifdef SYSLOG
	    syslog( LOG_CRIT, "select failure" );
#endif
//	    (void) fprintf( stderr, "%s: select failure\n", argv0 );
	    exit( 1 );
	    }
	if ( conn_fd < 0 )
	    {
	    if ( errno == EINTR )
		continue;	/* try again */
#ifdef EPROTO
	    if ( errno == EPROTO )
		continue;	/* try again */
#endif /* EPROTO */
#ifdef SYSLOG
	    syslog( LOG_CRIT, "accept - %m" );
	    perror( "accept" );
#endif
	    exit( 1 );
	    }
    
	/* Fork a sub-process to handle the connection. */

/* Jeff Sun -- Apr.28.2005 */
/* If Management via WLAN disable,and user is wlan client,block it */

    /* block access from wlan */
    p = nvram_safe_get("wlan_manage");
    if(strlen(p)){
        wlan_mgr = *p - '0';        
        free(p);
    }
    else {
        free(p);
        wlan_mgr = 0;
    }

    if(!wlan_mgr){ 
        /* bug: IPv6 Access isn't arp mac */
        strcpy(wl_tmp, ntoa(&usa));
        if(strchr(wl_tmp, ':') != NULL)
            goto out;
        
        wl_mac[0]='\0';
        wl_p=fopen("/proc/net/arp","r");
        if(wl_p!=NULL){
        	while(fgets(wl_tmp,256,wl_p)!=NULL)
        	{		
                sscanf(wl_tmp, "%s %*s %*s %s %*s %*s",wl_ipaddr,wl_mac);                
                
                if(!strcmp(wl_ipaddr,ntoa(&usa)))
                    break;

                wl_mac[0]='\0';
        	}
            fclose(wl_p);
        }
        /*
        p = nvram_get("fw_remote");  
        remote_manage = *p - '0';
        free(p);
        if(wl_mac[0] == '\0' &&  remote_manage == 0){
            (void) close( conn_fd );
            continue;
        }
        if(wl_mac[0] == '\0'){
            (void) close( conn_fd );
            continue;
        }*/
        if(wl_mac[0] != '\0'){
#ifdef AIRGO_WIFI
        system("/usr/etc/airgo/usr/sbin/aniSdkTool -g sta >> /tmp/wlan_mac");
        {
            int i;
            for(i=0;wl_mac[i]!=0;i++) wl_mac[i]=tolower(wl_mac[i]);
        }
#else
        char name[60];
        
        for(unit=0; unit<1; unit++)
        {   
            for(vap=0; vap<4; vap++)
            {
                sprintf(delifPath, "/tmp/wlan_delif.ath%d%d",unit,vap);
                if(access(delifPath, F_OK) == 0)
                    SYSTEM("/sbin/wlanconfig ath%d%d list sta >> /tmp/wlan_mac",unit,vap);
            }            
        }
        {
            /* To lower 
            int i;
            for(i=0;wl_mac[i]!=0;i++) wl_mac[i]=tolower(wl_mac[i]);*/
            scToLows(wl_mac);
        }
#endif
        wl_mac_match=0;
        wl_p=fopen("/tmp/wlan_mac","r");
        if(wl_p!=NULL){
        	while(fgets(wl_tmp,256,wl_p)!=NULL)
        	{
        	    if(strstr(wl_tmp,wl_mac)!=NULL){
                    wl_mac_match=1;
                    break;
    	        }
        	}
            fclose(wl_p);
        }
        unlink("/tmp/wlan_mac");
        if(wl_mac_match==1){
           	(void) close( conn_fd );
            continue;
        }
        }
    }
out:
/* Ron */
	
/* if someone_in_use=1 will show auth fail msg */
/* web_last_access_time is record user last action */

	/* if does not the same user and per user not timeout*/
	if (*last_remote_ip=='\0'){
		someone_in_use=0;
		strcpy(last_remote_ip,remote_ip);
		strcpy(remote_ip,ntoa(&usa));			
	}else if (strcmp(remote_ip,ntoa(&usa))!=0 && !check_timeout() && last_access_time!=0){     
        /* For enable multi-user login */
        LOGOUT;
        someone_in_use=0;
//        someone_in_use=1; 
	}
	else{	
		someone_in_use=0;
		strcpy(last_remote_ip,remote_ip);
		strcpy(remote_ip,ntoa(&usa));			
	}

    //syslog(LOG_INFO,"some %d DEBUG IP:%s <%s>",someone_in_use,remote_ip,last_remote_ip);
	
/* Ron */
	r = fork();
	if ( r < 0 )
	    {
#ifdef SYSLOG
	    syslog( LOG_CRIT, "fork - %m" );
	    perror( "fork" );
#endif
	    exit( 1 );
	    }
	else if ( r == 0 )
	    {
	    /* Child process. */
	    client_addr = usa;
	    if ( listen4_fd != -1 )
		(void) close( listen4_fd );
	   if ( listen6_fd != -1 )
			(void) close( listen6_fd );
#ifdef USE_SSL		
	    if ( listen4s_fd != -1 )
		(void) close( listen4s_fd );
		if ( listen6s_fd != -1 )
		(void) close( listen6s_fd );
#endif		
	    handle_request();
	    exit( 0 );
	    }
	
	(void) close( conn_fd );
	
	}
}

#if 0
static void
usage( void )
    {
#ifdef USE_SSL
#ifdef OPEN_SSL
    (void) fprintf( stderr, "usage:  %s [-C configfile] [-D] [-S] [-E certfile] [-Y cipher] [-p port] [-d dir] [-c cgipat] [-u user] [-h hostname] [-r] [-v] [-l logfile] [-i pidfile] [-T charset] [-P P3P] [-M maxage]\n", argv0 );
#endif
#else /* USE_SSL */
  (void) fprintf( stderr, "usage:  %s [-C configfile] [-D] [-p port] [-d dir] [-c cgipat] [-u user] [-h hostname] [-r] [-v] [-l logfile] [-i pidfile] [-T charset] [-P P3P] [-M maxage]\n", argv0 );
#endif /* USE_SSL */
    exit( 1 );
    }
#endif

static void
read_config( char* filename )
    {
    FILE* fp;
    char line[10000];
    char* cp;
    char* cp2;
    char* name;
    char* value;

    fp = fopen( filename, "r" );
    if ( fp == (FILE*) 0 )
	{
#ifdef SYSLOG
	syslog( LOG_CRIT, "%s - %m", filename );
	perror( filename );
#endif
	exit( 1 );
	}

    while ( fgets( line, sizeof(line), fp ) != (char*) 0 )
	{
	/* Trim comments. */
	if ( ( cp = strchr( line, '#' ) ) != (char*) 0 )
	    *cp = '\0';

	/* Split line into words. */
	for ( cp = line; *cp != '\0'; cp = cp2 )
	    {
	    /* Skip leading whitespace. */
	    cp += strspn( cp, " \t\n\r" );
	    /* Find next whitespace. */
	    cp2 = cp + strcspn( cp, " \t\n\r" );
	    /* Insert EOS and advance next-word pointer. */
	    while ( *cp2 == ' ' || *cp2 == '\t' || *cp2 == '\n' || *cp2 == '\r' )
		*cp2++ = '\0';
	    /* Split into name and value. */
	    name = cp;
	    value = strchr( name, '=' );
	    if ( value != (char*) 0 )
		*value++ = '\0';
	    /* Interpret. */
	    if ( strcasecmp( name, "debug" ) == 0 )
		{
		no_value_required( name, value );
		debug = 1;
		}
	    else if ( strcasecmp( name, "port" ) == 0 )
		{
		value_required( name, value );
		port = atoi( value );
		}
	    else if ( strcasecmp( name, "dir" ) == 0 )
		{
		value_required( name, value );
		dir = e_strdup( value );
		}
	    else if ( strcasecmp( name, "chroot" ) == 0 )
		{
		no_value_required( name, value );
		do_chroot = 1;
		}
	    else if ( strcasecmp( name, "nochroot" ) == 0 )
		{
		no_value_required( name, value );
		do_chroot = 0;
		}
	    else if ( strcasecmp( name, "user" ) == 0 )
		{
		value_required( name, value );
		user = e_strdup( value );
		}
	    else if ( strcasecmp( name, "cgipat" ) == 0 )
		{
		value_required( name, value );
		cgi_pattern = e_strdup( value );
		}
	    else if ( strcasecmp( name, "urlpat" ) == 0 )
		{
		value_required( name, value );
		url_pattern = e_strdup( value );
		}
	    else if ( strcasecmp( name, "noemptyreferers" ) == 0 )
		{
		value_required( name, value );
		no_empty_referers = 1;
		}
	    else if ( strcasecmp( name, "localpat" ) == 0 )
		{
		value_required( name, value );
		local_pattern = e_strdup( value );
		}
	    else if ( strcasecmp( name, "host" ) == 0 )
		{
		value_required( name, value );
		hostname = e_strdup( value );
		}
	    else if ( strcasecmp( name, "logfile" ) == 0 )
		{
		value_required( name, value );
		logfile = e_strdup( value );
		}
	    else if ( strcasecmp( name, "vhost" ) == 0 )
		{
		no_value_required( name, value );
		vhost = 1;
		}
	    else if ( strcasecmp( name, "pidfile" ) == 0 )
		{
		value_required( name, value );
		pidfile = e_strdup( value );
		}
	    else if ( strcasecmp( name, "charset" ) == 0 )
		{
		value_required( name, value );
		charset = e_strdup( value );
		}
	    else if ( strcasecmp( name, "p3p" ) == 0 )
		{
		value_required( name, value );
		p3p = e_strdup( value );
		}
	    else if ( strcasecmp( name, "max_age" ) == 0 )
		{
		value_required( name, value );
		max_age = atoi( value );
		}
#ifdef USE_SSL
#ifdef OPEN_SSL
	    else if ( strcasecmp( name, "ssl" ) == 0 )
		{
		no_value_required( name, value );
		do_ssl = 1;
		}
	    else if ( strcasecmp( name, "certfile" ) == 0 )
		{
		value_required( name, value );
		certfile = e_strdup( value );
		}
	    else if ( strcasecmp( name, "cipher" ) == 0 )
		{
		value_required( name, value );
		cipher = e_strdup( value );
		}
#endif
#endif /* USE_SSL */
	    else
		{
//		(void) fprintf(
//		    stderr, "%s: unknown config option '%s'\n", argv0, name );
		exit( 1 );
		}
	    }
	}

    (void) fclose( fp );
    }


static void
value_required( char* name, char* value )
    {
    if ( value == (char*) 0 )
	{
//	(void) fprintf(
//	    stderr, "%s: value required for %s option\n", argv0, name );
	exit( 1 );
	}
    }


static void
no_value_required( char* name, char* value )
    {
    if ( value != (char*) 0 )
	{
//	(void) fprintf(
//	    stderr, "%s: no value required for %s option\n",
//	    argv0, name );
	exit( 1 );
	}
    }


static int
initialize_listen_socket( usockaddr* usaP )
    {
    int listen_fd;
    int i;

    /* Check sockaddr. */
    if ( ! sockaddr_check( usaP ) )
	{
#ifdef SYSLOG
	syslog(
	    LOG_ERR, "unknown sockaddr family on listen socket - %d",
	    usaP->sa.sa_family );
#endif
//	(void) fprintf(
//	    stderr, "%s: unknown sockaddr family on listen socket - %d\n",
//	    argv0, usaP->sa.sa_family );
	return -1;
	}

    listen_fd = socket( usaP->sa.sa_family, SOCK_STREAM, 0 );
    if ( listen_fd < 0 )
	{
#ifdef SYSLOG
	syslog( LOG_CRIT, "socket %.80s - %m", ntoa( usaP ) );
	perror( "socket" );
#endif
	return -1;
	}

    (void) fcntl( listen_fd, F_SETFD, 1 );

    i = 1;
    if ( setsockopt( listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*) &i, sizeof(i) ) < 0 )
	{
#ifdef SYSLOG
	syslog( LOG_CRIT, "setsockopt SO_REUSEADDR - %m" );
	perror( "setsockopt SO_REUSEADDR" );
#endif
	return -1;
	}

    if ( bind( listen_fd, &usaP->sa, sockaddr_len( usaP ) ) < 0 )
	{
#ifdef SYSLOG
	syslog( LOG_CRIT, "bind %.80s - %m", ntoa( usaP ) );
	perror( "bind" );
#endif
	return -1;
	}

    if ( listen( listen_fd, 1024 ) < 0 )
	{
#ifdef SYSLOG
	syslog( LOG_CRIT, "listen - %m" );
	perror( "listen" );
#endif
	return -1;
	}

#ifdef HAVE_ACCEPT_FILTERS
    {
    struct accept_filter_arg af;
    (void) bzero( &af, sizeof(af) );
    (void) strcpy( af.af_name, ACCEPT_FILTER_NAME );
    (void) setsockopt( listen_fd, SOL_SOCKET, SO_ACCEPTFILTER, (char*) &af, sizeof(af) );
    }
#endif /* HAVE_ACCEPT_FILTERS */

    return listen_fd;
    }


/* This runs in a child process, and exits when done, so cleanup is
** not needed.
*/
static void
handle_request( void )
    {
    char* method_str;
    char* line;
    char* cp;
    int r, file_len, i;
    const char* index_names[] = {
	"setup.cgi","index.html", "index.htm", "index.xhtml", "index.xht", "Default.htm",
	 };

    /* Set up a timeout. */
#ifdef HAVE_SIGSET
    (void) sigset( SIGALRM, handle_sigalrm );
#else /* HAVE_SIGSET */
    (void) signal( SIGALRM, handle_sigalrm );
#endif /* HAVE_SIGSET */
    (void) alarm( TIMEOUT );

    /* Initialize the request variables. */
    remoteuser = (char*) 0;
    method = METHOD_UNKNOWN;
    path = (char*) 0;
    file = (char*) 0;
    pathinfo = (char*) 0;
    query = "";
    protocol = (char*) 0;
    status = 0;
    bytes = -1;
    req_hostname = (char*) 0;

    authorization = (char*) 0;
    content_type = (char*) 0;
    content_length = -1;
    cookie = (char*) 0;
    host = (char*) 0;
    if_modified_since = (time_t) -1;
    referer = "";
    useragent = "";

#ifdef TCP_NOPUSH
    /* Set the TCP_NOPUSH socket option, to try and avoid the 0.2 second
    ** delay between sending the headers and sending the data.  A better
    ** solution is writev() (as used in thttpd), or send the headers with
    ** send(MSG_MORE) (only available in Linux so far).
    */
    r = 1;
    (void) setsockopt(
	conn_fd, IPPROTO_TCP, TCP_NOPUSH, (void*) &r, sizeof(r) );
#endif /* TCP_NOPUSH */

#ifdef USE_SSL
	if (do_ssl)
    {
#ifdef MATRIX_SSL
	ssl = SSL_new(keys);
    ssl->outBufferCount=0;
	SSL_set_fd( ssl, conn_fd );
	if ( SSL_accept( ssl ) <= 0 ) {
	    {
	    exit( 1 );
	    }
	  }  
#elif defined(OPEN_SSL)
	ssl = SSL_new( ssl_ctx );
	SSL_set_fd( ssl, conn_fd );
	if ( SSL_accept( ssl ) == 0 )
	    {
	    ERR_print_errors_fp( stderr );
	    exit( 1 );
	    }
#endif   
	}
#endif /* USE_SSL */
    /* Read in the request. */
    start_request();
    for (;;)
	{
	char buf[10000];
	int r = my_read( buf, sizeof(buf) );
	if ( r <= 0 )
	    break;
	(void) alarm( TIMEOUT );
	add_to_request( buf, r );
	if ( strstr( request, "\r\n\r\n" ) != (char*) 0 ||
	     strstr( request, "\n\n" ) != (char*) 0 )
	    break;
	}

    /* Parse the first line of the request. */
    method_str = get_request_line();

    if ( method_str == (char*) 0 )
	send_error( 400, "Bad Request", "", "Can't parse request." );
    path = strpbrk( method_str, " \t\n\r" );
    if ( path == (char*) 0 )
	send_error( 400, "Bad Request", "", "Can't parse request." );
    
    *path++ = '\0';
    /* Ron add for auto add setup.cgi?next_file*/
    if(strstr(path,".cgi")==NULL && (strstr(path,".htm") || strstr(path, ".xml")) )
    {
    	if(*path=='/') path++;
    	sprintf(fakepath,"/setup.cgi?next_file=%s",path);
    	path=fakepath;
    }
#ifdef _USE_COOKIES_ 
   	if(strstr(path,"login.htm")){
   		if((strstr(path,"login.htm&no=3")==0)&&(strstr(path,"login.htm&no=2")==0))
    	{
    				unlink("/tmp/nextpage");
    	}
 	}
 	else if((strstr(path,"Home.htm")==NULL)&&(strstr(path,"index.htm")==NULL)&&((strstr(path,".htm"))||(strstr(path,".cgi")))){
 		if(access("/tmp/nextpage", F_OK) == 0){
    			send_return(1);
    	}
 	}
#endif    	
/* 
* By MD@SC_CPUAP for saving current path  2007-7-31
* Added Begin
*/
    strcpy(scpath,path);     
/* 
* Added End
*/    
    
    path += strspn( path, " \t\n\r" );
    protocol = strpbrk( path, " \t\n\r" );
    if ( protocol == (char*) 0 )
	send_error( 400, "Bad Request", "", "Can't parse request." );
    *protocol++ = '\0';
    query = strchr( path, '?' );

    if ( query == (char*) 0 )
	query = "";
    else
	*query++ = '\0';

/* 
* By MD@SC_CPUAP for autohttps  2007-7-31
* Added Begin
*/
#ifdef USE_SSL 
    if(autohttps){
        if (do_ssl == 0){            
            sendAutoHttps(scpath);
        }   
    }
#endif  
/* 
* Added End
*/	
/* Add by archer for auto http */
#ifdef USE_SSL
	if(autohttp){
        if (do_ssl == 1){
            sendAutoHttp(scpath);
        }   
    }
#endif  
/* Add end */
    /* Parse the rest of the request headers. */
    while ( ( line = get_request_line() ) != (char*) 0 )
	{
	if ( line[0] == '\0' )
	    break;
	else if ( strncasecmp( line, "Authorization:", 14 ) == 0 )
	    {
	    cp = &line[14];
	    cp += strspn( cp, " \t" );
	    authorization = cp;
	    }
	else if ( strncasecmp( line, "Content-Length:", 15 ) == 0 )
	    {
	    cp = &line[15];
	    cp += strspn( cp, " \t" );
	    content_length = atol( cp );
	    }
	else if ( strncasecmp( line, "Content-Type:", 13 ) == 0 )
	    {
	    cp = &line[13];
	    cp += strspn( cp, " \t" );
	    content_type = cp;
	    }
	else if ( strncasecmp( line, "Cookie:", 7 ) == 0 )
	    {
	    cp = &line[7];
	    cp += strspn( cp, " \t" );
	    cookie = cp;
	    }
	else if ( strncasecmp( line, "Host:", 5 ) == 0 )
	    {
	    cp = &line[5];
	    cp += strspn( cp, " \t" );
	    host = cp;
	    }
	else if ( strncasecmp( line, "If-Modified-Since:", 18 ) == 0 )
	    {
	    cp = &line[18];
	    cp += strspn( cp, " \t" );
	    if_modified_since = tdate_parse( cp );
	    }
	else if ( strncasecmp( line, "Referer:", 8 ) == 0 )
	    {
	    cp = &line[8];
	    cp += strspn( cp, " \t" );
	    referer = cp;
	    }
	else if ( strncasecmp( line, "User-Agent:", 11 ) == 0 )
	    {
	    cp = &line[11];
	    cp += strspn( cp, " \t" );
	    useragent = cp;
	    }
	}

    if ( strcasecmp( method_str, get_method_str( METHOD_GET ) ) == 0 )
	method = METHOD_GET;
    else if ( strcasecmp( method_str, get_method_str( METHOD_HEAD ) ) == 0 )
	method = METHOD_HEAD;
    else if ( strcasecmp( method_str, get_method_str( METHOD_POST ) ) == 0 )
	method = METHOD_POST;
    else
	send_error( 501, "Not Implemented", "", "That method is not implemented." );

  /* Breeze add for ptimeout */
    if (strstr(path,"ptimeout.cgi")){
        NEEDCHECKPW;
        send_authenticateWarning("");  
    }

    strdecode( path, path );
    if ( path[0] != '/' )
	send_error( 400, "Bad Request", "", "Bad filename." );
 /*Ron*/
    if ( someone_in_use == 1 )
	send_error( 401, "Unauthorized", "", "Another Administrator online." );
 /*Ron*/

    file = &(path[1]);
    
    de_dotdot( file );
    if ( file[0] == '\0' )
	file = "./";
    if ( file[0] == '/' ||
	 ( file[0] == '.' && file[1] == '.' &&
	   ( file[2] == '\0' || file[2] == '/' ) ) )
	send_error( 400, "Bad Request", "", "Illegal filename." );
    if ( vhost )
	file = virtual_file( file );

    (void) alarm( 0 );
    r = stat( file, &sb );
    if ( r < 0 )
	r = get_pathinfo();
    if ( r < 0 )
	send_error( 404, "Not Found", "", "File not found." );
    file_len = strlen( file );
    if ( ! S_ISDIR( sb.st_mode ) )
	{
	/* Not a directory. */
	while ( file[file_len - 1] == '/' )
	    {
	    file[file_len - 1] = '\0';
	    --file_len;
	    }
	do_file();
	}
    else
	{
	char idx[10000];

	/* The filename is a directory.  Is it missing the trailing slash? */
	if ( file[file_len - 1] != '/' && pathinfo == (char*) 0 )
	    {
	    char location[10000];
	    if ( query[0] != '\0' )
		(void) snprintf(
		    location, sizeof(location), "Location: %s/?%s", path,
		    query );
	    else
		(void) snprintf(
		    location, sizeof(location), "Location: %s/", path );
	    send_error( 302, "Found", location, "Directories must end with a slash." );
	    }

	/* Check for an index file. */
	for ( i = 0; i < sizeof(index_names) / sizeof(char*); ++i )
	    {
		    if(strcmp(file,"./")==0){
		    (void) snprintf( idx, sizeof(idx), "%s",index_names[i] );
#ifdef _USE_COOKIES_ 				
				query="next_file=login.htm&no=0";
#endif
			}
		    else
		    (void) snprintf( idx, sizeof(idx), "%s%s", file, index_names[i] );

	    if ( stat( idx, &sb ) >= 0 )
		{
		file = idx;
		do_file();
		continue;
		}
	    }
//printf("do_dir\n");
	/* Nope, no index file, so it's an actual directory request. */
//	do_dir();

//	got_one:
	}

#ifdef USE_SSL
    SSL_free( ssl );
#endif /* USE_SSL */
    }


static void
de_dotdot( char* file )
    {
    char* cp;
    char* cp2;
    int l;

    /* Collapse any multiple / sequences. */
    while ( ( cp = strstr( file, "//") ) != (char*) 0 )
	{
	for ( cp2 = cp + 2; *cp2 == '/'; ++cp2 )
	    continue;
	(void) strcpy( cp + 1, cp2 );
	}

    /* Remove leading ./ and any /./ sequences. */
    while ( strncmp( file, "./", 2 ) == 0 )
	(void) strcpy( file, file + 2 );
    while ( ( cp = strstr( file, "/./") ) != (char*) 0 )
	(void) strcpy( cp, cp + 2 );

    /* Alternate between removing leading ../ and removing xxx/../ */
    for (;;)
	{
	while ( strncmp( file, "../", 3 ) == 0 )
	    (void) strcpy( file, file + 3 );
	cp = strstr( file, "/../" );
	if ( cp == (char*) 0 )
	    break;
	for ( cp2 = cp - 1; cp2 >= file && *cp2 != '/'; --cp2 )
	    continue;
	(void) strcpy( cp2 + 1, cp + 4 );
	}

    /* Also elide any xxx/.. at the end. */
    while ( ( l = strlen( file ) ) > 3 &&
	    strcmp( ( cp = file + l - 3 ), "/.." ) == 0 )
	{
	for ( cp2 = cp - 1; cp2 >= file && *cp2 != '/'; --cp2 )
	    continue;
	if ( cp2 < file )
	    break;
	*cp2 = '\0';
	}
    }


static int
get_pathinfo( void )
    {
    int r;

    pathinfo = &file[strlen(file)];
    for (;;)
	{
	do
	    {
	    --pathinfo;
	    if ( pathinfo <= file )
		{
		pathinfo = (char*) 0;
		return -1;
		}
	    }
	while ( *pathinfo != '/' );
	*pathinfo = '\0';
	r = stat( file, &sb );
	if ( r >= 0 )
	    {
	    ++pathinfo;
	    return r;
	    }
	else
	    *pathinfo = '/';
	}
    }

static int unhex( char c );

#ifdef _USE_COOKIES_ 
void
nsldapi_hex_unescape( char *s )
{
/*
 * Remove URL hex escapes from s... done in place.  The basic concept for
 * this routine is borrowed from the WWW library HTUnEscape() routine.
 */
	char	*p;

	for ( p = s; *s != '\0'; ++s ) {
		if ( *s == '%' ) {
			if ( *++s != '\0' ) {
				*p = unhex( *s ) << 4;
			}
			if ( *++s != '\0' ) {
				*p++ += unhex( *s );
			}
		} else {
			*p++ = *s;
		}
	}

	*p = '\0';
}

#endif
static int
unhex( char c )
{
	return( c >= '0' && c <= '9' ? c - '0'
	    : c >= 'A' && c <= 'F' ? c - 'A' + 10
	    : c - 'a' + 10 );
}


#ifdef _USE_COOKIES_ 

static int get_name_pwd(char *pwd,char *name)
{
	char *p1,*p2;
	int l;
	char buf[128]={0};
	
	if ( cookie == (char*) 0 ){
        not_auth=1;
    	return 0;    
    }
    strcpy(buf, cookie);

    p1=strstr(buf,"LoginPWD=");
    if(!p1){
    	return 0;  
    }
    p1=p1+strlen("LoginPWD=");
    if(!(*p1)){
    	return 0;  
    }
    p2=strchr(p1,';');
    if(p2)			//LoginPWD,LoginName
    	*p2++=0;
    else			//LoginName,LoginPWD
    	p2=buf;
    nsldapi_hex_unescape(p1);
    l = b64_decode(p1, (unsigned char*) pwd, 500-1 );
    
    p1=strstr(p2,"LoginName=");
	if(!p1){
    	return 0;  
    }
	p1=p1+strlen("LoginName=");
    if(!(*p1)){
    	return 0;  
    }
    p2=strchr(p1,';');	
	if(p2){			//LoginName,LoginPWD
    	*p2=0;
    }
	nsldapi_hex_unescape(p1);
	l = b64_decode(p1, (unsigned char*) name, 500 - 1 );
    return 1;
}



void AddLOGFlag(){
	if(access(LOG_ENANLE, F_OK) != 0)
    {
        FILE *fp;
        fp = fopen(LOG_ENANLE, "w");
        if(fp!=NULL)
        {            
            fprintf(fp, "%d\n", 1);
            fclose(fp);
    		syslog(LOG_INFO,"[SC][AuthLogin]Authorized Login from %s",remote_ip); 
        }            
    }
    else{
    		if(strcmp(last_remote_ip,remote_ip)!=0){
    		syslog(LOG_INFO,"[SC][AuthLogin]Authorized Login from %s",remote_ip);
    	}
    }
}


static void
cookie_check( char* dirname ){
	 char authpath[10000];
	 char pwd[500],name[500];
	 FILE* fp;
	 static char line[10000];
	  char* cryp;
	  struct stat sb;
	  int l;
    if ( dirname[strlen(dirname) - 1] == '/' )
	(void) snprintf( authpath, sizeof(authpath), "%s%s", dirname, AUTH_FILE );
    else
	(void) snprintf( authpath, sizeof(authpath), "%s/%s", dirname, AUTH_FILE );
	sprintf(authpath,"/tmp/htpasswd");
    if ( stat( authpath, &sb ) < 0 || !needPassword(authpath, line) ){
    	if(strcmp(last_remote_ip,remote_ip)!=0){
    		syslog(LOG_INFO,"Administrator login successful - IP:%s",remote_ip);
    		SAVETIME;	
    	}else if(check_timeout())
    		{
    			 //fprintf( stderr, "===%d=ptimeout\n",__LINE__ );
				 send_return(1);
    		}
    	else
    	SAVETIME;    		
    	return;
    }
    not_auth=0;
     memset(name,0,sizeof(name));
     memset(pwd,0,sizeof(pwd));

    if(get_name_pwd(pwd,name)==0)
    	{
    		 not_auth=1;
    		 
    		 if(check_timeout()) {
    			int IsHelp=0;
    			char *p;
    			FILE *fp = NULL;
    		//	fprintf( stderr, "2111=-query---%s-----------------\n",query );
    			p=strstr(query,"next_file=");
    			if(p){
    			p=p+10;
    			if(p){
    			 	fp = fopen("/tmp/nextpage", "w");
   					 if(fp)
   					 {
   					 	if(strstr(p,"WClientMACList.htm")){
   					 		fputs("WMACFilter.htm",fp);
   					 	}
   					 	else if((strstr(p,"SiteSurvey.htm"))||(strstr(p,"SurveyWait.htm"))||(strstr(p,"RogueLegal.htm")))
   					 	{
   					 			fputs("ApMode.htm",fp);
   					 	}
   					 	else if(strstr(p,"log_data.htm")){
   					 		fputs("Log.htm",fp);
   					 	}
   					 	else if(strstr(p,"Ping.htm")){
   					 		fputs("Diagnostics.htm",fp);
   					 	}
   					 	else {
      				 		if(strstr(p,"help/h_")){
      				 			p=p+7;

      				 			IsHelp=1;
      				 		}

      				 		if(p){
      				 			 fputs(p,fp);
      				 			}
      				 	}
       				  	fclose(fp);
   					 }
   				}
   				}
   				if(IsHelp)
   					send_return(2);
				else
					 send_return(1);
    		}
    		 send_return(0);
    	}
       /* Open the password file. */
    fp = fopen( authpath, "r" );
    if ( fp == (FILE*) 0 )
	{
	send_error( 403, "Forbidden", "", "File is protected." );
	}	
	
	  while ( fgets( line, sizeof(line), fp ) != (char*) 0 )
	{
	l = strlen( line );
	if ( line[l - 1] == '\n' )
	    line[l - 1] = '\0';
	cryp = strchr( line, ':' );
	if ( cryp == (char*) 0 )
	    continue;
	*cryp++ = '\0';
	if ( strcmp( line, name ) == 0 )
	{
        struct sysinfo info;
        sysinfo(&info);
       
	    (void) fclose( fp );

    	if ( strcmp(pwd, cryp ) == 0){
    	/*	if((strcmp(last_remote_ip,remote_ip)!=0)){
		if (last_remote_ip[0]!=0)
    				 send_return(0);
    		}else 
    		*/if(check_timeout()) {
    			int IsHelp=0;
    			char *p;
    			FILE *fp = NULL;
    		//	fprintf( stderr, "2111=-query---%s-----------------\n",query );
    			p=strstr(query,"next_file=");
    			if(p){
    			p=p+10;
    			if(p){
    			 	fp = fopen("/tmp/nextpage", "w");
   					 if(fp)
   					 {
   					 	if(strstr(p,"WClientMACList.htm")){
   					 		fputs("WMACFilter.htm",fp);
   					 	}
   					 	else if((strstr(p,"SiteSurvey.htm"))||(strstr(p,"SurveyWait.htm"))||(strstr(p,"RogueLegal.htm")))
   					 	{
   					 			fputs("ApMode.htm",fp);
   					 	}
   					 	else if(strstr(p,"log_data.htm")){
   					 		fputs("Log.htm",fp);
   					 	}
   					 	else if(strstr(p,"Ping.htm")){
   					 		fputs("Diagnostics.htm",fp);
   					 	}
   					 	else {
      				 		if(strstr(p,"help/h_")){
      				 			p=p+7;

      				 			IsHelp=1;
      				 		}

      				 		if(p){
      				 			 fputs(p,fp);
      				 			}
      				 	}
       				  	fclose(fp);
   					 }
   				}
   				}
   				if(IsHelp)
   					send_return(2);
				else
					 send_return(1);
    		}
    	//	fprintf( stderr, "2121=----%s-----------------\n",path );
    		if(strstr(query,"Home.htm")){
    		//	fprintf( stderr, "2121=---------------------\n" );
    		  if(access("/tmp/nextpage", F_OK) == 0){
    			send_return(3);
    			}
   			 }
   			AddLOGFlag();
    	
    		remoteuser = line;
    		
    					
    		SAVETIME;	
    		return;
    	}else{		

        	not_auth=1;
			 if(check_timeout()) {
    			int IsHelp=0;
    			char *p;
    			FILE *fp = NULL;
    		//	fprintf( stderr, "2111=-query---%s-----------------\n",query );
    			p=strstr(query,"next_file=");
    			if(p){
    			p=p+10;
    			if(p){
    			 	fp = fopen("/tmp/nextpage", "w");
   					 if(fp)
   					 {
   					 	if(strstr(p,"WClientMACList.htm")){
   					 		fputs("WMACFilter.htm",fp);
   					 	}
   					 	else if((strstr(p,"SiteSurvey.htm"))||(strstr(p,"SurveyWait.htm"))||(strstr(p,"RogueLegal.htm")))
   					 	{
   					 			fputs("ApMode.htm",fp);
   					 	}
   					 	else if(strstr(p,"log_data.htm")){
   					 		fputs("Log.htm",fp);
   					 	}
   					 	else if(strstr(p,"Ping.htm")){
   					 		fputs("Diagnostics.htm",fp);
   					 	}
   					 	else {
      				 		if(strstr(p,"help/h_")){
      				 			p=p+7;

      				 			IsHelp=1;
      				 		}

      				 		if(p){
      				 			 fputs(p,fp);
      				 			}
      				 	}
       				  	fclose(fp);
   					 }
   				}
   				}
   				if(IsHelp)
   					send_return(2);
				else
					 send_return(1);
    		}

		    send_return(0);
	    }
	}


	}
	 (void) fclose( fp );

    send_return(0);
    
    
}
#endif
static void
do_file( void )
    {
    char buf[10000];
    char mime_encodings[500];
    char* mime_type;
    char fixed_mime_type[500];
    char* cp;
    int fd;
    /* Check authorization for this directory. */
    (void) strncpy( buf, file, sizeof(buf) );
  //  fprintf( stderr, "%d==buf==%s,query=%s,file=%s\n", __LINE__,buf,query,file );

    if(strstr(buf,"upload.cgi")!=0) goto kk;
    /* reboot page needn't auth after upgrade finished */
    if(strstr(query,"mgt_upgmsg.htm")!=0 
        || strstr(query,"galert.htm")!=0
        || strstr(query,"index.htm")!=0
#ifdef _USE_COOKIES_
        ||strstr(query,"login.htm")!=0
    	|| strstr(buf,".gif")!=0
    	|| strstr(buf,".jpg")!=0
    	|| strstr(buf,".js")!=0
#endif
    	|| strstr(buf,".png")!=0
    	|| strstr(buf,".css")!=0)
    	{ 
    		if(strstr(query,"login.htm")!=0)LOGOUT;
    		goto kk;
		}
    cp = strrchr( buf, '/' );
    if ( cp == (char*) 0 )
	(void) strcpy( buf, "." );
    else
	*cp = '\0';
#ifdef _USE_COOKIES_
    cookie_check(buf);
  
#else    
   auth_check( buf );
#endif
kk:
    /* Check if the filename is the AUTH_FILE itself - that's verboten. */
    if ( strcmp( file, AUTH_FILE ) == 0 ||
	 ( strcmp( &(file[strlen(file) - sizeof(AUTH_FILE) + 1]), AUTH_FILE ) == 0 &&
	   file[strlen(file) - sizeof(AUTH_FILE)] == '/' ) )
	{
#ifdef SYSLOG
	syslog(
	    LOG_NOTICE, "%.80s URL \"%.80s\" tried to retrieve an auth file",
	    ntoa( &client_addr ), path );
#endif
	send_error( 403, "Forbidden", "", "File is protected." );
	}

    /* Referer check. */
    check_referer();

    /* Is it CGI? */
   	
 	
   if ( cgi_pattern != (char*) 0 && match( cgi_pattern, file ) )
	{

	do_cgi();
	return;
	}
    else if ( pathinfo != (char*) 0 ){
		send_error( 404, "Not Found", "", "File not found." );
	}
 
    fd = open( file, O_RDONLY );
    if ( fd < 0 )
	{
#ifdef SYSLOG
	syslog(
	    LOG_INFO, "%.80s File \"%.80s\" is protected",
	    ntoa( &client_addr ), path );
#endif
	send_error( 403, "Forbidden", "", "File is protected." );
	}

    mime_type = figure_mime( file, mime_encodings, sizeof(mime_encodings) );
    (void) snprintf(
	fixed_mime_type, sizeof(fixed_mime_type), mime_type, charset );
#if 0
    if ( if_modified_since != (time_t) -1 &&
	 if_modified_since >= sb.st_mtime )
	{
	add_headers(
	    304, "Not Modified", "", mime_encodings, fixed_mime_type,
	    sb.st_size, sb.st_mtime );
	send_response();
	return;
	}
#endif

    add_headers(
	200, "Ok", "", mime_encodings, fixed_mime_type, sb.st_size,
	sb.st_mtime );
    send_response();

    if ( method == METHOD_HEAD )
	return;

    if ( sb.st_size > 0 )	/* ignore zero-length files */
	{
#ifdef HAVE_SENDFILE

#ifndef USE_SSL
	(void) my_sendfile( fd, conn_fd, 0, sb.st_size );
#else /* USE_SSL */
	if ( do_ssl )
	    send_via_write( fd, sb.st_size );
	else
	    (void) my_sendfile( fd, conn_fd, 0, sb.st_size );
#endif /* USE_SSL */

#else /* HAVE_SENDFILE */

   	send_via_write( fd, sb.st_size );

#endif /* HAVE_SENDFILE */
	}

    (void) close( fd );
    }

#if 0
static void
do_dir( void )
    {
    char buf[10000];
    size_t buflen;
    char* contents;
    size_t contents_size, contents_len;
#ifdef HAVE_SCANDIR
    int n, i;
    struct dirent **dl;
    char* name_info;
#else /* HAVE_SCANDIR */
    char command[10000];
    FILE* fp;
#endif /* HAVE_SCANDIR */

    if ( pathinfo != (char*) 0 )
	send_error( 404, "Not Found", "", "File not found." );

    /* Check authorization for this directory. */
    auth_check( file );

    /* Referer check. */
    check_referer();

#ifdef HAVE_SCANDIR
    n = scandir( file, &dl, NULL, alphasort );
    if ( n < 0 )
	{
	syslog(
	    LOG_INFO, "%.80s Directory \"%.80s\" is protected",
	    ntoa( &client_addr ), path );
	send_error( 403, "Forbidden", "", "Directory is protected." );
	}
#endif /* HAVE_SCANDIR */

    contents_size = 0;
    buflen = snprintf( buf, sizeof(buf), "\
<HTML>\n\
<HEAD><TITLE>Index of %s</TITLE></HEAD>\n\
<BODY BGCOLOR=\"#99cc99\" TEXT=\"#000000\" LINK=\"#2020ff\" VLINK=\"#4040cc\">\n\
<H4>Index of %s</H4>\n\
<PRE>\n",
	file, file );
    add_to_buf( &contents, &contents_size, &contents_len, buf, buflen );

#ifdef HAVE_SCANDIR

    for ( i = 0; i < n; ++i )
	{
	name_info = file_details( file, dl[i]->d_name );
	add_to_buf(
	    &contents, &contents_size, &contents_len, name_info,
	    strlen( name_info ) );
	}

#else /* HAVE_SCANDIR */
    /* Magic HTML ls command! */
    if ( strchr( file, '\'' ) == (char*) 0 )
	{
	(void) snprintf(
	    command, sizeof(command),
	    "ls -lgF '%s' | tail +2 | sed -e 's/^\\([^ ][^ ]*\\)\\(  *[^ ][^ ]*  *[^ ][^ ]*  *[^ ][^ ]*\\)\\(  *[^ ][^ ]*\\)  *\\([^ ][^ ]*  *[^ ][^ ]*  *[^ ][^ ]*\\)  *\\(.*\\)$/\\1 \\3  \\4  |\\5/' -e '/ -> /!s,|\\([^*]*\\)$,|<A HREF=\"\\1\">\\1</A>,' -e '/ -> /!s,|\\(.*\\)\\([*]\\)$,|<A HREF=\"\\1\">\\1</A>\\2,' -e '/ -> /s,|\\([^@]*\\)\\(@* -> \\),|<A HREF=\"\\1\">\\1</A>\\2,' -e 's/|//'",
	    file );
	fp = popen( command, "r" );
	for (;;)
	    {
	    size_t r;
	    r = fread( buf, 1, sizeof(buf), fp );
	    if ( r == 0 )
		break;
	    add_to_buf( &contents, &contents_size, &contents_len, buf, r );
	    }
	(void) pclose( fp );
	}
#endif /* HAVE_SCANDIR */

    buflen = snprintf( buf, sizeof(buf), "\
</PRE>\n\
<HR>\n\
<ADDRESS><A HREF=\"%s\">%s</A></ADDRESS>\n\
</BODY>\n\
</HTML>\n",
	SERVER_URL, SERVER_SOFTWARE );
    add_to_buf( &contents, &contents_size, &contents_len, buf, buflen );

    add_headers( 200, "Ok", "", "", "text/html", contents_len, sb.st_mtime );
    if ( method != METHOD_HEAD )
	add_to_response( contents, contents_len );
    send_response();
    }
#endif

#ifdef HAVE_SCANDIR

static char*
file_details( const char* dir, const char* name )
    {
    struct stat sb;
    char f_time[20];
    static char encname[1000];
    static char buf[2000];

    (void) snprintf( buf, sizeof(buf), "%s/%s", dir, name );
    if ( lstat( buf, &sb ) < 0 )
	return "???";
    (void) strftime( f_time, sizeof( f_time ), "%d%b%Y %H:%M", localtime( &sb.st_mtime ) );
    strencode( encname, sizeof(encname), name );
#ifdef HAVE_INT64T
    (void) snprintf(
	buf, sizeof( buf ), "<A HREF=\"%s\">%-32.32s</A>    %15s %14lld\n",
	encname, name, f_time, (int64_t) sb.st_size );
#else /* HAVE_INT64T */
    (void) snprintf(
	buf, sizeof( buf ), "<A HREF=\"%s\">%-32.32s</A>    %15s %14ld\n",
	encname, name, f_time, (long) sb.st_size );
#endif /* HAVE_INT64T */
    return buf;
    }


/* Copies and encodes a string. */
static void
strencode( char* to, size_t tosize, const char* from )
    {
    int tolen;

    for ( tolen = 0; *from != '\0' && tolen + 4 < tosize; ++from )
	{
	if ( isalnum(*from) || strchr( "/_.-~", *from ) != (char*) 0 )
	    {
	    *to = *from;
	    ++to;
	    ++tolen;
	    }
	else
	    {
	    (void) sprintf( to, "%%%02x", (int) *from & 0xff );
	    to += 3;
	    tolen += 3;
	    }
	}
    *to = '\0';
    }

#endif /* HAVE_SCANDIR */


static void
do_cgi( void )
    {
    char** argp;
    char** envp;
    int parse_headers;
    char* binary;
    char* directory;

    if ( method != METHOD_GET && method != METHOD_POST )
	send_error( 501, "Not Implemented", "", "That method is not implemented for CGI." );

    /* If the socket happens to be using one of the stdin/stdout/stderr
    ** descriptors, move it to another descriptor so that the dup2 calls
    ** below don't screw things up.  We arbitrarily pick fd 3 - if there
    ** was already something on it, we clobber it, but that doesn't matter
    ** since at this point the only fd of interest is the connection.
    ** All others will be closed on exec.
    */
    if ( conn_fd == STDIN_FILENO || conn_fd == STDOUT_FILENO || conn_fd == STDERR_FILENO )
	{
	int newfd = dup2( conn_fd, STDERR_FILENO + 1 );
	if ( newfd >= 0 )
	    conn_fd = newfd;
	/* If the dup2 fails, shrug.  We'll just take our chances.
	** Shouldn't happen though.
	*/
	}

    /* Make the environment vector. */
    envp = make_envp();

    /* Make the argument vector. */
    argp = make_argp();

    /* Set up stdin.  For POSTs we may have to set up a pipe from an
    ** interposer process, depending on if we've read some of the data
    ** into our buffer.  We also have to do this for all SSL CGIs.
    */
#ifdef USE_SSL
    if ( ( method == METHOD_POST && request_len > request_idx ) || do_ssl )
#else /* USE_SSL */
    if ( ( method == METHOD_POST && request_len > request_idx ) )
#endif /* USE_SSL */
	{
	int p[2];
	int r;
	if ( pipe( p ) < 0 ){

	    send_error( 500, "Internal Error", "", "Something unexpected went wrong making a pipe." );
	
	}
	r = fork();
	if ( r < 0 ){

	    send_error( 500, "Internal Error", "", "Something unexpected went wrong forking an interposer." );
	}
	if ( r == 0 )
	    {

	    /* Interposer process. */
	    (void) close( p[0] );
	    cgi_interpose_input( p[1] );
	    exit( 0 );
	    }
	(void) close( p[1] );
	if ( p[0] != STDIN_FILENO )
	    {
	    (void) dup2( p[0], STDIN_FILENO );
	    (void) close( p[0] );
	    }
	}
    else
	{

	/* Otherwise, the request socket is stdin. */
	if ( conn_fd != STDIN_FILENO )
	    (void) dup2( conn_fd, STDIN_FILENO );
	}

    /* Set up stdout/stderr.  For SSL, or if we're doing CGI header parsing,
    ** we need an output interposer too.
    */
    if ( strncmp( argp[0], "nph-", 4 ) == 0 )
	parse_headers = 0;
    else
	parse_headers = 1;
#ifdef USE_SSL
    if ( parse_headers || do_ssl )
#else /* USE_SSL */
    if ( parse_headers )
#endif /* USE_SSL */
	{
	int p[2];
	int r;

	if ( pipe( p ) < 0 )
	    send_error( 500, "Internal Error", "", "Something unexpected went wrong making a pipe." );

	r = fork();
	if ( r < 0 )
	    send_error( 500, "Internal Error", "", "Something unexpected went wrong forking an interposer." );

	if ( r == 0 )
	    {

	    /* Interposer process. */
	    (void) close( p[1] );

	    cgi_interpose_output( p[0], parse_headers );
	    exit( 0 );
	    }
	(void) close( p[0] );
	if ( p[1] != STDOUT_FILENO )
	    (void) dup2( p[1], STDOUT_FILENO );
	if ( p[1] != STDERR_FILENO )
	    (void) dup2( p[1], STDERR_FILENO );
	if ( p[1] != STDOUT_FILENO && p[1] != STDERR_FILENO )
	    (void) close( p[1] );
	}
    else
	{
	/* Otherwise, the request socket is stdout/stderr. */
	if ( conn_fd != STDOUT_FILENO )
	    (void) dup2( conn_fd, STDOUT_FILENO );
	if ( conn_fd != STDERR_FILENO )
	    (void) dup2( conn_fd, STDERR_FILENO );
	}

    /* At this point we would like to set conn_fd to be close-on-exec.
    ** Unfortunately there seems to be a Linux problem here - if we
    ** do this close-on-exec in Linux, the socket stays open but stderr
    ** gets closed - the last fd duped from the socket.  What a mess.
    ** So we'll just leave the socket as is, which under other OSs means
    ** an extra file descriptor gets passed to the child process.  Since
    ** the child probably already has that file open via stdin stdout
    ** and/or stderr, this is not a problem.
    */
    /* (void) fcntl( conn_fd, F_SETFD, 1 ); */

    /* Close the log file. */
    if ( logfp != (FILE*) 0 )
	(void) fclose( logfp );

    /* Close syslog. */
    closelog();

    /* Set priority. */
    (void) nice( CGI_NICE );

    /* Split the program into directory and binary, so we can chdir()
    ** to the program's own directory.  This isn't in the CGI 1.1
    ** spec, but it's what other HTTP servers do.
    */
    directory = e_strdup( file );
    binary = strrchr( directory, '/' );
    if ( binary == (char*) 0 )
	binary = file;
    else
	{
	*binary++ = '\0';
	(void) chdir( directory );	/* ignore errors */
	}

    /* Default behavior for SIGPIPE. */
#ifdef HAVE_SIGSET
    (void) sigset( SIGPIPE, SIG_DFL );
#else /* HAVE_SIGSET */
    (void) signal( SIGPIPE, SIG_DFL );
#endif /* HAVE_SIGSET */

    /* Run the program. */
    (void) execve( binary, argp, envp );

    /* Something went wrong. */
    send_error( 500, "Internal Error", "", "Something unexpected went wrong running a CGI program." );

    }


/* This routine is used only for POST requests.  It reads the data
** from the request and sends it to the child process.  The only reason
** we need to do it this way instead of just letting the child read
** directly is that we have already read part of the data into our
** buffer.
**
** Oh, and it's also used for all SSL CGIs.
*/
static void
cgi_interpose_input( int wfd )
    {
    size_t c;
    ssize_t r;
    char buf[1024];

    c = request_len - request_idx;
    if ( c > 0 )
	{
	if ( write( wfd, &(request[request_idx]), c ) != c )
	    return;
	}
    while ( c < content_length )
	{
	r = my_read( buf, min( sizeof(buf), content_length - c ) );
	if ( r == 0 )
	    return;
	else if ( r < 0 )
	    {
	    if ( errno == EAGAIN )
		sleep( 1 );
	    else
		return;
	    }
	else
	    {
	    if ( write( wfd, buf, r ) != r )
		return;
	    c += r;
	    }
	}
    post_post_garbage_hack();
    }


/* Special hack to deal with broken browsers that send a LF or CRLF
** after POST data, causing TCP resets - we just read and discard up
** to 2 bytes.  Unfortunately this doesn't fix the problem for CGIs
** which avoid the interposer process due to their POST data being
** short.  Creating an interposer process for all POST CGIs is
** unacceptably expensive.
*/
static void
post_post_garbage_hack( void )
    {
    char buf[2];

#ifdef USE_SSL
    if ( do_ssl )
	/* We don't need to do this for SSL, since the garbage has
	** already been read.  Probably.
	*/
	return;
#endif /* USE_SSL */

    set_ndelay( conn_fd );
    (void) read( conn_fd, buf, sizeof(buf) );
    clear_ndelay( conn_fd );
    }


/* This routine is used for parsed-header CGIs and for all SSL CGIs. */
static void
cgi_interpose_output( int rfd, int parse_headers )
    {
    ssize_t r;
    char buf[1024];

    if ( ! parse_headers )
	{
	/* If we're not parsing headers, write out the default status line
	** and proceed to the echo phase.
	*/
	char http_head[] = "HTTP/1.0 200 OK\r\n";

	(void) my_write( http_head, sizeof(http_head) );
	}
    else
	{
	/* Header parsing.  The idea here is that the CGI can return special
	** headers such as "Status:" and "Location:" which change the return
	** status of the response.  Since the return status has to be the very
	** first line written out, we have to accumulate all the headers
	** and check for the special ones before writing the status.  Then
	** we write out the saved headers and proceed to echo the rest of
	** the response.
	*/
	size_t headers_size, headers_len;
	char* headers;
	char* br;
	int status;
	char* title;
	char* cp;

	/* Slurp in all headers. */
	headers_size = 0;
	add_to_buf( &headers, &headers_size, &headers_len, (char*) 0, 0 );
	for (;;)
	    {
	    r = read( rfd, buf, sizeof(buf) );
	    if ( r <= 0 )
		{
		br = &(headers[headers_len]);
		break;
		}
	    add_to_buf( &headers, &headers_size, &headers_len, buf, r );
	    if ( ( br = strstr( headers, "\r\n\r\n" ) ) != (char*) 0 ||
		 ( br = strstr( headers, "\n\n" ) ) != (char*) 0 )
		break;
	    }

	/* If there were no headers, bail. */
	if ( headers[0] == '\0' )
	    return;

	/* Figure out the status. */
	status = 200;
	if ( ( cp = strstr( headers, "Status:" ) ) != (char*) 0 &&
	     cp < br &&
	     ( cp == headers || *(cp-1) == '\n' ) )
	    {
	    cp += 7;
	    cp += strspn( cp, " \t" );
	    status = atoi( cp );
	    }

	if ( ( cp = strstr( headers, "Location:" ) ) != (char*) 0 &&
	     cp < br &&
	     ( cp == headers || *(cp-1) == '\n' ) )
	    status = 302;

	/* Write the status line. */
	switch ( status )
	    {
	    case 200: title = "OK"; break;
	    case 302: title = "Found"; break;
	    case 304: title = "Not Modified"; break;
	    case 400: title = "Bad Request"; break;
	    case 401: title = "Unauthorized"; break;
	    case 403: title = "Forbidden"; break;
	    case 404: title = "Not Found"; break;
	    case 408: title = "Request Timeout"; break;
	    case 500: title = "Internal Error"; break;
	    case 501: title = "Not Implemented"; break;
	    case 503: title = "Service Temporarily Overloaded"; break;
	    default: title = "Something"; break;
	    }
	(void) snprintf(
	    buf, sizeof(buf), "HTTP/1.0 %d %s\r\n", status, title );
	(void) my_write( buf, strlen( buf ) );

	/* Write the saved headers. */
	(void) my_write( headers, headers_len );
	}

    /* Echo the rest of the output. */
    for (;;)
	{
	r = read( rfd, buf, sizeof(buf) );
	if ( r <= 0 )
	    break;
	if ( my_write( buf, r ) != r )
	    break;
	}
    shutdown( conn_fd, SHUT_WR );
    }


/* Set up CGI argument vector.  We don't have to worry about freeing
** stuff since we're a sub-process.  This gets done after make_envp() because
** we scribble on query.
*/
static char**
make_argp( void )
    {
    char** argp;
    int argn;
    char* cp1;
    char* cp2;

    /* By allocating an arg slot for every character in the query, plus
    ** one for the filename and one for the NULL, we are guaranteed to
    ** have enough.  We could actually use strlen/2.
    */
    argp = (char**) malloc( ( strlen( query ) + 2 ) * sizeof(char*) );
    if ( argp == (char**) 0 )
	return (char**) 0;

    argp[0] = strrchr( file, '/' );
    if ( argp[0] != (char*) 0 )
	++argp[0];
    else
	argp[0] = file;

    argn = 1;
    /* According to the CGI spec at http://hoohoo.ncsa.uiuc.edu/cgi/cl.html,
    ** "The server should search the query information for a non-encoded =
    ** character to determine if the command line is to be used, if it finds
    ** one, the command line is not to be used."
    */
    if ( strchr( query, '=' ) == (char*) 0 )
	{
	for ( cp1 = cp2 = query; *cp2 != '\0'; ++cp2 )
	    {
	    if ( *cp2 == '+' )
		{
		*cp2 = '\0';
		strdecode( cp1, cp1 );
		argp[argn++] = cp1;
		cp1 = cp2 + 1;
		}
	    }
	if ( cp2 != cp1 )
	    {
	    strdecode( cp1, cp1 );
	    argp[argn++] = cp1;
	    }
	}

    argp[argn] = (char*) 0;
    return argp;
    }


/* Set up CGI environment variables. Be real careful here to avoid
** letting malicious clients overrun a buffer.  We don't have
** to worry about freeing stuff since we're a sub-process.
*/
static char**
make_envp( void )
    {
    static char* envp[50];
    int envn;
    char* cp;
    char buf[256];

    envn = 0;
    envp[envn++] = build_env( "PATH=%s", CGI_PATH );
    envp[envn++] = build_env( "LD_LIBRARY_PATH=%s", CGI_LD_LIBRARY_PATH );
    envp[envn++] = build_env( "SERVER_SOFTWARE=%s", SERVER_SOFTWARE );
    if ( ! vhost )
	cp = hostname;
    else
	cp = req_hostname;	/* already computed by virtual_file() */
    if ( cp != (char*) 0 )
	envp[envn++] = build_env( "SERVER_NAME=%s", cp );
    envp[envn++] = "GATEWAY_INTERFACE=CGI/1.1";
    envp[envn++] = "SERVER_PROTOCOL=HTTP/1.0";
    (void) snprintf( buf, sizeof(buf), "%d", port );
    envp[envn++] = build_env( "SERVER_PORT=%s", buf );
    envp[envn++] = build_env(
	"REQUEST_METHOD=%s", get_method_str( method ) );
    envp[envn++] = build_env( "SCRIPT_NAME=%s", path );
    if ( pathinfo != (char*) 0 )
	{
	envp[envn++] = build_env( "PATH_INFO=/%s", pathinfo );
	(void) snprintf( buf, sizeof(buf), "%s%s", cwd, pathinfo );
	envp[envn++] = build_env( "PATH_TRANSLATED=/%s", buf );
	}
    if ( query[0] != '\0' )
	envp[envn++] = build_env( "QUERY_STRING=%s", query );
    envp[envn++] = build_env( "REMOTE_ADDR=%s", ntoa( &client_addr ) );
    if ( referer[0] != '\0' )
	envp[envn++] = build_env( "HTTP_REFERER=%s", referer );
    if ( useragent[0] != '\0' )
	envp[envn++] = build_env( "HTTP_USER_AGENT=%s", useragent );
    if ( cookie != (char*) 0 )
	envp[envn++] = build_env( "HTTP_COOKIE=%s", cookie );
    if ( content_type != (char*) 0 )
	envp[envn++] = build_env( "CONTENT_TYPE=%s", content_type );
    if ( content_length != -1 )
	{
	(void) snprintf(
	    buf, sizeof(buf), "%lu", (unsigned long) content_length );
	envp[envn++] = build_env( "CONTENT_LENGTH=%s", buf );
	}
    if ( remoteuser != (char*) 0 )
	envp[envn++] = build_env( "REMOTE_USER=%s", remoteuser );
    if ( authorization != (char*) 0 )
	envp[envn++] = build_env( "AUTH_TYPE=%s", "Basic" );
    if ( getenv( "TZ" ) != (char*) 0 )
	envp[envn++] = build_env( "TZ=%s", getenv( "TZ" ) );

    envp[envn] = (char*) 0;
    return envp;
    }


static char*
build_env( char* fmt, char* arg )
    {
    char* cp;
    int size;
    static char* buf;
    static int maxbuf = 0;

    size = strlen( fmt ) + strlen( arg );
    while ( size > maxbuf )
	{
	if ( maxbuf == 0 )
	    {
	    maxbuf = 256;
	    buf = (char*) e_malloc( maxbuf );
	    }
	else
	    {
	    maxbuf *= 2;
	    buf = (char*) e_realloc( (void*) buf, maxbuf );
	    }
	}
    (void) snprintf( buf, maxbuf, fmt, arg );
    cp = e_strdup( buf );
    return cp;
    }


/*
 * needPassword() -- check if user need input password.
 *
 *      1 ==> Yes, you need input password.
 *      0 ==> No. no need password input.
 */
int needPassword(char *authpath, char *line) {
    FILE *fp ;
    int l;
    char *cryp;

    if(!authpath) {
        /* no authpath, ofcourse no need password */
        return 0;
    }

    fp = fopen(authpath, "r");
    if(!fp) {
        /* no authpath, ofcourse no need password */
        return 0;
    }

    while ( fgets( line, sizeof(line), fp ) != (char*) 0 ) {
    	/* Nuke newline. */
    	l = strlen( line );
    	if ( line[l - 1] == '\n' )
    	    line[l - 1] = '\0';
    	/* Split into user and encrypted password. */
    	cryp = strchr( line, ':' );
    	if ( cryp == (char*) 0 )
    	    continue;
        if(strlen(line) > 1 ) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

static void
auth_check( char* dirname )
    {
    char authpath[10000];
    struct stat sb;
    char authinfo[500];
    char* authpass;
    char* colon;
    static char line[10000];
    int l;
    FILE* fp;
    char* cryp;
    
    /* Construct auth filename. */
    if ( dirname[strlen(dirname) - 1] == '/' )
	(void) snprintf( authpath, sizeof(authpath), "%s%s", dirname, AUTH_FILE );
    else
	(void) snprintf( authpath, sizeof(authpath), "%s/%s", dirname, AUTH_FILE );

	/* carl modify for http authtication*/
	sprintf(authpath,"/tmp/htpasswd");
    /* Does this directory have an auth file? or no need input password*/
    if ( stat( authpath, &sb ) < 0 || !needPassword(authpath, line) ){
        /* Ron */

    	if(strcmp(last_remote_ip,remote_ip)!=0){
    		syslog(LOG_INFO,"Administrator login successful - IP:%s",remote_ip);
    		SAVETIME;	
    	}else if(check_timeout())
    		LOGOUT;
    	else
    	SAVETIME;    		
    	/* Ron */
    	/* Nope, let the request go through. */
    	return;
    }

    not_auth=0;
    /* Does this request contain authorization info? */
    if ( authorization == (char*) 0 ){
        not_auth=1;
    	/* Nope, return a 401 Unauthorized. */
    	send_authenticate( dirname );
    	    

    }
    /* Basic authorization info? */
    if ( strncmp( authorization, "Basic ", 6 ) != 0 ){
	    send_authenticate( dirname );
	       

    }
    /* Decode it. */

    l = b64_decode(
	&(authorization[6]), (unsigned char*) authinfo, sizeof(authinfo) - 1 );
    authinfo[l] = '\0';
    /* Split into user and password. */
    authpass = strchr( authinfo, ':' );
    if ( authpass == (char*) 0 )
    {
		/* No colon?  Bogus auth info. */
		send_authenticate( dirname );
	}

    *authpass++ = '\0';
    /* If there are more fields, cut them off. */
    colon = strchr( authpass, ':' );
    if ( colon != (char*) 0 )
	*colon = '\0';

    /* Open the password file. */
    fp = fopen( authpath, "r" );
    if ( fp == (FILE*) 0 )
	{
	/* The file exists but we can't open it?  Disallow access. */
#ifdef SYSLOG
	syslog(
	    LOG_ERR, "%.80s auth file %.80s could not be opened - %m",
	    ntoa( &client_addr ), authpath );
#endif
	send_error( 403, "Forbidden", "", "File is protected." );
	}

    /* Read it. */
    while ( fgets( line, sizeof(line), fp ) != (char*) 0 )
	{
	/* Nuke newline. */
	l = strlen( line );
	if ( line[l - 1] == '\n' )
	    line[l - 1] = '\0';
	/* Split into user and encrypted password. */
	cryp = strchr( line, ':' );
	if ( cryp == (char*) 0 )
	    continue;
	*cryp++ = '\0';
	/* Is this the right user? */
	if ( strcmp( line, authinfo ) == 0 )
	{
        struct sysinfo info;
        sysinfo(&info);
	    /* Yes. */
	    (void) fclose( fp );
	    /* So is the password right? */
/*Ron*/
#if 0
	    if ( strcmp( crypt( authpass, cryp ), cryp ) == 0 )
#endif
		
    	/* OK !*/
    	if ( strcmp(authpass, cryp ) == 0){
    		if((strcmp(last_remote_ip,remote_ip)!=0)){
/* Mike@CPU_AP add for send syslog */
    			//mBUG("<%s,%d>:authlogin", __FUNCTION__, __LINE__);    			
    			syslog(LOG_INFO,"[SC][AuthLogin]Authorized Login from %s",remote_ip);   			
/* add end */
        			/* if different user must authentication in frist time */
    			if (last_remote_ip[0]!=0)
    				send_authenticate( dirname );
    			/*Time out */
    		}else if(check_timeout()) {
    	        send_authenticateTimeout( dirname );
    		}
    		remoteuser = line;
    		LOGOUT;
    		SAVETIME;	
    		return;
    	}else{		
		    /* No. */
        	not_auth=1;
/* Mike@CPU_AP add for send syslog */
        	syslog(LOG_INFO,"[SC][UnAuthLogin]Unauthorized Login Attempt from %s",remote_ip);
/* add end */
		    send_authenticate( dirname );
	    }
	}

/* Mike@CPU_AP add for send syslog */
	//mBUG("<%s,%d>:usrname wrong", __FUNCTION__, __LINE__);
	syslog(LOG_INFO,"[SC][UnAuthLogin]Unauthorized Login Attempt from %s",remote_ip);
/* add end */
	}

    /* Didn't find that user.  Access denied. */
    (void) fclose( fp );

    send_authenticate( dirname );
    }


static void
send_authenticate( char* realm )
    {
    char header[2048];
    char realm_buf[64];
    int length;
/* Ron */  
    if(http_realm==NULL)
	    http_realm=realm;
    LOGOUT;
/* Ron */	
	length = strlen(http_realm);
	strncpy(realm_buf,http_realm,length);
	realm_buf[length] = 0;
    (void) snprintf(header, sizeof(header), "WWW-Authenticate: Basic realm=\"%s\"", realm_buf);
    send_error( 401, "Unauthorized", header, "Authorization required." );
    }

static void
send_authenticateTimeout( char* realm )
{
    char scPWTimeoutMessage[]= "<html><head><meta http-equiv=\'Pragma\' content=\'no-cache\'><meta http-equiv=\'Cache-Control\' content=\'no-cache\'><title> Authorization warning</title><script language=\"javascript\" type=\"text/javascript\">function cancelevent(){document.formname.submit();}</script></head><body onload=cancelevent()><form name=\"formname\" method=\"POST\" action=\"ptimeout.cgi\"><b>401 Unauthorized</b><input type=\"hidden\" name=\"inputname\" value=\"\"></form></body></html>";
    char header[2048];
    char realm_buf[64];
    int length;
/* Ron */  
    if(http_realm==NULL)
	    http_realm=realm;
	LOGOUT;    
/* Ron */	
	length = strlen(http_realm);
	strncpy(realm_buf,http_realm,length);
	realm_buf[length] = 0;
    (void) snprintf(header, sizeof(header), "WWW-Authenticate: Basic realm=\"%s\"", realm_buf);
    send_error( 401, "Unauthorized", header, scPWTimeoutMessage );
}

static void
send_authenticateWarning( char* realm )
{
    char scPWWarningMessage[]="<html><head><meta name=\"description\" content=\"EWCAP 1001\"><meta http-equiv=\"content-type\" content=\"text/html;charset=ISO-8859-1\"><META http-equiv=\"Pragma\" CONTENT=\"no-cache\"><META HTTP-EQUIV=\"Cache-Control\" CONTENT=\"no-cache\"><meta HTTP-EQUIV=\"Expires\" CONTENT=\"Mon, 06 Jan 1990 00:00:01 GMT\"><title>Authorization warning</title></head><body background=\"#ffffff\"><table border=\"0\" cellpadding=\"0\" cellspacing=\"3\" width=\"100%\"><tr><td colspan=\"2\"><h1><font face=\"Arial, Helvetica, Geneva, Swiss, SunSans-Regular, sans-serif\" size=\"4\">System Authentication Failed.</font></h1><hr></td></tr><tr><td colspan=\"2\"><font face=\"Arial, Helvetica, Geneva, Swiss, SunSans-Regular, sans-serif\" size=\"2\">Please contact your system administrator for the correct information.</font><hr></td></tr></table></body></html>";
    int length;

    add_headers( 200, "Ok", "", "", "text/html", strlen(scPWWarningMessage), (time_t) -1);
    add_to_response( scPWWarningMessage, strlen(scPWWarningMessage));
    send_response();
    
#ifdef USE_SSL
    SSL_free( ssl );
#endif
    exit( 1 );
}

#ifdef USE_SSL
static char *auto_https_head = "<html><head><meta http-equiv=\'Content-Type\' content=\'text/html; charset=iso8859-1\'><META http-equiv=Refresh content=\"0; URL=";
static char *auto_https_tail = "\"></head><body bgcolor=#FFFFFF></body></html>";
typedef struct if_info_s{
	char ifname[16];
	char ipaddr[16];
	char mac[18];
	char mask[16];
	struct in_addr gw;
	int  mtu;
	
}if_info_t;
int getIFInfo(char *if_name, if_info_t *if_info)
{
	unsigned char *pt;
	struct ifreq ifr;
	struct sockaddr_in *saddr;
	int fd;
	int ret=0;

	strcpy(if_info->ifname,if_name);
	if ((fd=get_sockfd())>=0)
	{
		strcpy(ifr.ifr_name, if_info->ifname);
		ifr.ifr_addr.sa_family = AF_INET;
		/* get ip address */
		if (ioctl(fd, SIOCGIFADDR, &ifr)==0){
			saddr = (struct sockaddr_in *)&ifr.ifr_addr;
			strcpy(if_info->ipaddr,(char *)inet_ntoa(saddr->sin_addr));		
		}else
			ret=-1;
		return ret;

	}
	return -1;
}

int getMgtBrInfo(if_info_t *if_info)
{
    char ifName[16];
    FILE *fp;
    char line[64+1];
    int vlanId = 0;
    
    /*If vlan disabled*/
    if((fp=fopen("/tmp/vlan.conf","r"))==NULL)
    {
        sprintf(ifName,"br0");
    }
    else
    {
        fgets(line, 64, fp); /* eat native vlan id */
        fgets(line, 64, fp); /* management vlan id */
        vlanId = atoi(line);
        sprintf(ifName,"br%d", vlanId);
        fclose(fp);
    }
    return getIFInfo(ifName, if_info);
}

static void
sendAutoHttps(char* path)
{ 
    char url[150], lPath[65],ipstr[20];
    char outMsgBuf[2048]; 
    int len = 0;    
    char *pStr;
  #ifdef USE_SSL
    struct if_info_s if_info_t;
    struct if_info_s *if_info;
    if_info=&if_info_t;
    
    getMgtBrInfo(if_info);    
    strcpy(ipstr,if_info->ipaddr);
  #endif  
    
    if(ports!=DEFAULT_HTTPS_PORT)
        len += sprintf(url,"https://%s:%d",ipstr, ports);
    else
        len += sprintf(url,"https://%s",ipstr);
            
    if(path){
        strncpy(lPath, path, 64);
        pStr = strstr(lPath, " ");
        if(pStr)
            *pStr = 0;
            
        len += sprintf(url+len,"%s", lPath);
    }

    sprintf(outMsgBuf,"%s%s%s",auto_https_head,url,auto_https_tail );
    add_headers( 200, "Ok", "", "", "text/html", strlen(outMsgBuf), (time_t) -1);
    add_to_response( outMsgBuf, strlen(outMsgBuf));
    send_response();
    
#ifdef USE_SSL
    SSL_free( ssl );
#endif
    exit( 1 );            
}
#endif

static char *auto_http_head = "<html><head><meta http-equiv=\'Content-Type\' content=\'text/html; charset=iso8859-1\'><META http-equiv=Refresh content=\"0; URL=";
static char *auto_http_tail = "\"></head><body bgcolor=#FFFFFF></body></html>";

static void
sendAutoHttp(char* path)
{   
    char url[150], lPath[65],ipstr[20];
    char outMsgBuf[2048]; 
    int len = 0;    
    char *pStr;
    
    struct if_info_s if_info_t;

    getMgtBrInfo(&if_info_t);    
    strcpy(ipstr,if_info_t.ipaddr);

    if(port!=DEFAULT_HTTP_PORT)
        len += sprintf(url,"http://%s:%d",ipstr, httpport);
    else
        len += sprintf(url,"http://%s",ipstr);
            
    if(path){
        strncpy(lPath, path, 64);
        pStr = strstr(lPath, " ");
        if(pStr)
            *pStr = 0;
            
        len += sprintf(url+len,"%s", lPath);
    }

    sprintf(outMsgBuf,"%s%s%s",auto_http_head,url,auto_http_tail );
    add_headers( 200, "Ok", "", "", "text/html", strlen(outMsgBuf), (time_t) -1);
    add_to_response( outMsgBuf, strlen(outMsgBuf));
    send_response();
    
#ifdef USE_SSL
    SSL_free( ssl );
#endif
    exit( 1 );          
}
static char*
virtual_file( char* file )
    {
    char* cp;
    static char vfile[10000];

    /* Use the request's hostname, or fall back on the IP address. */
    if ( host != (char*) 0 )
	req_hostname = host;
    else
	{
	usockaddr usa;
	int sz = sizeof(usa);
	if ( getsockname( conn_fd, &usa.sa, &sz ) < 0 )
	    req_hostname = "UNKNOWN_HOST";
	else
	    req_hostname = ntoa( &usa );
	}
    /* Pound it to lower case. */
    for ( cp = req_hostname; *cp != '\0'; ++cp )
	if ( isupper( *cp ) )
	    *cp = tolower( *cp );
    (void) snprintf( vfile, sizeof(vfile), "%s/%s", req_hostname, file );
    return vfile;
    }
#ifdef _USE_COOKIES_ 
/*type=1:timeout ;type=3: redirect home page;else  pwd and user error */
static void send_return(int type)
{
	char buf[10000];
    int buflen;
    LOGOUT;
    if(type==1){
    	 buflen = snprintf(
	buf, sizeof(buf), "\
<HTML>\n\
<HEAD>\n\
<META http-equiv=\"Pragma\" CONTENT=\"no-cache\">\n\
<META HTTP-EQUIV=\"Cache-Control\" CONTENT=\"no-cache\">\n\
<meta http-equiv=\"Expires\" content=\"-1\">\n\
<script language=\"javascript\" type=\"text/javascript\">\n\
function loadnext() {\n\
if(window.opener){window.opener.top.location.href=\"login.htm&no=2\";self.close();}\n\
else{top.location.href=\"login.htm&no=2\";}}\n\
</script>\n\
</HEAD>\n\
<BODY  onload=\"loadnext()\">\n\
</BODY>\n\
</html>\n");
	}
	else if(type==2){
		 buflen = snprintf(
	buf, sizeof(buf), "\
<HTML>\n\
<HEAD>\n\
<META http-equiv=\"Pragma\" CONTENT=\"no-cache\">\n\
<META HTTP-EQUIV=\"Cache-Control\" CONTENT=\"no-cache\">\n\
<meta http-equiv=\"Expires\" content=\"-1\">\n\
<script language=\"javascript\" type=\"text/javascript\">\n\
function loadnext() {\n\
if(window.opener){window.opener.top.location.href=\"../login.htm&no=2\";self.close();}\n\
else{top.location.href=\"../login.htm&no=2\";}}\n\
</script>\n\
</HEAD>\n\
<BODY  onload=\"loadnext()\">\n\
</BODY>\n\
</html>\n");
	
	}
	else if(type==3){
		 FILE *fp = NULL;
    	char line[128];
    	 fp = fopen("/tmp/nextpage", "r");
   		 if(fp)
   		{
   			if(fgets(line, sizeof(line), fp))
				{
		 buflen = snprintf(
	buf, sizeof(buf), "\
<HTML>\n\
<HEAD>\n\
<META http-equiv=\"Pragma\" CONTENT=\"no-cache\">\n\
<META HTTP-EQUIV=\"Cache-Control\" CONTENT=\"no-cache\">\n\
<meta http-equiv=\"Expires\" content=\"-1\">\n\
<script language=\"javascript\" type=\"text/javascript\">\n\
function loadnext() {var fm = document.forms[0];fm.submit();}\n\
</script>\n\
</HEAD>\n\
<BODY  onload=\"loadnext()\">\n\
<form action=\"Home.htm\" method=\"post\">\n\
<input type=\"hidden\" name=\"next_file\" value=\"Home.htm\">\n\
<input type=\"hidden\" name=\"this_file\" value=\"Home.htm\">\n\
<input type=\"hidden\" name=\"rpage\" value=\"%s\">\n\
</form></BODY>\n\
</html>\n",line);
		}
		fclose(fp);
       	unlink("/tmp/nextpage");
	}
	}
    else{
	 buflen = snprintf(
	buf, sizeof(buf), "\
<HTML>\n\
<HEAD>\n\
<META http-equiv=\"Pragma\" CONTENT=\"no-cache\">\n\
<META HTTP-EQUIV=\"Cache-Control\" CONTENT=\"no-cache\">\n\
<meta http-equiv=\"Expires\" content=\"-1\">\n\
<script language=\"javascript\" type=\"text/javascript\">\n\
function loadnext() {\n\
if(window.opener){window.opener.top.location.href=\"login.htm&no=3\";self.close();}\n\
else{top.location.href=\"login.htm&no=3\";}}\n\
</script>\n\
</HEAD>\n\
<BODY  onload=\"loadnext()\">\n\
</BODY>\n\
</html>\n");
    syslog(LOG_INFO,"[SC][UnAuthLogin]Unauthorized Login Attempt from %s",remote_ip);

	}
	
    add_to_response( buf, buflen );
	send_response();
	#ifdef USE_SSL
    SSL_free( ssl );
	#endif /* USE_SSL */
    exit( 1 );
}
#endif
static void
send_error( int s, char* title, char* extra_header, char* text )
    {
    add_headers(
	s, title, extra_header, "", "text/html", (off_t) -1, (time_t) -1 );

    send_error_body( s, title, text );

//    send_error_tail();

    send_response();

#ifdef USE_SSL
    SSL_free( ssl );
#endif /* USE_SSL */
    exit( 1 );
    }


static void
send_error_body( int s, char* title, char* text )
    {
    char filename[1000];
    char buf[10000];
    int buflen;

    if ( vhost && req_hostname != (char*) 0 )
	{
	/* Try virtual-host custom error page. */
	(void) snprintf(
	    filename, sizeof(filename), "%s/%s/err%d.html",
	    req_hostname, ERR_DIR, s );
	if ( send_error_file( filename ) )
	    return;
	}

    /* Try server-wide custom error page. */
    if(someone_in_use==1)
    	(void) snprintf(filename, sizeof(filename), "%s/err888.html", ERR_DIR);
    else if(not_auth==1)
    	(void) snprintf(filename, sizeof(filename), "%s/err%d.htm", ERR_DIR, s );
    else
    	(void) snprintf(filename, sizeof(filename), "%s/err%d.html", ERR_DIR, s );
	    
    if ( send_error_file( filename ) )
	return;
    
    /* Send built-in error page. */
    buflen = snprintf(
	buf, sizeof(buf), "\
<HTML>\n\
<HEAD><TITLE>%d %s</TITLE></HEAD>\n\
<BODY BGCOLOR=\"#cc9999\" TEXT=\"#000000\" LINK=\"#2020ff\" VLINK=\"#4040cc\">\n\
<H4>%d %s</H4>\n",
	s, title, s, title );
    add_to_response( buf, buflen );
    buflen = snprintf( buf, sizeof(buf), "%s\n", text );
    add_to_response( buf, buflen );
    }


static int
send_error_file( char* filename )
    {
    FILE* fp;
    char buf[1000];
    size_t r;

    fp = fopen( filename, "r" );
    if ( fp == (FILE*) 0 )
	return 0;
    for (;;)
	{
	r = fread( buf, 1, sizeof(buf), fp );
	if ( r == 0 )
	    break;
	add_to_response( buf, r );
	}
    (void) fclose( fp );
    return 1;
    }


static void
send_error_tail( void )
    {
    char buf[500];
    int buflen;

    if ( match( "**MSIE**", useragent ) )
	{
	int n;
	buflen = snprintf( buf, sizeof(buf), "<!--\n" );
	add_to_response( buf, buflen );
	for ( n = 0; n < 6; ++n )
	    {
	    buflen = snprintf( buf, sizeof(buf), "Padding so that MSIE deigns to show this error instead of its own canned one.\n" );
	    add_to_response( buf, buflen );
	    }
	buflen = snprintf( buf, sizeof(buf), "-->\n" );
	add_to_response( buf, buflen );
	}

    buflen = snprintf( buf, sizeof(buf), "\
<HR>\n\
<ADDRESS><A HREF=\"%s\">%s</A></ADDRESS>\n\
</BODY>\n\
</HTML>\n",
	SERVER_URL, SERVER_SOFTWARE );
    add_to_response( buf, buflen );
    }


static void
add_headers( int s, char* title, char* extra_header, char* me, char* mt, off_t b, time_t mod )
    {
    time_t now, expires;
    char timebuf[100];
    char buf[10000];
    int buflen;
    const char* rfc1123_fmt = "%a, %d %b %Y %H:%M:%S GMT";

    status = s;
    bytes = b;
    make_log_entry();
    start_response();
    buflen = snprintf( buf, sizeof(buf), "%s %d %s\r\n", protocol, status, title );
    add_to_response( buf, buflen );
    buflen = snprintf( buf, sizeof(buf), "Server: %s\r\n", SERVER_SOFTWARE );
    add_to_response( buf, buflen );
    now = time( (time_t*) 0 );
    (void) strftime( timebuf, sizeof(timebuf), rfc1123_fmt, gmtime( &now ) );
    buflen = snprintf( buf, sizeof(buf), "Date: %s\r\n", timebuf );
    add_to_response( buf, buflen );
    if ( extra_header != (char*) 0 && extra_header[0] != '\0' )
	{
	buflen = snprintf( buf, sizeof(buf), "%s\r\n", extra_header );
	add_to_response( buf, buflen );
	}
    if ( me != (char*) 0 && me[0] != '\0' )
	{
	buflen = snprintf( buf, sizeof(buf), "Content-Encoding: %s\r\n", me );
	add_to_response( buf, buflen );
	}
    if ( mt != (char*) 0 && mt[0] != '\0' )
	{
	buflen = snprintf( buf, sizeof(buf), "Content-Type: %s\r\n", mt );
	add_to_response( buf, buflen );
	}
    if ( bytes >= 0 )
	{
#ifdef HAVE_INT64T
	buflen = snprintf(
	    buf, sizeof(buf), "Content-Length: %lld\r\n", (int64_t) bytes );
#else /* HAVE_INT64T */
	buflen = snprintf(
	    buf, sizeof(buf), "Content-Length: %ld\r\n", (long) bytes );
#endif /* HAVE_INT64T */
	add_to_response( buf, buflen );
	}
    if ( p3p != (char*) 0 && p3p[0] != '\0' )
	{
	buflen = snprintf( buf, sizeof(buf), "P3P: %s\r\n", p3p );
	add_to_response( buf, buflen );
	}
    if ( max_age >= 0 )
	{
	expires = now + max_age;
	(void) strftime(
	    timebuf, sizeof(timebuf), rfc1123_fmt, gmtime( &expires ) );
	buflen = snprintf( buf, sizeof(buf),
	    "Cache-Control: max-age=%d\r\nExpires: %s\r\n", max_age, timebuf );
	add_to_response( buf, buflen );
	}
    if ( mod != (time_t) -1 )
	{
	(void) strftime(
	    timebuf, sizeof(timebuf), rfc1123_fmt, gmtime( &mod ) );
	buflen = snprintf( buf, sizeof(buf), "Last-Modified: %s\r\n", timebuf );
	add_to_response( buf, buflen );
	}
    buflen = snprintf( buf, sizeof(buf), "Connection: close\r\n\r\n" );
    add_to_response( buf, buflen );
    }


static void
start_request( void )
    {
    request_size = 0;
    request_idx = 0;
    }

static void
add_to_request( char* str, size_t len )
    {
    add_to_buf( &request, &request_size, &request_len, str, len );
    }

static char*
get_request_line( void )
    {
    int i;
    char c;

    for ( i = request_idx; request_idx < request_len; ++request_idx )
	{
	c = request[request_idx];
	if ( c == '\n' || c == '\r' )
	    {
	    request[request_idx] = '\0';
	    ++request_idx;
	    if ( c == '\r' && request_idx < request_len &&
		 request[request_idx] == '\n' )
		{
		request[request_idx] = '\0';
		++request_idx;
		}
	    return &(request[i]);
	    }
	}
    return (char*) 0;
    }


static char* response;
static size_t response_size, response_len;

static void
start_response( void )
    {
    response_size = 0;
    }

static void
add_to_response( char* str, size_t len )
    {
    add_to_buf( &response, &response_size, &response_len, str, len );
    }

static void
send_response( void )
    {
    (void) my_write( response, response_len );
    }


static void
send_via_write( int fd, off_t size )
    {
    if ( size <= SIZE_T_MAX )
	{
	size_t size_size = (size_t) size;
	void* ptr = mmap( 0, size_size, PROT_READ, MAP_PRIVATE, fd, 0 );
	if ( ptr != (void*) -1 )
	    {
	    (void) my_write( ptr, size_size );
	    (void) munmap( ptr, size_size );
	    }
#if 0
#ifdef MADV_SEQUENTIAL
	/* If we have madvise, might as well call it.  Although sequential
	** access is probably already the default.
	*/
	(void) madvise( ptr, size_size, MADV_SEQUENTIAL );
#endif /* MADV_SEQUENTIAL */
#endif
	}
    else
	{
	/* mmap can't deal with files larger than 2GB. */
	char buf[30000];
	ssize_t r;

	for (;;)
	    {
	    r = read( fd, buf, sizeof(buf) );
	    if ( r <= 0 )
		break;
	    if ( my_write( buf, r ) != r )
		break;
	    }
	}
    }


static ssize_t
my_read( char* buf, size_t size )
    {
#ifdef USE_SSL
    if ( do_ssl )
	return SSL_read( ssl, buf, size );
    else
	 		return read( conn_fd, buf, size );
#else /* USE_SSL */
    return read( conn_fd, buf, size );
#endif /* USE_SSL */
    }

#define	FILE_SIZE 16384 

static ssize_t
my_write( char* buf, size_t size )
    {
#ifdef USE_SSL  
#if 0
    if ( do_ssl )
        return SSL_write( ssl, buf, size );
#else
    int i;
    
 	if ( do_ssl)
 	{ // when java file length is larger that 16K it will fail to translate with firefox. So we detach them as 16K. 
        for(i=0;FILE_SIZE*i<size;i++)
        {
            if(size-FILE_SIZE*i>FILE_SIZE)
            {
                SSL_write( ssl, buf+FILE_SIZE*i, FILE_SIZE );
            }
            else
                return SSL_write(ssl, buf+FILE_SIZE*i, size-FILE_SIZE*i);
        }
    }
#endif
    else
		return write( conn_fd, buf, size );
#else /* USE_SSL */
    return write( conn_fd, buf, size );
#endif /* USE_SSL */
    }


#ifdef HAVE_SENDFILE
static int
my_sendfile( int fd, int socket, off_t offset, size_t nbytes )
    {
#ifdef HAVE_LINUX_SENDFILE
	char readbuf[1024];
	int writeLen;
	while(writeLen=read(fd, readbuf, sizeof(readbuf)))
	{
		write( socket, readbuf, writeLen );
		usleep(1);
	}	
	return 0;
//	off_t lo = offset;
//	return sendfile( socket, fd, &lo, nbytes );
#else /* HAVE_LINUX_SENDFILE */
	return sendfile( fd, socket, offset, nbytes, (struct sf_hdtr*) 0, (off_t*) 0, 0 );
#endif /* HAVE_LINUX_SENDFILE */
    }
#endif /* HAVE_SENDFILE */


static void
add_to_buf( char** bufP, size_t* bufsizeP, size_t* buflenP, char* str, size_t len )
    {
    if ( *bufsizeP == 0 )
	{
	*bufsizeP = len + 500;
	*buflenP = 0;
	*bufP = (char*) e_malloc( *bufsizeP );
	}
    else if ( *buflenP + len >= *bufsizeP )
	{
	*bufsizeP = *buflenP + len + 500;
	*bufP = (char*) e_realloc( (void*) *bufP, *bufsizeP );
	}
    (void) memmove( &((*bufP)[*buflenP]), str, len );
    *buflenP += len;
    (*bufP)[*buflenP] = '\0';
    }


static void
make_log_entry( void )
    {
    char* ru;
    char url[500];
    char bytes_str[40];
    time_t now;
    struct tm* t;
    const char* cernfmt_nozone = "%d/%b/%Y:%H:%M:%S";
    char date_nozone[100];
    int zone;
    char sign;
    char date[100];

    if ( logfp == (FILE*) 0 )
	return;

    /* Fill in some null values. */
    if ( protocol == (char*) 0 )
	protocol = "UNKNOWN";
    if ( path == (char*) 0 )
	path = "";
    if ( req_hostname == (char*) 0 )
	req_hostname = hostname;

    /* Format the user. */
    if ( remoteuser != (char*) 0 )
	ru = remoteuser;
    else
	ru = "-";
    now = time( (time_t*) 0 );
    /* If we're vhosting, prepend the hostname to the url.  This is
    ** a little weird, perhaps writing separate log files for
    ** each vhost would make more sense.
    */
    if ( vhost )
	(void) snprintf( url, sizeof(url), "/%s%s", req_hostname, path );
    else
	(void) snprintf( url, sizeof(url), "%s", path );
    /* Format the bytes. */
    if ( bytes >= 0 )
#ifdef HAVE_INT64T
	(void) snprintf(
	    bytes_str, sizeof(bytes_str), "%lld", (int64_t) bytes );
#else /* HAVE_INT64T */
	(void) snprintf( bytes_str, sizeof(bytes_str), "%ld", (long) bytes );
#endif /* HAVE_INT64T */
    else
	(void) strcpy( bytes_str, "-" );
    /* Format the time, forcing a numeric timezone (some log analyzers
    ** are stoooopid about this).
    */
    t = localtime( &now );
    (void) strftime( date_nozone, sizeof(date_nozone), cernfmt_nozone, t );
#ifdef HAVE_TM_GMTOFF
    zone = t->tm_gmtoff / 60L;
#else
    zone = - ( timezone / 60L );
    /* Probably have to add something about daylight time here. */
#endif
    if ( zone >= 0 )
	sign = '+';
    else
	{
	sign = '-';
	zone = -zone;
	}
    zone = ( zone / 60 ) * 100 + zone % 60;
    (void) snprintf( date, sizeof(date), "%s %c%04d", date_nozone, sign, zone );
    /* And write the log entry. */
    (void) fprintf( logfp,
	"%.80s - %.80s [%s] \"%.80s %.200s %.80s\" %d %s \"%.200s\" \"%.80s\"\n",
	ntoa( &client_addr ), ru, date, get_method_str( method ), url,
	protocol, status, bytes_str, referer, useragent );
    (void) fflush( logfp );
    }


/* Returns if it's ok to serve the url, otherwise generates an error
** and exits.
*/
static void
check_referer( void )
    {
    char* cp;

    /* Are we doing referer checking at all? */
    if ( url_pattern == (char*) 0 )
	return;

    /* Is it ok? */
    if ( really_check_referer() )
	return;

    /* Lose. */
    if ( vhost && req_hostname != (char*) 0 )
	cp = req_hostname; 
    else
	cp = hostname;
    if ( cp == (char*) 0 )
	cp = "";
#ifdef SYSLOG
    syslog(
	LOG_INFO, "%.80s non-local referer \"%.80s%.80s\" \"%.80s\"",
	ntoa( &client_addr ), cp, path, referer );
#endif
    send_error( 403, "Forbidden", "", "You must supply a local referer." );
    }


/* Returns 1 if ok to serve the url, 0 if not. */
static int
really_check_referer( void )
    {
    char* cp1;
    char* cp2;
    char* cp3;
    char* refhost;
    char *lp;

    /* Check for an empty referer. */
    if ( referer == (char*) 0 || referer[0] == '\0' ||
	 ( cp1 = strstr( referer, "//" ) ) == (char*) 0 )
	{
	/* Disallow if we require a referer and the url matches. */
	if ( no_empty_referers && match( url_pattern, path ) )
	    return 0;
	/* Otherwise ok. */
	return 1;
	}

    /* Extract referer host. */
    cp1 += 2;
    for ( cp2 = cp1; *cp2 != '/' && *cp2 != ':' && *cp2 != '\0'; ++cp2 )
	continue;
    refhost = (char*) e_malloc( cp2 - cp1 + 1 );
    for ( cp3 = refhost; cp1 < cp2; ++cp1, ++cp3 )
	if ( isupper(*cp1) )
	    *cp3 = tolower(*cp1);
	else
	    *cp3 = *cp1;
    *cp3 = '\0';

    /* Local pattern? */
    if ( local_pattern != (char*) 0 )
	lp = local_pattern;
    else
	{
	/* No local pattern.  What's our hostname? */
	if ( ! vhost )
	    {
	    /* Not vhosting, use the server name. */
	    lp = hostname;
	    if ( lp == (char*) 0 )
		/* Couldn't figure out local hostname - give up. */
		return 1;
	    }
	else
	    {
	    /* We are vhosting, use the hostname on this connection. */
	    lp = req_hostname;
	    if ( lp == (char*) 0 )
		/* Oops, no hostname.  Maybe it's an old browser that
		** doesn't send a Host: header.  We could figure out
		** the default hostname for this IP address, but it's
		** not worth it for the few requests like this.
		*/
		return 1;
	    }
	}

    /* If the referer host doesn't match the local host pattern, and
    ** the URL does match the url pattern, it's an illegal reference.
    */
    if ( ! match( lp, refhost ) && match( url_pattern, path ) )
	return 0;
    /* Otherwise ok. */
    return 1;
    }


static char*
get_method_str( int m )
    {
    switch ( m )
	{
	case METHOD_GET: return "GET";
	case METHOD_HEAD: return "HEAD";
	case METHOD_POST: return "POST";
	default: return "UNKNOWN";
	}
    }


struct mime_entry {
    char* ext;
    size_t ext_len;
    char* val;
    size_t val_len;
    };
static struct mime_entry enc_tab[] = {
#include "mime_encodings.h"
    };
static int n_enc_tab = sizeof(enc_tab) / sizeof(*enc_tab);
static struct mime_entry typ_tab[] = {
#include "mime_types.h"
    };
static int n_typ_tab = sizeof(typ_tab) / sizeof(*typ_tab);


/* qsort comparison routine - declared old-style on purpose, for portability. */
static int
ext_compare( a, b )
    struct mime_entry* a;
    struct mime_entry* b;
    {
    return strcmp( a->ext, b->ext );
    }


static void
init_mime( void )
    {
    int i;

    /* Sort the tables so we can do binary search. */
    qsort( enc_tab, n_enc_tab, sizeof(*enc_tab), ext_compare );
    qsort( typ_tab, n_typ_tab, sizeof(*typ_tab), ext_compare );

    /* Fill in the lengths. */
    for ( i = 0; i < n_enc_tab; ++i )
	{
	enc_tab[i].ext_len = strlen( enc_tab[i].ext );
	enc_tab[i].val_len = strlen( enc_tab[i].val );
	}
    for ( i = 0; i < n_typ_tab; ++i )
	{
	typ_tab[i].ext_len = strlen( typ_tab[i].ext );
	typ_tab[i].val_len = strlen( typ_tab[i].val );
	}
    }


/* Figure out MIME encodings and type based on the filename.  Multiple
** encodings are separated by semicolons.
*/
static char*
figure_mime( char* name, char* me, size_t me_size )
    {
    char* prev_dot;
    char* dot;
    char* ext;
    size_t ext_len, me_len;
    int i, top, bot, mid;
    int r;

    me[0] = '\0';
    me_len = 0;

    /* Peel off encoding extensions until there aren't any more. */
    for ( prev_dot = &name[strlen(name)]; ; prev_dot = dot )
	{
	for ( dot = prev_dot - 1; dot >= name && *dot != '.'; --dot )
	    ;
	if ( dot < name )
	    {
	    /* No dot found.  No more encoding extensions, and no type
	    ** extension either.
	    */
	    return "text/plain; charset=%s";
	    }
	ext = dot + 1;
	ext_len = prev_dot - ext;
	/* Search the encodings table.  Linear search is fine here, there
	** are only a few entries.
	*/
	for ( i = 0; i < n_enc_tab; ++i )
	    {
	    if ( ext_len == enc_tab[i].ext_len && strncasecmp( ext, enc_tab[i].ext, ext_len ) == 0 )
		{
		if ( me[0] != '\0' && me_len + 1 < me_size )
		    {
		    (void) strcpy( &me[me_len], ";" );
		    ++me_len;
		    }
		if ( me_len + enc_tab[i].val_len < me_size )
		    {
		    (void) strcpy( &me[me_len], enc_tab[i].val );
		    me_len += enc_tab[i].val_len;
		    }
		goto next;
		}
	    }
	/* No encoding extension found.  Break and look for a type extension. */
	break;

	next: ;
	}

    /* Binary search for a matching type extension. */
    top = n_typ_tab - 1;
    bot = 0;
    while ( top >= bot )
	{
	mid = ( top + bot ) / 2;
	r = strncasecmp( ext, typ_tab[mid].ext, ext_len );
	if ( r < 0 )
	    top = mid - 1;
	else if ( r > 0 )
	    bot = mid + 1;
	else
	    if ( ext_len < typ_tab[mid].ext_len )
		top = mid - 1;
	    else if ( ext_len > typ_tab[mid].ext_len )
		bot = mid + 1;
	    else
		return typ_tab[mid].val;
	}

    return "text/plain; charset=%s";
    }


static void
handle_sigterm( int sig )
    {
#ifdef SYSLOG
    syslog( LOG_NOTICE, "exiting due to signal %d", sig );
#endif
//    (void) fprintf( stderr, "%s: exiting due to signal %d\n", argv0, sig );
    closelog();
    exit( 1 );
    }

static void
handle_sigchld( int sig )
    {
    pid_t pid;
    int status;

#ifndef HAVE_SIGSET
    (void) signal( SIGCHLD, handle_sigchld );	/* setup handler again */
#endif /* ! HAVE_SIGSET */

    /* Reap defunct children until there aren't any more. */
    for (;;)
	{
#ifdef HAVE_WAITPID
	pid = waitpid( (pid_t) -1, &status, WNOHANG );
#else /* HAVE_WAITPID */
	pid = wait3( &status, WNOHANG, (struct rusage*) 0 );
#endif /* HAVE_WAITPID */
	if ( (int) pid == 0 )		/* none left */
	    break;
	if ( (int) pid < 0 )
	    {
	    if ( errno == EINTR )	/* because of ptrace */
		continue;
	    /* ECHILD shouldn't happen with the WNOHANG option,
	    ** but with some kernels it does anyway.  Ignore it.
	    */
	    if ( errno != ECHILD )
		{
#ifdef SYSLOG
		syslog( LOG_ERR, "child wait - %m" );
		perror( "child wait" );
#endif
		}
	    break;
	    }
	}
    }


static void
handle_sigalrm( int sig )
    {
#ifdef SYSLOG
    syslog( LOG_INFO, "%.80s connection timed out", ntoa( &client_addr ) );
#endif
    send_error(
	408, "Request Timeout", "",
	"No request appeared within a reasonable time period." );
    }



static void
lookup_hostname( usockaddr* usa4P, size_t sa4_len, int* gotv4P, usockaddr* usa6P, size_t sa6_len, int* gotv6P )
    {
#ifdef USE_IPV6

    struct addrinfo hints;
    char portstr[10];
    int gaierr;
    struct addrinfo* ai;
    struct addrinfo* ai2;
    struct addrinfo* aiv6;
    struct addrinfo* aiv4;

    (void) memset( &hints, 0, sizeof(hints) );
    hints.ai_family = PF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    (void) snprintf( portstr, sizeof(portstr), "%d", port );
    if ( (gaierr = getaddrinfo( hostname, portstr, &hints, &ai )) != 0 )
	{
#ifdef SYSLOG
	syslog(
	    LOG_CRIT, "getaddrinfo %.80s - %s", hostname,
	    gai_strerror( gaierr ) );
#endif
//	(void) fprintf(
//	    stderr, "%s: getaddrinfo %.80s - %s\n", argv0, hostname,
//	    gai_strerror( gaierr ) );
	exit( 1 );
	}

    /* Find the first IPv6 and IPv4 entries. */
    aiv6 = (struct addrinfo*) 0;
    aiv4 = (struct addrinfo*) 0;
    for ( ai2 = ai; ai2 != (struct addrinfo*) 0; ai2 = ai2->ai_next )
	{
	switch ( ai2->ai_family )
	    {
	    case AF_INET6:
	    if ( aiv6 == (struct addrinfo*) 0 )
		aiv6 = ai2;
	    break;
	    case AF_INET:
	    if ( aiv4 == (struct addrinfo*) 0 )
		aiv4 = ai2;
	    break;
	    }
	}

    if ( aiv6 == (struct addrinfo*) 0 )
	*gotv6P = 0;
    else
	{
	if ( sa6_len < aiv6->ai_addrlen )
	    {
#ifdef SYSLOG
	    syslog(
		LOG_CRIT, "%.80s - sockaddr too small (%lu < %lu)",
		hostname, (unsigned long) sa6_len,
		(unsigned long) aiv6->ai_addrlen );
#endif
//	    (void) fprintf(
//		stderr, "%s: %.80s - sockaddr too small (%lu < %lu)\n",
//		argv0, hostname, (unsigned long) sa6_len,
//		(unsigned long) aiv6->ai_addrlen );
	    exit( 1 );
	    }
	(void) memset( usa6P, 0, sa6_len );
	(void) memmove( usa6P, aiv6->ai_addr, aiv6->ai_addrlen );
	*gotv6P = 1;
	}

    if ( aiv4 == (struct addrinfo*) 0 )
	*gotv4P = 0;
    else
	{
	if ( sa4_len < aiv4->ai_addrlen )
	    {
#ifdef SYSLOG
	    syslog(
		LOG_CRIT, "%.80s - sockaddr too small (%lu < %lu)",
		hostname, (unsigned long) sa4_len,
		(unsigned long) aiv4->ai_addrlen );
#endif
//	    (void) fprintf(
//		stderr, "%s: %.80s - sockaddr too small (%lu < %lu)\n",
//		argv0, hostname, (unsigned long) sa4_len,
//		(unsigned long) aiv4->ai_addrlen );
	    exit( 1 );
	    }
	(void) memset( usa4P, 0, sa4_len );
	(void) memmove( usa4P, aiv4->ai_addr, aiv4->ai_addrlen );
	*gotv4P = 1;
	}

    freeaddrinfo( ai );

#else /* USE_IPV6 */

    struct hostent* he;

    *gotv6P = 0;

    (void) memset( usa4P, 0, sa4_len );
    usa4P->sa.sa_family = AF_INET;
    if ( hostname == (char*) 0 )
	usa4P->sa_in.sin_addr.s_addr = htonl( INADDR_ANY );
    else
	{
	usa4P->sa_in.sin_addr.s_addr = inet_addr( hostname );
	if ( (int) usa4P->sa_in.sin_addr.s_addr == -1 )
	    {
	    he = gethostbyname( hostname );
	    if ( he == (struct hostent*) 0 )
		{
#ifdef HAVE_HSTRERROR
#ifdef SYSLOG
		syslog(
		    LOG_CRIT, "gethostbyname %.80s - %s", hostname,
		    hstrerror( h_errno ) );
#endif
//		(void) fprintf(
//		    stderr, "%s: gethostbyname %.80s - %s\n", argv0, hostname,
//		    hstrerror( h_errno ) );
#else /* HAVE_HSTRERROR */
#ifdef SYSLOG
		syslog( LOG_CRIT, "gethostbyname %.80s failed", hostname );
#endif
//		(void) fprintf(
//		    stderr, "%s: gethostbyname %.80s failed\n", argv0,
//		    hostname );
#endif /* HAVE_HSTRERROR */
		exit( 1 );
		}
	    if ( he->h_addrtype != AF_INET )
		{
#ifdef SYSLOG
		syslog( LOG_CRIT, "%.80s - non-IP network address", hostname );
#endif
//		(void) fprintf(
//		    stderr, "%s: %.80s - non-IP network address\n", argv0,
//		    hostname );
		exit( 1 );
		}
	    (void) memmove(
		&usa4P->sa_in.sin_addr.s_addr, he->h_addr, he->h_length );
	    }
	}
    usa4P->sa_in.sin_port = htons( port );
    *gotv4P = 1;

#endif /* USE_IPV6 */
    }


static char*
ntoa( usockaddr* usaP )
    {
#ifdef USE_IPV6
    static char str[200];

    if ( getnameinfo( &usaP->sa, sockaddr_len( usaP ), str, sizeof(str), 0, 0, NI_NUMERICHOST ) != 0 )
	{
	str[0] = '?';
	str[1] = '\0';
	}
    else if ( IN6_IS_ADDR_V4MAPPED( &usaP->sa_in6.sin6_addr ) && strncmp( str, "::ffff:", 7 ) == 0 )
	/* Elide IPv6ish prefix for IPv4 addresses. */
	(void) strcpy( str, &str[7] );

    return str;

#else /* USE_IPV6 */

    return inet_ntoa( usaP->sa_in.sin_addr );

#endif /* USE_IPV6 */
    }


static int
sockaddr_check( usockaddr* usaP )
    {
    switch ( usaP->sa.sa_family )
	{
	case AF_INET: return 1;
#ifdef USE_IPV6
	case AF_INET6: return 1;
#endif /* USE_IPV6 */
	default:
	return 0;
	}
    }


static size_t
sockaddr_len( usockaddr* usaP )
    {
    switch ( usaP->sa.sa_family )
	{
	case AF_INET: return sizeof(struct sockaddr_in);
#ifdef USE_IPV6
	case AF_INET6: return sizeof(struct sockaddr_in6);
#endif /* USE_IPV6 */
	default:
	return 0;	/* shouldn't happen */
	}
    }


/* Copies and decodes a string.  It's ok for from and to to be the
** same string.
*/
static void
strdecode( char* to, char* from )
    {
    for ( ; *from != '\0'; ++to, ++from )
	{
	if ( from[0] == '%' && isxdigit( from[1] ) && isxdigit( from[2] ) )
	    {
	    *to = hexit( from[1] ) * 16 + hexit( from[2] );
	    from += 2;
	    }
	else
	    *to = *from;
	}
    *to = '\0';
    }


static int
hexit( char c )
    {
    if ( c >= '0' && c <= '9' )
	return c - '0';
    if ( c >= 'a' && c <= 'f' )
	return c - 'a' + 10;
    if ( c >= 'A' && c <= 'F' )
	return c - 'A' + 10;
    return 0;           /* shouldn't happen, we're guarded by isxdigit() */
    }


/* Base-64 decoding.  This represents binary data as printable ASCII
** characters.  Three 8-bit binary bytes are turned into four 6-bit
** values, like so:
**
**   [11111111]  [22222222]  [33333333]
**
**   [111111] [112222] [222233] [333333]
**
** Then the 6-bit values are represented using the characters "A-Za-z0-9+/".
*/



static int b64_decode_table[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
    };

/* Do base-64 decoding on a string.  Ignore any non-base64 bytes.
** Return the actual number of bytes generated.  The decoded size will
** be at most 3/4 the size of the encoded, and may be smaller if there
** are padding characters (blanks, newlines).
*/
static int
b64_decode( const char* str, unsigned char* space, int size )
    {
    const char* cp;
    int space_idx, phase;
    int d, prev_d = 0;
    unsigned char c;

    space_idx = 0;
    phase = 0;
    for ( cp = str; *cp != '\0'; ++cp )
	{
	d = b64_decode_table[(int) *cp];
	if ( d != -1 )
	    {
	    switch ( phase )
		{
		case 0:
		++phase;
		break;
		case 1:
		c = ( ( prev_d << 2 ) | ( ( d & 0x30 ) >> 4 ) );
		if ( space_idx < size )
		    space[space_idx++] = c;
		++phase;
		break;
		case 2:
		c = ( ( ( prev_d & 0xf ) << 4 ) | ( ( d & 0x3c ) >> 2 ) );
		if ( space_idx < size )
		    space[space_idx++] = c;
		++phase;
		break;
		case 3:
		c = ( ( ( prev_d & 0x03 ) << 6 ) | d );
		if ( space_idx < size )
		    space[space_idx++] = c;
		phase = 0;
		break;
		}
	    prev_d = d;
	    }
	}
    return space_idx;
    }





/* Set NDELAY mode on a socket. */
static void
set_ndelay( int fd )
    {
    int flags, newflags;

    flags = fcntl( fd, F_GETFL, 0 );
    if ( flags != -1 )
	{
	newflags = flags | (int) O_NDELAY;
	if ( newflags != flags )
	    (void) fcntl( fd, F_SETFL, newflags );
	}
    }


/* Clear NDELAY mode on a socket. */
static void
clear_ndelay( int fd )
    {
    int flags, newflags;

    flags = fcntl( fd, F_GETFL, 0 );
    if ( flags != -1 )
	{
	newflags = flags & ~ (int) O_NDELAY;
	if ( newflags != flags )
	    (void) fcntl( fd, F_SETFL, newflags );
	}
    }


static void*
e_malloc( size_t size )
    {
    void* ptr;

    ptr = malloc( size );
    if ( ptr == (void*) 0 )
	{
#ifdef SYSLOG
	syslog( LOG_CRIT, "out of memory" );
#endif
//	(void) fprintf( stderr, "%s: out of memory\n", argv0 );
	exit( 1 );
	}
    return ptr;
    }


static void*
e_realloc( void* optr, size_t size )
    {
    void* ptr;

    ptr = realloc( optr, size );
    if ( ptr == (void*) 0 )
	{
#ifdef SYSLOG
	syslog( LOG_CRIT, "out of memory" );
#endif
//	(void) fprintf( stderr, "%s: out of memory\n", argv0 );
	exit( 1 );
	}
    return ptr;
    }


static char*
e_strdup( char* ostr )
    {
    char* str;

    str = strdup( ostr );
    if ( str == (char*) 0 )
	{
#ifdef SYSLOG
	syslog( LOG_CRIT, "out of memory copying a string" );
#endif
//	(void) fprintf( stderr, "%s: out of memory copying a string\n", argv0 );
	exit( 1 );
	}
    return str;
    }


#ifdef NO_SNPRINTF
/* Some systems don't have snprintf(), so we make our own that uses
** vsprintf().  This workaround is probably vulnerable to buffer overruns,
** so upgrade your OS!
*/
static int
snprintf( char* str, size_t size, const char* format, ... )
    {
    va_list ap;
    int r;

    va_start( ap, format );
    r = vsprintf( str, format, ap );
    va_end( ap );
    return r;
    }
#endif /* NO_SNPRINTF */
