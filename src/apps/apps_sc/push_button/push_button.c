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
#include <linux/netdevice.h>
#include <asm/uaccess.h>
#include <asm/mach-ar7100/ar7100.h>

#include "push_button.h"
#include "gpio.h"

//#define DEBUG
#ifdef DEBUG
#define PRINTK(format,argument...) printk(format,##argument)
#else
#define PRINTK(format,argument...)
#endif

MODULE_LICENSE("GPL");

static char MyBuf[32];

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

int
ar7100_gpio_in_val(int gpio)
{
    return((1 << gpio) & (ar7100_reg_rd(AR7100_GPIO_IN)));
}

static int buttonIsPushed(void)
{
/*
    if(*(unsigned int *)RSTBUT_DATA_REG &
            MY_BIT(RSTBUT_DATA_BIT))
*/
    if( ar7100_gpio_in_val(GPIO_RESET_BUTTON) )   	
        return 0;
    else  	
        return 1;
}


static ssize_t proc_read_push_button_fops(struct file *filp,
                                 char *buf,size_t count , loff_t *offp)
{
    int len=0;
    if(*offp!=0)
        return 0;

    *MyBuf = '\0';
    if(buttonIsPushed()) {
        strcat(MyBuf, "RST ");
    }
	len=strlen(MyBuf)+1;
	copy_to_user(buf,MyBuf,len);
	*offp = len;
	return len;
}



static ssize_t proc_write_push_button_fops(struct file *filp,const char *buffer,
                                     size_t count , loff_t *offp)
{
	return count;
}

struct file_operations push_button_fops = {
        read: proc_read_push_button_fops,
        write: proc_write_push_button_fops,
};

static struct proc_dir_entry *push_button_file;

static int __init push_button_file_init_module(void)
{
	PRINTK("init...\n");
	push_button_file=create_proc_entry("push_button",0666,&proc_root);
	push_button_file->owner = THIS_MODULE;
	push_button_file->proc_fops = &push_button_fops;

    ar7100_gpio_config_int(GPIO_RESET_BUTTON,0,1);
    ar7100_gpio_config_input(GPIO_RESET_BUTTON);   

	return 0;
}
static void __exit push_button_file_cleanup_module(void)
{
	remove_proc_entry("push_button",&proc_root);
	PRINTK("cleanup...\n");
}

module_init(push_button_file_init_module);
module_exit(push_button_file_cleanup_module);

