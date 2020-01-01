/*
 * NOOP platform setup
 *
 * Copyright (C) 2016 Nanjing University
 * Author: Wierton <141242068@smail.nju.edu.cn>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#include <linux/of_fdt.h>
#include <linux/of_platform.h>
#include <linux/serial_8250.h>
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/usb/sl811.h>
#include <linux/usb/isp1362.h>

#include <asm/prom.h>


#define U_Rx     0x0
#define U_Tx     0x4
#define U_STAT   0x8
#define U_CTRL   0xC

#ifndef BIT
#define BIT(n) (1 << (n))
#endif

#define SR_TX_FIFO_FULL		BIT(3) /* transmit FIFO full */
#define SR_TX_FIFO_EMPTY	BIT(2) /* transmit FIFO empty */
#define SR_RX_FIFO_VALID_DATA	BIT(0) /* data in receive FIFO */
#define SR_RX_FIFO_FULL		BIT(1) /* receive FIFO full */

#define ULITE_CONTROL_RST_TX	0x01
#define ULITE_CONTROL_RST_RX	0x02

#define CONFIG_DEBUG_UART_BASE ((void *)0xbfe50000)

struct uartlite {
  unsigned int rx_fifo;
  unsigned int tx_fifo;
  unsigned int status;
  unsigned int control;
};

#if IS_ENABLED(CONFIG_USB_SL811_HCD) || IS_ENABLED(CONFIG_USB_ISP1362_HCD)
static struct resource sl811_hcd_resources[] = {
  {
	.start = 0x1c020000,
	.end = 0x1c020000,
	.flags = IORESOURCE_MEM,
  }, {
	.start = 0x1c020004,
	.end = 0x1c020004,
	.flags = IORESOURCE_MEM,
  }, {
#if IS_ENABLED(CONFIG_DE2I_CYCLONE4)
	.start = 8+30,
	.end = 8+30,
#else
	.start = 5,
	.end = 5,
#endif
	//#if IS_ENABLED(CONFIG_USB_SL811_HCD)
	.flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
	//#else
	//		.flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE,
	//#endif
  },
};
#endif

#if IS_ENABLED(CONFIG_USB_SL811_HCD)
#if defined(CONFIG_USB_SL811_BFIN_USE_VBUS)
void sl811_port_power(struct device *dev, int is_on)
{
  // gpio_request(CONFIG_USB_SL811_BFIN_GPIO_VBUS, "usb:SL811_VBUS");
  // gpio_direction_output(CONFIG_USB_SL811_BFIN_GPIO_VBUS, is_on);
}
#endif

static struct sl811_platform_data sl811_priv = {
  .potpg = 10,
  .power = 250,       /* == 500mA */
#if defined(CONFIG_USB_SL811_BFIN_USE_VBUS)
  .port_power = &sl811_port_power,
#endif
};

static struct platform_device sl811_hcd_device = {
  .name = "sl811-hcd",
  .id = 0,
  .dev = {
	.platform_data = &sl811_priv,
  },
  .num_resources = ARRAY_SIZE(sl811_hcd_resources),
  .resource = sl811_hcd_resources,
};
#endif //CONFIG_USB_SL811_HCD

#if IS_ENABLED(CONFIG_USB_ISP1362_HCD)
static struct isp1362_platform_data isp1362_priv = {
  .sel15Kres = 1,
  .clknotstop = 0,
  .oc_enable = 0,
  .int_act_high = 1,
  .int_edge_triggered = 0,
  .remote_wakeup_connected = 0,
  .no_power_switching = 1,
  .power_switching_mode = 0,
};

static struct platform_device isp1362_hcd_device = {
  .name = "isp1362-hcd",
  .id = 0,
  .dev = {
	.platform_data = &isp1362_priv,
  },
  .num_resources = ARRAY_SIZE(sl811_hcd_resources),
  .resource = sl811_hcd_resources,
};
#endif

const char *get_system_type(void)
{
  return "NOOP";
}

void __init plat_mem_setup(void)
{
  __dt_setup_arch(__dtb_start);
  strlcpy(arcs_cmdline, boot_command_line, COMMAND_LINE_SIZE);
}

void prom_putchar(char ch)
{
  struct uartlite *regs = (struct uartlite *)CONFIG_DEBUG_UART_BASE;

  while (readl(&regs->status) & SR_TX_FIFO_FULL)
	;

  writel(ch & 0xff, &regs->tx_fifo);
}

void __init prom_init(void)
{
  struct uartlite *regs = (struct uartlite *)CONFIG_DEBUG_UART_BASE;

  writel(0, &regs->control);
  writel(ULITE_CONTROL_RST_RX | ULITE_CONTROL_RST_TX, &regs->control);
  readl(&regs->control);
}

void __init prom_free_prom_memory(void)
{
}

void __init device_tree_init(void)
{
  if (!initial_boot_params)
	return;

  unflatten_and_copy_device_tree();
}

static int __init plat_of_setup(void)
{
  /*
  if (!of_have_populated_dt())
	panic("Device tree not present");

  if (of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL))
	panic("Failed to populate DT");

#ifdef CONFIG_USB_SL811_HCD
  platform_device_register(&sl811_hcd_device);
#endif
#if IS_ENABLED(CONFIG_USB_ISP1362_HCD)
  platform_device_register(&isp1362_hcd_device);
#endif
  */

  return 0;
}
arch_initcall(plat_of_setup);
