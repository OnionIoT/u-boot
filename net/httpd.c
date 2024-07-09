/*
 *	Copyright 1994, 1995, 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000, 2001 DENX Software Engineering, Wolfgang Denk, wd@denx.de
 */

#include <common.h>
#include <command.h>
#include <net.h>
#include <asm/byteorder.h>
#include <linux/delay.h>
#include "httpd.h"

#include "../httpd/uipopt.h"
#include "../httpd/uip.h"
#include "../httpd/uip_arp.h"
#include "../include/mtd.h"

// progress state info
#define WEBFAILSAFE_PROGRESS_START			0
#define WEBFAILSAFE_PROGRESS_TIMEOUT			1
#define WEBFAILSAFE_PROGRESS_UPLOAD_READY		2
#define WEBFAILSAFE_PROGRESS_UPGRADE_READY		3
#define WEBFAILSAFE_PROGRESS_UPGRADE_FAILED		4

// update type
#define WEBFAILSAFE_UPGRADE_TYPE_FIRMWARE		0
#define WEBFAILSAFE_UPGRADE_TYPE_UBOOT			1
#define WEBFAILSAFE_UPGRADE_TYPE_ART			2
#define WEBFAILSAFE_UPLOAD_RAM_ADDRESS			0x81800000
#define WEBFAILSAFE_UPLOAD_UBOOT_SIZE_IN_BYTES		( 640 * 1024 )
#define WEBFAILSAFE_UPLOAD_ART_SIZE_IN_BYTES		( 64 * 1024 )

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
extern void led_on(void);
extern void led_off(void);
//#include <gpio.h>
// #include <spi_api.h>

static int arptimer = 0;

void HttpdHandler( void ){
	int i;

	if ( uip_len == 0 ) {
		for ( i = 0; i < UIP_CONNS; i++ ) {
			uip_periodic( i );
			if ( uip_len > 0 ) {
				uip_arp_out();
				net_send_httpd();
			}
		}

		if ( ++arptimer == 20 ) {
			uip_arp_timer();
			arptimer = 0;
		}
	} else {
		if ( BUF->type == htons( UIP_ETHTYPE_IP ) ) {
			uip_arp_ipin();
			uip_input();
			if ( uip_len > 0 ) {
				uip_arp_out();
				net_send_httpd();
			}
		} else if ( BUF->type == htons( UIP_ETHTYPE_ARP ) ) {
			uip_arp_arpin();
			if ( uip_len > 0 ) {
				net_send_httpd();
			}
		}
	}

}

// start http daemon
void HttpdStart( void ){
	uip_init();
	httpd_init();
}

int do_http_upgrade( ulong size, const int upgrade_type ){
	// need to take these values from configs
	if(size % 0x800){
		ulong rem = 0x800 - size % 0x800;
		size = size + rem;
	}
	#define WEBFAILSAFE_UPLOAD_RAM_ADDRESS 0x81800000

	if ( upgrade_type == WEBFAILSAFE_UPGRADE_TYPE_UBOOT ) {
		
		// not supported yet
		printf( "\n\n****************************\n*     U-BOOT UPGRADING     *\n* DO NOT POWER OFF DEVICE! *\n****************************\n\n" );
		return(-1);

	} else if ( upgrade_type == WEBFAILSAFE_UPGRADE_TYPE_FIRMWARE ) {

		printf( "\n\n****************************\n*    FIRMWARE UPGRADING    *\n* DO NOT POWER OFF DEVICE! *\n****************************\n\n" );
		run_command("mtd list", 0);
		run_command("mtd erase firmware", 0);
		run_command("mtd write firmware 0x81800000 0x0 fc0800", 0);
		
		return (0);

	} else {
		return(-1);
	}

	return(-1);
}

// info about current progress of failsafe mode
int do_http_progress( const int state ){

	/* toggle LED's here */
	switch ( state ) {
		case WEBFAILSAFE_PROGRESS_START:

			printf( "HTTP server is up and running.\n" );
			break;

		case WEBFAILSAFE_PROGRESS_TIMEOUT:
			//printf("Waiting for request...\n");
			break;

		case WEBFAILSAFE_PROGRESS_UPLOAD_READY:


			printf("HTTP upload is complete.\n");
			printf("Upgrading...\n");

			break;

		case WEBFAILSAFE_PROGRESS_UPGRADE_READY:

			printf( "HTTP ugrade is done! Rebooting...\n\n" );
			break;

		case WEBFAILSAFE_PROGRESS_UPGRADE_FAILED:
			printf( "## Error: HTTP ugrade failed!\n\n" );



			// wait 1 sec
			mdelay( 1000 );

			break;
	}

	return( 0 );
}
