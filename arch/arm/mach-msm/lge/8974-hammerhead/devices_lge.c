/*
 * arch/arm/mach-msm/lge/device_lge.c
 *
 * Copyright (C) 2012,2013 LGE, Inc
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/persistent_ram.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <asm/setup.h>
#include <asm/system_info.h>
#include <mach/board_lge.h>

#include <linux/of.h>
#include <linux/of_fdt.h>

#ifdef CONFIG_LGE_PM
#include <linux/qpnp/qpnp-adc.h>
#include <mach/board_lge.h>
#include <linux/power_supply.h>
#endif


#ifdef CONFIG_LGE_HANDLE_PANIC
#include <mach/lge_handle_panic.h>
#endif
#include <ram_console.h>

#ifdef CONFIG_ANDROID_RAM_CONSOLE
#define LGE_RAM_CONSOLE_SIZE (128 * SZ_1K * 2)
static char bootreason[128] = {0,};

int __init lge_boot_reason(char *s)
{
	int n;

	if (*s == '=')
		s++;
	n = snprintf(bootreason, sizeof(bootreason),
			"Boot info:\n"
			"Last boot reason: %s\n", s);
	bootreason[n] = '\0';
	return 1;
}
__setup("bootreason", lge_boot_reason);

struct ram_console_platform_data ram_console_pdata = {
	.bootinfo = bootreason,
};

static struct platform_device ram_console_device = {
	.name = "ram_console",
	.id = -1,
	.dev = {
		.platform_data = &ram_console_pdata,
	}
};
#endif

#ifdef CONFIG_PERSISTENT_TRACER
static struct platform_device persistent_trace_device = {
	.name = "persistent_trace",
	.id = -1,
};
#endif

#ifdef CONFIG_ANDROID_PERSISTENT_RAM
#define LGE_PERSISTENT_RAM_SIZE (SZ_1M)
static struct persistent_ram_descriptor pram_descs[] = {
#ifdef CONFIG_ANDROID_RAM_CONSOLE
	{
		.name = "ram_console",
		.size = LGE_RAM_CONSOLE_SIZE,
	},
#endif
#ifdef CONFIG_PERSISTENT_TRACER
	{
		.name = "persistent_trace",
		.size = LGE_RAM_CONSOLE_SIZE,
	},
#endif
};

static struct persistent_ram persist_ram = {
	.size = LGE_PERSISTENT_RAM_SIZE,
	.num_descs = ARRAY_SIZE(pram_descs),
	.descs = pram_descs,
};

void __init lge_add_persist_ram_devices(void)
{
	int ret;
	struct membank *bank;

	if (meminfo.nr_banks < 2) {
		pr_err("%s: not enough membank\n", __func__);
		return;
	}

	bank = &meminfo.bank[1];
	/* first 1MB is used by bootloader */
	persist_ram.start = bank->start + SZ_1M;

	pr_info("PERSIST RAM CONSOLE START ADDR : 0x%x\n",
			persist_ram.start);

	ret = persistent_ram_early_init(&persist_ram);
	if (ret)
		pr_err("%s: failed to initialize persistent ram\n", __func__);
}
#endif /* CONFIG_ANDROID_PERSISTENT_RAM */

void __init lge_reserve(void)
{
#if defined(CONFIG_ANDROID_PERSISTENT_RAM)
	lge_add_persist_ram_devices();
#endif
}

void __init lge_add_persistent_device(void)
{
#ifdef CONFIG_ANDROID_RAM_CONSOLE
	platform_device_register(&ram_console_device);
#ifdef CONFIG_LGE_HANDLE_PANIC
	/* write ram console addr to imem */
	lge_set_ram_console_addr(persist_ram.start,
			LGE_RAM_CONSOLE_SIZE);
#endif
#endif
#ifdef CONFIG_PERSISTENT_TRACER
	platform_device_register(&persistent_trace_device);
#endif

}

/* setting whether uart console is enalbed or disabled */
static unsigned int uart_console_mode = 1;  // Alway Off

unsigned int lge_get_uart_mode(void)
{
	return uart_console_mode;
}

void lge_set_uart_mode(unsigned int um)
{
	uart_console_mode = um;
}

static int __init lge_uart_mode(char *uart_mode)
{
	if (!strncmp("enable", uart_mode, 6)) {
		printk(KERN_INFO"UART CONSOLE : enable\n");
		lge_set_uart_mode((UART_MODE_ALWAYS_ON_BMSK | UART_MODE_EN_BMSK)
				& ~UART_MODE_ALWAYS_OFF_BMSK);
	} else {
		printk(KERN_INFO"UART CONSOLE : disable\n");
	}

	return 1;
}
__setup("uart_console=", lge_uart_mode);

/* get boot mode information from cmdline.
 * If any boot mode is not specified,
 * boot mode is normal type.
 */
static enum lge_boot_mode_type lge_boot_mode = LGE_BOOT_MODE_NORMAL;
int __init lge_boot_mode_init(char *s)
{
	if (!strcmp(s, "charger"))
		lge_boot_mode = LGE_BOOT_MODE_CHARGER;
	else if (!strcmp(s, "chargerlogo"))
		lge_boot_mode = LGE_BOOT_MODE_CHARGERLOGO;
	else if (!strcmp(s, "factory"))
		lge_boot_mode = LGE_BOOT_MODE_FACTORY;
	else if (!strcmp(s, "factory2"))
		lge_boot_mode = LGE_BOOT_MODE_FACTORY2;
	else if (!strcmp(s, "pifboot"))
		lge_boot_mode = LGE_BOOT_MODE_PIFBOOT;

	return 1;
}
__setup("androidboot.mode=", lge_boot_mode_init);

enum lge_boot_mode_type lge_get_boot_mode(void)
{
	return lge_boot_mode;
}

/* for board revision */
static hw_rev_type lge_bd_rev = HW_REV_B;

/* CAUTION: These strings are come from LK. */
char *rev_str[] = {"evb1", "evb2", "rev_a", "rev_b", "rev_c", "rev_d",
	"rev_e", "rev_f", "rev_g", "rev_h", "rev_10", "rev_11", "rev_12",
	"revserved"};

int __init board_revno_setup(char *rev_info)
{
	int i;

	for (i = 0; i < HW_REV_MAX; i++) {
		if (!strncmp(rev_info, rev_str[i], 6)) {
			lge_bd_rev = (hw_rev_type) i;
			/* it is defined externally in <asm/system_info.h> */
			system_rev = lge_bd_rev;
			break;
		}
	}

	printk(KERN_INFO "BOARD : LGE %s \n", rev_str[lge_bd_rev]);
	return 1;
}
__setup("lge.rev=", board_revno_setup);

hw_rev_type lge_get_board_revno(void)
{
    return lge_bd_rev;
}


/* BEGIN : janghyun.baek@lge.com 2012-12-26 For cable detection */
#ifdef CONFIG_LGE_PM
struct chg_cable_info_table {
	int threshhold;
	acc_cable_type type;
	unsigned ta_ma;
	unsigned usb_ma;
};


#ifdef CONFIG_LGE_QFPROM_INTERFACE
static struct platform_device qfprom_device = {
	.name = "lge-qfprom",
	.id = -1,
};

void __init lge_add_qfprom_devices(void)
{
	platform_device_register(&qfprom_device);
}
#endif
#define ADC_NO_INIT_CABLE   0
#define C_NO_INIT_TA_MA     0
#define C_NO_INIT_USB_MA    0
#define ADC_CABLE_NONE      1900000
#define C_NONE_TA_MA        700
#define C_NONE_USB_MA       500

#define MAX_CABLE_NUM		15
static bool cable_type_defined;
static struct chg_cable_info_table pm8941_acc_cable_type_data[MAX_CABLE_NUM];
#endif
/* END : janghyun.baek@lge.com 2012-12-26 */

/* BEGIN : janghyun.baek@lge.com 2012-12-26 For cable detection */
#ifdef CONFIG_LGE_PM
void get_cable_data_from_dt(void *of_node)
{
	int i;
	u32 cable_value[3];
	struct device_node *node_temp = (struct device_node *)of_node;
	const char *propname[MAX_CABLE_NUM] = {
		"lge,no-init-cable",
		"lge,cable-mhl-1k",
		"lge,cable-u-28p7k",
		"lge,cable-28p7k",
		"lge,cable-56k",
		"lge,cable-100k",
		"lge,cable-130k",
		"lge,cable-180k",
		"lge,cable-200k",
		"lge,cable-220k",
		"lge,cable-270k",
		"lge,cable-330k",
		"lge,cable-620k",
		"lge,cable-910k",
		"lge,cable-none"
	};
	if (cable_type_defined) {
		pr_info("Cable type is already defined\n");
		return;
	}

	for (i = 0 ; i < MAX_CABLE_NUM ; i++) {
		of_property_read_u32_array(node_temp, propname[i],
				cable_value, 3);
		pm8941_acc_cable_type_data[i].threshhold = cable_value[0];
		pm8941_acc_cable_type_data[i].type = i;
		pm8941_acc_cable_type_data[i].ta_ma = cable_value[1];
		pm8941_acc_cable_type_data[i].usb_ma = cable_value[2];
	}
	cable_type_defined = 1;
}

int lge_pm_get_cable_info(struct qpnp_vadc_chip *vadc,
		struct chg_cable_info *cable_info)
{
	char *type_str[] = {
		"NOT INIT", "MHL 1K", "U_28P7K", "28P7K", "56K",
		"100K", "130K", "180K", "200K", "220K",
		"270K", "330K", "620K", "910K", "OPEN"
	};

	struct qpnp_vadc_result result;
	struct chg_cable_info *info = cable_info;
	struct chg_cable_info_table *table;

	int table_size = ARRAY_SIZE(pm8941_acc_cable_type_data);
	int acc_read_value = 0;
	int i, rc;
	int count = 1;

	if (!info) {
		printk(KERN_ERR "%s : invalid info parameters\n",
				__func__);
		return -1;
	}

	if (!vadc) {
		printk(KERN_ERR "%s : invalid vadc parameters\n",
				__func__);
		return -1;
	}

	if (!cable_type_defined) {
		printk(KERN_ERR "%s : cable type is not defined yet.\n",
				__func__);
		return -1;
	}

	for (i = 0; i < count; i++) {
		rc = qpnp_vadc_read(vadc, LR_MUX10_USB_ID_LV, &result);
		if (rc < 0) {
			if (rc == -ETIMEDOUT) {
				/* reason: adc read timeout,
				 * assume it is open cable
				 */
				info->cable_type = CABLE_NONE;
				info->ta_ma = C_NONE_TA_MA;
				info->usb_ma = C_NONE_USB_MA;
			}

			printk(KERN_ERR "%s : adc read error - %d\n",
					__func__, rc);
			return rc;
		}

		acc_read_value = (int)result.physical;
		printk(KERN_ERR "%s : acc_read_value - %d\n",
				__func__, (int)result.physical);
		/* mdelay(10); */
	}

	info->cable_type = NO_INIT_CABLE;
	info->ta_ma = C_NO_INIT_TA_MA;
	info->usb_ma = C_NO_INIT_USB_MA;

	/* assume : adc value must be existed in ascending order */
	for (i = 0; i < table_size; i++) {
		table = &pm8941_acc_cable_type_data[i];

		if (acc_read_value <= table->threshhold) {
			info->cable_type = table->type;
			info->ta_ma = table->ta_ma;
			info->usb_ma = table->usb_ma;
			break;
		}
	}

	printk(KERN_ERR "\n\n[PM] Cable detected: %d(%s)(%d, %d)\n\n",
			acc_read_value, type_str[info->cable_type],
			info->ta_ma, info->usb_ma);
	return 0;
}

struct pseudo_batt_info_type pseudo_batt_info = {
	.mode = 0,
};

void pseudo_batt_set(struct pseudo_batt_info_type *info)
{
	struct power_supply *batt_psy;

	pr_err("pseudo_batt_set\n");

	batt_psy = power_supply_get_by_name("battery");

	if (!batt_psy) {
		pr_err("called before init\n");
		return;
	}

	pseudo_batt_info.mode = info->mode;
	pseudo_batt_info.id = info->id;
	pseudo_batt_info.therm = info->therm;
	pseudo_batt_info.temp = info->temp;
	pseudo_batt_info.volt = info->volt;
	pseudo_batt_info.capacity = info->capacity;
	pseudo_batt_info.charging = info->charging;

	power_supply_changed(batt_psy);
}

/* Belows are for using in interrupt context */
static struct chg_cable_info lge_cable_info;

acc_cable_type lge_pm_get_cable_type(void)
{
	return lge_cable_info.cable_type;
}

unsigned lge_pm_get_ta_current(void)
{
	return lge_cable_info.ta_ma;
}

unsigned lge_pm_get_usb_current(void)
{
	return lge_cable_info.usb_ma;
}

/* This must be invoked in process context */
void lge_pm_read_cable_info(struct qpnp_vadc_chip *vadc)
{
	lge_cable_info.cable_type = NO_INIT_CABLE;
	lge_cable_info.ta_ma = C_NO_INIT_TA_MA;
	lge_cable_info.usb_ma = C_NO_INIT_USB_MA;

	lge_pm_get_cable_info(vadc, &lge_cable_info);
}
#endif
/* END : janghyun.baek@lge.com 2012-12-26 For cable detection */

