// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Zheng Han <zh@onioniot.com>, Lazar Demin <lazar@onioniot.com>
 * Copyright (C) 2025 DENX Software Engineering GmbH, Philip Oberfichtner <pro@denx.de>
 */

#include <asm/io.h>
#include <env.h>
#include <init.h>
#include <mtd.h>

#define OMEGA2_REG(x)		(*((u32 *)(x)))
#define OMEGA2_SYSCTL_BASE	0xB0000000
#define OMEGA2_SYS_CNTL_BASE	(OMEGA2_SYSCTL_BASE)
#define OMEGA2_REG_PIODIR	(OMEGA2_SYSCTL_BASE + 0x600)

static int detect_rst(void)
{
	u32 val = OMEGA2_REG(0xb0000624); // Read GPIO 44 (reset button)

	return (val & 1 << 6) ? 1 : 0;
}

static void gpio_init(void)
{
	u32 val;

	printf("Initializing MT7688 GPIO system.\n");

	//set gpio2_mode - setting wled, and p0,p1,p2,p3,p4 LED pins to GPIO mode
	val = 0x555;
	OMEGA2_REG(OMEGA2_SYS_CNTL_BASE + 0x64) = val; // GPIO2_MODE register
	// GINT_FEDGE_1: setting "Enable falling edge triggered" for GPIOs 39-42
	OMEGA2_REG(0xb0000644) = 0x0f << 7;

	// set gpio_ctrl_1 register: set GPIO44 to output
	//gpio44 output gpio_ctrl_1 bit3=1
	val = OMEGA2_REG(OMEGA2_REG_PIODIR + 0x04);
	val |= 1 << 12;
	OMEGA2_REG(OMEGA2_REG_PIODIR + 0x04) = val;

	// set gpio1_mode register: set WDT_MODE to GPIO mode
	//set gpio1_mode 14=1b1
	val = OMEGA2_REG(OMEGA2_SYS_CNTL_BASE + 0x60);
	val |= 1 << 14;
	OMEGA2_REG(OMEGA2_SYS_CNTL_BASE + 0x60) = val;

	// set gpio_ctrl_1 resgister: set GPIO38 to input
	//gpio38 input gpio_ctrl_1 bit5=0
	val = OMEGA2_REG(OMEGA2_REG_PIODIR + 0x04);
	val &= ~1 << 6;
	OMEGA2_REG(OMEGA2_REG_PIODIR + 0x04) = val;
}

enum onion_board_variant {
	OMEGA2,
	OMEGA2P,
	UNKNOWN,
};

static enum onion_board_variant board_variant(void)
{
	struct mtd_info *mtd;

	mtd_probe_devices();

	mtd_for_each_device(mtd) {
		if (mtd->type != MTD_NORFLASH)
			continue;

		switch (mtd->size) {
		case 16 * 1024 * 1024: // 16 MB
			return OMEGA2;

		case 32 * 1024 * 1024: // 32 MB
			return OMEGA2P;

		default:
			break;
		}
	}

	return UNKNOWN;
}

#define ONION_MTDPARTS_BASE "spi0.0:192k(u-boot),64k(u-boot-env),64k(factory)"

static void set_mtdparts(void)
{
	switch (board_variant()) {
	case OMEGA2P:
		printf("Detected board variant OMEGA2+: ");
		env_set("mtdparts", ONION_MTDPARTS_BASE ",32448k(firmware)");
		break;

	case OMEGA2:
		printf("Detected board variant OMEGA2: ");
		env_set("mtdparts", ONION_MTDPARTS_BASE ",16064k(firmware)");
		break;

	default:
		printf("Unable to detect board variant! Using default value: ");
		env_set("mtdparts", ONION_MTDPARTS_BASE);
	}

	printf("mtdparts=\"%s\"\n", env_get("mtdparts"));
}

#define WELCOME_MESSAGE                                                     \
	"\n\n"                                                              \
	"   *************************************************************\n"\
	"   *  For more info on using U-Boot, visit                     *\n"\
	"   *  https://documentation.onioniot.com/bootloader/overview   *\n"\
	"   *                                                           *\n"\
	"   *  Hold the reset button to enter the U-Boot commandline.   *\n"\
	"   *************************************************************\n"\
	"\n"

int board_late_init(void)
{
	gpio_init();

	printf(WELCOME_MESSAGE);

	set_mtdparts();

	if (detect_rst()) {
		printf("Reset button pressed - entering shell ...\n");
		env_set("reset_pressed", "1");
		// This env variable is evaluated by our bootcmd.
	}

	return 0;
}
