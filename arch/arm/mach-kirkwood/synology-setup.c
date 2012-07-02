/*
 * Synology Kirkwood NAS Board Setup
 *
 * Maintained by:  Ben Peddell <klightspeed@killerwolves.net>
 *
 * Copyright 2012      Ben Peddell
 * Copyright 2009-2010 Synology, Inc.  All rights reserved.
 * Copyright 2009-2010 KueiHuan.Chen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mtd/physmap.h>
#include <linux/spi/flash.h>
#include <linux/spi/spi.h>
#include <linux/i2c.h>
#include <linux/mv643xx_eth.h>
#include <linux/ata_platform.h>
#include <linux/gpio.h>
#include <linux/gpio_keys.h>
#include <linux/leds.h>
#include <linux/gpio-fan.h>
#include <linux/serial_reg.h>
#include <linux/console.h>
#include <linux/io.h>
#include <linux/moduleparam.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/kirkwood.h>
#include <mach/synology.h>
#include "common.h"
#include "mpp.h"

/*****************************************************************************
 * Ethernet
 ****************************************************************************/

static struct mv643xx_eth_platform_data synology_ge00_data = {
	.phy_addr	= MV643XX_ETH_PHY_ADDR(8),
};

static struct mv643xx_eth_platform_data synology_ge01_data = {
	.phy_addr	= MV643XX_ETH_PHY_ADDR(9),
};

/*****************************************************************************
 * SATA
 ****************************************************************************/

static struct mv_sata_platform_data synology_sata_data = {
	.n_ports	= 2,
};

/*****************************************************************************
 * 4MB SPI Flash on Boot Device
 ****************************************************************************/
static struct mtd_partition synology_partitions[] = {
	{
		.name   = "RedBoot",		/* u-boot		*/
		.offset = 0x00000000,
		.size   = 0x00080000,		/* 512KiB		*/
		.mask_flags = MTD_WRITEABLE,	/* force read-only      */
	},
	{
		.name   = "zImage",		/* linux kernel image	*/
		.offset = 0x00080000,
		.size   = 0x00200000,		/* 2048KiB		*/
	},
	{
		.name   = "rd.gz",		/* ramdisk image	*/
		.offset = 0x00280000,
		.size   = 0x00140000,		/* 1280KiB		*/
	},
	{
		.name   = "vendor",		/* vendor specific data */
		.offset = 0x003C0000,
		.size   = 0x00010000,		/* 64KiB                */
	},
	{
		.name   = "RedBoot config",	/* configs for u-boot   */
		.offset = 0x003D0000,
		.size   = 0x00020000,		/* 128KiB               */
	},
	{
		.name   = "FIS directory",	/* flash partition table*/
		.offset = 0x003F0000,
		.size   = 0x00010000,		/* 64KiB                */
	},
};

static const struct flash_platform_data synology_spi_flash = {
	.type		= "m25p32",
	.name		= "spi_flash",
	.parts		= synology_partitions,
	.nr_parts	= ARRAY_SIZE(synology_partitions),
};

static struct spi_board_info __initdata synology_spi_info[] = {
	{
		.modalias	= "m25p80",
		.platform_data	= &synology_spi_flash,
		.irq		= -1,
		.max_speed_hz	= 20000000,
		.bus_num	= 0,
		.chip_select	= 0,
	},
};

/*****************************************************************************
 * GPIO HDD LEDs
 ****************************************************************************/

static struct gpio_led synology_hdd_led_20[] = {
	{ .name = "hdd1:green",	.gpio = 20, .active_low = 1 },
	{ .name = "hdd1:amber", .gpio = 21, .active_low = 1 },
	{ .name = "hdd2:green", .gpio = 22, .active_low = 1 },
	{ .name = "hdd2:amber", .gpio = 23, .active_low = 1 },
	{ .name = "hdd3:green", .gpio = 24, .active_low = 1 },
	{ .name = "hdd3:amber", .gpio = 25, .active_low = 1 },
	{ .name = "hdd4:green", .gpio = 26, .active_low = 1 },
	{ .name = "hdd4:amber", .gpio = 27, .active_low = 1 },
};

#define HDD_LED_20_COUNT(x) ((x) * 2)

/*
static struct gpio_led synology_hdd_led_21[] = {
	{ .name = "hdd1:green", .gpio = 21, .active_low = 1 },
	{ .name = "hdd1:amber", .gpio = 23, .active_low = 1 },
	{ .name = "hdd2:green", .gpio = 20, .active_low = 1 },
	{ .name = "hdd2:amber", .gpio = 22, .active_low = 1 },
};

#define HDD_LED_21_COUNT(x) (x)
*/

static struct gpio_led synology_hdd_led_36[] = {
	{ .name = "hdd1:green",	.gpio = 36, .active_low = 1 },
	{ .name = "hdd1:amber", .gpio = 37, .active_low = 1 },
	{ .name = "hdd2:green", .gpio = 38, .active_low = 1 },
	{ .name = "hdd2:amber", .gpio = 39, .active_low = 1 },
	{ .name = "hdd3:green", .gpio = 40, .active_low = 1 },
	{ .name = "hdd3:amber", .gpio = 41, .active_low = 1 },
	{ .name = "hdd4:green", .gpio = 42, .active_low = 1 },
	{ .name = "hdd4:amber", .gpio = 43, .active_low = 1 },
	{ .name = "hdd5:green", .gpio = 44, .active_low = 1 },
	{ .name = "hdd5:amber", .gpio = 45, .active_low = 1 },
};

#define HDD_LED_36_COUNT(x) ((x) * 2)

static struct gpio_led synology_hdd_led_38[] = {
	{ .name = "hdd1:green", .gpio = 38, .active_low = 1 },
	{ .name = "hdd1:amber", .gpio = 39, .active_low = 1 },
	{ .name = "hdd2:green", .gpio = 36, .active_low = 1 },
	{ .name = "hdd2:amber", .gpio = 37, .active_low = 1 },
};

#define HDD_LED_38_COUNT(x) ((x) * 2)

#define synology_hdd_led_NO NULL
#define HDD_LED_NO_COUNT(x) 0

#define HDD_LED(x) synology_hdd_led_ ## x
#define HDD_LED_COUNT(x,y) HDD_LED_ ## x ## _COUNT(y)

static struct gpio_led_platform_data synology_hdd_leds_data;

static struct platform_device synology_hdd_leds = {
	.name		= "leds-gpio",
	.id		= 0,
	.dev		= {
		.platform_data = &synology_hdd_leds_data,
	},
};

/*****************************************************************************
 * GPIO Alarm LED
 ****************************************************************************/

static struct gpio_led synology_alarm_led_pin_12[] = {
	{ .name = "alarm", .gpio = 12, .active_low = 1 },
};

static struct gpio_led_platform_data synology_alarm_led_data_12 = {
	.leds = synology_alarm_led_pin_12,
	.num_leds = 1,
};

static struct platform_device synology_alarm_led_12 = {
	.name		= "leds-gpio",
	.id		= 1,
	.dev		= {
		.platform_data = &synology_alarm_led_data_12,
	},
};

#define synology_alarm_leds_12 (&synology_alarm_led_12)
#define synology_alarm_leds_NO NULL

#define ALARM_LED(x) synology_alarm_leds_ ## x

/*****************************************************************************
 * GPIO fans
 ****************************************************************************/

static struct gpio_fan_speed synology_fan_speed_100[] = {
	{    0,  0 }, // xxx | xxx | xxx = xxx   0.0V
	{ 2500,  1 }, // 150 | xxx | xxx = 150   4.7V
	{ 3100,  2 }, // xxx | 100 | xxx = 100   5.8V
	{ 3800,  3 }, // 150 | 100 | xxx =  60   7.2V
	{ 4600,	 4 }, // xxx | xxx |  33 =  33   8.7V
	{ 4800,	 5 }, // 150 | xxx |  33 =  27   9.1V
	{ 4900,	 6 }, // xxx | 100 |  33 =  25   9.3V
	{ 5000,	 7 }, // 150 | 100 |  33 =  21   9.6V
};

static struct gpio_fan_speed synology_fan_speed_120[] = {
	{    0,  0 }, // xxx | xxx | xxx = xxx   0.0V
	{ 2500,  1 }, // 150 | xxx | xxx = 150   4.7V
	{ 2700,  2 }, // xxx | 120 | xxx = 120   5.3V
	{ 3000,	 4 }, // xxx | xxx | 100 = 100   5.8V
	{ 3600,  3 }, // 150 | 120 | xxx =  67   6.9V
	{ 3800,	 5 }, // 150 | xxx | 100 =  60   7.2V
	{ 3900,	 6 }, // xxx | 120 | 100 =  55   7.5V
	{ 4300,	 7 }, // 150 | 120 | 100 =  40   8.3V
};

static struct gpio_fan_speed synology_fan_speed_150[] = {
	{    0,  0 }, // xxx | xxx | xxx = xxx   0.0V
	{ 2200,  1 }, // 182 | xxx | xxx = 182   4.2V
	{ 2500,  2 }, // xxx | 150 | xxx = 150   4.7V
	{ 3000,	 4 }, // xxx | xxx | 100 = 100   5.8V
	{ 3300,  3 }, // 182 | 150 | xxx =  82   6.4V
	{ 3700,	 5 }, // 182 | xxx | 100 =  65   7.0V
	{ 3800,	 6 }, // xxx | 150 | 100 =  60   7.2V
	{ 4200,	 7 }, // 182 | 150 | 100 =  45   8.0V
};

#define FAN_SPEED(x) synology_fan_speed_ ## x

static unsigned synology_fan_ctrl_15[] = { 15, 16, 17 };
static unsigned synology_fan_ctrl_32[] = { 32, 33, 34 };

#define FAN_CTRL(x) synology_fan_ctrl_ ## x

static struct gpio_fan_alarm synology_fan_alarm_18[] = {
	{ .gpio = 18 },
};

static struct gpio_fan_alarm synology_fan_alarm_35[] = {
	{ .gpio = 35 },
	{ .gpio = 44 },
	{ .gpio = 45 },
};

#define FAN_ALARM(x) synology_fan_alarm_ ## x

static struct gpio_fan_platform_data synology_fan_data[3];

static struct platform_device synology_gpio_fan[3] = {
	{
		.name	= "gpio-fan",
		.id	= 0,
		.dev	= {
			.platform_data	= &synology_fan_data[0],
		},
	},
	{
		.name	= "gpio-fan",
		.id	= 1,
		.dev	= {
			.platform_data	= &synology_fan_data[1],
		},
	},
	{
		.name	= "gpio-fan",
		.id	= 2,
		.dev	= {
			.platform_data	= &synology_fan_data[2],
		},
	},
};

/*****************************************************************************
 * Synology specific power off and restart methods via UART1-attached PIC
 ****************************************************************************/

#define UART1_REG(x)	(UART1_VIRT_BASE + ((UART_##x) << 2))

static void synology_power_off(void){
	/* 9600 baud divisor */
	const unsigned divisor = ((kirkwood_tclk + (8 * 9600)) / (16 * 9600));

	pr_info("%s: triggering power-off...\n", __func__);

	/* hijack UART1 and reset into sane state (9600,8n1) */
	writel(0x83, UART1_REG(LCR));
	writel(divisor & 0xff, UART1_REG(DLL));
	writel((divisor >> 8) & 0xff, UART1_REG(DLM));
	writel(0x03, UART1_REG(LCR));
	writel(0x00, UART1_REG(IER));
	writel(0x00, UART1_REG(FCR));
	writel(0x00, UART1_REG(MCR));

	/* send the power-off command '1' to PIC */
	writel('1', UART1_REG(TX));

	while (1);
}

static void synology_restart(char mode, const char *cmd){
	/* 9600 baud divisor */
	const unsigned divisor = ((kirkwood_tclk + (8 * 9600)) / (16 * 9600));

	pr_info("%s: triggering restart...\n", __func__);

	/* hijack UART1 and reset into sane state (9600,8n1) */
	writel(0x83, UART1_REG(LCR));
	writel(divisor & 0xff, UART1_REG(DLL));
	writel((divisor >> 8) & 0xff, UART1_REG(DLM));
	writel(0x03, UART1_REG(LCR));
	writel(0x00, UART1_REG(IER));
	writel(0x00, UART1_REG(FCR));
	writel(0x00, UART1_REG(MCR));

	/* send the restart command 'C' to PIC */
	writel('C', UART1_REG(TX));

	while (1);
}


/*****************************************************************************
 * Ricoh RTC
 ****************************************************************************/

static struct i2c_board_info __initdata synology_ricoh_i2c_info[] = {
	{
		I2C_BOARD_INFO("rs5c372", 0x32),
	}
};

/*****************************************************************************
 * Seiko RTC
 ****************************************************************************/

static struct i2c_board_info __initdata synology_seiko_i2c_info[] = {
	{
		I2C_BOARD_INFO("s35390a", 0x30),
	}
};

/*****************************************************************************
 * Hard Disk power-up
 ****************************************************************************/

static unsigned synology_hdd_power_pin_29[] __initdata = { 29, 31 };
static unsigned synology_hdd_power_pin_30[] __initdata = { 30, 34, 44, 45 };

#define synology_hdd_power_pin_31 (synology_hdd_power_pin_29 + 1)
#define synology_hdd_power_pin_34 (synology_hdd_power_pin_30 + 1)
#define synology_hdd_power_pin_NO NULL

#define HDD_POWER(x) synology_hdd_power_pin_ ## x

static unsigned *synology_hdd_power_pin __initdata;
static unsigned synology_hdd_power_pin_count __initdata;

static void __init synology_hdd_powerup(void){
	int err;
	int i;
	int first = 1;
	for (i = 0; i < synology_hdd_power_pin_count; i++){
		int pin = synology_hdd_power_pin[i];
		if (!first && HDD_POWERUP_DELAY > 0){
			mdelay(HDD_POWERUP_DELAY);
		}
		err = gpio_request(pin, NULL);
		if (err == 0){
			err = gpio_direction_output(pin, 1);
			gpio_free(pin);
		}
		if (err){
			pr_err("%s: Failed to power up HDD%d\n", __func__, i + 1);
		} else {
			pr_info("%s: HDD%d powering up\n", __func__, i + 1);
			first = 0;
		}
	}
}

/*****************************************************************************
 * MPP Config
 ****************************************************************************/

static unsigned int synology_mpp_config[64] __initdata = { 0 };

struct synology_mpp_config_chain {
	struct synology_mpp_config_chain *parent;
	unsigned int config_data[];
};

static void __init synology_append_mpp_config(struct synology_mpp_config_chain *chain){
	int i;
	int j;
	if (chain->parent != NULL){
		synology_append_mpp_config(chain->parent);
	}

	for (i = 0; synology_mpp_config[i] != 0; i++);
	for (j = 0; (synology_mpp_config[i] = chain->config_data[j]) != 0; i++, j++);
}

static struct synology_mpp_config_chain synology_628x_mpp_config __initdata = {
	.parent = NULL,
	.config_data = {
		MPP0_SPI_SCn,
		MPP1_SPI_MOSI,
		MPP2_SPI_SCK,
		MPP3_SPI_MISO,
		MPP4_GPIO,		/* Interlock */
		MPP5_GPO,		/* Unused */		
		MPP6_SYSRST_OUTn,
		MPP7_GPO,		/* Unused */
		MPP8_TW0_SDA,
		MPP9_TW0_SCK,
		MPP10_UART0_TXD,
		MPP11_UART0_RXD,
		MPP12_GPO,		/* Alarm LED */
		MPP13_UART1_TXD,
		MPP14_UART1_RXD,
		MPP15_GPIO,		/* FAN 150 (409-410) / 150 (x11-) */
		MPP16_GPIO,		/* FAN 120 (409-410) / 100 (x11-) */
		MPP17_GPIO,		/* FAN 100 (409-410) /  33 (x11-) */
		MPP18_GPO,		/* FAN Sense (?) */
		MPP19_GPO,		/* Interlock (?) */
		/* MPP20-27 */		/* Gigabit Ethernet Interface 2 / HDD LEDs */
		MPP28_GPIO,		/* Model ID bit 0 */
		MPP29_GPIO,		/* Model ID bit 1 / HDD1 POWER */
		/* MPP30-33 */		/* Gigabit Ethernet Interface 2 / FAN / HDD POWER */
		/* MPP34-35 */		/* SATA Act LEDS / FAN */
		MPP36_GPIO,		/* HDD1 Green LED */
		MPP37_GPIO,		/* HDD1 Amber LED */
		MPP38_GPIO,		/* HDD2 Green LED */
		MPP39_GPIO,		/* HDD2 Amber LED */
		MPP40_GPIO,		/* HDD3 Green LED */
		MPP41_GPIO,		/* HDD3 Amber LED */
		MPP42_GPIO,		/* HDD4 Green LED */
		MPP43_GPIO,		/* HDD4 Amber LED */
		MPP44_GPIO,		/* HDD5 Green LED / HDD3 POWER / FAN2 SENSE */
		MPP45_GPIO,		/* HDD5 Amber LED / HDD4 POWER / FAN3 SENSE */
		MPP46_GPIO,		/* Buzzer Mute Ack / Model ID bit 2 */
		MPP47_GPIO,		/* Buzzer Mute Req / Model ID bit 3 */
		MPP48_GPIO,		/* RPS1 On */
		MPP49_GPIO,		/* RPS2 On */
		0
	}
};

static struct synology_mpp_config_chain synology_628x_2bay_mpp_config __initdata = {
	.parent = &synology_628x_mpp_config,
	.config_data = {
		/* 
		 * TODO: make this controllable by pinctl when the Orion platform
		 * gets pinctl.
		 */
		MPP20_SATA1_ACTn,	/* HDD2 Green LED */
		MPP21_SATA0_ACTn,	/* HDD1 Green LED */
		MPP22_SATA1_PRESENTn,	/* HDD2 Amber LED */
		MPP23_SATA0_PRESENTn,	/* HDD1 Amber LED */
		MPP24_GPIO,		/* Unused */
		MPP25_GPIO,		/* Unused */
		MPP26_GPIO,		/* Unused */
		MPP27_GPIO,		/* Unused */
		MPP30_GPIO,		/* Unused */
		MPP31_GPIO,		/* HDD2 POWER */
		MPP32_GPIO,		/* FAN 182 (x09-x10) / 150 (x11-) */
		MPP33_GPO,		/* FAN 150 (x09-x10) / 100 (x11-) */
		MPP34_GPIO,		/* FAN 100 (x09-x10) /  33 (x11-) */
		MPP35_GPIO,		/* FAN SENSE */
		0
	}
};

static struct synology_mpp_config_chain synology_6281_slim_mpp_config __initdata = {
	.parent = &synology_628x_mpp_config,
	.config_data = {
		MPP20_GPIO,		/* HDD1 Green LED */
		MPP21_GPIO,		/* HDD1 Amber LED */
		MPP22_GPIO,		/* HDD2 Green LED */
		MPP23_GPIO,		/* HDD2 Amber LED */
		MPP24_GPIO,		/* HDD3 Green LED */
		MPP25_GPIO,		/* HDD3 Amber LED */
		MPP26_GPIO,		/* HDD4 Green LED */
		MPP27_GPIO,		/* HDD4 Amber LED */
		MPP30_GPIO,		/* Unused */
		MPP31_GPIO,		/* BP Lock-Out */
		MPP32_GPIO,		/* FAN 150 */
		MPP33_GPO,		/* FAN 120 */
		MPP34_GPIO,		/* FAN 100 */
		MPP35_GPIO,		/* FAN SENSE */
		0
	}
};

static struct synology_mpp_config_chain synology_4bay_common_mpp_config __initdata = {
	.parent = &synology_628x_mpp_config,
	.config_data = {
		MPP20_GE1_TXD0,
		MPP21_GE1_TXD1,
		MPP22_GE1_TXD2,
		MPP23_GE1_TXD3,
		MPP24_GE1_RXD0,
		MPP25_GE1_RXD1,
		MPP26_GE1_RXD2,
		MPP27_GE1_RXD3,
		MPP30_GE1_RXCTL,
		MPP31_GE1_RXCLK,
		MPP32_GE1_TCLKOUT,
		MPP33_GE1_TXCTL,
		0
	}
};

static struct synology_mpp_config_chain synology_6281_4bay_mpp_config __initdata = {
	.parent = &synology_4bay_common_mpp_config,
	.config_data = {
		MPP34_SATA1_ACTn,
		MPP35_SATA0_ACTn,
		0
	}
};

static struct synology_mpp_config_chain synology_6282_4bay_mpp_config __initdata = {
	.parent = &synology_4bay_common_mpp_config,
	.config_data = {
		MPP34_GPIO,		/* HDD2 POWER / Mute Buzzer */
		MPP35_GPIO,		/* FAN1 SENSE */
		0
	}
};

#define M628x_2  synology_628x_2bay_mpp_config
#define M6281_4  synology_6281_4bay_mpp_config
#define M6281_S  synology_6281_slim_mpp_config
#define M6282_4  synology_6282_4bay_mpp_config

/*****************************************************************************
 * Board Configs
 ****************************************************************************/

struct syno_machine {
	char *name;
	struct synology_mpp_config_chain *mpp_config;
	struct mv643xx_eth_platform_data *eth1_data;

	struct i2c_board_info *rtc_info;
	unsigned rtc_info_size;

	struct gpio_fan_speed *fan_speed;
	unsigned fan_speed_count;

	unsigned *fan_ctrl_gpio;

	struct gpio_fan_alarm *fan_alarm;
	unsigned fan_alarm_count;

	struct gpio_led *hdd_leds;
	unsigned hdd_leds_count;

	unsigned *hdd_power_gpio;
	unsigned hdd_power_gpio_count;

	struct platform_device *alarm_led;

	unsigned modelid_gpio_count;
};

#define SYNO_MACHINE(NAME,MPP,RTC,ETH1,MODEL,NRFAN,FANSPD,FANCTRL,FANSENSE,HDDLED,NRHDD,HDDPWR,NRHDDPWR,ALARMLED) \
	{ \
		.name = NAME, \
		.mpp_config = &MPP, \
		.eth1_data = ETH1, \
		.rtc_info = RTC, \
		.rtc_info_size = ARRAY_SIZE(RTC), \
		.fan_speed = FAN_SPEED(FANSPD), \
		.fan_speed_count = ARRAY_SIZE(FAN_SPEED(FANSPD)), \
		.fan_ctrl_gpio = FAN_CTRL(FANCTRL), \
		.fan_alarm = FAN_ALARM(FANSENSE), \
		.fan_alarm_count = NRFAN, \
		.hdd_leds = HDD_LED(HDDLED), \
		.hdd_leds_count = HDD_LED_COUNT(HDDLED,NRHDD), \
		.hdd_power_gpio = HDD_POWER(HDDPWR), \
		.hdd_power_gpio_count = NRHDDPWR, \
		.alarm_led = ALARM_LED(ALARMLED), \
		.modelid_gpio_count = MODEL, \
	}

/* RTC Names */
#define RICOH   synology_ricoh_i2c_info
#define SEIKO   synology_seiko_i2c_info
/* ETH1 */
#define GE01    &synology_ge01_data

static struct syno_machine synology_machine_data[] __initdata = {
	/*           name         mpp      rtc    eth1  mdl  nr  fan fan fan hdd nr  hdd nr  alm */
	/*                        config                pin  fan spd ctl alm led hdd pwr pwr led */
	/* 1-bay 6281 */
	SYNO_MACHINE("DS109",     M628x_2, RICOH, NULL, 0,   1,  150, 32, 35, NO,  1, NO,  0, NO),
	SYNO_MACHINE("DS110jv10", M628x_2, SEIKO, NULL, 0,   1,  150, 32, 35, NO,  1, NO,  0, NO),
	SYNO_MACHINE("DS110jv20", M628x_2, RICOH, NULL, 0,   1,  150, 32, 35, NO,  1, NO,  0, NO),
	SYNO_MACHINE("DS110jv30", M628x_2, SEIKO, NULL, 0,   1,  150, 32, 35, NO,  1, NO,  0, NO),
	SYNO_MACHINE("DS110",     M628x_2, RICOH, NULL, 0,   1,  150, 32, 35, NO,  1, NO,  0, NO),

	/* 2-bay 6281 */
	SYNO_MACHINE("DS209",     M628x_2, RICOH, NULL, 0,   1,  150, 32, 35, NO,  2, 31,  1, NO),
	SYNO_MACHINE("DS210jv10", M628x_2, SEIKO, NULL, 0,   1,  150, 32, 35, NO,  2, 31,  1, NO),
	SYNO_MACHINE("DS210jv20", M628x_2, SEIKO, NULL, 0,   1,  150, 32, 35, NO,  2, 31,  1, NO),
	SYNO_MACHINE("DS210jv30", M628x_2, SEIKO, NULL, 0,   1,  150, 32, 35, NO,  2, 31,  1, NO),
	SYNO_MACHINE("DS211j",    M628x_2, SEIKO, NULL, 0,   1,  100, 32, 35, NO,  2, 31,  1, NO),
	SYNO_MACHINE("DS212jv10", M628x_2, SEIKO, NULL, 0,   1,  100, 32, 35, NO,  2, 29,  2, NO),
	SYNO_MACHINE("DS212jv20", M628x_2, SEIKO, NULL, 0,   1,  100, 32, 35, NO,  2, 29,  2, NO),

	/* 4-bay 6281 */
	SYNO_MACHINE("DS409slim", M6281_S, RICOH, NULL, 0,   1,  120, 32, 35, 20,  4, NO,  0, NO),
	SYNO_MACHINE("DS409",     M6281_4, RICOH, GE01, 2,   1,  120, 15, 18, 36,  5, NO,  0, 12),
	SYNO_MACHINE("RS409",     M6281_4, RICOH, GE01, 2,   1,  120, 15, 18, 36,  5, NO,  0, NO),
	SYNO_MACHINE("DS410j",    M6281_4, RICOH, GE01, 2,   1,  120, 15, 18, 36,  5, NO,  0, 12),
	SYNO_MACHINE("DS411j",    M6281_4, SEIKO, GE01, 2,   1,  100, 15, 35, 36,  5, NO,  0, 12),

	/* 1-bay 6282 */
	SYNO_MACHINE("DS111",     M628x_2, SEIKO, NULL, 4,   1,  100, 15, 35, NO,  1, NO,  0, NO),
	SYNO_MACHINE("DS112v10",  M628x_2, SEIKO, NULL, 4,   1,  100, 15, 35, NO,  1, 30,  1, NO),

	/* 2-bay 6282 */
	SYNO_MACHINE("DS211",     M628x_2, SEIKO, NULL, 4,   1,  100, 15, 35, NO,  2, 34,  1, NO),
	SYNO_MACHINE("DS211pv10", M628x_2, SEIKO, NULL, 4,   1,  100, 15, 35, NO,  2, 34,  1, NO),
	SYNO_MACHINE("DS211pv20", M628x_2, SEIKO, NULL, 4,   1,  100, 15, 35, NO,  2, 34,  1, NO),
	SYNO_MACHINE("DS212",     M628x_2, SEIKO, NULL, 4,   1,  100, 15, 35, NO,  2, 30,  2, NO),
	SYNO_MACHINE("DS212pv10", M628x_2, SEIKO, NULL, 4,   1,  100, 15, 35, NO,  2, 30,  2, NO),
	SYNO_MACHINE("DS212pv20", M628x_2, SEIKO, NULL, 4,   1,  100, 15, 35, NO,  2, 30,  2, NO),
	SYNO_MACHINE("RS212",     M6282_4, SEIKO, NULL, 4,   3,  100, 15, 35, 38,  2, 30,  2, NO),

	/* 4-bay 6282 */
	SYNO_MACHINE("DS411",     M6282_4, SEIKO, GE01, 4,   1,  100, 15, 35, 36,  4, 34,  3, NO),
	SYNO_MACHINE("DS411slim", M6282_4, SEIKO, GE01, 4,   1,  100, 15, 35, 36,  4, NO,  0, NO),
	SYNO_MACHINE("RS411",     M6282_4, SEIKO, GE01, 4,   3,  100, 15, 35, 36,  4, NO,  0, NO),
	SYNO_MACHINE("RS812",     M6282_4, SEIKO, GE01, 4,   3,  100, 15, 35, 36,  4, NO,  0, NO),
};

/*****************************************************************************
 * syno_hw_version parameter
 ****************************************************************************/

static struct syno_machine *synology_machine __initdata = NULL;
static char synology_machine_name[16];

static int __init synology_parse_hw_version(char *p){
	int i;

	strncpy (synology_machine_name, p, ARRAY_SIZE(synology_machine_name));
	synology_machine_name[ARRAY_SIZE(synology_machine_name)-1] = 0;

	for (i = 0; i < ARRAY_SIZE(synology_machine_data); i++){
		if (strnicmp(synology_machine_data[i].name, p, 16) == 0){
			synology_machine = &synology_machine_data[i];
			break;
		}
	}

	return 0;
}

early_param("syno_hw_version", synology_parse_hw_version);

/*****************************************************************************
 * Machine Init
 ****************************************************************************/

static void __init synology_init(void){
	/*
	 * Basic setup. Needs to be called early.
	 */
	kirkwood_init();

	kirkwood_spi_init();
	kirkwood_i2c_init();
	kirkwood_ehci_init();
	kirkwood_ge00_init(&synology_ge00_data);
	kirkwood_uart0_init();
	kirkwood_uart1_init();
	kirkwood_sata_init(&synology_sata_data);

	spi_register_board_info(synology_spi_info, ARRAY_SIZE(synology_spi_info));

	if (synology_machine != NULL){
		int i;
		synology_append_mpp_config(synology_machine->mpp_config);
		kirkwood_mpp_conf(synology_mpp_config);
		i2c_register_board_info(0, synology_machine->rtc_info, synology_machine->rtc_info_size);

		if (synology_machine->eth1_data != NULL){
			kirkwood_ge01_init(synology_machine->eth1_data);
		}
		
		if (synology_machine->hdd_leds != NULL && synology_machine->hdd_leds_count != 0){
			synology_hdd_leds_data.num_leds = synology_machine->hdd_leds_count;
			synology_hdd_leds_data.leds     = synology_machine->hdd_leds;
			platform_device_register(&synology_hdd_leds);
		}

		if (synology_machine->alarm_led != NULL){
			platform_device_register(synology_machine->alarm_led);
		}

		if (synology_machine->fan_ctrl_gpio != NULL){
			synology_fan_data[0].num_ctrl  = 3;
			synology_fan_data[0].ctrl      = synology_machine->fan_ctrl_gpio;
			synology_fan_data[0].num_speed = synology_machine->fan_speed_count;
			synology_fan_data[0].speed     = synology_machine->fan_speed;
		}

		for (i = 0; i < synology_machine->fan_alarm_count; i++){
			synology_fan_data[i].alarm = &synology_machine->fan_alarm[i];

			if (synology_fan_data[i].ctrl != NULL || synology_fan_data[i].alarm != NULL){
				platform_device_register(&synology_gpio_fan[i]);
			}
		}

		if (synology_machine->hdd_power_gpio != NULL){
			synology_hdd_power_pin       = synology_machine->hdd_power_gpio;
			synology_hdd_power_pin_count = synology_machine->hdd_power_gpio_count;

			synology_hdd_powerup();
		}

		pm_power_off = synology_power_off;
		arm_pm_restart = synology_restart;
	} else if (synology_machine_name[0] != 0){
		synology_append_mpp_config(&synology_628x_mpp_config);
		kirkwood_mpp_conf(synology_mpp_config);
		pr_err("%s: Unknown board \"%s\"\n", __func__, synology_machine_name);
	} else {
		pr_err("%s: syno_hw_version not set!\n", __func__);
	}

	panic_timeout = 10;
}

MACHINE_START(SYNOLOGY_6282, "Synology DiskStation / RackStation")
	.boot_params	= 0x100,
	.init_machine	= synology_init,
	.map_io		= kirkwood_map_io,
	.init_early	= kirkwood_init_early,
	.init_irq	= kirkwood_init_irq,
	.timer		= &kirkwood_timer,
MACHINE_END

