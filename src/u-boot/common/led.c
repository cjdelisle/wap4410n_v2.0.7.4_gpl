/************************************************************
 *				LED Control APIs							*
 ************************************************************/
#if 1
#include "sc_config.h"
#include "../cpu/mips/ar7100/ag7100.h"
#include "ar7100_soc.h"
#include "config.h"
#include "../include/asm/addrspace.h"
#include <command.h>


/* GPIO number define */
/* - all the gpio is LOW_ACTIVE*/
#ifdef _WAP4410N_
#define LED_WLAN 		0
#define LED_POWER		1
#define LED_COLD_START	7
#define LED_WARM_START	15

#define BOOT_RECORD		14	/* Input */
#define BOOT_CTL		12 	/* Output */
#define BUTTON_RST		21	/* input */
#endif

#ifdef _AP101NA_
#define LED_WLAN 		0
#define LED_POWER		1
#define LED_STATUS		3
#define LED_COLD_START	7
#define LED_WARM_START	15

#define BOOT_RECORD		14	/* Input */
#define BOOT_CTL		12 	/* Output */
#define BUTTON_RST		21	/* input */
#endif
/**********************/
/* Functions Protypes */
/**********************/
void gpio_init(void);

/*************************/
/* Functions Declaration */
/*************************/
void gpio_init(void)
{
	uint val = 0;
	
	printf("gpio_init called.\n");
	
	val = ar7100_reg_rd(AR7100_GPIO_OE);
	
#ifdef _WAP4410N_
	val |= ( (1<<LED_WLAN) 	  	|
			#if 0 /* already inited earlier */
			 (1<<LED_POWER)		|
			#endif
			 (1<<LED_COLD_START)|
			 (1<<LED_WARM_START)|
			 (1<<BOOT_CTL));

	val &= ~((1<<BUTTON_RST)	|
			 (1<<BOOT_RECORD));

#endif
#ifdef _AP101NA_
	val |= ( (1<<LED_WLAN) 	  	|
			#if 0 /* already inited earlier */
			 (1<<LED_POWER)		|
			#endif
			 (1<<LED_COLD_START)|
			 (1<<LED_WARM_START)|
			 (1<<LED_STATUS)	|
			 (1<<BOOT_CTL));

	val &= ~((1<<BUTTON_RST)	|
			 (1<<BOOT_RECORD));
#endif
			  
	ar7100_reg_wr(AR7100_GPIO_OE, val);
	
	val = ar7100_reg_rd(AR7100_GPIO_OE);
	
}

int rst_button_pushed(void)
{
	uint val = 0;
	
	val = ar7100_reg_rd(AR7100_GPIO_IN);
		
	if(val & (1<<BUTTON_RST))
		return 0;
	else
		return 1;
}

int cold_start(void)
{
	uint val = 0;
	
	val = ar7100_reg_rd(AR7100_GPIO_IN);
		
	if(val & (1<<BOOT_RECORD))
		return 0; /* warm start */
	else
		return 1; /* cold start */
}
void set_boot_ctl(int val)
{
	uint val_old = 0;
	
	val_old = ar7100_reg_rd(AR7100_GPIO_OUT);
	
	if(val)
		val_old |= (1<<BOOT_CTL);
	else
		val_old &= ~(1<<BOOT_CTL);
	
	ar7100_reg_wr(AR7100_GPIO_OUT, val_old);
}
int remove_cold_flag(void)
{
	udelay(500000);
	set_boot_ctl(0);
	udelay(500000);
	set_boot_ctl(1);
	udelay(500000);
}
void Led_Wlan(int val)
{
	uint val_old = 0;
	
	val_old = ar7100_reg_rd(AR7100_GPIO_OUT);
	
	if(val)
		val_old |= (1<<LED_WLAN);
	else
		val_old &= ~(1<<LED_WLAN);
	
	ar7100_reg_wr(AR7100_GPIO_OUT, val_old);
}

void Led_Power(int val)
{
	uint val_old = 0;
	
	val_old = ar7100_reg_rd(AR7100_GPIO_OUT);
	
	if(val)
		val_old |= (1<<LED_POWER);
	else
		val_old &= ~(1<<LED_POWER);
	
	ar7100_reg_wr(AR7100_GPIO_OUT, val_old);
}
#ifdef _AP101NA_
void Led_Status(int val)
{
	uint val_old = 0;
	
	val_old = ar7100_reg_rd(AR7100_GPIO_OUT);
	
	if(val)
		val_old |= (1<<LED_STATUS);
	else
		val_old &= ~(1<<LED_STATUS);
	
	ar7100_reg_wr(AR7100_GPIO_OUT, val_old);
}
#endif
void Led_Warm_Start(int val)
{
	uint val_old = 0;
	
	val_old = ar7100_reg_rd(AR7100_GPIO_OUT);
	
	if(val)
		val_old |= (1<<LED_WARM_START);
	else
		val_old &= ~(1<<LED_WARM_START);
	
	ar7100_reg_wr(AR7100_GPIO_OUT, val_old);
}
void Led_Cold_Start(int val)
{
	uint val_old = 0;
	
	val_old = ar7100_reg_rd(AR7100_GPIO_OUT);
	
	if(val)
		val_old |= (1<<LED_COLD_START);
	else
		val_old &= ~(1<<LED_COLD_START);
	
	ar7100_reg_wr(AR7100_GPIO_OUT, val_old);
}

void Led_all_off(void)
{
	Led_Power(LED_OFF);
	Led_Wlan(LED_OFF);
#ifdef _AP101NA_
	Led_Status(LED_OFF); 
#endif
	Led_Warm_Start(LED_OFF);
	Led_Cold_Start(LED_OFF);
}

void Led_all_on(void)
{
#ifdef _AP101NA_
	Led_Status(LED_ON); 
#endif
	Led_Power(LED_ON);
	Led_Wlan(LED_ON);
	Led_Warm_Start(LED_ON);
	Led_Cold_Start(LED_ON);
}
/*
void Led_Strength_1(int val)
{
}

void Led_Strength_2(int val)
{
}

void Led_Wps_up(int val)
{
}

void Led_Wps_low(int val)
{
}
*/
void delay_ns(int n)
{
    int i = 0;
    for(;i<10*n;i++)
        udelay(100000);
}
int do_leds(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	Led_all_off();
	while(1)
	{
		Led_Power(LED_ON);
		delay_ns(5);
		Led_Power(LED_OFF);
		delay_ns(5);
		
		Led_Wlan(LED_ON);
		delay_ns(5);
		Led_Wlan(LED_OFF);
		delay_ns(5);
		
		Led_Warm_Start(LED_ON);
		delay_ns(5);
		Led_Warm_Start(LED_OFF);
		delay_ns(5);
		
		Led_Cold_Start(LED_ON);
		delay_ns(5);
		Led_Cold_Start(LED_OFF);
		delay_ns(5);
	}	
}

int get_led_status(int led_num)
{
	int val;
	
	val = ar7100_reg_rd(AR7100_GPIO_OUT);
	
	if(val & (1<<led_num))
		return LED_OFF;
	else	
		return LED_ON;
}



#endif


int do_gpio_oe( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int gpio_num = 0;
	int if_oe	 = 0;
	
	uint val = 0;	
	val = ar7100_reg_rd(AR7100_GPIO_OE);	
	
	if (argc < 3) 
	{
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	
	gpio_num = simple_strtoul(argv[1], NULL, 16);
	if_oe = simple_strtoul(argv[2], NULL, 16);
	
	if(if_oe)
	{
		val |= (1<<gpio_num);
	}
	else
	{
		val &= ~(1<<gpio_num);
	}
	ar7100_reg_wr(AR7100_GPIO_OE, val);
	return 0;
}

int do_gpio_set( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int gpio_num = 0;
	int gpio_val = 0;
	uint val_old = 0;
	
	val_old = ar7100_reg_rd(AR7100_GPIO_OUT);	
	
	uint val_oe = 0;	
	val_oe = ar7100_reg_rd(AR7100_GPIO_OE);	
	
	if (argc < 3) 
	{
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	
	gpio_num = simple_strtoul(argv[1], NULL, 16);
	gpio_val = simple_strtoul(argv[2], NULL, 16);
	
	if(val_oe & (1<<gpio_num))
	{
		if(gpio_val)
			val_old |= (1<<gpio_num);
		else
			val_old &= ~(1<<gpio_num);		
		
		ar7100_reg_wr(AR7100_GPIO_OUT, val_old);		
	}
	else
	{
		printf("GPIO %d is INPUT - You can not set its value!\n", gpio_num);
	}
	
}


int do_gpio_get( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int gpio_num = 0;
		
	uint val_oe = 0;	
	val_oe = ar7100_reg_rd(AR7100_GPIO_OE);	
	
	if (argc < 2) 
	{
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	
	gpio_num = simple_strtoul(argv[1], NULL, 16);
	
	if(val_oe & (1<<gpio_num))
	{
		printf("GPIO %d is OUTPUT - You can not read its value!\n", gpio_num);
		return -1;
	}
	else
	{
		uint val = 0;
	
		val = ar7100_reg_rd(AR7100_GPIO_IN);
		
		if(val & (1<<gpio_num))
		{
			printf("	%d\n", 1);
			return 1;
		}
		else
		{
			printf("	%d\n", 0);
			return 0;
		}
	}	
}
