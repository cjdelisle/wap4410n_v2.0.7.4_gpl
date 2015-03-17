/* vi: set sw=4 ts=4: */
/*
 * Mini syslogd implementation for busybox
 *
 * Copyright (C) 1999,2000 by Lineo, inc. and Erik Andersen
 * Copyright (C) 1999,2000,2001 by Erik Andersen <andersee@debian.org>
 *
 * Copyright (C) 2000 by Karl M. Hegbloom <karlheg@debian.org>
 *
 * "circular buffer" Copyright (C) 2001 by Gennady Feldman <gfeldman@cachier.com>
 *
 * Maintainer: Gennady Feldman <gena01@cachier.com> as of Mar 12, 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <paths.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include "sysklogd.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "nvram.h"
/* SYSLOG_NAMES defined to pull some extra junk from syslog.h */
#define SYSLOG_NAMES
#include <sys/syslog.h>
#include <sys/uio.h>
#include <net/if.h>
#include <sys/ioctl.h>

/* Path for the file where all log messages are written */
#define __LOG_FILE "/var/log/systemLog"
#define __CONF_FILE "/etc/syslog.conf"
#define SHOW_HOSTNAME

/* Path to the unix socket */
static char lfile[MAXPATHLEN]="";

static char *logFilePath = __LOG_FILE;

#define dprintf(msg,...)
struct syslog_conf conf;

#define ALERT_MAX_INTERVAL 3*60
static time_t last_send_mail=0;

/* interval between marks in seconds */
static int MarkInterval = 10 * 60;

#ifdef SHOW_HOSTNAME
/* localhost's name */
static char LocalHostName[256]="";
#endif
#ifdef _WIRELESS_
#define LOGIC_LAN_IFNAME  "br0"
#else
#define LOGIC_LAN_IFNAME  "eth0"
#endif

#ifdef BB_FEATURE_REMOTE_LOG
#include <netinet/in.h>
/* udp socket for logging to remote host */
static int remotefd = -1;
/* where do we log? */
static char *RemoteHost=NULL;
/* what port to log to? */
static int RemotePort = 514;
/* To remote log or not to remote log, that is the question. */
static int doRemoteLog = FALSE;
static int local_logging = FALSE;
#endif

static int email_send_threshold = 0;
#define MAXLINE         1024            /* maximum line length */


/* circular buffer variables/structures */
#define BB_FEATURE_IPC_SYSLOG
#ifdef BB_FEATURE_IPC_SYSLOG
#if __GNU_LIBRARY__ < 5
#error Sorry.  Looks like you are using libc5.
#error libc5 shm support isnt good enough.
#error Please disable BB_FEATURE_IPC_SYSLOG
#endif

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

/* our shared key */
static const long KEY_ID = 0x414e4547; /*"GENA"*/

// Semaphore operation structures
static struct shbuf_ds {
    int num;                // number of message
    int size;               // size of data written
    int head;               // start of message list
    int tail;               // end of message list
    /* can't use char *data */
    char data[1];           // data/messages
} *buf = NULL;                  // shared memory pointer

static struct sembuf SMwup[1] = {{1, -1, IPC_NOWAIT}}; // set SMwup
static struct sembuf SMwdn[3] = {{0, 0}, {1, 0}, {1, +1}}; // set SMwdn

static int      shmid = -1;     // ipc shared memory id
static int      s_semid = -1;   // ipc semaphore id
int     data_size = 50000; // data size
int     shm_size = 50000 + sizeof(*buf); // our buffer size
static int circular_logging = TRUE;

/* Ron */
static void clear_signal(int sig);
static void reload_signal(int sig);
static char last_log[1024]="";

//#define DEBUG_SYSLOG
#ifdef DEBUG_SYSLOG
int mylog(const char *format, ...)
{
    va_list args;
    FILE *fp;

    fp = fopen("/tmp/log_view", "a+");

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
#endif

void logMessage (int pri, char *msg);
/*
 * sem_up - up()'s a semaphore.
 */
static inline void sem_up(int semid)
{
    if ( semop(semid, SMwup, 1) == -1 )
        perror_msg_and_die("semop[SMwup]");
}

/*
 * sem_down - down()'s a semaphore
 */
static inline void sem_down(int semid)
{
    if ( semop(semid, SMwdn, 3) == -1 )
        perror_msg_and_die("semop[SMwdn]");
}


void ipcsyslog_cleanup(void){
    if (shmid != -1)
        shmdt(buf);

    if (shmid != -1)
        shmctl(shmid, IPC_RMID, NULL);
    if (s_semid != -1)
        semctl(s_semid, 0, IPC_RMID, 0);
}

void ipcsyslog_init(void){
    if (buf == NULL){
        if ((shmid = shmget(KEY_ID, shm_size, IPC_CREAT | 1023)) == -1)
            perror_msg_and_die("shmget");


        if ((buf = shmat(shmid, NULL, 0)) == (void *)-1)
            perror_msg_and_die("shmat");


        buf->size=data_size;
        buf->num=buf->head=buf->tail=0;

        // we'll trust the OS to set initial semval to 0 (let's hope)
        if ((s_semid = semget(KEY_ID, 2, IPC_CREAT | IPC_EXCL | 1023)) == -1){
            if (errno == EEXIST){
                if ((s_semid = semget(KEY_ID, 2, 0)) == -1)
                    perror_msg_and_die("semget");
            }else
                perror_msg_and_die("semget");
        }
    }else{
        dprintf("Buffer already allocated just grab the semaphore?");
    }
}

static void send_mail_signal(int sig)
{
    if(conf.mail_enable==1){
        char cmd[1024];

        sprintf(cmd,"/usr/sbin/smtpc -m -h %s -r %s -f %s -s \"%s\" <%s &"
            ,conf.mail_server
            ,conf.mail_receiver
            ,conf.mail_sender
            ,conf.mail_subject
            ,"/var/log/systemLog");
          
#ifdef DEBUG_SYSLOG
        mylog("<%s,%d>:%s\n", __FUNCTION__, __LINE__,cmd);
#endif
        system(cmd);
        buf->head=0;
        buf->tail=0;
        buf->num=0;
        unlink("/var/log/systemLog");
        time(&last_send_mail);
    }
}

/* write message to buffer */
void circ_message(const char *msg){
    int l=strlen(msg); /* count the whole message w/ '\0' included */

    sem_down(s_semid);

    buf->num++;
    if ( (buf->tail + l) < buf->size ){
        if ( buf->tail < buf->head){
            if ( (buf->tail + l) >= buf->head ){
                int k= buf->tail + l - buf->head;
                char *c=memchr(buf->data+buf->head + k,'\n',buf->size - (buf->head + k));
                buf->head=(c != NULL)?( c - buf->data + 1):0;

            }
        }
        strncpy(buf->data + buf->tail,msg,l); /* append our message */
        buf->tail+=l;
    }else{
        char *c;
        int k=buf->tail + l - buf->size;

        c=memchr(buf->data + k ,'\n', buf->size - k);

        if (c != NULL) {
            buf->head=c-buf->data+1;
            strncpy(buf->data + buf->tail, msg, l - k - 1);
            strcpy(buf->data, &msg[l-k-1]);
            buf->tail = k + 1;
        }else{
            buf->head = buf->tail = 0;
        }

    }
    sem_up(s_semid);
}
#endif  /* BB_FEATURE_IPC_SYSLOG */

/* try to open up the specified device */
int device_open(char *device, int mode)
{
    int m, f, fd = -1;

    m = mode | O_NONBLOCK;

    /* Retry up to 5 times */
    for (f = 0; f < 5; f++)
        if ((fd = open(device, m, 0600)) >= 0)
            break;
    if (fd < 0)
        return fd;
    /* Reset original flags. */
    if (m != mode)
        fcntl(fd, F_SETFL, mode);
    return fd;
}

int vdprintf(int d, const char *format, va_list ap)
{
    char buf[BUF_SIZE];
    int len;

    len = vsnprintf(buf, sizeof(buf), format, ap);
    return write(d, buf, len);
}

enum {
    NEEDNOT_DST,    /*need not daylight saving time*/
    NA_DST,         /*North America need DST*/
    EU_DST,         /*Europe DST*/
    CHILE_DST,		/*Chile dst*/
    SA_DST,			/* sourth america,for example: Brazil*/
    IRAQ_DST,		/* Iraq and Iran */
    AU2_DST,		/*Australia - Tasmania*/
    AU3_DST,		/*New Zealand, Chatham */
    AF_DST,		/* Egypt*/
    AU_DST,          /*Australia DST*/
};

/* Note: There is also a function called "message()" in init.c */
/* Print a message to the log file. */
static void message (char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
static void message (char *fmt, ...)
{
    int fd;
    struct flock fl;
    va_list arguments;

    fl.l_whence = SEEK_SET;
    fl.l_start  = 0;
    fl.l_len    = 1;
#ifdef BB_FEATURE_IPC_SYSLOG
    if ((circular_logging == TRUE) && (buf != NULL)){
        char b[1024];
        va_start (arguments, fmt);
        vsnprintf (b, sizeof(b)-1, fmt, arguments);
        va_end (arguments);
        circ_message(b);
#ifdef DEBUG
        printf("head=%d tail=%d\n",buf->head,buf->tail);
#endif
        /* print_circ_buf */
        if((fd=open(logFilePath,O_WRONLY| O_CREAT|O_TRUNC|O_NONBLOCK))<0)
            return;
        fl.l_type = F_WRLCK;
        fcntl(fd, F_SETLKW, &fl);
        if(buf->tail > buf->head){
            write(fd,buf->data,buf->tail);
            write(fd,"\0",1);
        }else {

            write(fd,buf->data+buf->head,buf->size-buf->head-1);
            write(fd,buf->data,buf->tail);
            write(fd,"\0",1);

            if(conf.mail_log_full==1)
                send_mail_signal(0);

        }
        if(buf->num >= conf.email_qlen) {
            send_mail_signal(0);
        }
        fl.l_type = F_UNLCK;
        fcntl(fd, F_SETLKW, &fl);
        close(fd);
    }else
#endif
        if ((fd = device_open (logFilePath,
                        O_WRONLY | O_CREAT | O_NOCTTY | O_APPEND |
                        O_NONBLOCK)) >= 0) {
            fl.l_type = F_WRLCK;
            fcntl (fd, F_SETLKW, &fl);
            va_start (arguments, fmt);
            vdprintf (fd, fmt, arguments);
            va_end (arguments);
            fl.l_type = F_UNLCK;
            fcntl (fd, F_SETLKW, &fl);
            close (fd);
        } else {
            /* Always send console messages to /dev/console so people will see them. */
            if ((fd = device_open (_PATH_CONSOLE,
                            O_WRONLY | O_NOCTTY | O_NONBLOCK)) >= 0) {
                va_start (arguments, fmt);
                vdprintf (fd, fmt, arguments);
                va_end (arguments);
                close (fd);
            } else {
                fprintf (stderr, "Bummer, can't print: ");
                va_start (arguments, fmt);
                vfprintf (stderr, fmt, arguments);
                fflush (stderr);
                va_end (arguments);
            }
        }
}

inline int check_level(int pri) {
    return conf.log_level[pri&0x07];
}

void strccpy2(char *dst, char *src,char *key,char c)
{
    char *pt=strstr(src,key);
    if(pt==NULL){
        dst[0]='\0';
        return ;
    }
    pt+=strlen(key);
    for(;*pt!=c && *pt!='\0';*dst++=*pt++);
    *dst='\0';

}
/*
 * check if msg is only white space
 */
#define is_whitespace(c)    (((c)==' ') || ((c) == '\t') || ((c) == '\n') )// 

static int white_msg(char *msg) {
    char *p = msg;

    for(; *p; p++) {
        if(!is_whitespace(*p)) {
            return 0;
        }
    }
    return 1;
}

void logMessage (int pri, char *msg)
{
    time_t now;
    struct tm *st;
    char timestamp[32];
    char res[20];
    char msgType[128];
    int send_mail=email_send_threshold;
    int j;
    char *p,*q;

#ifdef DEBUG_SYSLOG
    mylog("<%s,%d>:%s\n", __FUNCTION__, __LINE__,msg);
#endif
#ifdef DEBUG_SYSLOG
    mylog("<%s,%d>:%d%d%d%d%d%d%d%d\n", __FUNCTION__, __LINE__,conf.log_level[7],conf.log_level[6],conf.log_level[5],conf.log_level[4],conf.log_level[3],conf.log_level[2],conf.log_level[1],conf.log_level[0]);
#endif
    /*
    if(!check_level(pri)) {
#ifdef DEBUG_SYSLOG
    mylog("<%s,%d>pri is too low.\n", __FUNCTION__, __LINE__);
#endif        
        return;
    }
    */
    /*
     * messge format:Jan  1 00:00:59 kernel: [SC][deAuth][ssid][00:80:C8:17:14:98]-DEAUTH
     */

    /*remove it time stamp*/
    msg += (sizeof("Jan  1 00:00:59 ") - 1);
    
    char *msgP=strstr(msg, ": ");
    if(msgP){        
        memset(msgType,0,sizeof(msgType));
        for(q=msg,j=0;(j<128)&&(q<=msgP+1);q++)
            msgType[j++] = *q;
        msg = msgP+1;   
    }
   
    while(*msg!='\0' && *msg++!=' '); /* remove 'auth: ' */

    if(white_msg(msg)) {
        return;
    }

    /*
     * Msg format from now on:kernel: [SC][deAuth][ssid][00:80:C8:17:14:98]-DEAUTH
     */
    {
        char *pt,*Msgg;
        char tmp[20],TMP[2048];
        int i;

        strncpy(tmp,msg,4);
        tmp[4]=0;
        if(strncmp(tmp,"[SC]",4)==0){
            /*remove msg head '[SC]'*/
            msg += (sizeof("[SC]") - 1);  
        }else return;
        
        Msgg=msg;
        memset(tmp,0,20);
        char *left = msg;
        char *right = strchr(msg,']');
        if(left && right){
            pt=msg;
            for(i=0;(pt<=right)&&(i<=19);pt++)
                tmp[i++] = *pt;  
       msg=Msgg;
#ifdef DEBUG_SYSLOG
        mylog("<%s,%d>:%x\n", __FUNCTION__, __LINE__,conf.log_send_type);
#endif

        /* if all of send_log type is disable , return, but clear locallog so can't return */
//        if(!(conf.log_send_type & 0x3f)){ 
//            return;      
//        }
       if(((strcmp(tmp,"[CFG_Change]")==0) && (conf.log_send_type & 0x08)) ||
        ((strcmp(tmp,"[systme_error]")==0)/* && (conf.log_send_type & 0x04)*/) ||
        ((strcmp(tmp,"[AuthLogin]")==0) && (conf.log_send_type & 0x02)) ||
        ((strcmp(tmp,"[WE_SUCCESS]")==0) && (conf.log_send_type & 0x10)) ||
        ((strcmp(tmp,"[WE_FAILURE]")==0) /*&& (conf.log_send_type & 0x20)*/) ||
        ((strcmp(tmp,"[UnAuthLogin]")==0) && (conf.log_send_type & 0x01)) ||
        (strcmp(tmp,"[ROGUEAP]")==0)){                
            msg += strlen(tmp);
            sprintf(TMP,"%s%s",msgType,msg);
            sprintf(msg,"%s",TMP);
            memset(TMP,0,sizeof(TMP));
#ifdef DEBUG_SYSLOG
            mylog("<%s,%d>:%s\n", __FUNCTION__, __LINE__,msg);
#endif 
            goto add_timestamp; 
        }else if(strcmp(tmp,"[CLEAR]")==0){
            buf->head=0;
            buf->tail=0;
            buf->num=0;
            unlink("/var/log/systemLog");
            msg += (sizeof("[CLEAR]")-1);
#ifdef DEBUG_SYSLOG
            mylog("<%s,%d>:%s\n", __FUNCTION__, __LINE__,msg);
#endif            
            goto add_timestamp;
        }else return;
        }else return;      
 
    }

add_timestamp:
    /* time stamp */
    time(&now);
#ifdef DEBUG_SYSLOG
    mylog("<%s,%d>time(now):%d\n", __FUNCTION__, __LINE__,now);
#endif
    st=localtime(&now);
#ifdef DEBUG_SYSLOG
    mylog("<%s,%d>time(now):%d\n", __FUNCTION__, __LINE__,now);
#endif
    memset(timestamp, 0, sizeof(timestamp));
    asctime_r(st, timestamp); /* "Wed Jun 30 21:33:44 1987\n" */
    memmove(timestamp, timestamp+4, strlen(timestamp)-3); /* "Jun 30 21:33:44 1987\n" */
    p = strrchr(timestamp, ' ');
    if(p) {
        p++;
        *p = '\0';
    }/* "Jun 30 21:33:44 " */
    if(conf.mail_enable==1){
        char cmd[1024];
        FILE *fp;
        if(email_send_threshold<=atoi(conf.dos_thresholds)){//record msg,the max number of the msg is dos_thresholds.
            if(email_send_threshold>send_mail){//if msg match mail_keyword,no match no record.
                fp=fopen("/var/log/alert","a+");

                if(fp==NULL)
                    return ;
#ifdef DEBUG_SYSLOG
                mylog("<%s,%d>:I don't want it to reach here\n", __FUNCTION__, __LINE__);
#endif
                fprintf(fp,"No.%03d  %s - %s\n",email_send_threshold, timestamp, msg);
                fclose(fp);
            }
        }

        if(email_send_threshold>=atoi(conf.dos_thresholds)){
            if((now-last_send_mail)>ALERT_MAX_INTERVAL){                
#ifdef DEBUG_SYSLOG
                mylog("<%s,%d>:I don't want it to reach here also\n", __FUNCTION__, __LINE__);
#endif
                sprintf(cmd,"/usr/sbin/smtpc -m -h %s -r %s -f %s -s \"%s\" </var/log/alert "
                        ,conf.mail_server
                        ,conf.mail_receiver
                        ,conf.mail_sender
                        ,conf.mail_subject_alert);

                system(cmd);
                time(&last_send_mail);
                email_send_threshold=0;
                send_mail=0;
                unlink("/var/log/alert");
            }
        }
    }
    else
        email_send_threshold=0;
    /* todo: supress duplicates */
#ifdef BB_FEATURE_REMOTE_LOG
if(check_level(pri)) 
{
    sprintf(res, "<%d>", pri);
    /* send message to remote logger */
    if ( (-1 != remotefd) && (doRemoteLog==TRUE)){
        static const int IOV_COUNT = 5;
        struct iovec iov[IOV_COUNT];
        struct iovec *v = iov;
        char *space = " ";

        v->iov_base = res ;
        v->iov_len = strlen(res);
        v++;

        v->iov_base = timestamp ;
        v->iov_len = strlen(timestamp);
        v++;

        v->iov_base = LocalHostName ;
        v->iov_len = strlen(LocalHostName);
        v++;

        v->iov_base = space ;
        v->iov_len = strlen(space);
        v++;

        v->iov_base = msg;
        v->iov_len = strlen(msg);
writev_retry:
        if ( -1 == writev(remotefd,iov, IOV_COUNT)){
            if (errno == EINTR) goto writev_retry;
            error_msg_and_die("cannot write to remote file handle on"
                    "%s:%d",RemoteHost,RemotePort);
        }
    }
}
#endif

    if(conf.log_enable ==0) {
        /* Local log disabled */
        return ;
    }

#ifdef DEBUG_SYSLOG
    mylog("<%s,%d>to_localLog:%s\n", __FUNCTION__, __LINE__,msg);
#endif 
    message("%s%s\n", timestamp, msg);
}

static void quit_signal(int sig)
{
    unlink(lfile);
#ifdef BB_FEATURE_IPC_SYSLOG
    ipcsyslog_cleanup();
#endif
    exit(TRUE);
}
static int log_start=0;

static inline void router_start()
{
    time_t now;
    struct tm *st;
    char timestamp[128];
    char *p;

    if(log_start==1)
        return ;

    time(&now);
    st=localtime(&now);

    memset(timestamp, 0, sizeof(timestamp));
    asctime_r(st, timestamp); /* "Wed Jun 30 21:33:44 1987\n" */

    memmove(timestamp, timestamp+4, strlen(timestamp)-3); /* "Jun 30 21:33:44 1987\n" */
    p = strrchr(timestamp, ' ');
    if(p) {
        p++;
        *p = '\0';
    }

    if(st)
        free(st);
    log_start=1;
    message("%s%s\n", timestamp, "Syslogd start up");
}

static void domark(int sig)
{
    if(log_start==0){
        router_start();
    }
    last_log[0]='\0';
    alarm(MarkInterval);
}

/* This must be a #define, since when DODEBUG and BUFFERS_GO_IN_BSS are
 * enabled, we otherwise get a "storage size isn't constant error. */
static int serveConnection (char* tmpbuf, int n_read)
{
    char *p = tmpbuf;

    while (p < tmpbuf + n_read) {

        int           pri = (LOG_USER | LOG_NOTICE);
        char          line[ MAXLINE + 1 ];
        unsigned char c;
        int find=0;

        char *q = line;
  
        while ( (c = *p) && q < &line[ sizeof (line) - 1 ]) {
            if (c == '<' && find==0) {
                /* Parse the magic priority number. */
                pri = 0;
                find= 1;
                while (isdigit (*(++p))) {
                    pri = 10 * pri + (*p - '0');
                }
                if (pri & ~(LOG_FACMASK | LOG_PRIMASK)){
                    pri = (LOG_USER | LOG_NOTICE);
                }
            } else if (c == '\n') {
                *q++ = ' ';
            } else if (iscntrl (c) && (c < 0177)) {
                *q++ = '^';
                *q++ = c ^ 0100;
            } else {
                *q++ = c;
            }
            p++;
        }
        *q = '\0';
        p++;
        /* Now log it */
#ifdef DEBUG_SYSLOG
        mylog("<%s,%d>:%d\n", __FUNCTION__, __LINE__,pri);
#endif        
        logMessage (pri, line);
    }
    return n_read;
}

#ifdef BB_FEATURE_REMOTE_LOG
static void init_RemoteLog (void)
{

    struct sockaddr_in remoteaddr;
    struct hostent *hostinfo;
    int len = sizeof(remoteaddr);
    int so_bc=1;

    memset(&remoteaddr, 0, len);

    remotefd = socket(AF_INET, SOCK_DGRAM, 0);

    if (remotefd < 0) {
        error_msg_and_die("cannot create socket");
    }

    remoteaddr.sin_family = AF_INET;

    /* Ron */
    /* allow boardcast */
    setsockopt(remotefd,SOL_SOCKET,SO_BROADCAST,&so_bc,sizeof(so_bc));
    hostinfo = gethostbyname(RemoteHost);
    remoteaddr.sin_addr = *(struct in_addr *) *hostinfo->h_addr_list;
    remoteaddr.sin_port = htons(RemotePort);

    /*
       Since we are using UDP sockets, connect just sets the default host and port
       for future operations
     */
    if ( 0 != (connect(remotefd, (struct sockaddr *) &remoteaddr, len))){
        error_msg_and_die("cannot connect to remote host %s:%d", RemoteHost, RemotePort);
    }
}
#endif

static void doSyslogd (void) __attribute__ ((noreturn));
static void doSyslogd (void)
{
    struct sockaddr_un sunx;
    socklen_t addrLength;
    struct timeval tv;
    int sock_fd;
    fd_set fds;
    time_t now;

    /* Set up signal handlers. */
    signal (SIGINT,  quit_signal);
    signal (SIGTERM, quit_signal);
    signal (SIGQUIT, quit_signal);
    signal (SIGHUP,  send_mail_signal);
    signal (SIGUSR1, clear_signal);
    signal (SIGUSR2, reload_signal);
    signal (SIGCHLD,  SIG_IGN);
#ifdef SIGCLD
    signal (SIGCLD,  SIG_IGN);
#endif
    signal (SIGALRM, domark);
    //wait ntp get correct time
    alarm (MarkInterval);

    /* Create the syslog file so realpath() can work. */
    if (realpath (_PATH_LOG, lfile) != NULL)
        unlink (lfile);

    memset (&sunx, 0, sizeof (sunx));
    sunx.sun_family = AF_UNIX;
    strncpy (sunx.sun_path, lfile, sizeof (sunx.sun_path));
    if ((sock_fd = socket (AF_UNIX, SOCK_DGRAM, 0)) < 0)
        perror_msg_and_die ("Couldn't get file descriptor for socket " _PATH_LOG);

    addrLength = sizeof (sunx.sun_family) + strlen (sunx.sun_path);
    if (bind(sock_fd, (struct sockaddr *) &sunx, addrLength) < 0)
        perror_msg_and_die ("Could not connect to socket " _PATH_LOG);

    if (chmod (lfile, 0666) < 0)
        perror_msg_and_die ("Could not set permission on " _PATH_LOG);


#ifdef BB_FEATURE_IPC_SYSLOG
    if (circular_logging == TRUE ){
        ipcsyslog_init();
    }
#endif

#ifdef BB_FEATURE_REMOTE_LOG
    if (doRemoteLog == TRUE){
        init_RemoteLog();
    }
#endif

    for (;;) {

        FD_ZERO (&fds);
        FD_SET (sock_fd, &fds);

        tv.tv_sec = 60;
        tv.tv_usec = 0;

        if (select (sock_fd+1, &fds, NULL, NULL, &tv) < 0) {
            if (errno == EINTR) {
                /* alarm may have happened. */
                continue;
            }
            perror_msg_and_die ("select error");
        }
        time(&now);
        if (FD_ISSET (sock_fd, &fds)) {
            int   i;
            RESERVE_BB_BUFFER(tmpbuf, BUFSIZ + 1);

            memset(tmpbuf, '\0', BUFSIZ+1);
            if ( (i = recv(sock_fd, tmpbuf, BUFSIZ, 0)) > 0) {
                serveConnection(tmpbuf, i);
            } else {
                perror_msg_and_die ("UNIX socket error");
            }
            RELEASE_BB_BUFFER (tmpbuf);
        }/* FD_ISSET() */

        if((now - last_send_mail >= conf.email_interval) && buf->num && (conf.mail_enable==1)) {
#ifdef DEBUG_SYSLOG
            mylog("<%s,%d>:It is time to send email:%d,%d\n", __FUNCTION__, __LINE__,now,last_send_mail);
#endif
            send_mail_signal(0);
#ifdef DEBUG_SYSLOG
            mylog("<%s,%d>:last_time changed:%d\n", __FUNCTION__, __LINE__,last_send_mail);
#endif            
        }
    } /* for main loop */
}

char *config_file_path;

int parse_config(char *conf_path);

static void clear_signal(int sig)
{
    buf->head=0;
    buf->tail=0;
}

static void reload_signal(int sig)
{
    parse_config(config_file_path);
}


/* Modify by Jeff -Feb.22.2005- */
int parse_config(char *conf_path)
{
    FILE *fp;
    char buf[2048];
    char tmp[32];
    int i;
#ifdef DEBUG
    printf("conf_path==%s\n",conf_path);
#endif
    if(conf_path==NULL)
        fp=fopen(__CONF_FILE,"r");
    else
        fp=fopen(conf_path,"r");

    if(fp==NULL)
        return FALSE;

    fread(buf,sizeof(buf),1,fp);
    fclose(fp);

    /* initial conf */
    bzero(&conf,sizeof(conf));
    memset(&conf.log_list,-1,sizeof(conf.log_list));
    /* initial conf */

    if(strstr(buf,"email_alert=1")) conf.mail_enable=1;

    /* if email is not enable ,we don't need to parser those config*/
    if(conf.mail_enable==1){
        if(strstr(buf,"mail_log_full=1")) conf.mail_log_full=1;
        strccpy2(conf.mail_server,buf,"smtp_mail_server=",'\n');
        strccpy2(conf.mail_receiver,buf,"email_alert_addr=",'\n');
        strccpy2(conf.mail_sender,buf,"email_return_addr=",'\n');
        strccpy2(conf.mail_subject,buf,"mail_subject=",'\n');
        strccpy2(conf.mail_subject_alert,buf,"mail_subject_alert=",'\n');
        strccpy2(conf.mail_keyword,buf,"mail_keyword=",'\n');
        strccpy2(conf.dos_thresholds,buf,"dos_thresholds=",'\n');
        strccpy2(conf.mail_auth_enable,buf, "mail_auth_enable=",'\n');
        strccpy2(tmp, buf, "email_qlen=",'\n');
        conf.email_qlen = atoi(tmp);
        strccpy2(tmp, buf, "email_interval=",'\n');
/* MK@CPU_AP change for linksys mail */
        //conf.email_interval = atoi(tmp)*60;
        conf.email_interval = atoi(tmp);
/* change end */
    }    
    strccpy2(tmp,buf,"log_send_type=",'\n');
    conf.log_send_type = atoi(tmp);

    strccpy2(conf.TZ,buf,"TZ=",'\n');

    /*setenv("TZ",conf.TZ,1);*/

    strccpy2(conf.daylight,buf,"daylight=",'\n');

    if(strstr(buf,"log_enable=1")) conf.log_enable=1;

    strccpy2(conf.time_mode, buf, "time_mode=",'\n');

    strccpy2(conf.log_keyword,buf,"log_keyword=",'\n');
    //set log_level default is 0
    for(i=0; i<(sizeof(conf.log_level)/sizeof(conf.log_level[0])); i++) {
        conf.log_level[i] = 0;
    }

    strccpy2(tmp, buf, "log_level=",'\n');    
    for(i=0; i<=atoi(tmp); i++) {
        conf.log_level[i] = 1;
    }

    return TRUE;
}

int syslogd_main(int argc, char **argv)
    //int main(int argc, char **argv)
{
    int opt;
#if ! defined(__uClinux__)
    int doFork = TRUE;
#endif

    char *p;

    /* do normal option parsing */
    while ((opt = getopt(argc, argv, "m:nO:R:f:LC")) > 0) {
        switch (opt) {
            case 'm':
                MarkInterval = atoi(optarg) * 60;
                break;
#if ! defined(__uClinux__)
            case 'n':
                doFork = FALSE;
                break;
#endif
            case 'O':
                logFilePath = strdup(optarg);
                break;
#ifdef BB_FEATURE_REMOTE_LOG
            case 'R':
                if(RemoteHost!=NULL) free(RemoteHost);
                RemoteHost = strdup(optarg);
                if ( (p = strchr(RemoteHost, ':'))){
                    RemotePort = atoi(p+1);
                    *p = '\0';
                }
                doRemoteLog = TRUE;
                break;
            case 'L':
                local_logging = TRUE;
                break;
#endif
#ifdef BB_FEATURE_IPC_SYSLOG
            case 'C':
                circular_logging = TRUE;
                break;
#endif
            case 'f':
                config_file_path=optarg;
                if(parse_config(optarg)==FALSE)
                    show_usage();
                break;

            default:
                show_usage();
        }
    }
#ifdef BB_FEATURE_REMOTE_LOG
    /* If they have not specified remote logging, then log locally */
    if (doRemoteLog == FALSE)
        local_logging = TRUE;
#endif

#ifdef SHOW_HOSTNAME
    /* Store away localhost's name before the fork */
    gethostname(LocalHostName, sizeof(LocalHostName));
    if ((p = strchr(LocalHostName, '.'))) {
        *p++ = '\0';
    }

    /* If Hostname is NULL or contain embeded space, Show LAN IP address instead*/
    if(!strlen(LocalHostName) || strchr(LocalHostName, ' ')) {
        int sockfd = -1;
        struct ifreq ifr;
        struct sockaddr_in *saddr;

        if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("user: socket creating failed");
        }
        else {
            strcpy(ifr.ifr_name, LOGIC_LAN_IFNAME);
            ifr.ifr_addr.sa_family = AF_INET;
            /* get ip address */
            if (ioctl(sockfd, SIOCGIFADDR, &ifr)==0) {
                saddr = (struct sockaddr_in *)&ifr.ifr_addr;
                strcpy(LocalHostName, (char *)inet_ntoa(saddr->sin_addr));
            }
        }
    }
#endif
    umask(0);

#if ! defined(__uClinux__)
    if (doFork == TRUE) {
        if (daemon(0, 1) < 0)
            perror_msg_and_die("daemon");
    }
#endif
    doSyslogd();

    return EXIT_SUCCESS;
}
#if 1
extern int klogd_main (int argc ,char **argv);

int main(int argc ,char **argv)
{
    int ret = 0;
    char *base = strrchr(argv[0], '/');
    time(&last_send_mail);

    if (strstr(base ? (base + 1) : argv[0], "syslogd"))
        ret = syslogd_main(argc,argv);
    else if (strstr(base ? (base + 1) : argv[0], "klogd"))
        ret = klogd_main(argc,argv);
    else
        show_usage();

    return ret;
}
#endif
/*
   Local Variables
   c-file-style: "linux"
   c-basic-offset: 4
   tab-width: 4
End:
 */
