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

/* 0.1 sec */
#define FAST_INTERVAL 10*HZ/100
/* 0.5 sec */
#define SLOW_INTERVAL 50*HZ/100

#define LED_FAST 1
#define LED_SLOW 0

/* all gpio were negative logic */
#define GPIO_WIRELESS           (0)
#define GPIO_POWER_GREEN     	(1)
#define GPIO_POE                (3)
#define GPIO_POE_DETECT         (4) /*not Led*/

#define GPIO_RESERVER_LED1     	(11)
#define GPIO_RESERVER_LED2     	(2)
#define GPIO_RESERVER_LED4     	(9)
#define GPIO_RESERVER_LED5     	(10)

#define GPIO_NULL               (255)

struct led_data_s{
	unsigned long gpio_pin;
	unsigned int count;
	int logic;  /* -1 negative +1 */
	int speed;  /* 0:slow 1:fast */
	int last_stat; /* last stat 0:off 1:on 2:previous*/
	int init; /* 0:off 1:on */
};
static struct led_data_s led_data[]={ 
    {GPIO_POE,	            0,-1,0,2,1},
	{GPIO_WIRELESS,	        0,-1,0,2,0},    
	{GPIO_POWER_GREEN,	    0,-1,0,2,1},
	{GPIO_NULL,0,0,0,0,0},
	{GPIO_RESERVER_LED1,	0,-1,0,0,0},
	{GPIO_RESERVER_LED2,	0,-1,0,0,0},
	{GPIO_NULL,0,0,0,0,0}
};

#endif
