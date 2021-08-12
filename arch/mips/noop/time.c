/*
 * Xilfpga clocksource/timer setup
 *
 * Copyright (C) 2015 Imagination Technologies
 * Author: Zubair Lutfullah Kakakhel <Zubair.Kakakhel@imgtec.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */
//本文件基本没有修改
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/clocksource.h>
#include <linux/of.h>

#include <asm/time.h>

void __init plat_time_init(void)
{
#ifdef CONFIG_NOOP_SOC_ON_BOARD
	struct device_node *np;
	struct clk *clk;

	of_clk_init(NULL);
	// clocksource_probe();
	//被欧先飞注释的上面
	np = of_get_cpu_node(0, NULL);
	if (!np) {
		pr_err("Failed to get CPU node\n");
		return;
	}

	clk = of_clk_get(np, 0);
	if (IS_ERR(clk)) {
		pr_err("Failed to get CPU clock: %ld\n", PTR_ERR(clk));
		return;
	}

	mips_hpt_frequency = clk_get_rate(clk);
	printk("mips_hpt_frequency: %d HZ\n", mips_hpt_frequency);
	//频率也是老哥改的
	clk_put(clk);
#endif
}
