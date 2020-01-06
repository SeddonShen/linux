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
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/platform_device.h>

#include <asm/prom.h>

#define U_Rx 0x0
#define U_Tx 0x4
#define U_STAT 0x8
#define U_CTRL 0xC

#ifndef BIT
#define BIT(n) (1 << (n))
#endif

#define SR_TX_FIFO_FULL BIT(3) /* transmit FIFO full */
#define SR_TX_FIFO_EMPTY BIT(2) /* transmit FIFO empty */
#define SR_RX_FIFO_VALID_DATA BIT(0) /* data in receive FIFO */
#define SR_RX_FIFO_FULL BIT(1) /* receive FIFO full */

#define ULITE_CONTROL_RST_TX 0x01
#define ULITE_CONTROL_RST_RX 0x02

#define CONFIG_DEBUG_UART_BASE ((void *)0xbfe50000)

struct uartlite {
  unsigned int rx_fifo;
  unsigned int tx_fifo;
  unsigned int status;
  unsigned int control;
};

const char *get_system_type(void)
{
  return "noop";
}

void __init plat_mem_setup(void)
{
  __dt_setup_arch(__dtb_start);
  // strlcpy(arcs_cmdline, boot_command_line, COMMAND_LINE_SIZE);
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
  return 0;
}

arch_initcall(plat_of_setup);
