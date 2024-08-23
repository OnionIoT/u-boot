// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Zheng Han <zh@onioniot.com>, Lazar Demin <lazar@onioniot.com>
 */

#include <common.h>
#include <asm/io.h>

#define OMEGA3_REG(x)		    (*((volatile u32 *)(x)))
#define	OMEGA3_SYSCTL_BASE 	    0xB0000000
#define OMEGA3_SYS_CNTL_BASE    (OMEGA3_SYSCTL_BASE)
#define	OMEGA3_REG_PIODIR	    (OMEGA3_SYSCTL_BASE + 0x600)
#define	OMEGA3_REG_PIODATA      (OMEGA3_REG_PIODIR + 0x20)

int detect_rst( void )
{
	u32 val;
	val=OMEGA3_REG(0xb0000624); // Read GPIO 44 (reset button)

    return (val&1<<6) ? 1 : 0;
}


void gpio_init(void)
{
    u32 val;
    printf( "Initializing MT7688 GPIO system.\n" );
    
    //set gpio2_mode - setting wled, and p0,p1,p2,p3,p4 LED pins to GPIO mode
    val = 0x555;
    OMEGA3_REG(OMEGA3_SYS_CNTL_BASE+0x64)=val; // GPIO2_MODE register
    OMEGA3_REG(0xb0000644)=0x0f<<7; // GINT_FEDGE_1: setting "Enable falling edge triggered" for GPIOs 39-42
    
    // set gpio_ctrl_1 register: set GPIO44 to output
    //gpio44 output gpio_ctrl_1 bit3=1
    val=OMEGA3_REG(OMEGA3_REG_PIODIR+0x04);
    val|=1<<12;
    OMEGA3_REG(OMEGA3_REG_PIODIR+0x04)=val;
    
    // set gpio1_mode register: set WDT_MODE to GPIO mode
    //set gpio1_mode 14=1b1
    val=OMEGA3_REG(OMEGA3_SYS_CNTL_BASE+0x60);
    val|=1<<14;
    OMEGA3_REG(OMEGA3_SYS_CNTL_BASE+0x60)=val;
    
    // set gpio_ctrl_1 resgister: set GPIO38 to input
    //gpio38 input gpio_ctrl_1 bit5=0
    val=OMEGA3_REG(OMEGA3_REG_PIODIR+0x04);
    val&=~1<<6;
    OMEGA3_REG(OMEGA3_REG_PIODIR+0x04)=val;
}

int  board_late_init (void)
{
    gpio_init();

    return 0;
}
