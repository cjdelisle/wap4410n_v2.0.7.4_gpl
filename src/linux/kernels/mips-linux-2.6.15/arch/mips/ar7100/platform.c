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

#include <linux/config.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/resource.h>

#include <linux/console.h>
#include <asm/serial.h>

#include <linux/tty.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>

#include <asm/mach-ar7100/ar7100.h>

extern uint32_t ar7100_ahb_freq;

/* 
 * OHCI (USB full speed host controller) 
 */
static struct resource ar7100_usb_ohci_resources[] = {
	[0] = {
		.start		= AR7100_USB_OHCI_BASE,
		.end		= AR7100_USB_OHCI_BASE + AR7100_USB_WINDOW - 1,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= AR7100_MISC_IRQ_USB_OHCI,
		.end		= AR7100_MISC_IRQ_USB_OHCI,
		.flags		= IORESOURCE_IRQ,
	},
};

/* 
 * The dmamask must be set for OHCI to work 
 */
static u64 ohci_dmamask = ~(u32)0;

static struct platform_device ar7100_usb_ohci_device = {
	.name		= "ar7100-ohci",
	.id		    = 0,
	.dev = {
		.dma_mask		= &ohci_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(ar7100_usb_ohci_resources),
	.resource	= ar7100_usb_ohci_resources,
};

/* 
 * EHCI (USB full speed host controller) 
 */
static struct resource ar7100_usb_ehci_resources[] = {
	[0] = {
		.start		= AR7100_USB_EHCI_BASE,
		.end		= AR7100_USB_EHCI_BASE + AR7100_USB_WINDOW - 1,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= AR7100_CPU_IRQ_USB,
		.end		= AR7100_CPU_IRQ_USB,
		.flags		= IORESOURCE_IRQ,
	},
};

/* 
 * The dmamask must be set for EHCI to work 
 */
static u64 ehci_dmamask = ~(u32)0;

static struct platform_device ar7100_usb_ehci_device = {
	.name		= "ar7100-ehci",
	.id		    = 0,
	.dev = {
		.dma_mask		= &ehci_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(ar7100_usb_ehci_resources),
	.resource	= ar7100_usb_ehci_resources,
};

static struct resource ar7100_uart_resources[] = {
	{
		.start = AR7100_UART_BASE,
		.end = AR7100_UART_BASE+0x0fff,
		.flags = IORESOURCE_MEM,
	},
};

static struct plat_serial8250_port ar7100_uart_data[] = {
	{
                .membase        = (char *)KSEG1ADDR(AR7100_UART_BASE),
                .irq            = AR7100_MISC_IRQ_UART,
                .flags          = STD_COM_FLAGS,
                .iotype         = UPIO_MEM32,
                .regshift       = 2,
                .uartclk        = 0, /* ar7100_ahb_freq, */
	},
        { },
};

static struct platform_device ar7100_uart = {
	 .name               = "serial8250",
        .id                 = 0,
        .dev.platform_data  = ar7100_uart_data,
        .num_resources      = 1, 
        .resource           = ar7100_uart_resources

};

static struct platform_device *ar7100_platform_devices[] __initdata = {
	&ar7100_usb_ohci_device,
    &ar7100_usb_ehci_device,
    &ar7100_uart
};

int ar7100_platform_init(void)
{
        /* need to set clock appropriately */
        ar7100_uart_data[0].uartclk = ar7100_ahb_freq; 
	return platform_add_devices(ar7100_platform_devices, 
                                ARRAY_SIZE(ar7100_platform_devices));
}

arch_initcall(ar7100_platform_init);
