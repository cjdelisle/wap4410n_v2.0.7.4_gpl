/*
 * snmptrap.c - send snmp traps to a network entity.
 *
 */
/******************************************************************
	Copyright 1989, 1991, 1992 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/
#include <net-snmp/net-snmp-config.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>
#if HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#include <stdio.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <net-snmp/net-snmp-includes.h>

//#define SC_TRAP_DEBUG

#ifdef LINKSYS
oid             objid_enterprise[] = { 1, 3, 6, 1, 4, 1, 9 };
oid             objid_snmptrap[] = { 1, 3, 6, 1, 4, 1, 9, 6, 1, 32, 4410 };
#else
oid             objid_enterprise[] = { 1, 3, 6, 1, 4, 1, 915 };
oid             objid_snmptrap[] = { 1, 3, 6, 1, 4, 1, 915, 4, 5 };
#endif

oid             objid_sysdescr[] = { 1, 3, 6, 1, 2, 1, 1, 1 };
oid             objid_sysuptime[] = { 1, 3, 6, 1, 2, 1, 1, 3 };
int             inform = 0;

static int
snmp_input(int operation,
           netsnmp_session * session,
           int reqid, netsnmp_pdu *pdu, void *magic)
{
    return 1;
}

static int snmp_trap_args(netsnmp_session *session)
{
    FILE *fp;
    char line[64+1];
    char *delim = "=";
    char *p = NULL;
    
    /*If open the trap config file fail, then return*/
    if((fp=fopen("/tmp/snmpdtrap.conf","r"))==NULL) 
        return -1;
    
    /*init session*/
    snmp_sess_init(session);
    
    /*Paser the config*/
    while(fgets(line, 64, fp)){
        
        if((p = strstr(line, delim)) == NULL)
                continue;
            p++;
            
        /*version*/
        if(strstr(line, "ver")){
             
#ifdef SC_TRAP_DEBUG
            printf("version=%s\n", p);
#endif            
            switch(atoi(p)){
                default:
                case 1:
                    session->version = SNMP_VERSION_1;
                    break;
                case 2:
                    session->version = SNMP_VERSION_2c;    
                    break;
                case 3:
                    session->version = SNMP_VERSION_3;
                    break;                    
            }
        }
        /*community*/
        else if(strstr(line, "com")){

#ifdef SC_TRAP_DEBUG
            printf("comminity=%s\n", p);
#endif            
            session->community_len = strlen(p);
            session->community = (u_char *) malloc(session->community_len);
            memmove(session->community, p, session->community_len);
        }
        /*receive IP*/
        else if(strstr(line, "pee")){

#ifdef SC_TRAP_DEBUG
            printf("receiveIp=%s\n", p);
#endif            
            session->peername = (char *)malloc(strlen(p) + 1);
            strcpy(session->peername, p);
        }
    }

    return 0;
}

int
main(int argc, char *argv[])
{
    netsnmp_session session, *ss;
    netsnmp_pdu    *pdu, *response;
    in_addr_t      *pdu_in_addr_t;
    int             status;
    int             exitval = 0;
    int             ch;
    int             trapType = 0;
    int             specifictype = 0;
    char            trapMessage[512];
    
    if( snmp_trap_args(&session) == -1)
        exit(0);
    
    memset(trapMessage,0,512);
  
    while((ch = getopt(argc,argv,"t:m:s:"))!= -1)
    {
        switch(ch)
        {
            case 't':
                trapType = atoi(optarg);
                break;
            case 'm':
                strcpy(trapMessage, optarg);
                break;
            case 's':
                specifictype = atoi(optarg);
                break;
            default:
                break;
        }
    }
  
    SOCK_STARTUP;

    session.callback = snmp_input;
    session.callback_magic = NULL;
    netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_DEFAULT_PORT, 
		       SNMP_TRAP_PORT);
   
    if (session.version == SNMP_VERSION_3 && !inform) {
        /*
         * for traps, we use ourselves as the authoritative engine
         * which is really stupid since command line apps don't have a
         * notion of a persistent engine.  Hence, our boots and time
         * values are probably always really wacked with respect to what
         * a manager would like to see.
         * 
         * The following should be enough to:
         * 
         * 1) prevent the library from doing discovery for engineid & time.
         * 2) use our engineid instead of the remote engineid for
         * authoritative & privacy related operations.
         * 3) The remote engine must be configured with users for our engineID.
         * 
         * -- Wes 
         */

        /*
         * setup the engineID based on IP addr.  Need a different
         * algorthim here.  This will cause problems with agents on the
         * same machine sending traps. 
         */
        setup_engineID(NULL, NULL);

        /*
         * pick our own engineID 
         */
        if (session.securityEngineIDLen == 0 ||
            session.securityEngineID == NULL) {
            session.securityEngineID =
                snmpv3_generate_engineID(&session.securityEngineIDLen);
        }
        if (session.contextEngineIDLen == 0 ||
            session.contextEngineID == NULL) {
            session.contextEngineID =
                snmpv3_generate_engineID(&session.contextEngineIDLen);
        }

        /*
         * set boots and time, which will cause problems if this
         * machine ever reboots and a remote trap receiver has cached our
         * boots and time...  I'll cause a not-in-time-window report to
         * be sent back to this machine. 
         */
        if (session.engineBoots == 0)
            session.engineBoots = 1;
        if (session.engineTime == 0)    /* not really correct, */
            session.engineTime = get_uptime();  /* but it'll work. Sort of. */
    }
   
    ss = snmp_open(&session);
    if (ss == NULL) {
        /*
         * diagnose snmp_open errors with the input netsnmp_session pointer 
         */
        snmp_sess_perror("snmptrap", &session);
        SOCK_CLEANUP;
        exit(1);
    }
  
    if (session.version == SNMP_VERSION_1) {
   
        if (inform) {
            fprintf(stderr, "Cannot send INFORM as SNMPv1 PDU\n");
            exit(1);
        }
        pdu = snmp_pdu_create(SNMP_MSG_TRAP);
        pdu_in_addr_t = (in_addr_t *) pdu->agent_addr;

        pdu->enterprise = (oid *) malloc(sizeof(objid_enterprise));
        memcpy(pdu->enterprise, objid_enterprise,
               sizeof(objid_enterprise));
        pdu->enterprise_length =
            sizeof(objid_enterprise) / sizeof(oid);

        *pdu_in_addr_t = get_myaddr();
       
        pdu->trap_type = trapType;

        pdu->specific_type = specifictype;

        pdu->time = get_uptime();

    } else {
        long            sysuptime;
        char            csysuptime[20];
   
        pdu = snmp_pdu_create(inform ? SNMP_MSG_INFORM : SNMP_MSG_TRAP2);
        
        sysuptime = get_uptime();
        sprintf(csysuptime, "%ld", sysuptime);
        snmp_add_var(pdu, objid_sysuptime,
                     sizeof(objid_sysuptime) / sizeof(oid), 't', csysuptime);    
                     
    }
 
    if(strlen(trapMessage) > 0)  
    if (snmp_add_var(pdu, objid_snmptrap, sizeof(objid_snmptrap) / sizeof(oid),
             's', trapMessage) != 0) {
        SOCK_CLEANUP;
        exit(1);
    }    
    
    if (inform)
        status = snmp_synch_response(ss, pdu, &response);
    else
        status = snmp_send(ss, pdu) == 0;
    if (status) {
        snmp_sess_perror(inform ? "snmpinform" : "snmptrap", ss);
        if (!inform)
            snmp_free_pdu(pdu);
        exitval = 1;
    } else if (inform)
        snmp_free_pdu(response);   
        
    snmp_close(ss);
    SOCK_CLEANUP;
    
    return exitval;
}
