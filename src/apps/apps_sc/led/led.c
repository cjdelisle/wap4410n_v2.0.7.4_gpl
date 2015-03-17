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

#define EXPORT_SYMTAB

#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
#define MODEVERSIONS
#endif

#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <asm/uaccess.h>
#include <asm/mach-ar7100/ar7100.h>

#include "led.h"
#include "gpio.h"

//#define DEBUG
#ifdef DEBUG
#define PRINTK(format,argument...) printk(format,##argument)
#else
#define PRINTK(format,argument...)
#endif

MODULE_LICENSE("GPL");

//////////////////////////////////////////////
void
ar7100_gpio_config_int(int gpio, 
                       ar7100_gpio_int_type_t type,
                       ar7100_gpio_int_pol_t polarity)
{
    /*
     * TODO allow edge sensitive/rising edge too
     */
    ar7100_reg_rmw_set(AR7100_GPIO_INT_ENABLE, (1 << gpio));
}

void
ar7100_gpio_config_input(int gpio)
{
    ar7100_reg_rmw_clear(AR7100_GPIO_OE, (1 << gpio));
}

void
ar7100_gpio_config_output(int gpio)
{
    ar7100_reg_rmw_set(AR7100_GPIO_OE, (1 << gpio));
}

void
ar7100_gpio_out_val(int gpio, int val)
{
    if (val & 0x1) {
        ar7100_reg_rmw_set(AR7100_GPIO_OUT, (1 << gpio));
    }
    else {
        ar7100_reg_rmw_clear(AR7100_GPIO_OUT, (1 << gpio));
    }
}

// cheneyli writed newly
int 
ar7100_gpio_out_add(int gpio)
{
    return( (1<<gpio) & (ar7100_reg_rd(AR7100_GPIO_OUT)));
}

int
ar7100_gpio_in_val(int gpio)
{
    return((1 << gpio) & (ar7100_reg_rd(AR7100_GPIO_IN)));
}

#if 0 /* DEL POE function */
int IsPoeOn(void)
{
    if( ar7100_gpio_in_val(GPIO_POE_DETECT) )   	
        return 1;
    else  	
        return 0;
}
#endif

static struct semaphore led_sem;
static spinlock_t       device_lock;
static ssize_t proc_read_led_fops(struct file *filp,
                                 char *buf,size_t count , loff_t *offp)
{
	char tmp[128];
	int len=0;
	
	if(*offp!=0)
		return 0;
#if 0		
	sprintf(tmp,"POE=%d", IsPoeOn());
#endif
    
	len=strlen(tmp)+1;
	copy_to_user(buf,tmp,len);
	*offp = len;
	
	return len;
}


static int led_atoi( char *name)
{
	int val = 0;
    for(;;name++)
	{
		switch(*name)
		{
			case '0'...'9':
				val = val*10+(*name - '0');
				break;
			default:
				return val;
		}
	}
}

static void led_on(unsigned long gpio,int logic)
{
    if(logic > 0)
        ar7100_gpio_out_val(gpio, 1);
    else
        ar7100_gpio_out_val(gpio, 0);    
}

static void led_off(unsigned long gpio,int logic)
{
    if(logic > 0)
        ar7100_gpio_out_val(gpio, 0);
    else
        ar7100_gpio_out_val(gpio, 1);   
}

static void led_init(unsigned long gpio,int init,int logic)
{
    ar7100_gpio_config_int(gpio, 0, 0);
    ar7100_gpio_config_output(gpio);
	if(init)
		led_on(gpio,logic);
	else
		led_off(gpio,logic);
}

static void led_reverse(unsigned long gpio)
{
	if( ar7100_gpio_out_add(gpio) )
    		ar7100_gpio_out_val(gpio, 0);	
	else
		ar7100_gpio_out_val(gpio, 1);
}


static struct timer_list led_slow_timer;
static struct timer_list led_fast_timer;
static struct timer_list led_wsc_timer;

static int led_fast_timer_running=0;
static int led_slow_timer_running=0;

static void led_blink_timer(unsigned long speed)
{
	struct led_data_s *led;
	int need_timer=0;

	for(led=led_data;led->gpio_pin!=GPIO_NULL;led++){
		if(led->count > 0 && led->speed==speed ){
			led_reverse(led->gpio_pin);
			led->count--;

			if(led->count>0)
				need_timer=1;
			else{
				/* last stat is off*/
				if(led->last_stat==0)
					led_off(led->gpio_pin,led->logic);
				/* last stat is on */
				else if(led->last_stat==1)
					led_on(led->gpio_pin,led->logic);
			}
		}
	}

	if(need_timer){
		if(speed==LED_FAST){
			led_fast_timer.data=speed;
        		led_fast_timer.expires=jiffies+FAST_INTERVAL;
	        	add_timer(&led_fast_timer);
		}
		else{
			led_slow_timer.data=speed;
        		led_slow_timer.expires=jiffies+SLOW_INTERVAL;
	        	add_timer(&led_slow_timer);
		}
	}
	else{
		if(speed==LED_FAST)
			led_fast_timer_running=0;
		else
			led_slow_timer_running=0;
	}
}

void led_blink(unsigned long gpio,int count ,int speed,int last_stat){

	struct led_data_s *led;
    
	for(led=led_data;led->gpio_pin!=GPIO_NULL;led++){
		if(led->gpio_pin == gpio ){			
			led->speed=speed;
			led->count=2*count;
			led->last_stat=last_stat;
		}
	}

	if(speed==LED_FAST && led_fast_timer_running==0){
		led_fast_timer_running=1;
		led_fast_timer.data=speed;
	        led_fast_timer.expires=jiffies+FAST_INTERVAL;
        	add_timer(&led_fast_timer);
	}
	else if(speed==LED_SLOW && led_slow_timer_running==0){
		led_slow_timer_running=1;
		led_slow_timer.data=speed;
	        led_slow_timer.expires=jiffies+SLOW_INTERVAL;
        	add_timer(&led_slow_timer);
	}
}

#ifndef WAP4410N
/*
* Led for wsc action
*/
typedef struct wsc_led_overrider_s{
    unsigned char type;             /*0: light; 1: flash*/
#define LED_OVERRIDER_FIX       0
#define LED_OVERRIDER_SWITCH    1
    
    unsigned int  on_time;
    unsigned int  off_time;
    unsigned int  cur_on_time;
    unsigned int  cur_off_time;
    unsigned char val;              /*0: 0ff; 1: on*/
#define LED_OVERRIDER_OFF       0
#define LED_OVERRIDER_ON        1
    
}WSC_LED_OVERRIDER; 

typedef struct wsc_led_controller_s{
    unsigned char type;             /*0: disable; 1: light; 2: flash*/
#define WSC_CONTROLLER_DISABLE  0
#define WSC_CONTROLLER_LIGHT    1 
#define WSC_CONTROLLER_FLASH    2

    unsigned char color;            /*1: yellow; 2: red; 3: green*/
#define WSC_LED_YELLOW          1
#define WSC_LED_RED             2
#define WSC_LED_GREEN           3
    
    unsigned int  on_time;
    unsigned int  off_time;
    unsigned int  cur_on_time;
    unsigned int  cur_off_time;
    WSC_LED_OVERRIDER overrider;
}WSC_LED_CONTROLLER;

static WSC_LED_CONTROLLER wsc_led_controller = {0,0,0,0,0,0,{0,0,0,0,0,1}};

 
//Led_WSC_Orange - turn on/off LED
//Input :       0 LED off, non-zero LED on
//Fsh_WSC_Orange - set LED to flash
//Input :       Rate to flash = 0x80 | pr_active_cyc, pr_active_cyc=1 to 0x7f
static void Led_WSC_Oragne(char dbValue)
{
    if(dbValue)
        led_on(GPIO_SES_AMBER,-1);
    else
        led_off(GPIO_SES_AMBER,-1);
}

static void Led_WSC_Green(char dbValue)
{
     if(dbValue)
        led_on(GPIO_SES_WHITE,-1);
    else
        led_off(GPIO_SES_WHITE,-1);
}

static void turnOffAllLed(void)
{
    Led_WSC_Oragne(0);
    Led_WSC_Green(0);
}    
static void successLed(void)
{
    turnOffAllLed();
    
    /*green on 300 seconds first , then off*/
    wsc_led_controller.type = WSC_CONTROLLER_FLASH;
    wsc_led_controller.color = WSC_LED_GREEN;
    wsc_led_controller.on_time = 3000; 
    wsc_led_controller.off_time = 0;
    wsc_led_controller.cur_on_time = 0;
    wsc_led_controller.cur_off_time = 0;
    
    /*fix override to 1*/
    wsc_led_controller.overrider.type = LED_OVERRIDER_FIX;
    wsc_led_controller.overrider.val = LED_OVERRIDER_ON;
}

static void inprogressLed(void)
{
    turnOffAllLed();
    
    wsc_led_controller.type = WSC_CONTROLLER_FLASH;
    wsc_led_controller.color = WSC_LED_YELLOW;
    wsc_led_controller.on_time = 2; 
    wsc_led_controller.off_time = 1;
    wsc_led_controller.cur_on_time = 0;
    wsc_led_controller.cur_off_time = 0;
    
    /*fix override to 1*/
    wsc_led_controller.overrider.type = LED_OVERRIDER_FIX;
    wsc_led_controller.overrider.val = LED_OVERRIDER_ON;

}

static void failedLed()
{
    
    turnOffAllLed();

    wsc_led_controller.type = WSC_CONTROLLER_FLASH;
    wsc_led_controller.color = WSC_LED_RED;
    wsc_led_controller.on_time = 1; 
    wsc_led_controller.off_time = 1;
    wsc_led_controller.cur_on_time = 0;
    wsc_led_controller.cur_off_time = 0;
    
    /*fix override to 1*/
    wsc_led_controller.overrider.type = LED_OVERRIDER_FIX;
    wsc_led_controller.overrider.val = LED_OVERRIDER_ON;

}
static void overlapLed()
{
    turnOffAllLed();
    
    wsc_led_controller.type = WSC_CONTROLLER_FLASH;
    wsc_led_controller.color = WSC_LED_RED;
    wsc_led_controller.on_time = 1; 
    wsc_led_controller.off_time = 1;
    wsc_led_controller.cur_on_time = 0;
    wsc_led_controller.cur_off_time = 0;
    
    /*fix override to 1*/
    wsc_led_controller.overrider.type = LED_OVERRIDER_SWITCH;
    wsc_led_controller.overrider.on_time = 10;
    wsc_led_controller.overrider.off_time = 5;
    wsc_led_controller.overrider.cur_on_time = 0;
    wsc_led_controller.overrider.cur_off_time = 0;
    wsc_led_controller.overrider.val = LED_OVERRIDER_ON;
    
}

static void wsc_button_pushed_led()
{
    turnOffAllLed();
    
    wsc_led_controller.type = WSC_CONTROLLER_FLASH;
    wsc_led_controller.color = WSC_LED_YELLOW;
    wsc_led_controller.on_time = 5; 
    wsc_led_controller.off_time = 0;
    wsc_led_controller.cur_on_time = 0;
    wsc_led_controller.cur_off_time = 0;
    
    /*fix override to 1*/
    wsc_led_controller.overrider.type = LED_OVERRIDER_FIX;
    wsc_led_controller.overrider.val = LED_OVERRIDER_ON;
}


static void wsc_led_on(unsigned char color)
{
    if(color == WSC_LED_YELLOW){
        Led_WSC_Oragne(wsc_led_controller.overrider.val&1);
    }else if(color == WSC_LED_RED){
        Led_WSC_Green(wsc_led_controller.overrider.val&1);
    }else if(color == WSC_LED_GREEN){
        Led_WSC_Oragne(wsc_led_controller.overrider.val&1);
        Led_WSC_Green(wsc_led_controller.overrider.val&1);
    }    
}    


static void wsc_led_off()
{
    turnOffAllLed();
}    
/*
*  Time_Poll will be triggered by interrupt every 0.1 second.
*/
void WSC_Time_Poll()
{
    unsigned int irqflags = 0x00;
    
	spin_lock_irqsave(&device_lock, irqflags);
	
    /*overrider */
    switch(wsc_led_controller.overrider.type)
    {
                    
        default:
            break;
            
        case LED_OVERRIDER_SWITCH:
            /*if not started, start it*/
            if(!wsc_led_controller.overrider.cur_on_time && !wsc_led_controller.overrider.cur_off_time){
                wsc_led_controller.overrider.cur_on_time = 1;
                wsc_led_controller.overrider.val = LED_OVERRIDER_ON;
                break;
            }    
        
            /*if overrider is on*/ 
            if(wsc_led_controller.overrider.cur_on_time){
                /*if time expired*/
                if(wsc_led_controller.overrider.cur_on_time >= wsc_led_controller.overrider.on_time){
                    wsc_led_controller.overrider.cur_on_time = 0;
                    wsc_led_controller.overrider.cur_off_time = 1;
                    wsc_led_controller.overrider.val = LED_OVERRIDER_OFF;
                    
                    /* if off time is unlimited, then disable controller*/
                    if(wsc_led_controller.overrider.off_time == 0)
                        wsc_led_controller.overrider.type = LED_OVERRIDER_FIX;
                        
                    break;
                }    
                wsc_led_controller.overrider.cur_on_time++;
                break;
            }else{
                /*else overrider is off*/
                /* if time expired*/
                if(wsc_led_controller.overrider.cur_off_time >= wsc_led_controller.overrider.off_time){
                    wsc_led_controller.overrider.cur_off_time = 0;
                    wsc_led_controller.overrider.cur_on_time = 1;
                    wsc_led_controller.overrider.val = LED_OVERRIDER_ON;
                    break;
                }    
                
                wsc_led_controller.overrider.cur_off_time++;
                break;
            }
            break;
    }  
    
    switch(wsc_led_controller.type)
    {   
        case WSC_CONTROLLER_FLASH:
            /*if not started, start it*/
            if(!wsc_led_controller.cur_on_time && !wsc_led_controller.cur_off_time){
                wsc_led_controller.cur_on_time = 1;
                wsc_led_on(wsc_led_controller.color);
                break;
            }
            /*if led is on*/ 
            if(wsc_led_controller.cur_on_time){
                /*if time expired*/
                if(wsc_led_controller.cur_on_time >= wsc_led_controller.on_time){
                    wsc_led_controller.cur_on_time = 0;
                    wsc_led_controller.cur_off_time = 1;
                    wsc_led_off();
                    
                    /* if off time is unlimited, then disable controller*/
                    if(wsc_led_controller.off_time == 0)
                        wsc_led_controller.type = WSC_CONTROLLER_DISABLE;
                        
                    break;
                }    
                wsc_led_controller.cur_on_time++;
                break;
            }else{
                /*else led is off*/
                /* if time expired*/
                if(wsc_led_controller.cur_off_time >= wsc_led_controller.off_time){
                    wsc_led_controller.cur_off_time = 0;
                    wsc_led_controller.cur_on_time = 1;
                    wsc_led_on(wsc_led_controller.color);
                    break;
                }    
                
                wsc_led_controller.cur_off_time++;
                break;
            }
            break;
        case WSC_CONTROLLER_LIGHT:
             /*if not started, start it*/
            if(!wsc_led_controller.cur_on_time){
                wsc_led_controller.cur_on_time = 1;
                wsc_led_on(wsc_led_controller.color);
                break;
            }
            /*if led is on*/ 
            if(wsc_led_controller.cur_on_time){
                /*if time expired*/
                if(wsc_led_controller.cur_on_time >= wsc_led_controller.on_time){
                    wsc_led_controller.cur_on_time = 0;
                    wsc_led_off();
                    wsc_led_controller.type = WSC_CONTROLLER_DISABLE;
                    break;
                }    
                wsc_led_controller.cur_on_time++;
                break;
            }
            break;    
        default:
            ;    
    }
    led_wsc_timer.expires=jiffies+FAST_INTERVAL;
    add_timer(&led_wsc_timer);
    spin_unlock_irqrestore(&device_lock, irqflags);
}
#endif

static ssize_t proc_write_led_fops(struct file *filp,const char *buffer,
                                     size_t count , loff_t *offp)
{
	char line[10];
	unsigned int irqflags = 0x00;

	copy_from_user(line,buffer,count);
	line[count]=0x00;

	PRINTK("<0>echo in==%s\n",line);

	if(count<1)
		return count;
	PRINTK("<0>echo count==%d\n",count);
	down_interruptible(&led_sem);

	spin_lock_irqsave(&device_lock, irqflags);
	PRINTK("<0>echo line[0]==%c\n",line[0]);	
	switch(line[0]){
		/* upgrade */
		case 'u':
			led_on(GPIO_POWER_GREEN,-1);
			led_blink(GPIO_POWER_GREEN,led_atoi(&line[1]),LED_SLOW,1);		
			break;
		/* boot */	
		case 'b':
			led_on(GPIO_POWER_GREEN,-1);
			mdelay(1000);
			led_blink(GPIO_POWER_GREEN,led_atoi(&line[1]),LED_SLOW,1);			
			break;
		/* reboot */
		case 'r':
			led_on(GPIO_POWER_GREEN,-1);
			led_blink(GPIO_POWER_GREEN,led_atoi(&line[1]),LED_SLOW,1);			
			break;
		/* wireless traffic*/	
		case 'w':
		    PRINTK("<0>echo line[1]==%c\n",line[1]);
		    if(line[1]=='0'){
		        led_off(GPIO_WIRELESS,-1);
		    }else{
		        led_blink(GPIO_WIRELESS, led_atoi(&line[1]), LED_FAST, 1);	
		    }	
			break;
		/*poe led*/
#if 0		
		case 'p':
			PRINTK("<0>echo line[1]==%c\n gpio=%d\n",line[1], GPIO_POE );
			ar7100_gpio_config_int(GPIO_POE, 0, 0);
            ar7100_gpio_config_output(GPIO_POE);
		    if(line[1]=='0'){
		        led_off(GPIO_POE, -1);
		    }else if(line[1]=='1'){
		        led_on(GPIO_POE, -1);
		    }	
			break;
#endif
		case 'm':
			mdelay(1000);			
			break;
#ifndef WAP4410N						
		/* WPS control */	
		case 's':
		    PRINTK("<0>echo line[1]==%c\n",line[1]);	
		    if(line[1]=='0'){
		        successLed();
		    }else if(line[1]=='1'){
		        inprogressLed();
		    }else if(line[1]=='2'){
		    	failedLed();
		    }else if(line[1]=='3'){
		        overlapLed();
		    }else if(line[1]=='4'){
		        wsc_button_pushed_led();
		    }else if(line[1]=='5'){
		        wsc_led_off();
		    }
			break;
#endif					
		/* prompt user to release button for the action of restore to default */
		case 't':
			led_blink(GPIO_POWER_GREEN, led_atoi(&line[1]), LED_SLOW, 0);	
			break;
		/* restore to default */
		case 'd':
			led_blink(GPIO_POWER_GREEN, led_atoi(&line[1]), LED_SLOW, 1);
			break;

#ifdef VOIP
		case 'i':
			if(line[1]=='1')
      	        led_on(GPIO_INTERNET,-1);
			else
				led_off(GPIO_INTERNET,-1);
			break;
		case 'l':
			if(line[1]=='1')
				led_on(GPIO_LIFELINE,1);
			else
				led_off(GPIO_LIFELINE,1);
			break;
#endif
	}

	spin_unlock_irqrestore(&device_lock, irqflags);

	up(&led_sem);

	return count;
}

static struct file_operations led_fops = {
        read: proc_read_led_fops,
        write: proc_write_led_fops,
};

static struct proc_dir_entry *led_file;

static int __init led_init_module(void)
{
	int i=0;
	
	PRINTK("init...\n");
	led_file=create_proc_entry("led",0666,&proc_root);
	led_file->owner = THIS_MODULE;
	led_file->proc_fops = &led_fops;

	for(i=0;led_data[i].logic!=0;i++){
		led_init(led_data[i].gpio_pin,led_data[i].init,led_data[i].logic);
	}

    /*Init POE detect gpio*/
#if 0
    ar7100_gpio_config_int(GPIO_POE_DETECT,0,1);
    ar7100_gpio_config_input(GPIO_POE_DETECT); 
#endif
	init_timer(&led_slow_timer);
	led_slow_timer.function=led_blink_timer;

	init_timer(&led_fast_timer);
	led_fast_timer.function=led_blink_timer;

#ifndef WAP4410N
    init_timer(&led_wsc_timer);
    led_wsc_timer.function=WSC_Time_Poll;
    led_wsc_timer.expires=jiffies+FAST_INTERVAL;
    add_timer(&led_wsc_timer);
#endif
    
	sema_init(&led_sem,1);

	return 0;
}
static void __exit led_cleanup_module(void)
{
	remove_proc_entry("led",&proc_root);
	del_timer(&led_fast_timer);
	del_timer(&led_slow_timer);
	del_timer(&led_wsc_timer);
	
	PRINTK("<0>cleanup...\n");
}
module_init(led_init_module);
module_exit(led_cleanup_module);

