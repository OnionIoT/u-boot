// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Lazar Demin <lazar@onioniot.com>
 */

#include <common.h>
#include <asm/io.h>

#define OMEGA2_REG(x)		    (*((volatile u32 *)(x)))
#define	OMEGA2_SYSCTL_BASE 	    0xB0000000
#define OMEGA2_SYS_CNTL_BASE    (OMEGA2_SYSCTL_BASE)
#define	OMEGA2_REG_PIODIR	    (OMEGA2_SYSCTL_BASE + 0x600)
#define	OMEGA2_REG_PIODATA      (OMEGA2_REG_PIODIR + 0x20)

int detect_rst( void )
{
	u32 val;
	val=OMEGA2_REG(0xb0000624); // Read GPIO 44 (reset button)

    return (val&1<<6) ? 1 : 0;
}


void gpio_init(void)
{
    u32 val;
    printf( "Initializing MT7688 GPIO system.\n" );
    
    //set gpio2_mode 1:0=2b01 wled,p0,p1,p2,p3,p4 as gpio
    val = 0x555;
    OMEGA2_REG(OMEGA2_SYS_CNTL_BASE+0x64)=val; // GPIO2_MODE register
    OMEGA2_REG(0xb0000644)=0x0f<<7;
    
    //gpio44 output gpio_ctrl_1 bit3=1
    val=OMEGA2_REG(OMEGA2_REG_PIODIR+0x04);
    val|=1<<12;
    OMEGA2_REG(OMEGA2_REG_PIODIR+0x04)=val;
    
    //set gpio1_mode 14=1b1
    val=OMEGA2_REG(OMEGA2_SYS_CNTL_BASE+0x60);
    val|=1<<14;
    OMEGA2_REG(OMEGA2_SYS_CNTL_BASE+0x60)=val;
    
    //gpio38 input gpio_ctrl_1 bit5=0
    val=OMEGA2_REG(OMEGA2_REG_PIODIR+0x04);
    val&=~1<<6;
    OMEGA2_REG(OMEGA2_REG_PIODIR+0x04)=val;

    //setting GPIO 11 High, required for the reset button to work
    val=OMEGA2_REG(OMEGA2_REG_PIODIR);
    val|=1<<11;
    OMEGA2_REG(OMEGA2_REG_PIODIR) = val; // GPIO 11 direction output
    val=OMEGA2_REG(OMEGA2_REG_PIODATA);
    val|=1<<11;
    OMEGA2_REG(OMEGA2_REG_PIODATA) = val; // GPIO 11 High
}

int  board_late_init (void)
{
    gpio_init();

    return 0;
}
