#include <common.h>
#include <command.h>

uchar *Config_Base = CFG_FLASH_BASE + CFG_ASTCONFIG_OFFSET;

int do_configdump(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	uchar *config_base = Config_Base + 4;
	int i, j, k, nxt;

	for (i = 0; (*(config_base + i) != '\0') && i < CFG_ASTCONFIG_SIZE; i = nxt+1) {
		for (nxt = i; (*(config_base + nxt) != '\0') && i < CFG_ASTCONFIG_SIZE; ++nxt)
			;
		for (k = i; k < nxt; ++k)
			putc(*(config_base + k));
		putc  ('\n');
	}
	return 0;
}


U_BOOT_CMD(
	configdump,	1,	0,	do_configdump,
	"configdump - dump AST configurations\n",
	NULL
);

int do_configflush(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	uchar buffer[6];/* 4 bytes of CRC & first 2 bytes of data*/
	memset(buffer, 0, 6);
	flash_sect_erase(Config_Base, Config_Base + CFG_ASTCONFIG_SIZE - 1);
	flash_write(buffer, Config_Base, 6);
	return 0;
}


U_BOOT_CMD(
	configflush,	1,	0,	do_configflush,
	"configflush - flush AST configurations\n",
	NULL
);

