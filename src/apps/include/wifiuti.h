/*
 * Copyright (C) 2005 SerComm Corporation. All Rights Reserved.
 *
 * SerComm Corporation reserves the right to make changes to this document
 * without notice. SerComm Corporation makes no warranty, representation
 * or guarantee regarding the suitability of its products for any
 * particular purpose. SerComm Corporation assumes no liability arising
 * out of the application or use of any product or circuit. SerComm
 * Corporation specifically disclaims any and all liability, including
 * without limitation consequential or incidental damages; neither does
 * it convey any license under its patent rights, nor the rights of
 * others.
 */
#ifndef _WIFI_UTI_H_
#define _WIFI_UTI_H_
void ATH_SCRIPT_DEL(char *scriptFile);
int ATH_SCRIPT_ADD(char *scriptFile, const char *format, ...);
void ATH_SCRIPT_RUN(char *scriptFile);
void wlanCreatWscConfig(int unit, int vap);
void wlanKillVap(char *scriptfile, int unit, int vap);
void wlanKillAllVap(char *scriptfile);
void wlanBasicApply(char *scriptfile, int unit, int vap, int, int);
void wlanAdvanceApply(char *scriptfile, int unit, int vap, int);
void wlanAclApply(char *scriptfile, int unit, int vap);
void wlanAclListApply(char *scriptfile);
void wlanSecurityApply(char *scriptfile, int unit, int vap);
void wlanNoApSecurityApply(char *scriptfile, int unit, int vap);
void wlanKillVap(char *scriptfile, int unit, int vap);
void wlanKillAllVap(char *scriptfile);
void wlanCorrectChannelOffsetApply(int unit);
int wlanModuleLoad(char *scriptfile, int unit);
int wlanModuleUnload(char *scriptfile, int unit);
int wlanStart(char *scriptfile, int unit);
int wlanStop(char *scriptfile, int unit, int unload_module/*1,0*/);
#endif
