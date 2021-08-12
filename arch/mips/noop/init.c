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
#include <linux/serial_8250.h>//Seddon New add
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

#define CONFIG_DEBUG_UART_BASE ((void *)0xbfe40000)//For nscscc soc
#define EARLY_PRINT_UART_BASE  0xbfe40000
#define NOOP_HALT_ADDR ((void *)0xb0000000)

extern void (*_machine_halt)(void);

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
	// struct uartlite *regs = (struct uartlite *)CONFIG_DEBUG_UART_BASE;

	// while (readl(&regs->status) & SR_TX_FIFO_FULL)
	// 	;

	// writel(ch & 0xff, &regs->tx_fifo);
	while(!(readl((uint32_t*)(EARLY_PRINT_UART_BASE+0x5)) & 0x20)); //LSR
	writel(ch, (uint32_t*)(EARLY_PRINT_UART_BASE+0));

}

void noop_halt(void)
{
	writel(0, NOOP_HALT_ADDR);
}

void __init prom_init(void)
{
	writel(7, (uint32_t*)(EARLY_PRINT_UART_BASE+2)); //Turn on FIFO
	writel(0x80, (uint32_t*)(EARLY_PRINT_UART_BASE+3)); //DLAB=1
	writel(54, (uint32_t*)(EARLY_PRINT_UART_BASE+0)); //DLL=54=100000000/(16*115200)  TLL = 54
	writel(0, (uint32_t*)(EARLY_PRINT_UART_BASE+4)); //DLM=0
	writel(3, (uint32_t*)(EARLY_PRINT_UART_BASE+0x3)); //DLAB=0,8N1 Mode
	writel(3, (uint32_t*)(EARLY_PRINT_UART_BASE+4)); //MCR Modem控制寄存器
	writel(1, (uint32_t*)(EARLY_PRINT_UART_BASE+1)); //IER enable interrupt
	// int i;
	// int argc = fw_arg0;
	// char **argv = (char **)fw_arg1;
	// struct uartlite *regs = (struct uartlite *)CONFIG_DEBUG_UART_BASE;

	// for (i = 0; i < argc; i++) {
	// 	int argi_len = strlen(argv[i]);
	// 	int arcs_len = strlen(arcs_cmdline);
	// 	if (arcs_len + argi_len + 1 >= sizeof(arcs_cmdline))
	// 		break;

	// 	if (arcs_len > 0 && arcs_cmdline[arcs_len - 1] != ' ')
	// 		strcat(&arcs_cmdline[arcs_len], " ");

	// 	if (argi_len > 0)
	// 		strcat(&arcs_cmdline[arcs_len], argv[i]);
	// }

	// writel(0, &regs->control);
	// writel(ULITE_CONTROL_RST_RX | ULITE_CONTROL_RST_TX, &regs->control);
	// readl(&regs->control);

	// /* init machine_halt */
	// _machine_halt = noop_halt;
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
