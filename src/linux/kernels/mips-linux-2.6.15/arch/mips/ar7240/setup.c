#include <linux/config.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/serial.h>
#include <linux/console.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/pci.h>

#include <asm/reboot.h>
#include <asm/io.h>
#include <asm/time.h>
#include <asm/pgtable.h>
#include <asm/processor.h>
#include <asm/reboot.h>
#include <asm/system.h>
#include <asm/serial.h>
#include <asm/traps.h>
#include <linux/serial_core.h>

#include "ar7240.h"

#ifdef CONFIG_AR7240_EMULATION
#define         AG7240_CONSOLE_BAUD (9600)
#else
#define         AG7240_CONSOLE_BAUD (115200)
#endif

uint32_t ar7240_cpu_freq = 0, ar7240_ahb_freq, ar7240_ddr_freq;

static int __init ar7240_init_ioc(void);
void Uart16550Init(void);
void serial_print(char *fmt, ...);
void writeserial(char *str,int count);
static void ar7240_sys_frequency(void);
u8 Uart16550GetPoll(void);
/* 
 * Export AHB freq value to be used by Ethernet MDIO.
 */
EXPORT_SYMBOL(ar7240_ahb_freq);

void
ar7240_restart(char *command)
{
    for(;;) {
        ar7240_reg_wr(AR7240_RESET, AR7240_RESET_FULL_CHIP);
    }
}

void
ar7240_halt(void)
{
        printk(KERN_NOTICE "\n** You can safely turn off the power\n");
        while (1);
}

void
ar7240_power_off(void)
{
        ar7240_halt();
}

const char 
*get_system_type(void)
{
    return "Atheros AR7240 (Python)";
}

EXPORT_SYMBOL(get_system_type);
/*
 * The bootloader musta set cpu_pll_config.
 * We extract the pll divider, multiply it by the base freq 40.
 * The cpu and ahb are divided off of that.
 */
//#define FB50 1
static void
ar7240_sys_frequency(void)
{

#ifdef CONFIG_AR7240_EMULATION
#ifdef FB50
    ar7240_cpu_freq = 66000000;
    ar7240_ddr_freq = 66000000;
    ar7240_ahb_freq = 33000000;
#else
#if 1
    ar7240_cpu_freq = 300000000;
    ar7240_ddr_freq = 300000000;
    ar7240_ahb_freq = 150000000;
#else
    ar7240_cpu_freq = 62500000;
    ar7240_ddr_freq = 62500000;
    ar7240_ahb_freq = 31250000;
#endif

#endif
    return;
#else
    uint32_t pll, pll_div, ahb_div, ddr_div, freq, ref_div;

    if (ar7240_cpu_freq)
        return;

    pll = ar7240_reg_rd(AR7240_PLL_CONFIG);

    pll_div  = ((pll >> PLL_DIV_SHIFT) & PLL_DIV_MASK);
    ref_div  = (pll >> REF_DIV_SHIFT) & REF_DIV_MASK;
    ddr_div  = ((pll >> DDR_DIV_SHIFT) & DDR_DIV_MASK) + 1;
    ahb_div  = (((pll >> AHB_DIV_SHIFT) & AHB_DIV_MASK) + 1)*2;

    freq     = pll_div * ref_div * 5000000;

    ar7240_cpu_freq = freq;
    ar7240_ddr_freq = freq/ddr_div;
    ar7240_ahb_freq = ar7240_cpu_freq/ahb_div;
#endif
}

void __init
serial_setup(void)
{
	struct uart_port p;

	memset(&p, 0, sizeof(p));

	p.flags     = STD_COM_FLAGS;
	p.iotype    = UPIO_MEM32;
	p.uartclk   = ar7240_ahb_freq;
	p.irq       = AR7240_MISC_IRQ_UART;
	p.regshift  = 2;
	p.membase   = (u8 *)KSEG1ADDR(AR7240_UART_BASE);

	if (early_serial_setup(&p) != 0)
		printk(KERN_ERR "early_serial_setup failed\n");
	
}

static void __init
ar7240_timer_init(void)
{
    mips_hpt_frequency =  ar7240_cpu_freq/2;
}

static void __init
ar7240_timer_setup(struct irqaction *irq)
{
    unsigned int count;

    setup_irq(AR7240_CPU_IRQ_TIMER, irq);
    /* 
     * to generate the first CPU timer interrupt
     */
    count = read_c0_count();
    write_c0_compare(count + 1000);
}

int 
ar7240_be_handler(struct pt_regs *regs, int is_fixup)
{
#if 0
    if (!is_fixup && (regs->cp0_cause & 4)) {
        /* Data bus error - print PA */
        printk("DBE physical address: %010Lx\n",
                __read_64bit_c0_register($26, 1));
    }
#endif
#ifdef CONFIG_PCI
    int error = 0, status, trouble = 0;
    error = ar7240_reg_rd(AR7240_PCI_ERROR) & 3;

    if (error) {
        printk("PCI error %d at PCI addr 0x%x\n", 
                error, ar7240_reg_rd(AR7240_PCI_ERROR_ADDRESS));
        ar7240_reg_wr(AR7240_PCI_ERROR, error);
#if !defined(CONFIG_PERICOM)
        ar7240_local_read_config(PCI_STATUS, 2, &status);
        printk("PCI status: %#x\n", status);
#endif
        trouble = 1;
    }

    error = 0;
    error = ar7240_reg_rd(AR7240_PCI_AHB_ERROR) & 1;

    if (error) {
        printk("AHB error at AHB address 0x%x\n", 
                  ar7240_reg_rd(AR7240_PCI_AHB_ERROR_ADDRESS));
        ar7240_reg_wr(AR7240_PCI_AHB_ERROR, error);
#if !defined(CONFIG_PERICOM)
        ar7240_local_read_config(PCI_STATUS, 2, &status);
        printk("PCI status: %#x\n", status);
#endif
        trouble = 1;
    }
#endif

    printk("ar7240 data bus error: cause %#x\n", read_c0_cause());
    return (is_fixup ? MIPS_BE_FIXUP : MIPS_BE_FATAL);
}



void __init plat_setup(void)
{

#if 1
    board_be_handler = ar7240_be_handler;
#endif
    board_time_init     =  ar7240_timer_init;
    board_timer_setup   =  ar7240_timer_setup;
    _machine_restart    =  ar7240_restart;
    _machine_halt       =  ar7240_halt;
    _machine_power_off  =  ar7240_power_off;


    /* 
    ** early_serial_setup seems to conflict with serial8250_register_port() 
    ** In order for console to work, we need to call register_console().
    ** We can call serial8250_register_port() directly or use
    ** platform_add_devices() function which eventually calls the 
    ** register_console(). AP71 takes this approach too. Only drawback
    ** is if system screws up before we register console, we won't see
    ** any msgs on the console.  System being stable now this should be
    ** a special case anyways. Just initialize Uart here.
    */ 

    Uart16550Init();
    serial_print("Booting AR7240(Python)...\n");

#if 0
    serial_setup();
#endif
}

/*
 * -------------------------------------------------
 * Early printk hack
 */
/* === CONFIG === */

#define		REG_OFFSET		4

/* === END OF CONFIG === */

/* register offset */
#define         OFS_RCV_BUFFER          (0*REG_OFFSET)
#define         OFS_TRANS_HOLD          (0*REG_OFFSET)
#define         OFS_SEND_BUFFER         (0*REG_OFFSET)
#define         OFS_INTR_ENABLE         (1*REG_OFFSET)
#define         OFS_INTR_ID             (2*REG_OFFSET)
#define         OFS_DATA_FORMAT         (3*REG_OFFSET)
#define         OFS_LINE_CONTROL        (3*REG_OFFSET)
#define         OFS_MODEM_CONTROL       (4*REG_OFFSET)
#define         OFS_RS232_OUTPUT        (4*REG_OFFSET)
#define         OFS_LINE_STATUS         (5*REG_OFFSET)
#define         OFS_MODEM_STATUS        (6*REG_OFFSET)
#define         OFS_RS232_INPUT         (6*REG_OFFSET)
#define         OFS_SCRATCH_PAD         (7*REG_OFFSET)

#define         OFS_DIVISOR_LSB         (0*REG_OFFSET)
#define         OFS_DIVISOR_MSB         (1*REG_OFFSET)

#define         UART16550_READ(y)   ar7240_reg_rd((AR7240_UART_BASE+y))
#define         UART16550_WRITE(x, z)  ar7240_reg_wr((AR7240_UART_BASE+x), z)

static int serial_inited = 0;

#define         MY_WRITE(y, z)  ((*((volatile u32*)(y))) = z)

void Uart16550Init()
{
    int freq, div;

    ar7240_sys_frequency();
    freq = ar7240_ahb_freq;

    MY_WRITE(0xb8040000, 0xcff);
    MY_WRITE(0xb8040008, 0x3b);
    /* Enable UART , SPI and Disable S26 UART */ 
    MY_WRITE(0xb8040028, (ar7240_reg_rd(0xb8040028) | 0x48002));

    MY_WRITE(0xb8040008, 0x2f);

    div = freq/(AG7240_CONSOLE_BAUD*16);

//    div = 0xCB;
        /* set DIAB bit */
    UART16550_WRITE(OFS_LINE_CONTROL, 0x80);
        
    /* set divisor */
    UART16550_WRITE(OFS_DIVISOR_LSB, (div & 0xff));
    UART16550_WRITE(OFS_DIVISOR_MSB, (div >> 8) & 0xff);

    /*UART16550_WRITE(OFS_DIVISOR_LSB, 0x61);
    UART16550_WRITE(OFS_DIVISOR_MSB, 0x03);*/

    /* clear DIAB bit*/ 
    UART16550_WRITE(OFS_LINE_CONTROL, 0x00);

    /* set data format */
    UART16550_WRITE(OFS_DATA_FORMAT, 0x3);

    UART16550_WRITE(OFS_INTR_ENABLE, 0);
}


u8 Uart16550GetPoll()
{
    while((UART16550_READ(OFS_LINE_STATUS) & 0x1) == 0);
    return UART16550_READ(OFS_RCV_BUFFER);
}

void Uart16550Put(u8 byte)
{
    if (!serial_inited) {
        serial_inited = 1;
        Uart16550Init();
    }
    while (((UART16550_READ(OFS_LINE_STATUS)) & 0x20) == 0x0);
    UART16550_WRITE(OFS_SEND_BUFFER, byte);
}

extern int vsprintf(char *buf, const char *fmt, va_list args);
static char sprint_buf[1024];

void
serial_print(char *fmt, ...)
{
        va_list args;
        int n;

        va_start(args, fmt);
        n = vsprintf(sprint_buf, fmt, args);
        va_end(args);
        writeserial(sprint_buf,n);
}

void writeserial(char *str,int count)
{
  int i;
  for(i = 0 ;i <= count ; i++)
	Uart16550Put(str[i]);

	Uart16550Put('\r');
  memset(str,'\0',1024);
  return;
}

#include <asm/uaccess.h>
#define M_PERFCTL_EVENT(event)          ((event) << 5)
unsigned int clocks_at_start;

void
start_cntrs(unsigned int event0, unsigned int event1)
{
    write_c0_perfcntr0(0x00000000);
    write_c0_perfcntr1(0x00000000);
    /*
     * go...
     */
    write_c0_perfctrl0(0x80000000|M_PERFCTL_EVENT(event0)|0xf);
    write_c0_perfctrl1(0x00000000|M_PERFCTL_EVENT(event1)|0xf);
}

void
stop_cntrs(void)
{
    write_c0_perfctrl0(0);
    write_c0_perfctrl1(0);
}

void
read_cntrs(unsigned int *c0, unsigned int *c1)
{
    *c0 = read_c0_perfcntr0();
    *c1 = read_c0_perfcntr1();
}

static int ar7240_ioc_open(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t
ar7240_ioc_read(struct file * file, char * buf, size_t count, loff_t *ppos)
{

    unsigned int c0, c1, ticks = (read_c0_count() - clocks_at_start);
    char str[256];
    unsigned int secs = ticks/mips_hpt_frequency;

    read_cntrs(&c0, &c1);
    stop_cntrs();
    sprintf(str, "%d secs (%#x) event0:%#x event1:%#x", secs, ticks, c0, c1);
    copy_to_user(buf, str, strlen(str));

    return (strlen(str));
}
#if 0
static void
ar7240_dcache_test(void)
{
    int i, j;
    unsigned char p;

    for(i = 0; i < 4; i++) {
        for(j = 0; j < (10*1024); j++) {
            p = *((unsigned char *)0x81000000 + j);
        }
    }
}
#endif
            
static ssize_t
ar7240_ioc_write(struct file * file, const char * buf, size_t count, loff_t *ppos)
{
    int event0, event1;

    sscanf(buf, "%d:%d", &event0, &event1);
    printk("\nevent0 %d event1 %d\n", event0, event1);

    clocks_at_start = read_c0_count();
    start_cntrs(event0, event1);

    return (count);
}

struct file_operations ar7240_ioc_fops = {
    open:   ar7240_ioc_open,
    read:   ar7240_ioc_read,
    write:  ar7240_ioc_write,
};

/*
 * General purpose ioctl i/f
 */
static int __init
ar7240_init_ioc()
{
    static int _mymajor;

    _mymajor = register_chrdev(77, "AR7240_GPIOC", 
                               &ar7240_ioc_fops);

    if (_mymajor < 0) {
        printk("Failed to register GPIOC\n");
        return _mymajor;
    }

    printk("AR7240 GPIOC major %d\n", _mymajor);
    return 0;
}

device_initcall(ar7240_init_ioc);

