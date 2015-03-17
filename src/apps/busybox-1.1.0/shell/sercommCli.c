/* 
 *  Linux CLI solution, porting from Atheros VxWorks SDK.
 *                                          Terry_Yang@SDC.SerComm.com
 */
#include "../../include/apcfg.h"
#include "stdio.h"
#include "stdlib.h"

#define MAX_TOKEN       50
#define CLI_INBUF       512
#define MAX_SCREEN_LINE         20

#define CLI_SUPERUSER   "superuser"
#define CLI_HIDDENCMD   "."
#define CLI_TOKEN_NULL_STRING           "\"\""

#define CLI_PARSE_OK                    0            
#define CLI_PARSE_ERROR                 -1
#define CLI_PARSE_NO_TBL                1
#define CLI_PARSE_NO_VALUE              2
#define CLI_PARSE_TOO_FEW               3
#define CLI_PARSE_TOO_MANY              4
#define CLI_PARSE_UNKNOWN               5
#define CLI_PARSE_INPUT_ERROR           6
#define CLI_PARSE_INVALID_PARAMETER     7
#define CLI_PARSE_QUIT                  9999

#define PARSE_TOKEN_DELIMITER { "", NULL, NULL, NULL, NULL, 0 }

typedef struct cli_s {
    struct parse_token_s *ParseTbl[MAX_TOKEN];
    int     securityCheck;
    int     autoLogOff;
    int     unit;
    int     vap;
    char    *token[MAX_TOKEN];
    char    *pToken;
    char    *pToken0;
    int     tokenIdx;
    int     tokenLvl;
    int     token_count;
    int     parseTblIdx;
    int     keyLast;
    char    ibuf[CLI_INBUF];
}CLI;

struct parse_token_s {
    char    *pCmd;
    char    *pHelpText;
    int     (*fHandler)(struct cli_s *, char *, struct parse_token_s *);
    struct  parse_token_s *pNxtParseTbl;
    char    *pProtected;
    int     opMode;               /* Wireless mode */
    int     abbrevMatchCnt;
};

char prompt[64];
static CLI *gpCli;
static struct parse_token_s *pCurTokenTbl;
static long lineCounter = 0;
static char txtDisable[] = "disable";
static char txtEnable[]  = "enable";

static const char *pAbleStr[] = {
    "disable",
    "enable"
};

static const char *pSecurityStr[] = {
    "None",
    "WEP",
    "WPA-PSK",
    "WPA2-PSK",
    "WPA-PSK and WPA2-PSK",
    "WPA with RADIUS",
    "WPA2 with RADIUS",
    "WPA and WPA2 with RADIUS",
    "802.1x"
};
static const char *pAuthStr[] = {
    "Automatic",
    "Open System",
    "Shared Key"
};
static int   genericCmdHandler(CLI *, char *, struct parse_token_s *);
static int   genericCliCmdSet(CLI *, char *, struct parse_token_s *);

static int   sccli_helpCmdHandler(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   factoryCmdHandler(CLI *, char *, struct parse_token_s *);
static int   applyCmdHandler(CLI *, char *, struct parse_token_s *);
static int   configCliCmdVap(CLI *, char *, struct parse_token_s *);
static void  sccli_exitCmdHandler(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int dot11nGuardIntervalCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int dot11nGuardIntervalCmdAuto(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int dot11nGuardIntervalCmdShort(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int dot11nGuardIntervalCmdLong(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int dot11nSubChannelCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int dot11nSubChannelCmdBelow(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int dot11nSubChannelCmdAbove(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int dot11nRadioBandCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int dot11nRadioBandCmdStandard(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int dot11nRadioBandCmdAuto(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int dot11nRadioBandCmdWide(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int dot11dModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int dot11dModeCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int dot11dModeCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   dot11dModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   dot11dModeCmdEnable(CLI *, char *, struct parse_token_s *);
static int   dot11dModeCmdDisable(CLI *, char *, struct parse_token_s *);
static int 	 acctSrvCliCmdGetPri(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int 	 acctSrvCliCmdSetPri(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int 	 acctPortCliCmdGetPri(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int 	 acctPortCliCmdSetPri(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int 	 acctSecretCliCmdGetPri(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   acctSecretCliCmdSetPri(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   acctSrvCliCmdGetBck(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   acctSrvCliCmdSetBck(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   acctPortCliCmdGetBck(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   acctPortCliCmdSetBck(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int 	 acctSecretCliCmdGetBck(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int 	 acctSecretCliCmdSetBck(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   aclCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   aclCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   aclCliCmdLocal(CLI *, char *, struct parse_token_s *);
static int   aclCliCmdRadius(CLI *, char *, struct parse_token_s *);
static int   aclCliCmdAdd(CLI *, char *, struct parse_token_s *);
static int   aclCliCmdDel(CLI *, char *, struct parse_token_s *);
static int   activeModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   activeModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   activeModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   agingCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   agingCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   sysCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   sysCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   descCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   descCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   versionCmdGet(CLI *, char *, struct parse_token_s *);
static int   todCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   uptimeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   countryCodeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   countryCodeCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   dhcpModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   dhcpCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   dhcpCliCmdClient(CLI *, char *, struct parse_token_s *);
static int   dhcpCliCmdServer(CLI *, char *, struct parse_token_s *);
static int 	 dhcpsEndIpCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int 	 dhcpsEndIpCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int 	 dhcpsStartIpCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int 	 dhcpsStartIpCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   ipAddrCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   ipAddrCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   ipMaskCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   ipMaskCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   gatewayCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   gatewayCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   nameSrvCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   nameSrvCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   ntpServerCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   ntpServerCliCmdGet(CLI *, char *, struct parse_token_s *);
/* MD@CPU_AP add at 20080121 */

static int   winsServerCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   winsServerCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   autochanCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   autochanCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   autochanCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   radioModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   radioModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   radioModeCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   wirelessModeCliCmd11b(CLI *, char *, struct parse_token_s *);
static int   wirelessModeCliCmd11bg(CLI *, char *, struct parse_token_s *);
//static int   wirelessModeCliCmd11g(CLI *, char *, struct parse_token_s *);
static int   opModeCliCmdUc(CLI *, char *, struct parse_token_s *);
static int   opModeCliCmdUr(CLI *, char *, struct parse_token_s *);
static int	 remotessidCliCmdGet(CLI *, char *, struct parse_token_s *);
static int	 remotessidCliCmdSet(CLI *, char *, struct parse_token_s *);
static int	 remoteUcrMacAddrCliCmdGet(CLI *, char *, struct parse_token_s *);
static int	 remoteUcrMacAddrCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   snmpContactCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpContactCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpDeviceNameCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpDeviceNameCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpLocationCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpLocationCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpTrapComCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpTrapComCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   emailAlertCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   emailAlertCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   emailAlertCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   emailServerCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   emailServerCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   emailAddressCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   emailAddressCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   emailQueueLengthCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   emailQueueLengthCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   emailSendPeriodCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   emailSendPeriodCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   loginSuccessCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   loginSuccessCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   loginSuccessCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   loginFailCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   loginFailCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   loginFailCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   sysErrorCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   sysErrorCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   sysErrorCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   confChangeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   confChangeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   confChangeCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   httpRDModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   httpRDModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   httpRDModeCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   httpURLCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   httpURLCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   aclLocalModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   aclLocalModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   aclLocalModeCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   magvlanCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   magvlanCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   defaultvlanCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   defaultvlanCliCmdSet(CLI *, char *, struct parse_token_s *);



#define AP101NA_LINKSYS
//#ifdef AP101NA_LINKSYS
static int   hostnameCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   hostnameCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   devicenameCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   devicenameCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   nameSrv2CliCmdGet(CLI *, char *, struct parse_token_s *);
static int   nameSrv2CliCmdSet(CLI *, char *, struct parse_token_s *);
static int   ipv6ModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   ipv6ModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   ipv6ModeCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   ipv6AddrCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   ipv6AddrCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   ipv6gatewayCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   ipv6gatewayCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   timeModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   timeModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   timeModeCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   ntpModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   ntpModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   ntpModeCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   wlanAccessModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   wlanAccessModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   wlanAccessModeCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   sshModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   sshModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   sshModeCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   ctsProtectModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   ctsProtectModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   ctsProtectModeCliCmdAuto(CLI *, char *, struct parse_token_s *);
static int   loadBalanceModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   loadBalanceModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   loadBalanceModeCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   loadBalanceSSIDCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   loadBalanceSSIDCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   vlanTagModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   vlanTagModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   vlanTagModeCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   vlanWDSTagModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   vlanWDSTagModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   vlanWDSTagModeCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   priorityCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   priorityCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   tocCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   todCliCmdSet(CLI *, char *, struct parse_token_s *);
//#endif



/* add end */
static int   dot1xSuppCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   dot1xSuppCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   dot1xSuppCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   dot1xSuppTypeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   dot1xSuppTypeCliCmdUser(CLI *, char *, struct parse_token_s *);
static int   dot1xSuppTypeCliCmdMac(CLI *, char *, struct parse_token_s *);
static int   dot1xSuppUserCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   dot1xSuppUserCliCmdSet(CLI *pCli, char *, struct parse_token_s *);
static int   dot1xSuppPassCliCmdGet(CLI *pCli, char *, struct parse_token_s *);
static int   dot1xSuppPassCliCmdSet(CLI *pCli, char *, struct parse_token_s *);
static int   vlanModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   vlanModeCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   vlanModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   nativeVlanIdCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   nativeVlanIdCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   vlanPvidCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   vlanPvidCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   wirelessModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   wirelessModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   wirelessModeCliCmd11bgn(CLI *, char *, struct parse_token_s *);
static int   wirelessModeCliCmd11gn(CLI *, char *, struct parse_token_s *);
static int   wirelessModeCliCmd11n(CLI *, char *, struct parse_token_s *);

static int   opModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   opModeCliCmdAp(CLI *, char *, struct parse_token_s *);
static int   opModeCliCmdPpt(CLI *, char *, struct parse_token_s *);
static int   opModeCliCmdMpt(CLI *, char *, struct parse_token_s *);
static int   opModeCliCmdApPtp(CLI *, char *, struct parse_token_s *);
static int   opModeCliCmdApPtmp(CLI *, char *, struct parse_token_s *);
static int   remotePtpMacAddrCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   remotePtpMacAddrCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   remotePtmpMacListCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   remotePtmpMacListCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   chanCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   chanCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   sepCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   sepCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   sepCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   wmeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   wmeCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   wmeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int wmeNoAckCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int wmeNoAckCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int wmeNoAckCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   fragCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   fragCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   beaconCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   beaconCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   rtsCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   rtsCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   shortPreambleCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   shortPreambleCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   shortPreambleCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   interVapForwardingCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   interVapForwardingCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   interVapForwardingCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   vapNameCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   vapNameCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   ssidCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   ssidCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   ssidModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   ssidModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   ssidModeCliCmdEnable(CLI *, char *, struct parse_token_s *);

static int stpModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int stpModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int stpModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int lltdModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int lltdModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int lltdModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   secCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   secCliCmdNone(CLI *, char *, struct parse_token_s *);
static int   secCliCmdWpapsk(CLI *, char *, struct parse_token_s *);
static int   secCliCmdWpa2psk(CLI *, char *, struct parse_token_s *);
static int   secCliCmdWpapskauto(CLI *, char *, struct parse_token_s *);
static int   secCliCmdWpa(CLI *, char *, struct parse_token_s *);
static int   secCliCmdWpa2(CLI *, char *, struct parse_token_s *);
static int   secCliCmdWpaauto(CLI *, char *, struct parse_token_s *);
static int   authCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   authCliCmdOpen(CLI *, char *, struct parse_token_s *);
static int   authCliCmdShared(CLI *, char *, struct parse_token_s *);
static int   authCliCmdAuto(CLI *, char *, struct parse_token_s *);
static int   keyLengthCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   keyLengthCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   defaultKeyCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   defaultKeyCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   keyCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   keyCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   pskCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   pskCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   groupKeyUpdateCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   groupKeyCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   groupKeyCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   groupKeyUpdateIntervalCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   groupKeyUpdateIntervalCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   gtkUpdateStrictCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   gtkUpdateStrictCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   gtkUpdateStrictCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int radiusSrvCliCmdGetPri(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int radiusSrvCliCmdSetPri(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int radiusSrvCliCmdGetBck(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int radiusSrvCliCmdSetBck(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int radiusPortCliCmdGetPri(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int radiusPortCliCmdSetPri(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int radiusPortCliCmdGetBck(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int radiusPortCliCmdSetBck(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int radiusSecretCliCmdGetPri(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int radiusSecretCliCmdSetPri(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int radiusSecretCliCmdGetBck(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int radiusSecretCliCmdSetBck(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   dot1xKeyCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   dot1xKeyCliCmdDyn(CLI *, char *, struct parse_token_s *);
static int   dot1xKeyCliCmdSta(CLI *, char *, struct parse_token_s *);
static int   dot1xKeyUpdateCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   dot1xKeyUpdateCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   dot1xKeyUpdateCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   dot1xKeyLifeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   dot1xKeyLifeCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   usernameCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   usernameCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   passwordCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   passwordCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   httpCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   httpsCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   httpCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   httpCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   httpsCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   httpsCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   httpPortCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   httpsPortCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   httpPortCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   httpsPortCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   telnetCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   telnetCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   telnetCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int timezoneCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int timezoneCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpReadComCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpReadComCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpWriteComCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpWriteComCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpManageCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpManageCliCmdAny(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpManageCliCmdIpStart(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpManageCliCmdIpEnd(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpTrapCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpTrapCliCmdBroadcast(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpTrapCliCmdUnicast(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpTrapVerCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpTrapVerCliCmdV1(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpTrapVerCliCmdV2c(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   snmpTrapVerCliCmdV3(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int snmpV3UserNameCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int snmpV3UserNameCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int snmpV3AuthProtoCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int snmpV3AuthProtoCliCmdNone(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int snmpV3AuthProtoCliCmdMd5(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int snmpV3AuthKeyCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int snmpV3AuthKeyCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int snmpV3PrivProtoCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int snmpV3PrivProtoCliCmdNone(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int snmpV3PrivProtoCliCmdDes(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int snmpV3PrivKeyCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int snmpV3PrivKeyCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   syslogCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   syslogCliCmdDisable(CLI *, char *, struct parse_token_s *);
#ifdef AP101NA_LINKSYS
static int   syslogCliCmdEnable(CLI *, char *, struct parse_token_s *);
#endif
static int   syslogCliCmdBroadcast(CLI *, char *, struct parse_token_s *);
static int   syslogCliCmdUnicast(CLI *, char *, struct parse_token_s *);
static int   syslogPortCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   syslogPortCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   syslogSeverityCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   syslogSeverityCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   syslogSrvCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   syslogSrvCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   rogueCmdGet(CLI *, char *, struct parse_token_s *);
static int   roguedetectCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   roguedetectCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   rogueIntervalCmdGet(CLI *, char *, struct parse_token_s *);
static int   rogueIntervalCmdSet(CLI *, char *, struct parse_token_s *);  
static int   roguetypeCmdGet(CLI *, char *, struct parse_token_s *);
static int   roguetype1CliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   roguetype2CliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   roguetype3CliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   roguesnmpCmdGet(CLI *, char *, struct parse_token_s *);
static int   roguesnmpCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   roguesnmpCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   roguelegalCmdGet(CLI *, char *, struct parse_token_s *);
static int   roguelegalCmdAdd(CLI *, char *, struct parse_token_s *);
static int   roguelegalCmdDel(CLI *, char *, struct parse_token_s *);


static struct parse_token_s configCmdTbl[] =
{
    {"vap",       "Config Virtual AP X",                        configCliCmdVap,        NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s acctSrvCliCmdGetTbl[] = {
    {"primary",         "",                     acctSrvCliCmdGetPri,          NULL,           NULL,           0},
    {"backup",          "",                     acctSrvCliCmdGetBck,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s acctSrvCliCmdSetTbl[] = {
    {"primary",         "",                     acctSrvCliCmdSetPri,          NULL,           NULL,           0},
    {"backup",          "",                     acctSrvCliCmdSetBck,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s acctPortCliCmdGetTbl[] = {
    {"primary",         "",                     acctPortCliCmdGetPri,          NULL,           NULL,           0},
    {"backup",          "",                     acctPortCliCmdGetBck,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s acctPortCliCmdSetTbl[] = {
    {"primary",         "",                     acctPortCliCmdSetPri,          NULL,           NULL,           0},
    {"backup",          "",                     acctPortCliCmdSetBck,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s acctSecretCliCmdGetTbl[] = {
    {"primary",         "",                     acctSecretCliCmdGetPri,          NULL,           NULL,           0},
    {"backup",          "",                     acctSecretCliCmdGetBck,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s acctSecretCliCmdSetTbl[] = {
    {"primary",         "",                     acctSecretCliCmdSetPri,          NULL,           NULL,           0},
    {"backup",          "",                     acctSecretCliCmdSetBck,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s aclCmdTbl[] = {
    {txtDisable,        "Disable Access Control",               aclCliCmdDisable,       NULL,           NULL,           0},
    {"local",           "Enable Local Access Control",          aclCliCmdLocal,         NULL,           NULL,           0},
    {"radius",          "Enable Radius Access Control",         aclCliCmdRadius,        NULL,           NULL,           0},
    {"add",             "Add a Trusted MAC Address",            aclCliCmdAdd,           NULL,           NULL,           0},
    {"del",             "Delete a Trusted MAC Address",         aclCliCmdDel,           NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s activeModeCmdTbl[] = {
    {txtEnable,         "Active Current VAP",                   activeModeCliCmdEnable, NULL,           NULL,           0},
    {txtDisable,        "Disable Current VAP",                  activeModeCliCmdDisable,NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s dhcpCmdTbl[] = {
#ifndef AP101NA_LINKSYS
    {"disable",         "Set DHCP Disable",                     dhcpCliCmdDisable,      NULL,           NULL,           0},
    {"client",          "Set DHCP Client",                      dhcpCliCmdClient,       NULL,           NULL,           0},
    {"server",          "Set DHCP Server",                      dhcpCliCmdServer,       NULL,           NULL,           0},
#else
    {"static",         	"Set DHCP Disable",                     dhcpCliCmdDisable,      NULL,           NULL,           0},
    {"automatic",      	"Set DHCP Client",                      dhcpCliCmdClient,       NULL,           NULL,           0},
#endif
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s dot1xSuppCmdTbl[] = {
    {txtDisable,        "Disable 802.1x Client on Ethernet",    dot1xSuppCliCmdDisable, NULL,           NULL,           0},
    {txtEnable,         "Enable 802.1x Client on Ethernet",     dot1xSuppCliCmdEnable,  NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s dot1xSuppTypeCmdTbl[] = {
    {"user",            "Use Manual Username/Password",         dot1xSuppTypeCliCmdUser,NULL,         NULL,           0},
    {"mac",             "Use MAC Address As Username/Password", dot1xSuppTypeCliCmdMac,NULL,          NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s vlanModeCmdTbl[] = {
    {txtDisable,        "Disable VLAN Operation",               vlanModeCliCmdDisable,  NULL,           NULL,           0},
    {txtEnable,         "Enable VLAN Operation",                vlanModeCliCmdEnable,   NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};


static struct parse_token_s wirelessModeCmdTbl[] = {
#ifdef AP101NA_LINKSYS
    {"disable",         "Disable Current Radio",                wirelessModeCliCmdDisable,  NULL,       NULL,           0},
#endif
    {"11b",             "802.11b only",    						wirelessModeCliCmd11b,    	NULL,       NULL,  			0}, 
  //{"11g",           	"802.11g only",            				wirelessModeCliCmd11g,     	NULL,       NULL,  			0},
    {"11bg",            "802.11b and 802.11g",   				wirelessModeCliCmd11bg,		NULL,       NULL,  			0},   
	{"11nbg",           "Mixed 802.11n,802.11b and 802.11g",    wirelessModeCliCmd11bgn,    NULL,       NULL,           0}, 
#ifndef AP101NA_LINKSYS
	{"11ng",            "802.11n and 802.11g",            		wirelessModeCliCmd11gn,     NULL,       NULL,  			0},//CLI_HIDDENCMD
	{"11n",             "802.11n only",                         wirelessModeCliCmd11n,      NULL,       NULL,  			0},
#endif
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s opModeCmdTbl[] = {
    {"ap",              "Operating as Access Point",                   	opModeCliCmdAp,         NULL,           NULL,           0},
    {"ptp",             "Operating as Bridge(Point-to-Point)",         	opModeCliCmdPpt,        NULL,           NULL,           0},
    {"ptmp",            "Operating as Bridge(Multi-point)",           	opModeCliCmdMpt,        NULL,           NULL,           0},
    {"apPtp",           "Operating as Bridge(Point-to-Point)+ Access Point",    opModeCliCmdApPtp,      NULL,   NULL,       	0},
    {"apPtmp",          "Operating as Bridge(Multi-point) + Access Point",      opModeCliCmdApPtmp,     NULL,   NULL,       	0},
    {"uc",            	"Operating as Bridge(Unversal Client)",     	opModeCliCmdUc,       	NULL,           NULL,           0},
    {"ur",           	"Operating as Bridge(Unversal Repeater)",    	opModeCliCmdUr,     	NULL,       NULL,           	0},
  //{"rogueAp",       	"Operating as Rogue AP",      					opModeCliCmdRogueAp,  	NULL,       NULL,           	0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s radiusSrvCliCmdGetTbl[] = {
    {"primary",         "",                     radiusSrvCliCmdGetPri,          NULL,           NULL,           0},
    {"backup",          "",                     radiusSrvCliCmdGetBck,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s radiusSrvCliCmdSetTbl[] = {
    {"primary",         "",                     radiusSrvCliCmdSetPri,          NULL,           NULL,           0},
    {"backup",          "",                     radiusSrvCliCmdSetBck,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};


static struct parse_token_s radiusPortCliCmdGetTbl[] = {
    {"primary",         "",                     radiusPortCliCmdGetPri,          NULL,           NULL,           0},
    {"backup",          "",                     radiusPortCliCmdGetBck,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s radiusPortCliCmdSetTbl[] = {
    {"primary",         "",                     radiusPortCliCmdSetPri,          NULL,           NULL,           0},
    {"backup",          "",                     radiusPortCliCmdSetBck,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s radiusSecretCliCmdGetTbl[] = {
    {"primary",         "",                     radiusSecretCliCmdGetPri,          NULL,           NULL,           0},
    {"backup",          "",                     radiusSecretCliCmdGetBck,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s radiusSecretCliCmdSetTbl[] = {
    {"primary",         "",                     radiusSecretCliCmdSetPri,          NULL,           NULL,           0},
    {"backup",          "",                     radiusSecretCliCmdSetBck,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s remotePtmpMacListCmdTbl[] = {
    {"1",         "Remote Mac Address 1",                     remotePtmpMacListCliCmdSet,          NULL,           NULL,           0},
    {"2",         "Remote Mac Address 2",                     remotePtmpMacListCliCmdSet,          NULL,           NULL,           0},
    {"3",         "Remote Mac Address 3",                     remotePtmpMacListCliCmdSet,          NULL,           NULL,           0},
    {"4",         "Remote Mac Address 4",                     remotePtmpMacListCliCmdSet,          NULL,           NULL,           0},
#ifndef AP101NA_LINKSYS
    {"5",         "Remote Mac Address 5",                     remotePtmpMacListCliCmdSet,          NULL,           NULL,           0},
    {"6",         "Remote Mac Address 6",                     remotePtmpMacListCliCmdSet,          NULL,           NULL,           0},
    {"7",         "Remote Mac Address 7",                     remotePtmpMacListCliCmdSet,          NULL,           NULL,           0},
    {"8",         "Remote Mac Address 8",                     remotePtmpMacListCliCmdSet,          NULL,           NULL,           0},
#endif
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s sepCmdTbl[] = {
    {txtDisable,        "Disable Isolation within SSID",        sepCliCmdDisable,        NULL,           NULL,           0},
    {txtEnable,         "Enable Isolation within SSID",         sepCliCmdEnable,         NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};      

static struct parse_token_s  dot11nGuardIntervalCmdTbl[] = {
	{"auto",         "Set Auto 11n Guard Interval",            	dot11nGuardIntervalCmdAuto,         NULL,           NULL,           0},
    {"short",        "Set Short 11n Guard Interval",            dot11nGuardIntervalCmdShort,        NULL,           NULL,           0},
    {"long",         "Set Long 11n Guard Interval",             dot11nGuardIntervalCmdLong,         NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
}; 

static struct parse_token_s  dot11nSubChannelCmdTbl[] = {
    {"below",        "Set 11n Sub-Channel to Below Primary Channel",    dot11nSubChannelCmdBelow,       NULL,           NULL,           0},
    {"above",        "Set 11n Sub-Channel to Above Primary Channel",    dot11nSubChannelCmdAbove,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
}; 

static struct parse_token_s  dot11nRadioBandCmdTbl[] = {
    {"standard",    "Set Channel Bandwidth to Standard-20MHz Channel",         dot11nRadioBandCmdStandard,     NULL,           NULL,           0},
    {"auto",        "Set Channel Bandwidth to Auto-20/40MHz Channel",          dot11nRadioBandCmdAuto,         NULL,           NULL,           0},
    {"wide",        "Set Channel Bandwidth to Wide-40MHz Channel",             dot11nRadioBandCmdWide,         NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};   

static struct parse_token_s  dot11dModeCmdTbl[] = {
    {txtDisable,        "Disable 802.11d Mode",                 dot11dModeCmdDisable,   NULL,           NULL,           0},
    {txtEnable,         "Enable 802.11d Mode",                  dot11dModeCmdEnable,    NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s wmeCmdTbl[] = {
    {"disable",         "Disable WMM",                          wmeCliCmdDisable,       NULL,           NULL,           0},
    {"enable",          "Enable WMM",                           wmeCliCmdEnable,        NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s wmeNoAckCmdTbl[] = {
    {"disable",         "Disable WMM No Acknowledgement",       wmeNoAckCliCmdDisable,       NULL,           NULL,           0},
    {"enable",          "Enable WMM No Acknowledgement",        wmeNoAckCliCmdEnable,        NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s shortPreambleCmdTbl[] = {
    {txtDisable,        "Disable Short Preamble (Use Only Long)",shortPreambleCliCmdDisable,NULL,       NULL,           MODE_SELECT_11B | MODE_SELECT_11G},
    {txtEnable,         "Enable Short and Long Preamble",       shortPreambleCliCmdEnable,NULL,         NULL,           MODE_SELECT_11B | MODE_SELECT_11G},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s interVapForwardingCmdTbl[] = {
    {txtDisable,        "Disable Isolate Between SSID",             interVapForwardingCliCmdDisable,NULL,   NULL,           0},
    {txtEnable,         "Enable Isolate Between SSID",              interVapForwardingCliCmdEnable,NULL,    NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s ssidModeCmdTbl[] = {
    {txtDisable,        "Disable SSID Broadcast",               ssidModeCliCmdDisable,  NULL,           NULL,           0},
    {txtEnable,         "Enable SSID Broadcast",                ssidModeCliCmdEnable,   NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s stpModeCmdTbl[] = {
    {txtDisable,        "Disable STP",                          stpModeCliCmdDisable,  NULL,           NULL,           0},
    {txtEnable,         "Enable STP",                           stpModeCliCmdEnable,   NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s lltdModeCmdTbl[] = {
    {txtDisable,        "Disable LLTD",                         lltdModeCliCmdDisable,  NULL,           NULL,           0},
    {txtEnable,         "Enable LLTD",                          lltdModeCliCmdEnable,   NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s authCmdTbl[] = {
    {"open",            "Select Open-System Authentication Type",authCliCmdOpen,        NULL,           NULL,           0},
    {"shared",          "Select Shared-Key Authentication Type",authCliCmdShared,       NULL,           NULL,           0},
#ifndef AP101NA_LINKSYS
    {"auto",            "Select auto Authentication Type",      authCliCmdAuto,         NULL,           NULL,           0},
#endif
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s dot1xKeyCmdTbl[] = {
    {"dynamic",         "Dynamic WEP key (EAP-TLS, PEAP etc)",  dot1xKeyCliCmdDyn,      NULL,           NULL,           0},
    {"static",          "Static WEP Key (EAP-MD5)",             dot1xKeyCliCmdSta,      NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s secCmdTbl[] = {
#ifndef AP101NA_LINKSYS
    {"none",            "None Security Mode",                   secCliCmdNone,          NULL,           NULL,           0},
    {"wep",             "WEP Mode(Open/Share)",                 genericCmdHandler,      authCmdTbl,     NULL,           0},
    {"wpapsk",          "WPA-PSK Mode",                         secCliCmdWpapsk,        NULL,           NULL,           0},
    {"wpa2psk",         "WPA2-PSK Mode",                        secCliCmdWpa2psk,       NULL,           NULL,           0},
    {"wpapskauto",      "Auto WPA/WPA2-PSK Mode",               secCliCmdWpapskauto,    NULL,           NULL,           0},
    {"wparadius",       "WPA With RADIUS Mode",                 secCliCmdWpa,           NULL,           NULL,           0},
    {"wpa2radius",      "WPA2 With RADIUS Mode",                secCliCmdWpa2,          NULL,           NULL,           0},
    {"wparadiusauto",   "Auto WPA/WPA2 With RADIUS Mode",       secCliCmdWpaauto,       NULL,           NULL,           0},
    {"802.1x",          "802.1x Mode",                          genericCmdHandler,       dot1xKeyCmdTbl, NULL,           0},
#else
    {"wep",             "WEP Mode(Open/Share)",                 genericCmdHandler,      authCmdTbl,     NULL,           0},
    {"wpapsk",          "WPA-Personal Mode",                    secCliCmdWpapsk,        NULL,           NULL,           0},
    {"wpa2psk",         "WPA2-Personal Mode",                   secCliCmdWpa2psk,       NULL,           NULL,           0},
    {"wpa2pskmixed",    "WPA/WPA2-Personal Mixed Mode",       	secCliCmdWpapskauto,    NULL,           NULL,           0},
    {"wparadius",       "WPA-Enterprise Mode",                 	secCliCmdWpa,           NULL,           NULL,           0},
    {"wpa2radius",      "WPA2-Enterprise Mode",                	secCliCmdWpa2,          NULL,           NULL,           0},
    {"wpa2radiusmixed", "WPA/WPA2-Enterprise Mixed Mode",    	secCliCmdWpaauto,       NULL,           NULL,           0},
    {"radius",          "RADIUS Mode",                          genericCmdHandler,    	dot1xKeyCmdTbl, NULL,           0},
	{"disable",         "None Security Mode",                   secCliCmdNone,          NULL,           NULL,           0},
#endif
    PARSE_TOKEN_DELIMITER
};
                
static struct parse_token_s countryCodeCmdTbl[] = {
    {"Asia",            "Country Code: 410",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Australia",       "Country Code: 36",                 countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Canada",          "Country Code: 124",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Denmark",         "Country Code: 208",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Europe",          "Country Code: 40",                 countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Finland",         "Country Code: 246",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"France",          "Country Code: 250",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Germany",         "Country Code: 276",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Ireland",         "Country Code: 372",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Italy",           "Country Code: 380",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Japan",           "Country Code: 392",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Mexico",          "Country Code: 484",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Netherlands",     "Country Code: 528",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"NewZealand",      "Country Code: 554",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Norway",          "Country Code: 578",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"PuertoRico",      "Country Code: 630",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"SouthAmerica",    "Country Code: 340",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Spain",           "Country Code: 724",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Sweden",          "Country Code: 752",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Switzerland",     "Country Code: 756",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"UnitedKingdom",   "Country Code: 826",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"UnitedStates",    "Country Code: 840",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};


static struct parse_token_s groupKeyCmdTbl[] = {
    {txtDisable,        "Disable Group Key Update",             groupKeyCliCmdDisable,  NULL,           NULL,           0},
    {txtEnable,         "Enable Group Key Update",              groupKeyCliCmdEnable,   NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s gtkUpdateStrictCmdTbl[] = {
    {txtDisable,        "Disable Update Group Key When Any Membership Terminates",gtkUpdateStrictCliCmdDisable,NULL,NULL,0},
    {txtEnable,         "Enable Update Group Key When Any Membership Terminates",gtkUpdateStrictCliCmdEnable,NULL,NULL,0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s dot1xKeyUpdateCmdTbl[] = {
    {txtDisable,        "Disable 802.1x Dynamic Key Update",    dot1xKeyUpdateCliCmdDisable,    NULL,       NULL,           0},
    {txtEnable,         "Enable 802.1x Dynamic Key Update",     dot1xKeyUpdateCliCmdEnable,     NULL,        NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s httpCmdTbl[] = {
    {txtDisable,        "Disable HTTP",                         httpCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Enable HTTP",                          httpCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s httpsCmdTbl[] = {
    {txtDisable,        "Disable HTTPS",                        httpsCliCmdDisable,     NULL,           NULL,           0},
    {txtEnable,         "Enable HTTPS",                         httpsCliCmdEnable,      NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

/* MD@CPU_AP add at 20080121 */
static struct parse_token_s autochanCmdTbl[] = {
    {txtDisable,        "Disable Auto Channel",                 autochanCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Enable Auto Channel",                  autochanCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s radiomodeCmdTbl[] = {
    {txtDisable,        "Disable Radio Turn on",                 radioModeCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Enable Radio Turn on",                  radioModeCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s emailalertCmdTbl[] = {
    {txtDisable,        "Disable Email Alert",                 emailAlertCliCmdDisable,    		NULL,           NULL,           0},
    {txtEnable,         "Enable Email Alert",                  emailAlertCliCmdEnable,       	NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s loginsuccessCmdTbl[] = {
    {txtDisable,        "Disable Unauthorized Login Attempt",  loginSuccessCliCmdDisable,      	NULL,           NULL,           0},
    {txtEnable,         "Enable Unauthorized Login Attempt",   loginSuccessCliCmdEnable,       	NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s loginfailCmdTbl[] = {
    {txtDisable,        "Disable Authorized Login",            loginFailCliCmdDisable,      	NULL,           NULL,           0},
    {txtEnable,         "Enable Authorized Login",             loginFailCliCmdEnable,       	NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s syserrorCmdTbl[] = {
    {txtDisable,        "Disable System Error Messages",       sysErrorCliCmdDisable,      		NULL,           NULL,           0},
    {txtEnable,         "Enable System Error Messages",        sysErrorCliCmdEnable,       		NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s confchangeCmdTbl[] = {
    {txtDisable,        "Disable Configuration Changes",       confChangeCliCmdDisable,      	NULL,           NULL,           0},
    {txtEnable,         "Enable Configuration Changes",        confChangeCliCmdEnable,       	NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s httpredirectmodeCmdTbl[] = {
    {txtDisable,        "Disable HTTP Redirct",                 httpRDModeCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Enable HTTP Redirct",                  httpRDModeCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s acllocalmodeCmdTbl[] = {
    {"allow",        "Allow only following MAC addresses to connect to wireless network",    aclLocalModeCliCmdDisable,      NULL,           NULL,           0},
    {"deny",         "Prevent following MAC addresses from connecting to wireless network",  aclLocalModeCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

#ifdef AP101NA_LINKSYS
static struct parse_token_s ipv6modeCmdTbl[] = {
    {txtDisable,        "Disable IPv6",                 		ipv6ModeCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Enable IPv6",                  		ipv6ModeCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s timemodeCmdTbl[] = {
    {"auto",        	"Disable Manual",                 		timeModeCliCmdDisable,      NULL,           NULL,           0},
    {"manual",         	"Enable Manual",                  		timeModeCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s ntpmodeCmdTbl[] = {
    {txtDisable,        "Disable NTP",                 			ntpModeCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Enable NTP",                  			ntpModeCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s wlanaccessmodeCmdTbl[] = {
    {txtDisable,        "Disable Wireless Web Access",          wlanAccessModeCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Enable Wireless Web Access",           wlanAccessModeCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s sshmodeCmdTbl[] = {
    {txtDisable,        "Disable SSH",                 			sshModeCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Enable SSH",                  			sshModeCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s ctsprotectmodeCmdTbl[] = {
    {txtDisable,        "Disable CTS Protection",    	ctsProtectModeCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Auto CTS Protection",  		ctsProtectModeCliCmdAuto,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s loadbalancemodeCmdTbl[] = {
    {txtDisable,        "Disable Load Balance",    		loadBalanceModeCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Enable Load Balance",  		loadBalanceModeCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s vlantagmodeCmdTbl[] = {
    {"untagged",        "Untagged Vlan Tag",    		vlanTagModeCliCmdDisable,      NULL,           NULL,           0},
    {"tagged",          "Tagged Vlan Tag",  			vlanTagModeCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s vlanWDStagmodeCmdTbl[] = {
    {txtDisable,        "Disable Vlan Tag",    			vlanWDSTagModeCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Enable Vlan Tag",  			vlanWDSTagModeCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};
static struct parse_token_s timesetCmdTbl[] = {
    {"date",    "Set Current System Dtae. Foramt : xxxx/xx/xx(year/month/day)",  todCliCmdSet,      NULL,           NULL,           0},
    {"clock",   "Set Current System Time. Foramt : xx/xx/xx(hour/minute/second)",	tocCliCmdSet,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};
#endif

/* add end */
static struct parse_token_s telnetCmdTbl[] = {
    {txtDisable,        "Disable Telnet access",                telnetCliCmdDisable,    NULL,           NULL,           0},
    {txtEnable,         "Enable Telnet access",                 telnetCliCmdEnable,     NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER,
};

static struct parse_token_s timezoneCmdTbl[] = {
    {"GMT-12",          "(GMT-12:00) Kwajalein",                timezoneCliCmdSet,      NULL,           NULL,           0},
    {"GMT-11",          "(GMT-11:00) Midway Island, Samoa",     timezoneCliCmdSet,      NULL,           NULL,           0},
    {"GMT-10",          "(GMT-10:00) Hawaii",                   timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT-9",           "(GMT-09:00) Alaska",                   timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT-8",           "(GMT-08:00) Pacific Time (USA &amp; Canada)",  timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT-7.1",         "(GMT-07:00) Arizona",                  timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT-7.2",         "(GMT-07:00) Mountain Time(USA &amp; Canada)",  timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT-6.1",         "(GMT-06:00) Mexico",                   timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT-6.2",         "(GMT-06:00) Central Time(USA &amp; Canada)",   timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT-5.1",         "(GMT-05:00) Indiana East, Colombia, Panama",   timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT-5.2",         "(GMT-05:00) Eastern Time(USA &amp; Canada)",   timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT-4.1",         "(GMT-04:00) Bolivia, Venezuela",       timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT-4.2",         "(GMT-04:00) Atlantic Time(Canada), Brazil West",   timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT-3.1",         "(GMT-03:00) Guyana",                   timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT-3.2",         "(GMT-03:00) Brazil East, Greenland",   timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT-2",           "(GMT-02:00) Mid-Atlantic",             timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT-1",           "(GMT-01:00) Azores",                   timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+0.1",         "(GMT) Gambia, Liberia, Morocco",       timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+0.2",         "(GMT) England",                         timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+1.1",         "(GMT+01:00) Tunisia",                  timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+1.2",         "(GMT+01:00) France, Germany, Italy",   timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+2.1",         "(GMT+02:00) South Africa",             timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+2.2",         "(GMT+02:00) Greece, Ukraine, Romania, Turkey",     timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+3",           "(GMT+03:00) Iraq, Jordan, Kuwait",     timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+4",           "(GMT+04:00) Armenia",                  timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+5",           "(GMT+05:00) Pakistan, Russia",         timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+6",           "(GMT+06:00) Bangladesh, Russia",       timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+7",           "(GMT+07:00) Thailand, Russia",         timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+8.1",         "(GMT+08:00) China, Hong Kong, Australia Western",  timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+8.2",         "(GMT+08:00) Singapore, Taiwan, Russia",timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+9",           "(GMT+09:00) Japan, Korea",             timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+10.1",        "(GMT+10:00) Guam, Russia",             timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+10.2",        "(GMT+10:00) Australia",                timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+11",          "(GMT+11:00) Solomon Islands",          timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+12.1",        "(GMT+12:00) Fiji",                     timezoneCliCmdSet,     NULL,           NULL,           0},
    {"GMT+12.2",        "(GMT+12:00) New Zealand",              timezoneCliCmdSet,     NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER,
};

static struct parse_token_s snmpModeCmdTbl[] = {
    {txtDisable,        "Disable SNMP",                         snmpCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Enable SNMP",                          snmpCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s snmpManageCmdTbl[] = {
    {"any",       		"Any Station",                  		snmpManageCliCmdAny,    NULL,           NULL,           0},
    {"ipstart",        	"from this Station",                 	snmpManageCliCmdIpStart,NULL,           NULL,           0},
	{"ipend",        	"to this Station",               	    snmpManageCliCmdIpEnd,  NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s snmpTrapModeCmdTbl[] = {
    {"broadcast",       "Broadcast the SNMP Trap",              snmpTrapCliCmdBroadcast,NULL,           NULL,           0},
    {"unicast",         "Unicast the SNMP Trap",                snmpTrapCliCmdUnicast,  NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s snmpTrapVerCmdTbl[] = {
    {"v1",              "Trap Version 1",                       snmpTrapVerCliCmdV1,       NULL,           NULL,           0},
    {"v2c",             "Trap Version 2c",                      snmpTrapVerCliCmdV2c,      NULL,           NULL,           0},
    {"v3",              "Trap Version 3",                       snmpTrapVerCliCmdV3,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s snmpV3AuthProtoCmdTbl[] = {
    {"none",                "",                                 snmpV3AuthProtoCliCmdNone,  NULL,           NULL,           0},
    {"hmac-md5",            "",                                 snmpV3AuthProtoCliCmdMd5,   NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s snmpV3PrivProtoCmdTbl[] = {
    {"none",                "",                                 snmpV3PrivProtoCliCmdNone,  NULL,           NULL,           0},
    {"cbs-des",             "",                                 snmpV3PrivProtoCliCmdDes,   NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s syslogCmdTbl[] = {
#ifdef AP101NA_LINKSYS
	{"disable",         "Disable Syslog",                       syslogCliCmdDisable,    NULL,           NULL,           0},
	{"enable",          "Enable Syslog",                        syslogCliCmdEnable,     NULL,           NULL,           0},
#else     
    {"disable",         "Disable Syslog",                       syslogCliCmdDisable,    NULL,           NULL,           0},
    {"broadcast",       "Send Syslog Broadcast",                syslogCliCmdBroadcast,  NULL,           NULL,           0},
    {"unicast",         "Send Syslog to a Specified Server",    syslogCliCmdUnicast,    NULL,           NULL,           0},
#endif
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s syslogSeverityCmdTbl[] = {
    {"1",               "Set Syslog Severity to Level: 1 - Alert Messages",syslogSeverityCliCmdSet,NULL,NULL,          0},
    {"2",               "Set Syslog Severity to Level: 2 - Critical Messages",syslogSeverityCliCmdSet,NULL,NULL,       0},
    {"3",               "Set Syslog Severity to Level: 3 - Error Messages",syslogSeverityCliCmdSet,NULL,NULL,0},
    {"4",               "Set Syslog Severity to Level: 4 - Warning Messages",syslogSeverityCliCmdSet,NULL,NULL,        0},
    {"5",               "Set Syslog Severity to Level: 5 - Notice Messages",syslogSeverityCliCmdSet,NULL,NULL,         0},
    {"6",               "Set Syslog Severity to Level: 6 - Informational Messages",syslogSeverityCliCmdSet,NULL,NULL,  0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s roguedetectCmdTbl[] = {
    {txtDisable,        "Set Rogue AP Detection Disable",           roguedetectCliCmdDisable,NULL,      NULL,           0},
    {txtEnable,         "Set Rogue AP Detection Enable",            roguedetectCliCmdEnable,NULL,       NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s roguetypeCmdTbl[] = {
    {"inseurity",       "Set Rogue AP Type as No Security",         roguetype1CliCmdEnable, NULL,       NULL,           0},
    {"illegal",         "Set Rogue AP Type as Not in Legal AP List",roguetype2CliCmdEnable, NULL,       NULL,           0},
    {"both",            "Set Rogue AP Type as Both No Security and Not in Legal AP List",roguetype3CliCmdEnable,NULL,NULL,0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s roguesnmpCmdTbl[] = {
    {txtDisable,        "Set Rogue AP Detection SNMP Trap Disable", roguesnmpCliCmdDisable, NULL,       NULL,           0},
    {txtEnable,         "Set Rogue AP Detection SNMP Trap Enable",  roguesnmpCliCmdEnable,  NULL,       NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s roguelegalCmdTbl[] = {
    {"add",             "Add a AP MAC/OUI into Legal AP List",      roguelegalCmdAdd,       NULL,       NULL,           0},
    {"delete",          "Delete a AP MAC/OUI from Legal AP List",   roguelegalCmdDel,       NULL,       NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s getCmdTbl[] = {
#ifdef AP101NA_LINKSYS
    {"11nguardinterval","Set 11n Guard Interval Mode",          dot11nGuardIntervalCliCmdGet,   NULL,   NULL,           0},
#endif
#ifndef AP101NA_LINKSYS
    {"11nsubchannel",   "Set 11n Extension Sub-Channel",        dot11nSubChannelCliCmdGet,      NULL,   NULL,           0},
#endif
    {"11nchanbandwidth","Set 11n Channel Bandwidth",            dot11nRadioBandCliCmdGet,       NULL,   NULL,           0},
    {"802.11d",         "Display 802.11d Mode",                 dot11dModeCliCmdGet,    NULL,           NULL,           0},
#ifndef AP101NA_LINKSYS
    {"acctserver",      "Display Accounting Server",            genericCmdHandler,      acctSrvCliCmdGetTbl,           NULL,           0},
    {"acctport",        "Display Accounting Port",              genericCmdHandler,      acctPortCliCmdGetTbl,           NULL,           0},
    {"acctsecret",      "Display Accounting Secret",            genericCmdHandler,      acctSecretCliCmdGetTbl,           NULL,           0},
#endif
    {"acl",             "Display Access Control Status",        aclCliCmdGet,           NULL,           NULL,           0},
    {"active",          "Display VAP Active (up) Mode",         activeModeCliCmdGet,    NULL,           NULL,           0},
    {"aging",           "Display Idle Timeout Interval",        agingCliCmdGet,         NULL,           NULL,           0},
    {"authentication",  "Display Authentication Type of WEP",   authCliCmdGet,          NULL,           NULL,           0},
    {"beaconinterval",  "Display Beacon Interval",              beaconCliCmdGet,        NULL,           NULL,           0},
    {"channel",         "Display Radio Channel",                chanCliCmdGet,          NULL,           NULL,           0},
    {"country",         "Display Country/Domain",               countryCodeCliCmdGet,   NULL,           NULL,           0},
    {"defaultkey"  ,    "Display Default Key Index",            defaultKeyCliCmdGet,    NULL,           NULL,           0},    
    {"description",     "Display Access Point Description",     descCliCmdGet,          NULL,           NULL,           0},
    {"dhcp"  ,          "Display DHCP Mode",                    dhcpModeCliCmdGet,      NULL,           NULL,           0},
#ifndef AP101NA_LINKSYS
    {"dhcpserverendip", "Display DHCP Server End IP Address",   dhcpsEndIpCliCmdGet,    NULL,           NULL,           0},
    {"dhcpserverstartip","Display DHCP Server start IP Address",dhcpsStartIpCliCmdGet,  NULL,           NULL,           0},
#endif
    {"dnsserver",       "Display IP Address of DNS Server",     nameSrvCliCmdGet,       NULL,           NULL,           0},
#ifndef AP101NA_LINKSYS
    {"dot1xdynkeyupdate","Display 802.1x Dynamic Key Update Mode",dot1xKeyUpdateCliCmdGet,NULL,         NULL,           0},
    {"dot1xdynkeylife", "Display 802.1x Dynamic Key Life Time (in Minutes)",dot1xKeyLifeCliCmdGet,NULL, NULL,           0},
#endif  
    {"dot1xkeytype",    "Display 802.1x Distribute Key Method", dot1xKeyCliCmdGet,      NULL,           NULL,           0},
    {"fragthreshold",   "Display Fragment Threshold",           fragCliCmdGet,          NULL,           NULL,           0},
    {"gateway",         "Display Gateway IP Address",           gatewayCliCmdGet,       NULL,           NULL,           0},
#ifndef AP101NA_LINKSYS
    {"groupkeyupdate",       "Display Group Key Update Mode",        groupKeyUpdateCliCmdGet,NULL,           NULL,           0},
#endif
    {"groupkeyupdateinterval","Display Group Key Update Interval (in Seconds)", groupKeyUpdateIntervalCliCmdGet,NULL,NULL,0},
    {"http",            "Display HTTP Mode",                    httpCliCmdGet,          NULL,           NULL,           0},
    {"httpport",        "Display HTTP Port Number",             httpPortCliCmdGet,      NULL,           NULL,           0},
    {"https",           "Display HTTPS Mode",                   httpsCliCmdGet,         NULL,           NULL,           0},
    {"httpsport",       "Display HTTPS Port Number",            httpsPortCliCmdGet,     NULL,           NULL,           0},
    {"ipaddr",          "Display IP Address",                   ipAddrCliCmdGet,        NULL,           NULL,           0},
    {"ipmask",          "Display IP Subnet Mask",               ipMaskCliCmdGet,        NULL,           NULL,           0},
    {"wirelessisolationbetweenssid",       "Display Isolate All Virtual APs State",interVapForwardingCliCmdGet,NULL,       NULL,           0},
    {"key",             "Display WEP Key Value",                keyCliCmdGet,           NULL,           NULL,           0},
    {"keylength",       "Display WEP Key Length",               keyLengthCliCmdGet,     NULL,           NULL,           0},
    {"lltd",            "Display LLTD Mode",                    lltdModeCliCmdGet,      NULL,           NULL,           0}, 
	{"md5supplicant",   "Display 802.1x MD5 Supplicant Mode",   dot1xSuppCliCmdGet,     NULL,           NULL,           0},
	{"md5suppname",     "Display 802.1x Supplicant MD5 Name",   dot1xSuppUserCliCmdGet, NULL,           NULL,           0},
	{"md5supppassword", "Display 802.1x Supplicant MD5 Password",dot1xSuppPassCliCmdGet,NULL,           NULL,           0},
	{"md5supptype",     "Display 802.1x MD5 Supplicant Type",   dot1xSuppTypeCliCmdGet, NULL,           NULL,           0},
    {"nativevlanid",    "Display Native VLAN ID",               nativeVlanIdCliCmdGet,  NULL,           NULL,           0},
    {"ntpserver",       "Display NTP Server IP Address",        ntpServerCliCmdGet,     NULL,           NULL,           0},
/* MD@CPU_AP add at 20080121 */
#ifndef AP101NA_LINKSYS
	{"wins",            "Display WINS Server IP Address",       winsServerCliCmdGet,    NULL,           NULL,           0},
    {"radio",           "Display RADIO Mode",                   radioModeCliCmdGet,     NULL,           NULL,           0},
	{"autochannel",     "Display Auto Channel Mode",       		autochanCliCmdGet,		NULL,           NULL,           0},
#endif 
	{"remotessid",      "Display UC/UR's Remote SSID",			remotessidCliCmdGet,	NULL,       	NULL,           0},
    {"remoteucr",      	"Display UC/UR's Remote MAC Address",	remoteUcrMacAddrCliCmdGet,NULL,       	NULL,           0},
    {"snmpcontact",   	"Display SNMP Contact",      			snmpContactCliCmdGet,   NULL,           NULL,           0},
    {"snmpdevice",  	"Display SNMP Devic Name",     			snmpDeviceNameCliCmdGet,NULL,           NULL,           0},
	{"snmplocation",  	"Display SNMP Location",     			snmpLocationCliCmdGet,  NULL,           NULL,           0},
	{"snmptrapcommunity", 	"Display SNMP Trap Community",     	snmpTrapComCliCmdGet, 	NULL,           NULL,           0},
	{"emailalert",     	"Display Email Alert Mode",       			emailAlertCliCmdGet,		NULL,           NULL,           0},
	{"emailserver",     "Display Email Alert Server",       		emailServerCliCmdGet,		NULL,           NULL,           0},
	{"emailaddress",   	"Display Email Address for Log",       		emailAddressCliCmdGet,		NULL,           NULL,           0},
	{"emailqueue",     	"Display Email Queue Length",       		emailQueueLengthCliCmdGet,	NULL,           NULL,           0},
	{"emailinterval", 	"Display Email Time Threshold",       		emailSendPeriodCliCmdGet,	NULL,           NULL,           0},
	{"logsuccess",     	"Display Log Unauthorized Login Attempt Mode",loginSuccessCliCmdGet,	NULL,           NULL,           0},
	{"logfail",     	"Display Log Authorized Login Mode",       	loginFailCliCmdGet,			NULL,           NULL,           0},
	{"logsyserror",     "Display Log System Error Messages Mode",   sysErrorCliCmdGet,			NULL,           NULL,           0},
	{"logconfchange",	"Display Log Configuration Changes Mode",   confChangeCliCmdGet,		NULL,           NULL,           0},
	{"httpredirect",    "Display HTTP Redirect Mode",       		httpRDModeCliCmdGet,		NULL,           NULL,           0},
	{"httpredirectURL", "Display HTTP Redirect URL",       			httpURLCliCmdGet,		NULL,           NULL,           0},
	{"vlanmanagement",	"Display AP Management VLAN",               magvlanCliCmdGet,      NULL,           NULL,           0},
	{"vlandefault",		"Display AP Management VLAN",               defaultvlanCliCmdGet,      NULL,           NULL,           0},
	{"acllocal",    	"Display Access Control Local Mode",     	aclLocalModeCliCmdGet,           NULL,           NULL,           0},
#ifdef AP101NA_LINKSYS
    {"hostname",      	"Display Access Point Host Name",     		hostnameCliCmdGet,           NULL,           NULL,           0},
    {"devicename",      "Display Access Point Dvice Name",     		devicenameCliCmdGet,         NULL,           NULL,           0},
    {"dns2server",      "Display IP Address of Secondary DNS Server", nameSrv2CliCmdGet,         NULL,           NULL,           0},
	{"ipv6",     		"Display IPv6 Mode",       					ipv6ModeCliCmdGet,			NULL,           NULL,           0},
	{"ipv6addr",     	"Display IPv6 Local Address",       		ipv6AddrCliCmdGet,			NULL,           NULL,           0},
	{"ipv6gateway",     "Display IPv6 Default Gateway",       		ipv6gatewayCliCmdGet,		NULL,           NULL,           0},
	{"timemode",     	"Display Time Mode",       					timeModeCliCmdGet,		NULL,           NULL,           0},
	{"ntp",     		"Display NTP Mode",       					ntpModeCliCmdGet,		NULL,           NULL,           0},
	{"wlanaccess",     	"Display Wireless Web Access Mode",       	wlanAccessModeCliCmdGet,		NULL,           NULL,           0},
	{"ssh",     		"Display SSH Mode",       					sshModeCliCmdGet,		NULL,           NULL,           0},
    {"ctsprotect",    	"Display CTS Protect Mode",     			ctsProtectModeCliCmdGet,           NULL,           NULL,           0},
    {"loadbalance",    	"Display Load Balance Mode",     			loadBalanceModeCliCmdGet,           NULL,           NULL,           0},
	{"loadbalancessid", "Display Load Balance SSID",     			loadBalanceSSIDCliCmdGet,           NULL,           NULL,           0},
	{"vlantag",    		"Display Vlan Tag Mode",     				vlanTagModeCliCmdGet,           NULL,           NULL,           0},
	{"vlantagoverwds",  "Display Vlan Tag Over WDS Mode",     		vlanWDSTagModeCliCmdGet,           NULL,           NULL,           0},
	{"priority",		"Display Priority",               			priorityCliCmdGet,      NULL,           NULL,           0},











#endif
/* add end */
    {"operationmode",   "Display Operation Mode",               opModeCliCmdGet,        NULL,           NULL,           0},
    {"password",        "Display Login Password",               passwordCliCmdGet,      NULL,           NULL,           0},
    {"psk",             "Display Pre-shared Key",               pskCliCmdGet,           NULL,           NULL,           0},
    {"radiusserver",    "Display RADIUS Server IP Address",     genericCmdHandler,      radiusSrvCliCmdGetTbl,          NULL,           0},
    {"radiusport",      "Display RADIUS Port Number",           genericCmdHandler,      radiusPortCliCmdGetTbl,         NULL,           0},
    {"radiussecret",    "Display RADIUS Shared Secret",         genericCmdHandler,      radiusSecretCliCmdGetTbl,       NULL,           0},
    {"remoteptmp",      "Display PTMP's Remote MAC Address List",remotePtmpMacListCliCmdGet,NULL,       NULL,           0},
    {"remoteptp",       "Display PTP's Remote MAC Address",     remotePtpMacAddrCliCmdGet,NULL,         NULL,           0},
#ifndef AP101NA_LINKSYS
    {"roguedetect",     "Display Rogue AP Detection Mode",      rogueCmdGet,            NULL,           NULL,           0},
    {"rogueinteval",    "Display Interval of Every Rogue AP Detection",rogueIntervalCmdGet,NULL,        NULL,           0},
    {"roguelegal",      "Display Legal AP List of Legal AP",    roguelegalCmdGet,       NULL,           NULL,           0},
    {"roguetrap",       "Display Rogue AP Detection Send SNMP Trap Mode",roguesnmpCmdGet,NULL,          NULL,           0},
    {"roguetype",       "Display Rogue AP Definition",          roguetypeCmdGet,        NULL,           NULL,           0},     
#endif
    {"rtsthreshold",    "Display RTS/CTS Threshold",            rtsCliCmdGet,           NULL,           NULL,           0},
    {"security",        "Display Wireless Security Mode",       secCliCmdGet,           NULL,           NULL,           0},
    {"shortpreamble",   "Display Short Preamble Usage",         shortPreambleCliCmdGet, NULL,           NULL,           0},
    {"snmpreadcommunity",   "Display SNMP Read Community",      snmpReadComCliCmdGet,   NULL,           NULL,           0},
    {"snmpwritecommunity",  "Display SNMP Write Community",     snmpWriteComCliCmdGet,  NULL,           NULL,           0},
    {"snmpmode",        "Display SNMP Mode",                    snmpModeCliCmdGet,      NULL,           NULL,           0},
    {"snmpmanagemode",  "Display SNMP Manager Mode",            snmpManageCliCmdGet,    NULL,           NULL,           0},
    {"snmptrapmode",    "Display SNMP Trap Mode",               snmpTrapCliCmdGet,      NULL,           NULL,           0},
#ifndef AP101NA_LINKSYS
    {"snmptrapversion", "Display SNMP Trap Version",            snmpTrapVerCliCmdGet,   NULL,           NULL,           0},
    {"snmpv3username",  "Display SNMP v3 User Name",                snmpV3UserNameCliCmdGet,    NULL,           NULL,           0},
    {"snmpv3authproto", "Display SNMP v3 Authentication Protocol",  snmpV3AuthProtoCliCmdGet,   NULL,           NULL,           0},
    {"snmpv3authkey",   "Display SNMP v3 Authentication Key",       snmpV3AuthKeyCliCmdGet,     NULL,           NULL,           0},
    {"snmpv3privproto", "Display SNMP v3 Private Protocol",         snmpV3PrivProtoCliCmdGet,   NULL,           NULL,           0},
    {"snmpv3privkey",   "Display SNMP v3 Private Key",              snmpV3PrivKeyCliCmdGet,     NULL,           NULL,           0},
#endif
    {"ssid",            "Display Service Set ID",               ssidCliCmdGet,          NULL,           NULL,           0},
    {"ssidbroadcast",   "Display SSID Broadcast Mode",          ssidModeCliCmdGet,      NULL,           NULL,           0},
    {"stp",             "Display STP Mode",                     stpModeCliCmdGet,       NULL,           NULL,           0},
#ifndef AP101NA_LINKSYS
    {"strictgtkupdate", "Display Group Key Update Strict Status",gtkUpdateStrictCliCmdGet,NULL,         NULL,           0},
#endif
    {"syslog",          "Display Syslog Mode",                  syslogCliCmdGet,        NULL,           NULL,           0},
#ifndef AP101NA_LINKSYS
    {"syslogport",      "Display Syslog Port",                  syslogPortCliCmdGet,    NULL,           NULL,           0},
#endif
    {"syslogserver",    "Display Unicast Syslog Server Address",syslogSrvCliCmdGet,     NULL,           NULL,           0},
#ifndef AP101NA_LINKSYS
    {"syslogseverity",  "Display Syslog Severity Level",        syslogSeverityCliCmdGet,NULL,           NULL,           0},
    {"systemname",      "Display Access Point System Name",     sysCliCmdGet,           NULL,           NULL,           0},
    {"telnet",          "Display Telnet Mode",                  telnetCliCmdGet,        NULL,           NULL,           0},
#endif
    {"time",            "Display Current System Time",          todCliCmdGet,           NULL,           NULL,           0},
    {"timezone",        "Display Time Zone Setting",            timezoneCliCmdGet,      NULL,           NULL,           0},
    {"uptime",          "Display Access Point Up Time",         uptimeCliCmdGet,        NULL,           NULL,           0},
    {"username",        "Display Login User Name",              usernameCliCmdGet,      NULL,           NULL,           0},
#ifndef AP101NA_LINKSYS
    {"vapname",         "Display Virtual AP Name",              vapNameCliCmdGet,       NULL,           NULL,           0},
#endif  
    {"version",         "Display Firmware Version",             versionCmdGet,          NULL,           NULL,           0},
    {"vlan",            "Display VLAN Operational State",       vlanModeCliCmdGet,      NULL,           NULL,           0},
    {"vlanid",          "Display the VLAN ID",                  vlanPvidCliCmdGet,      NULL,           NULL,           0},
    {"wirelessmode",    "Display Wireless LAN Mode",            wirelessModeCliCmdGet,  NULL,           NULL,           0},
    {"wirelessisolationwithinssid","Display Isolation within VAP",        sepCliCmdGet,           NULL,           NULL,           0},
    {"wmm",             "Display WMM Mode",                     wmeCliCmdGet,           NULL,           NULL,           0},
#ifndef AP101NA_LINKSYS
    {"wmmnoack",        "Display WMM No Acknowledgement status",wmeNoAckCliCmdGet,      NULL,           NULL,           0},
#endif
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s setCmdTbl[] = {
#ifdef AP101NA_LINKSYS
    {"11nguardinterval","Set 11n Guard Interval Mode",          genericCmdHandler,       dot11nGuardIntervalCmdTbl,  NULL,          0},
#endif
#ifndef AP101NA_LINKSYS
    {"11nsubchannel",   "Set 11n Extension Sub-Channel",        genericCmdHandler,       dot11nSubChannelCmdTbl,     NULL,          0},
#endif
    {"11nchanbandwidth", "Set 11n Channel Bandwidth",            genericCmdHandler,       dot11nRadioBandCmdTbl,      NULL,          0},
    {"802.11d",         "Set 802.11d Mode",                     genericCmdHandler,       dot11dModeCmdTbl,NULL,          0},
#ifndef AP101NA_LINKSYS
    {"acctserver",      "Set Accounting Server",                genericCmdHandler,       acctSrvCliCmdSetTbl,           NULL,           0},
    {"acctport",        "Set Accounting Port",                  genericCmdHandler,       acctPortCliCmdSetTbl,           NULL,           0},
    {"acctsecret",      "Set Accounting Secret",                genericCmdHandler,       acctSecretCliCmdSetTbl,           NULL,           0},
#endif
    {"acl",             "Set Access Control",                   genericCmdHandler,       aclCmdTbl,      NULL,           0},
    {"active",          "Set Active (up) Mode",                 genericCmdHandler,       activeModeCmdTbl,NULL,          0},
    {"aging",           "Set Idle Timeout Interval",            agingCliCmdSet,         NULL,           NULL,           0},
    {"authentication",  "Set Authentication Type of WEP",       genericCmdHandler,       authCmdTbl,     NULL,           0},
    {"beaconinterval",  "Set Beacon Interval",                  beaconCliCmdSet,        NULL,           NULL,           0},
    {"channel",         "Set Radio Channel",                    chanCliCmdSet,          NULL,           NULL,           0},
    {"country",         "Set Country/Domain",                   genericCmdHandler,      countryCodeCmdTbl,              NULL,           0},
    {"defaultkey"  ,    "Set Default Key Index",                defaultKeyCliCmdSet,    NULL,           NULL,           0},    
    {"description",     "Set Access Point Description",         descCliCmdSet,          NULL,           NULL,           0},
    {"dhcp",            "Set DHCP Mode",                        genericCmdHandler,       dhcpCmdTbl,     NULL,           0},
#ifndef AP101NA_LINKSYS
    {"dhcpserverendip", "Set DHCP Server End IP Address",       dhcpsEndIpCliCmdSet,    NULL,           NULL,           0},
    {"dhcpserverstartip","Set DHCP Server start IP Address",    dhcpsStartIpCliCmdSet,  NULL,           NULL,           0},
#endif
    {"dnsserver",       "Set DNS Server IP Address",            nameSrvCliCmdSet,       NULL,           NULL,           0},
#ifndef AP101NA_LINKSYS
    {"dot1xdynkeyupdate","Set 802.1x Dynamic Key Update Mode",  genericCmdHandler,       dot1xKeyUpdateCmdTbl,NULL,      0},
    {"dot1xdynkeylife", "Set 802.1x Dynamic Key Life Time (in Minutes)",dot1xKeyLifeCliCmdSet,NULL,     NULL,           0},
    {"dot1xkeytype",    "Set 802.1x Distribute Key Method",     genericCmdHandler,       dot1xKeyCmdTbl, NULL,           0},
#endif
    {"fragthreshold",   "Set Fragment Threshold",               fragCliCmdSet,          NULL,           NULL,           0},
    {"gateway",         "Set Gateway IP Address",               gatewayCliCmdSet,       NULL,           NULL,           0},
#ifndef AP101NA_LINKSYS
    {"groupkeyupdate",  "Set Group Key Update Mode",            genericCmdHandler,       groupKeyCmdTbl, NULL,           0},
#endif
    {"groupkeyupdateinterval","Set Group Key Update Interval (in Seconds)", groupKeyUpdateIntervalCliCmdSet,NULL,NULL,  0},
    {"http",            "Set HTTP Mode",                        genericCmdHandler,       httpCmdTbl,     NULL,           0},
    {"httpport",        "Set HTTP Port Number",                 httpPortCliCmdSet,      NULL,           NULL,           0},
    {"https",           "Set HTTPS Enable/Disable",             genericCmdHandler,       httpsCmdTbl,    NULL,           0},
    {"httpsport",       "Set HTTPS Port Number",                httpsPortCliCmdSet,     NULL,           NULL,           0},
    {"ipaddr",          "Set IP Address",                       ipAddrCliCmdSet,        NULL,           NULL,           0},
    {"ipmask",          "Set IP Subnet Mask",                   ipMaskCliCmdSet,        NULL,           NULL,           0},
    {"wirelessisolationbetweenssid",       "Set Isolate All Virtual APs State",    genericCmdHandler,      interVapForwardingCmdTbl,NULL,  0},
    {"key",             "Set WEP Key Value",                    keyCliCmdSet,           NULL,           NULL,           0},
    {"keylength",       "Set WEP Key Length",                   keyLengthCliCmdSet,     NULL,           NULL,           0}, 
    {"lltd",            "Set LLTD Mode",                        genericCmdHandler,      lltdModeCmdTbl, NULL,           0},      
	{"md5supplicant",   "Set 802.1x MD5 Supplicant Mode",       genericCmdHandler,      dot1xSuppCmdTbl,NULL,           0},
	{"md5suppname",     "Set 802.1x Supplicant MD5 Name",       dot1xSuppUserCliCmdSet, NULL,           NULL,           0},
	{"md5supppassword", "Set 802.1x Supplicant MD5 Password",   dot1xSuppPassCliCmdSet, NULL,           NULL,           0},
	{"md5supptype",     "Set 802.1x MD5 Supplicant Type",       genericCmdHandler,       dot1xSuppTypeCmdTbl,NULL,        0},
    {"nativevlanid",    "Set Native VLAN ID",                   nativeVlanIdCliCmdSet,  NULL,           NULL,           0},
    {"ntpserver",       "Set NTP Server IP Address",            ntpServerCliCmdSet,    NULL,           NULL,           0},  
/* MD@CPU_AP add at 20080121 */
#ifndef AP101NA_LINKSYS
	{"wins",            "Set WINS Server IP Address",           winsServerCliCmdSet,    NULL,           NULL,           0},  
	{"radio",           "Set RADIO Mode",                       genericCmdHandler,      radiomodeCmdTbl,NULL,           0},
	{"autochannel",     "Set Auto Channel Mode",           		genericCmdHandler,		autochanCmdTbl, NULL,           0},  
#endif
	{"remotessid",      "Set UC/UR's Remote SSID",       		remotessidCliCmdSet,	NULL,         	NULL,           0},
    {"remoteucr",       "Set UC/UR's Remote MAC Address",       remoteUcrMacAddrCliCmdSet,NULL,         NULL,           0},
    {"snmpcontact",   	"Set SNMP Contact",      				snmpContactCliCmdSet,   NULL,           NULL,           0},
    {"snmpdevice",  	"Set SNMP Devic Name",     				snmpDeviceNameCliCmdSet,NULL,           NULL,           0},
	{"snmplocation",  	"Set SNMP Location",     				snmpLocationCliCmdSet,  NULL,           NULL,           0},
	{"snmptrapcommunity", 	"Set SNMP Trap Community",     		snmpTrapComCliCmdSet, 	NULL,           NULL,           0},
	{"emailalert",     	"Set Email Alert Mode",           		genericCmdHandler,		emailalertCmdTbl, 	NULL,           0},  
	{"emailserver",     "Set Email Alert Server",           	emailServerCliCmdSet,	NULL, 				NULL,           0},  
	{"emailaddress",   	"Set Email Address for Log",           	emailAddressCliCmdSet,	NULL, 				NULL,           0},  
	{"emailqueue",     	"Set Email Queue Length",           	emailQueueLengthCliCmdSet,	NULL, 			NULL,           0},  
	{"emailinterval",	"Set Email Time Threshold",           	emailSendPeriodCliCmdSet,	NULL, 			NULL,           0},  
	{"logsuccess",     	"Set Log Unauthorized Login Attempt Mode",  genericCmdHandler,	loginsuccessCmdTbl, NULL,           0},  
	{"logfail",     	"Set Log Authorized Login Mode",        genericCmdHandler,		loginfailCmdTbl, 	NULL,           0},  
	{"logsyserror",     "Set Log System Error Messages Mode",   genericCmdHandler,		syserrorCmdTbl, 	NULL,           0},  
	{"logconfchange", 	"Set Log Configuration Changes Mode",   genericCmdHandler,		confchangeCmdTbl, 	NULL,           0},  
	{"httpredirect",    "Set HTTP Redirect Mode",       		genericCmdHandler,			httpredirectmodeCmdTbl,           NULL,           0},
	{"httpredirectURL", "Set HTTP Redirect URL",       			httpURLCliCmdSet,			NULL,           NULL,           0},
	{"acllocal",    "Set Access Control Local Mode",     	genericCmdHandler,          acllocalmodeCmdTbl,           NULL,           0},
	{"vlanmanagement",	"Set AP Management VLAN",               magvlanCliCmdSet,      NULL,           NULL,           0},
	{"vlandefault",		"Set AP Management VLAN",               defaultvlanCliCmdSet,      NULL,           NULL,           0},

#ifdef AP101NA_LINKSYS
    {"hostname",      	"Set Access Point Host Name",     		hostnameCliCmdSet,          NULL,           NULL,           0},
    {"devicename",      "Set Access Point Dvice Name",     		devicenameCliCmdSet,        NULL,           NULL,           0},
    {"dns2server",      "Set IP Address of Secondary DNS Server", nameSrv2CliCmdSet,        NULL,           NULL,           0},
	{"ipv6",     		"Display IPv6 Mode",       				genericCmdHandler,			ipv6modeCmdTbl, NULL,           0},
	{"ipv6addr",     	"Display IPv6 Local Address",       	ipv6AddrCliCmdSet,			NULL,           NULL,           0},
	{"ipv6gateway",     "Display IPv6 Default Gateway",     	ipv6gatewayCliCmdSet,		NULL,           NULL,           0},
	{"timemode",     	"Set Time Mode",       					genericCmdHandler,			timemodeCmdTbl,           NULL,           0},
	{"ntp",     		"Set NTP Mode",       					genericCmdHandler,			ntpmodeCmdTbl,           NULL,           0},
	{"wlanaccess",     	"Set Wireless Web Access Mode",     	genericCmdHandler,			wlanaccessmodeCmdTbl,           NULL,           0},
	{"ssh",     		"Set SSH Mode",       					genericCmdHandler,			sshmodeCmdTbl,           NULL,           0},
    {"ctsprotect",    	"Set CTS Protect Mode",     			genericCmdHandler,          ctsprotectmodeCmdTbl,           NULL,           0},
	{"loadbalance",    	"Set Load Balance Mode",     			genericCmdHandler,          loadbalancemodeCmdTbl,           NULL,           0},
	{"loadbalancessid", "Set Load Balance SSID",     			loadBalanceSSIDCliCmdSet,           NULL,           NULL,           0},
	{"vlantag",    		"Set Vlan Tag Mode",     				genericCmdHandler,          vlantagmodeCmdTbl,           NULL,           0},
	{"vlantagoverwds",  "Set Vlan Tag Over WDS Mode",     		genericCmdHandler,          vlanWDStagmodeCmdTbl,           NULL,           0},
	{"priority",		"Set Priority",               			priorityCliCmdSet,      	NULL,           NULL,           0},
	{"time",            "Set Current System Time",          	genericCmdHandler,          timesetCmdTbl,           NULL,           0},
#endif

/* add end */
    {"operationmode",   "Set operation Mode",                   genericCmdHandler,      opModeCmdTbl,   NULL,           0},
    {"password",        "Modify Login Password",                passwordCliCmdSet,      NULL,           NULL,           0},
    {"psk",             "Modify Pre-shared Key",                pskCliCmdSet,           NULL,           NULL,           0},
    {"radiusserver",    "Set RADIUS IP Address",                genericCmdHandler,      radiusSrvCliCmdSetTbl,           NULL,           0},
    {"radiusport",      "Set RADIUS Port Number",               genericCmdHandler,      radiusPortCliCmdSetTbl,         NULL,           0},
    {"radiussecret",    "Set RADIUS Shared Secret",             genericCmdHandler,      radiusSecretCliCmdSetTbl,       NULL,           0},
    {"remoteptmp",      "Set PTMP's Remote MAC Address List",   genericCmdHandler,      remotePtmpMacListCmdTbl,        NULL,           0},
    {"remoteptp",       "Set Remote PTP MAC Address",           remotePtpMacAddrCliCmdSet,NULL,          NULL,           0},
#ifndef AP101NA_LINKSYS
	{"roguedetect",     "Set Rogue AP Detection Mode",          genericCmdHandler,       roguedetectCmdTbl,NULL,         0},
    {"rogueinteval",    "Set Interval of Rogue AP Detection(Range: 3 ~ 99)",rogueIntervalCmdSet,NULL,   NULL,           0},
    {"roguelegal",      "Add/Delete Legal AP MAC/OUI",          genericCmdHandler,       roguelegalCmdTbl,NULL,          0},
    {"roguesnmp",       "Set Rogue AP Detection SNMP Trap Mode",genericCmdHandler,       roguesnmpCmdTbl,NULL,           0},
    {"roguetype",       "Set Rogue AP Definition",              genericCmdHandler,       roguetypeCmdTbl,NULL,           0},   
#endif  
    {"rtsthreshold",    "Set RTS/CTS Threshold",                rtsCliCmdSet,           NULL,           NULL,           0},
    {"security",        "Set Wireless Security Mode",           genericCmdHandler,       secCmdTbl,      NULL,           0},
    {"shortpreamble",   "Set Short Preamble",                   genericCmdHandler,       shortPreambleCmdTbl,NULL,      0},
    {"snmpreadcommunity",   "Set SNMP Read Community",          snmpReadComCliCmdSet,   NULL,           NULL,           0},
    {"snmpwritecommunity",  "Set SNMP Write Community",         snmpWriteComCliCmdSet,  NULL,           NULL,           0},
    {"snmpmode",        "Set SNMP Mode",                        genericCmdHandler,      snmpModeCmdTbl, NULL,           0},
    {"snmpmanagemode",  "Set SNMP Manager Mode",                genericCmdHandler,      snmpManageCmdTbl,   NULL,       0},
    {"snmptrapmode",    "Set SNMP Trap Mode",                   genericCmdHandler,      snmpTrapModeCmdTbl, NULL,       0},
#ifndef AP101NA_LINKSYS
    {"snmptrapversion", "Set SNMP Trap Version",                genericCmdHandler,      snmpTrapVerCmdTbl,  NULL,       0},
    {"snmpv3username",  "Set SNMP v3 User Name",                snmpV3UserNameCliCmdSet,NULL,           NULL,           0},
    {"snmpv3authproto", "Set SNMP v3 Authentication Protocol",  genericCmdHandler,      snmpV3AuthProtoCmdTbl,NULL,           0},
    {"snmpv3authkey",   "Set SNMP v3 Authentication Key",       snmpV3AuthKeyCliCmdSet, NULL,           NULL,           0},
    {"snmpv3privproto", "Set SNMP v3 Private Protocol",         genericCmdHandler,      snmpV3PrivProtoCmdTbl,NULL,           0},
    {"snmpv3privkey",   "Set SNMP v3 Private Key",              snmpV3PrivKeyCliCmdSet, NULL,           NULL,           0},
#endif  
    {"ssid",            "Set Service Set ID",                   ssidCliCmdSet,          NULL,           NULL,           0},
    {"ssidbroadcast",    "Set SSID Broadcast Mode",              genericCmdHandler,      ssidModeCmdTbl, NULL,           0},
    {"stp",             "Set STP Mode",                         genericCmdHandler,      stpModeCmdTbl,  NULL,           0},
#ifndef AP101NA_LINKSYS
    {"strictgtkupdate", "Set Group Key Update Strict Status",   genericCmdHandler,      gtkUpdateStrictCmdTbl,NULL,     0},
#endif 
    {"syslog",          "Set Syslog Mode",                      genericCmdHandler,      syslogCmdTbl,   NULL,           0},
#ifndef AP101NA_LINKSYS
    {"syslogport",      "Set Syslog Port",                      syslogPortCliCmdSet,    NULL,           NULL,           0},
#endif 
    {"syslogserver",    "Set Unicast Syslog Server Address",    syslogSrvCliCmdSet,     NULL,           NULL,           0},
#ifndef AP101NA_LINKSYS
    {"syslogseverity",  "Set Syslog Severity Level",            genericCmdHandler,       syslogSeverityCmdTbl,NULL,      0},
    {"systemname",      "Set Access Point System Name",         sysCliCmdSet,           NULL,           NULL,           0},
    {"telnet",          "Set Telnet Mode",                      genericCmdHandler,       telnetCmdTbl,   NULL,           0},
#endif
    {"timezone",        "Set Time Zone Setting",                genericCmdHandler,      timezoneCmdTbl,           NULL,           0},      
    {"username",        "Modify Login User Name",               usernameCliCmdSet,      NULL,           NULL,           0},
#ifndef AP101NA_LINKSYS
    {"vapname",         "Set Virtual AP Name",                  vapNameCliCmdSet,       NULL,           NULL,           0},
#endif
    {"vlan",            "Set VLAN Operational State",           genericCmdHandler,       vlanModeCmdTbl, NULL,           0},
    {"vlanid",          "Set the VLAN Tag",                     vlanPvidCliCmdSet,      NULL,           NULL,           0},
    {"wirelessmode",    "Set Wireless LAN Mode",                genericCmdHandler,       wirelessModeCmdTbl,NULL,        0},
    {"wirelessisolationwithinssid","Set Isolation within VAP",  genericCmdHandler,       sepCmdTbl,      NULL,           0},
    {"wmm",             "Set WMM Mode",                         genericCmdHandler,       wmeCmdTbl,      NULL,           0},
#ifndef AP101NA_LINKSYS
    {"wmmnoack",        "Set WMM No Acknowledgement Mode",      genericCmdHandler,       wmeNoAckCmdTbl, NULL,           0},
#endif
    PARSE_TOKEN_DELIMITER
};

struct parse_token_s cmdTbl[] = {
    {"config",          "Config WLAN X or Virtual AP X",        genericCmdHandler,      configCmdTbl,   NULL,           0},
    {"?",               "Display CLI Command List",             sccli_helpCmdHandler,   NULL,           CLI_HIDDENCMD,  0},
    {"help",            "Display CLI Command List",             sccli_helpCmdHandler,   NULL,           CLI_HIDDENCMD,  0},
    {"get",             NULL,                                   genericCmdHandler,      getCmdTbl,      NULL,           0},
    {"set",             NULL,                                   genericCliCmdSet,       setCmdTbl,      NULL,           0},
    {"factoryrestore",  "Restore to Default Factory Settings",  factoryCmdHandler,      NULL,           NULL,           0},
    {"apply",           "To make the changes take effect",      applyCmdHandler,        NULL,           NULL,           0},
    {"exit",            "Quit the Remove Console",              sccli_exitCmdHandler,   NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};


static void sccli_Init(void)
{
    if(gpCli != NULL)
        return;
        
    gpCli = malloc(sizeof(struct cli_s));
    memset(gpCli, 0, sizeof(struct cli_s));
    
    gpCli->vap = 0;
    gpCli->unit = RADIO_24G;
    
    gpCli->securityCheck = TRUE;
}

static int sccli_IsSuperUser(void)
{
    return 1;
}

static void sccli_tokenInit(CLI *pCli)
{
    pCli->pToken = (char *)pCli->token;
    bzero((char *)pCli->token, sizeof(pCli->token));
    pCli->tokenIdx = 0;
    pCli->tokenLvl = 0;
    pCli->token_count = 0;
    pCli->parseTblIdx = 0;
    bzero((char *)&pCli->ParseTbl, sizeof(pCli->ParseTbl));
}

static void sccli_tokenCurTblSet (struct parse_token_s *pTbl)
{
    pCurTokenTbl = pTbl;
}

static struct parse_token_s* sccli_tokenCurTblGet(void)
{
    return pCurTokenTbl;
}

static int sccli_tokenCount(CLI *pCli)
{
    int i, idx, cnt = 0;

    idx = pCli->tokenIdx;
    for (i = 0; i < MAX_TOKEN; i++) {
        if (pCli->token[idx]) {
            cnt++;
        }
        idx = (idx + 1) % MAX_TOKEN;
    }
    return cnt;
}

/*
 *  Pop a token off the top of the token stack
 */
static char *sccli_tokenPop(CLI *pCli)
{
    int i, idx;

    idx = pCli->tokenIdx;
    for (i = 0; i < MAX_TOKEN; i++) {
        if (pCli->token[idx]) {
            int cnt;
            char *p;
            p = pCli->token[idx];
            cnt = sccli_tokenCount(pCli);
            pCli->token[idx] = 0;
            #ifdef T_DEBUG    
                printf("<%s,%d>: %p -- %s, len: %d, cnt: %d\n",__FUNCTION__,__LINE__, (void *)pCli, p, strlen(p), cnt);
            #endif    
            return p;
        }
        idx = (idx + 1) % MAX_TOKEN;
    }
    return NULL;
}

/*
 *  Push a token onto the token stack
 */
static void sccli_tokenPush(CLI *pCli, char *p)
{
    pCli->token[pCli->tokenIdx] = p;
    if (pCli->tokenIdx == 0) 
    {
        pCli->pToken0 = p;
    }
    pCli->tokenIdx = (pCli->tokenIdx + 1) % MAX_TOKEN;
    #ifdef T_DEBUG    
        printf("<%s,%d>: tokenPush %p -- %s, len: %d, cnt: %d\n",__FUNCTION__,__LINE__, (void *)pCli, p, strlen(p),
              sccli_tokenCount(pCli));
    #endif    
}

static char *sccli_tokenPopWoCase(CLI *pCli)
{
    int i, idx;

    idx = pCli->tokenIdx;
    for (i = 0; i < MAX_TOKEN; i++) {
        if (pCli->token[idx]) {
            char *pBuf = (char *)pCli->ibuf;
            char *p = pCli->token[idx];
            char ch;

            pCli->token[idx] = 0;

            while (*p) {
                ch = *pBuf = *p;
                if (isupper(ch)) {  /* Convert upper to lower case */
                    *pBuf = ch - 'A' + 'a';
                }
                p++;
                pBuf++;	
            }
            *pBuf = 0;
            pBuf = (char *)pCli->ibuf;
            #ifdef T_DEBUG    
                printf("<%s,%d>: tokenPopWoCase -- %s, len: %d, cnt: %d\n",__FUNCTION__,__LINE__,
                      pBuf, strlen(pBuf), sccli_tokenCount(pCli));
            #endif    
            return pBuf;
        }
        idx = (idx + 1) % MAX_TOKEN;
    }
    return NULL;
}

static int sccli_tokenPopNum(CLI *pCli, int *pV, int base)
{
    char    *p;
    char    *pEnd;

    pCli->tokenLvl++;
    p = sccli_tokenPop(pCli);
    if (p) {
        #ifdef T_DEBUG    
            printf("<%s,%d>: p: %s, len: %d\n",__FUNCTION__,__LINE__, p, strlen(p));
        #endif
        *pV = strtoul(p, &pEnd, base);
        if (p == pEnd || *pEnd) {
            printf("Invalid input\n");
            return CLI_PARSE_ERROR;
        }
        return CLI_PARSE_OK;
    }

    printf("Error -- No value specified\n");
    return CLI_PARSE_ERROR;
}


static int sccli_tokenGetFpNumHandler(CLI *pCli, int *pV, int *pVr, int base)
{
    char    *p;
    char    *pEnd;

    *pVr = 0;
    pCli->tokenLvl++;
    p = sccli_tokenPop(pCli);
    if (p) {
        #ifdef T_DEBUG    
            printf("<%s,%d>: p: %s, len: %d\n",__FUNCTION__,__LINE__, p, strlen(p));
        #endif    
        *pV = strtoul(p, &pEnd, base);
        if (*pEnd == 0) {
            return CLI_PARSE_OK;
        }
        if (*pEnd == '.') {
            p = pEnd + 1;
            *pVr = strtoul(p, &pEnd, base);
            if (p == pEnd || *pEnd) {
                printf("Invalid input!\n");
                return CLI_PARSE_ERROR;
            }
            if (*pEnd == 0) {
                return CLI_PARSE_OK;
            }
            return CLI_PARSE_ERROR;
        }
        return CLI_PARSE_ERROR;
    }

    printf("Error -- No value specified\n");
    return CLI_PARSE_ERROR;
}

static int sccli_tokenGetNetAddrHandler(CLI *pCli, struct in_addr *pIaddr)
{
    char        *p;
    int         addr;
    char        *pEnd;
    char        *q;
    int         i;

    pCli->tokenLvl++;
    p = sccli_tokenPop(pCli);
    if (p) {
        #ifdef T_DEBUG    
            printf("<%s,%d>: p: %s, len: %d\n",__FUNCTION__,__LINE__, p, strlen(p));
        #endif    
        q = p;
        for (i = 0; i < 4; i++) {
            int v;
            v = strtoul(q, &pEnd, 10);
            if (*pEnd) {
                if (v >= 256)
                    return -1;
                if (v == 0 && q == pEnd)
                    return -1;
            } else {
                if (i != 3)
                    return -1;
            }
            q = pEnd + 1;
        }
        addr = inet_addr(p);
        if (addr == -1)
            return -1;
        pIaddr->s_addr = addr;
        return 0;
    }

    printf("No value specified\n");
    return -1;
}

static int sccli_tokenIsValidOpMode(CLI *pCli, struct parse_token_s *pTbl)
{
//    if (pTbl->opMode) {
//        if (pTbl->opMode & MODE_SELECT_XR) {
//            return isXrAp(GET_PHY_INFO(pCli->unit)) ? 1 : 0;
//        } 
//        else if(pTbl->opMode & MODE_SELECT_24G)
//        {
//            return (pCli->unit == DEV_24G);
//        }
//        else if(pTbl->opMode & MODE_SELECT_5G)
//        {
//            return (pCli->unit == DEV_5G);
//        }
//        else {
//            return pTbl->opMode & apCfgFreqSpecGet(pCli->unit) ? 1 : 0;
//        }
//    }
    return 1;
}

static int sccli_tokenIsProtected(struct parse_token_s *pTbl)
{
    if (pTbl->pProtected) {
        if (bcmp(pTbl->pProtected, CLI_SUPERUSER, sizeof(CLI_SUPERUSER)) == 0) {
            return 1;
        }
    }
    return 0;
}

static int sccli_tokenIsHidden(struct parse_token_s *pTbl)
{
    if (pTbl->pProtected) {
        if (bcmp(pTbl->pProtected, CLI_HIDDENCMD, sizeof(CLI_HIDDENCMD)) == 0) {
            return 1;
        }
    }
    return 0;
}

static int sccli_tokencmp(char *p1, char *p2, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        if (tolower(p1[i]) !=  tolower(p2[i])) {
            return 1;
        }
    }
    return 0;
}

static int sccli_helpCmdScan(CLI *pCli, struct parse_token_s *pTbl, char *pTxt,
                      char *pFilter, int verbose)
{
    struct parse_token_s *pTblx;
    char   buf[132];
    int    bmatch;

    if (pTbl == NULL || pTxt == NULL) {
        return CLI_PARSE_OK;
    }
        
    for (pTblx = pTbl; pTblx->fHandler; pTblx++) {
        if (pCli->securityCheck) {
            if (sccli_tokenIsProtected(pTblx) == TRUE &&
                sccli_IsSuperUser() == FALSE)
                continue;
            if (sccli_tokenIsHidden(pTblx) == TRUE || sccli_tokenIsValidOpMode(pCli, pTblx) == FALSE)
                continue;
        }
        if (verbose && pTblx->pNxtParseTbl) {
            sprintf(buf, "%s %s", pTxt, pTblx->pCmd);
            if (verbose == 1) {
                if (pFilter) {
                    bmatch = bcmp(pTblx->pCmd, pFilter, strlen(pFilter));
                    if (bmatch == 0) {
                        sccli_helpCmdScan(pCli, pTblx->pNxtParseTbl, buf, NULL, 0);
                    }
                } else {
                    sccli_helpCmdScan(pCli, pTblx->pNxtParseTbl, buf, pFilter, 0);
                }
            }
            if (verbose >= 10) {
                sccli_helpCmdScan(pCli, pTblx->pNxtParseTbl, buf, NULL, verbose);
            }
        } else {
            sprintf(buf, "%s %s", pTxt, pTblx->pCmd);
            if (pTblx->pHelpText) {
                if (pFilter) {
                    bmatch = bcmp(pTblx->pCmd, pFilter, strlen(pFilter));
                    #ifdef T_DEBUG    
                        printf("<%s,%d>:cmd: %s, filter: %s, len: %d, bcmp: %d \n",
                            __FUNCTION__,__LINE__,
                            pTblx->pCmd, pFilter, strlen(pFilter), bmatch);
                    #endif    
                    if (bmatch == 0) {
                        printf("%-35s -- %s\n", buf,
                                 pTblx->pHelpText ? pTblx->pHelpText : "");
                    }
                } else {
                    printf("%-35s -- %s\n", buf,
                             pTblx->pHelpText ? pTblx->pHelpText : "");

                }
            }
        }
    }
    return CLI_PARSE_OK;
}


static int
sccli_helpCmdHandler(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char    *pToken;
    int     verbose = 1;

    printf("List of Access Point CLI commands:\n");

    if (sccli_tokenCount(pCli)) {
        pToken = sccli_tokenPop(pCli);
        verbose = 10;
    }
    sccli_helpCmdScan(pCli, cmdTbl, "", NULL, verbose);
    return CLI_PARSE_OK;
}

static int sccli_tokenParser(CLI *pCli, struct parse_token_s *pTbl)
{
    char    *p;
    char    *pFilter;
    int     rc;

    #ifdef T_DEBUG    
        printf("<%s,%d>: tokenParser\n",__FUNCTION__,__LINE__);
    #endif  
      
    pCli->tokenLvl++;
    pCli->ParseTbl[pCli->parseTblIdx++] = pTbl;
    p = sccli_tokenPopWoCase(pCli);
    if (p) 
    {
        struct parse_token_s *pTblx;
        int len = strlen(p);
        int k, match;

        for (pTblx = pTbl; pTblx->fHandler; pTblx++) 
        {
            pTblx->abbrevMatchCnt = 0;
        }

        #ifdef T_DEBUG    
            printf("<%s,%d>: p: %s, len: %d\n",__FUNCTION__,__LINE__, p, strlen(p));
        #endif  
          
        pFilter = p;
        match = 0;
        for (pTblx = pTbl; pTblx->fHandler; pTblx++) 
        {
            if (len == strlen(pTblx->pCmd) &&
                sccli_tokenIsValidOpMode(pCli, pTblx) == 1 &&
                sccli_tokencmp(p, pTblx->pCmd, len) == 0) 
            {
                #ifdef T_DEBUG    
                    printf("<%s,%d>: Exact match found\n",__FUNCTION__,__LINE__);
                #endif
                    
                match = 1;              /* exact match */
                pTblx->abbrevMatchCnt = len;
                break;
            }
        }

        if (match == 0) 
        {
            for (k = 0; k < len; k++) 
            {
                for (pTblx = pTbl; pTblx->fHandler; pTblx++) 
                {
                    /* Don't let priv and hidden tokens out of the bag */
                    if (pCli->securityCheck && sccli_tokenIsProtected(pTblx) == 1 &&
                        sccli_IsSuperUser() == 0) 
                    {
                        continue;
                    }
                    if (sccli_tokenIsHidden(pTblx) || 
                        sccli_tokenIsValidOpMode(pCli, pTblx) == 0) 
                    {
                        continue;
                    }

                    if (*(p+k) && *(pTblx->pCmd+k)) 
                    {
                        if (*(p+k) == *(pTblx->pCmd+k)) 
                        {
                            if (k == 0) 
                            {
                                match++;
                            }
                            if (k == pTblx->abbrevMatchCnt) 
                            {
                                pTblx->abbrevMatchCnt++;
                                #ifdef T_DEBUG    
                                    printf("<%s,%d>: %s -- abbrevMatch: %d\n",
                                        __FUNCTION__,__LINE__,pTblx->pCmd, pTblx->abbrevMatchCnt);
                                #endif    
                            }
                        }
                    }
                }
            }
        }
        #ifdef T_DEBUG    
            printf("<%s,%d>: %d match found, filter: %s\n",__FUNCTION__,__LINE__, match, pFilter);
        #endif    
        if (match == 1) 
        {
            match = 0;
            pFilter = p;
            len = strlen(p);
            for (pTblx = pTbl; pTblx->fHandler; pTblx++) 
            {
                if (pTblx->abbrevMatchCnt) 
                {
                    if (len <= strlen(pTblx->pCmd)) 
                    {
                        if (sccli_tokencmp(p, pTblx->pCmd, len) == 0) 
                        {
                            if (!pCli->securityCheck || sccli_IsSuperUser() == 1 ||
                                sccli_tokenIsProtected(pTblx) == 0) 
                            {
                                sccli_tokenCurTblSet(pTblx);
                                rc = (*pTblx->fHandler)(pCli, p, pTblx->pNxtParseTbl);
                                if (rc == CLI_PARSE_NO_VALUE ||
                                    rc == CLI_PARSE_UNKNOWN  ||
                                    rc == CLI_PARSE_INVALID_PARAMETER)
                                {
                                    if (rc == CLI_PARSE_INVALID_PARAMETER) 
                                    {
                                        sccli_helpCmdScan(pCli, pCli->ParseTbl[pCli->parseTblIdx-1],
                                                    "", NULL, 1);
                                    } 
                                    else 
                                    {
                                        if (rc == CLI_PARSE_UNKNOWN) 
                                        {
                                            sccli_helpCmdScan(pCli, pCli->ParseTbl[pCli->parseTblIdx-1],
                                                        pTblx->pCmd, pFilter, 1);
                                        } 
                                        else 
                                        {
                                            sccli_helpCmdScan(pCli, pTblx->pNxtParseTbl,
                                                        pTblx->pCmd, NULL, 1);
                                        }
                                    }
                                    rc = (rc == CLI_PARSE_NO_VALUE) ?
                                        CLI_PARSE_TOO_FEW : CLI_PARSE_ERROR;
                                }
                                lineCounter = 0;
                                return rc;
                            }
                            match = 1;
                        }
                        #ifdef T_DEBUG    
                            printf("<%s,%d>: tokenParser -- %s can't match with table item %s, pTbl: %p\n",
                                __FUNCTION__,__LINE__,p, pTblx->pCmd, (void *)pTblx);
                        #endif    
                    }
                }
            }
        } 
        else if (match != 0) 
        {
            /*
             *  Multiple matches found. Find the one with the highest match
             */
            int cnt;
            cnt = pTbl->abbrevMatchCnt;
            for (pTblx = pTbl; pTblx->fHandler; pTblx++) 
            {
                if (pTblx->abbrevMatchCnt) 
                {
                    if (pTblx->abbrevMatchCnt > cnt) 
                    {
                        cnt = pTblx->abbrevMatchCnt;
                    }
                }
            }
            #ifdef T_DEBUG    
                printf("<%s,%d>: highest abmatch cnt: %d\n",__FUNCTION__,__LINE__,cnt);
            #endif    
            match = 0;
            for (pTblx = pTbl; pTblx->fHandler; pTblx++) 
            {
                if (pTblx->abbrevMatchCnt == cnt) 
                {
                    if (cnt == strlen(p) && cnt == strlen(pTblx->pCmd)) 
                    {
                        match = 1;
                        break;
                    }
                    match++;
                }
            }
            #ifdef T_DEBUG    
                printf("<%s,%d>: match -- %d\n",__FUNCTION__,__LINE__, match);
            #endif    
            if (match) 
            {
                len = strlen(p);
                for (pTblx = pTbl; pTblx->fHandler; pTblx++) 
                {
                    if (pTblx->abbrevMatchCnt == cnt) 
                    {
                        if (len <= strlen(pTblx->pCmd)) 
                        {
                            if (sccli_tokencmp(p, pTblx->pCmd, len) == 0) 
                            {
                                if (!pCli->securityCheck || sccli_IsSuperUser() == 1 ||
                                    sccli_tokenIsProtected(pTblx) == 0) 
                                {
                                    sccli_tokenCurTblSet (pTblx);
                                    rc = (*pTblx->fHandler)(pCli, p, pTblx->pNxtParseTbl);
                                    if (rc == CLI_PARSE_NO_VALUE) 
                                    {
                                        sccli_helpCmdScan(pCli, pTblx->pNxtParseTbl, pTblx->pCmd, NULL, 1);
                                        rc = CLI_PARSE_TOO_FEW;
                                    }
                                    lineCounter = 0;
                                    return rc;
                                }
                            }
                        }
                        break;
                    }
                }
                #ifdef T_DEBUG    
                    printf("<%s,%d>: %s won't match with table item %s, pTbl: %p\n",
                        __FUNCTION__,__LINE__,p, pTblx->pCmd,(void *)pTblx);
                #endif    
            }
        }

        if (pCli->tokenLvl > 1) 
        {
            printf("Invalid parameter: %s", p);
        } 
        else 
        {
            printf("Unknown command: %s", p);
        }

        while ((p = sccli_tokenPop(pCli)) != NULL) 
        {
            printf(" %s", p);
        }
        printf("\n");
        #ifdef T_DEBUG    
            printf("<%s,%d>: parseTblIdx: %d\n",__FUNCTION__,__LINE__, pCli->parseTblIdx);
        #endif    
        if (!(pCli->parseTblIdx-1 == 0 &&
              *(pCli->ParseTbl[pCli->parseTblIdx-1]->pCmd) == '#')) 
        {
            if (pCli->tokenLvl <= 2) 
            {
                printf("Type \"help\" for a list of valid commands.\n");
            } 
            else 
            {
                printf("List of valid parameter(s):\n");
            }
        }
        lineCounter = 0;
        return pCli->tokenLvl <= 2 ? CLI_PARSE_UNKNOWN : CLI_PARSE_INVALID_PARAMETER;
    }
    lineCounter = 0;
    return CLI_PARSE_OK;
}

int sccli_parser(CLI *pCli, struct parse_token_s *pTbl, int argc, char **argv)
{
    char buf[CLI_INBUF];
    int i;
    int length = 0;
    char *p;
    char *pToken;
    char ch;
    int status;
    
    sccli_tokenInit(pCli);
    
    p = buf;
    
    for(i=0; i<argc; i++)
    {
        length += sprintf(p+length, "%s", argv[i]);
        *(p+length) = 0;
        length++;
    }
    
    pToken = NULL;
    p = buf;
    
    for (i = 0; i < length; i++) 
    {
        ch = *p;
        if (ch) 
        {
            if (!pToken) 
            {
                pToken = p;
            }
        } 
        else 
        {
            if (pToken) 
            {
                sccli_tokenPush(pCli, pToken);
            }
            pToken = NULL;
        }
        p++;
    }
    status = sccli_tokenParser(pCli, pTbl);
    return status;
}

char *sccli_getPrompt(void)
{
    int len = 0;
    len = sprintf(prompt,"[VAP%d @ %s]# ", gpCli->vap, apCfgSysNameGet());
    prompt[len] = 0;
    return prompt;
}

int sercomm_cmd(int argc, char **argv)
{
    INTOFF;
    sccli_parser(gpCli, cmdTbl, argc, argv);
    INTON;
    return 0;
}

static int genericCmdHandler(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    if (sccli_tokenCount(pCli)) {
        if (pTbl) {
            return sccli_tokenParser(pCli, pTbl);
        }
        return CLI_PARSE_NO_TBL;
    }
    return CLI_PARSE_NO_VALUE;
}

static int genericCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int return_value;
    
    return_value = genericCmdHandler(pCli, p, pTbl);
       
    if(CLI_PARSE_OK == return_value)
        apcfg_submit();
    return return_value;
}

static int   applyCmdHandler(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    COMMAND("sleep 1; scapply action");
    return CLI_PARSE_OK;
}


static void  sccli_exitCmdHandler(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    exit(1);
}

/*
===================================================================
Routine (CLI Command Implement)
===================================================================
*/
int configCliCmdVap(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int v;
    if (sccli_tokenPopNum(pCli, &v, 10) != CLI_PARSE_ERROR) 
    {
        if (sccli_tokenCount(pCli) == 0) 
        {
            if ((v < 0) || (v >= WLAN_MAX_VAP)) 
            {
                printf("Invalid wlan bss number %d\n", v);
            } 
            else 
            {
                pCli->vap = v;
            }
            printf("Current virtual AP: %d\n", pCli->vap);
            return CLI_PARSE_OK;
        }
    }
    return CLI_PARSE_ERROR;
} 

int
dot11nGuardIntervalCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("11n Guard Interval: %s\n", (scApCfgShortGIGet(pCli->unit)==0)? "Auto":((scApCfgShortGIGet(pCli->unit)==1)?"Short":"Long"));
    return CLI_PARSE_OK;
}

int
dot11nGuardIntervalCmdAuto(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    scApCfgShortGISet(pCli->unit, 0);
    printf("11n Guard Interval: %s\n", "Auto");
    return CLI_PARSE_OK;
}

int
dot11nGuardIntervalCmdShort(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    scApCfgShortGISet(pCli->unit, 1);
    printf("11n Guard Interval: %s\n", "Short");
    return CLI_PARSE_OK;
}

int
dot11nGuardIntervalCmdLong(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    scApCfgShortGISet(pCli->unit, 2);
    printf("11n Guard Interval: %s\n", "Long");
    return CLI_PARSE_OK;
}

int
dot11nSubChannelCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("11n Sub-Channel: %s\n", (atoi(apCfgChannelOffsetGet(pCli->unit)) == 1)? "Above Primary Channel":"Below Primary Channel");
    return CLI_PARSE_OK;
}

int
dot11nSubChannelCmdBelow(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgChannelOffsetSet(pCli->unit, "-1");
    printf("11n Sub-Channel: %s\n", "Below Primary Channel");
    return CLI_PARSE_OK;
} 

int
dot11nSubChannelCmdAbove(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgChannelOffsetSet(pCli->unit, "1");
    printf("11n Sub-Channel: %s\n", "Above Primary Channel");
    return CLI_PARSE_OK;
}

int
dot11nRadioBandCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("11n Channel Bandwidth: %s\n", apCfgChannelWidthModeGet(pCli->unit)==0? "Standard-20MHz Channel":
        (apCfgChannelWidthModeGet(pCli->unit)==1?"Auto-20/40MHz Channel":"Wide-40MHz Channel"));
    return CLI_PARSE_OK;
}

int
dot11nRadioBandCmdStandard(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgChannelWidthModeSet(pCli->unit, 0);
    printf("11n Channel Bandwidth: %s\n", "Standard-20MHz Channel");
    return CLI_PARSE_OK;
} 

int
dot11nRadioBandCmdAuto(CLI *pCli, char *p, struct parse_token_s *pTbl)
{

    apCfgChannelWidthModeSet(pCli->unit, 1);
    printf("11n Channel Bandwidth: %s\n", "Auto-20/40MHz Channel");
    return CLI_PARSE_OK;
}

int
dot11nRadioBandCmdWide(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    if(pCli->unit == RADIO_24G)
        printf("On 2.4G, \"Wide-40MHz Channel\" is not supported!");
    else{
        apCfgChannelWidthModeSet(pCli->unit, 2);
        printf("11n Channel Bandwidth: %s\n", "Wide-40MHz Channel");
    }    
    return CLI_PARSE_OK;
}

int
dot11dModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("802.11d Mode: %s\n", pAbleStr[scApCfg80211dEnabledGet(pCli->unit)]);
    return CLI_PARSE_OK;
}    

int
dot11dModeCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    scApCfg80211dEnabledSet(pCli->unit,TRUE);
    printf("802.11d Mode: %s\n", pAbleStr[scApCfg80211dEnabledGet(pCli->unit)]);
    return CLI_PARSE_OK;
}   
 
int   
dot11dModeCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    scApCfg80211dEnabledSet(pCli->unit,FALSE);
    printf("802.11d Mode: %s\n", pAbleStr[scApCfg80211dEnabledGet(pCli->unit)]);
    return CLI_PARSE_OK;
} 

int
acctSrvCliCmdGetPri(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Primary Accounting Server IP Address: %s\n", scApCfgAcctServerGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int
acctSrvCliCmdSetPri(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    struct in_addr iaddr;
    int i;
    
    if (sccli_tokenGetNetAddrHandler(pCli, &iaddr) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            char netstr[20];   
            strcpy(netstr, inet_ntoa(iaddr)); 
            if (iaddr.s_addr) {
                for(i=0; i<WLAN_MAX_VAP; i++)
                    scApCfgAcctServerSet(pCli->unit, i, netstr);
                printf("Primary Accounting Server IP Address: %s\n",
                         scApCfgAcctServerGet(pCli->unit, pCli->vap));
                return CLI_PARSE_OK;
            }
            printf("Invalid IP Address %s\n", netstr);
        }
    } else {
        printf("Invalid IP Address\n");
    }
    return CLI_PARSE_ERROR;
}

int 
acctPortCliCmdGetPri(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Primary Accounting Port Number: %d\n", scApCfgAcctPortGet(pCli->unit ,pCli->vap));
    return CLI_PARSE_OK;
}

int 
acctPortCliCmdSetPri(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int port;
    int i;
    
    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        if (sccli_tokenPopNum(pCli, &port, 10) != CLI_PARSE_OK) {
            return CLI_PARSE_ERROR;
        }
        if (port<0 || port > 65534)
            return CLI_PARSE_ERROR;
        for(i=0; i<WLAN_MAX_VAP; i++)    
            scApCfgAcctPortSet(pCli->unit ,i, port);
        printf("Accounting Port Number: %d\n", scApCfgAcctPortGet(pCli->unit ,pCli->vap));
        return CLI_PARSE_OK;
    }
    return CLI_PARSE_ERROR;
}

int 
acctSecretCliCmdGetPri(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Primary Accounting Server secret: %s\n", scApCfgAcctSecretGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int
acctSecretCliCmdSetPri(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS  rc;
    int i;
    
    switch (sccli_tokenCount(pCli)) {
        case 0:
            return CLI_PARSE_NO_VALUE;
        case 1:
        {    
            char *pStr = sccli_tokenPop(pCli);
            for(i=0; i<WLAN_MAX_VAP; i++)
            {
                rc = scApCfgAcctSecretSet(pCli->unit, i, pStr);
            
                if (rc != A_OK) {
                    printf("Primary Accounting Server secret is too long\n");
                    return CLI_PARSE_ERROR;
                }
            }
            printf("Primary Accounting Server secret: %s\n", scApCfgAcctSecretGet(pCli->unit, pCli->vap));
            return CLI_PARSE_OK;
        }
    }
    return CLI_PARSE_ERROR;
}

int
acctSrvCliCmdGetBck(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Backup Accounting Server IP Address: %s\n", scApCfgBackupAcctServerGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int
acctSrvCliCmdSetBck(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    struct in_addr iaddr;
    int i;
    
    if (sccli_tokenGetNetAddrHandler(pCli, &iaddr) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            char netstr[20];    
            strcpy(netstr, inet_ntoa(iaddr)); 
            if (iaddr.s_addr) {
                for(i=0; i<WLAN_MAX_VAP; i++)
                    scApCfgBackupAcctServerSet(pCli->unit, i, netstr);
                printf("Backup Accounting Server IP Address: %s\n",
                         scApCfgBackupAcctServerGet(pCli->unit, pCli->vap));
                return CLI_PARSE_OK;
            }
            printf("Invalid IP Address %s\n", netstr);
        }
    } else {
        printf("Invalid IP Address\n");
    }
    return CLI_PARSE_ERROR;
}


int 
acctPortCliCmdGetBck(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Backup Accounting Port Number: %d\n", scApCfgBackupAcctPortGet(pCli->unit ,pCli->vap));
    return CLI_PARSE_OK;
}

int 
acctPortCliCmdSetBck(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int port;
    int i;
    
    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        if (sccli_tokenPopNum(pCli, &port, 10) != CLI_PARSE_OK) {
            return CLI_PARSE_ERROR;
        }
        if (port<0 || port > 65534)
            return CLI_PARSE_ERROR;
        for(i=0; i<WLAN_MAX_VAP; i++)    
            scApCfgBackupAcctPortSet(pCli->unit ,i, port);
        printf("Backup Accounting Port Number: %d\n", scApCfgBackupAcctPortGet(pCli->unit ,pCli->vap));
        return CLI_PARSE_OK;
    }
    return CLI_PARSE_ERROR;
}


int 
acctSecretCliCmdGetBck(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Backup Accounting Server secret: %s\n", scApCfgBackupAcctSecretGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int
acctSecretCliCmdSetBck(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS  rc;
    int i;
    
    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        {    
            char *pStr = sccli_tokenPop(pCli);
            for(i=0; i<WLAN_MAX_VAP; i++)
            {
                rc = scApCfgBackupAcctSecretSet(pCli->unit, i, pStr);
            
                if (rc != A_OK) {
                    printf("Backup Accounting Server secret is too long\n");
                    return CLI_PARSE_ERROR;
                }
            }
            printf("Backup Accounting Server secret: %s\n", scApCfgBackupAcctSecretGet(pCli->unit, pCli->vap));
            return CLI_PARSE_OK;
        }      
    }
    return CLI_PARSE_ERROR;
}

int
aclCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int unit = pCli->unit;
    struct scAclBuf_s *pScAclCurr = NULL, *pScAcl = NULL;
    
    printf("Access Control: %s\n", apCfgAclModeGet(unit, 0)==APCFG_ACL_DISABLED? "Disabled" : 
        (apCfgAclModeGet(unit, 0)==APCFG_ACL_LOCAL?"Local":"Radius"));
    if(apCfgAclModeGet(unit, 0)==APCFG_ACL_LOCAL)
    {
        printf("     Trusted Stations List: ");
        while (!scAclBufGet(unit, pCli->vap, &pScAcl)) ;
    	if(pScAcl)
    	{
            printf("\n===MAC Address=======Name=======\n");
    	    for(pScAclCurr = pScAcl; pScAclCurr; pScAclCurr = pScAclCurr->next)
    	    {
    			printf("%s   %s\n",pScAclCurr->mac,pScAclCurr->name);        	
    	    }
    	}	
    	else
    	{
    	    printf("None\n");
    	}
        scAclBufFree(unit, pCli->vap, pScAcl); 
    }
    return CLI_PARSE_OK;
}

int
aclCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgAclModeSet(pCli->unit, 0, APCFG_ACL_DISABLED);
    printf("Access Control: %s\n", "Disabled");
    return CLI_PARSE_OK;
}

int
aclCliCmdLocal(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgAclModeSet(pCli->unit, 0, APCFG_ACL_LOCAL);
    printf("Access Control: %s\n", "Local");
    return CLI_PARSE_OK;
}

int
aclCliCmdRadius(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgAclModeSet(pCli->unit, 0, APCFG_ACL_RADIUS);
    printf("Access Control: %s\n", "Radius");
    return CLI_PARSE_OK;
}


int
aclCliCmdAdd(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char macStr12[13], macStr17[18], *pStr;
    A_STATUS rc;
    char tmpName[16];
    int tokenCountNum = sccli_tokenCount(pCli);
    
    switch(tokenCountNum)
    {
        case 1:
        case 2:
            pStr = sccli_tokenPop(pCli);
            if(strlen(pStr) !=12 && strlen(pStr)!=17){
                printf("Invalid Input\n");
                return CLI_PARSE_ERROR;
            }
            if(strlen(pStr)==17){
                scMacStr17ToStr12(pStr, macStr12);
            }else{
                strcpy(macStr12, pStr);
            }     
            if(!scValidHexs(macStr12, 12)){
                printf("Invalid Input\n");
                return CLI_PARSE_ERROR;
            }
            scMacStr12ToStr17(macStr12, macStr17, ":");
            
            if(tokenCountNum == 2)
                strcpy(tmpName, sccli_tokenPop(pCli));
            else
                strcpy(tmpName, "Unknown");
            rc = apCfgAclAdd(pCli->unit, 0, macStr17,  tmpName, 1);
            if (rc == A_OK) 
            {
                printf("%s had added to trusted list.\n",macStr17);
                return CLI_PARSE_OK;
            } 

            goto aclAddUsage;
            break;
        default:
            goto aclAddUsage;
    }
    return CLI_PARSE_OK;
    
aclAddUsage:
    printf("set acl add \"MAC Address\" [Name]\n");
    return CLI_PARSE_ERROR;
}

int
aclCliCmdDel(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char macStr12[13], macStr17[18], *pStr;
    A_STATUS rc;
    int tokenCountNum = sccli_tokenCount(pCli);
    
    switch(tokenCountNum)
    {
        case 1:
            pStr = sccli_tokenPop(pCli);
            if(strlen(pStr) !=12 && strlen(pStr)!=17){
                printf("Invalid Input\n");
                return CLI_PARSE_ERROR;
            }
            if(strlen(pStr)==17){
                scMacStr17ToStr12(pStr, macStr12);
            }else{
                strcpy(macStr12, pStr);
            }    
            if(!scValidHexs(macStr12, 12)){
                printf("Invalid Input\n");
                return CLI_PARSE_ERROR;
            }
            scMacStr12ToStr17(macStr12, macStr17, ":");
            
            rc = apCfgAclDel(pCli->unit, 0, macStr17);	
            if (rc == A_OK) 
            {
                printf("%s had deleted from trusted list.\n",macStr17);
                return CLI_PARSE_OK;
            } 
            else if(rc == A_ENOENT)
            {
                printf("%s is not in trusted list.\n",macStr17);
                return CLI_PARSE_ERROR;
            }
            
            goto aclDelUsage;
            break;
        default:
            goto aclDelUsage;
    }
    return CLI_PARSE_OK;
aclDelUsage:
    printf("set acl del \"MAC Address\"\n");
    return CLI_PARSE_ERROR;
}

int
activeModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("VAP Active (up) Mode: %s\n", pAbleStr[apCfgActiveModeGet(pCli->unit, pCli->vap)]);
    return CLI_PARSE_OK;
}

int
activeModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    if(pCli->vap == 0){
        printf("VAP 0 could not be disabled\n");
    }else{
        apCfgActiveModeSet(pCli->unit, pCli->vap, FALSE);
        printf("VAP Active Mode: Disabled\n");
    }
    return CLI_PARSE_OK;
}

int
activeModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgActiveModeSet(pCli->unit, pCli->vap, TRUE);
    printf("VAP Active Mode: Enabled\n");
    return CLI_PARSE_OK;
}

int
agingCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Idle Timeout Interval: %d Minutes\n", scApCfgIdleTimeoutIntervalGet(pCli->unit));
    return CLI_PARSE_OK;
}

int
agingCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    int v;

    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            if (v<SC_MIN_IDLETIMEOUT_INTERVAL || v>SC_MAX_IDLETIMEOUT_INTERVAL)
                printf("Idle timeout interval is invalid. Valid range is %d ~ %d\n",SC_MIN_IDLETIMEOUT_INTERVAL, SC_MAX_IDLETIMEOUT_INTERVAL);
            else
                rc = scApCfgIdleTimeoutIntervalSet(pCli->unit,v);
            printf("Idle Timeout Interval: %d Minutes\n", scApCfgIdleTimeoutIntervalGet(pCli->unit));
            return CLI_PARSE_OK;
        }
    }
    return CLI_PARSE_ERROR;
}

int cliGetWirelessSecurity(int unit, int pro)
{ 
    int index;
    switch (apCfgAuthTypeGet(unit, pro)) 
    {
        case APCFG_AUTH_NONE:
            index = 0;
            break;
    	case APCFG_AUTH_WPAPSK:
    		index = 2;
    		break;
        case APCFG_AUTH_WPA2PSK:
            index = 3;
            break;
        case  APCFG_AUTH_WPA_AUTO_PSK:
            index = 4;
            break;
        case APCFG_AUTH_WPA:
            index = 5;
            break;
        case APCFG_AUTH_WPA2:
            index = 6;
            break;
        case APCFG_AUTH_WPA_AUTO:
            index = 7;
            break;   
        case APCFG_AUTH_DOT1X:
            index = 8;
            break;          
        default:
            index = 1;              
            break;
    }
    return index;
}

void cliSetWirelessSecurity(int unit, int vap, int index)
{
    switch (index)
    {
        case 0:     //None
            apCfgAuthTypeSet(unit, vap,APCFG_AUTH_NONE);
            break;
        case 1:     //WEP
            apCfgAuthTypeSet(unit, vap, APCFG_AUTH_OPEN_SYSTEM);         
            break;
        case 2:     //WPA PSK
            apCfgAuthTypeSet(unit, vap, APCFG_AUTH_WPAPSK);
            apCfgWPACipherSet(unit, vap, WPA_CIPHER_TKIP);
            break;
        case 3:     //WPA2 PSK
            apCfgAuthTypeSet(unit, vap,APCFG_AUTH_WPA2PSK);
            apCfgWPACipherSet(unit, vap,WPA_CIPHER_AES);
            break;
        case 4:     //WPA/WPA2 PSK
            apCfgAuthTypeSet(unit, vap,APCFG_AUTH_WPA_AUTO_PSK);
            apCfgWPACipherSet(unit, vap,WPA_CIPHER_AUTO);
            break;
        case 5:     //WPA Radius
            apCfgAuthTypeSet(unit, vap,APCFG_AUTH_WPA);
            apCfgWPACipherSet(unit, vap,WPA_CIPHER_TKIP);
            break;
        case 6:     //WPA2 Radius
            apCfgAuthTypeSet(unit, vap,APCFG_AUTH_WPA2);
            apCfgWPACipherSet(unit, vap,WPA_CIPHER_AES);
            break;
        case 7:     //WPA/WAP2 Radius
            apCfgAuthTypeSet(unit, vap,APCFG_AUTH_WPA_AUTO);
            apCfgWPACipherSet(unit, vap,WPA_CIPHER_AUTO);
            break;
        case 8:     //802.1x
        {    
            apCfgAuthTypeSet(unit, vap,APCFG_AUTH_DOT1X|DOT1X_MODE_DYNAMIC);
            break;
        }    
        default:
            break;
        }
    return;    
} 

int cliGetWEPAuth(int unit, int pro)
{
	switch(apCfgAuthTypeGet(unit, pro))
	{
		case APCFG_AUTH_SHARED_KEY:
        	return 2;
    	case APCFG_AUTH_OPEN_SYSTEM:
        	return 1;
    	case APCFG_AUTH_AUTO:
        	return 0;
    	default:
        	return 1;
    }
}

void cliSetWEPAuth(int unit, int pro, int index)
{
	switch(index)
	{
		case 0:
			apCfgAuthTypeSet(unit,pro,APCFG_AUTH_AUTO);
			break;
		case 1:
			apCfgAuthTypeSet(unit,pro,APCFG_AUTH_OPEN_SYSTEM);
			break;
		case 2:
			apCfgAuthTypeSet(unit,pro,APCFG_AUTH_SHARED_KEY);
			break;
		default:
			;
	}	
}

int
sysCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("System Name: %s\n", apCfgSysNameGet());
    return CLI_PARSE_OK;
}

int
sysCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char     buf[CLI_INBUF];
    A_STATUS rc;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    buf[0] = '\0';
    while (sccli_tokenCount(pCli)) {
        if (buf[0] != '\0') {
            strcat(buf, " ");
        }
        strcat(buf, sccli_tokenPop(pCli));
    }
    
    if(!scValidNetbiosName(buf, strlen(buf))){
		printf("Access Point Name is invalid. Do not use punctuation or special characters.\n");
    	return CLI_PARSE_ERROR;
    }
    
    rc = apCfgSysNameSet(buf);
    if (rc != A_OK) {
        printf("system name too long!\n");
    }
    printf("System Name: %s\n", apCfgSysNameGet());
    return CLI_PARSE_OK;
}

int descCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Access Point Description: %s\n", apCfgDescGet());
    return CLI_PARSE_OK;
}

int descCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char     buf[CLI_INBUF];
    A_STATUS rc;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    buf[0] = '\0';
    while (sccli_tokenCount(pCli)) {
        if (buf[0] != '\0') {
            strcat(buf, " ");
        }
        strcat(buf, sccli_tokenPop(pCli));
    }

    rc = apCfgDescSet(buf);
    if (rc != A_OK) {
        printf("Description too long!\n");
    }
    printf("Access Point Description: %s\n", apCfgDescGet());
    return CLI_PARSE_OK;
}

int
versionCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char versionStr[30];
    getVersion(versionStr);
    printf("\t%s\n", versionStr);
    return CLI_PARSE_OK;
}

int
todCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char timebuf[80];
    getTimeofDay(timebuf, TIME_FORMAT_NORMAL);
    printf("System Time: %s\n",timebuf);
    return CLI_PARSE_OK;
}

#ifdef AP101NA_LINKSYS
//set time of date
todCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    /*
     * day set format : 1999/01/01 
     */

    return CLI_PARSE_OK;
}

static int isValidChar(char c)
{
	const char validChar[] = {"0123456789"};
	if(index(validChar, (int)c))
		return 1;
	else
		return 0;
}

//set time of clock 
tocCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    /*
     * time set format : 16:05:58
     */   
    return CLI_PARSE_OK;
//    char buf[64];
//	char tmp_h[3], tmp_m[3], tmp_s[3];
//	int i, hour, minute, second;
//    A_STATUS result;
//    
//    buf[0] = '\0';
//    
//    switch (sccli_tokenCount(pCli)) {
//    case 0:
//        printf("Input Null.\n");
//        return CLI_PARSE_NO_VALUE;
//    case 1:
//        {
//        char *p = sccli_tokenPop(pCli);
////        if(p == NULL){
////            printf("<%s,%d>\n", __FUNCTION__, __LINE__);
////            return CLI_PARSE_ERROR;	
////        }else 
//        	printf("<%s,%d>:%d,%s,%c,%c\n", __FUNCTION__, __LINE__, strlen(p), p, *(p+2), *(p+5));        
//        if(strlen(p)!=8 || *(p+2)!=':' || *(p+5)!=':'){
//        	printf("Time setting is invalid. Valid format is xx:xx:xx\n");
//        	return CLI_PARSE_ERROR;
//        }
//		printf("<%s,%d>\n", __FUNCTION__, __LINE__);
////		for (i=0; i<strlen(p); i++){
////			if (!isValidChar(*(p+i)) && *(p+i)!=':'){
////				printf("Time setting2 is invalid. Valid format is xx:xx:xx\n");
////				return CLI_PARSE_ERROR;
////			}
////		}
//		printf("<%s,%d>\n", __FUNCTION__, __LINE__);
//        if (buf[0] != '\0') {                   
//            strcat(buf, " ");
//        }
//        strcat(buf, p);
//
//	    printf("<%s,%d>%s\n", __FUNCTION__, __LINE__, buf);
//	    strcpy(tmp_h,strtok(buf,':'));
//	    printf("<%s,%d>%s\n", __FUNCTION__, __LINE__, tmp_h);
//	    strcpy(tmp_m,strtok(buf,':'));
//	    printf("<%s,%d>%s\n", __FUNCTION__, __LINE__, tmp_m);
//	    strcpy(tmp_s,buf);
//	    printf("<%s,%d>%s\n", __FUNCTION__, __LINE__, tmp_s);
//	    hour = atoi(tmp_h);
//	    minute = atoi(tmp_m);
//	    second = atoi(tmp_s);
//	    result = apCfgTimeHourSet(tmp_h);
//	    result = apCfgTimeMinSet(tmp_m);
//	    result = apCfgTimeSecSet(tmp_s);
//	    if (result != A_OK) {
//	        printf("SSID is invalid.\n");
//	        return CLI_PARSE_ERROR;
//	    }
//	    printf("Time Set: \n");
//	    return CLI_PARSE_OK;
//	}
//}
}












#endif
int
uptimeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char buf[80];

    getUpTime(buf);
    printf("AP Uptime: %s\n", buf);
    return CLI_PARSE_OK;
}

int
countryCodeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{ 
    int i;
    
    for(i=0; i<22; i++){
        if(allCountryList[i].countryCode == apCfgCountryCodeGet())
            break;
    }
    if(i<22)
        printf("Country/Domain: %s\n",allCountryList[i].countryName);
    else{
        printf("Country/Domain: Error\n");
    }    
    return CLI_PARSE_OK;
}

int
countryCodeCliCmdSet(CLI *pCli, char *pToken, struct parse_token_s *pNextTbl)
{   
    if (sccli_tokenCount(pCli) == 0) {
        struct parse_token_s *pTbl = sccli_tokenCurTblGet();

        for ( ; pTbl->fHandler; pTbl++) {
            if (strcmp(pToken, pTbl->pCmd) == 0) {
                
                int i;
    
                for(i=0; i<22; i++){
                    if(strcmp(allCountryList[i].countryName, pTbl->pCmd) == 0)
                        break;
                }
                if(i>=22){
                    printf("Invalid Country/Domain\n");
                    return CLI_PARSE_ERROR;
                }
                apCfgCountryCodeSet(allCountryList[i].countryCode);
                printf("Country/Domain: %s\n", allCountryList[i].countryName);
                return CLI_PARSE_OK;
            }
        }
    }
    
    printf("Invalid Country/Domain\n");
    return CLI_PARSE_ERROR;
}

int dhcpModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("DHCP Mode: %s\n",apCfgDhcpEnableGet()?"Client" : 
        (apCfgDhcpServerEnableGet()? "Server":"Disabled"));
    return CLI_PARSE_OK;
}

int dhcpCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgDhcpEnableSet(FALSE);
    apCfgDhcpServerEnableSet(FALSE);
    printf("DHCP Mode: %s\n", "Disabled");
    return CLI_PARSE_OK;
}

int dhcpCliCmdClient(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgDhcpEnableSet(TRUE);
    apCfgDhcpServerEnableSet(FALSE);
    printf("DHCP Mode: %s\n", "Client");
    return CLI_PARSE_OK;
}

int dhcpCliCmdServer(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgDhcpEnableSet(FALSE);
    apCfgDhcpServerEnableSet(TRUE);
    printf("DHCP Mode: %s\n", "Server");
    return CLI_PARSE_OK;
}

int dhcpsEndIpCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("DHCP Server End IP: %s\n",apCfgDhcpServerEndGet());
    return CLI_PARSE_OK;
}

int
dhcpsEndIpCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    struct in_addr iaddr;
   
    if (sccli_tokenGetNetAddrHandler(pCli, &iaddr) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            char netstr[20];
            
            strcpy(netstr, inet_ntoa(iaddr)); 
            if (iaddr.s_addr) {
                
                struct in_addr addr;

                addr.s_addr = apCfgIpAddrGet();
                addr.s_addr = (addr.s_addr&0xffffff00) | (iaddr.s_addr&0x000000ff);
                
                strcpy(netstr, inet_ntoa(iaddr)); 
                apCfgDhcpServerEndSet(netstr);
            	printf("DHCP Server End IP Address: %s\n", netstr);
                return CLI_PARSE_OK;
            }
            printf("Invalid DHCP Server End IP Address %s\n", netstr);
        }
    }
    printf("Invalid DHCP Server End IP Address\n");
    return CLI_PARSE_ERROR;
}

int dhcpsStartIpCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("DHCP Server Start IP: %s\n",apCfgDhcpServerStartGet());
    return CLI_PARSE_OK;
}

int
dhcpsStartIpCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    struct in_addr iaddr;
   
    if (sccli_tokenGetNetAddrHandler(pCli, &iaddr) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            char netstr[20];
            
            strcpy(netstr, inet_ntoa(iaddr)); 
            if (iaddr.s_addr) {
                
                struct in_addr addr;

                addr.s_addr = apCfgIpAddrGet();
                addr.s_addr = (addr.s_addr&0xffffff00) | (iaddr.s_addr&0x000000ff);
                
                strcpy(netstr, inet_ntoa(iaddr)); 
                apCfgDhcpServerStartSet(netstr);
            	printf("DHCP Server Start IP Address: %s\n", netstr);
                return CLI_PARSE_OK;
            }
            printf("Invalid DHCP Server Start IP Address %s\n", netstr);
        }
    }
    printf("Invalid DHCP Server Start IP Address\n");
    return CLI_PARSE_ERROR;
}

int
ipAddrCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    struct in_addr iaddr;
    char netstr[20];

    iaddr.s_addr = apCfgIpAddrGet();
    strcpy(netstr, inet_ntoa(iaddr)); 
    printf("IP Address: %s\n", netstr);
    return CLI_PARSE_OK;
}

int
ipAddrCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    struct in_addr iaddr;
   
    if (sccli_tokenGetNetAddrHandler(pCli, &iaddr) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            char netstr[20];
            
            strcpy(netstr, inet_ntoa(iaddr)); 
            if (iaddr.s_addr) {
                   	
            	if(scValidIpAddress(iaddr.s_addr))
        		{	
                    apCfgIpAddrSet(iaddr.s_addr);
                    printf("IP Address: %s\n", netstr);
                    if(!scValidIpMaskGateWay(iaddr.s_addr, apCfgIpMaskGet(), apCfgGatewayAddrGet()))
                        printf("Warning: Gateway in different subnet. Please check the Gateway IP and Subnet Mask. \n");
                    return CLI_PARSE_OK;
                }
            }
            printf("Invalid IP Address %s\n", netstr);
        }
    }
    printf("Invalid IP Address\n");
    return CLI_PARSE_ERROR;
}

int
ipMaskCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    struct in_addr iaddr;
    char netstr[20];

    iaddr.s_addr = apCfgIpMaskGet();
    strcpy(netstr, inet_ntoa(iaddr)); 
    printf("IP Subnet Mask: %s\n", netstr);
    return CLI_PARSE_OK;
}

int
ipMaskCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    struct in_addr ipmask;
   
    if (sccli_tokenGetNetAddrHandler(pCli, &ipmask) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            char netstr[20];

            strcpy(netstr, inet_ntoa(ipmask));
            
            if(scValidIpMask(ipmask.s_addr, NULL) && 
   			    scValidIpMaskGateWay(apCfgIpAddrGet(), ipmask.s_addr, apCfgGatewayAddrGet()))
   			{	
                apCfgIpMaskSet(ipmask.s_addr);
                printf("IP Subnet Mask: %s\n", netstr);
                return CLI_PARSE_OK;
            }
        }
    }
    printf("Invalid IP Subnet Mask\n");
    return CLI_PARSE_ERROR;
}

int
gatewayCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    struct in_addr iaddr;
    char netstr[20];

    iaddr.s_addr = apCfgGatewayAddrGet();
    strcpy(netstr, inet_ntoa(iaddr)); 
    printf("Gateway IP Address: %s\n", netstr);
    return CLI_PARSE_OK;
}

int
gatewayCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    struct in_addr iaddr;

    if (sccli_tokenGetNetAddrHandler(pCli, &iaddr) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            char netstr[20];

            strcpy(netstr, inet_ntoa(iaddr)); 
            if (!iaddr.s_addr) {
                netstr[0] = '\0';
            }
           
            if(scValidIpAddress(iaddr.s_addr) && 
   					scValidIpMaskGateWay(apCfgIpAddrGet(), apCfgIpMaskGet(), iaddr.s_addr))
   			{	
                apCfgGatewayAddrSet(iaddr.s_addr);
                printf("Gateway IP Address: %s\n", netstr);
                return CLI_PARSE_OK;
            }
        }
    }
    printf("Invalid Gateway IP Address\n");
    return CLI_PARSE_ERROR;
}

int
nameSrvCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("DNS Server IP Address: %s\n", apCfgNameSrvGet());
    return CLI_PARSE_OK;
}

int
nameSrvCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    struct    in_addr iaddr;
    A_STATUS rc;
    
    switch (sccli_tokenCount(pCli)) 
    {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        if (sccli_tokenGetNetAddrHandler(pCli, &iaddr) != A_ERROR){
            if (sccli_tokenCount(pCli) == 0) {
                char netstr[20];
    
                strcpy(netstr, inet_ntoa(iaddr)); 
                if (!iaddr.s_addr) {
                    netstr[0] = '\0';
                }
                rc = apCfgNameSrvSet(netstr);
                if (rc != A_OK) 
                {
                    printf("Name server address too long.\n");
                    return CLI_PARSE_ERROR;
                }
                printf("Name Server IP Address: %s\n", netstr);
                return CLI_PARSE_OK;
            }
        }
        printf("Invalid Name Server IP Address\n");
    }
    return CLI_PARSE_ERROR;
}

int
ntpServerCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("SNTP/NTP Server IP Address: %s\n", apCfgNtpServerGet());
    return CLI_PARSE_OK;
}

int
ntpServerCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{          
    struct in_addr iaddr;

    if (sccli_tokenGetNetAddrHandler(pCli, &iaddr) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            char netstr[20];
            strcpy(netstr, inet_ntoa(iaddr)); 
            if (iaddr.s_addr) {
                apCfgNtpServerSet(netstr);
                printf("SNTP/NTP Server IP Address: %s\n", apCfgNtpServerGet());
                return CLI_PARSE_OK;
            }
            printf("Invalid IP Address %s\n", netstr);
        }
    } else {
        printf("Invalid IP Address\n");
    }
    return CLI_PARSE_ERROR;
}

/* MD@CPU_AP add at 20080121 */
int
winsServerCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("WINS Server IP Address: %s\n", apCfgWinsServerGet());
    return CLI_PARSE_OK;
}

int
winsServerCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{          
    struct in_addr iaddr;

    if (sccli_tokenGetNetAddrHandler(pCli, &iaddr) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            char netstr[20];
            strcpy(netstr, inet_ntoa(iaddr)); 
            if (iaddr.s_addr) {
                apCfgWinsServerSet(netstr);
                printf("WINS Server IP Address: %s\n", apCfgWinsServerGet());
                return CLI_PARSE_OK;
            }
            printf("Invalid IP Address %s\n", netstr);
        }
    } else {
        printf("Invalid IP Address\n");
    }
    return CLI_PARSE_ERROR;
}

int
autochanCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Auto Channel: %s\n", pAbleStr[apCfgAutoChannelGet(pCli->unit)]);
    return CLI_PARSE_OK;
}

int 
autochanCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgRadioChannelSet(pCli->unit,0);
    printf("Auto Channel: Disabled\n");
    return CLI_PARSE_OK;
}

int 
autochanCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgRadioChannelSet(pCli->unit,1);
    printf("Auto Channel: Enabled\n");
    return CLI_PARSE_OK;
}

int
wirelessModeCliCmd11b(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWlanStateSet(pCli->unit, 1);
    apCfgFreqSpecSet(pCli->unit, MODE_SELECT_11B);
    printf("Wireless Mode: %s\n", "11b");
    return CLI_PARSE_OK;
}

//int
//wirelessModeCliCmd11g(CLI *pCli, char *p, struct parse_token_s *pTbl)
//{
//    apCfgWlanStateSet(pCli->unit, 1);
//    apCfgFreqSpecSet(pCli->unit, MODE_SELECT_11G);
//    printf("Wireless Mode: %s\n", "11g");
//    return CLI_PARSE_OK;
//}

int
wirelessModeCliCmd11bg(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWlanStateSet(pCli->unit, 1);
    apCfgFreqSpecSet(pCli->unit, MODE_SELECT_11BG);
    printf("Wireless Mode: %s\n", "11bg");
    return CLI_PARSE_OK;
}


int
opModeCliCmdUc(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgOpModeSet(pCli->unit, CFG_OP_MODE_UC);
    printf("Operating as %s\n", "Bridge(Unversal Client)");
    return CLI_PARSE_OK;
}

int
opModeCliCmdUr(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgOpModeSet(pCli->unit, CFG_OP_MODE_UR);
    printf("Operating as %s\n", "Bridge(UnversalRepeater)");
    return CLI_PARSE_OK;
}

int
remotessidCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{

    printf("Remote SSID: %s\n", apCfgSsidGet(pCli->unit, 0));
    return CLI_PARSE_OK;
}

int
remotessidCliCmdSet(CLI *pCli, char *s, struct parse_token_s *pTbl)
{
    char buf[64], *pStr;
    int  ssidLen = 0;
    A_STATUS result;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    buf[0] = '\0';

    pStr = buf;
    while (sccli_tokenCount(pCli)) {
        char *p = sccli_tokenPop(pCli);

        ssidLen += strlen(p) + 1;               /* +1 for null or space */
        if (ssidLen > (MAX_SSID + 1)) {
            printf("SSID string is too long\n");
            return CLI_PARSE_ERROR;
        }

        if (buf[0] != '\0') {                   /* not first word */
            strcat(buf, " ");
        }
        strcat(buf, p);
    }

    result = apCfgSsidSet(pCli->unit, 0, buf);
    if (result != A_OK) {
        printf("SSID is invalid.\n");
        return CLI_PARSE_ERROR;
    }
    printf("Remote SSID: %s\n",apCfgSsidGet(pCli->unit, 0));
    return CLI_PARSE_OK;
}

int
remoteUcrMacAddrCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char macStr[18];
    A_STATUS rc;
   
    rc = apCfgRemoteApMacAddrGet(pCli->unit, macStr);
    if (rc != A_OK) {
       return CLI_PARSE_ERROR; 
    }
    printf("Remote AP MAC Address: %s\n", macStr);
    return CLI_PARSE_OK;
}

int
remoteUcrMacAddrCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char macStr12[13], macStr17[18], *pStr;
    A_STATUS     rc = A_OK;
    
    if (sccli_tokenCount(pCli) == 1) {
        
        pStr = sccli_tokenPop(pCli);

        if(strlen(pStr) !=12 && strlen(pStr)!=17){
            printf("Invalid Input\n");
            return CLI_PARSE_ERROR;
        }
        if(strlen(pStr)==17){
            scMacStr17ToStr12(pStr, macStr12);
        }else{
            strcpy(macStr12, pStr);
        }
         
        if(!scValidHexs(macStr12, 12)){
            printf("Invalid Input\n");
            return CLI_PARSE_ERROR;
        }
        
        scMacStr12ToStr17(macStr12, macStr17, ":");

        rc = apCfgRemoteApMacAddrSet(pCli->unit, macStr17);
        if (rc == A_OK) {
            printf("Remote AP MAC Address: %s\n", macStr17);
            return CLI_PARSE_OK;
        }
    } 
    printf("Invalid Input\n");
    return CLI_PARSE_ERROR;
}

int
radioModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Radio State: %s\n", pAbleStr[apCfgWlanStateGet(pCli->unit)]);
    return CLI_PARSE_OK;
}

int 
radioModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWlanStateSet(pCli->unit,0);
    printf("Turn On Radio: Disabled\n");
    return CLI_PARSE_OK;
}

int 
radioModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWlanStateSet(pCli->unit,1);
    printf("Turn On Radio: Enabled\n");
    return CLI_PARSE_OK;
}

int 
snmpContactCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("SNMP Contact: %s\n",apCfgSnmpContactGet());
    return CLI_PARSE_OK;
}

int 
snmpContactCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS     rc;

    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        {    
            char *pStr = sccli_tokenPop(pCli);
    
            rc = apCfgSnmpContactSet(pStr);
            
            if (rc == A_OK) {
                printf("SNMP Contact: %s\n", apCfgSnmpContactGet());
                return CLI_PARSE_OK;
            }
            printf("SNMP Contact is too long\n");
        }      
    }
    return CLI_PARSE_ERROR;
}

int 
snmpDeviceNameCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("SNMP Device Name: %s\n",apCfgSnmpDviceNameGet());
    return CLI_PARSE_OK;
}

int 
snmpDeviceNameCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS     rc;

    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        {    
            char *pStr = sccli_tokenPop(pCli);
    
            rc = apCfgSnmpDviceNameSet(pStr);
            
            if (rc == A_OK) {
                printf("SNMP Device Name: %s\n", apCfgSnmpDviceNameGet());
                return CLI_PARSE_OK;
            }
            printf("SNMP Device Name is too long\n");
        }      
    }
    return CLI_PARSE_ERROR;
}

int 
snmpLocationCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("SNMP Location: %s\n",apCfgSnmpLocationGet());
    return CLI_PARSE_OK;
}

int 
snmpLocationCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS     rc;

    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        {    
            char *pStr = sccli_tokenPop(pCli);
    
            rc = apCfgSnmpLocationSet(pStr);
            
            if (rc == A_OK) {
                printf("SNMP Location: %s\n", apCfgSnmpLocationGet());
                return CLI_PARSE_OK;
            }
            printf("SNMP Location is too long\n");
        }      
    }
    return CLI_PARSE_ERROR;
}

int 
snmpTrapComCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("SNMP Trap Community: %s\n",apCfgSnmpTrapCommunityGet());
    return CLI_PARSE_OK;
}

int 
snmpTrapComCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS     rc;

    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        {    
            char *pStr = sccli_tokenPop(pCli);
    
            rc = apCfgSnmpTrapCommunitySet(pStr);
            
            if (rc == A_OK) {
                printf("SNMP Trap Community: %s\n", apCfgSnmpTrapCommunityGet());
                return CLI_PARSE_OK;
            }
            printf("SNMP Trap Community is too long\n");
        }      
    }
    return CLI_PARSE_ERROR;
}

int
emailAlertCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Email Alert: %s\n", pAbleStr[apCfgemailAlertsEnabledGet()]);
    return CLI_PARSE_OK;
}
int 
emailAlertCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgemailAlertsEnabledSet(0);
    printf("Email Alert: Disabled\n");
    return CLI_PARSE_OK;
}
int 
emailAlertCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgemailAlertsEnabledSet(1);
    printf("Email Alert: Enabled\n");
    return CLI_PARSE_OK;
}

int
emailServerCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("SMTP Server: %s\n", apCfgsmtpMailServerGet());
    return CLI_PARSE_OK;
}
int
emailServerCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{          
    struct in_addr iaddr;

    if (sccli_tokenGetNetAddrHandler(pCli, &iaddr) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            char netstr[20];
            strcpy(netstr, inet_ntoa(iaddr)); 
            if (iaddr.s_addr) {
                apCfgsmtpMailServerSet(netstr);
                printf("Set SMTP Server: %s\n", apCfgsmtpMailServerGet());
                return CLI_PARSE_OK;
            }
            printf("Invalid IP Address %s\n", netstr);
        }
    } else {
        printf("Invalid IP Address\n");
    }
    return CLI_PARSE_ERROR;
}

int
emailAddressCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{

    printf("E-Mail Address for Logs: %s\n", apCfgemailAddrForLogGet());
    return CLI_PARSE_OK;
}
int
emailAddressCliCmdSet(CLI *pCli, char *s, struct parse_token_s *pTbl)
{
    char buf[64], *pStr;
    int  mailaddrLen = 0;
    A_STATUS result;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    buf[0] = '\0';

    pStr = buf;
    while (sccli_tokenCount(pCli)) {
        char *p = sccli_tokenPop(pCli);

        mailaddrLen += strlen(p) + 1;               /* +1 for null or space */
        if (mailaddrLen > 32) {
            printf("Email Address string is too long\n");
            return CLI_PARSE_ERROR;
        }

        if (buf[0] != '\0') {                   /* not first word */
            strcat(buf, " ");
        }
        strcat(buf, p);
    }

    result = apCfgemailAddrForLogSet(buf);
    if (result != A_OK) {
        printf("Email Address is invalid.\n");
        return CLI_PARSE_ERROR;
    }
    printf("Email Address: %s\n",apCfgemailAddrForLogGet());
    return CLI_PARSE_OK;
}

int
emailQueueLengthCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Log Queue Length: %d\n", apCfgemailAlertsQlenGet());
    return CLI_PARSE_OK;
}
int
emailQueueLengthCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    int v;

    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            if (v<1 || v>500)
                printf("Log Queue Length is invalid. Valid range is %d ~ %d\n",1, 500);
            else
                rc = scApCfgemailAlertsQlenSet(v);
            printf("Log Queue Length: %d\n", apCfgemailAlertsQlenGet());
            return CLI_PARSE_OK;
        }
    }
    return CLI_PARSE_ERROR;
}

int
emailSendPeriodCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Log Time Threshold: %d\n", scApCfgemailAlertsIntervalGet());
    return CLI_PARSE_OK;
}
int
emailSendPeriodCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    int v;

    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            if (v<60 || v>600)
                printf("Log Time Threshold is invalid. Valid range is %d ~ %d\n",60, 600);
            else
                rc = scApCfgemailAlertsIntervalSet(v);
            printf("Log Time Threshold: %d\n", scApCfgemailAlertsIntervalGet());
            return CLI_PARSE_OK;
        }
    }
    return CLI_PARSE_ERROR;
}

int
loginSuccessCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Authorized Login: %s\n", pAbleStr[apCfgAuthLoginGet()]);
    return CLI_PARSE_OK;
}
int 
loginSuccessCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgAuthLoginSet(0);
    printf("Authorized Login: Disabled\n");
    return CLI_PARSE_OK;
}
int 
loginSuccessCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgAuthLoginSet(1);
    printf("Authorized Login: Enabled\n");
    return CLI_PARSE_OK;
}

int
loginFailCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Unauthorized Login Attempt: %s\n", pAbleStr[apCfgDeauthGet()]);
    return CLI_PARSE_OK;
}
int 
loginFailCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgDeauthSet(0);
    printf("Unauthorized Login Attempt: Disabled\n");
    return CLI_PARSE_OK;
}
int 
loginFailCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgDeauthSet(1);
    printf("Unauthorized Login Attempt: Enabled\n");
    return CLI_PARSE_OK;
}

int
sysErrorCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("System Error Messages: %s\n", pAbleStr[apCfgChangeSysFucGet()]);
    return CLI_PARSE_OK;
}
int 
sysErrorCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgChangeSysFucSet(0);
    printf("System Error Messages: Disabled\n");
    return CLI_PARSE_OK;
}
int 
sysErrorCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgChangeSysFucSet(1);
    printf("System Error Messages: Enabled\n");
    return CLI_PARSE_OK;
}

int
confChangeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Configuration Changes: %s\n", pAbleStr[apCfgChangeCfgGet()]);
    return CLI_PARSE_OK;
}
int 
confChangeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgChangeCfgSet(0);
    printf("Configuration Changes: Disabled\n");
    return CLI_PARSE_OK;
}
int 
confChangeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgChangeCfgSet(1);
    printf("Configuration Changes: Enabled\n");
    return CLI_PARSE_OK;
}





//#ifdef AP101NA_LINKSYS
int
hostnameCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Host Name: %s\n", apCfgSysNameGet());
    return CLI_PARSE_OK;
}
int
hostnameCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char     buf[CLI_INBUF];
    A_STATUS rc;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    buf[0] = '\0';
    while (sccli_tokenCount(pCli)) {
        if (buf[0] != '\0') {
            strcat(buf, " ");
        }
        strcat(buf, sccli_tokenPop(pCli));
    }
    
    if(!scValidNetbiosName(buf, strlen(buf))){
		printf("Access Point Host Name is invalid. Do not use punctuation or special characters.\n");
    	return CLI_PARSE_ERROR;
    }
    
    rc = apCfgSysNameSet(buf);
    if (rc != A_OK) {
        printf("host name too long!\n");
    }
    printf("Host Name: %s\n", apCfgSysNameGet());
    return CLI_PARSE_OK;
}

int
devicenameCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Device Name: %s\n", apCfgDescGet());
    return CLI_PARSE_OK;
}
int
devicenameCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char     buf[CLI_INBUF];
    A_STATUS rc;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    buf[0] = '\0';
    while (sccli_tokenCount(pCli)) {
        if (buf[0] != '\0') {
            strcat(buf, " ");
        }
        strcat(buf, sccli_tokenPop(pCli));
    }
    
    if(!scValidNetbiosName(buf, strlen(buf))){
		printf("Access Point Device Name is invalid. Do not use punctuation or special characters.\n");
    	return CLI_PARSE_ERROR;
    }
    
    rc = apCfgDescSet(buf);
    if (rc != A_OK) {
        printf("device name too long!\n");
    }
    printf("Device Name: %s\n", apCfgDescGet());
    return CLI_PARSE_OK;
}

int
nameSrv2CliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Secondary DNS Server IP Address: %s\n", apCfgNameSrv2Get());
    return CLI_PARSE_OK;
}
int
nameSrv2CliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    struct    in_addr iaddr;
    A_STATUS rc;
    
    switch (sccli_tokenCount(pCli)) 
    {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        if (sccli_tokenGetNetAddrHandler(pCli, &iaddr) != A_ERROR){
            if (sccli_tokenCount(pCli) == 0) {
                char netstr[20];
    
                strcpy(netstr, inet_ntoa(iaddr)); 
                if (!iaddr.s_addr) {
                    netstr[0] = '\0';
                }
                rc = apCfgNameSrv2Set(netstr);
                if (rc != A_OK) 
                {
                    printf("Name server address too long.\n");
                    return CLI_PARSE_ERROR;
                }
                printf("Name Server IP Address: %s\n", netstr);
                return CLI_PARSE_OK;
            }
        }
        printf("Invalid Name Server IP Address\n");
    }
    return CLI_PARSE_ERROR;
}

int
ipv6ModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("IPv6: %s\n", pAbleStr[apCfgipv6modeGet()]);
    return CLI_PARSE_OK;
}

int 
ipv6ModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgipv6modeSet(0);
    printf("IPv6: Disable\n");
    return CLI_PARSE_OK;
}

int 
ipv6ModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgipv6modeSet(1);
    printf("IPv6: Enable\n");
    return CLI_PARSE_OK;
}

int
ipv6AddrCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Local IP Address: %s\n", apCfgIpv6AddrGet());
    return CLI_PARSE_OK;
}
int
ipv6AddrCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char     buf[CLI_INBUF];
    A_STATUS rc;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    buf[0] = '\0';
    while (sccli_tokenCount(pCli)) {
        if (buf[0] != '\0') {
            strcat(buf, " ");
        }
        strcat(buf, sccli_tokenPop(pCli));
    }   
   
    rc = apCfgIpv6AddrSet(buf);
    if (rc != A_OK) {
        printf("Input Ipv6 Address is wrong!\n");
    }
    printf("Local IP Address: %s\n", apCfgIpv6AddrGet());
    return CLI_PARSE_OK;
}

int
ipv6gatewayCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Default Gateway: %s\n", apCfgGatewayv6AddrGet());
    return CLI_PARSE_OK;
}
int
ipv6gatewayCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char     buf[CLI_INBUF];
    A_STATUS rc;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    buf[0] = '\0';
    while (sccli_tokenCount(pCli)) {
        if (buf[0] != '\0') {
            strcat(buf, " ");
        }
        strcat(buf, sccli_tokenPop(pCli));
    }   
     
    rc = apCfgGatewayv6AddrSet(buf);
    if (rc != A_OK) {
        printf("Input Default Gateway is wrong!\n");
    }
    printf("Default Gateway: %s\n", apCfgGatewayv6AddrGet());
    return CLI_PARSE_OK;
}

int
timeModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Time Mode: %s\n", apCfgTimeModeGet()?"Manually":"Automatically");
    return CLI_PARSE_OK;
}
int 
timeModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgTimeModeSet(0);
    printf("Time Mode: Automatically\n");
    return CLI_PARSE_OK;
}
int 
timeModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgTimeModeSet(1);
    printf("Time Mode: Manually\n");
    return CLI_PARSE_OK;
}

int
ntpModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("NTP Mode: %s\n", pAbleStr[apCfgNtpModeGet()]);
    return CLI_PARSE_OK;
}
int 
ntpModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgNtpModeSet(0);
    printf("NTP Mode: Disable\n");
    return CLI_PARSE_OK;
}
int 
ntpModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgNtpModeSet(1);
    printf("NTP Mode: Enable\n");
    return CLI_PARSE_OK;
}

int
httpRDModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("HTTP Redirect Mode: %s\n", pAbleStr[apCfgRedirectModeGet()]);
    return CLI_PARSE_OK;
}
int 
httpRDModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgRedirectModeSet(0);
    printf("HTTP Redirect Mode: Disable\n");
    return CLI_PARSE_OK;
}
int 
httpRDModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgRedirectModeSet(1);
    printf("HTTP Redirect Mode: Enable\n");
    return CLI_PARSE_OK;
}

int
httpURLCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{

    printf("HTTP Redirect URL: %s\n", apCfgRedirectUrlGet());
    return CLI_PARSE_OK;
}
int
httpURLCliCmdSet(CLI *pCli, char *s, struct parse_token_s *pTbl)
{
    char buf[64], *pStr;
    int  mailaddrLen = 0;
    A_STATUS result;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    buf[0] = '\0';

    pStr = buf;
    while (sccli_tokenCount(pCli)) {
        char *p = sccli_tokenPop(pCli);

        mailaddrLen += strlen(p) + 1;               /* +1 for null or space */
        if (mailaddrLen > 32) {
            printf("HTTP Redirect URL string is too long\n");
            return CLI_PARSE_ERROR;
        }

        if (buf[0] != '\0') {                   /* not first word */
            strcat(buf, " ");
        }
        strcat(buf, p);
    }

    result = apCfgRedirectUrlSet(buf);
    if (result != A_OK) {
        printf("HTTP Redirect URL is invalid.\n");
        return CLI_PARSE_ERROR;
    }
    printf("HTTP Redirect URL: %s\n",apCfgRedirectUrlGet());
    return CLI_PARSE_OK;
}

int
wlanAccessModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Wireless Web Access Mode: %s\n", pAbleStr[apCfgWlanAccessGet()]);
    return CLI_PARSE_OK;
}
int 
wlanAccessModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWlanAccessSet(0);
    printf("Wireless Web Access: Disable\n");
    return CLI_PARSE_OK;
}
int 
wlanAccessModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWlanAccessSet(1);
    printf("Wireless Web Access: Enable\n");
    return CLI_PARSE_OK;
}

int
sshModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("SSH Mode: %s\n", pAbleStr[apCfgSSHGet()]);
    return CLI_PARSE_OK;
}
int 
sshModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgSSHSet(0);
    printf("SSH: Disable\n");
    return CLI_PARSE_OK;
}
int 
sshModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgSSHSet(1);
    printf("SSH: Enable\n");
    return CLI_PARSE_OK;
}

int
aclLocalModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Access Control Local Mode: %s\n", apCfgAclTypeGet(pCli->unit, pCli->vap)?"Deny":"Allow");
    return CLI_PARSE_OK;
}
int 
aclLocalModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgAclTypeSet(pCli->unit, pCli->vap, 0);
    printf("Access Control Local Mode: Allow\n");
    return CLI_PARSE_OK;
}
int 
aclLocalModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgAclTypeSet(pCli->unit, pCli->vap, 1);
    printf("Access Control Local Mode: Deny\n");
    return CLI_PARSE_OK;
}

int
ctsProtectModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("CTS Protection Mode: %s\n", pAbleStr[apCfgCTSModeGet(pCli->unit)]);
    return CLI_PARSE_OK;
}
int 
ctsProtectModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgCTSModeSet(pCli->unit, 0);
    printf("CTS Protection: Disable\n");
    return CLI_PARSE_OK;
}
int 
ctsProtectModeCliCmdAuto(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgCTSModeSet(pCli->unit, 1);
    printf("CTS Protection: Auto\n");
    return CLI_PARSE_OK;
}

int
loadBalanceModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Load Balancing Mode: %s\n", pAbleStr[apCfgBalanceModeGet(pCli->unit)]);
    return CLI_PARSE_OK;
}
int 
loadBalanceModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgBalanceModeSet(pCli->unit, 0);
    printf("Load Balancing: Disable\n");
    return CLI_PARSE_OK;
}
int 
loadBalanceModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgBalanceModeSet(pCli->unit, 1);
    printf("Load Balancing: Enable\n");
    return CLI_PARSE_OK;
}

int
loadBalanceSSIDCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Load Balancing SSID %d: %s\n", pCli->vap, apCfgLoadBalanceGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}
int 
loadBalanceSSIDCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int v;
    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {            
//            if (v <3 || v >99) {
//                printf("range of Rogue AP Detection is 3 ~ 99\n");
//                return CLI_PARSE_ERROR;
//            }
            apCfgLoadBalanceSet(pCli->unit, pCli->vap, v); 
            printf("Set Load Balancing SSID %d: \n", pCli->vap, apCfgLoadBalanceGet(pCli->unit, pCli->vap));
            return CLI_PARSE_OK;
        }
    }
    printf("Invalid Input\n");
    return CLI_PARSE_ERROR;
}

int 
magvlanCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("AP Management VLAN: %d\n", apCfgManagementVlanIdGet());
    return CLI_PARSE_OK;
}
int
magvlanCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int v;
    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {            
            if (v <VLAN_TAG_MIN || v >VLAN_TAG_MAX) {
                printf("range of AP Management VLAN %d ~ %d\n", VLAN_TAG_MIN, VLAN_TAG_MAX);
                return CLI_PARSE_ERROR;
            }
            apCfgManagementVlanIdSet(v); 
            printf("Set AP Management VLAN: %d\n", apCfgManagementVlanIdGet());
            return CLI_PARSE_OK;
        }
    }
    printf("Invalid Vlan\n");
    return CLI_PARSE_ERROR;
}

int 
defaultvlanCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Default VLAN: %d\n", apCfgNativeVlanIdGet());
    return CLI_PARSE_OK;
}
int
defaultvlanCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int v;
    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {            
            if (v <VLAN_TAG_MIN || v >VLAN_TAG_MAX) {
                printf("range of Default VLAN %d ~ %d\n", VLAN_TAG_MIN, VLAN_TAG_MAX);
                return CLI_PARSE_ERROR;
            }
            apCfgNativeVlanIdSet(v); 
            printf("Set Default VLAN: %d\n", apCfgNativeVlanIdGet());
            return CLI_PARSE_OK;
        }
    }
    printf("Invalid Vlan\n");
    return CLI_PARSE_ERROR;
}

int
vlanTagModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Vlan Tag Mode: %s\n", (apCfgNativeVlanTagGet()==0)?"Untagged":"Tagged");
    return CLI_PARSE_OK;
}
int 
vlanTagModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgNativeVlanTagSet(0);
    printf("Vlan Tag: Untagged\n");
    return CLI_PARSE_OK;
}
int 
vlanTagModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgNativeVlanTagSet(1);
    printf("Vlan Tag: Tagged\n");
    return CLI_PARSE_OK;
}

int
vlanWDSTagModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("VLAN Tag over WDS Mode: %s\n", pAbleStr[apCfgWdsVlanTagGet()]);
    return CLI_PARSE_OK;
}
int 
vlanWDSTagModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWdsVlanTagSet(0);
    printf("VLAN Tag over WDS: Disable\n");
    return CLI_PARSE_OK;
}
int 
vlanWDSTagModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWdsVlanTagSet(1);
    printf("VLAN Tag over WDS: Enable\n");
    return CLI_PARSE_OK;
}

int
priorityCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Priority: %d\n", apCfgPriorityGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}
int
priorityCliCmdSet(CLI *pCli, char *s, struct parse_token_s *pTbl)
{
    int v;
    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {            
            if (v <0 || v >7) {
                printf("range of Priority 0 ~ 7\n");
                return CLI_PARSE_ERROR;
            }
            apCfgPrioritySet(pCli->unit, pCli->vap, v); 
            printf("Set Priority: %d\n", apCfgPriorityGet(pCli->unit, pCli->vap));
            return CLI_PARSE_OK;
        }
    }
    printf("Invalid Priority\n");
    return CLI_PARSE_ERROR;
}


















//#endif
/* add end */

static int dot1xSuppCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	printf("802.1x MD5 Supplicant: %s\n", pAbleStr[scApCfgDot1xSuppEnableGet()]);
    return CLI_PARSE_OK; 
}

static int dot1xSuppCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	scApCfgDot1xSuppEnableSet(0);
	printf("802.1x MD5 Supplicant: %s\n", pAbleStr[scApCfgDot1xSuppEnableGet()]);
    return CLI_PARSE_OK; 
}

static int dot1xSuppCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	scApCfgDot1xSuppEnableSet(1);
	printf("802.1x MD5 Supplicant: %s\n", pAbleStr[scApCfgDot1xSuppEnableGet()]);
    return CLI_PARSE_OK; 
}

static int dot1xSuppTypeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	printf("802.1x MD5 Supplicant Type: %s\n", scApCfgDot1xSuppMacEnableGet()==0? "user":"mac");
    return CLI_PARSE_OK; 
}

static int dot1xSuppTypeCliCmdUser(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	scApCfgDot1xSuppMacEnableSet(0);
	printf("802.1x MD5 Supplicant Type: user\n");
    return CLI_PARSE_OK; 
}

static int dot1xSuppTypeCliCmdMac(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	scApCfgDot1xSuppMacEnableSet(1);
	printf("802.1x MD5 Supplicant Type: mac\n");
    return CLI_PARSE_OK; 
}

static int dot1xSuppUserCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	printf("802.1x Supplicant MD5 Name: %s\n", scApCfgDot1xSuppUsernameGet());
    return CLI_PARSE_OK;
}

static int dot1xSuppUserCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{	
	A_STATUS rc;
    if (sccli_tokenCount(pCli) == 0) {
       return CLI_PARSE_NO_VALUE;
    }

    if (sccli_tokenCount(pCli) == 1) 
    {
        rc = scApCfgDot1xSuppUsernameSet(sccli_tokenPop(pCli));
        if (rc != A_OK) 
        {
            printf("802.1x Supplicant MD5 Name is too long\n");
        }
        printf("802.1x Supplicant MD5 Name: %s\n", scApCfgDot1xSuppUsernameGet());
        return CLI_PARSE_OK;
    }
    return CLI_PARSE_ERROR;
}

static int dot1xSuppPassCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	printf("802.1x Supplicant MD5 Password: %s\n", scApCfgDot1xSuppPasswordGet());
    return CLI_PARSE_OK;
}

static int dot1xSuppPassCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{	
    A_STATUS rc;
	if (sccli_tokenCount(pCli) == 0) 
	{
        scApCfgDot1xSuppPasswordSet("");
		printf("802.1x Supplicant MD5 Password: %s\n", scApCfgDot1xSuppPasswordGet());
    	return CLI_PARSE_OK;
    }

    if (sccli_tokenCount(pCli) == 1) 
    {
        rc = scApCfgDot1xSuppPasswordSet(sccli_tokenPop(pCli));
        if (rc != A_OK) 
        {
            printf("802.1x Supplicant MD5 Password is too long\n");
        }
        printf("802.1x Supplicant MD5 Password: %s\n", scApCfgDot1xSuppPasswordGet());
        return CLI_PARSE_OK;        
    }
    return CLI_PARSE_ERROR;
}

int
vlanModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("VLAN Operational State: %s\n", pAbleStr[apCfgVlanModeGet()]);
    return CLI_PARSE_OK;
}

int
vlanModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgVlanModeSet(FALSE);
    printf("VLAN Operational State: Disabled\n");
    return CLI_PARSE_OK;
}

int
vlanModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgVlanModeSet(TRUE);
    printf("VLAN Operational State: Enabled\n");
    return CLI_PARSE_OK;
}

int 
nativeVlanIdCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Native VLAN ID: %d\n", apCfgManagementVlanIdGet());
    return CLI_PARSE_OK;
}

int
nativeVlanIdCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    int v;

    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            rc = apCfgManagementVlanIdSet(v);
            if (rc != A_OK) {
                printf("Unable to set the VLAN ID.\n");
            }
            printf("Native VLAN ID: %d\n", apCfgManagementVlanIdGet());
            return CLI_PARSE_OK;
        }
    }
    return CLI_PARSE_ERROR;
}


int 
vlanPvidCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("VLAN ID: %d\n", apCfgVlanPvidGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int
vlanPvidCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    int v;

    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            rc = apCfgVlanPvidSet(pCli->unit, pCli->vap, v);
            if (rc != A_OK) {
                printf("Unable to set the VLAN Id.\n");
            }
            printf("VLAN Id: %d\n", apCfgVlanPvidGet(pCli->unit, pCli->vap));
            return CLI_PARSE_OK;
        }
    }
    return CLI_PARSE_ERROR;
}

int
wirelessModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Wireless Mode: %s\n", (apCfgWlanStateGet(pCli->unit) == 0)? "Disabled" : 
              ((apCfgFreqSpecGet(pCli->unit) == MODE_SELECT_11G)? "11ng": 
              ((apCfgFreqSpecGet(pCli->unit) == MODE_SELECT_11B)? "11b":
              ((apCfgFreqSpecGet(pCli->unit) == MODE_SELECT_11BG)?"11bg":	
              ((apCfgFreqSpecGet(pCli->unit) == MODE_SELECT_11N)? "11n":"11nbg")))));
    return CLI_PARSE_OK;
}

int
wirelessModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    
    apCfgWlanStateSet(pCli->unit, 0);
    printf("Wireless Mode: %s\n", "Disabled");
    return CLI_PARSE_OK;
}

int
wirelessModeCliCmd11bgn(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWlanStateSet(pCli->unit, 1);
    apCfgFreqSpecSet(pCli->unit, MODE_SELECT_11NG);
    printf("Wireless Mode: %s\n", "11nbg");
    return CLI_PARSE_OK;
}

int
wirelessModeCliCmd11gn(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWlanStateSet(pCli->unit, 1);
    apCfgFreqSpecSet(pCli->unit, MODE_SELECT_11G);
    printf("Wireless Mode: %s\n", "11ng");
    return CLI_PARSE_OK;
}

int
wirelessModeCliCmd11n(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWlanStateSet(pCli->unit, 1);
    apCfgFreqSpecSet(pCli->unit, MODE_SELECT_11N);
    printf("Wireless Mode: %s\n", "11n");
    return CLI_PARSE_OK;
}

int
opModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Operation Mode: %s\n", (apCfgOpModeGet(pCli->unit)== CFG_OP_MODE_AP)? "Access Point": 
              ((apCfgOpModeGet(pCli->unit)== CFG_OP_MODE_PPT)? "Bridge(Point-to-Point)":
              ((apCfgOpModeGet(pCli->unit)== CFG_OP_MODE_MPT)? "Bridge(Multi-point)":
              ((apCfgOpModeGet(pCli->unit)== CFG_OP_MODE_AP_PTP)?"Bridge(Point-to-Point)+ Access Point":
              ((apCfgOpModeGet(pCli->unit)== CFG_OP_MODE_AP_PTMP)?"Bridge(Multi-point) + Access Point":
              ((apCfgOpModeGet(pCli->unit)== CFG_OP_MODE_UC)?"Bridge(Unversal Client)":
              ((apCfgOpModeGet(pCli->unit)== CFG_OP_MODE_UR)?"Bridge(Unversal Repeater)":"Rogue AP")))))));     
    return CLI_PARSE_OK;
}

int
opModeCliCmdAp(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgOpModeSet(pCli->unit, CFG_OP_MODE_AP);
    printf("Operating as %s\n", "Access Point");
    return CLI_PARSE_OK;
}

int
opModeCliCmdPpt(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgOpModeSet(pCli->unit, CFG_OP_MODE_PPT);
    printf("Operating as %s\n", "Bridge(Point-to-Point)");
    return CLI_PARSE_OK;
}

int
opModeCliCmdMpt(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgOpModeSet(pCli->unit, CFG_OP_MODE_MPT);
    printf("Operating as %s\n", "Bridge(Multi-point)");
    return CLI_PARSE_OK;
}

int
opModeCliCmdApPtp(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgOpModeSet(pCli->unit, CFG_OP_MODE_AP_PTP);
    printf("Operating as %s\n", "Bridge(Point-to-Point)+ Access Point");
    return CLI_PARSE_OK;
}

int
opModeCliCmdApPtmp(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgOpModeSet(pCli->unit, CFG_OP_MODE_AP_PTMP);
    printf("Operating as %s\n", "Bridge(Multi-point) + Access Point");
    return CLI_PARSE_OK;
}

int
remotePtpMacAddrCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char macStr[18];
    A_STATUS rc;
   
    rc = apCfgRemoteApMacAddrGet(pCli->unit, macStr);
    if (rc != A_OK) {
       return CLI_PARSE_ERROR; 
    }
    printf("Remote AP MAC Address: %s\n", macStr);
    return CLI_PARSE_OK;
}

int
remotePtpMacAddrCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char macStr12[13], macStr17[18], *pStr;
    A_STATUS     rc = A_OK;
    
    if (sccli_tokenCount(pCli) == 1) {
        
        pStr = sccli_tokenPop(pCli);

        if(strlen(pStr) !=12 && strlen(pStr)!=17){
            printf("Invalid Input\n");
            return CLI_PARSE_ERROR;
        }
        if(strlen(pStr)==17){
            scMacStr17ToStr12(pStr, macStr12);
        }else{
            strcpy(macStr12, pStr);
        }
         
        if(!scValidHexs(macStr12, 12)){
            printf("Invalid Input\n");
            return CLI_PARSE_ERROR;
        }
        
        scMacStr12ToStr17(macStr12, macStr17, ":");

        rc = apCfgRemoteApMacAddrSet(pCli->unit, macStr17);
        if (rc == A_OK) {
            printf("Remote AP MAC Address: %s\n", macStr17);
            return CLI_PARSE_OK;
        }
    } 
    printf("Invalid Input\n");
    return CLI_PARSE_ERROR;
}

int
remotePtmpMacListCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char macStr17[18];    
    int i;

    printf("PTMP's Remote Mac Address List: \n");
    for(i=0; i<8; i++){
        apCfgRemoteWbrMacAddrGet(pCli->unit, i, macStr17);
        printf("\t%d\t%s\n", i+1, macStr17);
    }
    return CLI_PARSE_OK;
}

int
remotePtmpMacListCliCmdSet(CLI *pCli, char *pToken, struct parse_token_s *pNextTbl)
{
    char macStr12[13], macStr17[18], *pStr;
    A_STATUS     rc = A_OK;
    
    if (sccli_tokenCount(pCli) == 1) {
        struct parse_token_s *pTbl = sccli_tokenCurTblGet();

        for ( ; pTbl->fHandler; pTbl++) {
            if (strcmp(pToken, pTbl->pCmd) == 0) {
                
                int index = atoi(pToken)-1;
                
                if(index < 0 && index >= 8 ){
                    return CLI_PARSE_ERROR;
                }
                
                pStr = sccli_tokenPop(pCli);
                if(strlen(pStr) !=12 && strlen(pStr)!=17){
                    printf("Invalid Input\n");
                    return CLI_PARSE_ERROR;
                }
                if(strlen(pStr)==17){
                    scMacStr17ToStr12(pStr, macStr12);
                }else{
                    strcpy(macStr12, pStr);
                }     
                if(!scValidHexs(macStr12, 12)){
                    printf("Invalid Input\n");
                    return CLI_PARSE_ERROR;
                }
                scMacStr12ToStr17(macStr12, macStr17, ":");
                
                rc = apCfgRemoteWbrMacAddrSet(pCli->unit, index, macStr17);
                if (rc == A_OK) {
                    printf("Remote MAC Address: %s\n", macStr17);
                    return CLI_PARSE_OK;
                } 
            }
        }
    }
    
    printf("Invalid Remote Mac Address\n");
    return CLI_PARSE_ERROR;
}

int
chanCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    if (apCfgRadioChannelGet(pCli->unit) == 0){
        FILE     *fp;
        char buffer[130];
        
    	system("/usr/sbin/iwconfig ath00 getCurrentChannel >> /tmp/channel");

    	do
    	{
        	fp = fopen("/tmp/channel", "r");
        }while(fp == NULL);
    	
    	while(!feof(fp))
    	{
    	    buffer[0]='\0';
            fgets(buffer,129,fp);  
            if(strcmp(buffer,"current:\n") == 0)
            { 
                fgets(buffer,129,fp);
                break;
            }
        }
        fclose(fp);
        unlink("/tmp/channel");
        
        if(atoi(buffer) == 0){
            printf("Channel: (Automatic)\n");
        }else{
            printf("Channel: %d (Automatic)\n", atoi(buffer));
        }
    }else
        printf("Channel: %d\n", apCfgRadioChannelGet(pCli->unit));
    return CLI_PARSE_OK;
}

int
chanCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    int v;
      
    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            if(v<0 || v>14){
                printf("The valid channel: 0~14 (0 means Automatic)\n");
                CLI_PARSE_OK;
            }
            
            rc = apCfgRadioChannelSet(pCli->unit, v);
            if (rc != A_OK) {
                printf("Invalid Channel: %d\n", v);
            }
            printf("Channel: %d\n", apCfgRadioChannelGet(pCli->unit));
            return CLI_PARSE_OK;
        }
    }
    printf("The valid channel: 0~14 (0 means Automatic)\n");
    return CLI_PARSE_ERROR;
}

int sepCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Wireless Separate: %s\n", pAbleStr[!apCfgIntraVapForwardingGet(pCli->unit, pCli->vap)]);
    return CLI_PARSE_OK;
}

int sepCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgIntraVapForwardingSet(pCli->unit, pCli->vap, FALSE);
    printf("Isolation within SSID: Enabled\n");
    return CLI_PARSE_OK;
}

int sepCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgIntraVapForwardingSet(pCli->unit, pCli->vap, TRUE);
    printf("Wireless Separate: Disabled\n");
    return CLI_PARSE_OK;
}

int
wmeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("WMM Mode: %s\n", pAbleStr[apCfgWmeGet(pCli->unit, pCli->vap)]);
    return CLI_PARSE_OK;
}

int
wmeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWmeSet(pCli->unit, pCli->vap, FALSE);
    printf("WMM: Disabled\n");
    return CLI_PARSE_OK;
}

int
wmeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWmeSet(pCli->unit, pCli->vap, TRUE);
    printf("WMM: Enabled\n");
    return CLI_PARSE_OK;
}

int
wmeNoAckCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("WMM No Acknowledgement: %s\n", pAbleStr[apCfgNoAckGet(pCli->unit, pCli->vap)]);
    return CLI_PARSE_OK;
}

int
wmeNoAckCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgNoAckSet(pCli->unit, pCli->vap, FALSE);
    printf("WMM No Acknowledgement: Disabled\n");
    return CLI_PARSE_OK;
}

int
wmeNoAckCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgNoAckSet(pCli->unit, pCli->vap, TRUE);
    printf("WMM No Acknowledgement: Enabled\n");
    return CLI_PARSE_OK;
}

int
factoryCmdHandler(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgFactoryRestore();
    apcfg_submit();
    return CLI_PARSE_OK;
}

int
fragCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Fragmentation Length: %d\n", apCfgFragThresholdGet(pCli->unit));
    return CLI_PARSE_OK;
}

int
fragCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    int v;

    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            rc = apCfgFragThresholdSet(pCli->unit, v);
            if (rc != A_OK) {
                printf("Invalid Fragmentation Threshold: %d (must be between %d and %d)\n",
                         v, MIN_FRAG_THRESHOLD, MAX_FRAG_THRESHOLD);
            }
            printf("Fragmentation Length: %d\n", apCfgFragThresholdGet(pCli->unit));
        }
    }
    return CLI_PARSE_ERROR;
}

int
beaconCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Beacon Interval: %d\n", apCfgBeaconIntervalGet(pCli->unit));
    return CLI_PARSE_OK;
}

int
beaconCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    int v;

    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            rc = apCfgBeaconIntervalSet(pCli->unit, v);
            if (rc != A_OK) {
                printf("Invalid Beacon Interval: %d\n", v);
            }
            printf("Beacon Interval: %d\n", apCfgBeaconIntervalGet(pCli->unit));
            return CLI_PARSE_OK;
        }
    }
    return CLI_PARSE_ERROR;
}

int
rtsCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("RTS/CTS Threshold: %d\n", apCfgRtsThresholdGet(pCli->unit));
    return CLI_PARSE_OK;
}

int
rtsCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    int v;

    if ((sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) && (sccli_tokenCount(pCli) == 0)) {
        rc = apCfgRtsThresholdSet(pCli->unit, v);
        if (rc == A_OK) {
            printf("RTS/CTS Threshold: %d\n", apCfgRtsThresholdGet(pCli->unit));
            return CLI_PARSE_OK;
        }
    }
    printf("Invalid RTS/CTS Threshold %d\n", v);
    return CLI_PARSE_ERROR;
}

int
shortPreambleCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Short Preamble: %s\n",
             pAbleStr[apCfgShortPreambleGet(pCli->unit)]);
    return CLI_PARSE_OK;
}

int
shortPreambleCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgShortPreambleSet(pCli->unit, TRUE);
    printf("Short Preamble: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}

int
shortPreambleCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgShortPreambleSet(pCli->unit, FALSE);
    printf("Short Preamble: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}

int
interVapForwardingCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Isolate All Virtual APs State: %s\n", 
             pAbleStr[!apCfgInterVapForwardingGet(pCli->unit)]);
    return CLI_PARSE_OK;
}

int
interVapForwardingCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgInterVapForwardingSet(pCli->unit, TRUE);
    printf("Isolate Between SSID State: Disabled\n");
    return CLI_PARSE_OK;
}

int
interVapForwardingCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgInterVapForwardingSet(pCli->unit, FALSE);
    printf("Isolate All Virtual APs State: Enabled\n");
    return CLI_PARSE_OK;
}

int
vapNameCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Virtual AP Name: %s\n", apCfgVapNameGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int
vapNameCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char     buf[CLI_INBUF];
    A_STATUS rc;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    buf[0] = '\0';
    while (sccli_tokenCount(pCli)) {
        if (buf[0] != '\0') {
            strcat(buf, " ");
        }
        strcat(buf, sccli_tokenPop(pCli));
    }
    
    if(buf[0] == '*')
    {
		printf("Virtual AP name is invalid. It can't begin with '*'.");
    	return CLI_PARSE_ERROR;
    }
    
    rc = apCfgVapNameSet(pCli->unit, pCli->vap, buf);
    if (rc != A_OK) {
        printf("Virtual AP name too long!\n");
    }
    printf("Virtual AP Name: %s\n", apCfgVapNameGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int
ssidCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{

    printf("SSID: %s\n", apCfgSsidGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int
ssidCliCmdSet(CLI *pCli, char *s, struct parse_token_s *pTbl)
{
    char buf[64], *pStr;
    int  ssidLen = 0;
    A_STATUS result;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    buf[0] = '\0';

    pStr = buf;
    while (sccli_tokenCount(pCli)) {
        char *p = sccli_tokenPop(pCli);

        ssidLen += strlen(p) + 1;               /* +1 for null or space */
        if (ssidLen > (MAX_SSID + 1)) {
            printf("SSID string is too long\n");
            return CLI_PARSE_ERROR;
        }

        if (buf[0] != '\0') {                   /* not first word */
            strcat(buf, " ");
        }
        strcat(buf, p);
    }

    result = apCfgSsidSet(pCli->unit, pCli->vap, buf);
    if (result != A_OK) {
        printf("SSID is invalid.\n");
        return CLI_PARSE_ERROR;
    }
    printf("SSID: %s\n",apCfgSsidGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int
ssidModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("SSID Broadcast Mode: %s\n", pAbleStr[(apCfgSsidModeGet(pCli->unit, pCli->vap)+1)%2]);
    return CLI_PARSE_OK;
}

int
ssidModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgSsidModeSet(pCli->unit, pCli->vap, TRUE);
    printf("SSID Broadcast Mode: Disabled\n");
    return CLI_PARSE_OK;
}

int
ssidModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgSsidModeSet(pCli->unit, pCli->vap, FALSE);
    printf("SSID Broadcast Mode: Enabled\n");
    return CLI_PARSE_OK;
}

int
stpModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("STP Mode: %s\n", pAbleStr[apCfgStpGet()]);
    return CLI_PARSE_OK;
}

int
stpModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgStpSet(FALSE);
    printf("STP Mode: Disabled\n");
    return CLI_PARSE_OK;
}

int
stpModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgStpSet(TRUE);
    printf("STP Mode: Enabled\n");
    return CLI_PARSE_OK;
}


int
lltdModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("LLTD Mode: %s\n", pAbleStr[apCfglltdGet()]);
    return CLI_PARSE_OK;
}

int
lltdModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfglltdSet(FALSE);
    printf("LLTD Mode: Disabled\n");
    return CLI_PARSE_OK;
}

int
lltdModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfglltdSet(TRUE);
    printf("LLTD Mode: Enabled\n");
    return CLI_PARSE_OK;
}

int
secCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = cliGetWirelessSecurity(pCli->unit, pCli->vap);
    printf("Wireless Seucrity Mode: %s\n", pSecurityStr[index]);
    return CLI_PARSE_OK;
}

int   
secCliCmdNone(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = 0;
    cliSetWirelessSecurity(pCli->unit, pCli->vap, index);
    printf("Wireless Seucrity Mode: %s\n", pSecurityStr[index]);
    return CLI_PARSE_OK;
}

int   
secCliCmdWpapsk(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = 2;
    cliSetWirelessSecurity(pCli->unit, pCli->vap, index);
    printf("Wireless Seucrity Mode: %s\n", pSecurityStr[index]);
    return CLI_PARSE_OK;
}

int   
secCliCmdWpa2psk(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = 3;
    cliSetWirelessSecurity(pCli->unit, pCli->vap, index);
    printf("Wireless Seucrity Mode: %s\n", pSecurityStr[index]);
    return CLI_PARSE_OK;
}

int   
secCliCmdWpapskauto(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = 4;
    cliSetWirelessSecurity(pCli->unit, pCli->vap, index);
    printf("Wireless Seucrity Mode: %s\n", pSecurityStr[index]);
    return CLI_PARSE_OK;
}

int   
secCliCmdWpa(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = 5;
    cliSetWirelessSecurity(pCli->unit, pCli->vap, index);
    printf("Wireless Seucrity Mode: %s\n", pSecurityStr[index]);
    return CLI_PARSE_OK;
}

int   
secCliCmdWpa2(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = 6;
    cliSetWirelessSecurity(pCli->unit, pCli->vap, index);
    printf("Wireless Seucrity Mode: %s\n", pSecurityStr[index]);
    return CLI_PARSE_OK;
}

int   
secCliCmdWpaauto(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = 7;
    cliSetWirelessSecurity(pCli->unit, pCli->vap, index);
    printf("Wireless Seucrity Mode: %s\n", pSecurityStr[index]);
    return CLI_PARSE_OK;
}

int
authCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Authentication Type: %s\n", pAuthStr[cliGetWEPAuth(pCli->unit, pCli->vap)]);
    return CLI_PARSE_OK;
}

int
authCliCmdOpen(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = 1;
    cliSetWEPAuth(pCli->unit, pCli->vap,index);
    printf("Authentication Type: %s\n", pAuthStr[index]);
    return CLI_PARSE_OK;
}

int
authCliCmdShared(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = 2;
    cliSetWEPAuth(pCli->unit, pCli->vap,index);
    printf("Authentication Type: %s\n", pAuthStr[index]);
    return CLI_PARSE_OK;
}

int
authCliCmdAuto(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = 0;
    cliSetWEPAuth(pCli->unit, pCli->vap,index);
    printf("Authentication Type: %s\n", pAuthStr[index]);
    return CLI_PARSE_OK;
}

int   
keyLengthCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("WEP Key Length: %d\n",apCfgKeyBitLenGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int   
keyLengthCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int v;
	A_STATUS rc;
    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) 
    {
        if (sccli_tokenCount(pCli) == 0 && (v==40 || v==104 || v == 128)) 
        {
            apCfgDot1xKeyLenSet(pCli->unit, pCli->vap, v);   
            rc = apCfgKeyBitLenSet(pCli->unit, pCli->vap, v);
            if (rc == A_OK) 
            {
                printf("WEP Key Length: %d\n",apCfgKeyBitLenGet(pCli->unit, pCli->vap));
                return CLI_PARSE_OK;
            }
        }
    }
    printf("Usage: set keylength 40|104|128\n");
    return CLI_PARSE_ERROR;
}

int   
defaultKeyCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Default Key Index: %d\n",apCfgDefKeyGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int   
defaultKeyCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int v;
	A_STATUS rc;
    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) 
    {
        if (sccli_tokenCount(pCli) == 0 && v>0 && v<5) 
        {
            rc = apCfgDefKeySet(pCli->unit, pCli->vap, v);
            if (rc == A_OK) 
            {
                printf("Default Key Index: %d\n",apCfgDefKeyGet(pCli->unit, pCli->vap));
                return CLI_PARSE_OK;
            }
        }
    }
    printf("Usage: set defaultkey (1-4)\n");
    return CLI_PARSE_ERROR;
}

int
keyCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int i;
    printf("=Index= ===Value===\n");
    for (i = 1; i<5; i++) 
    {
        printf("   %d    %s\n",i,apCfgKeyValGet(pCli->unit, pCli->vap, i));
    }
    return CLI_PARSE_OK;
}

int
keyCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int v;
    char *pStr;
    A_UINT32 keyLen;
    A_STATUS rc;
    
    if(sccli_tokenCount(pCli) == 2)
    {
        if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) 
        {
            if (v>0 && v<5) 
            {
                pStr = sccli_tokenPop(pCli);
                keyLen = strlen(pStr);
                if(keyLen == 10 || keyLen == 26 || keyLen == 32)
                {
                    rc = apCfgKeyValSet(pCli->unit, pCli->vap, v, pStr);
                    if (rc == A_OK) 
                    {
                        printf("Key %d: %s\n",v,apCfgKeyValGet(pCli->unit, pCli->vap, v));
                        return CLI_PARSE_OK;
                    }
                }
                else
                {
                    printf("Invalid key length.\n");
                }
            }
            else
            {
                printf("Invalid key index.\n");
            }
        }
    }
    printf("Usage: set key index(1-4) value(HEX)\n");
    return CLI_PARSE_ERROR;
}


int
pskCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl) 
{
    printf("Pre-shared Key: %s\n", apCfgPassphraseGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int
pskCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl) 
{
    unsigned char passKey[CFG_MAX_PASSPHRASE];
    A_UINT8      passphrase[CLI_INBUF];
    int          bufLen = 0;
    A_STATUS    rc;
    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    if (sccli_tokenCount(pCli) == 1) {
        p = sccli_tokenPop(pCli);
        bufLen = strlen(p);
        
        /* Check for buffer overflow */
        if (sizeof(passphrase) < bufLen) {
            printf("Input Phrase is too long\n");
            return CLI_PARSE_ERROR;
        }
        /* Check for Minimum passphrase length */
        if (bufLen < MIN_PASSPHRASE_SIZE) {
            printf("Input Phrase is too short, should be minimum of 8 characters long.\n");
            return CLI_PARSE_ERROR;
        }
        /* Check for Max size passphrase */
        if (bufLen > CFG_MAX_PASSPHRASE) {
            printf("Input Phrase is too long, should be max of 64 characters long.\n");
            return CLI_PARSE_ERROR;
        }
        strcpy(passphrase, p);
        /* make sure that all 64 characters are HEX */
        if (( bufLen == CFG_MAX_PASSPHRASE ) && (asciiToPassphraseKey(passphrase, passKey, CFG_MAX_PASSPHRASE) != A_OK)) {
            printf("A_ERROR: Invalid HEX passphrase\n");
            return CLI_PARSE_ERROR;
        }

        rc = apCfgPassphraseSet(pCli->unit, pCli->vap, p);
        if (rc != A_OK) {
            printf("Pre-shared Key is too long\n");
        }
        printf("Pre-shared Key: %s\n", apCfgPassphraseGet(pCli->unit, pCli->vap));
        return CLI_PARSE_OK;
    }
    return CLI_PARSE_ERROR;
}

int
groupKeyUpdateCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Group Key Update: %s\n", pAbleStr[scApCfgGroupKeyUpdateEnabledGet(pCli->unit, pCli->vap)]);
    return CLI_PARSE_OK;
}

int
groupKeyCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    scApCfgGroupKeyUpdateEnabledSet(pCli->unit, pCli->vap,FALSE);
    printf("Group Key Update: %s\n", pAbleStr[scApCfgGroupKeyUpdateEnabledGet(pCli->unit, pCli->vap)]);
    return CLI_PARSE_OK;
}

int
groupKeyCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    scApCfgGroupKeyUpdateEnabledSet(pCli->unit, pCli->vap,TRUE);
    printf("Group Key Update: %s\n", pAbleStr[scApCfgGroupKeyUpdateEnabledGet(pCli->unit, pCli->vap)]);
    return CLI_PARSE_OK;
}

int
groupKeyUpdateIntervalCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Group Key Update Interval: %d Seconds\n", apCfgGroupKeyUpdateIntervalGet(pCli->unit, pCli->vap)/60);
    return CLI_PARSE_OK;
}

int
groupKeyUpdateIntervalCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    int v;

    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            if(v<10 || v>600)
            {
                printf("Group Key Update Interval range invalid\n");
                return CLI_PARSE_ERROR;
            }
            rc = apCfgGroupKeyUpdateIntervalSet(pCli->unit, pCli->vap, v*60);
            if (rc != A_OK) {
                printf("Unable to set Group Key Update interval.\n");
            }
            printf("Group Key Update Interval: %d seconds\n", apCfgGroupKeyUpdateIntervalGet(pCli->unit, pCli->vap));
            return CLI_PARSE_OK;
        }
    }
    return CLI_PARSE_ERROR;
}

int
gtkUpdateStrictCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Strict Group Key Update: %s\n", pAbleStr[scApCfgGroupKeyUpdateTerminatedGet(pCli->unit, pCli->vap)]);
    return CLI_PARSE_OK;
}

int
gtkUpdateStrictCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    scApCfgGroupKeyUpdateTerminatedSet(pCli->unit, pCli->vap,FALSE);
    printf("Strict Group Key Update: %s\n", pAbleStr[scApCfgGroupKeyUpdateTerminatedGet(pCli->unit, pCli->vap)]);
    return CLI_PARSE_OK;
}

int
gtkUpdateStrictCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    scApCfgGroupKeyUpdateTerminatedSet(pCli->unit, pCli->vap, TRUE);
    printf("Strict Group Key Update: %s\n", pAbleStr[scApCfgGroupKeyUpdateTerminatedGet(pCli->unit, pCli->vap)]);
    return CLI_PARSE_OK;
}

int
radiusSrvCliCmdGetPri(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Primary RADIUS Address: %s\n", apCfgRadiusServerGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int
radiusSrvCliCmdSetPri(CLI *pCli, char *p, struct parse_token_s *pTbl)
{

    switch (sccli_tokenCount(pCli)) {
    case 0:
        printf("Input Null.\n");
        return CLI_PARSE_NO_VALUE;
    case 1:
        {    
            struct in_addr iaddr;
       
            if (sccli_tokenGetNetAddrHandler(pCli, &iaddr) != A_ERROR) {
                if (sccli_tokenCount(pCli) == 0) {
                    char netstr[20];
                    
                    strcpy(netstr, inet_ntoa(iaddr)); 
                    if (iaddr.s_addr) {
                        int i = 0;
                        for(i=0; i<WLAN_MAX_VAP; i++)
                            apCfgRadiusServerSet(pCli->unit, i, netstr);   
                        printf("RADIUS Server IP Address: %s\n", apCfgRadiusServerGet(pCli->unit, pCli->vap));
                        return CLI_PARSE_OK;
                    }
                }
            }
            printf("Invalid RADIUS Server IP Address\n");
        }      
    }
    return CLI_PARSE_ERROR;  
}


int
radiusSrvCliCmdGetBck(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Backup RADIUS Address: %s\n", apCfgBackupRadiusServerGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int
radiusSrvCliCmdSetBck(CLI *pCli, char *p, struct parse_token_s *pTbl)
{

    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        {    
            struct in_addr iaddr;
       
            if (sccli_tokenGetNetAddrHandler(pCli, &iaddr) != A_ERROR) {
                if (sccli_tokenCount(pCli) == 0) {
                    char netstr[20];
                    
                    strcpy(netstr, inet_ntoa(iaddr)); 
                    if (iaddr.s_addr) {
                        int i = 0;
                        for(i=0; i<WLAN_MAX_VAP; i++)
                            apCfgBackupRadiusServerSet(pCli->unit, i, netstr);   
                        printf("Backup RADIUS Server IP Address: %s\n", apCfgBackupRadiusServerGet(pCli->unit, pCli->vap));
                        return CLI_PARSE_OK;
                    }
                }
            }
            printf("Invalid RADIUS Server IP Address\n");
        }      
    }
    return CLI_PARSE_ERROR;  
}

int 
radiusPortCliCmdGetPri(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Primary RADIUS Port: %d\n", apCfgRadiusPortGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int
radiusPortCliCmdSetPri(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int port;
    int i;
    
    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        if (sccli_tokenPopNum(pCli, &port, 10) != CLI_PARSE_OK) {
            return CLI_PARSE_ERROR;
        }
        for(i=0; i<WLAN_MAX_VAP; i++)
            apCfgRadiusPortSet(pCli->unit, i, port);
        printf("Primary RADIUS Port Number: %d\n", apCfgRadiusPortGet(pCli->unit, pCli->vap));
        return CLI_PARSE_OK;
    }
    return CLI_PARSE_ERROR;
}

int 
radiusPortCliCmdGetBck(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Backup RADIUS Port: %d\n", apCfgBackupRadiusPortGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int
radiusPortCliCmdSetBck(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int port;
    int i;
    
    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        if (sccli_tokenPopNum(pCli, &port, 10) != CLI_PARSE_OK) {
            return CLI_PARSE_ERROR;
        }
        for(i=0; i<WLAN_MAX_VAP; i++)
            apCfgBackupRadiusPortSet(pCli->unit, i, port);
        printf("Backup RADIUS Port Number: %d\n", apCfgBackupRadiusPortGet(pCli->unit, pCli->vap));
        return CLI_PARSE_OK;
    }
    return CLI_PARSE_ERROR;
}

int 
radiusSecretCliCmdGetPri(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Primary RADIUS Server secret: %s\n", apCfgRadiusSecretGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int
radiusSecretCliCmdSetPri(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS     rc;

    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        {    
            char *pStr = sccli_tokenPop(pCli);
    
            rc = apCfgRadiusSecretSet(pCli->unit, pCli->vap, pStr);
            
            if (rc == A_OK) {
                printf("Primary Radius Server secret: %s\n", apCfgRadiusSecretGet(pCli->unit, pCli->vap));
                return CLI_PARSE_OK;
            }
            printf("Primary Radius Server secret is too long\n");
        }      
    }
    return CLI_PARSE_ERROR;
}


int 
radiusSecretCliCmdGetBck(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Backup RADIUS Server secret: %s\n", apCfgBackupRadiusSecretGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int
radiusSecretCliCmdSetBck(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS     rc;

    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        {    
            char *pStr = sccli_tokenPop(pCli);
    
            rc = apCfgBackupRadiusSecretSet(pCli->unit, pCli->vap, pStr);
            
            if (rc == A_OK) {
                printf("Backup Radius Server secret: %s\n", apCfgBackupRadiusSecretGet(pCli->unit, pCli->vap));
                return CLI_PARSE_OK;
            }
            printf("Backup Radius Server secret is too long\n");
        }      
    }
    return CLI_PARSE_ERROR;
}

int   
dot1xKeyCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("802.1x Distribute Key Method: %s\n",(apCfgDot1xKeyModeGet(pCli->unit, pCli->vap) & DOT1X_MODE_DYNAMIC)? "Dynamic":"Static");
    return CLI_PARSE_OK;
}

int   
dot1xKeyCliCmdDyn(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgAuthTypeSet(pCli->unit, pCli->vap,APCFG_AUTH_DOT1X);
    apCfgDot1xKeyModeSet(pCli->unit, pCli->vap, DOT1X_MODE_DYNAMIC);
    printf("802.1x Distribute Key Method: Dynamic\n");
    return CLI_PARSE_OK;
}

int   
dot1xKeyCliCmdSta(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgAuthTypeSet(pCli->unit, pCli->vap,APCFG_AUTH_DOT1X);
    apCfgDot1xKeyModeSet(pCli->unit, pCli->vap, DOT1X_MODE_STATIC);
    printf("802.1x Distribute Key Method: Static\n");
    return CLI_PARSE_OK;
}

int   
dot1xKeyUpdateCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("802.1x Dynamic Key Update: %s\n", pAbleStr[scApCfgGroupKeyUpdateEnabledGet(pCli->unit ,pCli->vap)]);
    return CLI_PARSE_OK;
}

int   
dot1xKeyUpdateCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    scApCfgGroupKeyUpdateEnabledSet(pCli->unit ,pCli->vap, 0);
    printf("802.1x Dynamic Key Update: Disabled\n");
    return CLI_PARSE_OK;
}

int   
dot1xKeyUpdateCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    scApCfgGroupKeyUpdateEnabledSet(pCli->unit ,pCli->vap, 1);
    printf("802.1x Dynamic Key Update: Enabled\n");
    return CLI_PARSE_OK;
}

int   
dot1xKeyLifeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("802.1x Dynamic Key Update Interval: %d Minutes\n", apCfgGroupKeyUpdateIntervalGet(pCli->unit ,pCli->vap)/60);
    return CLI_PARSE_OK;

}

int   
dot1xKeyLifeCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int interval;

    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        if (sccli_tokenPopNum(pCli, &interval, 10) != CLI_PARSE_OK) {
            return CLI_PARSE_ERROR;
        }
        if(interval<10 || interval>600)
        {
            printf("802.1x dynamic Key update interval range invalid\n");
            return CLI_PARSE_ERROR;
        }
        apCfgGroupKeyUpdateIntervalSet(pCli->unit ,pCli->vap, interval*60);
        printf("802.1x Dynamic Key Update Interval: %d Minutes\n", apCfgGroupKeyUpdateIntervalGet(pCli->unit ,pCli->vap)/60);
        return CLI_PARSE_OK;
    }
    return CLI_PARSE_ERROR;
}

int
usernameCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Login Username: %s\n", apCfgLoginGet());
    return CLI_PARSE_OK;
}

int
usernameCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    if (sccli_tokenCount(pCli) == 1) {
        p = sccli_tokenPop(pCli);
        if (strcmp(CLI_TOKEN_NULL_STRING, p) == 0) {
            printf("Username can't be blank\n");
            return CLI_PARSE_ERROR;
        } else {
            rc = apCfgLoginSet(p);
        }
        if (rc != A_OK) {
            printf("Username is too long\n");
        }
        printf("Login Username: %s\n", apCfgLoginGet());
        return CLI_PARSE_OK;
    }
    return CLI_PARSE_ERROR;
}
int
passwordCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Login Password: %s\n", apCfgPasswordGet());
    return CLI_PARSE_OK;
}

int
passwordCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    if (sccli_tokenCount(pCli) == 1) {
        p = sccli_tokenPop(pCli);
        if (strcmp(CLI_TOKEN_NULL_STRING, p) == 0) {
            printf("Login Password can't be blank\n");
            return CLI_PARSE_ERROR;
        } else {
            rc = apCfgPasswordSet(p);
        }
        if (rc != A_OK) {
            printf("Login Password is too long\n");
        }
        printf("Login Password: %s\n", apCfgPasswordGet());
        return CLI_PARSE_OK;
    }
    return CLI_PARSE_ERROR;
}

int 
httpCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("HTTP: %s\n", pAbleStr[apCfgHttpModeGet()]);
    return CLI_PARSE_OK;
}

int 
httpsCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("HTTPS: %s\n", pAbleStr[apCfgHttpsModeGet()]);
    return CLI_PARSE_OK;
}

int 
httpCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgHttpModeSet(0);
    printf("HTTP: Disabled\n");
    return CLI_PARSE_OK;
}

int 
httpCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgHttpModeSet(1);
    printf("HTTP: Enabled\n");
    return CLI_PARSE_OK;
}

int 
httpsCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgHttpsModeSet(0);
    printf("HTTPS: Disabled\n");
    return CLI_PARSE_OK;
}

int 
httpsCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgHttpsModeSet(1);
    printf("HTTPS: Enabled\n");
    return CLI_PARSE_OK;
}

int 
httpPortCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("HTTP Port Number: %d\n", apCfgHttpPortGet());
    return CLI_PARSE_OK;
}

int 
httpsPortCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("HTTPS Port Number: %d\n", apCfgHttpsPortGet());
    return CLI_PARSE_OK;
}


int 
httpPortCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int port;

    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        if (sccli_tokenPopNum(pCli, &port, 10) != CLI_PARSE_OK) {
            return CLI_PARSE_ERROR;
        }
        if( port != apCfgHttpsPortGet()){
            if (scInvalidHttpPort(port, 1)) {
            	printf("The well-known port can not be used.\n");
    	   		return CLI_PARSE_ERROR;
            }
            apCfgHttpPortSet(port);
            printf("HTTP Port Number: %d\n", apCfgHttpPortGet());
            return CLI_PARSE_OK;
	   	}else{
	   		printf("HTTP and HTTPS must use different port numbers.\n");
	   		printf("HTTP Port Number: %d\n", apCfgHttpPortGet());
	   		return CLI_PARSE_ERROR;
	   	}     
    }
    return CLI_PARSE_ERROR;
}

static int httpsPortCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int port;

    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        if (sccli_tokenPopNum(pCli, &port, 10) != CLI_PARSE_OK) {
            return CLI_PARSE_ERROR;
        }
        if( port != apCfgHttpPortGet()){
            if (scInvalidHttpPort(port, 2)) {
            	printf("The well-known port can not be used.\n");
    	   		return CLI_PARSE_ERROR;
            }
    
            apCfgHttpsPortSet(port);
            printf("HTTPS Port Number: %d\n", apCfgHttpsPortGet());
            return CLI_PARSE_OK;
	   	}else{
	   		printf("HTTP and HTTPS must use different port numbers.\n");
	   		printf("HTTPS Port Number: %d\n", apCfgHttpsPortGet());
	        return CLI_PARSE_ERROR;
	   	}     
    }
    return CLI_PARSE_ERROR;
}

int
telnetCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Telnet Access: %s\n", pAbleStr[apCfgTelnetModeGet()]);
    return CLI_PARSE_OK;
}

int
telnetCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgTelnetModeSet(TRUE);
    printf("Telnet Access: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}
int
telnetCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgTelnetModeSet(FALSE);
    printf("Telnet Access: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}

int
timezoneCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Time Zone: %s\n", apCfgTimezoneOffsetGet());
    return CLI_PARSE_OK;
}

int
timezoneCliCmdSet(CLI *pCli, char *pToken, struct parse_token_s *pNextTbl)
{
    scToUppers(pToken);
    if (sccli_tokenCount(pCli) == 0) {
        struct parse_token_s *pTbl = sccli_tokenCurTblGet();

        for ( ; pTbl->fHandler; pTbl++) {
            if (strcmp(pToken, pTbl->pCmd) == 0) {
                
                apCfgTimezoneOffsetSet(pTbl->pCmd);
                printf("Time Zone: %s\n", apCfgTimezoneOffsetGet());
                return CLI_PARSE_OK;
            }
        }
    }
    
    printf("Invalid Time Zone\n");
    return CLI_PARSE_ERROR;
}

int 
snmpReadComCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("SNMP Read Community: %s\n",apCfgSnmpReadCommGet());
    return CLI_PARSE_OK;
}

int 
snmpReadComCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS     rc;

    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        {    
            char *pStr = sccli_tokenPop(pCli);
    
            rc = apCfgSnmpReadCommSet(pStr);
            
            if (rc == A_OK) {
                printf("SNMP Read Community: %s\n", apCfgSnmpReadCommGet());
                return CLI_PARSE_OK;
            }
            printf("SNMP Read Community is too long\n");
        }      
    }
    return CLI_PARSE_ERROR;
}

int 
snmpWriteComCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("SNMP Write Community: %s\n",apCfgSnmpWriteCommGet());
    return CLI_PARSE_OK;
}

int 
snmpWriteComCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS     rc;

    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        {    
            char *pStr = sccli_tokenPop(pCli);
    
            rc = apCfgSnmpWriteCommSet(pStr);
            
            if (rc == A_OK) {
                printf("SNMP Write Community: %s\n", apCfgSnmpWriteCommGet());
                return CLI_PARSE_OK;
            }
            printf("SNMP Write Community is too long\n");
        }      
    }
    return CLI_PARSE_ERROR;
}

int 
snmpModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("SNMP Mode: %s\n",pAbleStr[apCfgSnmpModeGet()]);
    return CLI_PARSE_OK;
}

int 
snmpCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgSnmpModeSet(TRUE);
    printf("SNMP: Enabled\n");
    return CLI_PARSE_OK;
}

int 
snmpCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgSnmpModeSet(FALSE);
    printf("SNMP: Disabled\n");
    return CLI_PARSE_OK;
}

int 
snmpManageCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    
    if(apCfgSnmpAnyManagerGet()){
         printf("SNMP Manager: Any Station\n");
    }else{
        char netstr[20];
        struct in_addr iaddr;
        iaddr.s_addr = apCfgSnmpManagerIpGet();
        if(iaddr.s_addr){
            strcpy(netstr, inet_ntoa(iaddr)); 
            printf("SNMP Manager IP Address from: %s", netstr);
        }
        iaddr.s_addr = apCfgSnmpManagerIpEndGet();
        if(iaddr.s_addr){
            strcpy(netstr, inet_ntoa(iaddr)); 
            printf(" to %s\n", netstr);
        }		
    }
    return CLI_PARSE_OK;
}

int 
snmpManageCliCmdAny(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgSnmpAnyManagerSet(TRUE);
    printf("SNMP Manager: Any Station\n");
    return CLI_PARSE_OK;
}

int 
snmpManageCliCmdIpStart(CLI *pCli, char *p, struct parse_token_s *pTbl)
{    
    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        {    
            struct in_addr iaddr;
            if (sccli_tokenGetNetAddrHandler(pCli, &iaddr) != A_ERROR) {
                char netstr[20];
        
                strcpy(netstr, inet_ntoa(iaddr)); 
                if (iaddr.s_addr) {
                    apCfgSnmpManagerIpSet(iaddr.s_addr);
                    printf("SNMP Manager IP Address Start: %s\n", netstr);
                    return CLI_PARSE_OK;
                }
                printf("Invalid Start IP Address %s\n", netstr);
                return CLI_PARSE_ERROR;
            }
            printf("Invalid Start IP Address\n");
            return CLI_PARSE_ERROR;
        }      
    }
    return CLI_PARSE_ERROR;
}

int 
snmpManageCliCmdIpEnd(CLI *pCli, char *p, struct parse_token_s *pTbl)
{    
    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        {    
            struct in_addr iaddr;
            if (sccli_tokenGetNetAddrHandler(pCli, &iaddr) != A_ERROR) {
                char netstr[20];
        
                strcpy(netstr, inet_ntoa(iaddr)); 
                if (iaddr.s_addr) {
                    apCfgSnmpManagerIpEndSet(iaddr.s_addr);
                    printf("SNMP Manager IP Address End: %s\n", netstr);
                    return CLI_PARSE_OK;
                }
                printf("Invalid End IP Address %s\n", netstr);
                return CLI_PARSE_ERROR;
            }
            printf("Invalid End IP Address\n");
            return CLI_PARSE_ERROR;
        }      
    }
    return CLI_PARSE_ERROR;
}

int 
snmpTrapCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    
    if(apCfgSnmpAnyManagerGet()){
         printf("SNMP Manager: Any Station\n");
    }else{
        char netstr[20];
        struct in_addr iaddr;
        iaddr.s_addr = apCfgSnmpManagerIpGet();
        if(iaddr.s_addr){
            strcpy(netstr, inet_ntoa(iaddr)); 
            printf("SNMP Manager: %s\n", netstr);
        }else{
            printf("SNMP Manager: Any Station\n");
            apCfgSnmpAnyManagerSet(TRUE);
        }
    }
    return CLI_PARSE_OK;
}

int 
snmpTrapCliCmdBroadcast(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char *broadcastIpStr = "255.255.255.255";

    apCfgSnmpTrapRecvIpSet(inet_addr(broadcastIpStr));
    printf("SNMP Trap: broadcast\n");
    return CLI_PARSE_OK;
}

int 
snmpTrapCliCmdUnicast(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        {    
            struct in_addr iaddr;
            if (sccli_tokenGetNetAddrHandler(pCli, &iaddr) != A_ERROR) {
                char netstr[20];
        
                strcpy(netstr, inet_ntoa(iaddr)); 
                if (iaddr.s_addr) {
                    apCfgSnmpTrapRecvIpSet(iaddr.s_addr);
                    printf("SNMP Trap to IP Address: %s\n", netstr);
                    return CLI_PARSE_OK;
                }
                printf("Invalid IP Address %s\n", netstr);
                return CLI_PARSE_ERROR;
            }
            printf("Invalid IP Address\n");
            return CLI_PARSE_ERROR;
        }      
    }
    return CLI_PARSE_ERROR;
}

int 
snmpTrapVerCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("SNMP Trap Version: %s\n",(apCfgSnmpTrapVersionGet()==1)? "v1":
            (apCfgSnmpTrapVersionGet()==2? "v2c":"v3"));
    return CLI_PARSE_OK;
}

int 
snmpTrapVerCliCmdV1(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgSnmpTrapVersionSet(1);
    printf("SNMP Trap Version:  v1\n");
    return CLI_PARSE_OK;
}

int 
snmpTrapVerCliCmdV2c(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgSnmpTrapVersionSet(2);
    printf("SNMP Trap Version: v2c\n");
    return CLI_PARSE_OK;
}


int 
snmpTrapVerCliCmdV3(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgSnmpTrapVersionSet(3);
    printf("SNMP Trap Version: v3\n");
    return CLI_PARSE_OK;
}

int 
snmpV3UserNameCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    if(strlen(apCfgSnmpUserNameGet()))
        printf("SNMP v3 User Name: %s\n", apCfgSnmpUserNameGet());
    else
        printf("None SNMP v3 User Name\n");    
    return CLI_PARSE_OK;
}

int 
snmpV3UserNameCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    if (sccli_tokenCount(pCli) == 1) {
        p = sccli_tokenPop(pCli);
        if (strcmp(CLI_TOKEN_NULL_STRING, p) == 0) {
            printf("Username can't be blank\n");
            return CLI_PARSE_ERROR;
        } else {
            rc = apCfgSnmpUserNameSet(p);
        }
        if (rc != A_OK) {
            printf("Username is too long\n");
        }
        printf("SNMP v3 Username: %s\n", apCfgLoginGet());
        return CLI_PARSE_OK;
    }
    return CLI_PARSE_ERROR;
}

int 
snmpV3AuthProtoCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("SNMP v3 Authentication Protocol: %s\n", 
        apCfgSnmpAuthProtocolGet()==SNMP_AUTH_NONE? "None" : "HMAC-MD5");
    return CLI_PARSE_OK;
}

int 
snmpV3AuthProtoCliCmdNone(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgSnmpAuthProtocolSet(SNMP_AUTH_NONE);
    printf("SNMP v3 Authentication Protocol: None\n");
    return CLI_PARSE_OK;
}

int 
snmpV3AuthProtoCliCmdMd5(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgSnmpAuthProtocolSet(SNMP_AUTH_MD5);
    printf("SNMP v3 Authentication Protocol: HMAC-MD5\n");
    return CLI_PARSE_OK;
}

int 
snmpV3AuthKeyCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    if(strlen(apCfgSnmpAuthKeyGet()))
        printf("SNMP v3 Authentication Key: %s\n", apCfgSnmpAuthKeyGet());
    else
        printf("None SNMP v3 Authentication Key\n");
    return CLI_PARSE_OK;
}

int 
snmpV3AuthKeyCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    if (sccli_tokenCount(pCli) == 1) {
        p = sccli_tokenPop(pCli);

        rc = apCfgSnmpAuthKeySet(p);
        if (rc != A_OK) {
            printf("SNMP v3 Authentication Key is too long\n");
        }
        printf("SNMP v3 Authentication Key: %s\n", apCfgSnmpAuthKeyGet());
        return CLI_PARSE_OK;
    }
    return CLI_PARSE_ERROR;
}

int 
snmpV3PrivProtoCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("SNMP v3 Private Protocol: %s\n", 
        apCfgSnmpPrivProtocolGet()==SNMP_PRIV_NONE? "None" : "CBC-DES");
    return CLI_PARSE_OK;
}

int 
snmpV3PrivProtoCliCmdNone(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgSnmpPrivProtocolSet(SNMP_PRIV_NONE);
    printf("SNMP v3 Private Protocol: None\n");
    return CLI_PARSE_OK;
}

int 
snmpV3PrivProtoCliCmdDes(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgSnmpPrivProtocolSet(SNMP_PRIV_DES);
    printf("SNMP v3 Private Protocol: CBC-DES\n");
    return CLI_PARSE_OK;
}
int 
snmpV3PrivKeyCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    if(strlen(apCfgSnmpPrivKeyGet()))
        printf("SNMP v3 Private Key: %s\n", apCfgSnmpPrivKeyGet());
    else
        printf("None SNMP v3 Private Key\n");
    return CLI_PARSE_OK;
}

int 
snmpV3PrivKeyCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    if (sccli_tokenCount(pCli) == 1) {
        p = sccli_tokenPop(pCli);

        rc = apCfgSnmpPrivKeySet(p);
        if (rc != A_OK) {
            printf("SNMP v3 Private Key is too long\n");
        }
        printf("SNMP v3 Private Key: %s\n", apCfgSnmpPrivKeyGet());
        return CLI_PARSE_OK;
    }
    return CLI_PARSE_ERROR;
}

int 
syslogCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
#ifdef AP101NA_LINKSYS
    printf("Syslog: %s\n", pAbleStr[apCfgsysLogEnabledGet()]);
    return CLI_PARSE_OK;
#else    
    int syslogType = (apCfgsysLogEnabledGet() == 0)?0:(apCfgsysLogBroadcastGet()?1:2);    
    switch(syslogType){
        case 0:
        default:
            printf("Syslog: Disabled\n");
            return CLI_PARSE_OK;
        case 1:
            printf("Syslog: Broadcast\n");
            return CLI_PARSE_OK;
        case 2:
            printf("Syslog: Unicast\n");
            return CLI_PARSE_OK; 
    }  
#endif         
} 
   
int 
syslogCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgsysLogEnabledSet(0);    
    printf("Syslog: Disabled\n");
    return CLI_PARSE_OK;       
}
#ifdef AP101NA_LINKSYS
int 
syslogCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgsysLogEnabledSet(1); 
    apCfgsysLogSeveritySet(6);
    printf("Syslog: Enabled\n");
    return CLI_PARSE_OK;       
}
#endif
int 
syslogCliCmdBroadcast(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgsysLogEnabledSet(1);    
    apCfgsysLogBroadcastSet(1);
    printf("Syslog: Broadcast\n");
    return CLI_PARSE_OK;       
} 

int 
syslogCliCmdUnicast(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgsysLogEnabledSet(1);
    apCfgsysLogBroadcastSet(0);
    printf("Syslog: Unicast\n");
    return CLI_PARSE_OK;       
} 

int 
syslogPortCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Syslog Port: %d\n", apCfgsysLogServerPortGet());
    return CLI_PARSE_OK;       
}

int 
syslogPortCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int port;
    A_STATUS rc;
    
    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        if (sccli_tokenPopNum(pCli, &port, 10) != CLI_PARSE_OK) {
            return CLI_PARSE_ERROR;
        }
        rc = apCfgsysLogServerPortSet(port);
        if (rc != A_OK) 
        {
            printf("Invalid Syslog Port\n");
            return CLI_PARSE_ERROR;
        }
        printf("Syslog Port: %d\n", apCfgsysLogServerPortGet());
        return CLI_PARSE_OK;    
    }
    return CLI_PARSE_ERROR;   
}

int 
syslogSeverityCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    switch(apCfgsysLogSeverityGet()){
        case 1:
            printf("Syslog Severity Level: 1 - Alert Messages\n");
            return CLI_PARSE_OK;
        case 2:
            printf("Syslog Severity Level: 2 - Critical Messages\n");
            return CLI_PARSE_OK;
        case 3:
        default:
            printf("Syslog Severity Level: 3 - Error Messages\n");
            return CLI_PARSE_OK;
        case 4:
            printf("Syslog Severity Level: 4 - Warning Messages\n");
            return CLI_PARSE_OK;
        case 5:
            printf("Syslog Severity Level: 5 - Notice Messages\n");
            return CLI_PARSE_OK;
        case 6:
            printf("Syslog Severity Level: 6 - Informational Messages\n");
            return CLI_PARSE_OK;
    } 
}  
  
int 
syslogSeverityCliCmdSet(CLI *pCli, char *pToken, struct parse_token_s *pNxtTbl)
{
    A_UINT32 v;
    
    if (sccli_tokenCount(pCli) == 0) {
        struct parse_token_s *pTbl = sccli_tokenCurTblGet();

        for ( ; pTbl->fHandler; pTbl++) {
            if (strcmp(pToken, pTbl->pCmd) == 0) {
                
                v = atoi(pToken);
                if (v <0 || v >6) {
                    printf("Invalid Syslog Severity Level\n");
                    return CLI_PARSE_ERROR;
                }
                apCfgsysLogSeveritySet(v); 
                printf("Syslog Severity Level: %ld\n",apCfgsysLogSeverityGet());
                return CLI_PARSE_OK;
            }
        }
    }
    
    printf("Invalid Syslog Severity Level\n");
    return CLI_PARSE_ERROR;
}

int 
syslogSrvCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    if(strlen(apCfgsysLogServerGet()))
        printf("Syslog Server: %s\n",apCfgsysLogServerGet());
    else
        printf("Syslog server: None\n"); 
    return CLI_PARSE_OK;       
}  
  
int 
syslogSrvCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    switch (sccli_tokenCount(pCli)) {
        case 0:
            return CLI_PARSE_NO_VALUE;
        case 1:
            rc = apCfgsysLogServerSet(sccli_tokenPop(pCli));
            if (rc != A_OK) 
            {
                printf("Invalid Syslog Server Address\n");
                return CLI_PARSE_ERROR;
            }
            printf("Syslog Server Address: %s\n", apCfgsysLogServerGet());
            return CLI_PARSE_OK;
    }
    return CLI_PARSE_ERROR;
}

static int rogueCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Rogue AP Detection: %s\n", pAbleStr[scApCfgRogueDetectGet()]);
    return CLI_PARSE_OK; 
}    
static int roguedetectCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    scApCfgRogueDetectSet(0);
    printf("Rogue AP Detection: %s\n", pAbleStr[scApCfgRogueDetectGet()]);
    return CLI_PARSE_OK;
}    
static int roguedetectCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    scApCfgRogueDetectSet(1);
    printf("Rogue AP Detection: %s\n", pAbleStr[scApCfgRogueDetectGet()]);
    return CLI_PARSE_OK;
}   
static int rogueIntervalCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Rogue AP Detection every %d minutes\n", scApCfgRogueDetectIntGet());
    return CLI_PARSE_OK;
}    
static int rogueIntervalCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int v;
    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {            
            if (v <3 || v >99) {
                printf("range of Rogue AP Detection is 3 ~ 99\n");
                return CLI_PARSE_ERROR;
            }
            scApCfgRogueDetectIntSet(v); 
            printf("Rogue AP Detection every %d minutes\n",scApCfgRogueDetectIntGet());
            return CLI_PARSE_OK;
        }
    }
    printf("Invalid Input\n");
    return CLI_PARSE_ERROR;
}

static int roguetypeCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    switch(scApCfgRogueApTypeGet()){
        case 1:
        default:
            printf("Rogue AP type: No Security\n");
            return CLI_PARSE_OK;
        case 2:
            printf("Rogue AP type: Not in Legal AP List\n");
            return CLI_PARSE_OK;
        case 3:
            printf("Rogue AP type: No Security and Not in Legal AP List\n");
            return CLI_PARSE_OK;
    }    
}       
static int roguetype1CliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    scApCfgRogueApTypeSet(1);
    switch(scApCfgRogueApTypeGet()){
        case 1:        
            printf("Rogue AP type: No Security\n");
            return CLI_PARSE_OK;
        case 2:
        case 3:
        default:
            printf("failed to define Rogue AP type as No Security\n");
            return CLI_PARSE_ERROR;
    }
}    
static int roguetype2CliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    scApCfgRogueApTypeSet(2);
    switch(scApCfgRogueApTypeGet()){
        case 2:        
            printf("Rogue AP type: Not in Legal AP List\n");
            return CLI_PARSE_OK;
        case 1:
        case 3:
        default:
            printf("failed to define Rogue AP type as Not in Legal AP List\n");
            return CLI_PARSE_ERROR;
    }
}    
static int roguetype3CliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    scApCfgRogueApTypeSet(3);
    switch(scApCfgRogueApTypeGet()){
        case 3:        
            printf("Rogue AP type: No Security and Not in Legal AP List\n");
            return CLI_PARSE_OK;
        case 1:
        case 2:
        default:
            printf("failed to define Rogue AP type as No Security and Not in Legal AP List\n");
            return CLI_PARSE_ERROR;
    }
}    
static int roguesnmpCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Rogue AP Detection SNMP Trap: %s\n", ((scApCfgRogueSendLogGet() & 0x4)==0x4)? pAbleStr[1] : pAbleStr[0]);
    return CLI_PARSE_OK;
}    
static int roguesnmpCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    scApCfgRogueDetectSet(0x3);
    printf("Rogue AP Detection SNMP Trap: %s\n", ((scApCfgRogueSendLogGet() & 0x4)==0x4)? pAbleStr[1] : pAbleStr[0]);
    return CLI_PARSE_OK;
} 
static int roguesnmpCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    scApCfgRogueDetectSet(0x7);
    printf("Rogue AP Detection SNMP Trap: %s\n", ((scApCfgRogueSendLogGet() & 0x4)==0x4)? pAbleStr[1] : pAbleStr[0]);
    return CLI_PARSE_OK;
} 

static int roguelegalCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char *pList = scApCfgLegalApListGet();
    char *delim= ",";
    char *pValue = NULL;
    
    if (!strlen(pList)){
    	printf("Legal AP/OUI List is empty\n");
    }else{
        pValue = strtok(pList, delim);
        
        printf("Legal AP/OUI List:\n");
        printf("\t%s\n", pValue);
    	while((pValue = strtok(NULL, delim)) != NULL)
    	    printf("\t%s\n", pValue);
    }
    return CLI_PARSE_OK;
}
 
static int roguelegalCmdAdd(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char    macOrOui[13], tokenBuf[20], *pToken; 
    int 	len;
      
    if (sccli_tokenCount(pCli) != 1) {
        return CLI_PARSE_ERROR;
    }

    pToken = sccli_tokenPop(pCli);
    len = strlen(pToken);
    
    if(len!=6 && len!=8 && len!=12 && len!=17){
        printf("Invalid input\n");
        return CLI_PARSE_ERROR;
    }
    
    strcpy(tokenBuf, pToken);
    if(6 == len)
    	strcat(tokenBuf,"******");
    else if(8 == len)
    	strcat(tokenBuf,":**:**:**");
    
    if(len == 8 || len == 17)
        scMacStr17ToStr12(tokenBuf, macOrOui);
    
    if(!scValidHexs(macOrOui, (len<10)? 6:12)){
        printf("Invalid input\n");
        return CLI_PARSE_ERROR;
    }
    
    scToUppers(macOrOui);
    
    if(scApCfgLegalApListAdd(macOrOui) != A_OK){
        printf("Add legal Mac/Oui Failed\n");
        return CLI_PARSE_ERROR;
    }
    printf("Add legal Mac/Oui :%s\n",macOrOui);
    return CLI_PARSE_OK;    
}    
static int roguelegalCmdDel(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
 	char    macOrOui[13], tokenBuf[20], *pToken; 
    int 	len;
      
    if (sccli_tokenCount(pCli) != 1) {
        return CLI_PARSE_ERROR;
    }

    pToken = sccli_tokenPop(pCli);
    len = strlen(pToken);
    
    if(len!=6 && len!=8 && len!=12 && len!=17){
        printf("Invalid input\n");
        return CLI_PARSE_ERROR;
    }
    
    strcpy(tokenBuf, pToken);
    if(6 == len)
    	strcat(tokenBuf,"******");
    else if(8 == len)
    	strcat(tokenBuf,":**:**:**");
    
    if(len == 8 || len == 17)
        scMacStr17ToStr12(tokenBuf, macOrOui);
    
    if(!scValidHexs(macOrOui, (len<10)? 6:12)){
        printf("Invalid input\n");
        return CLI_PARSE_ERROR;
    }
    
    scToUppers(macOrOui);
    
    if(scApCfgLegalApListDel(macOrOui) != A_OK){
        printf("Delete legal Mac/Oui Failed\n");
         return CLI_PARSE_ERROR;
    }
    printf("Delete legal Mac/Oui :\t%s\n",macOrOui);   
        
    return CLI_PARSE_OK;    
}    
