#include <common.h>
#include <command.h>
#include <net.h>

static int do_web_recovery(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	net_loop_httpd();

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	webrec, 2,	1,	do_web_recovery,
	"Firmware recovery via http server",
	""
);
