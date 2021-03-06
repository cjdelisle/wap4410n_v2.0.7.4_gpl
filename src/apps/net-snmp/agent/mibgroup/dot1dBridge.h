/*
 * Note: this file originally auto-generated by mib2c using
 *        : mib2c.old-api.conf,v 1.3 2002/10/17 09:40:46 dts12 Exp $
 */
#ifndef DOT1DBRIDGE_H
#define DOT1DBRIDGE_H


#define BP_UID_LEN  8
#define SP_UID_LEN  4
#define TP_UID_LEN  4
#define TF_UID_LEN  20


/****************************************************************************
*                               Includes                                    *
****************************************************************************/

#include <sys/queue.h>

/****************************************************************************
*                             Linked List Defines                           *
****************************************************************************/
// here are some Linked List MACROS I wanted to use, 
// but curiously were not in /usr/includes/sys/queue.h

#ifndef LIST_EMPTY
  #define	LIST_EMPTY(head)	((head)->lh_first == NULL)
#endif

#ifndef LIST_NEXT
  #define	LIST_NEXT(elm, field)	((elm)->field.le_next)
#endif

#ifndef LIST_INSERT_BEFORE
  #define	LIST_INSERT_BEFORE(listelm, elm, field) do {			\
	  (elm)->field.le_prev = (listelm)->field.le_prev;		\
	  LIST_NEXT((elm), field) = (listelm);				\
	  *(listelm)->field.le_prev = (elm);				\
	  (listelm)->field.le_prev = &LIST_NEXT((elm), field);		\
  } while (0)
#endif

#ifndef LIST_FIRST
  #define	LIST_FIRST(head)	((head)->lh_first)
#endif

/****************************************************************************
*                           Linked List Structure                           *
****************************************************************************/
static struct awNode {  
  LIST_ENTRY ( awNode ) nodes; 
  char *data;                                 // pointer to data
};

typedef LIST_HEAD ( , awNode ) awList_t;

/****************************************************************************
*                          bridge MIB structures                            *
****************************************************************************/
static struct dotbridge_data {

  long  ifIndex;                                   // ifindex of card 
  
  long  stpbridgehellotime;
  long  agingtime;
  long  stpmaxage;
  char  stpdesignatedroot[40];
  long  stptopchanges;
  long  stpbridgeforwarddelay;
  long  stopforwarddelay;
  long  stpholdtime;
  long  basetype;
  long  stppriority;
  long  stpbridgemaxage;
  long  stprootcost;  
  long  stphellotime;
  long  basenumport;
  long  stprootport;
  char  basebridgeadress[20];
  long  stpprotocolspecification;
  long  stptimesincetopologychange;
  long  tplearnedentrydiscards; 
    
} ndb, *db = &ndb;


/****************************************************************************
*                             dot1dBase Group                                *
****************************************************************************/
/****************************************************************************
*                          dot1dBasePortTable                                *
****************************************************************************/
static struct bpTbl_data {

  char  UID       [ BP_UID_LEN + 1 ];
  long  ifIndex;                                    // ifindex of card

  long  baseport;
  long  baseportfindex;
  char  baseportcircuit[20];
  long  baseportdelayexceededdiscards;
  long  baseportmtuexceededdiscards;
  
    
} nbp, *bp = &nbp;

static awList_t bpList;

/****************************************************************************
*                             dot1dStp Group                                *
****************************************************************************/
/****************************************************************************
*                          dot1dStpPortTable                                *
****************************************************************************/
static struct spTbl_data {

  
  char  UID       [ SP_UID_LEN + 1 ];
  long  ifIndex;                                    // ifindex of card

  long  stpport;
  long  stpportpriority;
  long  stpportstate;
  long  stpportenable;
  long  stpportpathcost;
  char  stpportdesignatedroot[60];
  long  stpportdesignatedcost;
  char  stpportdesignatedbridge[60];
  char  stpportdesignatedport[2];
  long  stpportforwardtransitions;
    
} nsp, *sp = &nsp;

static awList_t spList;

/****************************************************************************
*                             dot1dTp Group                                 *
****************************************************************************/
/****************************************************************************
*                          dot1dTpFdbTable                                  *
****************************************************************************/
static struct tfTbl_data {

  char  UID       [ TF_UID_LEN + 1 ];
  long  ifIndex;                                    // ifindex of card

  char  tpfdbaddress[20];
  long  tpfdbport;  
  long  tpfdbstatus;
    
} ntf, *tf = &ntf;

static awList_t tfList;

/****************************************************************************
*                          dot1dTpPortTable                                *
****************************************************************************/
static struct tpTbl_data {

  char  UID       [ TP_UID_LEN + 1 ];  
  long  ifIndex;                                    // ifindex of card

  long  tpport;
  long  tpportmaxinfo;  
  long  tpportinframes;
  long  tpportoutframes;
  long  tpportindiscards;
    
} ntp, *tp = &ntp;

static awList_t tpList;


/*
 * function declarations 
 */
void            init_dot1dBridge(void);
FindVarMethod   var_dot1dBridge;
FindVarMethod   var_dot1dStpPortTable;
FindVarMethod   var_dot1dTpFdbTable;
FindVarMethod   var_dot1dBasePortTable;
FindVarMethod   var_dot1dTpPortTable;
FindVarMethod   var_dot1dStaticTable;
WriteMethod     write_dot1dStpBridgeHelloTime;
WriteMethod     write_dot1dTpAgingTime;
WriteMethod     write_dot1dStpBridgeForwardDelay;
WriteMethod     write_dot1dStpPriority;
WriteMethod     write_dot1dStpBridgeMaxAge;
WriteMethod     write_dot1dStpPortPriority;
WriteMethod     write_dot1dStpPortEnable;
WriteMethod     write_dot1dStpPortPathCost;
WriteMethod     write_dot1dStaticAddress;
WriteMethod     write_dot1dStaticReceivePort;
WriteMethod     write_dot1dStaticAllowedToGoTo;
WriteMethod     write_dot1dStaticStatus;

#endif                          /* DOT1DBRIDGE_H */
