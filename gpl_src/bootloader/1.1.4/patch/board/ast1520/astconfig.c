#include <common.h>
#include <command.h>

uchar *Config_Base = CFG_FLASH_BASE + CFG_ASTCONFIG_OFFSET;
uchar *Misc_Base = CFG_FLASH_BASE + 0x70000;

int matchMiscParam(char *s1, int i2)
{
	char *param = Misc_Base + 4;
	while (*s1 == param[i2++])
		if (*s1++ == '=')
			return(i2);
	if ((*s1 == '\0') && (param[i2 - 1] == '='))
		return(i2);
	return(-1);
}

char *getMiscParam(char *name)
{
	int i, j, nxt;
	char *param = Misc_Base + 4;

	j = -1;

	for (i = 0; param[i] != '\0'; i = nxt+1) {
		for (nxt = i; param[nxt] != '\0'; ++nxt)
			;
		j = matchMiscParam(name, i);
		if (j < 0) {
			continue;
		}
		//printf("%s", &param[j]);
		return &param[j];
		break;
	}

	if (j < 0) {
		printf ("\"%s\" not defined\n", name);
	}
	return NULL;
}

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

