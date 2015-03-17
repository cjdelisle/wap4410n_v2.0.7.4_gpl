/* 
 *  Linux CLI solution, porting from Atheros VxWorks SDK.
 *                                          Terry_Yang@SDC.SerComm.com
 */
#include "../../include/apcfg.h"
#include "stdio.h"
#include "stdlib.h"
#define __ICONV_SUPPORT__
#ifdef __ICONV_SUPPORT__
#include "lca_conv.h"
#include "lca_conv.c"
#endif
#define MAX_TOKEN       50
#define CLI_INBUF       512
#define MAX_SCREEN_LINE         20

#define CLI_SUPERUSER   "superuser"
#define CLI_HIDDENCMD   "."
#define CLI_TOKEN_NULL_STRING           "\"\""

//#define	DAILY_REBOOT	1

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

#define SKU_US  0x10
#define SKU_AU  0x35
#define SKU_JP  0x40
#define SKU_LA  0x50
#define SKU_BR  0x51
#define SKU_CN  0x70
#define SKU_KR  0x75
#define SKU_G5  0x80

#define PARSE_TOKEN_DELIMITER { "", NULL, NULL, NULL, NULL, 0 }
#define _BONJOUR_
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

char prompt[64]={0};
static CLI *gpCli;
static struct parse_token_s *pCurTokenTbl;
static long lineCounter = 0;
static char txtDisable[] = "disable";
static char txtEnable[]  = "enable";

static const char *pAbleStr[] = {
    "disabled",
    "enabled"
};

static const char *pEthAutoNegoStr[] = {
    "disabled",
    "enabled"
};

static const char *pEthDuplexModeStr[] = {
    "Half Duplex",
    "Full Duplex"
};
static const char *pEthDataRateStr[] = {
    "Auto",
    "1000Mbps",
    "100Mbps",
    "10Mbps"
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

static const char *pEncryStr[] = {
    "TKIP",
    "AES",
    "TKIP or AES"
};
static int   genericCmdHandler(CLI *, char *, struct parse_token_s *);
static int   genericCliCmdSet(CLI *, char *, struct parse_token_s *);

int changeSKUToUS(CLI *pCli, char *p, struct parse_token_s *pTbl);
int changeSKUToAU(CLI *pCli, char *p, struct parse_token_s *pTbl);
int changeSKUToJP(CLI *pCli, char *p, struct parse_token_s *pTbl);
int changeSKUToLA(CLI *pCli, char *p, struct parse_token_s *pTbl);
int changeSKUToBR(CLI *pCli, char *p, struct parse_token_s *pTbl);
int changeSKUToCN(CLI *pCli, char *p, struct parse_token_s *pTbl);
int changeSKUToKR(CLI *pCli, char *p, struct parse_token_s *pTbl);
int changeSKUToG5(CLI *pCli, char *p, struct parse_token_s *pTbl);
int skuCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);

static int   tftpCmdHandler(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   sccli_helpCmdHandler(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   factoryCmdHandler(CLI *, char *, struct parse_token_s *);
static int   genWpsPinCmdHandler(CLI *, char *, struct parse_token_s *);
static int   genWpaPskCmdHandler(CLI *, char *, struct parse_token_s *);
static int   applyCmdHandler(CLI *, char *, struct parse_token_s *);
static int   configCliCmdVap(CLI *, char *, struct parse_token_s *);
static void  sccli_exitCmdHandler(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   dot11nGuardIntervalCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   dot11nGuardIntervalCmdAuto(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   dot11nGuardIntervalCmdShort(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   dot11nGuardIntervalCmdLong(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   dot11nRadioBandCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   dot11nRadioBandCmdStandard(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   dot11nRadioBandCmdAuto(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   dot11nRadioBandCmdWide(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   dot11dModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   dot11dModeCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   dot11dModeCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   dtimModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   dtimModeCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl);
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
//static int   sysCliCmdGet(CLI *, char *, struct parse_token_s *);
//static int   sysCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   fwversionCmdGet(CLI *, char *, struct parse_token_s *);
static int   todCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   uptimeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   countryCodeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   countryCodeCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   dhcpModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   dhcpCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   dhcpCliCmdClient(CLI *, char *, struct parse_token_s *);
static int   dhcpv6ModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   dhcpv6CliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   dhcpv6CliCmdClient(CLI *, char *, struct parse_token_s *);

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

static int   AutoRebootStartTimeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   AutoRebootStartTimeCliCmdSet(CLI *, char *, struct parse_token_s *);

/* MD@CPU_AP add at 20080121 */
static int   opModeCliCmdUc(CLI *, char *, struct parse_token_s *);
static int   opModeCliCmdUr(CLI *, char *, struct parse_token_s *);
static int   opModeCliCmdRogueAp(CLI *, char *, struct parse_token_s *);
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
#ifdef _BONJOUR_
static int   BonjourModeCliCmdGet(CLI *, char *, struct parse_token_s *);//add by carole
static int   BonjourModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   BonjourModeCliCmdEnable(CLI *, char *, struct parse_token_s *);
#endif
static int   WpsModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   WpsModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   WpsModeCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   WpsPinERCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   WpsPinERCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   WpsPinERCliCmdEnable(CLI *, char *, struct parse_token_s *);

static int   WpsPinCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   WpsPinCliCmdSet(CLI *, char *, struct parse_token_s *);

static int   AutoRebootCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   AutoRebootCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   AutoRebootCliCmdEnable(CLI *, char *, struct parse_token_s *);

static int   force100mModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   force100mModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   force100mModeCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   httpRDModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   httpRDModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   httpRDModeCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   httpURLCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   httpURLCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   aclLocalModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   aclLocalModeCliCmdAllow(CLI *, char *, struct parse_token_s *);
static int   aclLocalModeCliCmdPrevent(CLI *, char *, struct parse_token_s *);
static int   magvlanCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   magvlanCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   defaultvlanCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   defaultvlanCliCmdSet(CLI *, char *, struct parse_token_s *);

int outPowerCliCmd5(CLI *pCli, char *p, struct parse_token_s *pTbl);
int outPowerCliCmd6(CLI *pCli, char *p, struct parse_token_s *pTbl);
int outPowerCliCmd8(CLI *pCli, char *p, struct parse_token_s *pTbl);
int outPowerCliCmd10(CLI *pCli, char *p, struct parse_token_s *pTbl);
int outPowerCliCmd13(CLI *pCli, char *p, struct parse_token_s *pTbl);
int outPowerCliCmd16(CLI *pCli, char *p, struct parse_token_s *pTbl);
int outPowerCliCmd20(CLI *pCli, char *p, struct parse_token_s *pTbl);
int outPowerCliCmd25(CLI *pCli, char *p, struct parse_token_s *pTbl);
int outPowerCliCmd32(CLI *pCli, char *p, struct parse_token_s *pTbl);
int outPowerCliCmd40(CLI *pCli, char *p, struct parse_token_s *pTbl);
int outPowerCliCmd50(CLI *pCli, char *p, struct parse_token_s *pTbl);
int outPowerCliCmd63(CLI *pCli, char *p, struct parse_token_s *pTbl);
int outPowerCliCmd79(CLI *pCli, char *p, struct parse_token_s *pTbl);
int outPowerCliCmd100(CLI *pCli, char *p, struct parse_token_s *pTbl);

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
static int   ipv6dns1CliCmdGet(CLI *, char *, struct parse_token_s *);
static int   ipv6dns1CliCmdSet(CLI *, char *, struct parse_token_s *);
static int   ipv6dns2CliCmdGet(CLI *, char *, struct parse_token_s *);
static int   ipv6dns2CliCmdSet(CLI *, char *, struct parse_token_s *);
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
static int   wdsVlanListCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   wdsVlanListCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   vlanPvidCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   vlanPvidCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   wirelessModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   wirelessModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   wirelessModeCliCmd11n(CLI *, char *, struct parse_token_s *);
static int   wirelessModeCliCmd11b(CLI *, char *, struct parse_token_s *);
static int   wirelessModeCliCmd11g(CLI *, char *, struct parse_token_s *);
static int   wirelessModeCliCmd11bg(CLI *, char *, struct parse_token_s *);
static int   wirelessModeCliCmd11bgn(CLI *, char *, struct parse_token_s *);

static int   opModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   opModeCliCmdAp(CLI *, char *, struct parse_token_s *);
//static int   opModeCliCmdPpt(CLI *, char *, struct parse_token_s *);
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
static int   wmmpsCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   outPowerCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   wmmpsCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   wmmpsCliCmdDisable(CLI *, char *, struct parse_token_s *);
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
static int   ssidCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   ssidCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   ssidModeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   ssidModeCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   ssidModeCliCmdEnable(CLI *, char *, struct parse_token_s *);

#if 0
static int stpModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int stpModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int stpModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl);
#endif
static int lltdModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int lltdModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int lltdModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   ethAutoNego(CLI *, char *, struct parse_token_s *);
static int   ethDuplexModeGet(CLI *, char *, struct parse_token_s *);

static int   ethAutoNegoCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   ethAutoNegoCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   ethDuplexModeCliCmdFull(CLI *, char *, struct parse_token_s *);
static int   ethDuplexModeCliCmdHalf(CLI *, char *, struct parse_token_s *);

static int   ethDataRateCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   ethDataRateCliCmdAuto(CLI *, char *, struct parse_token_s *);
static int   ethDataRateCliCmd1000(CLI *, char *, struct parse_token_s *);
static int   ethDataRateCliCmd100(CLI *, char *, struct parse_token_s *);
static int   ethDataRateCliCmd10(CLI *, char *, struct parse_token_s *);
static int   secCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   secCliCmdNone(CLI *, char *, struct parse_token_s *);
static int   secCliCmdWpapsk(CLI *, char *, struct parse_token_s *);
static int   secCliCmdWpa2psk(CLI *, char *, struct parse_token_s *);
static int   secCliCmdWpapskauto(CLI *, char *, struct parse_token_s *);
static int   secCliCmdWpa(CLI *, char *, struct parse_token_s *);
static int   secCliCmdWpa2(CLI *, char *, struct parse_token_s *);
static int   secCliCmdWpaauto(CLI *, char *, struct parse_token_s *);
static int   secCliCmdRadius(CLI *, char *, struct parse_token_s *);
static int   authCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   authCliCmdOpen(CLI *, char *, struct parse_token_s *);
static int   authCliCmdShared(CLI *, char *, struct parse_token_s *);
static int   encryptionCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   encryCliCmdTkip(CLI *, char *, struct parse_token_s *);
static int   encryCliCmdAes(CLI *, char *, struct parse_token_s *);
static int   keyLengthCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   keyLengthCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   defaultKeyCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   defaultKeyCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   keyCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   keyCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   pskCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   pskCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   groupKeyUpdateIntervalCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   groupKeyUpdateIntervalCliCmdSet(CLI *, char *, struct parse_token_s *);

static int   AutoRebootIntervalCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   AutoRebootIntervalCliCmdSet(CLI *, char *, struct parse_token_s *);

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
static int   dot1xKeyLifeCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   dot1xKeyLifeCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   usernameCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   usernameCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   passwordCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   passwordCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   httpCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   httpsCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   menhanceCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   hwversionCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   httpCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   httpCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   httpsCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   httpsCliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   multienhanceDisable(CLI *, char *, struct parse_token_s *);
static int   multienhanceEnable(CLI *, char *, struct parse_token_s *);
#if 0
static int   httpPortCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   httpsPortCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   httpPortCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   httpsPortCliCmdSet(CLI *, char *, struct parse_token_s *);
#endif
static int   timezoneCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   timezoneCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   daylightModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   daylightCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl);
static int   daylightCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl);
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
static int   syslogCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   syslogCliCmdDisable(CLI *, char *, struct parse_token_s *);
static int   syslogCliCmdEnable(CLI *, char *, struct parse_token_s *);
#if 0
static int   syslogCliCmdBroadcast(CLI *, char *, struct parse_token_s *);
static int   syslogCliCmdUnicast(CLI *, char *, struct parse_token_s *);
#endif
static int   syslogSrvCliCmdGet(CLI *, char *, struct parse_token_s *);
static int   syslogSrvCliCmdSet(CLI *, char *, struct parse_token_s *);
static int   roguetypeCmdGet(CLI *, char *, struct parse_token_s *);
static int   roguetype1CliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   roguetype2CliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   roguetype3CliCmdEnable(CLI *, char *, struct parse_token_s *);
static int   roguelegalCmdGet(CLI *, char *, struct parse_token_s *);
static int   roguelegalCmdAdd(CLI *, char *, struct parse_token_s *);
static int   roguelegalCmdDel(CLI *, char *, struct parse_token_s *);


static struct parse_token_s configCmdTbl[] =
{
    {"vap",       "Config Virtual AP X",                        configCliCmdVap,        NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s acllocalmodeCmdTbl[] = {
    {"allow",        "Allow only following MAC addresses to connect to wireless network",    aclLocalModeCliCmdAllow,      NULL,           NULL,           0},
    {"prevent",      "Prevent following MAC addresses from connecting to wireless network",  aclLocalModeCliCmdPrevent,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s aclCmdTbl[] = {
    {txtDisable,        "Disable Access Control",               aclCliCmdDisable,       NULL,           NULL,           0},
    {"local",           "Enable Local Access Control",          aclCliCmdLocal,         NULL,           NULL,           0},
    {"type",            "Set Access Control Local Mode",        genericCmdHandler,      acllocalmodeCmdTbl,           NULL,           0},
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
    {"static",         	"Set DHCPv4 Disable",                     dhcpCliCmdDisable,      NULL,           NULL,           0},
    {"automatic",      	"Set DHCPv4 Client",                      dhcpCliCmdClient,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s dhcpv6CmdTbl[] = {
    {"static",          "Set DHCPv6 Disable",                     dhcpv6CliCmdDisable,      NULL,           NULL,           0},
    {"automatic",       "Set DHCPv6 Client",                      dhcpv6CliCmdClient,       NULL,           NULL,           0},
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
    {"disable",         "Disable Current Radio",                wirelessModeCliCmdDisable,  NULL,       NULL,           0},
    {"11b",             "802.11b only",    						wirelessModeCliCmd11b,    	NULL,       NULL,  			0}, 
    {"11g",           	"802.11g only",            				wirelessModeCliCmd11g,     	NULL,       NULL,  			0},
	{"11n",             "802.11n only",                         wirelessModeCliCmd11n,      NULL,       NULL,  			0},
    {"11bg",            "802.11b and 802.11g",   				wirelessModeCliCmd11bg,		NULL,       NULL,  			0},   
	{"11bgn",           "Mixed 802.11n,802.11b and 802.11g",    wirelessModeCliCmd11bgn,    NULL,       NULL,           0}, 
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s opModeCmdTbl[] = {
    {"ap",              "Operating as Access Point.",                   opModeCliCmdAp,         NULL,           NULL,           0},
#if 0
    {"ptp",             "Operating as Bridge(Point-to-Point)",         	opModeCliCmdPpt,        NULL,           NULL,           0},
#endif
    {"wdsBridge",       "Operating as Wireless WDS Bridge.",           	opModeCliCmdMpt,        NULL,           NULL,           0},
    {"wdsRepeater",     "Operating as Wireless WDS Repeater.",          opModeCliCmdApPtp,      NULL,   NULL,       	0},
    {"apRepeater",      "Operating as Allow wireless signal repeater.", opModeCliCmdApPtmp,     NULL,   NULL,       	0},
    {"uc",            	"Operating as Bridge(Wireless Client).",     	opModeCliCmdUc,       	NULL,           NULL,           0},
    {"ur",           	"Operating as Bridge(Wireless Repeater).",    	opModeCliCmdUr,     	NULL,       NULL,           	0},
    {"rogueAp",       	"Operating as Rogue AP.",      					opModeCliCmdRogueAp,  	NULL,       NULL,           	0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s radiusSrvCliCmdGetTbl[] = {
    {"primary",         "Display primary RADIUS Server IP Address",     radiusSrvCliCmdGetPri,          NULL,           NULL,           0},
    {"backup",          "Display backup RADIUS Server IP Address",      radiusSrvCliCmdGetBck,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s radiusSrvCliCmdSetTbl[] = {
    {"primary",         "Set primary RADIUS Server IP Address",         radiusSrvCliCmdSetPri,          NULL,           NULL,           0},
    {"backup",          "Set backup RADIUS Server IP Address",          radiusSrvCliCmdSetBck,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};


static struct parse_token_s radiusPortCliCmdGetTbl[] = {
    {"primary",         "Display primary RADIUS Server Port Number",    radiusPortCliCmdGetPri,          NULL,           NULL,           0},
    {"backup",          "Display backup RADIUS Server Port Number",     radiusPortCliCmdGetBck,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s radiusPortCliCmdSetTbl[] = {
    {"primary",         "Set primary RADIUS Server Port Number",        radiusPortCliCmdSetPri,          NULL,           NULL,           0},
    {"backup",          "Set backup RADIUS Server Port Number",         radiusPortCliCmdSetBck,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s radiusSecretCliCmdGetTbl[] = {
    {"primary",         "Display primary RADIUS Server Shared Secret",  radiusSecretCliCmdGetPri,          NULL,           NULL,           0},
    {"backup",          "Display backup RADIUS Server Shared Secret",   radiusSecretCliCmdGetBck,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s radiusSecretCliCmdSetTbl[] = {
    {"primary",         "Set primary RADIUS Server Shared Secret",      radiusSecretCliCmdSetPri,          NULL,           NULL,           0},
    {"backup",          "Set backup RADIUS Server Shared Secret",       radiusSecretCliCmdSetBck,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s remotePtmpMacListCmdTbl[] = {
    {"1",         "Remote Mac Address 1",                     remotePtmpMacListCliCmdSet,          NULL,           NULL,           0},
    {"2",         "Remote Mac Address 2",                     remotePtmpMacListCliCmdSet,          NULL,           NULL,           0},
    {"3",         "Remote Mac Address 3",                     remotePtmpMacListCliCmdSet,          NULL,           NULL,           0},
    {"4",         "Remote Mac Address 4",                     remotePtmpMacListCliCmdSet,          NULL,           NULL,           0},
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

static struct parse_token_s  dot11nRadioBandCmdTbl[] = {
    {"standard",    "Set Channel Bandwidth to Standard-20MHz Channel",         dot11nRadioBandCmdStandard,     NULL,           NULL,           0},
    {"auto",        "Set Channel Bandwidth to Auto-20/40MHz Channel",          dot11nRadioBandCmdAuto,         NULL,           NULL,           0},
    {"wide",        "Set Channel Bandwidth to Wide-40MHz Channel",             dot11nRadioBandCmdWide,         NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};   

static struct parse_token_s  skuCmdTbl[] = {
    {"SKU_US",    "Change SKU to US.",         changeSKUToUS,     	  NULL,           NULL,           0},
    {"SKU_AU",    "Change SKU to AU.",         changeSKUToAU,         NULL,           NULL,           0},
    {"SKU_JP",    "Change SKU to JP.",         changeSKUToJP,         NULL,           NULL,           0},
    {"SKU_LA",    "Change SKU to AU.",         changeSKUToAU,         NULL,           NULL,           0},
    {"SKU_BR",    "Change SKU to BR.",         changeSKUToBR,         NULL,           NULL,           0},
    {"SKU_CN",    "Change SKU to CN.",         changeSKUToCN,         NULL,           NULL,           0},
    {"SKU_KR",    "Change SKU to KR.",         changeSKUToKR,         NULL,           NULL,           0},
    {"SKU_G5",    "Change SKU to G5.",         changeSKUToG5,         NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};  
#define BOOT_MTD_DEV            "/dev/mtdblock0"
#define BOOT_SIZE               0x00040000

#ifndef DOMAIN_OFFSET       
#define DOMAIN_OFFSET       (BOOT_SIZE - 0x80)
#endif

int ModifySKUInFlash(unsigned char domain)
{
    int fd, ret=0;

    if((fd=open(BOOT_MTD_DEV, O_WRONLY)) < 0)
		return -1;
    if((ret=lseek(fd, DOMAIN_OFFSET, SEEK_SET)) != DOMAIN_OFFSET)
        return -1;			
	write(fd, &domain, 1);
	close(fd);
	return 0;
}

int GetSKUFromFlash(unsigned char *domain)
{
    int fd, ret=0;

    if((fd=open(BOOT_MTD_DEV, O_RDONLY)) < 0)
		return -1;
    if((ret=lseek(fd, DOMAIN_OFFSET, SEEK_SET)) != DOMAIN_OFFSET){
    	close(fd);
        return -1;			
	}       
    if(read(fd, domain, 1)!=1){
    	close(fd);
        return -1;
	}        
	close(fd);
	return 0;
}

int
changeSKUToUS(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	int ret=0;
	
	ret=ModifySKUInFlash(SKU_US);
	if(ret){
		printf("Fail change SKU to %s.\n", "SKU_US");
	}
	else{
    	printf("SKU is changed to %s.\n", "SKU_US");
    	printf("Please reboot the device to take effect.\n");
    }	
    return CLI_PARSE_OK;
} 

int
changeSKUToAU(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	int ret=0;
	
	ret=ModifySKUInFlash(SKU_AU);
	if(ret){
		printf("Fail change SKU to %s.\n", "SKU_AU");
	}
	else{
    	printf("SKU is changed to %s.\n", "SKU_AU");
    	printf("Please reboot the device to take effect.\n");
    }	
    return CLI_PARSE_OK;
} 

int
changeSKUToJP(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	int ret=0;
	
	ret=ModifySKUInFlash(SKU_JP);
	if(ret){
		printf("Fail change SKU to %s.\n", "SKU_JP");
	}
	else{
    	printf("SKU is changed to %s.\n", "SKU_JP");
    	printf("Please reboot the device to take effect.\n");
    }		
    return CLI_PARSE_OK;
} 

int
changeSKUToLA(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	int ret=0;
	
	ret=ModifySKUInFlash(SKU_LA);
	if(ret){
		printf("Fail change SKU to %s.\n", "SKU_LA");
	}
	else{
    	printf("SKU is changed to %s.\n", "SKU_LA");
    	printf("Please reboot the device to take effect.\n");
    }		
    return CLI_PARSE_OK;
} 

int
changeSKUToBR(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	int ret=0;
	
	ret=ModifySKUInFlash(SKU_BR);
	if(ret){
		printf("Fail change SKU to %s.\n", "SKU_BR");
	}
	else{
    	printf("SKU is changed to %s.\n", "SKU_BR");
    	printf("Please reboot the device to take effect.\n");
    }
    return CLI_PARSE_OK;
} 

int
changeSKUToCN(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	int ret=0;
	
	ret=ModifySKUInFlash(SKU_CN);
	if(ret){
		printf("Fail change SKU to %s.\n", "SKU_CN");
	}
	else{
    	printf("SKU is changed to %s.\n", "SKU_CN");
    	printf("Please reboot the device to take effect.\n");
    }	

    return CLI_PARSE_OK;
} 

int
changeSKUToKR(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	int ret=0;
	
	ret=ModifySKUInFlash(SKU_KR);
	if(ret){
		printf("Fail change SKU to %s.\n", "SKU_KR");
	}
	else{
    	printf("SKU is changed to %s.\n", "SKU_KR");
    	printf("Please reboot the device to take effect.\n");
    }	

    return CLI_PARSE_OK;
} 

int
changeSKUToG5(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	int ret=0;
	
	ret=ModifySKUInFlash(SKU_G5);
	if(ret){
		printf("Fail change SKU to %s.\n", "SKU_G5");
	}
	else{
    	printf("SKU is changed to %s.\n", "SKU_G5");
    	printf("Please reboot the device to take effect.\n");
    }		
    return CLI_PARSE_OK;
} 

int
skuCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	unsigned char domain=SKU_US;
	
	GetSKUFromFlash(&domain);	
	switch(domain){
		case SKU_US:
    		printf("Current SKU: %s\n", "US");
    		break;
		case SKU_KR:
    		printf("Current SKU: %s\n", "KR");
    		break; 
		case SKU_CN:
    		printf("Current SKU: %s\n", "CN");
    		break;     	
		case SKU_BR:
    		printf("Current SKU: %s\n", "BR");
    		break;       		
		case SKU_LA:
    		printf("Current SKU: %s\n", "LA");
    		break;         		
		case SKU_JP:
    		printf("Current SKU: %s\n", "JP");
    		break;      		
		case SKU_AU:
    		printf("Current SKU: %s\n", "AU");
    		break;    
		case SKU_G5:
    		printf("Current SKU: %s\n", "G5");
    		break;      		  
    	default:
    		break;	
    }	   	
    return CLI_PARSE_OK;
}


static struct parse_token_s  dot11dModeCmdTbl[] = {
    {txtDisable,        "Disable 802.11d Mode",                 dot11dModeCmdDisable,   NULL,           NULL,           0},
    {txtEnable,         "Enable 802.11d Mode",                  dot11dModeCmdEnable,    NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s wmeCmdTbl[] = {
    {txtDisable,         "Disable WMM",                          wmeCliCmdDisable,       NULL,           NULL,           0},
    {txtEnable,          "Enable WMM",                           wmeCliCmdEnable,        NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s wmmpsCmdTbl[] = {
    {txtDisable,         "Disable WMM PowerSaving",                          wmmpsCliCmdDisable,       NULL,           NULL,           0},
    {txtEnable,          "Enable WMM PowerSaving",                           wmmpsCliCmdEnable,        NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s outPowerCmdTbl[] = {
    {"100%",		"100% of Maximum Out Power",                          	outPowerCliCmd100,       NULL,           NULL,           0},
    {"79%",         "79% of Maximum Out Power",                          	outPowerCliCmd79,       NULL,           NULL,           0},
    {"63%",         "63% of Maximum Out Power",                          	outPowerCliCmd63,       NULL,           NULL,           0},
    {"50%",         "50% of Maximum Out Power",                          	outPowerCliCmd50,       NULL,           NULL,           0},
    {"40%",         "40% of Maximum Out Power",                          	outPowerCliCmd40,       NULL,           NULL,           0},
    {"32%",         "32% of Maximum Out Power",                          	outPowerCliCmd32,       NULL,           NULL,           0},
    {"25%",         "25% of Maximum Out Power",                          	outPowerCliCmd25,       NULL,           NULL,           0},
    {"20%",         "20% of Maximum Out Power",                          	outPowerCliCmd20,       NULL,           NULL,           0},
    {"16%",         "16% of Maximum Out Power",                          	outPowerCliCmd16,       NULL,           NULL,           0},
    {"13%",         "13% of Maximum Out Power",                          	outPowerCliCmd13,       NULL,           NULL,           0},
    {"10%",         "10% of Maximum Out Power",                          	outPowerCliCmd10,       NULL,           NULL,           0},
    {"8%",          "8% of Maximum Out Power",                          	outPowerCliCmd8,       NULL,           NULL,           0},
    {"6%",         	"6% of Maximum Out Power",                          	outPowerCliCmd6,       NULL,           NULL,           0},
    {"5%",         	"5% of Maximum Out Power",                          	outPowerCliCmd5,       NULL,           NULL,           0},
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

#if 0
static struct parse_token_s stpModeCmdTbl[] = {
    {txtDisable,        "Disable STP",                          stpModeCliCmdDisable,  NULL,           NULL,           0},
    {txtEnable,         "Enable STP",                           stpModeCliCmdEnable,   NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};
#endif

static struct parse_token_s lltdModeCmdTbl[] = {
    {txtDisable,        "Disable LLTD",                         lltdModeCliCmdDisable,  NULL,           NULL,           0},
    {txtEnable,         "Enable LLTD",                          lltdModeCliCmdEnable,   NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s authCmdTbl[] = {
    {"open",            "Select Open-System Authentication Type",authCliCmdOpen,        NULL,           NULL,           0},
    {"shared",          "Select Shared-Key Authentication Type",authCliCmdShared,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};
static struct parse_token_s encryCmdTbl[] = {
    {"TKIP",            "Select TKIP Encryption Algorithm Type", encryCliCmdTkip,        NULL,           NULL,           0},
    {"AES",             "Select AES Encryption Algorithm Type", encryCliCmdAes,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};
static struct parse_token_s secCmdTbl[] = {
    {"wep",             "WEP Mode(Open/Share)",                 genericCmdHandler,      authCmdTbl,     NULL,           0},
    {"wpapsk",          "WPA-Personal Mode",                    secCliCmdWpapsk,        NULL,           NULL,           0},
    {"wpa2psk",         "WPA2-Personal Mode",                   secCliCmdWpa2psk,       NULL,           NULL,           0},
    {"wpa2pskmixed",    "WPA/WPA2-Personal Mixed Mode",       	secCliCmdWpapskauto,    NULL,           NULL,           0},
    {"wparadius",       "WPA-Enterprise Mode",                 	secCliCmdWpa,           NULL,           NULL,           0},
    {"wpa2radius",      "WPA2-Enterprise Mode",                	secCliCmdWpa2,          NULL,           NULL,           0},
    {"wpa2radiusmixed", "WPA/WPA2-Enterprise Mixed Mode",    	secCliCmdWpaauto,       NULL,           NULL,           0},
    {"radius",          "RADIUS Mode",                          secCliCmdRadius,    	NULL,	 		NULL,           0},
	{"disable",         "None Security Mode",                   secCliCmdNone,          NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s ethAutoNegoCmdTbl[] = {
    {"1",    "Auto Negotiation Enable",  ethAutoNegoCliCmdEnable,  NULL,   NULL,   0},
    {"0",    "Auto Negotiation Disable",     ethAutoNegoCliCmdDisable,  NULL,   NULL,   0},
    PARSE_TOKEN_DELIMITER
};
static struct parse_token_s ethDuplexModeCmdTbl[] = {
    {"1",    "Full Duplex Mode",  ethDuplexModeCliCmdFull,  NULL,   NULL,   0},
    {"0",    "Half Duplex Mode",     ethDuplexModeCliCmdHalf,  NULL,   NULL,   0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s ethDataRateCmdTbl[] = {
    {"auto",    "Auto detect",  ethDataRateCliCmdAuto,  NULL,   NULL,   0},
    {"1000",    "1000Mbps",     ethDataRateCliCmd1000,  NULL,   NULL,   0},
    {"100",     "100Mbps",      ethDataRateCliCmd100,   NULL,   NULL,   0},
    {"10",      "10Mbps",       ethDataRateCliCmd10,    NULL,   NULL,   0},
    PARSE_TOKEN_DELIMITER
};

                
static struct parse_token_s countryCodeCmdTbl[] = {
    {"UnitedStates",    "Country Code: 840",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Australia",       "Country Code: 36",                 countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Brazil",          "Country Code: 76",                 countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"China",           "Country Code: 156",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Asia",            "Country Code: 1702",               countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Denmark",         "Country Code: 208",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Europe",          "Country Code: 1276",               countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Finland",         "Country Code: 246",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"France",          "Country Code: 250",                countryCodeCliCmdSet,      NULL,           NULL,           0},    
    {"Germany",         "Country Code: 276",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"ireland",         "Country Code: 372",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Italy",           "Country Code: 380",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Japan",           "Country Code: 392",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Netherlands",     "Country Code: 528",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"NewZealand",      "Country Code: 554",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Norway",          "Country Code: 578",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Spain",           "Country Code: 724",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Sweden",          "Country Code: 752",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Switzerland",     "Country Code: 756",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"UnitedKingdom",   "Country Code: 826",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"Mexico",          "Country Code: 484",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"PuertoRico",      "Country Code: 630",                countryCodeCliCmdSet,      NULL,           NULL,           0},
    {"SouthAmerica",    "Country Code: 1076",               countryCodeCliCmdSet,      NULL,           NULL,           0},
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

static struct parse_token_s multienhanceCmdTbl[] = {
    {txtDisable,        "Disable Multicast Packets Enhancement",                        multienhanceDisable,     NULL,           NULL,           0},
    {txtEnable,         "Enable Multicast Packets Enhancement",                         multienhanceEnable,      NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};


/* MD@CPU_AP add at 20080121 */
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
#ifdef _BONJOUR_
//add by carole
static struct parse_token_s	bonjourmodeCmdTbl[]	= {
    {txtDisable,        "Disable Bonjour",                 BonjourModeCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Enable Bonjour",                  BonjourModeCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};
//end add
#endif
static struct parse_token_s	wpsmodeCmdTbl[]	= {
    {txtDisable,        "Disable WPS",                 WpsModeCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Enable WPS",                  WpsModeCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s	wpsPinERCmdTbl[]	= {
    {txtDisable,        "Disable WPS PIN External Registrar",                 WpsPinERCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Enable WPS PIN External Registrar",                  WpsPinERCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s	autoRebootCmdTbl[]	= {
    {txtDisable,        "Disable Daily Reboot",                 AutoRebootCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Enable Daily Reboot",                  AutoRebootCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s	force100mmodeCmdTbl[]	= {
    {txtDisable,        "Disable Force LAN Port Speed to 100M", force100mModeCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Enable Force LAN Port Speed to 100M",  force100mModeCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};
static struct parse_token_s httpredirectmodeCmdTbl[] = {
    {txtDisable,        "Disable HTTP Redirct",                 httpRDModeCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Enable HTTP Redirct",                  httpRDModeCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

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
    {"clock",   "Set Current System Time. Foramt : xx:xx:xx(hour:minute:second)",	tocCliCmdSet,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s timezoneCmdTbl[] = {
    {"001-12:00",         "(GMT-12:00) International Date Line West",           timezoneCliCmdSet,     NULL,           NULL,           0},
    {"002-11:00",         "(GMT-11:00) Midway Island, Samoa",     				timezoneCliCmdSet,     NULL,           NULL,           0},
    {"003-10:00",         "(GMT-10:00) Hawaii",                   				timezoneCliCmdSet,     NULL,           NULL,           0},
    {"004-09:00",         "(GMT-09:00) Alaska",                   				timezoneCliCmdSet,     NULL,           NULL,           0},
    {"005-08:00",  	      "(GMT-08:00) Pacific Time (US & Canada); Tijuana",  	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"006-07:00",         "(GMT-07:00) Arizona",                  				timezoneCliCmdSet,     NULL,           NULL,           0},
    {"007-07:00",         "(GMT-07:00) Chihuahua, La Paz, Mazatlan",  			timezoneCliCmdSet,     NULL,           NULL,           0},
    {"008-07:00",         "(GMT-06:00) Mountain Time (US & Canada)",        	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"009-06:00",         "(GMT-06:00) Central America",   						timezoneCliCmdSet,     NULL,           NULL,           0},
    {"010-06:00",         "(GMT-06:00) Central Time (US & Canada)",   			timezoneCliCmdSet,     NULL,           NULL,           0},
    {"011-06:00",         "(GMT-06:00) Guadalajara, Mexico City, Monterrey",   	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"012-06:00",         "(GMT-06:00) Saskatchewan",       					timezoneCliCmdSet,     NULL,           NULL,           0},
    {"013-05:00",         "(GMT-05:00) Bogota, Lima, Quito",   					timezoneCliCmdSet,     NULL,           NULL,           0},
    {"014-05:00",         "(GMT-05:00) Eastern Time (US & Canada)",             timezoneCliCmdSet,     NULL,           NULL,           0},
    {"015-05:00",         "(GMT-05:00) Indiana (East)",   						timezoneCliCmdSet,     NULL,           NULL,           0},
    {"016-04:00",         "(GMT-04:00) Atlantic Time (Canada)",             	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"017-04:00",         "(GMT-04:00) Caracas, La Paz",                   		timezoneCliCmdSet,     NULL,           NULL,           0},
    {"018-04:00",         "(GMT-04:00) Santiago",       						timezoneCliCmdSet,     NULL,           NULL,           0},
    {"019-03:00",         "(GMT-03:00) Newfoundland",                         	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"020-03:00",         "(GMT-03:00) Brasilia",                 	 			timezoneCliCmdSet,     NULL,           NULL,           0},
    {"021-03:00",         "(GMT-03:00) Buenos Aires, Georgetown",   			timezoneCliCmdSet,     NULL,           NULL,           0},
    {"022-03:00",         "(GMT-03:00) Greenland",             					timezoneCliCmdSet,     NULL,           NULL,           0},
    {"023-02:00",         "(GMT-02:00) Mid-Atlantic",     						timezoneCliCmdSet,     NULL,           NULL,           0},
    {"024-01:00",         "(GMT-01:00) Azores",     							timezoneCliCmdSet,     NULL,           NULL,           0},
    {"025-01:00",         "(GMT-01:00) Cape Verde Is.",                  		timezoneCliCmdSet,     NULL,           NULL,           0},
    {"026+00:00",         "(GMT) Casablanca, Monrovia",         				timezoneCliCmdSet,     NULL,           NULL,           0},
    {"027+00:00",         "(GMT) Greenwich Mean Time : Dublin, Edinburgh, Lisbon, London",   	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"028+01:00",         "(GMT+01:00) Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna",      	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"029+01:00",         "(GMT+01:00) Belgrade, Bratislava, Budapest, Ljubljana, Prague",  	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"030+01:00",         "(GMT+01:00) Brussels, Copenhagen, Madrid, Paris",	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"031+01:00",         "(GMT+01:00) Sarajevo, Skopje, Warsaw, Zagreb",     	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"032+01:00",         "(GMT+01:00) West Central Africa",             		timezoneCliCmdSet,     NULL,           NULL,           0},
    {"033+02:00",         "(GMT+02:00) Athens, Beirut, Istanbul, Minsk",   		timezoneCliCmdSet,     NULL,           NULL,           0},
    {"034+02:00",         "(GMT+02:00) Bucharest",          					timezoneCliCmdSet,     NULL,           NULL,           0},
    {"035+02:00",         "(GMT+02:00) Cairo",                     				timezoneCliCmdSet,     NULL,           NULL,           0},
    {"036+02:00",         "(GMT+02:00) Harare, Pretoria",              			timezoneCliCmdSet,     NULL,           NULL,           0},
    {"037+02:00",         "(GMT+02:00) Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius",       	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"038+02:00",         "(GMT+02:00) Jerusalem",              				timezoneCliCmdSet,     NULL,           NULL,           0},
    {"039+03:00", 		 "(GMT+03:00) Baghdad",              					timezoneCliCmdSet,     NULL,           NULL,           0},
    {"040+03:00",		 "(GMT+03:00) Kuwait, Riyadh",              			timezoneCliCmdSet,     NULL,           NULL,           0},
    {"041+03:00",  		 "(GMT+03:00) Moscow, St. Petersburg, Volgograd",    	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"042+03:00",        "(GMT+03:00) Nairobi",              					timezoneCliCmdSet,     NULL,           NULL,           0},
    {"043+03:30",        "(GMT+03:30) Tehran",              					timezoneCliCmdSet,     NULL,           NULL,           0},
    {"044+04:00",        "(GMT+03:30) Tehran",              					timezoneCliCmdSet,     NULL,           NULL,           0},
    {"045+04:00",        "(GMT+04:00) Baku, Tbilisi, Yerevan",              	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"046+04:30",        "(GMT+04:30) Kabul",              						timezoneCliCmdSet,     NULL,           NULL,           0},
    {"047+05:00",        "(GMT+05:00) Ekaterinburg",              				timezoneCliCmdSet,     NULL,           NULL,           0},
    {"048+05:00",        "(GMT+05:00) Islamabad, Karachi, Tashkent",          	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"049+05:30",        "(GMT+05:30) Chennai, Kolkata, Mumbai, New Delhi", 	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"050+05:45",        "(GMT+05:45) Kathmandu",              					timezoneCliCmdSet,     NULL,           NULL,           0},
    {"051+06:00",        "(GMT+06:00) Almaty, Novosibirsk",              		timezoneCliCmdSet,     NULL,           NULL,           0},
    {"052+06:00",        "(GMT+06:00) Astana, Dhaka",              				timezoneCliCmdSet,     NULL,           NULL,           0},
    {"053+06:00",        "(GMT+06:00) Sri Jayawardenepura",              		timezoneCliCmdSet,     NULL,           NULL,           0},
    {"054+06:00",        "(GMT+06:00) Rangoon",              					timezoneCliCmdSet,     NULL,           NULL,           0},
    {"055+07:00",        "(GMT+07:00) Bangkok, Hanoi, Jakarta",              	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"056+07:00",        "(GMT+07:00) Krasnoyarsk",              				timezoneCliCmdSet,     NULL,           NULL,           0},
    {"057+08:00",        "(GMT+08:00) China, Hong Kong, Australia Western",   	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"058+08:00",        "(GMT+08:00) Irkutsk, Ulaan Bataar",              		timezoneCliCmdSet,     NULL,           NULL,           0},
    {"059+08:00",        "(GMT+08:00) Kuala Lumpur, Singapore",              	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"060+08:00",        "(GMT+08:00) Perth",              						timezoneCliCmdSet,     NULL,           NULL,           0},
    {"061+08:00",        "(GMT+08:00) Taipei",              					timezoneCliCmdSet,     NULL,           NULL,           0},
    {"062+09:00",        "(GMT+09:00) Osaka, Sapporo, Tokyo",              		timezoneCliCmdSet,     NULL,           NULL,           0},
    {"063+09:00",        "(GMT+09:00) Seoul",              						timezoneCliCmdSet,     NULL,           NULL,           0},
    {"064+09:00",        "(GMT+09:00) Yakutsk",              					timezoneCliCmdSet,     NULL,           NULL,           0},
    {"065+09:30",        "(GMT+09:30) Adelaide",              					timezoneCliCmdSet,     NULL,           NULL,           0},
    {"066+09:30",        "(GMT+09:30) Darwin",              					timezoneCliCmdSet,     NULL,           NULL,           0},
    {"067+10:00",        "(GMT+10:00) Brisbane",              					timezoneCliCmdSet,     NULL,           NULL,           0},
    {"068+10:00",        "(GMT+10:00) Canberra, Melbourne, Sydney",           	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"069+10:00",        "(GMT+10:00) Guam, Port Moresby",              		timezoneCliCmdSet,     NULL,           NULL,           0},
    {"070+10:00",        "(GMT+10:00) Hobart",              					timezoneCliCmdSet,     NULL,           NULL,           0},
    {"071+10:00",        "(GMT+10:00) Vladivostok",              				timezoneCliCmdSet,     NULL,           NULL,           0},
    {"072+11:00",        "(GMT+11:00) Magadan, Solomon Ls., New Caledonia",   	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"073+12:00",        "(GMT+12:00) Auckland, Wellington",              		timezoneCliCmdSet,     NULL,           NULL,           0},
    {"074+12:00",        "(GMT+12:00) Fiji, Kamchatka, Marshall ls.",          	timezoneCliCmdSet,     NULL,           NULL,           0},
    {"075+13:00",        "(GMT+13:00) Nuku alofa",              				timezoneCliCmdSet,     NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER,
};
static struct parse_token_s daylightModeCmdTbl[] = {
    {txtDisable,        "Disable daylight",                         daylightCliCmdDisable,      NULL,           NULL,           0},
    {txtEnable,         "Enable daylight",                          daylightCliCmdEnable,       NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
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

static struct parse_token_s syslogCmdTbl[] = {
	{txtDisable,         "Disable Syslog",                       syslogCliCmdDisable,    NULL,           NULL,           0},
	{txtEnable,          "Enable Syslog",                        syslogCliCmdEnable,     NULL,           NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s roguetypeCmdTbl[] = {
    {"inseurity",       "Set Rogue AP Type as No Security",         roguetype1CliCmdEnable, NULL,       NULL,           0},
    {"illegal",         "Set Rogue AP Type as Not in Legal AP List",roguetype2CliCmdEnable, NULL,       NULL,           0},
    {"both",            "Set Rogue AP Type as Both No Security and Not in Legal AP List",roguetype3CliCmdEnable,NULL,NULL,0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s roguelegalCmdTbl[] = {
    {"add",             "Add a AP MAC/OUI into Legal AP List",      roguelegalCmdAdd,       NULL,       NULL,           0},
    {"delete",          "Delete a AP MAC/OUI from Legal AP List",   roguelegalCmdDel,       NULL,       NULL,           0},
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s getCmdTbl[] = {
    {"11nguardinterval","Set 11n Guard Interval Mode",          dot11nGuardIntervalCliCmdGet,   NULL,   NULL,           0},
    {"11nchanbandwidth","Set 11n Channel Bandwidth",            dot11nRadioBandCliCmdGet,       NULL,   NULL,           0},
    {"802_11d",         "Display 802.11d Mode",                 dot11dModeCliCmdGet,    NULL,           NULL,           0},
    {"acl",             "Display Access Control Status",        aclCliCmdGet,           NULL,           NULL,           0},
	{"acltype",    	    "Display Access Control Local Mode",    aclLocalModeCliCmdGet,           NULL,           NULL,           0},
    {"active",          "Display VAP Active (up) Mode",         activeModeCliCmdGet,    NULL,           NULL,           0},
/*    {"aging",           "Display Idle Timeout Interval",        agingCliCmdGet,         NULL,           NULL,           0},*/
    {"authentication",  "Display Authentication Type of WEP",   authCliCmdGet,          NULL,           NULL,           0},
    {"beaconinterval",  "Display Beacon Interval,(Range:20 ~ 1000)",              beaconCliCmdGet,        NULL,           NULL,           0},
    {"channel",         "Display Radio Channel",                chanCliCmdGet,          NULL,           NULL,           0},
    {"country",         "Display Country/Domain",               countryCodeCliCmdGet,   NULL,           NULL,           0},
    {"ethdatarate",      "Display Ethernet Data Rate",          ethDataRateCliCmdGet,   NULL,           NULL,           0},
    {"ethAutoNegotiation",      "Display Ethernet Negotiation Mode",          ethAutoNego,   NULL,           NULL,           0},
    {"ethDuplexMode",      "Display Ethernet Duplex Mode",          ethDuplexModeGet,   NULL,           NULL,           0},
    {"ctsprotect",    	"Display CTS Protect Mode",     			ctsProtectModeCliCmdGet,           NULL,           NULL,           0},
    {"defaultkey"  ,    "Display Default Key Index",            defaultKeyCliCmdGet,    NULL,           NULL,           0},    
    {"devicename",      "Display Access Point Dvice Name",     		devicenameCliCmdGet,         NULL,           NULL,           0},
    {"dhcp"  ,          "Display DHCPv4 Mode",                    dhcpModeCliCmdGet,      NULL,           NULL,           0},
    {"dhcpv6"  ,          "Display DHCPv6 Mode",                    dhcpv6ModeCliCmdGet,      NULL,           NULL,           0},
    {"dns2server",      "Display IP Address of Secondary DNS Server", nameSrv2CliCmdGet,         NULL,           NULL,           0},
    {"dnsserver",       "Display IP Address of DNS Server",     nameSrvCliCmdGet,       NULL,           NULL,           0},
    {"dot1xdynkeylife", "Display 802.1x Dynamic Key Life Time (in Seconds)",dot1xKeyLifeCliCmdGet,NULL, NULL,           0},
	{"dtiminterval"  ,  "Display DTIM interval,(Range:1 ~ 255)",    dtimModeCliCmdGet,      NULL,           NULL,           0},
    {"emailaddress",   	"Display Email Address for Log",       		emailAddressCliCmdGet,		NULL,           NULL,           0},
	{"emailalert",     	"Display Email Alert Mode",       			emailAlertCliCmdGet,		NULL,           NULL,           0},
	{"emailinterval", 	"Display Email Time Threshold",       		emailSendPeriodCliCmdGet,	NULL,           NULL,           0},
	{"emailqueue",     	"Display Email Queue Length",       		emailQueueLengthCliCmdGet,	NULL,           NULL,           0},
	{"emailserver",     "Display Email Alert Server",       		emailServerCliCmdGet,		NULL,           NULL,           0},
    {"encryption",      "WPA encryption Algorithm(TKIP/AES)",       encryptionCliCmdGet,		NULL,           NULL,           0},
    {"fragthreshold",   "Display Fragment Threshold,(Default: 2346, Range: 256 ~ 2346)",           fragCliCmdGet,          NULL,           NULL,           0},
    {"fwversion",       "Display Firmware Version",             fwversionCmdGet,          NULL,           NULL,           0},
    {"gateway",         "Display Gateway IP Address",           gatewayCliCmdGet,       NULL,           NULL,           0},
    {"hostname",      	"Display Access Point Host Name",     		hostnameCliCmdGet,           NULL,           NULL,           0},
/*    {"http",            "Display HTTP Mode",                    httpCliCmdGet,          NULL,           NULL,           0},*/
#ifdef _BONJOUR_
	{"bonjour",			"Display Bonjour Mode",       			   BonjourModeCliCmdGet,		NULL,           NULL,           0},//add  by carole
#endif
	{"wps",			    "Display WPS Status",			                WpsModeCliCmdGet,		NULL,           NULL,           0},
    {"wps_pin",      	"Display WPS PIN",     		WpsPinCliCmdGet,           NULL,           NULL,           0},
	{"wps_pin_er",			    "Display WPS PIN External Registrar",			                WpsPinERCliCmdGet,		NULL,           NULL,           0},

	{"forcelan100m",    "Display Force LAN Port Speed to 100M",     force100mModeCliCmdGet,		NULL,           NULL,           0},
	{"httpredirect",    "Display HTTP Redirect Mode",       		httpRDModeCliCmdGet,		NULL,           NULL,           0},
	{"httpredirectURL", "Display HTTP Redirect URL",       			httpURLCliCmdGet,		NULL,           NULL,           0},
    {"https",           "Display HTTPS Mode",                   httpsCliCmdGet,         NULL,           NULL,           0},
    {"pidvid",          "Display PID VID",                      hwversionCliCmdGet,     NULL,           NULL,           0},
    {"ipaddr",          "Display IP Address",                   ipAddrCliCmdGet,        NULL,           NULL,           0},
    {"ipmask",          "Display IP Subnet Mask",               ipMaskCliCmdGet,        NULL,           NULL,           0},
	{"ipv6",     		"Display IPv6 Mode",       					ipv6ModeCliCmdGet,			NULL,           NULL,           0},
	{"ipv6addr",     	"Display IPv6 Local Address",       		ipv6AddrCliCmdGet,			NULL,           NULL,           0},
	
	{"ipv6dns1",        "Display IPv6 Primary DNS Server",       		ipv6dns1CliCmdGet,		NULL,           NULL,           0},
    {"ipv6dns2",        "Display IPv6 Secondary DNS Server",       		ipv6dns2CliCmdGet,		NULL,           NULL,           0},
    
    {"ipv6gateway",     "Display IPv6 Default Gateway",       		ipv6gatewayCliCmdGet,		NULL,           NULL,           0},
    {"isolationbetweenssid",       "Display Isolate All Virtual APs State",interVapForwardingCliCmdGet,NULL,       NULL,           0},
    {"isolationwithinssid","Display Isolation within VAP",        sepCliCmdGet,           NULL,           NULL,           0},
    {"key",             "Display WEP Key Value",                keyCliCmdGet,           NULL,           NULL,           0},
    {"keylength",       "Display WEP Key Length",               keyLengthCliCmdGet,     NULL,           NULL,           0},
    {"keyrenew",        "Display Group Key Update Interval (in Seconds)", groupKeyUpdateIntervalCliCmdGet,NULL,NULL,0},
    {"lltd",            "Display LLTD Mode",                    lltdModeCliCmdGet,      NULL,           NULL,           0}, 
    {"loadbalance",    	"Display Load Balance Mode",     			loadBalanceModeCliCmdGet,           NULL,           NULL,           0},
	{"loadbalancessid", "Display Load Balance SSID",     			loadBalanceSSIDCliCmdGet,           NULL,           NULL,           0},
	{"logconfchange",	"Display Log Configuration Changes Mode",   confChangeCliCmdGet,		NULL,           NULL,           0},
	{"logfail",     	"Display Log Authorized Login Mode",       	loginFailCliCmdGet,			NULL,           NULL,           0},
	{"logsuccess",     	"Display Log Unauthorized Login Attempt Mode",loginSuccessCliCmdGet,	NULL,           NULL,           0},
	{"logsyserror",     "Display Log System Error Messages Mode",   sysErrorCliCmdGet,			NULL,           NULL,           0},
    {"mcastenhance",           "Display Multicast Packets Enhancement Mode",                   menhanceCliCmdGet,         NULL,           NULL,           0},
	{"md5supplicant",   "Display 802.1x MD5 Supplicant Mode",   dot1xSuppCliCmdGet,     NULL,           NULL,           0},
	{"md5suppname",     "Display 802.1x Supplicant MD5 Name",   dot1xSuppUserCliCmdGet, NULL,           NULL,           0},
	{"md5supppassword", "Display 802.1x Supplicant MD5 Password",dot1xSuppPassCliCmdGet,NULL,           NULL,           0},
	{"md5supptype",     "Display 802.1x MD5 Supplicant Type",   dot1xSuppTypeCliCmdGet, NULL,           NULL,           0},
    {"nativevlanid",    "Display Native VLAN ID",               nativeVlanIdCliCmdGet,  NULL,           NULL,           0},
	{"ntp",     		"Display NTP Mode",       					ntpModeCliCmdGet,		NULL,           NULL,           0},
    {"ntpserver",       "Display NTP Server IP Address",        ntpServerCliCmdGet,     NULL,           NULL,           0},
    {"operationmode",   "Display Operation Mode",               opModeCliCmdGet,        NULL,           NULL,           0},
    {"password",        "Display Login Password",               passwordCliCmdGet,      NULL,           NULL,           0},
	{"priority",		"Display Priority",               			priorityCliCmdGet,      NULL,           NULL,           0},
    {"psk",             "Display Pre-shared Key",               pskCliCmdGet,           NULL,           NULL,           0},
    {"radiusserver",    "Display RADIUS Server IP Address",     genericCmdHandler,      radiusSrvCliCmdGetTbl,          NULL,           0},
    {"radiusport",      "Display RADIUS Port Number",           genericCmdHandler,      radiusPortCliCmdGetTbl,         NULL,           0},
    {"radiussecret",    "Display RADIUS Shared Secret",         genericCmdHandler,      radiusSecretCliCmdGetTbl,       NULL,           0},
    {"bridgemac",       "Display WDS Bridge's Remote MAC Address List",remotePtmpMacListCliCmdGet,NULL,       NULL,           0},
    {"repeatermac",     "Display WDS Repeater's Remote MAC Address",     remotePtpMacAddrCliCmdGet,NULL,         NULL,           0},
	{"remotessid",      "Display UC/UR's Remote SSID",			remotessidCliCmdGet,	NULL,       	NULL,           0},
    {"remoteucr",      	"Display UC/UR's Remote MAC Address",	remoteUcrMacAddrCliCmdGet,NULL,       	NULL,           0},
    {"roguelegal",      "Display Legal AP List of Legal AP",    roguelegalCmdGet,       NULL,           NULL,           0},
    {"roguetype",       "Display Rogue AP Definition",          roguetypeCmdGet,        NULL,           NULL,           0},     
    {"rtsthreshold",    "Display RTS Threshold,(Default: 2347, Range: 1 ~ 2347)",                rtsCliCmdGet,           NULL,           NULL,           0},
    {"security",        "Display Wireless Security Mode",       secCliCmdGet,           NULL,           NULL,           0},
/*    {"shortpreamble",   "Display Short Preamble Usage",         shortPreambleCliCmdGet, NULL,           NULL,           0},*/
    {"snmpcontact",   	"Display SNMP Contact",      			snmpContactCliCmdGet,   NULL,           NULL,           0},
    {"snmpdevice",  	"Display SNMP Devic Name",     			snmpDeviceNameCliCmdGet,NULL,           NULL,           0},
	{"snmplocation",  	"Display SNMP Location",     			snmpLocationCliCmdGet,  NULL,           NULL,           0},
    {"snmpreadcommunity",   "Display SNMP Read Community",      snmpReadComCliCmdGet,   NULL,           NULL,           0},
	{"snmptrapcommunity", 	"Display SNMP Trap Community",     	snmpTrapComCliCmdGet, 	NULL,           NULL,           0},
    {"snmpwritecommunity",  "Display SNMP Write Community",     snmpWriteComCliCmdGet,  NULL,           NULL,           0},
    {"snmpmode",        "Display SNMP Mode",                    snmpModeCliCmdGet,      NULL,           NULL,           0},
 /*    {"snmpmanagemode",  "Display SNMP Manager Mode",            snmpManageCliCmdGet,    NULL,           NULL,           0},
   {"snmptrapmode",    "Display SNMP Trap Mode",               snmpTrapCliCmdGet,      NULL,           NULL,           0},*/
	{"ssh",     		"Display SSH Mode",       					sshModeCliCmdGet,		NULL,           NULL,           0},
    {"ssid",            "Display Service Set ID",               ssidCliCmdGet,          NULL,           NULL,           0},
    {"ssidbroadcast",   "Display SSID Broadcast Mode",          ssidModeCliCmdGet,      NULL,           NULL,           0},
#if 0
    {"stp",             "Display STP Mode",                     stpModeCliCmdGet,       NULL,           NULL,           0},
#endif
    {"syslog",          "Display Syslog Mode",                  syslogCliCmdGet,        NULL,           NULL,           0},
    {"syslogserver",    "Display Unicast Syslog Server Address",syslogSrvCliCmdGet,     NULL,           NULL,           0},
    {"time",            "Display Current System Time",          todCliCmdGet,           NULL,           NULL,           0},
	{"timemode",     	"Display Time Mode",       					timeModeCliCmdGet,		NULL,           NULL,           0},
    {"timezone",        "Display Time Zone Setting",            timezoneCliCmdGet,      NULL,           NULL,           0},
    {"daylight",         "Display Automatically adjust clock for daylight saving changes", daylightModeCliCmdGet,      NULL,           NULL,           0},
    {"uptime",          "Display Access Point Up Time",         uptimeCliCmdGet,        NULL,           NULL,           0},
    {"username",        "Display Login User Name",              usernameCliCmdGet,      NULL,           NULL,           0},
    {"vlan",            "Display VLAN Operational State",       vlanModeCliCmdGet,      NULL,           NULL,           0},
	{"vlandefault",		"Display AP Default VLAN",               defaultvlanCliCmdGet,      NULL,           NULL,           0},
    {"vlanid",          "Display the VLAN ID",                  vlanPvidCliCmdGet,      NULL,           NULL,           0},
	{"vlanmanagement",	"Display AP Management VLAN",               magvlanCliCmdGet,      NULL,           NULL,           0},
	{"vlantag",    		"Display Vlan Tag Mode",     				vlanTagModeCliCmdGet,           NULL,           NULL,           0},
	{"vlantagoverwds",  "Display Vlan Tag Over WDS Mode",     		vlanWDSTagModeCliCmdGet,           NULL,           NULL,           0},
 // {"wdsvlanlist",     "Display WDS VLAN List",                wdsVlanListCliCmdGet,   NULL,           NULL,           0},
	{"wlanaccess",     	"Display Wireless Web Access Mode",       	wlanAccessModeCliCmdGet,		NULL,           NULL,           0},
    {"wirelessmode",    "Display Wireless LAN Mode",            wirelessModeCliCmdGet,  NULL,           NULL,           0},
    {"wmm",             "Display WMM Mode",                     wmeCliCmdGet,           NULL,           NULL,           0},
    {"wmmps",             "Display WMM PowerSaving Mode",                     wmmpsCliCmdGet,           NULL,           NULL,           0},
//    {"sku",             "Display WLAN SKU",           				   skuCliCmdGet,      NULL,				NULL,           0},
 //   {"outPower",		"Display Out Power",                     outPowerCliCmdGet,           NULL,           NULL,           0},
#ifdef DAILY_REBOOT
    {"daily_reboot",       "Display status of Daily Reboot",        AutoRebootCliCmdGet,     NULL,           NULL,           0},
    {"daily_reboot_time",       "Display Time of Daily Reboot",        AutoRebootStartTimeCliCmdGet,     NULL,           NULL,           0},
//    {"auto_reboot_interval",        "Display Interval of Auto Reboot(in hours)", AutoRebootIntervalCliCmdGet,NULL,NULL,0},
#endif    
    PARSE_TOKEN_DELIMITER
};

static struct parse_token_s setCmdTbl[] = {
    {"11nguardinterval","Set 11n Guard Interval Mode",          genericCmdHandler,       dot11nGuardIntervalCmdTbl,  NULL,          0},
    {"11nchanbandwidth", "Set 11n Channel Bandwidth",           genericCmdHandler,       dot11nRadioBandCmdTbl,      NULL,          0},
    {"802_11d",         "Set 802.11d Mode",                     genericCmdHandler,       dot11dModeCmdTbl,NULL,          0},
    {"acl",             "Set Access Control",                   genericCmdHandler,       aclCmdTbl,      NULL,           0},
    {"active",          "Set Active (up) Mode",                 genericCmdHandler,       activeModeCmdTbl,NULL,          0},
/*    {"aging",           "Set Idle Timeout Interval",            agingCliCmdSet,          NULL,           NULL,           0},*/
    {"authentication",  "Set Authentication Type of WEP",       genericCmdHandler,       authCmdTbl,     NULL,           0},
    {"beaconinterval",  "Set Beacon Interval",                  beaconCliCmdSet,        NULL,           NULL,           0},
    {"bridgemac",       "Set Bridge's Remote MAC Address List",   genericCmdHandler,      remotePtmpMacListCmdTbl,        NULL,           0},
    {"channel",         "Set Radio Channel",                    chanCliCmdSet,          NULL,           NULL,           0},
    {"country",         "Set Country/Domain",                   genericCmdHandler,   countryCodeCmdTbl,           NULL,           0},
    {"ethdatarate",     "Set Ethernet Data Rate",               genericCmdHandler,   ethDataRateCmdTbl,           NULL,           0},
    {"ethAutoNegotiation",     "Set Ethernet Auto Negotiation Mode",               genericCmdHandler,   ethAutoNegoCmdTbl,           NULL,           0},
    {"ethDuplexMode",     "Set Ethernet Duplex Mode",               genericCmdHandler,   ethDuplexModeCmdTbl,           NULL,           0},

    {"ctsprotect",    	"Set CTS Protect Mode",     			genericCmdHandler,          ctsprotectmodeCmdTbl,           NULL,           0},
    {"daylight",        "Set Automatically adjust clock for daylight saving changes", genericCmdHandler,      daylightModeCmdTbl,           NULL,           0},      
    {"defaultkey"  ,    "Set Default Key Index",                defaultKeyCliCmdSet,    NULL,           NULL,           0},    
    {"devicename",      "Set Access Point Dvice Name",     		devicenameCliCmdSet,        NULL,           NULL,           0},
    {"dhcp",            "Set DHCPv4 Mode",                        genericCmdHandler,       dhcpCmdTbl,     NULL,           0},
    {"dhcpv6",            "Set DHCPv6 Mode",                        genericCmdHandler,       dhcpv6CmdTbl,     NULL,           0},
    {"dns2server",      "Set IP Address of Secondary DNS Server", nameSrv2CliCmdSet,        NULL,           NULL,           0},
    {"dnsserver",       "Set DNS Server IP Address",            nameSrvCliCmdSet,       NULL,           NULL,           0},
    {"dot1xdynkeylife", "Set 802.1x Dynamic Key Life Time (in Seconds)",dot1xKeyLifeCliCmdSet,NULL,     NULL,           0},
	{"dtiminterval"  ,  "Set DTIM interval,(Range:1 ~ 255)",    dtimModeCliCmdSet,      NULL,           NULL,           0},
    {"emailaddress",   	"Set Email Address for Log",           	emailAddressCliCmdSet,	NULL, 				NULL,           0},  
	{"emailalert",     	"Set Email Alert Mode",           		genericCmdHandler,		emailalertCmdTbl, 	NULL,           0},  
	{"emailinterval",	"Set Email Time Threshold",           	emailSendPeriodCliCmdSet,	NULL, 			NULL,           0},  
	{"emailqueue",     	"Set Email Queue Length",           	emailQueueLengthCliCmdSet,	NULL, 			NULL,           0},  
	{"emailserver",     "Set Email Alert Server",           	emailServerCliCmdSet,	NULL, 				NULL,           0},  
    {"encryption",      "Set WPA encryption Algorithm(TKIP/AES)",genericCmdHandler,      encryCmdTbl,    NULL,           0},
    {"fragthreshold",   "Set Fragment Threshold",               fragCliCmdSet,          NULL,           NULL,           0},
    {"gateway",         "Set Gateway IP Address",               gatewayCliCmdSet,       NULL,           NULL,           0},
    {"hostname",      	"Set Access Point Host Name",     		hostnameCliCmdSet,          NULL,           NULL,           0},
/*    {"http",            "Set HTTP Mode",                        genericCmdHandler,       httpCmdTbl,     NULL,           0},*/
#ifdef _BONJOUR_
	{"bonjour",    		"Set Bonjour Mode",       				genericCmdHandler,			bonjourmodeCmdTbl,           NULL,           0},//add by carole
#endif
	{"wps",	            "Set WPS Mode",				        genericCmdHandler,			wpsmodeCmdTbl,           NULL,           0},
	{"wps_pin",	            "Set WPS PIN",				        WpsPinCliCmdSet,			NULL,           NULL,           0},
	{"wps_pin_er",	            "Set WPS PIN External Registrar",				        genericCmdHandler,			wpsPinERCmdTbl,           NULL,           0},
	
	{"forcelan100m",    "Set Force LAN Port Speed to 100M",	genericCmdHandler,		force100mmodeCmdTbl,         NULL,           0},
	{"httpredirect",    "Set HTTP Redirect Mode",       		genericCmdHandler,			httpredirectmodeCmdTbl,           NULL,           0},
	{"httpredirectURL", "Set HTTP Redirect URL",       			httpURLCliCmdSet,			NULL,           NULL,           0},
    {"https",           "Set HTTPS Enable/Disable",             genericCmdHandler,       httpsCmdTbl,    NULL,           0},
    {"ipaddr",          "Set IP Address",                       ipAddrCliCmdSet,        NULL,           NULL,           0},
    {"ipmask",          "Set IP Subnet Mask",                   ipMaskCliCmdSet,        NULL,           NULL,           0},
	{"ipv6",     		"Set IPv6 Mode",       				genericCmdHandler,			ipv6modeCmdTbl, NULL,           0},
	{"ipv6addr",     	"Set IPv6 Local Address",       	ipv6AddrCliCmdSet,			NULL,           NULL,           0},
	
	{"ipv6dns1",        "Set IPv6 Primary DNS Server",     	    ipv6dns1CliCmdSet,		NULL,           NULL,           0},
    {"ipv6dns2",        "Set IPv6 Secondary DNS Server",     	ipv6dns2CliCmdSet,		NULL,           NULL,           0},
    
    {"ipv6gateway",     "Set IPv6 Default Gateway",     	ipv6gatewayCliCmdSet,		NULL,           NULL,           0},
    {"isolationbetweenssid",       "Set Isolate All Virtual APs State",    genericCmdHandler,      interVapForwardingCmdTbl,NULL,  0},
    {"isolationwithinssid","Set Isolation within VAP",  genericCmdHandler,       sepCmdTbl,      NULL,           0},
    {"key",             "Set WEP Key Value",                    keyCliCmdSet,           NULL,           NULL,           0},
    {"keylength",       "Set WEP Key Length",                   keyLengthCliCmdSet,     NULL,           NULL,           0}, 
    {"keyrenew",        "Set Group Key Update Interval (in Seconds)", groupKeyUpdateIntervalCliCmdSet,NULL,NULL,  0},
    {"lltd",            "Set LLTD Mode",                        genericCmdHandler,      lltdModeCmdTbl, NULL,           0},      
	{"loadbalance",    	"Set Load Balance Mode",     			genericCmdHandler,          loadbalancemodeCmdTbl,           NULL,           0},
	{"loadbalancessid", "Set Load Balance SSID",     			loadBalanceSSIDCliCmdSet,           NULL,           NULL,           0},
	{"logconfchange", 	"Set Log Configuration Changes Mode",   genericCmdHandler,		confchangeCmdTbl, 	NULL,           0},  
	{"logfail",     	"Set Log Authorized Login Mode",        genericCmdHandler,		loginfailCmdTbl, 	NULL,           0},  
	{"logsuccess",     	"Set Log Unauthorized Login Attempt Mode",  genericCmdHandler,	loginsuccessCmdTbl, NULL,           0},  
	{"logsyserror",     "Set Log System Error Messages Mode",   genericCmdHandler,		syserrorCmdTbl, 	NULL,           0},  
    {"mcastenhance",	"Set Multicast Packets Enhancement Enable/Disable",             genericCmdHandler,       multienhanceCmdTbl,    NULL,           0},
	{"md5supplicant",   "Set 802.1x MD5 Supplicant Mode",       genericCmdHandler,      dot1xSuppCmdTbl,NULL,           0},
	{"md5suppname",     "Set 802.1x Supplicant MD5 Name",       dot1xSuppUserCliCmdSet, NULL,           NULL,           0},
	{"md5supppassword", "Set 802.1x Supplicant MD5 Password",   dot1xSuppPassCliCmdSet, NULL,           NULL,           0},
	{"md5supptype",     "Set 802.1x MD5 Supplicant Type",       genericCmdHandler,       dot1xSuppTypeCmdTbl,NULL,        0},
    {"nativevlanid",    "Set Native VLAN ID",                   nativeVlanIdCliCmdSet,  NULL,           NULL,           0},
	{"ntp",     		"Set NTP Mode",       					genericCmdHandler,			ntpmodeCmdTbl,           NULL,           0},
    {"ntpserver",       "Set NTP Server IP Address",            ntpServerCliCmdSet,    NULL,           NULL,           0},  
    {"operationmode",   "Set operation Mode",                   genericCmdHandler,      opModeCmdTbl,   NULL,           0},
    {"password",        "Modify Login Password",                passwordCliCmdSet,      NULL,           NULL,           0},
	{"priority",		"Set Priority",               			priorityCliCmdSet,      	NULL,           NULL,           0},
    {"psk",             "Modify Pre-shared Key. For example, set psk \"12345678\".",                pskCliCmdSet,           NULL,           NULL,           0},
    {"radiusserver",    "Set RADIUS IP Address",                genericCmdHandler,      radiusSrvCliCmdSetTbl,           NULL,           0},
    {"radiusport",      "Set RADIUS Port Number",               genericCmdHandler,      radiusPortCliCmdSetTbl,         NULL,           0},
    {"radiussecret",    "Set RADIUS Shared Secret",             genericCmdHandler,      radiusSecretCliCmdSetTbl,       NULL,           0},
    {"remotessid",      "Set UC/UR's Remote SSID",       		remotessidCliCmdSet,	NULL,         	NULL,           0},
    {"remoteucr",       "Set UC/UR's Remote MAC Address",       remoteUcrMacAddrCliCmdSet,NULL,         NULL,           0},
    {"repeatermac",     "Set Repeater's MAC Address",           remotePtpMacAddrCliCmdSet,NULL,          NULL,           0},
	{"roguelegal",      "Add/Delete Legal AP MAC/OUI",          genericCmdHandler,       roguelegalCmdTbl,NULL,          0},
    {"roguetype",       "Set Rogue AP Definition",              genericCmdHandler,       roguetypeCmdTbl,NULL,           0},   
    {"rtsthreshold",    "Set RTS/CTS Threshold",                rtsCliCmdSet,           NULL,           NULL,           0},
    {"security",        "Set Wireless Security Mode",           genericCmdHandler,       secCmdTbl,      NULL,           0},
    {"shortpreamble",   "Set Short Preamble",                   genericCmdHandler,       shortPreambleCmdTbl,NULL,      0},
    {"snmpcontact",   	"Set SNMP Contact",      				snmpContactCliCmdSet,   NULL,           NULL,           0},
    {"snmpdevice",  	"Set SNMP Devic Name",     				snmpDeviceNameCliCmdSet,NULL,           NULL,           0},
	{"snmplocation",  	"Set SNMP Location",     				snmpLocationCliCmdSet,  NULL,           NULL,           0},
    {"snmpreadcommunity",   "Set SNMP Read Community",          snmpReadComCliCmdSet,   NULL,           NULL,           0},
	{"snmptrapcommunity", 	"Set SNMP Trap Community",     		snmpTrapComCliCmdSet, 	NULL,           NULL,           0},
    {"snmpwritecommunity",  "Set SNMP Write Community",         snmpWriteComCliCmdSet,  NULL,           NULL,           0},
    {"snmpmode",        "Set SNMP Mode",                        genericCmdHandler,      snmpModeCmdTbl, NULL,           0},
 /*   {"snmpmanagemode",  "Set SNMP Manager Mode",                genericCmdHandler,      snmpManageCmdTbl,   NULL,       0},
    {"snmptrapmode",    "Set SNMP Trap Mode",                   genericCmdHandler,      snmpTrapModeCmdTbl, NULL,       0},
*/	{"ssh",     		"Set SSH Mode",       					genericCmdHandler,			sshmodeCmdTbl,           NULL,           0},
    {"ssid",            "Set Service Set ID",                   ssidCliCmdSet,          NULL,           NULL,           0},
    {"ssidbroadcast",   "Set SSID Broadcast Mode",              genericCmdHandler,      ssidModeCmdTbl, NULL,           0},
#if 0
    {"stp",             "Set STP Mode",                         genericCmdHandler,      stpModeCmdTbl,  NULL,           0},
#endif
    {"syslog",          "Set Syslog Mode",                      genericCmdHandler,      syslogCmdTbl,   NULL,           0},
    {"syslogserver",    "Set Unicast Syslog Server Address",    syslogSrvCliCmdSet,     NULL,           NULL,           0},
	{"time",            "Set Current System Time",          	genericCmdHandler,          timesetCmdTbl,           NULL,           0},
	{"timemode",     	"Set Time Mode",       					genericCmdHandler,			timemodeCmdTbl,           NULL,           0},
    {"timezone",        "Set Time Zone Setting",                genericCmdHandler,      timezoneCmdTbl,           NULL,           0},      
    {"username",        "Modify Login User Name",               usernameCliCmdSet,      NULL,           NULL,           0},
    {"vlan",            "Set VLAN Operational State",           genericCmdHandler,       vlanModeCmdTbl, NULL,           0},
	{"vlandefault",		"Set AP Native VLAN",               defaultvlanCliCmdSet,      NULL,           NULL,           0},
    {"vlanid",          "Set VAP VLAN id",                     vlanPvidCliCmdSet,      NULL,           NULL,           0},
	{"vlanmanagement",	"Set AP Management VLAN",               magvlanCliCmdSet,      NULL,           NULL,           0},
	{"vlantag",    		"Set Vlan Tag Mode",     				genericCmdHandler,          vlantagmodeCmdTbl,           NULL,           0},
	{"vlantagoverwds",  "Set Vlan Tag Over WDS Mode",     		genericCmdHandler,          vlanWDStagmodeCmdTbl,           NULL,           0},
//{"wdsvlanlist",     "Set WDS VLAN List",                    wdsVlanListCliCmdSet,   NULL,           NULL,           0},
	{"wlanaccess",     	"Set Wireless Web Access Mode",     	genericCmdHandler,			wlanaccessmodeCmdTbl,           NULL,           0},
    {"wirelessmode",    "Set Wireless LAN Mode",                genericCmdHandler,       wirelessModeCmdTbl,NULL,        0},
    {"wmm",             "Set WMM Mode",                         genericCmdHandler,       wmeCmdTbl,      NULL,           0},
    {"wmmps",             "Set WMM PowerSaving Mode",           genericCmdHandler,       wmmpsCmdTbl,      NULL,           0},
//    {"sku",             "Set WLAN SKU",           				genericCmdHandler,       skuCmdTbl,      NULL,           0},
    //{"outPower",		"Set Out Power",           				genericCmdHandler,       outPowerCmdTbl,      NULL,           0},
#ifdef DAILY_REBOOT
	{"daily_reboot",	     "Set Daily Reboot function.",				        genericCmdHandler,			autoRebootCmdTbl,           NULL,           0},
    {"daily_reboot_time",  "Set Time of Daily Reboot. Format is 'Hour:Minute'. Hour: 0-23, Minute: 0-59.", AutoRebootStartTimeCliCmdSet,       NULL,      NULL,           0},
//    {"auto_reboot_interval",  "Set Interval of Auto Reboot", AutoRebootIntervalCliCmdSet,       NULL,      NULL,           0},
#endif
    PARSE_TOKEN_DELIMITER
};

struct parse_token_s cmdTbl[] = {
    {"config",          "Config WLAN X or Virtual AP X",        genericCmdHandler,      configCmdTbl,   NULL,           0},
    {"?",               "Display CLI Command List",             sccli_helpCmdHandler,   NULL,           CLI_HIDDENCMD,  0},
    {"help",            "Display CLI Command List",             sccli_helpCmdHandler,   NULL,           CLI_HIDDENCMD,  0},
    {"get",             NULL,                                   genericCmdHandler,      getCmdTbl,      NULL,           0},
    {"set",             NULL,                                   genericCliCmdSet,       setCmdTbl,      NULL,           0},
    {"tftp",            "Software upgrade/download via TFTP",   tftpCmdHandler,         NULL,           NULL,           0},
    {"factoryrestore",  "Restore to Default Factory Settings",  factoryCmdHandler,      NULL,           NULL,           0},
    {"gen_wps_pin",  	"Generate a new WPS PIN.",  				genWpsPinCmdHandler,      NULL,           NULL,           0},    
    {"gen_wpa_psk",  	"Generate a new WPA/WPA2 PSK.",  				genWpaPskCmdHandler,      NULL,           NULL,           0},    
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

    //printf("Error -- No value specified\n");
    return CLI_PARSE_ERROR;
}

#if 0
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
#endif
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
            /* WDS mode allow config vap 0 */
            if(apCfgOpModeGet(RADIO_24G) == CFG_OP_MODE_AP) {              
                if ((v < 0) || (v >= WLAN_MAX_VAP)) 
                {
                    printf("Invalid wlan bss number %d\n", v);
                } 
                else 
                {
                    pCli->vap = v;
                }
            }
            else
                printf("In WDS mode, only vap 0 could be configed.\n");
            printf("Current virtual AP: %d\n", pCli->vap);
            return CLI_PARSE_OK;
        }
    }
    return CLI_PARSE_ERROR;
} 

int
dot11nGuardIntervalCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("11n Guard Interval: %s\n", (scApCfgShortGIGet(pCli->unit)==1)? "Auto":((scApCfgShortGIGet(pCli->unit)==0)?"Short":"Long"));
    return CLI_PARSE_OK;
}

int
dot11nGuardIntervalCmdAuto(CLI *pCli, char *p, struct parse_token_s *pTbl)
{    
    scApCfgShortGISet(pCli->unit, 1);
    printf("11n Guard Interval: %s\n", "Auto");
    return CLI_PARSE_OK;
}

int
dot11nGuardIntervalCmdShort(CLI *pCli, char *p, struct parse_token_s *pTbl)
{    
    scApCfgShortGISet(pCli->unit, 0);
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
    if(apCfgOpModeGet(pCli->unit) == CFG_OP_MODE_ROGAP || 
        apCfgOpModeGet(pCli->unit) == CFG_OP_MODE_UC || 
        apCfgOpModeGet(pCli->unit) == CFG_OP_MODE_UR){
        printf("On \"Wireless Client/Repeater\" Mode, \"%s\" is not support!\n", "Auto-20/40MHz Channel");
        return CLI_PARSE_ERROR;
    }
        
    apCfgChannelWidthModeSet(pCli->unit, 1);
    printf("11n Channel Bandwidth: %s\n", "Auto-20/40MHz Channel");
    return CLI_PARSE_OK;
}

int
dot11nRadioBandCmdWide(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    if(apCfgOpModeGet(pCli->unit) == CFG_OP_MODE_ROGAP || 
        apCfgOpModeGet(pCli->unit) == CFG_OP_MODE_UC || 
        apCfgOpModeGet(pCli->unit) == CFG_OP_MODE_UR){
        printf("On \"Wireless Client/Repeater\" Mode, \"%s\" is not support!\n", "Wide-40MHz Channel");
        return CLI_PARSE_ERROR;
    }
    if(0/*pCli->unit == RADIO_24G*/)
        printf("On 2.4G, \"Wide-40MHz Channel\" is supported!");
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

static int   dtimModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("DTIM Interval: %d\n", scApCfgDtimIntervalGet(pCli->unit));
    return CLI_PARSE_OK;
}

static int   dtimModeCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    int v;

    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            if(v<1 || v > 255)
            {
                printf("Invalid DTIM Interval Range: 1 ~ 255\n");
                return CLI_PARSE_ERROR;
            }
            rc = scApCfgDtimIntervalSet(pCli->unit, v);
            if (rc != A_OK) {
                printf("Invalid DTIM Interval: %d\n", v);
                return CLI_PARSE_ERROR;
            }
        }
        printf("DTIM Interval: %d\n", scApCfgDtimIntervalGet(pCli->unit));
    }
    return CLI_PARSE_OK;
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
            printf("\n\t===MAC Address===\n");
    	    for(pScAclCurr = pScAcl; pScAclCurr; pScAclCurr = pScAclCurr->next)
    	    {
    			printf("\t%s\n",pScAclCurr->mac);        	
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
/*            
            if(tokenCountNum == 2)
                strcpy(tmpName, sccli_tokenPop(pCli));
            else
                strcpy(tmpName, "Unknown");
*/
            rc = apCfgAclAdd(pCli->unit, 0, macStr17,  "Unknown", 1);
            if (rc == A_OK) 
            {
                printf("%s had added to trusted list.\n",macStr17);
                return CLI_PARSE_OK;
            } 
            else
            {
                printf("The ACL MAC List had Full.\nYou must remove some MAC before add it.\n");
                return CLI_PARSE_ERROR;
            }

            goto aclAddUsage;
            break;
        default:
            goto aclAddUsage;
    }
    return CLI_PARSE_OK;
    
aclAddUsage:
    printf("set acl add \"MAC Address\" \n");
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
        scSetDefaultVap(pCli->unit, pCli->vap);
        printf("VAP Active Mode: %s\n", pAbleStr[0]);
    }
    return CLI_PARSE_OK;
}

int
activeModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgActiveModeSet(pCli->unit, pCli->vap, TRUE);
    printf("VAP Active Mode: %s\n", pAbleStr[1]);
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

int cliGetAutoNego()
{
    int index;
    switch (apCfgAutonegoGet())
    {
        case 0:
            index = 0;
            break;
        case 1:
            index = 1;
            break;
        default:
            index = 0;
            break;
    }
    return index;
}

int cliGetEthDataRate()
{
    int index;
    switch (apCfgEthDataRateGet())
    {
        case 0:
            index = 0;
            break;
        case 1000:
            index = 1;
            break;
        case 100:
            index = 2;
            break;
        case 10:
            index = 3;
            break;
        default:
            index = 0;
            break;
    }
    return index;
}

void cliSetEthDataRate(int index)
{
    switch(index)
    {
        case 0:
            apCfgEthDataRateSet(0);
            break;
        case 1:
            apCfgEthDataRateSet(1000);
            break;
        case 2:
            apCfgEthDataRateSet(100);
            break;
        case 3:
            apCfgEthDataRateSet(10);
            break;
        default:
            apCfgEthDataRateSet(0);
            break;
    }
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
fwversionCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char versionStr[30];
    getVersion(versionStr);
    printf("Software Version: %s\n", versionStr);
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

//set time of date
/*
 * day set format : 1999/01/01 
 */
int
todCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char date[32], *pToken;
    char *value;
    char *separ = "/";
    int rc;
    
    if (sccli_tokenCount(pCli) != 1) {
        printf("USEAGE for Date:\n");
        printf("\tForamt : xxxx/xx/xx(year/month/day)\n");
        return CLI_PARSE_ERROR;
    }
    if(apCfgTimeModeGet() != 1){
        printf("Time Mode: Automatically\n");
        printf("Please set Time mode to Manually First.\n");
        return CLI_PARSE_ERROR;
    }
    pToken = sccli_tokenPop(pCli);
    
    strcpy(date, pToken);
    
    value = strtok(date, separ);
    rc = apCfgTimeYearSet(atoi(value));
    if(rc != A_OK) return CLI_PARSE_ERROR;
    value = strtok(NULL, separ);
    rc = apCfgTimeMonSet(atoi(value));
    if(rc != A_OK) return CLI_PARSE_ERROR;
    value = strtok(NULL, separ);
    rc = apCfgTimeDaySet(atoi(value));
    if(rc != A_OK) return CLI_PARSE_ERROR;
    
    printf("Date Set:%04d/%02d/%02d\n", apCfgTimeYearGet(), apCfgTimeMonGet(), apCfgTimeDayGet());
    return CLI_PARSE_OK;
}
#if 0
static int isValidChar(char c)
{
	const char validChar[] = {"0123456789"};
	if(index(validChar, (int)c))
		return 1;
	else
		return 0;
}
#endif
//set time of clock 
/*
 * time set format : 16:05:58
 */ 
int
tocCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char iclock[32], *pToken;
    char *value;
    char *separ = ":";
    int rc;
    
    if (sccli_tokenCount(pCli) != 1) {
        printf("USEAGE for Time:\n");
        printf("\tForamt : xx:xx:xx(hour:minute:second)\n");
        return CLI_PARSE_ERROR;
    }
    if(apCfgTimeModeGet() != 1){
        printf("Time Mode: Automatically\n");
        printf("Please set Time mode to Manually First.\n");
        return CLI_PARSE_ERROR;
    }
    pToken = sccli_tokenPop(pCli);
    
    strcpy(iclock, pToken);

    value = strtok(iclock, separ);
    rc = apCfgTimeHourSet(atoi(value));
    if(rc != A_OK) return CLI_PARSE_ERROR;
    value = strtok(NULL, separ);
    rc = apCfgTimeMinSet(atoi(value));
    if(rc != A_OK) return CLI_PARSE_ERROR;
    value = strtok(NULL, separ);
    rc = apCfgTimeSecSet(atoi(value));
    if(rc != A_OK) return CLI_PARSE_ERROR;

    printf("Time Set: %02d:%02d:%02d\n", apCfgTimeHourGet(), apCfgTimeMinGet(), apCfgTimeSecGet());
    return CLI_PARSE_OK;
}

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
    FILE *fp;
    char buffer[64];
    char *separator = "-";
    char *pCountry = NULL;
    char *pCode = NULL;
    int flag = 0;
    
    do{
        fp = fopen("/tmp/country_list", "r");
    }while(fp == NULL);
    while(!feof(fp)){
        buffer[0]='\0';
        fgets(buffer,63,fp);  
        pCountry = strtok(buffer, separator);
        pCode = strtok(NULL, separator);
        if(atoi(pCode) == apCfgCountryCodeGet()) {
            flag = 1;
            break;
        }
    }
    fclose(fp);
    
    if(flag == 1)
        printf("Country/Domain: %s\n",pCountry);
    else{
        printf("Country/Domain: Error\n");
    }    
    return CLI_PARSE_OK;
}

int
countryCodeCliCmdSet(CLI *pCli, char *pToken, struct parse_token_s *pNextTbl)
{   
    FILE *fp;
    char buffer[64];
    char *separator = "-";
    char *pCountry = NULL;
    char *pCode = NULL;
    int flag = 0;

    do{
        fp = fopen("/tmp/country_list", "r");
    }while(fp == NULL);
    
    if (sccli_tokenCount(pCli) == 0) {
        struct parse_token_s *pTbl = sccli_tokenCurTblGet();
        
        for ( ; pTbl->fHandler; pTbl++){
            if ( strncasecmp(pToken, pTbl->pCmd, strlen(pTbl->pCmd)) == 0){
                while(fgets(buffer,63,fp)){
                    if((pCountry = strchr(buffer, ' ')) != NULL)
                        memmove(pCountry, pCountry+1, strlen(pCountry+1));
                    pCountry = strtok(buffer, separator);
                    pCode = strtok(NULL, separator);
                    if(strncmp(pCountry, pTbl->pCmd, strlen(pTbl->pCmd)) == 0) {
                        flag = 1;
                        break;
                    }
                }
                
                fclose(fp);
                if(flag == 0){
                    printf("Invalid Country/Region. The Country isn't in current Domain.\n");
                    return CLI_PARSE_ERROR;
                }
                apCfgCountryCodeSet(atoi(pCode));                
                printf("Country/Region: %s\n", pTbl->pCmd);
                return CLI_PARSE_OK;
            }
        }
    }
    
    fclose(fp);
    printf("Invalid Country/Region.\n");
    return CLI_PARSE_ERROR;
}

int dhcpModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("DHCPv4 Mode: %s\n",apCfgDhcpEnableGet()?"Client" : 
        (apCfgDhcpServerEnableGet()? "Server":"Disabled"));
    return CLI_PARSE_OK;
}

int dhcpv6ModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("DHCPv6 Mode: %s\n",apCfgDhcp6EnableGet()?"Client" : "Disabled");
    return CLI_PARSE_OK;
}


int dhcpCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgDhcpEnableSet(FALSE);
    apCfgDhcpServerEnableSet(FALSE);
    printf("DHCPv4 Mode: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}

int dhcpv6CliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgDhcp6EnableSet(FALSE);
    printf("DHCPv6 Mode: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}

int dhcpCliCmdClient(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgDhcpEnableSet(TRUE);
    apCfgDhcpServerEnableSet(FALSE);
    printf("DHCPv4 Mode: %s\n", "Client");
    return CLI_PARSE_OK;
}

int dhcpv6CliCmdClient(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgDhcp6EnableSet(TRUE);
    printf("DHCPv6 Mode: %s\n", "Client");
    return CLI_PARSE_OK;
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
opModeCliCmdRogueAp(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgOpModeSet(pCli->unit, CFG_OP_MODE_ROGAP);
    printf("Operating as %s\n", "Wireless Monitor");
    return CLI_PARSE_OK;
}

int
remotessidCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{

    #ifdef  __ICONV_SUPPORT__
	char converted_text[128] = {0};
	char pDest[128];
	char *pSrc=NULL;
	int ret=0;
	pSrc=apCfgSsidGet(pCli->unit,0);
	 ret=do_convert(LAN2UTF, pSrc, strlen(pSrc), converted_text, 128);
	if(ret!=-1){
		if(strlen(converted_text)<128)
			strcpy(pDest, converted_text);
		else
			strcpy(pDest, pSrc);
		printf("Remote SSID: %s\n", pDest);
	}
	else{
		printf("Remote SSID: %s\n", pSrc);
	}
#else
    printf("Remote SSID: %s\n", apCfgSsidGet(pCli->unit, 0));
#endif
     return CLI_PARSE_OK;
}

int
remotessidCliCmdSet(CLI *pCli, char *s, struct parse_token_s *pTbl)
{
    char buf[64], *pStr;
    int  ssidLen = 0;
    A_STATUS result;
#ifdef  __ICONV_SUPPORT__
	char converted_text[128] = {0};
	char *pSrc=NULL;
	char pDest[128];
	int ret=0;
#endif
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
#ifdef  __ICONV_SUPPORT__
	 ret=do_convert(UTF2LAN, buf, strlen(buf), converted_text, 128);
	if(ret!=-1){
		if(strlen(converted_text)<128)
			strncpy(buf, converted_text, MAX_SSID);
	}
	
#endif
    result = apCfgSsidSet(pCli->unit, 0, buf);
    if (result != A_OK) {
        printf("SSID is invalid.\n");
        return CLI_PARSE_ERROR;
    }
   #ifdef __ICONV_SUPPORT__
	pSrc=apCfgSsidGet(pCli->unit, 0);
	 ret=do_convert(LAN2UTF, pSrc, strlen(pSrc), converted_text, 128);
	if(ret!=-1){
		if(strlen(converted_text)<128)
			strcpy(pDest, converted_text);
		else
			strcpy(pDest, pSrc);
		printf("Remote SSID: %s\n",pDest);
	}
	else{
		printf("Remote SSID: %s\n",pSrc);
	}
#else
    printf("Remote SSID: %s\n",apCfgSsidGet(pCli->unit, 0));
#endif    
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
    
		if((apCfgOpModeGet(pCli->unit)!= CFG_OP_MODE_UC)&&(apCfgOpModeGet(pCli->unit)!= CFG_OP_MODE_UR)){
			printf("Operation mode must be UC/UR.\n");
            return CLI_PARSE_ERROR;
		}
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
        if((macStr12[1]!='2')&&(macStr12[1]!='4')&&(macStr12[1]!='6')&&(macStr12[1]!='8')&&(macStr12[1]!='0'))
       	{
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
    printf("SNMP Device Name: %s\n",apCfgDescGet());
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
    
            rc = apCfgDescSet(pStr);
            
            if (rc == A_OK) {
                printf("SNMP Device Name: %s\n", apCfgDescGet());
                return CLI_PARSE_OK;
            }
            printf("SNMP Device Name is too long\n");
        }      
    }
    return CLI_PARSE_ERROR;
}

int 
AutoRebootCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	int auto_reboot=0;
	
    auto_reboot=apCfgAutoRebootModeGet();
    printf("Daily Reboot: %s\n", pAbleStr[auto_reboot]);
    return CLI_PARSE_OK;
}

int 
AutoRebootStartTimeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	int hour=0, min=0;
	
	apCfgAutoRebootTimeGet(&hour, &min);
    printf("Time of Daily Reboot: %02d:%02d\n", hour, min);
    return CLI_PARSE_OK;
}

int 
AutoRebootStartTimeCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS     rc;
	int hour=0, min=0;
	int i=0;
	
    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        {    
            char *pStr = sccli_tokenPop(pCli);

			if(!strchr(pStr, ':')){
				printf("Bad format, correct format is 'Hour:Minute'. Hour: 0-23, Minute: 0-59.\n");
				return CLI_PARSE_ERROR;
			}
			while(i<strlen(pStr)){
				if((pStr[i] < '0' || pStr[i] > '9') && pStr[i]!=':'){
					printf("Bad format, correct format is 'Hour:Minute'. Hour: 0-23, Minute: 0-59.\n");
					return CLI_PARSE_ERROR;
				}
				i++;
			}
			sscanf(pStr, "%d:%d", &hour, &min);
            rc = scApCfgAutoRebootTimeSet(hour, min);
            if (rc == A_OK) {
            	apCfgAutoRebootTimeGet(&hour, &min);
                printf("Time of Daily Reboot: %02d:%02d\n", hour, min);
                return CLI_PARSE_OK;
            }
            printf("Bad format, correct format is 'Hour:Minute'. Hour: 0-23, Minute: 0-59.\n");
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
    printf("Email Alert: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}
int 
emailAlertCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgemailAlertsEnabledSet(1);
    printf("Email Alert: %s\n", pAbleStr[1]);
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
    char buff[65];
    int  serverLen = 0;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    while (sccli_tokenCount(pCli)) {
        char *pStr = sccli_tokenPop(pCli);

        serverLen = strlen(pStr);              
        if (serverLen > 64) {
            printf("SMTP Server string is too long\n");
            return CLI_PARSE_ERROR;
        }
        memcpy(buff,pStr,serverLen);
        buff[serverLen] = '\0';
        if(scValidUrl(buff) && strncasecmp(buff,"http://",7)!=0 && strncasecmp(buff,"https://",8)!=0)
	{
            apCfgsmtpMailServerSet(buff);
            printf("SMTP Server: %s\n",apCfgsmtpMailServerGet());
	}
        else{
            printf("SMTP Server is invalid\n");
            return CLI_PARSE_ERROR;
        }
    }

    return CLI_PARSE_OK;
}

int
emailAddressCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{

    printf("E-Mail Address for Logs: %s\n", apCfgemailAddrForLogGet());
    return CLI_PARSE_OK;
}
int
emailAddressCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char buff[65];
    int  mailaddrLen = 0;
    A_STATUS result;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    buff[0] = '\0';
    while (sccli_tokenCount(pCli)) {
        char *pStr = sccli_tokenPop(pCli);

        mailaddrLen += strlen(pStr) + 1;               /* +1 for null or space */
        if (mailaddrLen > 65) {
            printf("Email Address string is too long\n");
            return CLI_PARSE_ERROR;
        }
        strcpy(buff, pStr);
        
         /*
        if((pStr = strchr(buff, '@')) == NULL) {
            printf("Email Address string is miss '@'\n");
            return CLI_PARSE_ERROR;
        }
        *pStr = 0;
        if(!scValidStr(buff) || !scValidUrl(pStr+1)) {
            printf("Email Address string is Incalid.\n");
            return CLI_PARSE_ERROR;
        }
        */
        if(!scValidEmailAddr(buff, mailaddrLen-1)) {
            printf("Email Address string is Invalid.\n");
            return CLI_PARSE_ERROR;
        }
    }
    
    result = apCfgemailAddrForLogSet(buff);
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
    printf("Authorized Login: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}
int 
loginSuccessCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgAuthLoginSet(1);
    printf("Authorized Login: %s\n", pAbleStr[1]);
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
    printf("Unauthorized Login Attempt: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}
int 
loginFailCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgDeauthSet(1);
    printf("Unauthorized Login Attempt: %s\n", pAbleStr[1]);
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
    printf("System Error Messages: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}
int 
sysErrorCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgChangeSysFucSet(1);
    printf("System Error Messages: %s\n", pAbleStr[1]);
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
    printf("Configuration Changes: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}
int 
confChangeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgChangeCfgSet(1);
    printf("Configuration Changes: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}

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
    
    if(!scValidHostName(buf, strlen(buf))){
		printf("Access Point Host Name is invalid. Host Name is:\n 1) contains only letters, digits and hyphens.\n 2) can't begin or end with a hyphen.\n 3) doesn't allow all digits string.\n");
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
WpsPinCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("WPS PIN: %s\n", apCfgDevicePinGet(0, 0));
    return CLI_PARSE_OK;
}

int ValidateChecksum(unsigned long int PIN)
{
    unsigned long int accum = 0;
    
    accum += 3 * ((PIN / 10000000) % 10);
    accum += 1 * ((PIN / 1000000) % 10);
    accum += 3 * ((PIN / 100000) % 10);
    accum += 1 * ((PIN / 10000) % 10);
    accum += 3 * ((PIN / 1000) % 10);
    accum += 1 * ((PIN / 100) % 10);
    accum += 3 * ((PIN / 10) % 10);
    accum += 1 * ((PIN / 1) % 10);
    
     if(0 == (accum % 10))
        return 1;
     else
        return 0;
}

int IsValidWpsPin(char *wps_pin)
{
	int i;
	int valid=0;
	       
	for(i = 0; i < WPS_PIN_LEN; i++){
		if(wps_pin[i] < '0' || wps_pin[i] > '9')
			break;
	}
        
 	if(WPS_PIN_LEN == i)
		valid = ValidateChecksum(atol(wps_pin));	
	else
		valid=0;
	return valid;
}
int
WpsPinCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
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
    
    if(!IsValidWpsPin(buf)){
		printf("Invalid PIN Code\n");
    	return CLI_PARSE_ERROR;
    }
    
    rc = apCfgDevicePinSet(0,0, buf);
    if (rc != A_OK) {
        printf("Invalid PIN Code\n");
    }
    printf("WPS PIN: %s\n", apCfgDevicePinGet(0,0));
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
    printf("IPv6: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}

int 
ipv6ModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	if((!apCfgIpv6AddrGet())||(!strcmp(apCfgIpv6AddrGet(),""))){
        	printf ("Invalid Ipv6 address,Address can not be blank\n" );
        	return CLI_PARSE_ERROR;	
        }
	 if(apCfgIpv6AddrGet()){
        	char *string = strchr(apCfgIpv6AddrGet(),'/');
        	if(string == NULL){
        		printf ("Invalid Ipv6 Prefix Length\n" );
        		return CLI_PARSE_ERROR;	
        	}
        	string=string+1;
        	if((string == NULL)||(!strcmp(string,""))){
        		printf ("Invalid Ipv6 Prefix Length\n" );
        		return CLI_PARSE_ERROR;	
        	}
    }
    apCfgipv6modeSet(1);
    printf("IPv6: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}

int
ipv6AddrCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int count = 0;

    if(apCfgipv6modeGet())
    {
        if_infov6_t iface_status;
        printf("IPv6 Address: ");

        if(apCfgDhcp6EnableGet() == 0)
	{
		printf("%s", apCfgIpv6AddrGet());
	}
	else if(apCfgDhcp6EnableGet() == 1) 
	{
	        getMgtBrv6Info(&iface_status, 1);
	        if(strlen(iface_status.ipaddr) > 0) {
	            printf("%s", iface_status.ipaddr);
	            count++;
	        }
	
	        if(apCfgRadvdEnableGet() == 1)
	        {
	            getMgtBrv6Info(&iface_status, 2);
	            if(strlen(iface_status.ipaddr) > 0) {
	                if(count == 0)
	                    printf("%s", iface_status.ipaddr);
	                else
	                    printf("\t\t\t %s", iface_status.ipaddr);
	                count++;
	             }
	        }
	}
	printf("\n");
    }
    else
        printf("IPv6 Disable.\n");

    return CLI_PARSE_OK;
}
int
ipv6AddrCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    char *ipv6Str = NULL;
    
    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    ipv6Str = sccli_tokenPop(pCli);
    if(!scValidIPv6(ipv6Str, 1)){
        printf("Input Ipv6 Address is wrong!\n");
        return CLI_PARSE_ERROR;
    }
    rc = apCfgIpv6AddrSet(ipv6Str);
    if (rc != A_OK) {
        printf("Input Ipv6 Address is wrong!\n");
        return CLI_PARSE_ERROR;
    }
    printf("IPv6 Address: %s\n", apCfgIpv6AddrGet());
    return CLI_PARSE_OK;
}
int   
ipv6dns1CliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("IPv6 Primary DNS: %s\n", apCfgNameSrv61Get());
    return CLI_PARSE_OK;
}
int   
ipv6dns1CliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    char *dns6Str = NULL;
    
    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    dns6Str = sccli_tokenPop(pCli);
    
    if(!scValidIPv6(dns6Str, 0)) {           //change 1 to 0
        printf("Input Primary DNS is wrong!\n");
        return CLI_PARSE_ERROR;
    }
    
    rc = apCfgNameSrv61Set(dns6Str);
    if (rc != A_OK) {
        printf("Input Primary DNS is wrong!\n");
        return CLI_PARSE_ERROR;
    }
    printf("IPv6 Primary DNS: %s\n", apCfgNameSrv61Get());
    
    return CLI_PARSE_OK;
}
int   
ipv6dns2CliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("IPv6 Secondary DNS: %s\n", apCfgNameSrv62Get());
    return CLI_PARSE_OK;
}
int   
ipv6dns2CliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    char *dns6Str = NULL;
    
    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    dns6Str = sccli_tokenPop(pCli);
    
    if(!scValidIPv6(dns6Str, 0)) {           //change 1 to 0
        printf("Input Secondary DNS is wrong!\n");
        return CLI_PARSE_ERROR;
    }
    
    rc = apCfgNameSrv62Set(dns6Str);
    if (rc != A_OK) {
        printf("Input Secondary DNS is wrong!\n");
        return CLI_PARSE_ERROR;
    }
    printf("IPv6 Secondary DNS: %s\n", apCfgNameSrv62Get());
    
    return CLI_PARSE_OK;
}
int
ipv6gatewayCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int count = 0;
    if(apCfgipv6modeGet())
    {
        if_infov6_t iface_status;
        printf("IPv6 Default Gateway: ");

        if(apCfgDhcp6EnableGet() == 0)
	{
		printf("%s", apCfgGatewayv6AddrGet());
	}
	else if(apCfgDhcp6EnableGet() == 1) 
	{
		getMgtBrv6Info(&iface_status, 1);
	        if(strlen(iface_status.gw) > 0) {
	            printf("%s", iface_status.gw);
	            count++;
	        }
	
	        if(apCfgRadvdEnableGet() == 1)
	        {
	            getMgtBrv6Info(&iface_status, 2);
	            if(strlen(iface_status.gw) > 0) {
	                if(count == 0)
	                    printf("%s", iface_status.gw);
	                else
	                    printf("\t\t\t %s", iface_status.gw);
	                count++;
		     }
	        }
        }
	printf("\n");
    }
    else
        printf("IPv6 Disable.\n");
    return CLI_PARSE_OK;
}
int
ipv6gatewayCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    char *gwv6Str = NULL;
    
    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    gwv6Str = sccli_tokenPop(pCli);
     
    if(!scValidGWv6(gwv6Str)) {
        printf("Input Default Gateway is wrong!\n");
        return CLI_PARSE_ERROR;
    }
    rc = apCfgGatewayv6AddrSet(gwv6Str);
    if (rc != A_OK) {
        printf("Input Default Gateway is wrong!\n");
        return CLI_PARSE_ERROR;
    }
    printf("IPv6 Default Gateway: %s\n", apCfgGatewayv6AddrGet());
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
    printf("NTP Mode: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}
int 
ntpModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgNtpModeSet(1);
    printf("NTP Mode: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}


#ifdef _BONJOUR_
//add by carole
int 
BonjourModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Bonjour Mode: %s\n", pAbleStr[apCfgBonjourGet()]);
    return CLI_PARSE_OK;
}
int 
BonjourModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgBonjourSet(0);
    printf("Bonjour Mode: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}
int 
BonjourModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgBonjourSet(1);
    printf("Bonjour Mode: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}
//end add
#endif

int
WpsModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("WPS Mode: %s\n", pAbleStr[apCfgWpsModeGet(0)]);
    return CLI_PARSE_OK;
}
int
WpsModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWpsModeSet(0, 0);
    printf("WPS Mode: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}
int
WpsModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWpsModeSet(0, 1);
    printf("WPS Mode: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}

int
WpsPinERCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("WPS PIN External Registrar: %s\n", pAbleStr[apCfgWpsPinERGet(0)]);
    return CLI_PARSE_OK;
}
int
WpsPinERCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWpsPinERSet(0, 0);
    printf("WPS PIN External Registrar: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}
int
WpsPinERCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWpsPinERSet(0, 1);
    printf("WPS PIN External Registrar: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}

int
AutoRebootCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgAutoRebootModeSet(0);
    printf("Daily Reboot: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}
int
AutoRebootCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgAutoRebootModeSet(1);
    printf("Daily Reboot: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}

int 
force100mModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Force LAN Port Speed to 100M: %s\n", pAbleStr[apCfgForce100mGet()]);
    return CLI_PARSE_OK;
}
int 
force100mModeCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgForce100mSet(0);
    printf("Force LAN Port Speed to 100M: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}
int 
force100mModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgForce100mSet(1);
    printf("Force LAN Port Speed to 100M: %s\n", pAbleStr[1]);
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
    printf("HTTP Redirect Mode: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}
int 
httpRDModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgRedirectModeSet(1);
    printf("HTTP Redirect Mode: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}

int
httpURLCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{

    printf("HTTP Redirect URL: %s\n", apCfgRedirectUrlGet());
    return CLI_PARSE_OK;
}
int
httpURLCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char buf[128];
    int  mailaddrLen = 0;
    A_STATUS result;

    if (sccli_tokenCount(pCli) == 0) {
        return CLI_PARSE_NO_VALUE;
    }

    if (sccli_tokenCount(pCli)) {
        char *pStr = sccli_tokenPop(pCli);
        strcpy(buf, pStr);
        
/*        if(!strncmp(pStr,"http://",7))
    	{
    	    pStr += 7;
    	}
    	else if(!strncmp(pStr,"https://",8))
    	{
    	    pStr += 8;
    	}
*/
		if(!strncmp(pStr,"https",5))
    	{
    	     printf("https can not support by http Redirect.\n");
    	    return CLI_PARSE_ERROR;
    	}
    	if(!scValidUrl(pStr)){
    	    printf("HTTP Redirect URL is invalid.\n");
    	    return CLI_PARSE_ERROR;
    	}
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
    printf("Wireless Web Access: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}
int 
wlanAccessModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWlanAccessSet(1);
    printf("Wireless Web Access: %s\n", pAbleStr[1]);
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
    printf("SSH: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}
int 
sshModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgSSHSet(1);
    printf("SSH: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}

int
aclLocalModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Access Control Local Mode: %s\n", apCfgAclTypeGet(pCli->unit, pCli->vap)?"Prevent":"Allow");
    return CLI_PARSE_OK;
}
int 
aclLocalModeCliCmdAllow(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgAclTypeSet(pCli->unit, pCli->vap, 0);
    printf("Access Control Local Mode: Allow\n");
    return CLI_PARSE_OK;
}
int 
aclLocalModeCliCmdPrevent(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgAclTypeSet(pCli->unit, pCli->vap, 1);
    printf("Access Control Local Mode: Prevent\n");
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
    printf("CTS Protection: %s\n", pAbleStr[0]);
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
    printf("Load Balancing: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}
int 
loadBalanceModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgBalanceModeSet(pCli->unit, 1);
    printf("Load Balancing: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}

int
loadBalanceSSIDCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int Balance = apCfgLoadBalanceGet(pCli->unit, pCli->vap);
    printf("Load Balancing SSID %d: %.2f%%\n", pCli->vap, Balance/100.0);
    return CLI_PARSE_OK;
}
int 
loadBalanceSSIDCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char *value = NULL;
    int v ;
    
    if (sccli_tokenCount(pCli) == 1) {
        value = sccli_tokenPop(pCli);
        v = (int)atof(value) * 100;
        if (v < 0 || v >10000) {
            printf("Load Balancing 0%% ~ 100%%\n");
            return CLI_PARSE_ERROR;
        }
        apCfgLoadBalanceSet(pCli->unit, pCli->vap, v); 
        printf("Set Load Balancing SSID %d: %.2f%%: \n", pCli->vap, apCfgLoadBalanceGet(pCli->unit, pCli->vap)/100.0);
        return CLI_PARSE_OK;
        
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
            apCfgVlanListApply(pCli->unit);
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
            apCfgVlanListApply(pCli->unit);
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
    printf("VLAN Tag over WDS: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}
int 
vlanWDSTagModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWdsVlanTagSet(1);
    printf("VLAN Tag over WDS: %s\n", pAbleStr[1]);
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
    printf("VLAN Operational State: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}

int
vlanModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgVlanModeSet(TRUE);
    printf("VLAN Operational State: %s\n", pAbleStr[1]);
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

int wdsVlanListCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTb1)
{
    printf("WDS VLAN List: %s\n", apCfgwdsVlanListGet());
    return CLI_PARSE_OK;
}

int wdsVlanListCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    char *wdsvlanlist = NULL;
    char testlist[21];
    memset(testlist, 0, 21);
    
    if (sccli_tokenCount(pCli) == 0)
    {
        apCfgwdsVlanListSet(testlist);
        printf("Cleanup WDS VLAN List!\n");
        return CLI_PARSE_OK;
    }

    wdsvlanlist = sccli_tokenPop(pCli);
    if(strlen(wdsvlanlist) < 21)
        strcpy(testlist, wdsvlanlist);
    else
        return CLI_PARSE_ERROR;
    if(scValidwdsVlanList(testlist) == FALSE)
    {
        printf("Invalid WDS VLAN List! \n");
        return CLI_PARSE_ERROR;
    }
    rc = apCfgwdsVlanListSet(wdsvlanlist);
    if(rc != A_OK)
        printf("Error happened when setting WDS VLAN List!\n");
    else
        printf("Set WDS VLAN List: %s\n",wdsvlanlist);
    return CLI_PARSE_OK;
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
            if (v <VLAN_TAG_MIN || v >VLAN_TAG_MAX) {
                printf("range of vap VLAN %d ~ %d\n", VLAN_TAG_MIN, VLAN_TAG_MAX);
                return CLI_PARSE_ERROR;
            }
            rc = apCfgVlanPvidSet(pCli->unit, pCli->vap, v);
            if (rc != A_OK) {
                printf("Unable to set the VLAN Id.\n");
                return CLI_PARSE_ERROR;
            }
            apCfgVlanListApply(pCli->unit);
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
wirelessModeCliCmd11n(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWlanStateSet(pCli->unit, 1);
    apCfgFreqSpecSet(pCli->unit, MODE_SELECT_11N);
    printf("Wireless Mode: %s\n", "11n");
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

int
wirelessModeCliCmd11g(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWlanStateSet(pCli->unit, 1);
    apCfgFreqSpecSet(pCli->unit, MODE_SELECT_11G);
    printf("Wireless Mode: %s\n", "11g");
    return CLI_PARSE_OK;
}

int
wirelessModeCliCmd11bg(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWlanStateSet(pCli->unit, 1);
    apCfgFreqSpecSet(pCli->unit, MODE_SELECT_11BG);
    printf("Wireless Mode: %s\n", "11bg");
    return CLI_PARSE_OK;
}

int
opModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Operation Mode: %s\n", (apCfgOpModeGet(pCli->unit)== CFG_OP_MODE_AP)? "Access Point": 
              ((apCfgOpModeGet(pCli->unit)== CFG_OP_MODE_PPT)? "Bridge(Point-to-Point)":
              ((apCfgOpModeGet(pCli->unit)== CFG_OP_MODE_MPT)? "Wireless WDS Bridge.":
              ((apCfgOpModeGet(pCli->unit)== CFG_OP_MODE_AP_PTP)?"Wireless WDS Repeater.":
              ((apCfgOpModeGet(pCli->unit)== CFG_OP_MODE_AP_PTMP)?"Wireless signal repeater.":
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
#if 0
int
opModeCliCmdPpt(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgOpModeSet(pCli->unit, CFG_OP_MODE_PPT);
    printf("Operating as %s\n", "Bridge(Point-to-Point)");
    return CLI_PARSE_OK;
}
#endif
int
opModeCliCmdMpt(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgOpModeSet(pCli->unit, CFG_OP_MODE_MPT);
    printf("Operating as %s\n", "Wireless WDS Bridge.");
    return CLI_PARSE_OK;
}

int
opModeCliCmdApPtp(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgOpModeSet(pCli->unit, CFG_OP_MODE_AP_PTP);
    printf("Operating as %s\n", "Wireless WDS Repeater.");
    return CLI_PARSE_OK;
}

int
opModeCliCmdApPtmp(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgOpModeSet(pCli->unit, CFG_OP_MODE_AP_PTMP);
    printf("Operating as %s\n", "Wireless signal repeater.");
    return CLI_PARSE_OK;
}

int
remotePtpMacAddrCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char macStr[18];
    A_STATUS rc;
    
    if(apCfgOpModeGet(pCli->unit)!= CFG_OP_MODE_AP_PTP)
    {
        printf("Operation mode must WDS repeater.\n");
        return CLI_PARSE_ERROR;
    }
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
    if(apCfgOpModeGet(pCli->unit)!= CFG_OP_MODE_AP_PTP)
    {
        printf("Operation mode must WDS repeater.\n");
        return CLI_PARSE_ERROR;
    }
    
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
         if((macStr12[1]!='2')&&(macStr12[1]!='4')&&(macStr12[1]!='6')&&(macStr12[1]!='8')&&(macStr12[1]!='0'))
       	{
       		printf("Invalid Input\n");
            return CLI_PARSE_ERROR;
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
    int maxMAC = (apCfgOpModeGet(pCli->unit)== CFG_OP_MODE_MPT)?4:
                    ((apCfgOpModeGet(pCli->unit)== CFG_OP_MODE_AP_PTMP)?3:0);
    if(maxMAC == 0)
    {
        printf("Operation Mode must be Bridge mode or Wireless signal repeater..\n");
        return CLI_PARSE_ERROR;
    }
    printf("Bridge's Remote Mac Address List: \n");
    for(i=0; i<maxMAC; i++){
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
                
                int maxMAC = (apCfgOpModeGet(pCli->unit)== CFG_OP_MODE_MPT)?4:
                                ((apCfgOpModeGet(pCli->unit)== CFG_OP_MODE_AP_PTMP)?3:0);
                int index = atoi(pToken)-1;
                
                if(index < 0 && index >= maxMAC ){
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
                 if((macStr12[1]!='2')&&(macStr12[1]!='4')&&(macStr12[1]!='6')&&(macStr12[1]!='8')&&(macStr12[1]!='0'))
      			 	{
       					printf("Invalid Input\n");
           				 return CLI_PARSE_ERROR;
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
    if (apCfgAutoChannelGet(pCli->unit) == 1){
        FILE     *fp;
        char buffer[32];
        
    	do
    	{
        	fp = fopen("/tmp/chan_curr", "r");
        }while(fp == NULL);
    	
    	while(!feof(fp))
    	{
    	    buffer[0]='\0';
            fgets(buffer,31,fp);
        }
        fclose(fp);
        
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
    FILE     *fp;
    char buffer[32];
    int Max_channel = 0;
    
    do{
        fp = fopen("/tmp/chan_list", "r");
    }while(fp == NULL);
    while(!feof(fp))
    {
        buffer[0]='\0';
        fgets(buffer,31,fp); 
        if(strcmp(buffer,"current:\n") == 0)
            break;
        Max_channel = atoi(buffer);
    }
    fclose(fp);
    
    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            if(v<0 || v>Max_channel){
                printf("The valid channel: 0~%d (0 means Automatic)\n",Max_channel);
                return CLI_PARSE_ERROR;
            }
            
            if(v != 0) {
                apCfgAutoChannelSet(pCli->unit, 0);
                rc = apCfgRadioChannelSet(pCli->unit, v);
            }
            else
                rc = apCfgAutoChannelSet(pCli->unit, 1);
            if (rc != A_OK) {
                printf("Invalid Channel: %d\n", v);
            }
            if(v == 0)
                printf("Channel: (Automatic)\n");
            else
                printf("Channel: %d\n", apCfgRadioChannelGet(pCli->unit));
            return CLI_PARSE_OK;
        }
    }
    printf("The valid channel: 0~%d (0 means Automatic)\n", Max_channel);
    return CLI_PARSE_ERROR;
}

int sepCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Wireless Isolation(within SSID): %s\n", pAbleStr[apCfgIntraVapForwardingGet(pCli->unit, pCli->vap)]);
    return CLI_PARSE_OK;
}

int sepCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgIntraVapForwardingSet(pCli->unit, pCli->vap, TRUE);
    printf("Wireless Isolation(within SSID): %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}

int sepCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgIntraVapForwardingSet(pCli->unit, pCli->vap, FALSE);
    printf("Wireless Isolation(within SSID): %s\n", pAbleStr[0]);
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
    printf("WMM: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}

int
wmeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWmeSet(pCli->unit, pCli->vap, TRUE);
    printf("WMM: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}

int
wmmpsCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("WMM PowerSaving Mode: %s\n", pAbleStr[apCfgWmmpsGet(pCli->unit, pCli->vap)]);
    return CLI_PARSE_OK;
}

int
outPowerCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	int PowerPerc[21] = {100,79,63,50,40,32,25,20,16,13,10,8,6,5,4,3,3,2,2,1,1};
	int index=apCfgPowerGet(0);	
	
	if(index<0 || index>20)
		index=0;
    printf("Out Power: %d%%\n", PowerPerc[index]);
    return CLI_PARSE_OK;
}

int
wmmpsCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWmmpsSet(pCli->unit, pCli->vap, FALSE);
    printf("WMM PowerSaving: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}

int
wmmpsCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgWmmpsSet(pCli->unit, pCli->vap, TRUE);
    printf("WMM PowerSaving: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}

int
tftpCmdHandler(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char    *hostname, *filename;
    char    *action;

    if(sccli_tokenCount(pCli) < 2) {
        printf("USEAGE For TFTP \n");
        printf("    Upgrade Firmware:       TFTP [serverip/name] [filename]\n");
        printf("    Upload/Download Config: TFTP [serverip/name] [filename] [up/down]\n");
        return CLI_PARSE_ERROR;
    }
    
    hostname = sccli_tokenPop(pCli);
    
    filename = sccli_tokenPop(pCli);
    
    if(strlen(hostname) > 32){
         printf("serverip/name string is too long.\n");
         return CLI_PARSE_ERROR;
    }
    if(strlen(filename) > 32){
         printf("filename string is too long.\n");
         return CLI_PARSE_ERROR;
    }
    
    if(sccli_tokenCount(pCli) == 1){
        action = sccli_tokenPop(pCli);
        if(strcmp(action, "up")==0 || strcmp(action, "down")==0)
            SYSTEM("tftp_cmd %s %s %s", hostname, filename, action); 
        else
            return CLI_PARSE_ERROR;
    }
    else if(sccli_tokenCount(pCli) == 0)
    {
        SYSTEM("tftp_cmd %s %s", hostname, filename);
    }else{
        printf("USEAGE For TFTP \n");
        printf("    Upgrade Firmware:       TFTP [serverip/name] [filename]\n");
        printf("    Upload/Download Config: TFTP [serverip/name] [filename] [up/down]\n");
        return CLI_PARSE_ERROR;
    }    
    
    return CLI_PARSE_OK;
}
int
factoryCmdHandler(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgFactoryRestore();
    apcfg_submit();
    COMMAND("reboot&");
    return CLI_PARSE_OK;
}

int generate_pin(char *pinValue)
{
    unsigned long accum = 0;
    int i;
    unsigned char buffer[7];
    int fd=0;

    fd=open("/dev/urandom", O_RDONLY);
    if(fd<0)
		return -1;
    read(fd, buffer, 7);
	close(fd);
    for(i=0;i<7;i++){
        buffer[i]%=10;
    }
    accum += 3 * (buffer[0] % 10); 
	accum += 1 * (buffer[1] % 10); 
	accum += 3 * (buffer[2] % 10); 
	accum += 1 * (buffer[3] % 10); 
	accum += 3 * (buffer[4] % 10); 
	accum += 1 * (buffer[5] % 10); 
	accum += 3 * (buffer[6] % 10); 
	
	accum = (accum % 10);
	accum = (10 - accum) % 10;
	sprintf(pinValue,"%01d%01d%01d%01d%01d%01d%01d%01ld",
	                    buffer[0],buffer[1],buffer[2],buffer[3],
	                    buffer[4],buffer[5],buffer[6],accum);
	pinValue[8] = '\0';
	return 0;
}

int
genWpsPinCmdHandler(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	char pin_str[12]={0};
	
	generate_pin(pin_str);
	printf("PIN Code: %s\n", pin_str);
	printf("You may use 'set wps_pin' to assign it into device.\n");
    return CLI_PARSE_OK;
}

int get_random_num(int *ran_num, int num, int char_len)
{
    int i;
    unsigned char buffer[128];
    int fd=0;

	if(num<=0)
		return -1;
    fd=open("/dev/urandom", O_RDONLY);
    if(fd<0)
		return -1;
    read(fd, buffer, num);
	close(fd);
    for(i=0;i<num;i++){
        ran_num[i]=buffer[i]%(char_len-1);
    }
    return 0;
}

char char_list[]="`1234567890-=~!@#$%^&*()_+qwertyuiop[]\{}|asdfghjkl;':zxcvbnm,./<>?";

int
genWpaPskCmdHandler(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	char psk_str[65]={0};
	int ran_num[128]={0}, hex=0, i=0, j=0;
	int v=0;
	
	if(sccli_tokenPopNum(pCli, &v, 10) == CLI_PARSE_ERROR){
		printf("Usage: gen_wpa_psk <key length>.\n");
		return CLI_PARSE_OK;
	}
	if(v<8 || v>64){
		printf("Invalid PSK length, valid rang is from 8 to 64.\n");
		return CLI_PARSE_OK;
	}
	if(v==64){
		hex=1;
		v=32;
	}
	get_random_num(ran_num, v, strlen(char_list));
	if(hex){
		for(i=0; i<v; i++){
			if((char_list[ran_num[i]]>>4 )< 0x0a)
				psk_str[j]='0'+(char_list[ran_num[i]]>>4);
			else
				psk_str[j]='A'+(char_list[ran_num[i]]>>4) - 10;
			j++;
			if((char_list[ran_num[i]] & 0x0F) < 0x0a)
				psk_str[j]='0'+ (char_list[ran_num[i]] & 0x0F);
			else
				psk_str[j]='A'+ (char_list[ran_num[i]] & 0x0F) -10;
			j++;
		}
		psk_str[j]=0;	
	}
	else{
		for(i=0; i<v; i++){
			psk_str[i]=char_list[ran_num[i]];
		}
		psk_str[i]=0;
	}
	printf("New PSK: %s\n", psk_str);
	printf("You may use 'set psk' to assign it into VAPs.\n");
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
    printf("RTS Threshold: %d\n", apCfgRtsThresholdGet(pCli->unit));
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
            printf("RTS Threshold: %d\n", apCfgRtsThresholdGet(pCli->unit));
            return CLI_PARSE_OK;
        }
    }
    printf("Invalid RTS Threshold %d\n", v);
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
    printf("Wireless Isolation (between SSID): %s\n", 
             pAbleStr[apCfgInterVapForwardingGet(pCli->unit)]);
    return CLI_PARSE_OK;
}

int
interVapForwardingCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgInterVapForwardingSet(pCli->unit, FALSE);
    printf("Wireless Isolation (between SSID): %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}

int
interVapForwardingCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgInterVapForwardingSet(pCli->unit, TRUE);
    printf("Wireless Isolation (between SSID): %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}
int
outPowerCliCmd5(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	apCfgPowerSet(0,13);
    return CLI_PARSE_OK;
}

int
outPowerCliCmd6(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	apCfgPowerSet(0,12);
    return CLI_PARSE_OK;
}

int
outPowerCliCmd8(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	apCfgPowerSet(0,11);
    return CLI_PARSE_OK;
}

int
outPowerCliCmd10(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	apCfgPowerSet(0,10);
    return CLI_PARSE_OK;
}

int
outPowerCliCmd13(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	apCfgPowerSet(0,9);
    return CLI_PARSE_OK;
}

int
outPowerCliCmd16(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	apCfgPowerSet(0,8);
    return CLI_PARSE_OK;
}

int
outPowerCliCmd20(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	apCfgPowerSet(0,7);
    return CLI_PARSE_OK;
}

int
outPowerCliCmd25(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	apCfgPowerSet(0,6);
    return CLI_PARSE_OK;
}

int
outPowerCliCmd32(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	apCfgPowerSet(0,5);
    return CLI_PARSE_OK;
}

int
outPowerCliCmd40(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	apCfgPowerSet(0,4);
    return CLI_PARSE_OK;
}

int
outPowerCliCmd50(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	apCfgPowerSet(0,3);
    return CLI_PARSE_OK;
}

int
outPowerCliCmd63(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	apCfgPowerSet(0,2);
    return CLI_PARSE_OK;
}

int
outPowerCliCmd79(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	apCfgPowerSet(0,1);
    return CLI_PARSE_OK;
}

int
outPowerCliCmd100(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
	apCfgPowerSet(0,0);
    return CLI_PARSE_OK;
}

int
ssidCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{

   #ifdef  __ICONV_SUPPORT__
	char converted_text[128] = {0};
	char pDest[128];
	char *pSrc=NULL;
	int ret=0;
	pSrc=apCfgSsidGet(pCli->unit, pCli->vap);
	 ret=do_convert(LAN2UTF, pSrc, strlen(pSrc), converted_text, 128);
	if(ret!=-1){
		if(strlen(converted_text)<128)
			strcpy(pDest, converted_text);
		else
			strcpy(pDest, pSrc);
		printf("SSID: %s\n",pDest);
	}
	else{
		printf("SSID: %s\n",pSrc);
	}
#else
    printf("SSID: %s\n", apCfgSsidGet(pCli->unit, pCli->vap));
#endif
return CLI_PARSE_OK;
}

int
ssidCliCmdSet(CLI *pCli, char *s, struct parse_token_s *pTbl)
{
    char buf[64], *pStr;
    int  ssidLen = 0;
    A_STATUS result;
#ifdef  __ICONV_SUPPORT__
	char converted_text[128] = {0};
	char *pSrc=NULL;
	char pDest[128];
	int ret=0;
#endif
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
#ifdef  __ICONV_SUPPORT__
	 ret=do_convert(UTF2LAN, buf, strlen(buf), converted_text, 128);
	if(ret!=-1){
		if(strlen(converted_text)<128)
			strncpy(buf, converted_text, MAX_SSID);
	}
#endif
    result = apCfgSsidSet(pCli->unit, pCli->vap, buf);
    if (result != A_OK) {
        printf("SSID is invalid.\n");
        return CLI_PARSE_ERROR;
    }
    if(ssidLen == 0){
        activeModeCliCmdDisable(pCli, s, pTbl);
        return CLI_PARSE_OK;
    }
    apCfgActiveModeSet(pCli->unit, pCli->vap,TRUE);
    apCfgSsidModeSet(pCli->unit, pCli->vap, FALSE);
    #ifdef __ICONV_SUPPORT__
	pSrc=apCfgSsidGet(pCli->unit, pCli->vap);
	 ret=do_convert(LAN2UTF, pSrc, strlen(pSrc), converted_text, 128);
	if(ret!=-1){
		if(strlen(converted_text)<128)
			strcpy(pDest, converted_text);
		else
			strcpy(pDest, pSrc);
		printf("SSID: %s\n",pDest);
	}
	else{
		printf("SSID: %s\n",pSrc);
	}
#else
    printf("SSID: %s\n",apCfgSsidGet(pCli->unit, pCli->vap));
#endif
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
    printf("SSID Broadcast Mode: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}

int
ssidModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgSsidModeSet(pCli->unit, pCli->vap, FALSE);
    printf("SSID Broadcast Mode: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}
/*
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
    printf("STP Mode: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}

int
stpModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgStpSet(TRUE);
    printf("STP Mode: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}
*/

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
    printf("LLTD Mode: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}

int
lltdModeCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfglltdSet(TRUE);
    printf("LLTD Mode: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}
int
ethAutoNego(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = cliGetAutoNego();

    printf("Ethernet Auto Negotiation: %s\n", pEthAutoNegoStr[index]);
    return CLI_PARSE_OK;
}

int
ethDuplexModeGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = apCfgDuplexmodeGet();

    printf("Ethernet Duplex Mode: %s\n", pEthDuplexModeStr[index]);
    return CLI_PARSE_OK;
}

int
ethAutoNegoCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    if (apCfgForce100mGet())
        printf("Warning: Force LAN Port Speed to 100M is enabled, disable it first to set Auto Negotiation Mode here.\n");
    else {
        apCfgAutonegoSet(1);
        printf("Ethernet Auto Negotiation Mode: %s\n", pEthAutoNegoStr[1]);
    }
    return CLI_PARSE_OK;
}

int
ethAutoNegoCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{

    if (apCfgForce100mGet())
        printf("Warning: Force LAN Port Speed to 100M is enabled, disable it first to set Auto Negotiation Mode here.\n");
    else {
        apCfgAutonegoSet(0);
        printf("Ethernet Auto Negotiation Mode: %s\n", pEthAutoNegoStr[0]);
    }
    return CLI_PARSE_OK;
}
int
ethDuplexModeCliCmdFull(CLI *pCli, char *p, struct parse_token_s *pTbl)
{

    if (apCfgForce100mGet() || apCfgAutonegoGet())
        printf("Warning: Force LAN Port Speed to 100M is enabled Or Auto Negotiation is ebabled, disable it first to set Duplex Mode here.\n");
    else {
        apCfgDuplexmodeSet(1);
        printf("Ethernet Duplex Mode: %s\n", pEthDuplexModeStr[1]);
    }
    return CLI_PARSE_OK;
}

int
ethDuplexModeCliCmdHalf(CLI *pCli, char *p, struct parse_token_s *pTbl)
{

    if (apCfgForce100mGet() || apCfgAutonegoGet())
        printf("Warning: Force LAN Port Speed to 100M is enabled Or Auto Negotiation is ebabled, disable it first to set Duplex Mode here.\n");
    else {
        apCfgDuplexmodeSet(0);
        printf("Ethernet Duplex Mode: %s\n", pEthDuplexModeStr[0]);
    }
    return CLI_PARSE_OK;
}

int
ethDataRateCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = cliGetEthDataRate();

    printf("Ethernet Data Rate: %s\n", pEthDataRateStr[index]);
    return CLI_PARSE_OK;
}

int
ethDataRateCliCmdAuto(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = 0;

    if (apCfgForce100mGet())
        printf("Warning: Force LAN Port Speed to 100M is enabled, disable it first to set ethernet data rate here.\n");
    else {
        cliSetEthDataRate(index);
        printf("Ethernet Data Rate: %s\n", pEthDataRateStr[index]);
    }
    return CLI_PARSE_OK;
}

int
ethDataRateCliCmd1000(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = 1;

    if (apCfgForce100mGet())
        printf("Warning: Force LAN Port Speed to 100M is enabled, disable it first to set ethernet data rate here.\n");
    else {
        cliSetEthDataRate(index);
        printf("Ethernet Data Rate: %s\n", pEthDataRateStr[index]);
    }
    return CLI_PARSE_OK;
}

int
ethDataRateCliCmd100(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = 2;

    if (apCfgForce100mGet())
        printf("Warning: Force LAN Port Speed to 100M is enabled, disable it first to set ethernet data rate here.\n");
    else {
        cliSetEthDataRate(index);
        printf("Ethernet Data Rate: %s\n", pEthDataRateStr[index]);
    }
    return CLI_PARSE_OK;
}

int
ethDataRateCliCmd10(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = 3;

    if (apCfgForce100mGet())
        printf("Warning: Force LAN Port Speed to 100M is enabled, disable it first to set ethernet data rate here.\n");
    else {
        cliSetEthDataRate(index);
        printf("Ethernet Data Rate: %s\n", pEthDataRateStr[index]);
    }
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
    
    if(apCfgOpModeGet(pCli->unit) == CFG_OP_MODE_UC ||
        apCfgOpModeGet(pCli->unit) == CFG_OP_MODE_UR){
        printf("In \"Wireless Client/Repeater\" Mode, %s is not support!\n", pSecurityStr[index]);
        return CLI_PARSE_ERROR;
    }

    cliSetWirelessSecurity(pCli->unit, pCli->vap, index);
    printf("Wireless Seucrity Mode: %s\n", pSecurityStr[index]);
    return CLI_PARSE_OK;
}

int   
secCliCmdWpa2(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = 6;    
    if(apCfgOpModeGet(pCli->unit) == CFG_OP_MODE_UC ||
        apCfgOpModeGet(pCli->unit) == CFG_OP_MODE_UR){
        printf("In \"Wireless Client/Repeater\" Mode, %s is not support!\n", pSecurityStr[index]);
        return CLI_PARSE_ERROR;
    }
    cliSetWirelessSecurity(pCli->unit, pCli->vap, index);
    printf("Wireless Seucrity Mode: %s\n", pSecurityStr[index]);
    return CLI_PARSE_OK;
}

int   
secCliCmdWpaauto(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = 7;    
    if(apCfgOpModeGet(pCli->unit) == CFG_OP_MODE_UC ||
        apCfgOpModeGet(pCli->unit) == CFG_OP_MODE_UR){
        printf("In \"Wireless Client/Repeater\" Mode, %s is not support!\n", pSecurityStr[index]);
        return CLI_PARSE_ERROR;
    }
    cliSetWirelessSecurity(pCli->unit, pCli->vap, index);
    printf("Wireless Seucrity Mode: %s\n", pSecurityStr[index]);
    return CLI_PARSE_OK;
}
int   
secCliCmdRadius(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int index = 8;
 
    if(apCfgOpModeGet(pCli->unit) == CFG_OP_MODE_UC ||
        apCfgOpModeGet(pCli->unit) == CFG_OP_MODE_UR){
        printf("In \"Wireless Client/Repeater\" Mode, %s is not support!\n", pSecurityStr[index]);
        return CLI_PARSE_ERROR;
    }

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
encryptionCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Encryption Algorithm Type: %s\n", pEncryStr[apCfgWPACipherGet(pCli->unit, pCli->vap)]);
    return CLI_PARSE_OK;
}

int
encryCliCmdTkip(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    if(apCfgAuthTypeGet(pCli->unit, pCli->vap)==APCFG_AUTH_WPAPSK || apCfgAuthTypeGet(pCli->unit, pCli->vap)==APCFG_AUTH_WPA){
        apCfgWPACipherSet(pCli->unit, pCli->vap, WPA_CIPHER_TKIP);
        printf("Encryption Algorithm Type: TKIP\n");
        return CLI_PARSE_OK;
    }
    else
    {
        printf("This Authentication Type is not allow change Encryption Algorithm Type\n");
        return CLI_PARSE_ERROR;
    }
}

int
encryCliCmdAes(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    if(apCfgAuthTypeGet(pCli->unit, pCli->vap)==APCFG_AUTH_WPAPSK || apCfgAuthTypeGet(pCli->unit, pCli->vap)==APCFG_AUTH_WPA){
        apCfgWPACipherSet(pCli->unit, pCli->vap, WPA_CIPHER_AES);
        printf("Encryption Algorithm Type: AES\n");
        return CLI_PARSE_OK;
    }
    else
    {
        printf("This Authentication Type is not allow change Encryption Algorithm Type\n");
        return CLI_PARSE_ERROR;
    }
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
        if (sccli_tokenCount(pCli) == 0 && (v==40 || v==104/* || v == 128*/)) 
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
    printf("Usage: set keylength 40 | 104\n");
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
    int i = -1;
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
                while(++i < keyLen)
		{
		     if(('0'<=pStr[i] && pStr[i]<='9') || ('A'<=pStr[i] && pStr[i]<='F')
		            || ('a'<=pStr[i] && pStr[i]<='f'))
			 ;
		     else {
			 printf("value is not HEX\n");
			 return CLI_PARSE_ERROR;
		     }
		}
                if(keyLen == 10 || keyLen == 26)
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
groupKeyUpdateIntervalCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Group Key Update Interval: %d Seconds\n", apCfgGroupKeyUpdateIntervalGet(pCli->unit, pCli->vap));
    return CLI_PARSE_OK;
}

int
groupKeyUpdateIntervalCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    int v;

    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            rc = apCfgGroupKeyUpdateIntervalSet(pCli->unit, pCli->vap, v);
            if (rc != A_OK) {
                printf("Group Key Update Interval range invalid.\n");
                return CLI_PARSE_ERROR;
            }
            printf("Group Key Update Interval: %d seconds\n", apCfgGroupKeyUpdateIntervalGet(pCli->unit, pCli->vap));
            return CLI_PARSE_OK;
        }
    }
    return CLI_PARSE_ERROR;
}

int
AutoRebootIntervalCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Auto Reboot Interval: %d hours\n", apCfgAutoRebootIntervalGet());
    return CLI_PARSE_OK;
}

int
AutoRebootIntervalCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    A_STATUS rc;
    int v;

    if (sccli_tokenPopNum(pCli, &v, 10) != A_ERROR) {
        if (sccli_tokenCount(pCli) == 0) {
            rc = scApCfgAutoRebootIntervalSet(v);
            if (rc != A_OK) {
                printf("Auto Reboot Interval is invalid.\n");
                return CLI_PARSE_ERROR;
            }
            printf("Auto Reboot Interval: %d hours\n", apCfgAutoRebootIntervalGet());
            return CLI_PARSE_OK;
        }
    }
    return CLI_PARSE_ERROR;
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
	if(port<1 || port>65534)
	{
            printf("Invalid value! it should be 1..65534\n");
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
dot1xKeyLifeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("802.1x Dynamic Key Update Interval: %d Seconds\n", apCfgGroupKeyUpdateIntervalGet(pCli->unit ,pCli->vap));
    return CLI_PARSE_OK;

}

int   
dot1xKeyLifeCliCmdSet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    int interval;
    int rc;
    
    switch (sccli_tokenCount(pCli)) {
    case 0:
        return CLI_PARSE_NO_VALUE;
    case 1:
        if (sccli_tokenPopNum(pCli, &interval, 10) != CLI_PARSE_OK) {
            return CLI_PARSE_ERROR;
        }

        rc = apCfgGroupKeyUpdateIntervalSet(pCli->unit ,pCli->vap, interval);
        if (rc != A_OK) {
            printf("802.1x dynamic Key update interval range invalid\n");
            return CLI_PARSE_ERROR;
        }
        printf("802.1x Dynamic Key Update Interval: %d Seconds\n", apCfgGroupKeyUpdateIntervalGet(pCli->unit ,pCli->vap));
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
    printf("HTTPS: %s\n", pAbleStr[apCfgAutohttpsModeGet()]);
    return CLI_PARSE_OK;
}

int 
menhanceCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Multicast Packets Enhancement: %s\n", pAbleStr[apCfgMultiEnhanceGet()]);
    return CLI_PARSE_OK;
}
int 
hwversionCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    char hwv[32];

    getHwVersion(hwv);
    printf("PID VID: %s\n", hwv);
    return CLI_PARSE_OK;
}

int 
httpCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgHttpModeSet(0);
    printf("HTTP: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}

int 
httpCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgHttpModeSet(1);
    printf("HTTP: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}

int 
httpsCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgAutohttpsModeSet(0);
    apCfgHttpsModeSet(0);
    printf("HTTPS: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}

int 
httpsCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgAutohttpsModeSet(1);
    apCfgHttpsModeSet(1);
    printf("HTTPS: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}

int
multienhanceDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgMultiEnhanceSet(0);
    printf("Multicast Packets Enhancement: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}

int 
multienhanceEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgMultiEnhanceSet(1);
    printf("Multicast Packets Enhancement: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}
#if 0
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
#endif
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
daylightModeCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Daylight Mode: %s\n", pAbleStr[apCfgDaylightSavingGet()]);
    return CLI_PARSE_OK;
}

int
daylightCliCmdDisable(CLI *pCli, char *pToken, struct parse_token_s *pNextTbl)
{
    apCfgDaylightSavingSet(FALSE);
    printf("daylight: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;
}
int
daylightCliCmdEnable(CLI *pCli, char *pToken, struct parse_token_s *pNextTbl)
{
    apCfgDaylightSavingSet(TRUE);
    printf("daylight: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
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
    printf("SNMP: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;
}

int 
snmpCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgSnmpModeSet(FALSE);
    printf("SNMP: %s\n", pAbleStr[0]);
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
                long int end_ip;
        
                strcpy(netstr, inet_ntoa(iaddr)); 
                if (iaddr.s_addr) {
                    end_ip = apCfgSnmpManagerIpEndGet();
                    if(end_ip >= (long int)iaddr.s_addr){
                        apCfgSnmpManagerIpSet(iaddr.s_addr);
                        printf("SNMP Manager IP Address Start: %s\n", netstr);
                        return CLI_PARSE_OK;
                    }
                    else {
                        printf("Start IP is bigger than End IP\n");
                        return CLI_PARSE_ERROR;
		    }
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
                long int start_ip;
        
                strcpy(netstr, inet_ntoa(iaddr)); 
                if (iaddr.s_addr) {
                    start_ip = apCfgSnmpManagerIpGet();
                    if(start_ip <= (long int)iaddr.s_addr){
                        apCfgSnmpManagerIpEndSet(iaddr.s_addr);
                        printf("SNMP Manager IP Address End: %s\n", netstr);
                        return CLI_PARSE_OK;
                    }
                    else {
                        printf("Start IP is bigger than End IP\n");
                        return CLI_PARSE_ERROR;
		    }
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
syslogCliCmdGet(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    printf("Syslog: %s\n", pAbleStr[apCfgsysLogEnabledGet()]);
    return CLI_PARSE_OK;      
} 
   
int 
syslogCliCmdDisable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgsysLogEnabledSet(0);    
    printf("Syslog: %s\n", pAbleStr[0]);
    return CLI_PARSE_OK;       
}

int 
syslogCliCmdEnable(CLI *pCli, char *p, struct parse_token_s *pTbl)
{
    apCfgsysLogEnabledSet(1); 
    apCfgsysLogSeveritySet(6);
    printf("Syslog: %s\n", pAbleStr[1]);
    return CLI_PARSE_OK;       
}
#if 0
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
#endif
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
    else
        strcpy(macOrOui,tokenBuf);

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
    else
        strcpy(macOrOui,tokenBuf);
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

