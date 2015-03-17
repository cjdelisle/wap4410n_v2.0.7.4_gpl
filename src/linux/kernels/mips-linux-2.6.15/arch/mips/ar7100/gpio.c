/*
 * Copyright (c) 2009, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 */

#ifndef EXPORT_SYMTAB
#define EXPORT_SYMTAB
#endif

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/init.h>
#include <linux/resource.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <asm/types.h>
#include <asm/irq.h>
#include <asm/delay.h>

#include "ar7100.h"
#define AR7100_FACTORY_RESET		0x89ABCDEF
static atomic_t ar7100_fr_status = ATOMIC_INIT(0);
static volatile int ar7100_fr_opened=0;
static wait_queue_head_t ar7100_fr_wq;

/*
 * GPIO interrupt stuff
 */
typedef enum {
    INT_TYPE_EDGE,
    INT_TYPE_LEVEL,
}ar7100_gpio_int_type_t;

typedef enum {
    INT_POL_ACTIVE_LOW,
    INT_POL_ACTIVE_HIGH,
}ar7100_gpio_int_pol_t;


/* 
** Simple Config stuff
*/

#if !defined(IRQ_NONE)
#define IRQ_NONE
#define IRQ_HANDLED
#endif /* !defined(IRQ_NONE) */

#ifdef CONFIG_AR9100
#define FACTORY_RESET_GPIO 21
#define JUMPSTART_GPIO    12
#else
#define FACTORY_RESET_GPIO 8
#define JUMPSTART_GPIO    3
#ifdef CONFIG_CUS100
#define JUMPSTART_GPIO    7
#endif
#endif
typedef irqreturn_t(*sc_callback_t)(int, void *, struct pt_regs *);

/*
** Multiple Simple Config callback support
** For multiple radio scenarios, we need to post the button push to
** all radios at the same time.  However, there is only 1 button, so
** we only have one set of GPIO callback pointers.
**
** Creating a structure that contains each callback, tagged with the
** name of the device registering the callback.  The unregister routine
** will need to determine which element to "unregister", so the device
** name will have to be passed to unregister also
*/

typedef struct {
char	*name;
sc_callback_t	registered_cb;
void			*cb_arg;
} Multi_Callback_t;

/*
** Specific instance of the callback structure
*/

static Multi_Callback_t	SCcallback[2] = {{NULL,NULL,NULL},{NULL,NULL,NULL}};

/*
** These are for the GPIO, so we don't create multiple instances
*/

static int ignore_pushbutton = 0;
static int ignore_resetbutton = 0;
static struct proc_dir_entry *simple_config_entry = NULL;
static struct proc_dir_entry *simulate_push_button_entry = NULL;
static struct proc_dir_entry *tricolor_led_entry = NULL;


void ar7100_gpio_config_int(int gpio, 
                       ar7100_gpio_int_type_t type,
                       ar7100_gpio_int_pol_t polarity)
{
    u32 val;

    /*
     * allow edge sensitive/rising edge too
     */
    if (type == INT_TYPE_LEVEL) {
        /* level sensitive */
        ar7100_reg_rmw_set(AR7100_GPIO_INT_TYPE, (1 << gpio));
    }
    else {
       /* edge triggered */
       val = ar7100_reg_rd(AR7100_GPIO_INT_TYPE);
       val &= ~(1 << gpio);
       ar7100_reg_wr(AR7100_GPIO_INT_TYPE, val);
    }

    if (polarity == INT_POL_ACTIVE_HIGH) {
        ar7100_reg_rmw_set (AR7100_GPIO_INT_POLARITY, (1 << gpio));
    }
    else {
       val = ar7100_reg_rd(AR7100_GPIO_INT_POLARITY);
       val &= ~(1 << gpio);
       ar7100_reg_wr(AR7100_GPIO_INT_POLARITY, val);
    }

    ar7100_reg_rmw_set(AR7100_GPIO_INT_ENABLE, (1 << gpio));
}

void
ar7100_gpio_config_output(int gpio)
{
    ar7100_reg_rmw_set(AR7100_GPIO_OE, (1 << gpio));
}

void
ar7100_gpio_config_input(int gpio)
{
    ar7100_reg_rmw_clear(AR7100_GPIO_OE, (1 << gpio));
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

int
ar7100_gpio_in_val(int gpio)
{
    return((1 << gpio) & (ar7100_reg_rd(AR7100_GPIO_IN)));
}

static void
ar7100_gpio_intr_enable(unsigned int irq)
{
    ar7100_reg_rmw_set(AR7100_GPIO_INT_MASK, 
                      (1 << (irq - AR7100_GPIO_IRQ_BASE)));
}

static void
ar7100_gpio_intr_disable(unsigned int irq)
{
    ar7100_reg_rmw_clear(AR7100_GPIO_INT_MASK, 
                        (1 << (irq - AR7100_GPIO_IRQ_BASE)));
}

static unsigned int
ar7100_gpio_intr_startup(unsigned int irq)
{
	ar7100_gpio_intr_enable(irq);
	return 0;
}

static void
ar7100_gpio_intr_shutdown(unsigned int irq)
{
	ar7100_gpio_intr_disable(irq);
}

static void
ar7100_gpio_intr_ack(unsigned int irq)
{
	ar7100_gpio_intr_disable(irq);
}

static void
ar7100_gpio_intr_end(unsigned int irq)
{
	if (!(irq_desc[irq].status & (IRQ_DISABLED | IRQ_INPROGRESS)))
		ar7100_gpio_intr_enable(irq);
}

static void
ar7100_gpio_intr_set_affinity(unsigned int irq, unsigned long mask)
{
	/* 
     * Only 1 CPU; ignore affinity request
     */
}

struct hw_interrupt_type ar7100_gpio_intr_controller = {
	"AR7100 GPIO",
	ar7100_gpio_intr_startup,
	ar7100_gpio_intr_shutdown,
	ar7100_gpio_intr_enable,
	ar7100_gpio_intr_disable,
	ar7100_gpio_intr_ack,
	ar7100_gpio_intr_end,
	ar7100_gpio_intr_set_affinity,
};

void
ar7100_gpio_irq_init(int irq_base)
{
	int i;

	for (i = irq_base; i < irq_base + AR7100_GPIO_IRQ_COUNT; i++) {
		irq_desc[i].status  = IRQ_DISABLED;
		irq_desc[i].action  = NULL;
		irq_desc[i].depth   = 1;
		irq_desc[i].handler = &ar7100_gpio_intr_controller;
	}
}


void register_simple_config_callback (char *cbname, void *callback, void *arg)
{
	printk("!!!!! SC Callback Registration for %s\n",cbname);
	if(!SCcallback[0].name)
	{
		SCcallback[0].name = cbname;
		SCcallback[0].registered_cb = (sc_callback_t) callback;
		SCcallback[0].cb_arg = arg;
	}
	else
	{
		SCcallback[1].name = cbname;
		SCcallback[1].registered_cb = (sc_callback_t) callback;
		SCcallback[1].cb_arg = arg;
	}
}
EXPORT_SYMBOL(register_simple_config_callback);

void unregister_simple_config_callback (char *cbname)
{
	if(SCcallback[1].name == cbname)
	{
		SCcallback[1].name = NULL;
		SCcallback[1].registered_cb = NULL;
		SCcallback[1].cb_arg = NULL;
	}
	else if(SCcallback[0].name == cbname)
	{
		SCcallback[0].name = NULL;
		SCcallback[0].registered_cb = NULL;
		SCcallback[0].cb_arg = NULL;
	}
	else
	{
		printk("!&!&!&!& ERROR: Unknown callback name %s\n",cbname);
	}
}
EXPORT_SYMBOL(unregister_simple_config_callback);

/*
 * Irq for front panel SW jumpstart switch
 * Connected to XSCALE through GPIO4
 */
irqreturn_t jumpstart_irq(int cpl, void *dev_id, struct pt_regs *regs)
{
    if (ignore_pushbutton) {
        ar7100_gpio_config_int (JUMPSTART_GPIO,INT_TYPE_LEVEL,
                                INT_POL_ACTIVE_HIGH);
        ignore_pushbutton = 0;
        return IRQ_HANDLED;
    }

    ar7100_gpio_config_int (JUMPSTART_GPIO,INT_TYPE_LEVEL,INT_POL_ACTIVE_LOW);
    ignore_pushbutton = 1;

    if (SCcallback[0].registered_cb) {
        SCcallback[0].registered_cb (cpl, SCcallback[0].cb_arg, regs);
    }
    if (SCcallback[1].registered_cb) {
        SCcallback[1].registered_cb (cpl, SCcallback[1].cb_arg, regs);
    }

    return IRQ_HANDLED;
}

static int push_button_read (char *page, char **start, off_t off,
                               int count, int *eof, void *data)
{
    return 0;
}

static int push_button_write (struct file *file, const char *buf,
                                        unsigned long count, void *data)
{
    if (SCcallback[0].registered_cb) {
        SCcallback[0].registered_cb (0, SCcallback[0].cb_arg, 0);
    }
    if (SCcallback[1].registered_cb) {
        SCcallback[1].registered_cb (0, SCcallback[1].cb_arg, 0);
    }
    return count;
}

#define TRICOLOR_LED_GREEN_PIN  5         /* GPIO 5 */
#define TRICOLOR_LED_YELLOW_PIN 4         /* GPIO 4 */
#define OFF 0
#define ON 1

typedef enum {
        LED_STATE_OFF   =       0,
        LED_STATE_GREEN =       1,
        LED_STATE_YELLOW =      2,
        LED_STATE_ORANGE =      3,
        LED_STATE_MAX =         4
} led_state_e;

static led_state_e gpio_tricolorled = LED_STATE_OFF;

static int gpio_tricolor_led_read (char *page, char **start, off_t off,
               int count, int *eof, void *data)
{
    return sprintf (page, "%d\n", gpio_tricolorled);
}

static int gpio_tricolor_led_write (struct file *file, const char *buf,
                                        unsigned long count, void *data)
{
    u_int32_t val, green_led_onoff = 0, yellow_led_onoff = 0;

    if (sscanf(buf, "%d", &val) != 1)
        return -EINVAL;

    if (val >= LED_STATE_MAX)
        return -EINVAL;

    if (val == gpio_tricolorled)
        return count;

    switch (val) {
        case LED_STATE_OFF :
                green_led_onoff = OFF;   /* both LEDs OFF */
                yellow_led_onoff = OFF;
                break;

        case LED_STATE_GREEN:
                green_led_onoff = ON;    /* green ON, Yellow OFF */
                yellow_led_onoff = OFF;
                break;

        case LED_STATE_YELLOW:
                green_led_onoff = OFF;   /* green OFF, Yellow ON */
                yellow_led_onoff = ON;
                break;

        case LED_STATE_ORANGE:
                green_led_onoff = ON;    /* both LEDs ON */
                yellow_led_onoff = ON;
                break;
    }

    ar7100_gpio_out_val (TRICOLOR_LED_GREEN_PIN, green_led_onoff);
    ar7100_gpio_out_val (TRICOLOR_LED_YELLOW_PIN, yellow_led_onoff);
    gpio_tricolorled = val;

    return count;
}


static int create_simple_config_led_proc_entry (void)
{
    if (simple_config_entry != NULL) {
        printk ("Already have a proc entry for /proc/simple_config!\n");
        return -ENOENT;
    }

    simple_config_entry = proc_mkdir("simple_config", NULL);
    if (!simple_config_entry)
        return -ENOENT;

    simulate_push_button_entry = create_proc_entry ("push_button", 0644,
                                                      simple_config_entry);
    if (!simulate_push_button_entry)
        return -ENOENT;

    simulate_push_button_entry->write_proc = push_button_write;
    simulate_push_button_entry->read_proc = push_button_read;

    tricolor_led_entry = create_proc_entry ("tricolor_led", 0644,
                                            simple_config_entry);
    if (!tricolor_led_entry)
        return -ENOENT;

    tricolor_led_entry->write_proc = gpio_tricolor_led_write;
    tricolor_led_entry->read_proc = gpio_tricolor_led_read;

    /* configure gpio as outputs */
    ar7100_gpio_config_output (TRICOLOR_LED_GREEN_PIN); 
    ar7100_gpio_config_output (TRICOLOR_LED_YELLOW_PIN); 

    /* switch off the led */
    ar7100_gpio_out_val(TRICOLOR_LED_GREEN_PIN, OFF);
    ar7100_gpio_out_val(TRICOLOR_LED_YELLOW_PIN, OFF);

    return 0;
}


int __init pbXX_simple_config_init(void)
{
#ifdef CONFIG_CUS100
	u32 mask = 0;
#endif
    int req;

#ifdef CONFIG_CUS100
	mask = ar7100_reg_rd(AR7100_MISC_INT_MASK);
	ar7100_reg_wr(AR7100_MISC_INT_MASK, mask | (1 << 2)); /* Enable GPIO interrupt mask */
    ar7100_gpio_config_int (JUMPSTART_GPIO, INT_TYPE_LEVEL,INT_POL_ACTIVE_HIGH);
	ar7100_gpio_intr_enable(JUMPSTART_GPIO);
	ar7100_gpio_config_input(JUMPSTART_GPIO);
#else
    /* configure GPIO 3 as level triggered interrupt */ 
    ar7100_gpio_config_int (JUMPSTART_GPIO, INT_TYPE_LEVEL,INT_POL_ACTIVE_HIGH);
#endif

    req = request_irq (AR7100_GPIO_IRQn(JUMPSTART_GPIO), jumpstart_irq, 0,
                       "SW JUMPSTART", NULL);
    if (req != 0) {
        printk (KERN_ERR "unable to request IRQ for SWJUMPSTART GPIO (error %d)\n", req);
    }

    create_simple_config_led_proc_entry ();
    return 0;
}

subsys_initcall(pbXX_simple_config_init);
static int
ar7100fr_open(struct inode *inode, struct file *file)
{
	if (MINOR(inode->i_rdev) != FACTORY_RESET_MINOR) {
		return -ENODEV;
	}

	if (ar7100_fr_opened) {
		return -EBUSY;
	}

        ar7100_fr_opened = 1;
	return nonseekable_open(inode, file);
}

static int
ar7100fr_close(struct inode *inode, struct file *file)
{
	if (MINOR(inode->i_rdev) != FACTORY_RESET_MINOR) {
		return -ENODEV;
	}

	ar7100_fr_opened = 0;
	return 0;
}

static ssize_t
ar7100fr_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	return -ENOTSUPP;
}

static ssize_t
ar7100fr_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	return -ENOTSUPP;
}

static int
ar7100fr_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		unsigned long arg)
{
	int ret = 0;

	switch(cmd) {
		case AR7100_FACTORY_RESET:
            
			/* Userspace process "factoryreset" is doing IOCTL
 			 * Hope AP is successfully UP
 			 * Turn the power LED on
 			 */
#ifdef CONFIG_AR9100
#define AP8x_GPIO_POWER_LED  (0x01 << 14) 
 			ar7100_reg_wr(AR7100_GPIO_OE, AP8x_GPIO_POWER_LED);
			ar7100_reg_wr(AR7100_GPIO_OUT,
	          ((ar7100_reg_rd(AR7100_GPIO_OUT)) | ((AP8x_GPIO_POWER_LED))));
#endif

            atomic_inc(&ar7100_fr_status);
			sleep_on(&ar7100_fr_wq);
			break;

		default: ret = -EINVAL;
	}

	return ret;
}

irqreturn_t factory_reset_irq(int cpl, void *dev_id, struct pt_regs *regs)
{
	unsigned int delay;
	
	if (ignore_resetbutton) {
        ar7100_gpio_config_int (FACTORY_RESET_GPIO,INT_TYPE_LEVEL,
                                INT_POL_ACTIVE_LOW);
        ignore_resetbutton = 0;
        return IRQ_HANDLED;
    }

    ar7100_gpio_config_int (FACTORY_RESET_GPIO,INT_TYPE_LEVEL,INT_POL_ACTIVE_HIGH);
    ignore_resetbutton = 1;

    if (atomic_read(&ar7100_fr_status))	{

        local_irq_disable();

#define UDELAY_COUNT 4000

        for (delay = UDELAY_COUNT; delay; delay--) {
            if (ar7100_gpio_in_val(FACTORY_RESET_GPIO)) {
                break;
            }
            udelay(1000);
        }

        if (!delay) {

			/* 
			 * since we are going to reboot the board, we
			 * don't need the interrupt handler anymore,
			 * so disable it. 
			 */
			disable_irq(AR7100_GPIO_IRQn(FACTORY_RESET_GPIO));
			wake_up(&ar7100_fr_wq);
			printk("\nar7100: factory configuration restored..\n");
		}
	    local_irq_enable();
	}
    return IRQ_HANDLED;
}

static struct file_operations ar7100fr_fops = {
	read:	ar7100fr_read,
	write:	ar7100fr_write,
	ioctl:	ar7100fr_ioctl,
	open:	ar7100fr_open,
	release:ar7100fr_close
};

static struct miscdevice ar7100fr_miscdev = 
{ FACTORY_RESET_MINOR, "Factory reset", &ar7100fr_fops };


int __init ar7100_factory_reset_init(void)
{
#ifdef CONFIG_CUS100
	u32 mask = 0;
#endif
    int req, ret;
    ret = misc_register(&ar7100fr_miscdev);

    if (ret < 0) {
            printk("*** ar7100 misc_register failed %d *** \n", ret);
            return -1;
    }
#ifdef CONFIG_CUS100
	mask = ar7100_reg_rd(ar7100_MISC_INT_MASK);
	ar7100_reg_wr(ar7100_MISC_INT_MASK, mask | (1 << 2)); /* Enable GPIO interrupt mask */
    ar7100_gpio_config_int (FACTORY_RESET_GPIO, INT_TYPE_LEVEL,INT_POL_ACTIVE_HIGH);
	ar7100_gpio_intr_enable(FACTORY_RESET_GPIO);
	ar7100_gpio_config_input(FACTORY_RESET_GPIO);
#else
	ar7100_gpio_config_int (FACTORY_RESET_GPIO, INT_TYPE_LEVEL,INT_POL_ACTIVE_HIGH);
#endif
	req = request_irq (AR7100_GPIO_IRQn(FACTORY_RESET_GPIO), factory_reset_irq, 0,
                       "FACTORY RESET", NULL);
    if (req != 0) {
        printk (KERN_ERR "unable to request IRQ for FACTORY_RESET GPIO (error %d)\n", req);
        misc_deregister(&ar7100fr_miscdev);
        ar7100_gpio_intr_shutdown(AR7100_GPIO_IRQn(FACTORY_RESET_GPIO));
        return -1;
    }

	init_waitqueue_head(&ar7100_fr_wq);
    return 0;
}
/* 
 * used late_initcall so that misc_register will succeed 
 * otherwise, misc driver won't be in a initializated state
 * thereby resulting in misc_register api to fail.
 */
late_initcall(ar7100_factory_reset_init);
