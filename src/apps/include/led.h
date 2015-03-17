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
#ifndef _LED_H_
#define _LED_H_

#define _BA8000PRO_11G_
//#define _TZ50_
/* wgt634 */
#ifdef _BA8000PRO_11G_
#define LED_LAN_RESET           1
#define LED_PUSH                42
#define LED_POWER               40
#define LED_STATUS              41
#define LED_WLAN                43
#endif

#ifdef _TZ50_
/* TZ50*/
#define LED_LAN_RESET           24
#define LED_PUSH                20
#define LED_POWER               29
#define LED_STATUS              1
#define LED_WLAN                14
#define LED_RF_RESET            26
#define LED_RF_STATUS           14
#define LED_SPIS                17
#endif


/* microseconds */
#define FLASH_TIMER_UPGRADE           400000

void initLED (int gpioPin);
void turnOnLED (int gpioPin);
void turnOffLED (int gpioPin);

void initPushButton (void);
int ButtonIsPushed(void);

#endif /* _LED_H_ */
