/****************************************************************************
*                                                                           *
*  File Name:           dot1dBridge.c                                       *
*  Used By:                                                                 *
*                                                                           *
*  Author:              MK@SC_CPUAP                                         *
*                                                                           *
*  Creation Date:       23/08/07                                            *
*                                                                           *
****************************************************************************/
/****************************************************************************
*                               Includes                                    *
****************************************************************************/
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "dot1dBridge.h"
#include "libbridge.h"
#include "../../busybox-1.1.0/networking/libbridge/brctl.h"
#include "utility.h"
#include "util_funcs.h"

/****************************************************************************
*                                Defines                                    *
****************************************************************************/
#define TABLE_SIZE   1

#ifndef u_char
  typedef unsigned char u_char;
#endif


/****************************************************************************
*                            Private Functions                              *
****************************************************************************/
static void loadtable();
static void initStructs();
static char *htob ( char * );

static void addList ( char *, char *, int );

static void initLists();
static void flushLists();
static void flushList ( char * );
static int  hasChanged ( char *, int );
static int compare_fdbs(const void *_f0, const void *_f1);
void br_cmd_sethello(struct bridge *br, char *time, char *arg1);


/****************************************************************************
*                            Private Variables                              *
****************************************************************************/
//static unsigned long lastLoad = 0;          

static struct awNode *lastNode, *newNode, *np;


/*
 * dot1dBridge_variables_oid:
 *   this is the top level oid that we want to register under.  This
 *   is essentially a prefix, with the suffix appearing in the
 *   variable below.
 */

oid   dot1dBridge_variables_oid[] = { 1, 3, 6, 1, 2, 1, 17 };

/*
 * variable7 dot1dBridge_variables:
 *   this variable defines function callbacks and type return information 
 *   for the  mib section 
 */

struct variable7 dot1dBridge_variables[] = {
    /*
     * magic number        , variable type , ro/rw , callback fn  , L, oidsuffix 
     */
#define DOT1DSTPBRIDGEHELLOTIME		1
    {DOT1DSTPBRIDGEHELLOTIME, ASN_INTEGER, RWRITE, var_dot1dBridge, 2,
     {2, 13}},
#define DOT1DTPAGINGTIME		2
    {DOT1DTPAGINGTIME, ASN_INTEGER, RWRITE, var_dot1dBridge, 2, {4, 2}},
#define DOT1DSTPMAXAGE		3
    {DOT1DSTPMAXAGE, ASN_INTEGER, RONLY, var_dot1dBridge, 2, {2, 8}},
#define DOT1DSTPDESIGNATEDROOT		4
    {DOT1DSTPDESIGNATEDROOT, ASN_OCTET_STR, RONLY, var_dot1dBridge, 2,
     {2, 5}},
#define DOT1DSTPTOPCHANGES		5
    {DOT1DSTPTOPCHANGES, ASN_COUNTER, RONLY, var_dot1dBridge, 2, {2, 4}},
#define DOT1DSTPBRIDGEFORWARDDELAY		6
    {DOT1DSTPBRIDGEFORWARDDELAY, ASN_INTEGER, RWRITE, var_dot1dBridge, 2,
     {2, 14}},
#define DOT1DSTPFORWARDDELAY		7
    {DOT1DSTPFORWARDDELAY, ASN_INTEGER, RONLY, var_dot1dBridge, 2,
     {2, 11}},
#define DOT1DSTPHOLDTIME		8
    {DOT1DSTPHOLDTIME, ASN_INTEGER, RONLY, var_dot1dBridge, 2, {2, 10}},
#define DOT1DBASETYPE		9
    {DOT1DBASETYPE, ASN_INTEGER, RONLY, var_dot1dBridge, 2, {1, 3}},
#define DOT1DSTPPRIORITY		10
    {DOT1DSTPPRIORITY, ASN_INTEGER, RWRITE, var_dot1dBridge, 2, {2, 2}},
#define DOT1DSTPBRIDGEMAXAGE		11
    {DOT1DSTPBRIDGEMAXAGE, ASN_INTEGER, RWRITE, var_dot1dBridge, 2,
     {2, 12}},
#define DOT1DSTPROOTCOST		12
    {DOT1DSTPROOTCOST, ASN_INTEGER, RONLY, var_dot1dBridge, 2, {2, 6}},
#define DOT1DSTPHELLOTIME		13
    {DOT1DSTPHELLOTIME, ASN_INTEGER, RONLY, var_dot1dBridge, 2, {2, 9}},
#define DOT1DBASENUMPORTS		14
    {DOT1DBASENUMPORTS, ASN_INTEGER, RONLY, var_dot1dBridge, 2, {1, 2}},
//#define NEWROOT		15
//    {NEWROOT,, RONLY, var_dot1dBridge, 2, {0, 1}},
//#define TOPOLOGYCHANGE		16
//    {TOPOLOGYCHANGE,, RONLY, var_dot1dBridge, 2, {0, 2}},
#define DOT1DSTPROOTPORT		17
    {DOT1DSTPROOTPORT, ASN_INTEGER, RONLY, var_dot1dBridge, 2, {2, 7}},
#define DOT1DBASEBRIDGEADDRESS		18
    {DOT1DBASEBRIDGEADDRESS, ASN_OCTET_STR, RONLY, var_dot1dBridge, 2,
     {1, 1}},
#define DOT1DSTPPROTOCOLSPECIFICATION		19
    {DOT1DSTPPROTOCOLSPECIFICATION, ASN_INTEGER, RONLY, var_dot1dBridge, 2,
     {2, 1}},
#define DOT1DSTPTIMESINCETOPOLOGYCHANGE		20
    {DOT1DSTPTIMESINCETOPOLOGYCHANGE, ASN_TIMETICKS, RONLY,
     var_dot1dBridge, 2, {2, 3}},
#define DOT1DTPLEARNEDENTRYDISCARDS		21
    {DOT1DTPLEARNEDENTRYDISCARDS, ASN_COUNTER, RONLY, var_dot1dBridge, 2,
     {4, 1}},

#define DOT1DSTPPORT		22
    {DOT1DSTPPORT, ASN_INTEGER, RONLY, var_dot1dStpPortTable, 4,
     {2, 15, 1, 1}},
#define DOT1DSTPPORTPRIORITY		23
    {DOT1DSTPPORTPRIORITY, ASN_INTEGER, RWRITE, var_dot1dStpPortTable, 4,
     {2, 15, 1, 2}},
#define DOT1DSTPPORTSTATE		24
    {DOT1DSTPPORTSTATE, ASN_INTEGER, RONLY, var_dot1dStpPortTable, 4,
     {2, 15, 1, 3}},
#define DOT1DSTPPORTENABLE		25
    {DOT1DSTPPORTENABLE, ASN_INTEGER, RWRITE, var_dot1dStpPortTable, 4,
     {2, 15, 1, 4}},
#define DOT1DSTPPORTPATHCOST		26
    {DOT1DSTPPORTPATHCOST, ASN_INTEGER, RWRITE, var_dot1dStpPortTable, 4,
     {2, 15, 1, 5}},
#define DOT1DSTPPORTDESIGNATEDROOT		27
    {DOT1DSTPPORTDESIGNATEDROOT, ASN_OCTET_STR, RONLY,
     var_dot1dStpPortTable, 4, {2, 15, 1, 6}},
#define DOT1DSTPPORTDESIGNATEDCOST		28
    {DOT1DSTPPORTDESIGNATEDCOST, ASN_INTEGER, RONLY, var_dot1dStpPortTable,
     4, {2, 15, 1, 7}},
#define DOT1DSTPPORTDESIGNATEDBRIDGE		29
    {DOT1DSTPPORTDESIGNATEDBRIDGE, ASN_OCTET_STR, RONLY,
     var_dot1dStpPortTable, 4, {2, 15, 1, 8}},
#define DOT1DSTPPORTDESIGNATEDPORT		30
    {DOT1DSTPPORTDESIGNATEDPORT, ASN_OCTET_STR, RONLY,
     var_dot1dStpPortTable, 4, {2, 15, 1, 9}},
#define DOT1DSTPPORTFORWARDTRANSITIONS		31
    {DOT1DSTPPORTFORWARDTRANSITIONS, ASN_COUNTER, RONLY,
     var_dot1dStpPortTable, 4, {2, 15, 1, 10}},
#define DOT1DTPFDBADDRESS		32
    {DOT1DTPFDBADDRESS, ASN_OCTET_STR, RONLY, var_dot1dTpFdbTable, 4,
     {4, 3, 1, 1}},
#define DOT1DTPFDBPORT		33
    {DOT1DTPFDBPORT, ASN_INTEGER, RONLY, var_dot1dTpFdbTable, 4,
     {4, 3, 1, 2}},
#define DOT1DTPFDBSTATUS		34
    {DOT1DTPFDBSTATUS, ASN_INTEGER, RONLY, var_dot1dTpFdbTable, 4,
     {4, 3, 1, 3}},
#define DOT1DBASEPORT		35
    {DOT1DBASEPORT, ASN_INTEGER, RONLY, var_dot1dBasePortTable, 4,
     {1, 4, 1, 1}},
#define DOT1DBASEPORTIFINDEX		36
    {DOT1DBASEPORTIFINDEX, ASN_INTEGER, RONLY, var_dot1dBasePortTable, 4,
     {1, 4, 1, 2}},
#define DOT1DBASEPORTCIRCUIT		37
    {DOT1DBASEPORTCIRCUIT, ASN_OBJECT_ID, RONLY, var_dot1dBasePortTable, 4,
     {1, 4, 1, 3}},
#define DOT1DBASEPORTDELAYEXCEEDEDDISCARDS		38
    {DOT1DBASEPORTDELAYEXCEEDEDDISCARDS, ASN_COUNTER, RONLY,
     var_dot1dBasePortTable, 4, {1, 4, 1, 4}},
#define DOT1DBASEPORTMTUEXCEEDEDDISCARDS		39
    {DOT1DBASEPORTMTUEXCEEDEDDISCARDS, ASN_COUNTER, RONLY,
     var_dot1dBasePortTable, 4, {1, 4, 1, 5}},
#define DOT1DTPPORT		40
    {DOT1DTPPORT, ASN_INTEGER, RONLY, var_dot1dTpPortTable, 4,
     {4, 4, 1, 1}},
#define DOT1DTPPORTMAXINFO		41
    {DOT1DTPPORTMAXINFO, ASN_INTEGER, RONLY, var_dot1dTpPortTable, 4,
     {4, 4, 1, 2}},
#define DOT1DTPPORTINFRAMES		42
    {DOT1DTPPORTINFRAMES, ASN_COUNTER, RONLY, var_dot1dTpPortTable, 4,
     {4, 4, 1, 3}},
#define DOT1DTPPORTOUTFRAMES		43
    {DOT1DTPPORTOUTFRAMES, ASN_COUNTER, RONLY, var_dot1dTpPortTable, 4,
     {4, 4, 1, 4}},
#define DOT1DTPPORTINDISCARDS		44
    {DOT1DTPPORTINDISCARDS, ASN_COUNTER, RONLY, var_dot1dTpPortTable, 4,
     {4, 4, 1, 5}},
#define DOT1DSTATICADDRESS		45
    {DOT1DSTATICADDRESS, ASN_OCTET_STR, RWRITE, var_dot1dStaticTable, 4,
     {5, 1, 1, 1}},
#define DOT1DSTATICRECEIVEPORT		46
    {DOT1DSTATICRECEIVEPORT, ASN_INTEGER, RWRITE, var_dot1dStaticTable, 4,
     {5, 1, 1, 2}},
#define DOT1DSTATICALLOWEDTOGOTO		47
    {DOT1DSTATICALLOWEDTOGOTO, ASN_OCTET_STR, RWRITE, var_dot1dStaticTable,
     4, {5, 1, 1, 3}},
#define DOT1DSTATICSTATUS		48
    {DOT1DSTATICSTATUS, ASN_INTEGER, RWRITE, var_dot1dStaticTable, 4,
     {5, 1, 1, 4}},
};

/*
 * (L = length of the oidsuffix) 
 */


/** Initializes the dot1dBridge module */
void
init_dot1dBridge(void)
{

    DEBUGMSGTL(("dot1dBridge", "Initializing\n"));

    /*
     * register ourselves with the agent to handle our mib tree 
     */
    REGISTER_MIB("dot1dBridge", dot1dBridge_variables, variable7,
                 dot1dBridge_variables_oid);
	initLists();
    /*
     * place any other initialization junk you need here 
     */
}

/****************************************************************************
*                                                                           *
*    shutdown_() - perform any required cleanup @ shutdown      *
*                                                                           *
****************************************************************************/
void shutdown_dot1dBridge ( void )
{
  flushLists();
}

/*
 * var_dot1dBridge():
 *   This function is called every time the agent gets a request for
 *   a scalar variable that might be found within your mib section
 *   registered above.  It is up to you to do the right thing and
 *   return the correct value.
 *     You should also correct the value of "var_len" if necessary.
 *
 *   Please see the documentation for more information about writing
 *   module extensions, and check out the examples in the examples
 *   and mibII directories.
 */
unsigned char  *
var_dot1dBridge(struct variable *vp,
                oid * name,
                size_t * length,
                int exact, size_t * var_len, WriteMethod ** write_method)
{
    /*
     * variables we may use later 
     */
    loadtable();
    static char MACWork[24];
	static char MACWork1[16];
    
    if (header_generic(vp, name, length, exact, var_len, write_method)
        == MATCH_FAILED)
        return NULL;

    /*
     * this is where we do the value assignments for the mib results.
     */
    
    switch (vp->magic) {
    case DOT1DSTPBRIDGEHELLOTIME:
        *write_method = write_dot1dStpBridgeHelloTime;
        return (u_char *) &db->stpbridgehellotime;
    case DOT1DTPAGINGTIME:
        *write_method = write_dot1dTpAgingTime;
        return (u_char *) &db->agingtime;
    case DOT1DSTPMAXAGE:
        return (u_char *) &db->stpmaxage;
    case DOT1DSTPDESIGNATEDROOT:
      MACWork[ 0] = db->stpdesignatedroot[ 0];
      MACWork[ 1] = db->stpdesignatedroot[ 1];
      MACWork[ 2] = db->stpdesignatedroot[ 2];
      MACWork[ 3] = db->stpdesignatedroot[ 3];
      MACWork[ 4] = db->stpdesignatedroot[ 4];
      MACWork[ 5] = db->stpdesignatedroot[ 5];
      MACWork[ 6] = db->stpdesignatedroot[ 6];
      MACWork[ 7] = db->stpdesignatedroot[ 7];
      MACWork[ 8] = db->stpdesignatedroot[ 8];
      MACWork[ 9] = db->stpdesignatedroot[ 9];
      MACWork[10] = db->stpdesignatedroot[10];
      MACWork[11] = db->stpdesignatedroot[11];
      MACWork[12] = db->stpdesignatedroot[12];
      MACWork[13] = db->stpdesignatedroot[13];
      MACWork[14] = db->stpdesignatedroot[14];
      MACWork[15] = db->stpdesignatedroot[15];
      MACWork[16] = '\0';
      *var_len = 8;    
      return ( u_char * ) htob ( MACWork ); 
    case DOT1DSTPTOPCHANGES:
        return (u_char *) &db->stptopchanges;
    case DOT1DSTPBRIDGEFORWARDDELAY:
        *write_method = write_dot1dStpBridgeForwardDelay;
        return (u_char *) &db->stpbridgeforwarddelay;
    case DOT1DSTPFORWARDDELAY:
        return (u_char *) &db->stopforwarddelay;
    case DOT1DSTPHOLDTIME:
        return (u_char *) &db->stpholdtime;
    case DOT1DBASETYPE:
        return (u_char *) &db->basetype;
    case DOT1DSTPPRIORITY:
        *write_method = write_dot1dStpPriority;
        return (u_char *) &db->stppriority;
    case DOT1DSTPBRIDGEMAXAGE:
        *write_method = write_dot1dStpBridgeMaxAge;
        return (u_char *) &db->stpbridgemaxage;
    case DOT1DSTPROOTCOST:
        return (u_char *) &db->stprootcost;
    case DOT1DSTPHELLOTIME:
        return (u_char *) &db->stphellotime;
    case DOT1DBASENUMPORTS:
        return (u_char *) &db->basenumport;
//    case NEWROOT:
//        return NULL;
//    case TOPOLOGYCHANGE:
//        return NULL;
    case DOT1DSTPROOTPORT:
        return (u_char *) &db->stprootport;
    case DOT1DBASEBRIDGEADDRESS:    	
      MACWork1[ 0] = db->stpdesignatedroot[ 4];
      MACWork1[ 1] = db->stpdesignatedroot[ 5];
      MACWork1[ 2] = db->stpdesignatedroot[ 6];
      MACWork1[ 3] = db->stpdesignatedroot[ 7];
      MACWork1[ 4] = db->stpdesignatedroot[ 8];
      MACWork1[ 5] = db->stpdesignatedroot[ 9];
      MACWork1[ 6] = db->stpdesignatedroot[10];
      MACWork1[ 7] = db->stpdesignatedroot[11];
      MACWork1[ 8] = db->stpdesignatedroot[12];
      MACWork1[ 9] = db->stpdesignatedroot[13];
      MACWork1[10] = db->stpdesignatedroot[14];
      MACWork1[11] = db->stpdesignatedroot[15];      
      MACWork1[12] = '\0';
      *var_len = 6;    
      return ( u_char * ) htob ( MACWork1 ); 
    case DOT1DSTPPROTOCOLSPECIFICATION:
        return (u_char *) &db->stpprotocolspecification;
    case DOT1DSTPTIMESINCETOPOLOGYCHANGE:
        return (u_char *) &db->stptimesincetopologychange;
    case DOT1DTPLEARNEDENTRYDISCARDS:
        return (u_char *) &db->tplearnedentrydiscards;
    default:
        ERROR_MSG("");
    }
    return NULL;
}


/*
 * var_dot1dStpPortTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char  *
var_dot1dStpPortTable(struct variable *vp,
                      oid * name,
                      size_t * length,
                      int exact,
                      size_t * var_len, WriteMethod ** write_method)
{
    /*
     * variables we may use later 
     */
    int found = FALSE;    
    oid rName [ MAX_OID_LEN ]; 
    static char MACWork[24];
	static char MACWork1[16];   

    loadtable();
    memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
    for ( np = LIST_FIRST ( &spList ); np != NULL; np = LIST_NEXT ( np, nodes )) 
    {
    	sp = ( struct spTbl_data * ) np->data;
    	rName[vp->namelen + 0] = sp->stpport;    
    	if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) == 0 )) || 
        	( !exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) >  0 )))
        {
        	found = TRUE;
        	break;
        }
    }
	if(!found)
		return NULL;

    /*
     * this is where we do the value assignments for the mib results.
     */
    memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 1 ) * sizeof ( oid ));
    *length = vp->namelen + 1;
    *var_len = sizeof ( long );
    *write_method = NULL; 
     
    switch (vp->magic) {
    case DOT1DSTPPORT:
        return (u_char *) &sp->stpport;
    case DOT1DSTPPORTPRIORITY:
        *write_method = write_dot1dStpPortPriority;
        return (u_char *) &sp->stpportpriority;
    case DOT1DSTPPORTSTATE:
        return (u_char *) &sp->stpportstate;
    case DOT1DSTPPORTENABLE:
        //*write_method = write_dot1dStpPortEnable;
        return (u_char *) &sp->stpportenable;
    case DOT1DSTPPORTPATHCOST:
        *write_method = write_dot1dStpPortPathCost;
        return (u_char *) &sp->stpportpathcost;
    case DOT1DSTPPORTDESIGNATEDROOT:
      MACWork[ 0] = sp->stpportdesignatedroot[ 0];
      MACWork[ 1] = sp->stpportdesignatedroot[ 1];
      MACWork[ 2] = sp->stpportdesignatedroot[ 2];
      MACWork[ 3] = sp->stpportdesignatedroot[ 3];
      MACWork[ 4] = sp->stpportdesignatedroot[ 4];
      MACWork[ 5] = sp->stpportdesignatedroot[ 5];
      MACWork[ 6] = sp->stpportdesignatedroot[ 6];
      MACWork[ 7] = sp->stpportdesignatedroot[ 7];
      MACWork[ 8] = sp->stpportdesignatedroot[ 8];
      MACWork[ 9] = sp->stpportdesignatedroot[ 9];
      MACWork[10] = sp->stpportdesignatedroot[10];
      MACWork[11] = sp->stpportdesignatedroot[11];
      MACWork[12] = sp->stpportdesignatedroot[12];
      MACWork[13] = sp->stpportdesignatedroot[13];
      MACWork[14] = sp->stpportdesignatedroot[14];
      MACWork[15] = sp->stpportdesignatedroot[15];
      MACWork[16] = '\0';
      *var_len = 8;    
      return ( u_char * ) htob ( MACWork );          
    case DOT1DSTPPORTDESIGNATEDCOST:
        return (u_char *) &sp->stpportdesignatedcost;
    case DOT1DSTPPORTDESIGNATEDBRIDGE:
      MACWork1[ 0] = sp->stpportdesignatedbridge[ 0];
      MACWork1[ 1] = sp->stpportdesignatedbridge[ 1];
      MACWork1[ 2] = sp->stpportdesignatedbridge[ 2];
      MACWork1[ 3] = sp->stpportdesignatedbridge[ 3];
      MACWork1[ 4] = sp->stpportdesignatedbridge[ 4];
      MACWork1[ 5] = sp->stpportdesignatedbridge[ 5];
      MACWork1[ 6] = sp->stpportdesignatedbridge[ 6];
      MACWork1[ 7] = sp->stpportdesignatedbridge[ 7];
      MACWork1[ 8] = sp->stpportdesignatedbridge[ 8];
      MACWork1[ 9] = sp->stpportdesignatedbridge[ 9];
      MACWork1[10] = sp->stpportdesignatedbridge[10];
      MACWork1[11] = sp->stpportdesignatedbridge[11];
      MACWork1[12] = sp->stpportdesignatedbridge[12];
      MACWork1[13] = sp->stpportdesignatedbridge[13];
      MACWork1[14] = sp->stpportdesignatedbridge[14];
      MACWork1[15] = sp->stpportdesignatedbridge[15];
      MACWork1[16] = '\0';
      *var_len = 8;    
      return ( u_char * ) htob ( MACWork1 ); 
    case DOT1DSTPPORTDESIGNATEDPORT:
        *var_len = 2;         
        return sp->stpportdesignatedport;
    case DOT1DSTPPORTFORWARDTRANSITIONS:
        return (u_char *) &sp->stpportforwardtransitions;
    default:
        ERROR_MSG("");
    }
    return NULL;
}

/*
 * var_dot1dTpFdbTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char  *
var_dot1dTpFdbTable(struct variable *vp,
                    oid * name,
                    size_t * length,
                    int exact,
                    size_t * var_len, WriteMethod ** write_method)
{
    /*
     * variables we may use later 
     */
    oid rName [ MAX_OID_LEN ];           
    int found = FALSE;
    static char MACWork[24];    
    /*
     * This assumes that the table is a 'simple' table.
     *  See the implementation documentation for the meaning of this.
     *  You will need to provide the correct value for the TABLE_SIZE parameter
     *
     * If this table does not meet the requirements for a simple table,
     *  you will need to provide the replacement code yourself.
     *  Mib2c is not smart enough to write this for you.
     *    Again, see the implementation documentation for what is required.
     */
    loadtable();
    memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
    for ( np = LIST_FIRST ( &tfList ); np != NULL; np = LIST_NEXT ( np, nodes )) 
    {
    tf = ( struct tfTbl_data * ) np->data;
    rName[vp->namelen + 0] = tf->ifIndex;    
    //rName[vp->namelen + 1] = tf->tpfdbaddress;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) == 0 )) || 
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) >  0 )))
        {
        	found = TRUE;
        	break;
        }
    }
	if(!found)
		return NULL; 

    /*
     * this is where we do the value assignments for the mib results.
     */
    memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 1 ) * sizeof ( oid ));
    *length = vp->namelen + 1;
    *var_len = sizeof ( long );
    *write_method = NULL;      
   
    switch (vp->magic) {
    case DOT1DTPFDBADDRESS:
      MACWork[ 0] = tf->tpfdbaddress[ 0];
      MACWork[ 1] = tf->tpfdbaddress[ 1];
      MACWork[ 2] = tf->tpfdbaddress[ 2];
      MACWork[ 3] = tf->tpfdbaddress[ 3];
      MACWork[ 4] = tf->tpfdbaddress[ 4];
      MACWork[ 5] = tf->tpfdbaddress[ 5];
      MACWork[ 6] = tf->tpfdbaddress[ 6];
      MACWork[ 7] = tf->tpfdbaddress[ 7];
      MACWork[ 8] = tf->tpfdbaddress[ 8];
      MACWork[ 9] = tf->tpfdbaddress[ 9];
      MACWork[10] = tf->tpfdbaddress[10];
      MACWork[11] = tf->tpfdbaddress[11];      
      MACWork[12] = '\0';
      *var_len = 6;    
      return ( u_char * ) htob ( MACWork );
    case DOT1DTPFDBPORT:
        return (u_char *) &tf->tpfdbport;
    case DOT1DTPFDBSTATUS:
        return (u_char *) &tf->tpfdbstatus;
    default:
        ERROR_MSG("");
    }
    return NULL;
}

/*
 * var_dot1dBasePortTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char  *
var_dot1dBasePortTable(struct variable *vp,
                       oid * name,
                       size_t * length,
                       int exact,
                       size_t * var_len, WriteMethod ** write_method)
{
    /*
     * variables we may use later 
     */
    oid rName [ MAX_OID_LEN ];
    int found = FALSE;        

    loadtable();
    memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
    for ( np = LIST_FIRST ( &bpList ); np != NULL; np = LIST_NEXT ( np, nodes )) 
    {
	    bp = ( struct bpTbl_data * ) np->data;	    
	    rName[vp->namelen + 0] = bp->baseport;
	    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) == 0 )) || 
        	( !exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) >  0 )))
        {
        	found = TRUE;
        	break;
        }
    }
	if(!found)
		return NULL;
		
    /*
     * this is where we do the value assignments for the mib results.
     */
    memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 1 ) * sizeof ( oid ));
    *length = vp->namelen + 1;
    *var_len = sizeof ( long );
    *write_method = NULL;      
   
    switch (vp->magic) {
    case DOT1DBASEPORT:
        return (u_char *) &bp->baseport;
    case DOT1DBASEPORTIFINDEX:
        return (u_char *) &bp->baseportfindex;
    case DOT1DBASEPORTCIRCUIT:
        return (u_char *) bp->baseportcircuit;
    case DOT1DBASEPORTDELAYEXCEEDEDDISCARDS:
        return (u_char *) &bp->baseportdelayexceededdiscards;
    case DOT1DBASEPORTMTUEXCEEDEDDISCARDS:
        return (u_char *) &bp->baseportmtuexceededdiscards;
    default:
        ERROR_MSG("");
    }
    return NULL;
}

/*
 * var_dot1dTpPortTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char  *
var_dot1dTpPortTable(struct variable *vp,
                     oid * name,
                     size_t * length,
                     int exact,
                     size_t * var_len, WriteMethod ** write_method)
{
    /*
     * variables we may use later 
     */
    oid rName [ MAX_OID_LEN ];
    int found = FALSE;    

    loadtable();
    memcpy (( char * ) rName, ( char * ) vp->name, ( int ) vp->namelen * sizeof ( oid ));
    for ( np = LIST_FIRST ( &tpList ); np != NULL; np = LIST_NEXT ( np, nodes )) 
    {
    tp = ( struct tpTbl_data * ) np->data;
    rName[vp->namelen] = tp->tpport;
    if ((  exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) == 0 )) || 
        ( !exact && ( snmp_oid_compare ( rName, vp->namelen + 1, name, *length ) >  0 )))
        {
        	found = TRUE;
        	break;        	
        } 
    }
	if(!found)
		return NULL;
    /*
     * this is where we do the value assignments for the mib results.
     */
    memcpy (( char * ) name, ( char * ) rName, ( vp->namelen + 1 ) * sizeof ( oid ));
    *length = vp->namelen + 1;
    *var_len = sizeof ( long );
    *write_method = NULL;      
     
    switch (vp->magic) {
    case DOT1DTPPORT:
        return (u_char *) &tp->tpport;
    case DOT1DTPPORTMAXINFO:
        return (u_char *) &tp->tpportmaxinfo;
    case DOT1DTPPORTINFRAMES:
        return (u_char *) &tp->tpportinframes;
    case DOT1DTPPORTOUTFRAMES:
        return (u_char *) &tp->tpportoutframes;
    case DOT1DTPPORTINDISCARDS:
        return (u_char *) &tp->tpportindiscards;
    default:
        ERROR_MSG("");
    }
    return NULL;
}

/*
 * var_dot1dStaticTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char  *
var_dot1dStaticTable(struct variable *vp,
                     oid * name,
                     size_t * length,
                     int exact,
                     size_t * var_len, WriteMethod ** write_method)
{
    /*
     * variables we may use later 
     */
    /*
     * This assumes that the table is a 'simple' table.
     *  See the implementation documentation for the meaning of this.
     *  You will need to provide the correct value for the TABLE_SIZE parameter
     *
     * If this table does not meet the requirements for a simple table,
     *  you will need to provide the replacement code yourself.
     *  Mib2c is not smart enough to write this for you.
     *    Again, see the implementation documentation for what is required.
     */
        return NULL;

    /*
     * this is where we do the value assignments for the mib results.
     */
    switch (vp->magic) {
    case DOT1DSTATICADDRESS:
        //*write_method = write_dot1dStaticAddress;
        return NULL;
    case DOT1DSTATICRECEIVEPORT:
        //*write_method = write_dot1dStaticReceivePort;
        return NULL;
    case DOT1DSTATICALLOWEDTOGOTO:
        *write_method = write_dot1dStaticAllowedToGoTo;
        return NULL;
    case DOT1DSTATICSTATUS:
        //*write_method = write_dot1dStaticStatus;
        return NULL;
    default:
        ERROR_MSG("");
    }
    return NULL;
}



int
write_dot1dStpBridgeHelloTime(int action,
                              u_char * var_val,
                              u_char var_val_type,
                              size_t var_val_len,
                              u_char * statP, 
                              oid * name, 
                              size_t name_len)
{
    //struct timeval tv;
    //struct bridge *b;
    
    long  value;
    int   size;	   
		

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            fprintf(stderr, "write to dot1dBridge not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            fprintf(stderr, "write to dot1dBridge: bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        if(*(long *)var_val < 10 || *(long *)var_val > 1000)
        {
            fprintf(stderr, "write to dot1dBridge: value out of range\n");
            return SNMP_ERR_WRONGVALUE;        	
        }
        break;

    case RESERVE2:
        size = var_val_len;
        value = *(long *) var_val;

        break;

    case FREE:
        /*
         * Release any resources that have been allocated 
         */
        break;

    case ACTION:        
        if(*(long *)var_val >= 10 && *(long *)var_val <= 1000)
        {
			int tmp;
			tmp=*(long *)var_val/10;
			SYSTEM("brctl sethello br0 %d > /dev/null 2>&1", tmp);	
        }
		
        break;

    case UNDO:
        /*
         * Back out any changes made in the ACTION case 
         */
        break;

    case COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}


int
write_dot1dTpAgingTime(int action,
                       u_char * var_val,
                       u_char var_val_type,
                       size_t var_val_len,
                       u_char * statP, oid * name, size_t name_len)
{
    long            value;
    int             size;

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            fprintf(stderr, "write to dot1dBridge not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            fprintf(stderr, "write to dot1dBridge: bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        if(*(long *)var_val < 10 || *(long *)var_val > 1000000)
        {
            fprintf(stderr, "write to dot1dBridge: value out of range\n");
            return SNMP_ERR_WRONGVALUE;        	
        }        
        break;

    case RESERVE2:
        size = var_val_len;
        value = *(long *) var_val;

        break;

    case FREE:
        /*
         * Release any resources that have been allocated 
         */
        break;

    case ACTION:
        if(*(long *)var_val >= 10 && *(long *)var_val <= 1000000)
        {
			SYSTEM("brctl setageing br0 %d > /dev/null 2>&1", *(long *)var_val);	
        }
        break;

    case UNDO:
        /*
         * Back out any changes made in the ACTION case 
         */
        break;

    case COMMIT:
        /*
         * Things are working well, so it's now safe to make the change
         * permanently.  Make sure that anything done here can't fail!
         */
        break;
    }
    return SNMP_ERR_NOERROR;
}


int
write_dot1dStpBridgeForwardDelay(int action,
                                 u_char * var_val,
                                 u_char var_val_type,
                                 size_t var_val_len,
                                 u_char * statP,
                                 oid * name, size_t name_len)
{
    long            value;
    int             size;

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            fprintf(stderr, "write to dot1dBridge not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            fprintf(stderr, "write to dot1dBridge: bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        if(*(long *)var_val < 10 || *(long *)var_val > 300)
        {
            fprintf(stderr, "write to dot1dBridge: value out of range\n");
            return SNMP_ERR_WRONGVALUE;        	
        }        
        break;

    case RESERVE2:
        size = var_val_len;
        value = *(long *) var_val;

        break;

    case FREE:
        /*
         * Release any resources that have been allocated 
         */
        break;

    case ACTION:
        if(*(long *)var_val >= 10 && *(long *)var_val <= 300)
        {
			SYSTEM("brctl setfd br0 %d > /dev/null 2>&1", *(long *)var_val);	
        }
        break;

    case UNDO:
        /*
         * Back out any changes made in the ACTION case 
         */
        break;

    case COMMIT:
        /*
         * Things are working well, so it's now safe to make the change
         * permanently.  Make sure that anything done here can't fail!
         */
        break;
    }
    return SNMP_ERR_NOERROR;
}


int
write_dot1dStpPriority(int action,
                       u_char * var_val,
                       u_char var_val_type,
                       size_t var_val_len,
                       u_char * statP, oid * name, size_t name_len)
{
    long            value;
    int             size;

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            fprintf(stderr, "write to dot1dBridge not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            fprintf(stderr, "write to dot1dBridge: bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        if(*(long *)var_val < 0 || *(long *)var_val > 65535)
        {
            fprintf(stderr, "write to dot1dBridge: value out of range\n");
            return SNMP_ERR_WRONGVALUE;        	
        }             
        break;

    case RESERVE2:
        size = var_val_len;
        value = *(long *) var_val;

        break;

    case FREE:
        /*
         * Release any resources that have been allocated 
         */
        break;

    case ACTION:
        if(*(long *)var_val >= 0 && *(long *)var_val <= 65535)
        {
			SYSTEM("brctl setbridgeprio br0 %d > /dev/null 2>&1", *(long *)var_val);	
        }
        break;

    case UNDO:
        /*
         * Back out any changes made in the ACTION case 
         */
        break;

    case COMMIT:
        /*
         * Things are working well, so it's now safe to make the change
         * permanently.  Make sure that anything done here can't fail!
         */
        break;
    }
    return SNMP_ERR_NOERROR;
}


int
write_dot1dStpBridgeMaxAge(int action,
                           u_char * var_val,
                           u_char var_val_type,
                           size_t var_val_len,
                           u_char * statP, oid * name, size_t name_len)
{
    long            value;
    int             size;

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            fprintf(stderr, "write to dot1dBridge not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            fprintf(stderr, "write to dot1dBridge: bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        if(*(long *)var_val < 100 || *(long *)var_val > 4000)
        {
            fprintf(stderr, "write to dot1dBridge: value out of range\n");
            return SNMP_ERR_WRONGVALUE;        	
        }         
        break;

    case RESERVE2:
        size = var_val_len;
        value = *(long *) var_val;

        break;

    case FREE:
        /*
         * Release any resources that have been allocated 
         */
        break;

    case ACTION:
        if(*(long *)var_val >= 100 && *(long *)var_val <= 4000)
        {
			int tmp;
			tmp=*(long *)var_val/10;
			SYSTEM("brctl setmaxage br0 %d > /dev/null 2>&1", tmp);	
        }
        break;

    case UNDO:
        /*
         * Back out any changes made in the ACTION case 
         */
        break;

    case COMMIT:
        /*
         * Things are working well, so it's now safe to make the change
         * permanently.  Make sure that anything done here can't fail!
         */
        break;
    }
    return SNMP_ERR_NOERROR;
}

int
write_dot1dStpPortPriority(int action,
                           u_char * var_val,
                           u_char var_val_type,
                           size_t var_val_len,
                           u_char * statP, 
                           oid * name, 
                           size_t name_len)
{
    struct bridge *b;
    struct port *pi;
    
    long  value;
    int   size;
    char  pName[IFNAMSIZ];
    char  bName[32];
    int   i=0;	

	sprintf(bName, "br%d", 0);
	br_init();		
	b = br_find_bridge(bName);
	pi = b->firstport;
	i = name[name_len-1];
	for(;i>1&&pi!= NULL;i--)
		pi = pi->next;	
    if_indextoname(pi->ifindex, pName);	

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            fprintf(stderr, "write to dot1dBridge not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            fprintf(stderr, "write to dot1dBridge: bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        if(*(long *)var_val < 0 || *(long *)var_val > 255)
        {
            fprintf(stderr, "write to dot1dBridge: value out of range\n");
            return SNMP_ERR_WRONGVALUE;        	
        }         
        break;

    case RESERVE2:
        size = var_val_len;
        value = *(long *) var_val;

        break;

    case FREE:
        /*
         * Release any resources that have been allocated 
         */
        break;

    case ACTION:
        if(*(long *)var_val >= 0 && *(long *)var_val <= 255)
        {
			SYSTEM("brctl setportprio br0 %s %d > /dev/null 2>&1", pName,*(long *)var_val);	
        }
        break;

    case UNDO:
        /*
         * Back out any changes made in the ACTION case 
         */
        break;

    case COMMIT:
        /*
         * Things are working well, so it's now safe to make the change
         * permanently.  Make sure that anything done here can't fail!
         */
        break;
    }
    return SNMP_ERR_NOERROR;
}

int
write_dot1dStpPortEnable(int action,
                         u_char * var_val,
                         u_char var_val_type,
                         size_t var_val_len,
                         u_char * statP, oid * name, size_t name_len)
{
    struct bridge *b;
    struct port *pi;
    
    long  value;
    int   size;
    char  pName[IFNAMSIZ];
    char  bName[32];
    int   i=0;	

	sprintf(bName, "br%d", 0);
	br_init();		
	b = br_find_bridge(bName);
	//printf("%s\n",bName);
	pi = b->firstport;
	i = name[name_len-1];
	for(;i>1&&pi!= NULL;i--)
		pi = pi->next;	
    if_indextoname(pi->ifindex, pName);	
    //printf("changed : %s\n",if_indextoname(pi->ifindex, pName));

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            fprintf(stderr, "write to dot1dBridge not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            fprintf(stderr, "write to dot1dBridge: bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        break;

    case RESERVE2:
        size = var_val_len;
        value = *(long *) var_val;

        break;

    case FREE:
        /*
         * Release any resources that have been allocated 
         */
        break;

    case ACTION:
        /*
         * The variable has been stored in 'value' for you to use,
         * and you have just been asked to do something with it.
         * Note that anything done here must be reversable in the UNDO case
         */
        break;

    case UNDO:
        /*
         * Back out any changes made in the ACTION case 
         */
        break;

    case COMMIT:
        /*
         * Things are working well, so it's now safe to make the change
         * permanently.  Make sure that anything done here can't fail!
         */
        break;
    }
    return SNMP_ERR_NOERROR;
}

int
write_dot1dStpPortPathCost(int action,
                           u_char * var_val,
                           u_char var_val_type,
                           size_t var_val_len,
                           u_char * statP, oid * name, size_t name_len)
{
    struct bridge *b;
    struct port *pi;
    
    long  value;
    int   size;
    char  pName[IFNAMSIZ];
    char  bName[32];
    int   i=0;	

	sprintf(bName, "br%d", 0);
	br_init();		
	b = br_find_bridge(bName);
	pi = b->firstport;
	i = name[name_len-1];
	for(;i>1&&pi!= NULL;i--)
		pi = pi->next;	
    if_indextoname(pi->ifindex, pName);	

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            fprintf(stderr, "write to dot1dBridge not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            fprintf(stderr, "write to dot1dBridge: bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        if(*(long *)var_val < 1 || *(long *)var_val > 65535)
        {
            fprintf(stderr, "write to dot1dBridge: value out of range\n");
            return SNMP_ERR_WRONGVALUE;        	
        }        
        break;

    case RESERVE2:
        size = var_val_len;
        value = *(long *) var_val;

        break;

    case FREE:
        /*
         * Release any resources that have been allocated 
         */
        break;

    case ACTION:
        if(*(long *)var_val >= 1 && *(long *)var_val <= 65535)
        {
			SYSTEM("brctl setpathcost br0 %s %d > /dev/null 2>&1", pName,*(long *)var_val);	
        }
        break;

    case UNDO:
        /*
         * Back out any changes made in the ACTION case 
         */
        break;

    case COMMIT:
        /*
         * Things are working well, so it's now safe to make the change
         * permanently.  Make sure that anything done here can't fail!
         */
        break;
    }
    return SNMP_ERR_NOERROR;
}

int
write_dot1dStaticAddress(int action,
                         u_char * var_val,
                         u_char var_val_type,
                         size_t var_val_len,
                         u_char * statP, oid * name, size_t name_len)
{
    char            value;
    int             size;

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_OCTET_STR) {
            fprintf(stderr, "write to dot1dBridge not ASN_OCTET_STR\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(char)) {
            fprintf(stderr, "write to dot1dBridge: bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        break;

    case RESERVE2:
        size = var_val_len;
        value = *(char *) var_val;

        break;

    case FREE:
        /*
         * Release any resources that have been allocated 
         */
        break;

    case ACTION:
        /*
         * The variable has been stored in 'value' for you to use,
         * and you have just been asked to do something with it.
         * Note that anything done here must be reversable in the UNDO case
         */
        break;

    case UNDO:
        /*
         * Back out any changes made in the ACTION case 
         */
        break;

    case COMMIT:
        /*
         * Things are working well, so it's now safe to make the change
         * permanently.  Make sure that anything done here can't fail!
         */
        break;
    }
    return SNMP_ERR_NOERROR;
}

int
write_dot1dStaticReceivePort(int action,
                             u_char * var_val,
                             u_char var_val_type,
                             size_t var_val_len,
                             u_char * statP, oid * name, size_t name_len)
{
    long            value;
    int             size;

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            fprintf(stderr, "write to dot1dBridge not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            fprintf(stderr, "write to dot1dBridge: bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        break;

    case RESERVE2:
        size = var_val_len;
        value = *(long *) var_val;

        break;

    case FREE:
        /*
         * Release any resources that have been allocated 
         */
        break;

    case ACTION:
        /*
         * The variable has been stored in 'value' for you to use,
         * and you have just been asked to do something with it.
         * Note that anything done here must be reversable in the UNDO case
         */
        break;

    case UNDO:
        /*
         * Back out any changes made in the ACTION case 
         */
        break;

    case COMMIT:
        /*
         * Things are working well, so it's now safe to make the change
         * permanently.  Make sure that anything done here can't fail!
         */
        break;
    }
    return SNMP_ERR_NOERROR;
}

int
write_dot1dStaticAllowedToGoTo(int action,
                               u_char * var_val,
                               u_char var_val_type,
                               size_t var_val_len,
                               u_char * statP, oid * name, size_t name_len)
{
    char            value;
    int             size;

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_OCTET_STR) {
            fprintf(stderr, "write to dot1dBridge not ASN_OCTET_STR\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(char)) {
            fprintf(stderr, "write to dot1dBridge: bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        break;

    case RESERVE2:
        size = var_val_len;
        value = *(char *) var_val;

        break;

    case FREE:
        /*
         * Release any resources that have been allocated 
         */
        break;

    case ACTION:
        /*
         * The variable has been stored in 'value' for you to use,
         * and you have just been asked to do something with it.
         * Note that anything done here must be reversable in the UNDO case
         */
        break;

    case UNDO:
        /*
         * Back out any changes made in the ACTION case 
         */
        break;

    case COMMIT:
        /*
         * Things are working well, so it's now safe to make the change
         * permanently.  Make sure that anything done here can't fail!
         */
        break;
    }
    return SNMP_ERR_NOERROR;
}

int
write_dot1dStaticStatus(int action,
                        u_char * var_val,
                        u_char var_val_type,
                        size_t var_val_len,
                        u_char * statP, oid * name, size_t name_len)
{
    long            value;
    int             size;

    switch (action) {
    case RESERVE1:
        if (var_val_type != ASN_INTEGER) {
            fprintf(stderr, "write to dot1dBridge not ASN_INTEGER\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(long)) {
            fprintf(stderr, "write to dot1dBridge: bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        break;

    case RESERVE2:
        size = var_val_len;
        value = *(long *) var_val;

        break;

    case FREE:
        /*
         * Release any resources that have been allocated 
         */
        break;

    case ACTION:
        /*
         * The variable has been stored in 'value' for you to use,
         * and you have just been asked to do something with it.
         * Note that anything done here must be reversable in the UNDO case
         */
        break;

    case UNDO:
        /*
         * Back out any changes made in the ACTION case 
         */
        break;

    case COMMIT:
        /*
         * Things are working well, so it's now safe to make the change
         * permanently.  Make sure that anything done here can't fail!
         */
        break;
    }
    return SNMP_ERR_NOERROR;
}


/****************************************************************************
*                                                                           *
*                     initStructs() - initialize structures                 *
*                                                                           *
****************************************************************************/
static void initStructs()
{
 
  memset (( char * ) &nsp, 0, sizeof ( nsp )); 
  memset (( char * ) &nbp, 0, sizeof ( nbp ));
  memset (( char * ) &ntf, 0, sizeof ( ntf ));  
  memset (( char * ) &ntp, 0, sizeof ( ntp ));

}

/****************************************************************************
*                                                                           *
*              initLists() - initialize all the linked lists                *
*                                                                           *
****************************************************************************/
static void initLists()
{
  LIST_INIT ( &spList );  
  LIST_INIT ( &bpList );  
  LIST_INIT ( &tfList );
  LIST_INIT ( &tpList );  
}

/****************************************************************************
*                                                                           *
*                 flushLists() - flush all linked lists                     *
*                                                                           *
****************************************************************************/
static void flushLists()
{
  flushList (( char * ) &spList );  
  flushList (( char * ) &bpList );
  flushList (( char * ) &tfList );  
  flushList (( char * ) &tpList );
}

/****************************************************************************
*                                                                           *
*                 compare_fdbs() and br_cmd_sethello()                      *
*                                                                           *
****************************************************************************/
static int compare_fdbs(const void *_f0, const void *_f1)
{
	const struct fdb_entry *f0 = _f0;
	const struct fdb_entry *f1 = _f1;

#if 0
	if (f0->port_no < f1->port_no)
		return -1;

	if (f0->port_no > f1->port_no)
		return 1;
#endif

	return memcmp(f0->mac_addr, f1->mac_addr, 6);
	
}

void br_cmd_sethello(struct bridge *br, char *time, char *arg1)
{
	double secs;
	struct timeval tv;

	sscanf(time, "%lf", &secs);
	tv.tv_sec = secs;
	tv.tv_usec = 1000000 * (secs - tv.tv_sec);
	br_set_bridge_hello_time(br, &tv);
}


/****************************************************************************
*                                                                           *
*                   flushList() - flush a linked list                       *
*                                                                           *
****************************************************************************/
static void flushList ( char *l )
{
  LIST_HEAD ( , awNode ) *list;
  
  list = ( LIST_HEAD ( , awNode ) * ) l;    // NOTE: don't know how to get 
  while ( !LIST_EMPTY ( list )) {           //  rid of compiler warning on
    np = LIST_FIRST ( list );               //  LISTHEAD typecast
    if ( np->data )
      free ( np->data );
    LIST_REMOVE ( np, nodes );
    free ( np );
  }
}

/****************************************************************************
*                                                                           *
*                addList() - add an entry to a linked list                  *
*                                                                           *
****************************************************************************/
static void 
addList ( char *l, char *data, int len  )
{
  char uid[256];
  LIST_HEAD ( , awNode ) *list;       

  // NOTE: this assumes the UID is at the begining of the 
  //       data structure and that UIDs are strings
  
  list = ( LIST_HEAD ( , awNode ) * ) l;            // NOTE: don't know how to get 
  strcpy ( uid, data );                             //  rid of compiler warning on
                                                    //  LISTHEAD typecast
  // create a new node and the data that goes in it
  newNode = malloc ( sizeof ( struct awNode ));
  newNode->data = malloc ( len );
  memcpy ( newNode->data, data, len );

  // this deals with an empty list
  if ( LIST_EMPTY ( list )) {
    LIST_INSERT_HEAD ( list, newNode, nodes );
    return;
  }

  // this deals with UIDs that match
  for ( np = LIST_FIRST ( list ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    if ( strncmp ( uid, np->data, strlen ( uid )) == 0 ) {                      // found matching UID
      LIST_INSERT_AFTER ( np, newNode, nodes );
      if ( np->data )
        free ( np->data );
      LIST_REMOVE ( np, nodes );
      free ( np );
      return;
    }
  }

  // this deals with inserting a new UID in the list
  for ( np = LIST_FIRST ( list ); np != NULL; np = LIST_NEXT ( np, nodes )) {
    lastNode = np;
    if ( strncmp ( np->data, uid, strlen ( uid )) > 0 ) {                       // old ID > new ID AND
      LIST_INSERT_BEFORE ( np, newNode, nodes );
      return;
    }
  }

  // this deals with a UID that needs to go on the end of the list
  LIST_INSERT_AFTER ( lastNode, newNode, nodes );

  return;
}

/****************************************************************************
*                                                                           *
*                 htob - converts hex string to binary                      *
*                                                                           *
****************************************************************************/
static char *htob ( char *s )
{
    char nibl, *byt;
    static char bin[20];

    byt = bin;

    while ((nibl = *s++) && nibl != ' ') {    /* While not end of string. */
      nibl -= ( nibl > '9') ?  ('A' - 10): '0';
      *byt = nibl << 4;                              /* place high nibble */
      if((nibl = *s++) && nibl != ' ') {
        nibl -= ( nibl > '9') ?  ('A' - 10): '0';
        *byt |= nibl;                                /*  place low nibble */
      }
      else break;
      ++byt;
    }
    *++byt = '\0';
    return ( bin );
}

/****************************************************************************
*                                                                           *
*           hasChanged() - see if area has been changed from NULLs          *
*                                                                           *
****************************************************************************/
static int hasChanged ( char *loc, int len )
{
	char *wrk;
	int changed = TRUE;

	wrk = malloc ( len );
	memset ( wrk, 0, len );
	if ( memcmp ( loc, wrk, len ) == 0 )
    changed = FALSE;
	free ( wrk );

	return ( changed );
}



/****************************************************************************
*                                                                           *
*                      loadtable() - Load the Tables                       *
*                                                                           *
****************************************************************************/

static void loadtable()
{
	unsigned char *x;
	struct bridge *br;
	struct port *p;
	//struct timeval *tv;
	struct fdb_entry fdb[1024];
	

	int offset;
	int i;
	//char *pstr;
	char brName[20];
	//char tmp[6];
	
	br_init();
	flushLists();
	initStructs();		
	
	sprintf(brName, "br%d", 0);	
	br = br_find_bridge(brName);
	if(br == NULL) return;
	
	p = br->firstport;
	if(p == NULL) return;	
	
//var-dot1dBridge
	ndb.stpbridgehellotime= (int) br->info.bridge_hello_time.tv_sec;	
	ndb.agingtime= (int) br->info.ageing_time.tv_sec;
	ndb.stpmaxage= (int) br->info.max_age.tv_sec;

	x=(unsigned char *) &(br->info.designated_root);	
	memset(ndb.stpdesignatedroot,0,sizeof(ndb.stpdesignatedroot[40]));
	sprintf(ndb.stpdesignatedroot,"%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x", x[0], x[1], x[2], x[3],
		   x[4], x[5], x[6], x[7]);	

	ndb.stpbridgeforwarddelay= (int) br->info.bridge_forward_delay.tv_sec;
	ndb.stopforwarddelay= (int) br->info.forward_delay.tv_sec;
	ndb.stpholdtime= (int) p->info.hold_timer_value.tv_sec;
	ndb.basetype= 2;
	//ndb.stppriority=
	ndb.stpbridgemaxage= (int) br->info.bridge_max_age.tv_sec;
	ndb.stprootcost=br->info.root_path_cost;
	ndb.stphellotime=(int) br->info.hello_time.tv_sec;
	for(i=0;p!=NULL;i++)p=p->next;
	ndb.basenumport=i;
	ndb.stprootport= br->info.root_port;
	ndb.stpprotocolspecification=3;
	//ndb.stptimesincetopologychange
	//ndb.tplearnedentrydiscards
	
	
	
//StpPortTable
	p = br->firstport;
	for(i=0;p!=NULL;p=p->next,i++)
	{	
		nsp.ifIndex=i+1;
		nsp.stpport= p->index;
		nsp.stpportpriority=0;	
		nsp.stpportstate= p->info.state;	
		nsp.stpportenable= (int) br->info.stp_enabled;	
		nsp.stpportpathcost= p->info.path_cost;
		x=(unsigned char *) &(p->info.designated_root);
		memset(nsp.stpportdesignatedroot,0,sizeof(nsp.stpportdesignatedroot[60]));
		sprintf(nsp.stpportdesignatedroot,"%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x", x[0], x[1], x[2], x[3],
		   x[4], x[5], x[6], x[7]);
	
		nsp.stpportdesignatedcost= p->info.designated_cost;
	
		x=(unsigned char *) &(p->info.designated_bridge);
		memset(nsp.stpportdesignatedbridge,0,sizeof(nsp.stpportdesignatedbridge[60]));
		sprintf(nsp.stpportdesignatedbridge,"%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x", x[0], x[1], x[2], x[3],
		   x[4], x[5], x[6], x[7]);		
	
		memset(nsp.stpportdesignatedport,0,sizeof(nsp.stpportdesignatedport[2]));
		memcpy(nsp.stpportdesignatedport,&p->info.designated_port,2);
		nsp.stpportforwardtransitions=0;
		sprintf ( nsp.UID, "%04ld", nsp.ifIndex );	
		addList (( char * ) &spList, ( char * ) &nsp, sizeof ( nsp ));
    }
    
	
//TpFdbTableS	
	offset = 0;	  
	while (1) 
	{	  
	 	int i;
	  	int num;
	  	num = br_read_fdb(br, fdb, offset, 1024);	  
	  	if (!num) break;	  	
	  	qsort(fdb, num, sizeof(struct fdb_entry), compare_fdbs);
	  	for (i = 0; i < num; i++)
  	  	{
			ntf.ifIndex=i+1;
			ntf.tpfdbport=(fdb + i)->port_no;
			sprintf(ntf.tpfdbaddress,"%.2x%.2x%.2x%.2x%.2x%.2x",
		   	(fdb + i)->mac_addr[0], (fdb + i)->mac_addr[1], (fdb + i)->mac_addr[2],
		   	(fdb + i)->mac_addr[3], (fdb + i)->mac_addr[4], (fdb + i)->mac_addr[5]);
	  		sprintf ( ntf.UID, "%04ld", ntf.ifIndex );
	  		addList (( char * ) &tfList, ( char * ) &ntf, sizeof ( ntf ));
	  	}				
	  	offset += num;		
	 }
	  //ntf.tpfdbstatus=

	
	
//BasePortTable
	p = br->firstport;
	for(i=0;p!=NULL;p=p->next,i++)
	{
		nbp.ifIndex=i+1;
		nbp.baseport=p->index;			
		nbp.baseportfindex=p->ifindex;
		nbp.baseportdelayexceededdiscards=0;
		nbp.baseportmtuexceededdiscards=0;
		sprintf ( nbp.UID, "%04ld%04ld", nbp.ifIndex, nbp.baseportfindex );
		addList (( char * ) &bpList, ( char * ) &nbp, sizeof ( nbp ));
	}

	
//TpPortTable
	p = br->firstport;
	for(i=0;p!=NULL;p=p->next,i++)
	{
	ntp.tpport=p->index;
	if ( hasChanged (( char * ) &ntp, sizeof ( ntp ))) 
	{  	
		ntp.ifIndex=ntp.tpport;
		sprintf ( ntp.UID, "%04ld", ntp.ifIndex );
    	addList (( char * ) &tpList, ( char * ) &ntp, sizeof ( ntp ));
    }
	}
	
}
