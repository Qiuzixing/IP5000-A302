#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <regex.h>
#include <ctype.h>
#include "debugtool.h"
#include "p3kswitch.h"
#include "common.h"
#include "funcexcute.h"
/*******************************************************************
******************************************************************/
#define MAX_PARAM_COUNT 10
typedef struct _P3K_PhraserToExecute_S
{
	char *cmd;
	int (*ParamPhraser)(char *reqparam, char *respParam, char *userdata);
	int paramnum;
} P3K_PhraserToExecute_S;

#define PARAM_SEPARATOR ','

int P3K_OtherChanges(char *info)
{
	int ret = 0;
	// test_printf();
	ret = EX_AutomaticReporting(info);
	return ret;
}

char *rtrim(char *str)
{
    if ((str == NULL) || (*str == '\0'))
    {
		DBG_ErrMsg("param empty\n");
        return str;
    }

    int len = strlen(str);
    char *p = str + len - 1;
    if ((isspace(*p)) || ((*p) == '\r') || ((*p) == '\n'))
    {
        *p = '\0';
        --p;
    }
    return str;
}

static int ERR_MSG(ERROR_TYPE_E i_Err, char *i_reqparam, char *o_respParam)
{
	char tmpdata[256] = {0};
	sprintf(tmpdata, "ERR %03d", i_Err);
	memcpy(o_respParam, tmpdata, strlen(tmpdata));
	return 0;
}

static int EX_ERR_MSG(int i_Err, char *i_reqparam, char *o_respParam)
{
	char tmpdata[256] = {0};
	if (i_Err == -1)
	{
		sprintf(tmpdata, "ERR %03d", ERR_COMMAND_NOT_AVAILABLE);
		memcpy(o_respParam, tmpdata, strlen(tmpdata));
	}
	else if (i_Err == -2)
	{
		sprintf(tmpdata, "ERR %03d", ERR_PARAMETER_OUT_OF_RANGE);
		memcpy(o_respParam, tmpdata, strlen(tmpdata));
	}
	else if (i_Err == -3)
	{
		sprintf(tmpdata, "ERR %03d", ERR_WRONG_MODE);
		memcpy(o_respParam, tmpdata, strlen(tmpdata));
	}
	return 0;
}

static int isnum(char s[])
{
	int i;
	for (i = 0; i < strlen(s); i++)
	{
		if (s[0] == '-')
		{
			continue;
		}
		if ((s[i] < '0') || (s[i] > '9'))
		{
			return -1;
		}
	}
	return 0;
}

static int Param_Check(int i_num, int i_isnum, char *i_reqparam, char *o_respParam)
{
	if (i_num != i_isnum)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, i_reqparam, o_respParam);
	}
}

static int checkisIp(char *IP)
{
	if ((strlen(IP) < 7) || (strlen(IP) > 15))
		return -1;
	char str[10][256] = {0};
	int count = 0;
	count = P3K_PhraserWithSeparator('.', IP, strlen(IP), str);
	if (count != 4)
		return -1;
	int ip1 = atoi(str[0]);
	int ip2 = atoi(str[1]);
	int ip3 = atoi(str[2]);
	int ip4 = atoi(str[3]);
	if (0 < ip1 && ip1 <= 255 && 0 <= ip2 && ip2 <= 255 && 0 <= ip3 && ip3 <= 255 && 0 < ip4 && ip4 < 255)
	{
		if ((ip1 == 127) || (ip1 > 223))
			return -1;
	}
	else
	{
		return -1;
	}
	return 0;
}

int IsSubnetMask(char *subnet)
{
	unsigned int b = 0;
	unsigned int i = 0;
	unsigned int n[4] = {0};
	sscanf(subnet, "%u.%u.%u.%u", &n[3], &n[2], &n[1], &n[0]);
	for (i = 0; i < 4; ++i)
		b += n[i] << (i * 8);
	b = ~b + 1;
	if ((b & (b - 1)) == 0)
		return 1;
	return 0;
}

int checkgateway(char *ip,char *mask,char *gateway)
{
	int i = 0;
	unsigned int ipstr[4] = {0};
	unsigned int maskstr[4] = {0};
	unsigned int gatewaystr[4] = {0};
	sscanf(ip, "%u.%u.%u.%u", &ipstr[0], &ipstr[1], &ipstr[2], &ipstr[3]);
	sscanf(mask, "%u.%u.%u.%u", &maskstr[0], &maskstr[1], &maskstr[2], &maskstr[3]);
	sscanf(gateway, "%u.%u.%u.%u", &gatewaystr[0], &gatewaystr[1], &gatewaystr[2], &gatewaystr[3]);
	if((gatewaystr[0] == 0) && (gatewaystr[1] == 0) && (gatewaystr[2] == 0) && (gatewaystr[3] == 0))
	{
		return 0;
	}
	for(i = 0; i < 3; i++)
	{
		if((ipstr[i] & maskstr[i]) == (gatewaystr[i] & maskstr[i]))
		{

		}
		else
			return -1;
	}
	return 0;
}

static int is_valid_mac_addr(char *mac)
{
	int status;
	const char *pattern = "^([A-Fa-f0-9]{2}[-,:]){5}[A-Fa-f0-9]{2}$";
	const int cflags = REG_EXTENDED | REG_NEWLINE;

	char ebuf[128];
	regmatch_t pmatch[1];
	int nmatch = 10;
	regex_t reg;
	status = regcomp(&reg, pattern, cflags);
	if (status != 0)
	{
		return -1;
	}
	status = regexec(&reg, mac, nmatch, pmatch, 0);
	if (status != 0)
	{
		return -1;
	}
	return 0;
}

static int is_valid_date(char *date)
{
	int status;
	const char *pattern = "^([0-9]{2}[-]){2}[0-9]{4}$";
	const int cflags = REG_EXTENDED | REG_NEWLINE;

	char ebuf[128] = {0};
	regmatch_t pmatch[1];
	int nmatch = 10;
	regex_t regdate;

	status = regcomp(&regdate, pattern, cflags);
	if (status != 0)
	{
		regerror(status, &regdate, ebuf, sizeof(ebuf));
		fprintf(stderr, "regcomp fail: %s , pattern '%s' \n", ebuf, pattern);
		return -1;
	}

	status = regexec(&regdate, date, nmatch, pmatch, 0);
	if (status != 0)
	{
		regerror(status, &regdate, ebuf, sizeof(ebuf));
		fprintf(stderr, "regexec fail: %s , date:\"%s\" \n", ebuf, date);
		return -1;
	}
	return 0;
}

static int is_valid_time(char *time)
{
	int status = 0;
	const char *pattern = "^([0-9]{2}[:]){2}[0-9]{2}$";
	const int cflags = REG_EXTENDED | REG_NEWLINE;
	char ebuf[128] = {0};
	regmatch_t pmatch[1];
	int nmatch = 10;
	regex_t regtime;
	status = regcomp(&regtime, pattern, cflags);
	if (status != 0)
	{
		regerror(status, &regtime, ebuf, sizeof(ebuf));
		fprintf(stderr, "regcomp fail: %s , pattern '%s' \n", ebuf, pattern);
		return -1;
	}
	status = regexec(&regtime, time, nmatch, pmatch, 0);
	if (status != 0)
	{
		regerror(status, &regtime, ebuf, sizeof(ebuf));
		fprintf(stderr, "regexec fail: %s , time:\"%s\" \n", ebuf, time);
		return -1;
	}
	return 0;
}

static int P3K_PhraserWithSeparator(char separator, char *param, int len, char str[][256])
{
	int tmpLen = 0;
	int i = 0;
	char *tmpdata = param;
	char *tmpdata1 = param;
	if ((param == NULL) || (len <= 0) || (len > 4096))
	{
		DBG_ErrMsg("param ERROR!\n");
		return -1;
	}
	while (tmpdata != NULL)
	{
		tmpdata = strchr(tmpdata, separator);
		if (tmpdata != NULL)
		{
			tmpLen = tmpdata - tmpdata1;
			if (tmpLen > MAX_PARAM_LEN)
				return -2;
			if (tmpLen == 0)
			{
				return -1;
			}
			memcpy(str[i], tmpdata1, tmpLen);
			i++;
			if (len >= tmpdata - param + 1)
			{
				tmpdata1 = tmpdata + 1;
				tmpdata = tmpdata + 1;
			}
			else
			{
				break;
			}
		}
	}
	if (strlen(tmpdata1) > MAX_PARAM_LEN)
	{
		return -2;
	}
	else if (strlen(tmpdata1) == 0)
	{
		return -1;
	}
	memcpy(str[i], tmpdata1, strlen(tmpdata1));
	i++;
	return i;
}

static int P3K_CheckNTFYCMD(char *data)
{
	int tmpDirec = NTFY_AUDIO;
	if ((!strcasecmp(data, "AUDIO")))
	{
		tmpDirec = NTFY_AUDIO;
	}
	else if ((!strcasecmp(data, "VIDEO")))
	{
		tmpDirec = NTFY_VIDEO;
	}
	else if ((!strcasecmp(data, "HDCP")))
	{
		tmpDirec = NTFY_HDCP;
	}
	else if ((!strcasecmp(data, "SWITCH")))
	{
		tmpDirec = NTFY_SWITCH;
	}
	else if ((!strcasecmp(data, "AUDIO_SWITCH")))
	{
		tmpDirec = NTFY_AUDIO_SWITCH;
	}
	else if ((!strcasecmp(data, "INPUT")))
	{
		tmpDirec = NTFY_INPUT;
	}
	else if ((!strcasecmp(data, "OUTPUT")))
	{
		tmpDirec = NTFY_OUTPUT;
	}
	else if ((!strcasecmp(data, "CEC_MSG")))
	{
		tmpDirec = NTFY_CEC_MSG;
	}
	else if ((!strcasecmp(data, "IR_MSG")))
	{
		tmpDirec = NTFY_IR_MSG;
	}
	else if ((!strcasecmp(data, "RS232_MSG")))
	{
		tmpDirec = NTFY_RS232_MSG;
	}
	else if ((!strcasecmp(data, "BUTTON")))
	{
		tmpDirec = NTFY_BUTTON;
	}
	else if ((!strcasecmp(data, "INPUT/OUTPUT")))
	{
		tmpDirec = NTFY_INPUT_OUTPUT;
	}
	else if ((!strcasecmp(data, "con_list")))
	{
		tmpDirec = NTFY_CON_LIST;
	}
	else
	{
		tmpDirec = -10;
	}
	return tmpDirec;
}

static int P3K_NTFYToStr(int type, char *data)
{
	char tmpbuf[32] = {0};
	memset(tmpbuf, 0, sizeof(tmpbuf));
	switch (type)
	{
	case NTFY_AUDIO:
		strcpy(tmpbuf, "AUDIO-NTFY");
		break;
	case NTFY_VIDEO:
		strcpy(tmpbuf, "VIDEO-NTFY");
		break;
	case NTFY_HDCP:
		strcpy(tmpbuf, "HDCP-NTFY");
		break;
	case NTFY_SWITCH:
		strcpy(tmpbuf, "SWITCH-NTFY");
		break;
	case NTFY_AUDIO_SWITCH:
		strcpy(tmpbuf, "AUDIO_SWITCH-NTFY");
		break;
	case NTFY_INPUT:
		strcpy(tmpbuf, "SIGNALS-LIST");
		break;
	case NTFY_OUTPUT:
		strcpy(tmpbuf, "SIGNALS-LIST");
		break;
	case NTFY_CEC_MSG:
		strcpy(tmpbuf, "CEC-NTFY");
		break;
	case NTFY_IR_MSG:
		strcpy(tmpbuf, "IR-NTFY");
		break;
	case NTFY_RS232_MSG:
		strcpy(tmpbuf, "RS232-NTFY");
		break;
	case NTFY_BUTTON:
		strcpy(tmpbuf, "BUTTON-NOTIFY");
		break;
	case NTFY_INPUT_OUTPUT:
		strcpy(tmpbuf, "SIGNALS-LIST");
		break;
	case NTFY_CON_LIST:
		strcpy(tmpbuf, "NET-STAT");
		break;
	default:
		strcpy(tmpbuf, "DEFAULT");
		break;
	}
	strcpy(data, tmpbuf);

	return 0;
}

static int P3K_CheckEdidMode(char *data)
{
	int tmpDirec = PASSTHRU;
	if ((!strcasecmp(data, "PASSTHRU")) || (!strcmp(data, "passthru")))
	{
		tmpDirec = PASSTHRU;
	}
	else if ((!strcasecmp(data, "CUSTOM")) || (!strcmp(data, "custom")))
	{
		tmpDirec = CUSTOM;
	}
	else if ((!strcasecmp(data, "DEFAULT")) || (!strcmp(data, "default")))
	{
		tmpDirec = DEFAULT;
	}
	else
	{
		{
			tmpDirec = -10;
		}
	}
	return tmpDirec;
}

static int P3K_ModeToStr(int type, char *data)
{
	char tmpbuf[32] = {0};
	memset(tmpbuf, 0, sizeof(tmpbuf));
	switch (type)
	{
	case PASSTHRU:
		strcpy(tmpbuf, "PASSTHRU");
		break;
	case CUSTOM:
		strcpy(tmpbuf, "CUSTOM");
		break;
	case DEFAULT:
		strcpy(tmpbuf, "DEFAULT");
		break;
	default:
		strcpy(tmpbuf, "DEFAULT");
		break;
	}
	memcpy(data, tmpbuf, strlen(tmpbuf));
	return 0;
}

static int P3K_CheckPortDirection(char *data)
{
	int tmpDirec = DIRECTION_IN;
	if (!strcasecmp(data, "in") || !strcasecmp(data, "[in"))
	{
		tmpDirec = DIRECTION_IN;
	}
	else if (!strcasecmp(data, "out") || !strcasecmp(data, "[out"))
	{
		tmpDirec = DIRECTION_OUT;
	}
	else if (!strcasecmp(data, "both") || !strcasecmp(data, "[both"))
	{
		tmpDirec = DIRECTION_BOTH;
	}
	else
	{
		tmpDirec = -10;
	}
	return tmpDirec;
}
static int P3K_PortDirectionToStr(int direc, char *data)
{
	char tmpbuf[32] = {0};
	memset(tmpbuf, 0, sizeof(tmpbuf));
	switch (direc)
	{
	case DIRECTION_IN:
		strcpy(tmpbuf, "in");
		break;
	case DIRECTION_OUT:
		strcpy(tmpbuf, "out");
		break;
	case DIRECTION_BOTH:
		strcpy(tmpbuf, "both");
		break;
	default:
		strcpy(tmpbuf, "in");
		break;
	}
	memcpy(data, tmpbuf, strlen(tmpbuf));
	return 0;
}

static int P3K_CheckPortFormat(char *data)
{
	int tmpFormat = PORT_HDMI;
	if (!strcasecmp(data, "hdmi"))
	{
		tmpFormat = PORT_HDMI;
	}
	else if (!strcasecmp(data, "analog_audio"))
	{
		tmpFormat = PORT_ANALOG_AUDIO;
	}
	else if (!strcasecmp(data, "analog"))
	{
		tmpFormat = PORT_ANALOG_AUDIO;
	}
	else if (!strcasecmp(data, "rs232"))
	{
		tmpFormat = PORT_RS232;
	}
	else if (!strcasecmp(data, "ir"))
	{
		tmpFormat = PORT_IR;
	}
	else if (!strcasecmp(data, "usb_a"))
	{
		tmpFormat = PORT_USB_A;
	}
	else if (!strcasecmp(data, "usb_b"))
	{
		tmpFormat = PORT_USB_B;
	}
	else if (!strcasecmp(data, "usb_c"))
	{
		tmpFormat = PORT_USB_C;
	}
	else if (!strcasecmp(data, "dante"))
	{
		tmpFormat = PORT_DANTE;
	}
	else if (!strcasecmp(data, "hdbt"))
	{
		tmpFormat = PORT_HDBT;
	}
	else if (!strcasecmp(data, "amplified_audio"))
	{
		tmpFormat = PORT_AMPLIFIED_AUDIO;
	}
	else if (!strcasecmp(data, "tos"))
	{
		tmpFormat = PORT_TOS;
	}
	else if (!strcasecmp(data, "spdif"))
	{
		tmpFormat = PORT_SPDIF;
	}
	else if (!strcasecmp(data, "mic"))
	{
		tmpFormat = PORT_MIC;
	}
	else if (!strcasecmp(data, "stream"))
	{
		tmpFormat = PORT_STREAM;
	}
	else
	{
		tmpFormat = -10;
	}
	return tmpFormat;
}
static int P3K_PortFormatToStr(int format, char *data)
{
	char tmpbuf[32] = {0};
	memset(tmpbuf, 0, sizeof(tmpbuf));
	switch (format)
	{
	case PORT_HDMI:
		strcpy(tmpbuf, "hdmi");
		break;
	case PORT_USB_A:
		strcpy(tmpbuf, "usb_a");
		break;
	case PORT_USB_B:
		strcpy(tmpbuf, "usb_b");
		break;
	case PORT_USB_C:
		strcpy(tmpbuf, "usb_c");
		break;
	case PORT_ANALOG_AUDIO:
		strcpy(tmpbuf, "analog_audio");
		break;
	case PORT_IR:
		strcpy(tmpbuf, "ir");
		break;
	case PORT_RS232:
		strcpy(tmpbuf, "rs232");
		break;
	case PORT_DANTE:
		strcpy(tmpbuf, "dante");
		break;
	case PORT_HDBT:
		strcpy(tmpbuf, "hdbi");
		break;
	case PORT_AMPLIFIED_AUDIO:
		strcpy(tmpbuf, "amplified_audio");
		break;
	case PORT_TOS:
		strcpy(tmpbuf, "tos");
		break;
	case PORT_SPDIF:
		strcpy(tmpbuf, "spdif");
		break;
	case PORT_MIC:
		strcpy(tmpbuf, "mic");
		break;
	case PORT_STREAM:
		strcpy(tmpbuf, "stream");
		break;
	default:
		strcpy(tmpbuf, "analog_audio");
		break;
	}
	memcpy(data, tmpbuf, strlen(tmpbuf));
	return 0;
}

static int P3K_StateToStr(int direc, char *data)
{
	char tmpbuf[32] = {0};
	memset(tmpbuf, 0, sizeof(tmpbuf));
	switch (direc)
	{
	case ON:
		strcpy(tmpbuf, "on");
		break;
	case OFF:
		strcpy(tmpbuf, "off");
		break;
	default:
		strcpy(tmpbuf, "off");
		break;
	}
	memcpy(data, tmpbuf, strlen(tmpbuf));
	return 0;
}

static int P3K_CheckSignalType(char *data)
{
	int tmpFormat = SIGNAL_VIDEO;
	if (!strcasecmp(data, "audio") || !strcasecmp(data, "[audio") || !strcasecmp(data, "audio]"))
	{
		tmpFormat = SIGNAL_AUDIO;
	}
	else if (!strcasecmp(data, "video") || !strcasecmp(data, "[video") || !strcasecmp(data, "video]"))
	{
		tmpFormat = SIGNAL_VIDEO;
	}
	else if (!strcasecmp(data, "ir") || !strcasecmp(data, "[ir") || !strcasecmp(data, "ir]"))
	{
		tmpFormat = SIGNAL_IR;
	}
	else if (!strcasecmp(data, "usb") || !strcasecmp(data, "[usb") || !strcasecmp(data, "usb]"))
	{
		tmpFormat = SIGNAL_USB;
	}
	else if (!strcasecmp(data, "arc") || !strcasecmp(data, "[arc") || !strcasecmp(data, "arc]"))
	{
		tmpFormat = SIGNAL_ARC;
	}
	else if (!strcasecmp(data, "rs232") || !strcasecmp(data, "[rs232") || !strcasecmp(data, "rs232]"))
	{
		tmpFormat = SIGNAL_RS232;
	}
	else if (!strcasecmp(data, "cec") || !strcasecmp(data, "[cec") || !strcasecmp(data, "cec]"))
	{
		tmpFormat = SIGNAL_CEC;
	}
	else if (!strcasecmp(data, "av_test_pattern") || !strcasecmp(data, "[av_test_pattern") || !strcasecmp(data, "av_test_pattern]"))
	{
		tmpFormat = SIGNAL_TEST;
	}
	else
	{
		tmpFormat = -10;
	}
	return tmpFormat;
}

static int P3K_CheckStateType(char *data)
{
	int tmpFormat = OFF;
	if (!strcasecmp(data, "off"))
	{
		tmpFormat = OFF;
	}
	else if (!strcasecmp(data, "on"))
	{
		tmpFormat = ON;
	}
	else
	{
		tmpFormat = -10;
	}
	return tmpFormat;
}

static int P3K_SignaleTypeToStr(int signaltype, char *data)
{
	char tmpbuf[32] = {0};
	memset(tmpbuf, 0, sizeof(tmpbuf));
	switch (signaltype)
	{
	case SIGNAL_VIDEO:
		strcpy(tmpbuf, "video");
		break;
	case SIGNAL_AUDIO:
		strcpy(tmpbuf, "audio");
		break;
	case SIGNAL_IR:
		strcpy(tmpbuf, "ir");
		break;
	case SIGNAL_ARC:
		strcpy(tmpbuf, "arc");
		break;
	case SIGNAL_USB:
		strcpy(tmpbuf, "usb");
		break;
	case SIGNAL_RS232:
		strcpy(tmpbuf, "rs232");
		break;
	case SIGNAL_CEC:
		strcpy(tmpbuf, "cec");
		break;
	default:
		strcpy(tmpbuf, "video");
		break;
	}
	memcpy(data, tmpbuf, strlen(tmpbuf));
	return 0;
}
static int P3K_CheckAudioSampleRate(char *data)
{
	int tmpRate = SAMPLE_RATE_44100;
	char *tmp = data;
	if (!strcasecmp(tmp, "44.1K"))
	{
		tmpRate = SAMPLE_RATE_44100;
	}
	else if (!strcasecmp(tmp, "48K"))
	{
		tmpRate = SAMPLE_RATE_48000;
	}
	else if (!strcasecmp(tmp, "32K"))
	{
		tmpRate = SAMPLE_RATE_32000;
	}
	else if (!strcasecmp(tmp, "22.5K"))
	{
		tmpRate = SAMPLE_RATE_22500;
	}
	else if (!strcasecmp(tmp, "16K"))
	{
		tmpRate = SAMPLE_RATE_16000;
	}
	else if (!strcasecmp(tmp, "8K"))
	{
		tmpRate = SAMPLE_RATE_8000;
	}
	else
	{
		tmpRate = -10;
	}
	return tmpRate;
}
static int P3K_AudioSampleRateToStr(int samplerate, char *data)
{
	char tmpbuf[32] = {0};
	memset(tmpbuf, 0, sizeof(tmpbuf));
	switch (samplerate)
	{
	case SAMPLE_RATE_192000:
		strcpy(tmpbuf, "192K");
		break;
	case SAMPLE_RATE_176400:
		strcpy(tmpbuf, "176.4K");
		break;
	case SAMPLE_RATE_96000:
		strcpy(tmpbuf, "96K");
		break;
	case SAMPLE_RATE_88200:
		strcpy(tmpbuf, "88.2K");
		break;
	case SAMPLE_RATE_48000:
		strcpy(tmpbuf, "48K");
		break;
	case SAMPLE_RATE_44100:
		strcpy(tmpbuf, "44.1K");
		break;
	case SAMPLE_RATE_32000:
		strcpy(tmpbuf, "32K");
		break;
	case SAMPLE_RATE_22500:
		strcpy(tmpbuf, "22.5K");
		break;
	case SAMPLE_RATE_16000:
		strcpy(tmpbuf, "16K");
		break;
	case SAMPLE_RATE_8000:
		strcpy(tmpbuf, "8K");
		break;
	default:
		strcpy(tmpbuf, "N/A");
		break;
	}
	memcpy(data, tmpbuf, strlen(tmpbuf));
	return 0;
}

static int P3K_CheckAudioType(char *data)
{
	int tmpType = AUDIO_FORMAT_PCM;
	// char *tmp = data;
	if (!strcasecmp(data, "PCM"))
	{
		tmpType = AUDIO_FORMAT_PCM;
	}
	else
	{
		tmpType = -10;
	}
	return tmpType;
}

static int P3K_AudioTypeToStr(int type, char *data)
{
	char tmpbuf[32] = {0};
	memset(tmpbuf, 0, sizeof(tmpbuf));
	switch (type)
	{
	case AUDIO_FORMAT_PCM:
		strcpy(tmpbuf, "PCM");
		break;
	default:
		strcpy(tmpbuf, "PCM");
		break;
	}
	memcpy(data, tmpbuf, strlen(tmpbuf));
	return 0;
}

static int P3K_CheckUartParity(char *data)
{
	int tmpType = AUDIO_FORMAT_PCM;
	// char *tmp = data;
	if (!strcasecmp(data, "none"))
	{
		tmpType = PARITY_NONE;
	}
	else if (!strcasecmp(data, "odd"))
	{
		tmpType = PARITY_ODD;
	}
	else if (!strcasecmp(data, "even"))
	{
		tmpType = PARITY_EVEN;
	}
	else if (!strcasecmp(data, "mark"))
	{
		tmpType = PARITY_MARK;
	}
	else if (!strcasecmp(data, "space"))
	{
		tmpType = PARITY_SPACE;
	}
	else
	{
		tmpType = -10;
	}
	return tmpType;
}

static int P3K_ParityToStr(int type, char *data)
{
	char tmpbuf[32] = {0};
	memset(tmpbuf, 0, sizeof(tmpbuf));
	switch (type)
	{
	case PARITY_NONE:
		strcpy(tmpbuf, "none");
		break;
	case PARITY_ODD:
		strcpy(tmpbuf, "odd");
		break;
	case PARITY_EVEN:
		strcpy(tmpbuf, "even");
		break;
	case PARITY_MARK:
		strcpy(tmpbuf, "mark");
		break;
	case PARITY_SPACE:
		strcpy(tmpbuf, "space");
		break;
	default:
		strcpy(tmpbuf, "none");
		break;
	}
	memcpy(data, tmpbuf, strlen(tmpbuf));
	return 0;
}

static int P3K_PhraserParam(char *param, int len, char str[][256])
{
	int tmpLen = 0;
	// int s32Ret = 0;
	int i = 0;
	if (param == NULL || len <= 0 || len > 4096)
	{
		return -1;
	}
	char *tmpdata = rtrim(param);
	char *tmpdata1 = rtrim(param);
	while (tmpdata != NULL)
	{
		tmpdata = strchr(tmpdata, PARAM_SEPARATOR);
		if (tmpdata != NULL)
		{
			tmpLen = tmpdata - tmpdata1;
			if (tmpLen > MAX_PARAM_LEN)
				return -2;
			if (tmpLen == 0)
			{
				DBG_ErrMsg("empty param!!!\n");
				return -1;
			}
			if (i >= MAX_PARAM_COUNT)
			{
				return -1;
			}
			// memset(str[i],0,MAX_PARAM_LEN);
			memcpy(str[i], tmpdata1, tmpLen);
			i++;
			if (len >= tmpdata - param + 1)
			{
				tmpdata1 = tmpdata + 1;
				tmpdata = tmpdata + 1;
			}
			else
			{
				break;
			}
		}
	}
	int paramlen = strlen(tmpdata1);
	if (paramlen > MAX_PARAM_LEN)
		return -2;
	if (paramlen == 0)
		return -1;
	if (i >= MAX_PARAM_COUNT)
	{
		return -1;
	}
	memcpy(str[i], tmpdata1, paramlen);
	i++;
	return i;
}

static int P3K_PhraserIRParam(char *param, int len, char str[][256], char irstr[4096])
{
	int tmpLen = 0;
	int i = 0;
	int tailLen = 0;
	if ((param == NULL) || (len <= 0))
	{
		return -1;
	}
	char *tmpdata = param;
	char *tmpdata1 = param;
	char *datahead = param;
	while (tmpdata != NULL)
	{
		tmpdata = strchr(tmpdata, PARAM_SEPARATOR);
		if (tmpdata != NULL)
		{
			tmpLen = tmpdata - tmpdata1;
			if (tmpLen > MAX_PARAM_LEN)
				return -2;
			if (tmpLen == 0)
			{
				DBG_ErrMsg("empty param!!!\n");
				return -1;
			}
			if (i < 6)
			{
				memcpy(str[i], tmpdata1, tmpLen);
				i++;
			}
			else
			{
				tmpLen = tmpdata1 - datahead;
				tailLen = strlen(param) - tmpLen;
				memcpy(irstr, tmpdata1, tailLen);
				return 0;
			}
			if (len >= tmpdata - param + 1)
			{
				tmpdata1 = tmpdata + 1;
				tmpdata = tmpdata + 1;
			}
			else
			{
				break;
			}
		}
	}
	if (strlen(tmpdata1) > MAX_PARAM_LEN)
		return -2;
	if (strlen(tmpdata1) == 0)
		return -1;
	if (i == 5)
	{
		memcpy(str[i], tmpdata1, strlen(tmpdata1));
		i++;
	}
	else
	{
		memcpy(irstr, tmpdata1, strlen(tmpdata1));
	}
	return i;
}

static int P3K_GetPortInfo(char *param, PortInfo_S *info, int num)
{
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	int count = 0;

	count = P3K_PhraserWithSeparator('.', param, strlen(param), str);
	if (count != num)
		return -3;

	info->direction = P3K_CheckPortDirection(str[0]);
	if (info->direction == -10)
	{
		return -3;
	}
	info->portFormat = P3K_CheckPortFormat(str[1]);
	if (info->portFormat == -10)
	{
		return -3;
	}
	int isnums = isnum(str[2]);
	if (isnums == -1)
	{
		return -3;
	}
	info->portIndex = atoi(str[2]);
	if (count > 3)
	{
		info->signal = P3K_CheckSignalType(str[3]);
		if (info->signal == -10)
		{
			return -3;
		}
		int isnums = isnum(str[4]);
		if (isnums == -1)
		{
			return -3;
		}
		info->index = atoi(str[4]);
	}
	return 0;
}

static int P3K_GetPortSInfo(char *param, PortInfo_S *info, int num, int paramnum)
{
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	int count = 0;
	count = P3K_PhraserWithSeparator('.', param, strlen(param), str);
	if (count != paramnum)
	{
		return -3;
	}

	info[num].direction = P3K_CheckPortDirection(str[0]);
	if (info[num].direction == -10)
	{
		return -3;
	}
	info[num].portFormat = P3K_CheckPortFormat(str[1]);
	if (info[num].portFormat == -10)
	{
		return -3;
	}
	int isnums = isnum(str[2]);
	if (isnums == -1)
	{
		return -3;
	}
	info[num].portIndex = atoi(str[2]);
	if (count > 3)
	{
		info[num].signal = P3K_CheckSignalType(str[3]);
		if (info[num].signal == -10)
		{
			return -3;
		}
		int isnums = isnum(str[4]);
		if (isnums == -1)
		{
			return -3;
		}
		info[num].index = atoi(str[4]);
	}
	return 0;
}

static int P3K_SetAudioInputMode(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("SetAudioInputMode\n");
	int mode = 0;
	int s32Ret = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	char tmpparam[MAX_PARAM_LEN] = {0};
	// sscanf(param,"%1d",mode);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	mode = atoi(str[0]);
	//��������
	s32Ret = EX_SetAudSrcMode(mode);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetAudSrcMode err\n");
		return -1;
		mode = 2;
	}

	//�ظ������cmd ��cmd������
	sprintf(tmpparam, "%d", mode);
	memcpy(respParam, tmpparam, MAX_PARAM_LEN);
	return 0;
}
static int P3K_GetAudioInputMode(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("GetAudioInputMode\n");
	int mode = 0;
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	s32Ret = EX_GetAudSrcMode(&mode);

	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetAudSrcMode err\n");
		return -1;
	}

	sprintf(tmpparam, "%d", mode);
	memcpy(respParam, tmpparam, MAX_PARAM_LEN);
	return 0;
}
static int P3K_SetAudLevel(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_SetAudLevel\n");
	//�����ײ���Ҫ����
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	// char tmpparam[MAX_PARAM_LEN] = {0};
	int gain = 0;
	int ret = 0;
	PortInfo_S tmpInfo = {0};
	int s32Ret = 0;
	// in.analog_audio.1.audio.1,10
	memset(&tmpInfo, 0, sizeof(PortInfo_S));
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	int isnums = isnum(str[1]);
	if (isnums == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}

	gain = atoi(str[1]);

	ret = P3K_GetPortInfo(str[0], &tmpInfo, 5);
	if (ret)
	{
		if (ret == -3)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}

	s32Ret = EX_SetAudGainLevel(&tmpInfo, gain);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetAudGainLevel err\n");
		return -1;
	}

	//�ظ������cmd ��cmd������
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetAudLevel(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_GetAudLevel\n");
	// int count = 0;
	// char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	PortInfo_S tmpInfo = {0};
	int gain = 0;
	// in.analog_audio.1.audio.1
	int ret = 0;
	ret = P3K_GetPortInfo(reqparam, &tmpInfo, 5);
	if (ret < 0)
	{
		if (ret == -3)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}

	s32Ret = EX_GetAudGainLevel(&tmpInfo, &gain);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetAudGainLevel err\n");
		return -1;
	}
	sprintf(tmpparam, "%s,%d", reqparam, gain);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetAudParam(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_GetAudParam\n");
	// int count = 0;
	int s32Ret = 0;
	// char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	PortInfo_S tmpInfo = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	AudioSignalInfo_S audioInfo = {0};
	char sampleRate[32] = {0};
	char audioType[32] = {0};
	char signal[32] = {0};
	char direc[32] = {0};
	char format[32] = {0};
	int ret = 0;
	//  out.hdmi.1
	tmpInfo.signal = SIGNAL_AUDIO;
	tmpInfo.index = 1;
	ret = P3K_GetPortInfo(reqparam, &tmpInfo, 3);
	if (ret < 0)
	{
		if (ret == -3)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	s32Ret = EX_GetAudParam(&tmpInfo, &audioInfo);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetAudParam err\n");
		return -1;
	}
	P3K_AudioSampleRateToStr(audioInfo.sampleRate, sampleRate);
	P3K_PortDirectionToStr(tmpInfo.direction, direc);
	P3K_PortFormatToStr(tmpInfo.portFormat, format);
	// P3K_AudioTypeToStr(audioInfo.format,audioType);
	P3K_SignaleTypeToStr(tmpInfo.signal, signal);
	if (audioInfo.chn > 0)
	{
		sprintf(tmpparam, "%s.%s.%d.%s.%d,%d,%s,%s", direc, format,
				tmpInfo.index, signal, tmpInfo.index, audioInfo.chn, sampleRate, audioInfo.format);
	}
	else
	{
		sprintf(tmpparam, "%s.%s.%d.%s.%d,N/A,N/A,N/A", direc, format,
				tmpInfo.index, signal, tmpInfo.index);
	}
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetAutoSwitchMode(char *reqparam, char *respParam, char *userdata)
{
	//#X-AV-SW-MODE <direction_type>. <port_format>. <port_index>. <signal_type>. <index>,connection_mode<CR>
	//~nn@X-AV-SW-MODE <direction_type>. <port_format>. <port_index>. <signal_type>. <index>,connection_mode<CR><LF>
	DBG_InfoMsg("P3K_SetAutoSwitchMode\n");
	// out.hdmi.1.video.1,2
	int mode = 0;
	int ret = 0;
	int count = 0;
	int s32Ret = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	PortInfo_S tmpInfo = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	// char tmpparam[MAX_PARAM_LEN] = {0};
	// sscanf(param,"%1d",mode);
	int isnums = isnum(str[1]);
	if (isnums == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	mode = atoi(str[1]);
	ret = P3K_GetPortInfo(str[0], &tmpInfo, 5);
	if (ret < 0)
	{
		if (ret == -3)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}

	s32Ret = EX_SetAutoSwitchMode(&tmpInfo, mode);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetAutoSwitchMode err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));

	return 0;
}

static int P3K_GetAutoSwitchMode(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_GetAutoSwitchMode\n");
	int count = 0;
	int ret = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	PortInfo_S tmpInfo = {0};
	int mode = 0;
	// in.analog_audio.1.audio.1
	ret = P3K_GetPortInfo(reqparam, &tmpInfo, 5);
	if (ret < 0)
	{
		if (ret == -3)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}

	s32Ret = EX_GetAutoSwitchMode(&tmpInfo, &mode);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("P3K_GetAutoSwitchMode err\n");
		return -1;
	}

	sprintf(tmpparam, "%s,%d", reqparam, mode);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetEdid(char *reqparam, char *respParam, char *userdata)
{
	//#GEDID io_mode,in_index<CR>
	//~nn@GEDID io_mode,in_index,size<CR><LF>
	DBG_InfoMsg("P3K_GetEdid\n");
	int mode = 0;
	int index = 0;
	int size = 0;
	int s32Ret = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	int count = 0;

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	int isnums = isnum(str[0]);
	if ((isnum(str[0]) == -1) || (isnum(str[1]) == -1))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	mode = atoi(str[0]);
	index = atoi(str[1]);

	s32Ret = EX_GetEDIDSupport(index, mode, &size);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetEDIDSupport err\n");
		return -1;
	}
	sprintf(respParam, "%s,%d", reqparam, size);
	return 0;
}

static int P3K_CopyEdid(char *reqparam, char *respParam, char *userdata)
{
	//#CPEDID edid_io,src_id,edid_io,dest_bitmap<CR>
	//#CPEDID edid_io,src_id,edid_io,dest_bitmap,safe_mode<CR>
	//~nn@CPEDID edid_io,src_id,edid_io,dest_bitmap<CR><LF>
	//~nn@CPEDID edid_io,src_id,edid_io,dest_bitmap,safe_mode<CR><LF>
	DBG_InfoMsg("P3K_CopyEdid\n");
	EDIDPortInfo_S src;
	EDIDPortInfo_S dest;
	int s32Ret = 0;
	int destBitMap = 0;
	int safeMode = -1;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	int count = 0;
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if ((isnum(str[0]) == -1) || (isnum(str[1]) == -1) || (isnum(str[2]) == -1) || (isnum(str[3]) == -1))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	src.type = atoi(str[0]);
	src.id = atoi(str[1]);
	dest.type = atoi(str[2]);
	dest.id = atoi(str[3]);
	if ((src.type < 0) || (src.type > 3) || (dest.type < 0) || (dest.type > 3))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
#if 0
	sscanf(str[4]);
	destBitMap = atoi(str[4]);
	if(count >5)
	{
		safeMode = atoi(str[5]);
	}
#endif
	s32Ret = EX_CopyEDID(&src, &dest, destBitMap, safeMode);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_CopyEDID err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_SetEdidCsMode(char *reqparam, char *respParam, char *userdata)
{
	//#EDID-CS <direction_type>.<port_format>.<port_index>. <signal_type>. <index>,cs_mode<CR>
	//~nn@EDID CS <direction_type>.<port_format>.<port_index>. <signal_type>. <index>,cs_mode<CR><LF>
	DBG_InfoMsg("P3K_SetEdidCsMode\n");
	int mode = 0;
	int ret = 0;
	int count = 0;
	int s32Ret = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	PortInfo_S tmpInfo = {0};
	// in.hdmi.3.video.1,0
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	char tmpparam[MAX_PARAM_LEN] = {0};
	// sscanf(param,"%1d",mode);
	if (isnum(str[1]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	mode = atoi(str[1]);
	ret = P3K_GetPortInfo(str[0], &tmpInfo, 5);
	if (ret < 0)
	{
		if (ret == -3)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}

	s32Ret = EX_SetEDIDColorSpaceMode(&tmpInfo, mode);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetEDIDColorSpaceMode err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetEdidCsMode(char *reqparam, char *respParam, char *userdata)
{
	//#EDID-CS? <direction_type>.<port_format>.<port_index>. <signal_type>. <index><CR>
	//~nn@EDID-CS <direction_type>.<port_format>.<port_index>. <signal_type>. <index>,cs_mode<CR><LF>
	DBG_InfoMsg("P3K_GetEdidCsMode\n");
	int count = 0;
	int s32Ret = 0;
	int ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	PortInfo_S tmpInfo = {0};
	int mode = 0;
	// in.hdmi.3.video.1
	ret = P3K_GetPortInfo(reqparam, &tmpInfo, 5);
	if (ret < 0)
	{
		if (ret == -3)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	s32Ret = EX_GetEDIDColorSpaceMode(&tmpInfo, &mode);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetEDIDColorSpaceMode err\n");
		return -1;
	}
	sprintf(tmpparam, "%s,%d", reqparam, mode);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}
static int P3K_SetEdidLockMode(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_SetEdidLockMode\n");
	int in_index = 0;
	int lockFlag = 0;
	int s32Ret = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	int count = 0;
	//#LOCK-EDID 2,1<CR>
	//~nn@LOCK-EDID 2,1<CR><LF>
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if ((isnum(str[0]) == -1) || (isnum(str[1]) == -1))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	in_index = atoi(str[0]);
	lockFlag = atoi(str[1]);

	s32Ret = EX_SetEDIDLockStatus(in_index, lockFlag);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetEDIDLockStatus err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}
static int P3K_GetEdidLockMode(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_GetEdidLockMode\n");
	int in_index = 0;
	int lockFlag = 0;
	int s32Ret = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	int count = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	//#LOCK-EDID? 2<CR>
	//~nn@LOCK-EDID? 2,1<CR><LF>
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	in_index = atoi(str[0]);
	s32Ret = EX_GetEDIDLockStatus(in_index, &lockFlag);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetEDIDLockStatus err\n");
		return -1;
	}
	sprintf(tmpparam, "%s,%d", reqparam, lockFlag);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetHDCPMode(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_SetHDCPMode\n");
	int in_index = 0;
	int mode = 0;
	int s32Ret = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	int count = 0;
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if ((isnum(str[0]) == -1) || (isnum(str[1]) == -1))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	in_index = atoi(str[0]);
	mode = atoi(str[1]);

	///#HDCP-MOD in_index,mode<CR>
	///~nn@HDCP-MOD in_index,mode<CR><LF>
	s32Ret = EX_SetHDCPMode(in_index, mode);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetHDCPMode err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));

	return 0;
}
static int P3K_GetHDCPMode(char *reqparam, char *respParam, char *userdata)
{
	//#HDCP-MOD? in_index<CR>
	//~nn@HDCP-MOD in_index,mode<CR><LF>
	DBG_InfoMsg("P3K_GetHDCPMode\n");
	int s32Ret = 0;
	int in_index = 0;
	int mode = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	int count = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	in_index = atoi(str[0]);
	s32Ret = EX_GetHDCPMode(in_index, &mode);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetHDCPMode err\n");
		return -1;
	}
	sprintf(tmpparam, "%s,%d", reqparam, mode);
	memcpy(respParam, tmpparam, strlen(tmpparam));

	return 0;
}

static int P3K_GetHDCPStatus(char *reqparam, char *respParam, char *userdata)
{
	//#HDCP STAT? io_mode,in_index<CR>
	//~nn@HDCP STAT io_mode,in_index,status<CR><LF>
	DBG_InfoMsg("P3K_GetHDCPStatus\n");
	int index = 0;
	int status = 0;
	int mode = 0;

	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	int count = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if ((isnum(str[0]) == -1) || (isnum(str[1]) == -1))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	mode = atoi(str[0]);
	index = atoi(str[1]);
	if (index != 0 && index != 1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}

	status = EX_GetHDCPStatus(mode, index);
	if (status < 0)
	{
		EX_ERR_MSG(status, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	sprintf(tmpparam, "%s,%d", reqparam, status);
	memcpy(respParam, tmpparam, strlen(tmpparam));

	return 0;
}

static int P3K_SetVideoWallMode(char *reqparam, char *respParam, char *userdata)
{
	//#VIEW-MOD mode,horizontal, vertical<CR>
	//~nn@VIEW-MOD mode, horizontal, vertical<CR><LF>

	DBG_InfoMsg("P3K_SetVideoWallMode\n");

	int s32Ret = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	int count = 0;
	int mode = 0;
	ViewModeInfo_S info;
	char tmpparam[MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (count == -2)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	mode = atoi(str[0]);
	if (mode == 15)
	{
		if (count != 3)
		{
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		if ((isnum(str[1]) == -1) || (isnum(str[2]) == -1))
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		info.hStyle = atoi(str[1]);
		info.vStyle = atoi(str[2]);
	}
	else
	{
		if (count != 1)
		{
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
	}
	s32Ret = EX_SetViewMode(mode, &info);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetViewMode err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetVideoWallMode(char *reqparam, char *respParam, char *userdata)
{
	//#VIEW-MOD? <CR>
	//~nn@VIEW-MOD mode, horizontal, vertical<CR><LF>
	DBG_InfoMsg("P3K_GetVideoWallMode\n");
	int mode = 0;
	int s32Ret = 0;
	ViewModeInfo_S info;
	char tmpparam[MAX_PARAM_LEN] = {0};
	s32Ret = EX_GetViewMode(&mode, &info);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetViewMode err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%d,%d", mode, info.hStyle, info.vStyle);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetWndBezel(char *reqparam, char *respParam, char *userdata)
{
	//#WND-BEZEL mode,out_index,h_value,v_value,h_offset, v_offset<CR>
	//~nn@WND-BEZEL mode,out_index,h_value,v_value,h_offset, v_offset<CR><LF>
	DBG_InfoMsg("P3K_SetWndBezel\n");
	int mode = 0;
	int s32Ret = 0;
	int out_index = 0;
	WndBezelinfo_S info;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	mode = atoi(str[0]);
	if ((mode < 0) || (mode > 13))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	if ((isnum(str[1]) == -1) || (isnum(str[2]) == -1) || (isnum(str[3]) == -1) || (isnum(str[4]) == -1) || (isnum(str[5]) == -1))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	out_index = atoi(str[1]);
	info.hValue = atoi(str[2]);
	info.vValue = atoi(str[3]);
	info.hOffset = atoi(str[4]);
	info.vOffset = atoi(str[5]);

	s32Ret = EX_SetWndBezelInfo(mode, out_index, &info);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetWndBezelInfo err\n");
		return -1;
	}

	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetWndBezel(char *reqparam, char *respParam, char *userdata)
{
	//#WND-BEZEL? <CR>
	//~nn@WND-BEZEL mode,out_index,h_value,v_value,h_offset, v_offset<CR><LF>
	DBG_InfoMsg("P3K_GetWndBezel\n");
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int mode = 0;
	int out_index = 0;
	WndBezelinfo_S info;

	s32Ret = EX_GetWndBezelInfo(&mode, &out_index, &info);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetWndBezelInfo err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%d,%d,%d,%d,%d", mode, out_index, info.hValue, info.vValue, info.hOffset, info.vOffset);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetVideoWallRotaion(char *reqparam, char *respParam, char *userdata)
{
	//#VIDEO-WALL-SETUP out_id,rotation<CR>
	//~nn@VIDEO-WALL-SETUP out_id,rotation<CR><LF>
	DBG_InfoMsg("P3K_SetVideoWallRotaion\n");
	int wallId;
	int s32Ret = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	VideoWallSetupInfo_S info;
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if ((isnum(str[0]) == -1) || (isnum(str[1]) == -1))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	wallId = atoi(str[0]);
	info.rotation = atoi(str[1]);
	s32Ret = EX_SetVideoWallSetupInfo(wallId, &info);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetVideoWallSetupInfo err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));

	return 0;
}

static int P3K_GetVideoWallRotaion(char *reqparam, char *respParam, char *userdata)
{
	//#VIDEO-WALL-SETUP? <CR>
	//~nn@VIDEO-WALL-SETUP out_id,rotation<CR><LF>
	DBG_InfoMsg("P3K_GetVideoWallRotaion\n");
	int wallId;
	int s32Ret = 0;
	VideoWallSetupInfo_S info;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetVideoWallSetupInfo(&wallId, &info);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetVideoWallSetupInfo err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%d", wallId, info.rotation);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_StartOverlay(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-START-OVERLAY profile_name,time_limit<CR>
	//~nn@KDS-START-OVERLAY profile_name,time_limit<CR><LF>
	DBG_InfoMsg("P3K_StartOverlay\n");
	int timeOver = 0;
	int s32Ret = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[1]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	timeOver = atoi(str[1]);
	s32Ret = EX_StartOverlay(str[0], timeOver);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_StartOverlay err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_StopOverlay(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-STOP-OVERLAY <CR>
	//~nn@KDS-STOP-OVERLAY <CR><LF>
	DBG_InfoMsg("P3K_StopOverlay\n");
	int s32Ret = 0;
	s32Ret = EX_StopOverlay();
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_StopOverlay err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_SetChannelId(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-DEFINE-CHANNEL ch_id<CR>
	//~nn@KDS-DEFINE-CHANNEL ch_id<CR><LF>
	DBG_InfoMsg("P3K_SetChannelId\n");
	int s32Ret = 0;
	int chn = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	chn = atoi(str[0]);
	if ((chn < 1) || (chn > 999))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	s32Ret = EX_SetEncoderAVChannelId(chn);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetEncoderAVChannelId err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetChannelId(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-DEFINE-CHANNEL? <CR>
	//~nn@KDS-DEFINE-CHANNEL ch_id<CR><LF>
	DBG_InfoMsg("P3K_GetChannelId\n");
	int chn = 0;
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	s32Ret = EX_GetEncoderAVChannelId(&chn);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetEncoderAVChannelId err\n");
		return -1;
	}
	sprintf(tmpparam, "%d", chn);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetChannleSelection(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-CHANNEL-SELECT  signal_type,ch_id<CR>
	//~nn@KDS-CHANNEL-SELECT  signal_type,ch_id<CR><LF>
	DBG_InfoMsg("P3K_SetChannleSelection\n");
	int chn = 0;
	int s32Ret = 0;
	int count = 0;
	ChSelect_S sSelect = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (count < 2)
	{
		ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	for (chn = 0; chn < count - 1; chn++)
	{
		sSelect.signal[chn] = P3K_CheckSignalType(str[chn]);
		if (sSelect.signal[chn] == -10)
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
	}
	if (isnum(str[count - 1]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	sSelect.ch_id = atoi(str[count - 1]);
	sSelect.i_signalnum = count - 1;
	s32Ret = EX_SetDecoderAVChannelId(&sSelect);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetDecoderAVChannelId err\n");
		return -1;
		// sprintf(reqparam+strlen(reqparam),",err_004");
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetChannleSelection(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_GetChannleSelection\n");
	int s32Ret = 0;
	int chn = 0;
	int count = 0;
	ChSelect_S sSelect = {0};
	char str1[16] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	// sSelect.signal = P3K_CheckSignalType(str[0]);
	sSelect.signal[0] = P3K_CheckSignalType(str[0]);
	if (sSelect.signal[0] == -10)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	sSelect.i_signalnum = count;
	s32Ret = EX_GetDecoderAVChannelId(&sSelect);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetDecoderAVChannelId err\n");
		return -1;
	}
	P3K_SignaleTypeToStr(sSelect.signal[sSelect.i_signalnum - 1], str1);
	sprintf(tmpparam + strlen(tmpparam), "%s,%d", str1, sSelect.ch_id);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetVideoOutMode(char *reqparam, char *respParam, char *userdata)
{
	//#IMAGE-PROP scaler_id,,video_mode<CR>
	//~nn@IMAGE-PROP scaler_id,video_mode��<CR><LF>
	DBG_InfoMsg("P3K_SetVideoOutMode\n");
	int s32Ret = 0;
	int scalerId = 0;
	int videoMode = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if ((isnum(str[0]) == -1) || (isnum(str[1]) == -1))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	scalerId = atoi(str[0]);
	videoMode = atoi(str[1]);

	s32Ret = EX_SetVideoImageStatus(scalerId, videoMode);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetVideoImageStatus err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetVideoOutMode(char *reqparam, char *respParam, char *userdata)
{
	//#IMAGE PROP? scaler_id<CR>:
	//~nn@IMAGE-PROP scaler_id,video_mode��<CR><LF>
	DBG_InfoMsg("P3K_GetVideoOutMode\n");
	int s32Ret = 0;
	int scalerId = 0;
	int videoMode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	scalerId = atoi(str[0]);

	s32Ret = EX_GetVideoImageStatus(scalerId, &videoMode);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetVideoImageStatus err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%d", scalerId, videoMode);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetVideoAction(char *reqparam, char *respParam, char *userdata)
{
	//#KDS ACTION kds_mode<CR>
	//~nn@KDS ACTION kds_mode<CR><LF>
	DBG_InfoMsg("P3K_SetVideoAction\n");
	int type = 0;
	int s32Ret = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	type = atoi(str[0]);

	s32Ret = EX_SetVideoCodecAction(type);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetVideoCodecAction err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetVideoAction(char *reqparam, char *respParam, char *userdata)
{
	//#KDS ACTION? <CR>
	//~nn@KDS ACTION kds_mode<CR><LF>
	DBG_InfoMsg("P3K_GetVideoAction\n");
	int type = 0;
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetVideoCodecAction(&type);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetVideoCodecAction err\n");
		return -1;
	}

	sprintf(tmpparam, "%d", type);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetVideoOutCS(char *reqparam, char *respParam, char *userdata)
{
	//#CS-CONVERT out_index,cs_mode<CR>
	//~nn@CS-CONVERT out_index,cs_mode<CR><LF>
	DBG_InfoMsg("P3K_SetVideoOutCS\n");
	int s32Ret = 0;
	int outId = 0;
	int csMode = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	outId = atoi(str[0]);
	if (outId < 1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	if (isnum(str[1]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	csMode = atoi(str[1]);
	s32Ret = EX_SetColorSpaceConvertMode(outId, csMode);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetColorSpaceConvertMode err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetVideoOutCS(char *reqparam, char *respParam, char *userdata)
{
	//#CS-CONVERT? out_index<CR>
	//~nn@CS-CONVERT out_index,cs_mode<CR><LF>
	DBG_InfoMsg("P3K_GetVideoOutCS\n");
	int outId;
	int csMode;
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	outId = atoi(str[0]);
	if (outId < 1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	s32Ret = EX_GetColorSpaceConvertMode(outId, &csMode);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetColorSpaceConvertMode err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%d", outId, csMode);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetVideoOutScaler(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-SCALE value<CR>
	//~nn@KDS-SCALE value,res_type<CR><LF>
	DBG_InfoMsg("P3K_SetVideoOutScaler\n");
	int s32Ret = 0;
	int mode = 0;
	int res = 0;
	int count = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if ((isnum(str[0]) == -1) || (isnum(str[1]) == -1))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	mode = atoi(str[0]);
	res = atoi(str[1]);
	s32Ret = EX_SetVideoImageScaleMode(mode, res);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetVideoImageScaleMode err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%d", mode, res);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetVideoOutScaler(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-SCALE? <CR>
	//~nn@KDS-SCALE value,res_type<CR><LF>
	DBG_InfoMsg("P3K_GetVideoOutScaler\n");
	int mode = 0;
	int s32Ret = 0;
	char res[64] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetVideoImageScaleMode(&mode, res);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetVideoImageScaleMode err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%s", mode, res);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetVideoReslotion(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-RESOL? io_mode,io_index,is_native<CR>
	//~nn@KDS-RESOL? io_mode,io_index,is_native,resolution<CR><LF>
	DBG_InfoMsg("P3K_GetVideoReslotion\n");
	int s32Ret = 0;
	int ioMode = 0;
	int ioIndex = 0;
	int nativeFlag = 0;
	int reslotion = 0;
	int count = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if ((isnum(str[0]) == -1) || (isnum(str[1]) == -1) || (isnum(str[2]) == -1))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	ioMode = atoi(str[0]);
	ioIndex = atoi(str[1]);
	nativeFlag = atoi(str[2]);
	s32Ret = EX_GetVideoViewReslotion(ioMode, ioIndex, nativeFlag, &reslotion);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetVideoViewReslotion err\n");
		return -1;
	}
	sprintf(tmpparam, "%s,%d", reqparam, reslotion);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetVideoFrameRate(char *reqparam, char *respParam, char *userdata)
{

	//#KDS-FR? <CR>
	//~nn@KDS-FR value<CR><LF>
	DBG_InfoMsg("P3K_GetVideoFrameRate\n");
	int fps = 0;
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetVideoFrameRate(&fps);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetVideoFrameRate err\n");
		return -1;
	}
	sprintf(tmpparam, "%d", fps);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetVideoBitRate(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-BR? <CR>
	//~nn@KDS-BR bitrate<CR><LF>
	DBG_InfoMsg("P3K_GetVideoBitRate\n");
	int bitRate = 0;
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetVideoBitRate(&bitRate);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetVideoBitRate err\n");
		return -1;
	}
	sprintf(tmpparam, "%d", bitRate);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SendCECMsg(char *reqparam, char *respParam, char *userdata)
{
	//#CEC-SND port_index,sn_id,cmd_name,cec_len,cec_command<CR>
	//~nn@CEC-SND port_index,sn_id,cmd_name,cec_mode<CR><LF>
	DBG_InfoMsg("P3K_SendCECMsg\n");
	CECMessageInfo_S cecMsg = {0};
	int ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if ((isnum(str[0]) == -1) || (isnum(str[1]) == -1) || (isnum(str[3]) == -1))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	cecMsg.portId = atoi(str[0]);
	cecMsg.serialNumb = atoi(str[1]);
	memcpy(cecMsg.cmdName, str[2], strlen(str[2]));
	cecMsg.hexByte = atoi(str[3]);
	if ((cecMsg.hexByte < 1) || (cecMsg.hexByte > 16))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	memcpy(cecMsg.cmdComent, str[4], strlen(str[4]));

	ret = EX_SendCECMsg(&cecMsg);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	sprintf(tmpparam, "%d,%d,%s,%d", cecMsg.portId, cecMsg.serialNumb, cecMsg.cmdName, ret);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_RecvCECNtfy(char *reqparam, char *respParam, char *userdata)
{
	//#CEC-NTFY <CR>
	//~nn@CEC-NTFY port_index,len,<cec_command��><CR><LF>
	DBG_InfoMsg("P3K_RecvCECNtfy\n");
	int s32Ret = 0;
	int portId;
	int hexByte;
	char cmdComment[CEC_MAX_CMD_COMENT_LEN + 1] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_RecvCECNtfy(&portId, &hexByte, cmdComment);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_RecvCECNtfy err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%d,%s", portId, hexByte, cmdComment);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetCECGWMode(char *reqparam, char *respParam, char *userdata)
{
	//#CEC-GW-PORT-ACTIVE gatewaymode <CR>P3K_GetCECGWMode
	//~nn@CEC-GW-PORT-ACTIVE gatewaymode <CR><LF>
	DBG_InfoMsg("P3K_SetCECGWMode\n");
	int s32Ret = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	int mode = 0;

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	mode = atoi(str[0]);

	s32Ret = EX_SetCECGateWayMode(mode);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetCECGateWayMode err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetCECGWMode(char *reqparam, char *respParam, char *userdata)
{
	//#CEC-GW-PORT-ACTIVE? gatewaymode <CR>
	//~nn@CEC-GW-PORT-ACTIVE gatewaymode <CR><LF>
	DBG_InfoMsg("P3K_GetCECGWMode\n");
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	int mode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	mode = EX_GetCECGateWayMode();
	if (mode < 0)
	{
		EX_ERR_MSG(mode, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	sprintf(tmpparam, "%d", mode);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SendIRMsg(char *reqparam, char *respParam, char *userdata)
{
	//#IR-SND ir_index,sn_id,cmd_name,repeat_amount,total_packages, package_id,<pronto command��><CR>
	//~nn@IR-SND ir_index,sn_id,cmd_name,ir_status<CR><LF>
	DBG_InfoMsg("P3K_SendIRMsg\n");
	IRMessageInfo_S irMsg;
	int ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[7][MAX_PARAM_LEN] = {0};
	memset(&irMsg, 0, sizeof(IRMessageInfo_S));
	count = P3K_PhraserIRParam(reqparam, strlen(reqparam), str, irMsg.cmdComent);
	if(count < 6)
	{
		ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return 0;
	}
	if ((isnum(str[0]) == -1) || (isnum(str[1]) == -1) || (isnum(str[3]) == -1) || (isnum(str[4]) == -1) || (isnum(str[5]) == -1))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	if (strlen(str[2]) > 24)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	memcpy(irMsg.cmdName, str[2], strlen(str[2]));
	irMsg.ir_index = atoi(str[0]);
	irMsg.irId = atoi(str[1]);
	irMsg.repeat = atoi(str[3]);
	irMsg.totalPacket = atoi(str[4]);
	irMsg.packId = atoi(str[5]);
	ret = EX_SendIRmessage(&irMsg);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	sprintf(tmpparam, "%d,%d,%s,%d", irMsg.ir_index, irMsg.irId, irMsg.cmdName, ret);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_StopIRMsg(char *reqparam, char *respParam, char *userdata)
{
	//#IR-STOP ir_index,sn_id,cmd_name<CR>
	//~nn@IR-STOP ir_index,sn_id,cmd_name,ir_status<CR><LF>
	DBG_InfoMsg("P3K_StopIRMsg\n");
	int ret = 0;
	int irId = 0;
	int serialId = 0;
	char command[IR_MAX_CMD_COMENT_LEN + 1] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if ((isnum(str[0]) == -1) || (isnum(str[1]) == -1))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	irId = atoi(str[0]);
	if (irId < 1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	serialId = atoi(str[1]);
	if (strlen(str[2]) > 24)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	memcpy(command, str[2], strlen(str[2]));

	ret = EX_SendIRStop(irId, serialId, command);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	sprintf(tmpparam, "%d,%d,%s,%d", irId, serialId, command, ret);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetXROUTEMatch(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_SetXROUTEMatch\n");
	//#X-ROUTE <direction_type1>. <port_type1>. <port_index1>. <signal_type1>. <index1>,<direction_type2>. <port_type2>. <port_index2>. <signal_type2>. <index2><CR>
	//~nn@X-ROUTE <direction_type1>. <port_type1>. <port_index1>. <signal_type1>. <index1>,<direction_type2>. <port_type2>. <port_index2>. <signal_type2>. <index2><CR><LF>
	PortInfo_S outInfo[12] = {0};
	PortInfo_S inInfo;
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	char str1[][MAX_PARAM_LEN] = {0};
	char str2[][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (count < 1)
	{
		if (count == -2)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	else if (count == 2)
	{
		int ret = 0;
		int sret = 0;
		sret = P3K_GetPortSInfo(str[0], &outInfo, 0, 5);
		if (sret < 0)
		{
			if (sret == -3)
				ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			else
				ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		ret = P3K_GetPortInfo(str[1], &inInfo, 5);
		if (ret < 0)
		{
			if (ret == -3)
				ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			else
				ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
	}
	else if (count > 2) //&& !memcmp(str[0],"[",strlen("[")))
	{
		int ret = 0;
		/*char * aTmp = "";
		memcpy(aTmp,str[count - 2],strlen(str[count - 2]));
		memset(str[count - 2],0,sizeof(str[count - 2]));*/
		memcpy(str[count - 2], str[count - 2], (strlen(str[count - 2]) - 1));
		int i = 0;
		int sret = 0;
		for (i = 0; i < count - 1; i++)
		{
			sret = P3K_GetPortSInfo(str[i], &outInfo, i, 5);
			if (sret < 0)
			{
				if (sret == -3)
					ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
				else
					ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
				strcpy(userdata, "error");
				return -1;
			}
		}
		ret = P3K_GetPortInfo(str[count - 1], &inInfo, 5);
		if (ret < 0)
		{
			if (ret == -3)
				ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			else
				ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
	}
	s32Ret = EX_SetRouteMatch(&outInfo, &inInfo, count - 1);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetRouteMatch err\n");
		return -1;
	}

	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetXROUTEMatch(char *reqparam, char *respParam, char *userdata)
{
	//#X-ROUTE? <direction_type1>. <port_type1>. <port_index1>. <signal_type1>. <index1><CR>
	//~nn@X-ROUTE <direction_type1>. <port_type1>. <port_index1>. <signal_type1>. <index1>,<direction_type2>. <port_type2>. <port_index2>. <signal_type2>. <index2><CR><LF>
	DBG_InfoMsg("P3K_GetXROUTEMatch\n");
	int s32Ret = 0;
	int ret = 0;
	PortInfo_S outInfo;
	PortInfo_S inInfo;
	char direc[32] = {0};
	char portFormat[32] = {0};
	char signal[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	char str1[][MAX_PARAM_LEN] = {0};
	char str2[][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);

	ret = P3K_GetPortInfo(str[0], &outInfo, 5);
	if (ret < 0)
	{
		if (ret == -3)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}

	s32Ret = EX_GetRouteMatch(&outInfo, &inInfo);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetRouteMatch err\n");
		return -1;
	}
	P3K_PortDirectionToStr(outInfo.direction, direc);
	P3K_PortFormatToStr(outInfo.portFormat, portFormat);
	P3K_SignaleTypeToStr(outInfo.signal, signal);
	sprintf(tmpparam, "%s.%s.%d.%s.%d", direc, portFormat, outInfo.portIndex, signal, outInfo.index);
	memset(direc, 0, sizeof(direc));
	memset(portFormat, 0, sizeof(portFormat));
	memset(signal, 0, sizeof(signal));

	P3K_PortDirectionToStr(inInfo.direction, direc);
	P3K_PortFormatToStr(inInfo.portFormat, portFormat);
	P3K_SignaleTypeToStr(inInfo.signal, signal);

	sprintf(tmpparam + strlen(tmpparam), ",%s.%s.%d.%s.%d", direc, portFormat, inInfo.portIndex, signal, inInfo.index);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetUartConf(char *reqparam, char *respParam, char *userdata)
{
	//#UART com_id,baud_rate,data_bits,parity,stop_bits_mode, serial_type,485_term<CR>
	//~nn@UART com_id,baud_rate,data_bits,parity,stop_bits_mode, serial_type,485_term<CR><LF>
	DBG_InfoMsg("P3K_SetUartConf\n");
	int s32Ret = 0;
	UartMessageInfo_S uartConf = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if(count < 6)
	{
		ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	if ((isnum(str[0]) == -1) || (isnum(str[1]) == -1) || (isnum(str[2]) == -1) || (isnum(str[3]) == -1))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	uartConf.comNumber = atoi(str[0]);
	uartConf.rate = atoi(str[1]);
	uartConf.bitWidth = atoi(str[2]);
	uartConf.parity = atoi(str[3]);
	if ((isnum(str[4]) == -1) || (isnum(str[5]) == -1))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	uartConf.stopBitsMode = atoi(str[4]);
	uartConf.serialType = atoi(str[5]);
	uartConf.term_485 = 0;
	if (count > 6)
	{
		if (isnum(str[6]) == -1)
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		uartConf.term_485 = atoi(str[6]);
	}

	s32Ret = EX_SetUartConf(&uartConf);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetUartConf err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int floatTostr(float in, char *dststr)
{
	char tmpstr[16] = {0};
	int len = 0;
	int i = 0;
	sprintf(tmpstr, "%f", in);
	len = strlen(tmpstr);
	if (tmpstr[len - 1] != '0')
	{
		memcpy(dststr, tmpstr, len);
		return 0;
	}
	for (i = len - 1; i > 0; i--)
	{
		if (tmpstr[i] != '0')
		{
			if (tmpstr[i] == '.')
			{
				memcpy(dststr, tmpstr, i);
			}
			else
			{
				memcpy(dststr, tmpstr, i + 1);
			}
			break;
		}
	}
	return 0;
}

static int P3K_GetUartConf(char *reqparam, char *respParam, char *userdata)
{
	//#UART? com_id<CR>
	//~nn@UART com_id,baud_rate,data_bits,parity,stop_bits_mode, serial_type,485_term<CR><LF>
	DBG_InfoMsg("P3K_GetUartConf\n");
	int s32Ret = 0;
	UartMessageInfo_S uartConf = {0};
	int comId = 0;
	char parity[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	char floatStr[16] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	comId = atoi(str[0]);

	s32Ret = EX_GetUartConf(comId, &uartConf);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetUartConf err\n");
		return -1;
	}
	//P3K_ParityToStr(uartConf.parity, parity);
	//floatTostr(uartConf.stopBitsMode, floatStr);
	if (uartConf.serialType == 0)
	{
		sprintf(tmpparam + strlen(tmpparam), "%d,%d,%d,%d,%d,%d",
				uartConf.comNumber, uartConf.rate, uartConf.bitWidth, uartConf.parity,
				uartConf.stopBitsMode, uartConf.serialType);
	}
	else
	{
		sprintf(tmpparam + strlen(tmpparam), "%d,%d,%d,%d,%d,%d,%d",
				uartConf.comNumber, uartConf.rate, uartConf.bitWidth, uartConf.parity,
				uartConf.stopBitsMode, uartConf.serialType, uartConf.term_485);
	}
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_AddComRoute(char *reqparam, char *respParam, char *userdata)
{
	//#COM-ROUTE-ADD com_id,port_type,port_id,eth_rep_en,timeout<CR>
	//~nn@COM ROUTE ADD com_id,port_type,port_id,eth_rep_en,timeout<CR><LF>
	DBG_InfoMsg("P3K_AddComRoute\n");
	int s32Ret = 0;
	ComRouteInfo_S routeInfo = {0};
	int comId = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if ((isnum(str[0]) == -1) || (isnum(str[1]) == -1) || (isnum(str[2]) == -1) || (isnum(str[3]) == -1) || (isnum(str[4]) == -1))
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	comId = atoi(str[0]);
	routeInfo.portType = atoi(str[1]);
	routeInfo.portNumber = atoi(str[2]);
	routeInfo.rePlay = atoi(str[3]);
	routeInfo.HeartTimeout = atoi(str[4]);

	s32Ret = EX_AddComRoute(&routeInfo, comId);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_AddComRoute err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_RemoveComRoute(char *reqparam, char *respParam, char *userdata)
{
	//#COM-ROUTE-REMOVE com_id<CR>
	//~nn@COM-ROUTE-REMOVE com_id<CR><LF>
	DBG_InfoMsg("P3K_RemoveComRoute\n");
	int s32Ret = 0;
	int comId = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	comId = atoi(str[0]);

	s32Ret = EX_RemoveComRoute(comId);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_AddComRoute err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetComRouteInfo(char *reqparam, char *respParam, char *userdata)
{
	//#COM-ROUTE? com_id<CR>
	//~nn@COM-ROUTE com_id,port_type,port_id,eth_rep_en,ping_val<CR><LF>
	int count = 0;
	int comId = 0;
	int ret = 0;
	ComRouteInfo_S info;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (0 == strcmp(str[0], "*"))
		comId = 1;
	else
	{
		if (isnum(str[0]) == -1)
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		comId = atoi(str[0]);
	}
	ret = EX_GetComRoute(comId, &info);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetComRoute err\n");
		return -1;
	}
	if (ret == 0) // enable
	{
		sprintf(respParam, "1,%d,%d,%d,%d", info.portType, info.portNumber, info.rePlay, info.HeartTimeout);
	}

	return 0;
}

static int P3K_GetEthTunnel(char *reqparam, char *respParam, char *userdata)
{
	//#ETH-TUNNEL? tunnel_id<CR>
	//~nn@ETH-TUNNEL tunnel_id,cmd_name,port_type,port_id,eth_ip,remote_port_id,eth_rep_en,connection_type<CR><LF>
	return 0;
}

static int P3K_SetUSBMode(char *reqparam, char *respParam, char *userdata)
{
	//#KVM-USB-CTRL usb_type<CR>
	//~nn@KVM-USB-CTRL usb_type<CR><LF>
	//   EX_GetUSBCtrl(int * type)
	int type = 0;
	int s32Ret = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	type = atoi(str[0]);
	s32Ret = EX_SetUSBCtrl(type);
	if (s32Ret < 0)
	{
		if (count == -2)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetUSBCtrl err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetMulticastStatus(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-MULTICAST? <CR>
	//~nn@KDS-ACTION group_ip,ttl<CR><LF>
	DBG_InfoMsg("P3K_GetMulticastStatus\n");
	int s32Ret = 0;
	char gIp[MAX_IP_ADDR_LEN] = {0};
	int ttl = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetMulticastInfo(gIp, &ttl);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetMulticastInfo err\n");
		return -1;
	}
	sprintf(tmpparam, "%s,%d", gIp, ttl);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetMacAddr(char *reqparam, char *respParam, char *userdata)
{
	//#FCT-MAC mac_address<CR>
	//~nn@FCT-MAC mac_address<CR><LF>
	DBG_InfoMsg("P3K_SetMacAddr\n");
	int s32Ret = 0;
	int id = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (count == 1)
	{
		char i_mac[24] = {0};
		strcpy(i_mac, str[0]);
		int isMac = is_valid_mac_addr(i_mac);
		if (isMac == 0)
		{
			s32Ret = EX_SetMacAddr(id, str[0]);
		}
		else
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
	}
	else
	{
		if (count == -2)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetMacAddr err\n");
		return -1;
		// sprintf(reqparam+strlen(reqparam),",err_004");
	}

	strcpy(tmpparam, reqparam);
	//    if(s32Ret == -1)
	//    {
	//       sprintf(reqparam+strlen(reqparam),",err_004");
	//   }
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetMacAddr(char *reqparam, char *respParam, char *userdata)
{
	//#NET-MAC? id<CR>
	//~nn@NET MAC id,mac_address<CR><LF>
	DBG_InfoMsg("P3K_GetMacAddr\n");
	int netId = 0;
	int s32Ret = 0;
	char mac[MAC_ADDR_LEN] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (count == -2)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	else if (count == -1)
	{
		netId = 0;
	}
	else if (count == 1)
	{
		if (isnum(str[0]) == -1)
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		netId = atoi(str[0]);
	}
	else
	{
		ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	s32Ret = EX_GetMacAddr(netId, mac);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetMacAddr err\n");
		return -1;
	}
	if (count == -1)
	{
		sprintf(tmpparam, "%s", mac);
	}
	else
	{
		sprintf(tmpparam, "%d,%s", netId, mac);
	}
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetDNSName(char *reqparam, char *respParam, char *userdata)
{
	//#NAME machine_name<CR>
	//~nn@NAME machine_name<CR><LF>
	DBG_InfoMsg("P3K_SetDNSName\n");
	int s32Ret = 0;
	int count = 0;
	int id = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	id = atoi(str[0]);
	if (strlen(str[1]) > 24 || strlen(str[1]) < 0 || str[1][0] == '-' || str[1][strlen(str[1]) - 1] == '-')
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		return -1;
	}
	s32Ret = EX_SetDNSName(id, str[1]);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetDNSName err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetDNSName(char *reqparam, char *respParam, char *userdata)
{
	//#NAME? <CR>
	//~nn@NAME machine_name<CR><LF>
	DBG_InfoMsg("P3K_GetDNSName\n");
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char name[MAX_DEV_NAME_LEN + 1] = {0};
	int id = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	int count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (count == -2)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	else if (count == -1)
	{
		id = 0;
	}
	else if (count == 1)
	{
		if (isnum(str[0]) == -1)
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		id = atoi(str[0]);
		if (id != 0 && id != 1)
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			return -1;
		}
	}
	else
	{
		ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	s32Ret = EX_GetDNSName(id, name);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetDNSName err\n");
		return -1;
	}
	if (count == -1)
	{
		sprintf(tmpparam, "%s", name);
	}
	else
	{
		sprintf(tmpparam, "%d,%s", id, name);
	}
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_ResetDNSName(char *reqparam, char *respParam, char *userdata)
{
	//#NAME-RST <CR>
	//~nn@NAME-RST ok<CR><LF>
	DBG_InfoMsg("P3K_ResetDNSName\n");
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char name[MAX_DEV_NAME_LEN + 1] = {0};

	s32Ret = EX_ResetDNSName(name);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_ResetDNSName err\n");
		return -1;
	}
	sprintf(tmpparam, "%s", "OK");
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetDHCPMode(char *reqparam, char *respParam, char *userdata)
{
	//#NET-DHCP netw_id,dhcp_state<CR>
	//~nn@NET-DHCP netw_id,dhcp_state<CR><LF>
	DBG_InfoMsg("P3K_SetDHCPMode\n");
	int s32Ret = 0;
	int nedId = 0;
	int dhcpMode = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (count == -2)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	else if (count == 1)
	{
		if (isnum(str[0]) == -1)
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		dhcpMode = atoi(str[0]);
	}
	else if (count == 2)
	{
		if (isnum(str[0]) == -1 || isnum(str[1]) == -1)
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		nedId = atoi(str[0]);
		dhcpMode = atoi(str[1]);
	}
	else
	{
		ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}

	s32Ret = EX_SetDHCPMode(nedId, dhcpMode);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetDHCPMode err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));

	return 0;
}

static int P3K_GetDHCPMode(char *reqparam, char *respParam, char *userdata)
{
	//#NET-DHCP? netw_id<CR>
	//~nn@NET-DHCP netw_id,dhcp_mode<CR><LF>
	DBG_InfoMsg("P3K_GetDHCPMode\n");
	int s32Ret = 0;
	int netId = 0;
	int mode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (count == 1)
	{
		if (isnum(str[0]) == -1)
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		netId = atoi(str[0]);
		if (netId < 0)
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
	}
	else if (count == -1)
	{
		netId = 0;
	}
	else
	{
		ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}

	s32Ret = EX_GetDHCPMode(netId, &mode);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetDHCPMode err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%d", netId, mode);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetNetConf(char *reqparam, char *respParam, char *userdata)
{
	//#NET-CONFIG netw_id,net_ip,net_mask,gateway,[dns1],[dns2]<CR>
	//~nn@NET-CONFIG netw_id,net_ip,net_mask,gateway<CR><LF>
	DBG_InfoMsg("P3K_SetNetConf\n");
	int s32Ret = 0;
	int netId;
	int ret = 0;
	NetWorkInfo_S netInfo = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (count != 4 && count != 6 && count != 5)
	{
		if (count == -2)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}

	if (strlen(str[0]) < 7)
	{
		// printf("----------------------------\n");
		if (isnum(str[0]) == -1)
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		netId = atoi(str[0]);
		if (netId < 0)
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		ret = checkisIp(str[1]);
		if (ret == -1)
		{
			DBG_ErrMsg("Invalid IP address\n");
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		memcpy(netInfo.ipAddr, str[1], strlen(str[1]));
		if (0 == IsSubnetMask(str[2]))
		{
			DBG_ErrMsg("Invalid mask address\n");
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		memcpy(netInfo.mask, str[2], strlen(str[2]));
		if(0 == strcasecmp(str[1],str[3]))
		{
			DBG_ErrMsg("IP address can not be same as gateway address\n");
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		int gatewayret = checkgateway(str[1], str[2], str[3]);
		if(gatewayret == -1)
		{
			DBG_ErrMsg("Gateway address is not in same subnet\n");
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		memcpy(netInfo.gateway, str[3], strlen(str[3]));
		if (count > 4)
		{
			memcpy(netInfo.dns1, str[4], strlen(str[4]));
		}
		if (count > 5)
		{
			memcpy(netInfo.dns1, str[5], strlen(str[5]));
		}
	}
	else
	{
		//	printf("--------------22--------------\n");
		ret = checkisIp(str[0]);
		if (ret == -1)
		{
			DBG_ErrMsg("Invalid IP address\n");
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		memcpy(netInfo.ipAddr, str[0], strlen(str[0]));
		if (0 == IsSubnetMask(str[1]))
		{
			DBG_ErrMsg("Invalid mask address\n");
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		memcpy(netInfo.mask, str[1], strlen(str[1]));
		if(0 == strcasecmp(str[0],str[2]))
		{
			DBG_ErrMsg("IP address can not be same as gateway address\n");
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		int gatewayret = checkgateway(str[0], str[1], str[2]);
		if(gatewayret == -1)
		{
			DBG_ErrMsg("Gateway address is not in same subnet\n");
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		memcpy(netInfo.gateway, str[2], strlen(str[2]));
		if (count > 3)
		{
			memcpy(netInfo.dns1, str[3], strlen(str[3]));
		}
		if (count > 4)
		{
			memcpy(netInfo.dns1, str[4], strlen(str[4]));
		}
	}

	s32Ret = EX_SetNetWorkConf(netId, &netInfo);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetNetWorkConf err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetNetConf(char *reqparam, char *respParam, char *userdata)
{
	//#NET-CONFIG? netw_id<CR>
	//~nn@NET-CONFIG netw_id,net_ip,net_mask,gateway<CR><LF>
	DBG_InfoMsg("P3K_GetNetConf\n");
	int s32Ret = 0;
	int netId = 0;
	NetWorkInfo_S netInfo = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	netId = atoi(str[0]);

	s32Ret = EX_GetNetWorkConf(netId, &netInfo);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetNetWorkConf err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%s,%s,%s", netId, netInfo.ipAddr, netInfo.mask, netInfo.gateway);
	/*if(strlen(netInfo.dns1) > 0)
	{
		strcat(tmpparam,",");
		strcat(tmpparam,netInfo.dns1);
	}
	if(strlen(netInfo.dns2) > 0)
	{
		strcat(tmpparam,",");
		strcat(tmpparam,netInfo.dns2);
	}*/
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetEthPort(char *reqparam, char *respParam, char *userdata)
{
	//#ETH-PORT port_type,port_id<CR>
	//~nn@ETH-PORT port_type,port_id<CR><LF>
	DBG_InfoMsg("P3K_SetEthPort\n");
	int s32Ret = 0;
	char ethType[16] = {0};
	int ethPort;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (count != 2)
	{
		if (count == -2)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	if (strcasecmp(str[0], "TCP") == 0)
	{
		memcpy(ethType, str[0], sizeof(ethType));
		if (isnum(str[1]) == -1)
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		ethPort = atoi(str[1]);
		if (ethPort < 5000 || ethPort > 5099)
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
	}
	else if (strcasecmp(str[0], "UDP") == 0)
	{
		memcpy(ethType, str[0], sizeof(ethType));
		if (isnum(str[1]) == -1)
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		ethPort = atoi(str[1]);
		if (ethPort < 50000 || ethPort > 50999)
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
	}
	else
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	s32Ret = EX_SetNetPort(ethType, ethPort);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetNetPort err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetEthPort(char *reqparam, char *respParam, char *userdata)
{
	//#ETH-PORT? port_type<CR>
	//~nn@ETH-PORT port_type,port_id<CR><LF>
	DBG_InfoMsg("P3K_GetEthPort\n");
	int s32Ret = 0;
	char ethType[16] = {0};
	int ethPort;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (count != 1)
	{
		if (count == -2)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	if ((strcasecmp(str[0], "TCP") == 0) || (strcasecmp(str[0], "UDP") == 0))
	{
		memcpy(ethType, str[0], sizeof(ethType));
	}
	else
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	s32Ret = EX_GetNetPort(ethType, &ethPort);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetNetPort err\n");
		return -1;
	}
	sprintf(tmpparam, "%s,%d", ethType, ethPort);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetSecurity(char *reqparam, char *respParam, char *userdata)
{
	//#SECUR security_state<CR>
	//~nn@SECUR security_state<CR><LF>
	DBG_InfoMsg("P3K_SetSecurity\n");
	int s32Ret = 0;
	int status = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	status = atoi(str[0]);

	s32Ret = EX_SetSecurityStatus(status);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetSecurityStatus err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetSecurity(char *reqparam, char *respParam, char *userdata)
{
	//#SECUR? <CR>
	//~nn@SECUR security_state<CR><LF>
	DBG_InfoMsg("P3K_GetSecurity\n");
	int s32Ret = 0;
	int status = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetSecurityStatus(&status);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetSecurityStatus err\n");
		return -1;
	}
	sprintf(respParam, "%d", status);
	return 0;
}

static int P3K_SetLogin(char *reqparam, char *respParam, char *userdata)
{
	//#LOGIN login_level,password<CR>
	//~nn@LOGIN login_level,password ok<CR><LF>
	// or
	//~nn@LOGIN err_004<CR><LF>
	DBG_InfoMsg("P3K_SetLogin\n");

	int ret = 0;
	char userName[32] = {0};
	char usrerPasswd[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	memcpy(userName, str[0], strlen(str[0]));
	memcpy(usrerPasswd, str[1], strlen(str[1]));

	sprintf(tmpparam, "%s,%s", reqparam, "OK");
	ret = EX_Login(userName, usrerPasswd);
	if (ret < 0)
	{
		ERR_MSG(ERR_UNAUTHORIZED_ACCESS, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_Login err\n");
		return -1;
	}

	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetLogin(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_GetLogin\n");
	int s32Ret = 0;
	char userName[32] = {0};
	char usrerPasswd[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetLoginInfo(userName, usrerPasswd);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetLoginInfo err\n");
		return -1;
	}
	sprintf(tmpparam, "%s", userName);

	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_LogOut(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_LogOut\n");
	int s32Ret = 0;
	//#LOGOUT<CR>
	//~nn@LOGOUT ok<CR><LF>
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_Logout();
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_Logout err\n");
		return -1;
	}
	sprintf(tmpparam, "%s", "OK");

	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetVersion(char *reqparam, char *respParam, char *userdata)
{
	//#VERSION? <CR>
	//~nn@VERSION firmware_version<CR><LF>
	DBG_InfoMsg("P3K_GetVersion\n");
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char ver[32] = {0};

	s32Ret = EX_GetDevVersion(ver);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetDevVersion err\n");
		return -1;
	}
	sprintf(tmpparam, "%s", ver);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetBootVersion(char *reqparam, char *respParam, char *userdata)
{
	//#BL-VERSION? <CR>
	//~nn@BL-VERSION bootload_version<CR><LF>
	DBG_InfoMsg("P3K_GetBootVersion\n");
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char ver[32] = {0};

	s32Ret = EX_GetBootVersion(ver);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetBootVersion err\n");
		return -1;
	}
	sprintf(tmpparam, "%s", ver);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetFCTMODEL(char *reqparam, char *respParam, char *userdata)
{
	//#FCT-MODEL model_name<CR>
	//~nn@FCT-MODEL model_name<CR><LF>
	DBG_InfoMsg("P3K_SetFCTMODEL\n");
	int s32Ret = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);

	if (strlen(str[0]) > 24)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	s32Ret = EX_SetDeviceNameModel(str[0]);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetDeviceNameModel err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetFCTMODEL(char *reqparam, char *respParam, char *userdata)
{
	//#MODEL? <CR>
	//~nn@MODEL model_name<CR><LF>
	DBG_InfoMsg("P3K_GetFCTMODEL\n");
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char mod[MAX_DEV_MOD_NAME_LEN + 1];

	s32Ret = EX_GetDeviceNameModel(mod);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetDeviceNameModel err\n");
		return -1;
	}
	sprintf(tmpparam, "%s", mod);

	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_Upgrade(char *reqparam, char *respParam, char *userdata)
{
	//#UPGRADE<CR>
	//~nn@UPGRADE ok<CR><LF>
	DBG_InfoMsg("P3K_Upgrade\n");
	int s32Ret = -1;
	char tmpparam[MAX_PARAM_LEN] = {0};

	struct stat buf;
	memset(&buf, 0, sizeof(buf));
	if ((0 == stat("/dev/shm/fw.tar.gz", &buf)) && (buf.st_size > 0))
	{
		s32Ret = EX_Upgrade();
		if (s32Ret < 0)
		{
			EX_ERR_MSG(s32Ret, reqparam, respParam);
			strcpy(userdata, "error");
			DBG_ErrMsg("EX_Upgrade err\n");
			return -1;
		}
		else if (s32Ret == 0)
		{
			sprintf(tmpparam, "%s", "OK");
		}
	}
	else
	{
		sprintf(tmpparam, "%s", "ERR 002");
	}

	memcpy(respParam, tmpparam, strlen(tmpparam));
	return s32Ret;
}

static int P3K_UpgradeStatus(char *reqparam, char *respParam, char *userdata)
{
	//#UPGRADE-STATUS<CR>
	//~nn@UPGRADE-STATUS ongoing,100,0<CR><LF>
	DBG_InfoMsg("P3K_UpgradeStatus\n");
	int s32Ret = 0;

	GetUpgradeStatus(respParam, MAX_PARAM_LEN);
	return 0;
}

static int P3K_SetSerailNum(char *reqparam, char *respParam, char *userdata)
{
	//#FCT-SN serial_num<CR>
	//~nn@FCT-SN serial_num<CR><LF>
	DBG_InfoMsg("P3K_SetSerailNum\n");
	int s32Ret = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);

	if (strlen(str[0]) > 14)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	int i = 0;
	for (i = 0; i < strlen(str[0]); i++)
	{
		if (isdigit(str[0][i]) == 0)
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
	}
	s32Ret = EX_SetSerialNumber(str[0]);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetSerialNumber err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetSerailNum(char *reqparam, char *respParam, char *userdata)
{
	//#SN? <CR>
	//~nn@SN serial_num<CR><LF>
	DBG_InfoMsg("P3K_GetSerailNum\n");
	int s32Ret = 0;
	char num[SERIAL_NUMBER_LEN] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetSerialNumber(num);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetSerialNumber err\n");
		return -1;
	}
	sprintf(tmpparam, "%s", num);
	memcpy(respParam, tmpparam, strlen(tmpparam));

	return 0;
}

static int P3K_SetLockFP(char *reqparam, char *respParam, char *userdata)
{
	//#LOCK-FP lock/unlock<CR>
	//~nn@LOCK-FP lock/unlock<CR><LF>
	DBG_InfoMsg("P3K_SetLockFP\n");
	int s32Ret = 0;

	int flag = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	flag = atoi(str[0]);

	s32Ret = EX_SetLockFP(flag);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetLockFP err\n");
		return -1;
	}

	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetLockFP(char *reqparam, char *respParam, char *userdata)
{
	//#LOCK-FP? <CR>
	//~nn@LOCK-FP lock/unlock<CR><LF>
	DBG_InfoMsg("P3K_GetLockFP\n");
	int s32Ret = 0;
	int flag = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetLockFP(&flag);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetLockFP err\n");
		return -1;
	}
	sprintf(tmpparam, "%d", flag);
	memcpy(respParam, tmpparam, strlen(tmpparam));

	return 0;
}

static int P3K_SetIDV(char *reqparam, char *respParam, char *userdata)
{
	//#IDV<CR>
	//~nn@IDV ok<CR><LF>
	DBG_InfoMsg("P3K_SetIDV\n");
	int s32Ret = 0;

	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_SetIDV();
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetIDV err\n");
		return -1;
	}
	sprintf(tmpparam, "%s", "OK");
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetStandByMode(char *reqparam, char *respParam, char *userdata)
{
	//#STANDBY value<CR>P3K_GetStandByMode
	//~nn@STANDBY value<CR><LF>
	DBG_InfoMsg("P3K_SetStandByMode\n");
	int s32Ret = 0;

	int mode = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	mode = atoi(str[0]);
	s32Ret = EX_SetStandbyMode(mode);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetStandbyMode err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetStandByMode(char *reqparam, char *respParam, char *userdata)
{
	//#STANDBY value<CR>P3K_GetStandByMode
	//~nn@STANDBY value<CR><LF>
	DBG_InfoMsg("P3K_GetStandByMode\n");
	int s32Ret = 0;

	int iValue = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	iValue = atoi(str[0]);
	s32Ret = EX_GetStandbyMode(&iValue);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetStandbyMode err\n");
		return -1;
	}
	sprintf(tmpparam, "%d", iValue);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_DoReset(char *reqparam, char *respParam, char *userdata)
{
	//#RESET<CR>
	//~nn@RESET ok<CR><LF>
	DBG_InfoMsg("P3K_DoReset\n");
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_DeviceReset();
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_DeviceReset err\n");
		return -1;
	}
	sprintf(tmpparam, "%s", "OK");
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_DoFactory(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_DoFactory\n");
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_FactoryRecovery();
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_FactoryRecovery err\n");
		return -1;
	}
	sprintf(tmpparam, "%s", "OK");

	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetBeaconEn(char *reqparam, char *respParam, char *userdata)
{
	//#BEACON-EN port_id,status,rate<CR>
	//~nn@BEACON-EN port_id,status,rate<CR><LF>
	DBG_InfoMsg("P3K_SetBeaconEn\n");
	int s32Ret = 0;
	int portNumber = 0;
	int status = 0;
	int sec = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1 || isnum(str[1]) == -1 || isnum(str[2]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	portNumber = atoi(str[0]);
	status = atoi(str[1]);
	sec = atoi(str[2]);
	s32Ret = EX_SetBeacon(portNumber, status, sec);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetBeacon err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetBeaconInfo(char *reqparam, char *respParam, char *userdata)
{
	//#BEACON INFO? port_id<CR>
	//~nn@BEACON INFO port_id,ip_string,udp_port,tcp_port, mac_address, model,name<CR><LF>
	DBG_InfoMsg("P3K_GetBeaconInfo\n");
	int s32Ret = 0;
	int portNumber;
	BeaconInfo_S beaconInfo = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	portNumber = atoi(str[0]);

	s32Ret = EX_GetBeaconInfo(portNumber, &beaconInfo);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetBeaconInfo err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%s,%d,%d,%s,%s,%s", portNumber, beaconInfo.ipAddr, beaconInfo.udpPort,
			beaconInfo.tcpPort, beaconInfo.macAddr, beaconInfo.deviceMod, beaconInfo.deviceName);

	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetBuildTime(char *reqparam, char *respParam, char *userdata)
{
	//#BUILD-DATE? <CR>
	//~nn@BUILD-DATE? date,time<CR><LF>
	DBG_InfoMsg("P3K_GetBuildTime\n");
	int s32Ret = 0;

	char date[32] = {0};
	char time[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetDevBuildDate(date, time);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetDevBuildDate err\n");
		return -1;
	}
	sprintf(tmpparam, "%s,%s", date, time);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetTime(char *reqparam, char *respParam, char *userdata)
{
	//#TIME day_of_week,date,data<CR>
	//~nn@TIME day_of_week,date,data<CR><LF>
	DBG_InfoMsg("P3K_SetTime\n");
	int s32Ret = 0;
	char pweekDay[16] = {0};
	char pdate[32] = {0};
	char ptime[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);

	if (strcasecmp(str[0], "SUN") == 0 || strcasecmp(str[0], "MON") == 0 || strcasecmp(str[0], "TUE") == 0 || strcasecmp(str[0], "WED") == 0 || strcasecmp(str[0], "THU") == 0 || strcasecmp(str[0], "FRI") == 0 || strcasecmp(str[0], "SAT") == 0)
	{
		sprintf(pdate, "%s", str[0]);
	}
	else
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	int dateret = is_valid_date(str[1]);
	if (dateret == -1)
	{
		ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	sprintf(pweekDay, "%s", str[1]);
	int timeret = is_valid_time(str[2]);
	if (timeret == -1)
	{
		ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	sprintf(ptime, "%s", str[2]);

	s32Ret = EX_SetTimeAndDate(pweekDay, pdate, ptime);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetTimeAndDate err\n");
		return -1;
	}
	sprintf(tmpparam, "%s,%s,%s", pweekDay, pdate, ptime);
	memcpy(respParam, tmpparam, strlen(tmpparam));

	return 0;
}

static int P3K_GetTime(char *reqparam, char *respParam, char *userdata)
{
	//#TIME? <CR>
	//~nn@TIME day_of_week,date,data<CR><LF>
	DBG_InfoMsg("P3K_GetTime\n");
	int s32Ret = 0;
	char weekDay[16] = {0};
	char date[32] = {0};
	char time[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetTimeAndDate(weekDay, date, time);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetTimeAndDate err\n");
		return -1;
	}
	sprintf(tmpparam, "%s,%s,%s", weekDay, date, time);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetTimeZero(char *reqparam, char *respParam, char *userdata)
{
	//#TIME-LOC utc_off,dst_state<CR>
	//~nn@TIME-LOC utc_off,dst_state<CR><LF>
	DBG_InfoMsg("P3K_SetTimeZero\n");
	int s32Ret = 0;
	int tz = 0;
	int timingMethod = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1 || isnum(str[1]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	tz = atoi(str[0]);
	timingMethod = atoi(str[1]);
	s32Ret = EX_SetTimeZero(tz, timingMethod);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetTimeZero err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetTimeZero(char *reqparam, char *respParam, char *userdata)
{
	//#TIME-LOC? <CR>
	//~nn@TIME-LOC utc_off,dst_state<CR><LF>
	DBG_InfoMsg("P3K_GetTimeZero\n");
	int s32Ret = 0;
	int tz = 0;
	int timingMethod = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetTimeZero(&tz, &timingMethod);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetTimeZero err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%d", tz, timingMethod);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetTimeServer(char *reqparam, char *respParam, char *userdata)
{
	//#TIME-SRV mode,time_server_ip,sync_hour<CR>
	//~nn@TIME SRV mode,time_server_ip,sync_hour,server_status<CR><LF>
	DBG_InfoMsg("P3K_SetTimeServer\n");

	TimeSyncConf_S syncInfo = {0};
	int status = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1 || isnum(str[2]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	syncInfo.enable = atoi(str[0]);
	int ipret = checkisIp(str[1]);
	if (ipret == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	memcpy(syncInfo.serverIp, str[1], sizeof(syncInfo.serverIp));
	syncInfo.syncInerval = atoi(str[2]);

	status = EX_SetTimeSyncInfo(&syncInfo);
	if (status < 0)
	{
		EX_ERR_MSG(status, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetTimeSyncInfo err\n");
		return -1;
	}
	sprintf(tmpparam, "%s,%d", reqparam, status);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetTimeServer(char *reqparam, char *respParam, char *userdata)
{
	//#TIME-SRV? <CR>
	//~nn@TIME-SRV mode,time_server_ip,sync_hour,server_status<CR><LF>
	DBG_InfoMsg("P3K_GetTimeServer\n");

	TimeSyncConf_S syncInfo = {0};
	int status = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	status = EX_GetTimeSyncInfo(&syncInfo);
	if (status < 0)
	{
		EX_ERR_MSG(status, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetTimeSyncInfo err\n");
		return -1;
	}

	sprintf(tmpparam, "%d,%s,%d,%d", syncInfo.enable, syncInfo.serverIp, syncInfo.syncInerval, status);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetSignalList(char *reqparam, char *respParam, char *userdata)
{
	//#SIGNALS-LIST? <CR>
	//~nn@SIGNALS-LIST [<direction_type>. <port_format>. <port_index>. <signal_type>. <index>,..,]<CR><LF>
	DBG_InfoMsg("P3K_GetSignalList\n");
	int ret = 0;
	char siglist[10][MAX_SIGNALE_LEN] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int i = 0;
	ret = EX_GetSignalList(&siglist[0], 10);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetSignalList err\n");
		return -1;
	}
	if (ret > 10)
	{
		DBG_WarnMsg("P3K_GetSignalList num=%d over 1o\n", ret);
		ret = 10;
	}
	for (i = 0; i < ret; i++)
	{
		strncat(tmpparam, siglist[i], MAX_SIGNALE_LEN);
		if (i < (ret - 1))
		{
			strncat(tmpparam, ",", 1);
		}
	}
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetPortList(char *reqparam, char *respParam, char *userdata)
{
	//#PORTS-LIST? <CR>
	//~nn@PORTS-LIST [<direction_type>. <port_format>. <port_index>,..,]<CR><LF>
	DBG_InfoMsg("P3K_GetPortList\n");
	int ret = 0;

	char portlist[16][MAX_PORT_LEN] = {0};
	int i = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	ret = EX_GetPortList(portlist, 16);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetPortList err\n");
		return -1;
	}
	if (ret > 10)
	{
		DBG_WarnMsg("P3K_GetPortList num=%d over 1o\n", ret);
		ret = 10;
	}
	for (i = 0; i < ret; i++)
	{
		strncat(tmpparam, portlist[i], MAX_PORT_LEN);
		if (i < (ret - 1))
		{
			strncat(tmpparam, ",", 1);
		}
	}
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetActiveCli(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-ACTIVE-CLNT? <CR>
	//~nn@KDS-ACTIVE-CLNT value<CR><LF>
	DBG_InfoMsg("P3K_GetActiveCli\n");

	char tmpparam[MAX_PARAM_LEN] = {0};
	int ret = 0;
	ret = EX_GetActiveCliNUm();
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetActiveCliNUm err\n");
		return -1;
	}
	sprintf(tmpparam, "%d", ret);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetLogAction(char *reqparam, char *respParam, char *userdata)
{
	//#LOG-ACTION action,period<CR>
	//~nn@LOG-ACTION action,period<CR><LF>
	DBG_InfoMsg("P3K_SetLogAction\n");
	int s32Ret = 0;

	int action = 0;
	int period = 0;

	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1 || isnum(str[1]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	action = atoi(str[0]);
	period = atoi(str[1]);

	s32Ret = EX_SetLogEvent(action, period);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetLogEvent err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));

	return 0;
}

static int P3K_GetLogAction(char *reqparam, char *respParam, char *userdata)
{
	//#LOG-ACTION?<CR>
	//~nn@LOG-ACTION action,period<CR><LF>
	DBG_InfoMsg("P3K_GetLogAction\n");
	int ret = 0;
	int action = 0;
	int period = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	ret = EX_GetLogEvent(&action, &period);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetLogEvent err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%d", action, period);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetLogTail(char *reqparam, char *respParam, char *userdata)
{
	//#LOG-TAIL? line_num<CR>
	//~nn@LOG-TAILnn<CR><LF>
	// Line content #1<CR><LF>
	// Line content #2<CR><LF>
	DBG_InfoMsg("P3K_GetLogTail\n");
	int ret = 0;
	int number = 10;
	char log[MAX_USR_STR_LEN + 1] = {0};

	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	int i = 0;
	char pestr[10] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (count > 0)
	{
		if (isnum(str[0]) == -1)
		{
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
		number = atoi(str[0]);
	}
	char tmplog[20][MAX_ONELOG_LEN] = {0};

	ret = EX_GetLogTail(number, tmplog);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetLogTail err\n");
		return -1;
	}
	for (i = 0; i < ret; i++)
	{
		memset(pestr, 0, sizeof(pestr));
		strncat(log, tmplog[i], MAX_ONELOG_LEN);
		sprintf(pestr, "#%d\r\n", i);
		strcat(log, pestr);
	}
	memcpy(userdata, log, strlen(log));
	sprintf(tmpparam, "%d", number);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetAudioInfo(char *param, AudioInfo_S *info, int num)
{
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;

	count = P3K_PhraserWithSeparator('.', param, strlen(param), str);
	if (count != num)
		return -3;

	info->direction = P3K_CheckPortDirection(str[0]);
	if (info->direction == -10)
	{
		return -3;
	}
	info->portFormat = P3K_CheckPortFormat(str[1]);
	if (info->portFormat == -10)
	{
		return -3;
	}
	if (isnum(str[2]) == -1)
	{
		return -3;
	}
	info->portIndex = atoi(str[2]);
	if (count > 3)
	{
		info->signal = P3K_CheckSignalType(str[3]);
		if (info->signal == -10)
		{
			return -3;
		}
		// info->index = atoi(str[4]);
	}
	return 0;
}

static int P3K_SetAudAnalogDir(char *reqparam, char *respParam, char *userdata)
{
	//#PORT-DIRECTION <direction_type>. <port_format>. <port_index>. <signal_type> , direction<CR>
	//~nn@PORT-DIRECTION <direction_type>. <port_format>. <port_index>. <signal_type>  , direction<CR><LF>
	DBG_InfoMsg("P3K_SetAudAnalogDir\n");
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char gain[MAX_PARAM_LEN] = {0};
	AudioInfo_S tmpInfo = {0};
	memset(&tmpInfo, 0, sizeof(AudioInfo_S));
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (strcasecmp(str[1], "IN") == 0 || strcasecmp(str[1], "OUT") == 0)
	{
		memcpy(gain, str[1], strlen(str[1]));
	}
	else
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	int inforet = P3K_GetAudioInfo(str[0], &tmpInfo, 4);
	if (inforet < 0)
	{
		if (inforet == -3)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	s32Ret = EX_SetAudAnalogGainDir(&tmpInfo, gain);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetAudAnalogGainDir err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetAudAnalogDir(char *reqparam, char *respParam, char *userdata)
{
	//#PORT-DIRECTION? <direction_type>. <port_format>. <port_index>. <signal_type> , direction<CR>
	//~nn@PORT-DIRECTION <direction_type>. <port_format>. <port_index>. <signal_type>  , direction<CR><LF>
	DBG_InfoMsg("P3K_GetAudAnalogDir\n");
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	PortInfo_S tmpInfo = {0};
	char gain[MAX_PARAM_LEN] = {0};
	// both.analog.1.audio
	int inforet = P3K_GetAudioInfo(reqparam, &tmpInfo, 4);
	if (inforet == -1)
	{
		ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	s32Ret = EX_GetAudAnalogGainDir(&tmpInfo, gain);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetAudAnalogGainDir err\n");
		return -1;
	}
	sprintf(tmpparam, "%s,%s", reqparam, gain);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetAudioSInfo(char *param, AudioInfo_S *info, int num, int paramnum)
{
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int i = 0;

	int count = P3K_PhraserWithSeparator('.', param, strlen(param), str);
	if (count != paramnum)
		return -3;

	info[num].direction = P3K_CheckPortDirection(str[0]);
	if (info[num].direction == -10)
	{
		return -3;
	}
	info[num].portFormat = P3K_CheckPortFormat(str[1]);
	if (info[num].portFormat == -10)
	{
		return -3;
	}
	if (isnum(str[2]) == -1)
	{
		return -3;
	}
	info[num].portIndex = atoi(str[2]);
	if (count > 3)
	{
		info[num].signal = P3K_CheckSignalType(str[3]);
		if (info[num].signal == -10)
		{
			return -3;
		}
		// info->index = atoi(str[4]);
	}

	return 0;
}

static int P3K_SetAutoSwitchPriority(char *reqparam, char *respParam, char *userdata)
{
	//#X-PRIORITY <direction_type>. <port_format>. <port_index> .<signal_type> , [<direction_type>. <port_format>. <port_index>.<signal_type> ,...]<CR>
	//~nn@X-PRIORITY <direction_type>. <port_format>. <port_index>.<signal_type>  , [<direction_type>. <port_format>. <port_index>.<signal_type> ,...]<CR><LF>
	DBG_InfoMsg("P3K_SetAutoSwitchPriority\n");
	int count = 0;
	int i = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	int s32Ret = 0;
	AudioInfo_S tmpparam[MAX_PARAM_COUNT] = {0};
	AudioInfo_S tmpInfo = {0};
	memset(&tmpInfo, 0, sizeof(AudioInfo_S));
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (count < 2)
	{
		if (count == -2)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	int inforet = 0;
	for (i = 0; i < count; i++)
	{
		inforet = P3K_GetAudioSInfo(str[i], &tmpparam, i, 4);
		if (inforet < 0)
		{
			if (inforet == -3)
				ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
			else
				ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
			strcpy(userdata, "error");
			return -1;
		}
	}
	s32Ret = EX_SetAutoSwitchPriority(&tmpInfo, &tmpparam, count);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetAutoSwitchPriority err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetAutoSwitchPriority(char *reqparam, char *respParam, char *userdata)
{
	//#X-PRIORITY? <direction_type>. <port_format>. <port_index>.<signal_type>
	//~nn@X-PRIORITY <direction_type>. <port_format>. <port_index>.<signal_type>  , [<direction_type>. <port_format>. <port_index>.<signal_type> ,...]<CR><LF>
	DBG_InfoMsg("P3K_GetAutoSwitchPriority\n");
	int count = 0;
	int num = 0;
	int ret = 0;
	int s32Ret = 0;
	char aStr[4] = ",[";
	char aStr1[3] = "]";
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	char aStr2[MAX_PARAM_LEN] = {0};
	AudioInfo_S tmp[MAX_PARAM_COUNT] = {0};
	AudioInfo_S tmpInfo = {0};

	memset(&tmpInfo, 0, sizeof(AudioInfo_S));
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (count != 1)
	{
		if (count == -2)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	int inforet = 0;
	inforet = P3K_GetAudioSInfo(str[0], &tmp, 0, 4);
	if (inforet < 0)
	{
		if (inforet == -3)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	ret = EX_GetAutoSwitchPriority(&tmp, count);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetAutoSwitchPriority err\n");
		return -1;
	}
	strncat(tmpparam, str[0], strlen(str[0]));
	strncat(tmpparam, aStr, strlen(aStr));
	for (num = 1; num < ret; num++)
	{
		char dir[16] = {0};
		char port[16] = {0};
		char signal[16] = {0};

		P3K_PortDirectionToStr(tmp[num].direction, dir);
		P3K_PortFormatToStr(tmp[num].portFormat, port);
		P3K_SignaleTypeToStr(tmp[num].signal, signal);
		sprintf(aStr2, "%s.%s.%d.%s,", dir, port, tmp[num].portIndex, signal);
		strncat(tmpparam, aStr2, strlen(aStr2));

		memset(aStr2, 0, sizeof(aStr2));
		// memset(tmp,0,sizeof(tmp));
	}

	if (num == ret)
	{
		char dir[16] = {0};
		char port[16] = {0};
		char signal[16] = {0};

		P3K_PortDirectionToStr(tmp[num].direction, dir);
		P3K_PortFormatToStr(tmp[num].portFormat, port);
		P3K_SignaleTypeToStr(tmp[num].signal, signal);
		sprintf(aStr2, "%s.%s.%d.%s]", dir, port, tmp[num].portIndex, signal);
		strncat(tmpparam, aStr2, strlen(aStr2));
		memset(aStr2, 0, sizeof(aStr2));
	}
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetEDIDMode(char *reqparam, char *respParam, char *userdata)
{
	//#EDID-MODE   Input_id, Mode, Index<CR>
	//~nn@#EDID-MODE   Input_id, Mode, Index<CR><LF>
	DBG_InfoMsg("P3K_SetEDIDMode\n");
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	EdidInfo_S tmpInfo = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (count < 2)
	{
		ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	if (isnum(str[0]) == -1 || isnum(str[2]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	tmpInfo.input_id = atoi(str[0]);
	tmpInfo.mode = P3K_CheckEdidMode(str[1]);
	if (tmpInfo.mode == -10)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	if (count > 2)
	{
		tmpInfo.index = atoi(str[2]);
	}
	else
		tmpInfo.index = 0;
	s32Ret = EX_SetEDIDMode(&tmpInfo);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetEDIDMode err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetEDIDMode(char *reqparam, char *respParam, char *userdata)
{
	//#EDID-MODE?  Input_id<CR>
	//~nn@#EDID-MODE   Input_id, Mode, Index<CR><LF>
	DBG_InfoMsg("P3K_GetEDIDMode\n");
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	EdidInfo_S tmpInfo = {0};
	int cmdID = 0;
	char parity[32] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	cmdID = atoi(str[0]);

	s32Ret = EX_GetEDIDMode(cmdID, &tmpInfo);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetEDIDMode err\n");
		return -1;
	}
	P3K_ModeToStr(tmpInfo.mode, parity);
	if (tmpInfo.mode == CUSTOM)
		sprintf(tmpparam + strlen(tmpparam), "%d,%s,%d", tmpInfo.input_id, parity, tmpInfo.index);
	else
		sprintf(tmpparam + strlen(tmpparam), "%d,%s", tmpInfo.input_id, parity);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetEDIDList(char *reqparam, char *respParam, char *userdata)
{
	//#EDID-LIST? <CR>
	//~nn@#EDID-LIST [0,��DEFAULT��],...<CR><LF>
	DBG_InfoMsg("P3K_GetEDIDList\n");
	int ret = 0;
	int i = 0;
	char edidlist[8][MAX_EDID_LEN] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	ret = EX_GetEdidList(edidlist, 8);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetEdidList err\n");
		return -1;
	}
	if (ret > 10)
	{
		DBG_WarnMsg("P3K_GetEDIDList num=%d over 10\n", ret);
		ret = 10;
	}
	for (i = 0; i < ret; i++)
	{
		strncat(tmpparam, edidlist[i], MAX_EDID_LEN);
		if (i < (ret - 1))
		{
			strncat(tmpparam, ",", 1);
		}
	}
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_AddEDID(char *reqparam, char *respParam, char *userdata)
{
	//#EDID-ADD Index,"Name" <CR>
	//~nn@#EDID-ADD Index,"Name"<CR><LF>
	DBG_InfoMsg("P3K_GetEDIDList\n");
	int cmdID = 0;
	int s32Ret = 0;
	int count = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	EdidName_S edidname = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	edidname.index = atoi(str[0]);
	if (strlen(str[1]) > 24 || strlen(str[1]) < 0 || str[1][0] == '-' || str[1][strlen(str[1])] == '-')
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		return -1;
	}
	memcpy(edidname.name, str[1], strlen(str[1]));

	s32Ret = EX_AddEDID(&edidname);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_AddEDID err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_RemoveEDID(char *reqparam, char *respParam, char *userdata)
{
	//#EDID-ADD Index <CR>
	//~nn@#EDID-RM Index<CR><LF>
	DBG_InfoMsg("P3K_RemoveEDID\n");
	int comID = 0;
	int s32Ret = 0;
	int count = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	EdidName_S edidname = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	comID = atoi(str[0]);

	s32Ret = EX_RemoveEDID(comID);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_RemoveEDID err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

// Activate specific EDID
static int P3K_SetActiveEDID(char *reqparam, char *respParam, char *userdata)
{
	//#EDID-ACTIVE  Input_id, Index<CR>
	//~nn@EDID-ACTIVE  Input_id, Index<CR><LF>
	DBG_InfoMsg("P3K_SetActiveEDID\n");
	int input_ID = 0;
	int index_ID = 0;
	int s32Ret = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1 || isnum(str[1]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	input_ID = atoi(str[0]);
	index_ID = atoi(str[1]);

	s32Ret = EX_SetActiveEDID(input_ID, index_ID);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetActiveEDID err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

// Get Current Activate EDID
static int P3K_GetActiveEDID(char *reqparam, char *respParam, char *userdata)
{
	//#EDID-ACTIVE?  Input_id<CR>
	//~nn@EDID-ACTIVE  Input_id, Index<CR><LF>
	DBG_InfoMsg("P3K_SetActiveEDID\n");
	int input_ID = 0;
	int ret = 0;
	int count = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	input_ID = atoi(str[0]);

	ret = EX_GetActiveEDID(input_ID);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetActiveEDID err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%d", input_ID, ret);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

// Set MAC on Net device to be EDID source
static int P3K_SetEDIDNetSrc(char *reqparam, char *respParam, char *userdata)
{
	//#EDID-NET-SRC input_id, src_ip<CR>
	//~nn@EDID-NET-SRC input_id, src_mac<CR><LF>
	DBG_InfoMsg("P3K_SetEDIDNetSrc\n");
	int s32Ret = 0;
	int input_id = 0;
	int id = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	input_id = atoi(str[0]);
	int macret = checkisIp(str[1]);
	if (macret == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	s32Ret = EX_SetEDIDNetSrc(input_id, str[1]);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetEDIDNetSrc err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

// Get MAC on Net device to be EDID source
static int P3K_GetEDIDNetSrc(char *reqparam, char *respParam, char *userdata) // X
{
	//#EDID-NET-SRC input_id, src_mac<CR>
	//~nn@EDID-NET-SRC input_id, src_mac<CR><LF>
	DBG_InfoMsg("P3K_GetEDIDNetSrc\n");
	int netid = 0;
	int count = 0;
	int s32Ret = 0;
	char mac[MAC_ADDR_LEN] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	netid = atoi(str[0]);
	s32Ret = EX_GetEDIDNetSrc(netid, mac);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetEDIDNetSrc err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%s", netid, mac);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

// Get Control Gateway Messages Counter from certain period
static int P3K_GetRecvMsgNum(char *reqparam, char *respParam, char *userdata)
{
	//#GTW-MSG-NUM? mmessage_type,data<CR>
	//~nn@GTW-MSG-NUM message_type,date,counter<CR><LF>
	DBG_InfoMsg("P3K_GetRecvMsgNum\n");
	int count = 0;
	int ret = 0;
	int msg_type = 0;
	char pdate[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	msg_type = atoi(str[0]);
	int dateret = is_valid_date(str[1]);
	if (dateret == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	memcpy(pdate, str[1], strlen(str[1]));

	ret = EX_GetRecvMsgNum(msg_type, pdate);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetRecvMsgNum err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%s,%d", msg_type, pdate, ret);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

// Get SEND Control Gateway Messages Counter from certain period
static int P3K_GetSendMsgNum(char *reqparam, char *respParam, char *userdata)
{
	//#GTW-MSG-NUM-SEND? mmessage_type,data<CR>
	//~nn@GTW-MSG-NUM-SEND message_type,date,counter<CR><LF>
	DBG_InfoMsg("P3K_GetSendMsgNum\n");
	int count = 0;
	int ret = 0;
	int msg_type = 0;
	char pdate[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	msg_type = atoi(str[0]);
	int dateret = is_valid_date(str[1]);
	if (dateret == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	memcpy(pdate, str[1], strlen(str[1]));

	ret = EX_GetSendMsgNum(msg_type, pdate);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetSendMsgNum err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%s,%d", msg_type, pdate, ret);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetVidOutRatio(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-RATIO? <CR>
	//~nn@KDS-RATIO value<CR><LF>
	DBG_InfoMsg("P3K_GetVidOutRatio\n");
	int s32Ret = 0;
	char value[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetVidOutRatio(value);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetVidOutRatio err\n");
		return -1;
	}
	sprintf(tmpparam, "%s", value);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

// Set Encoder AV channel Name.
static int P3K_SetChannelName(char *reqparam, char *respParam, char *userdata) // P3K_SetDanteName
{
	//#KDS-DEFINE-Name xxxx<CR>
	//~nn@KDS-DEFINE-NAME xxxx<CR><LF>
	DBG_InfoMsg("P3K_SetChannelName\n");
	int s32Ret = 0;
	int count = 0;
	char aName[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (strlen(str[0]) > 32)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	memcpy(aName, str[0], strlen(str[0]));

	s32Ret = EX_SetChannelName(aName);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetChannelName err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetChannelName(char *reqparam, char *respParam, char *userdata) //
{
	//#KDS-DEFINE-Name?<CR>
	//~nn@KDS-DEFINE-NAME xxxx<CR><LF>
	DBG_InfoMsg("P3K_GetChannelName\n");
	int s32Ret = 0;
	char aName[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetChannelName(aName);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetChannelName err\n");
		return -1;
	}
	sprintf(tmpparam, "%s", aName);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetMuteInfo(char *param, MuteInfo_S *info, int num)
{
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int i = 0;

	int count = P3K_PhraserWithSeparator('.', param, strlen(param), str);
	if (count != 5)
	{
		printf("-1\n");
		return -3;
	}
	if (isnum(str[2]) == -1 || isnum(str[4]) == -1)
	{
		printf("1\n");
		return -3;
	}
	info->direction = P3K_CheckPortDirection(str[0]);
	if (info->direction == -10)
	{
		printf("2\n");
		return -3;
	}
	info->portFormat = P3K_CheckPortFormat(str[1]);
	if (info->portFormat == -10)
	{
		printf("3\n");
		return -3;
	}
	info->portIndex = atoi(str[2]);
	info->signal = P3K_CheckSignalType(str[3]);
	if (info->signal == -10)
	{
		printf("4\n");
		return -3;
	}
	info->index = atoi(str[4]);

	return 0;
}

// Set action to mute/unmute by encoder/decoder
static int P3K_SetAudMute(char *reqparam, char *respParam, char *userdata)
{
	//#X-MUTE <direction_type>. <port_format>. <port_index> .<signal_type>.<index>,state<CR>
	//~nn@ X-MUTE <direction_type>. <port_format>. <port_index> .<signal_type>.<index>,state <CR><LF>
	DBG_InfoMsg("P3K_SetVidMute\n");
	int s32Ret = 0;
	int count = 0;
	MuteInfo_S sMute = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	int inforet = P3K_SetMuteInfo(str[0], &sMute, 5);
	if (inforet < 0)
	{
		if (inforet == -3){
			DBG_ErrMsg("inforet err\n");
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		}
		else
			ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	sMute.state =  P3K_CheckStateType(str[1]);
	if(sMute.state == -10)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	s32Ret = EX_SetVidMute(&sMute);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetVidMute err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetAudMute(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-AUDIO-MUTE?<CR>
	//~nn@KDS-AUDIO-MUTE mute_mode<CR><LF>
	DBG_InfoMsg("P3K_GetChannelName\n");
	int ret = 0;
	int count = 0;
	char aState[6] = {0};
	MuteInfo_S sMute = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	printf("0\n");
	int inforet = P3K_SetMuteInfo(str, &sMute, 5);
	if (inforet < 0)
	{
		if (inforet == -3)
			ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	ret = EX_GetVidMute(&sMute);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetVidMute err\n");
		return -1;
	}
	P3K_StateToStr(sMute.state, aState);
	sprintf(tmpparam, "%s,%s", reqparam, aState);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetConnectionList(char *reqparam, char *respParam, char *userdata)
{
	//#CONNECTION-LIST? <CR>
	//~nn@CONNECTION-LIST [ip_string,port_type,client_port,device_port],...<CR><LF>
	DBG_InfoMsg("P3K_GetConnectionList\n");
	int ret = 0;
	char connectionlist[64][MAX_SIGNALE_LEN] = {0};
	char tmpparam[4096] = {0};
	int i = 0;
	ret = EX_GetConnectionList(&connectionlist[0], 10);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetConnectionList err\n");
		return -1;
	}
	for (i = 0; i < ret; i++)
	{
		strncat(tmpparam, connectionlist[i], MAX_SIGNALE_LEN);
		if (i < (ret - 1))
		{
			strncat(tmpparam, ",", 1);
		}
	}
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetStandbyVersion(char *reqparam, char *respParam, char *userdata)
{
	//#STANDBY-VERSION? ? <CR>
	//~nn@STANDBY-VERSION?  standby_version<CR><LF>
	DBG_InfoMsg("P3K_GetStandbyVersion\n");
	int s32Ret = 0;
	char aVersion[24] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetStandbyVersion(aVersion);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetStandbyVersion err\n");
		return -1;
	}
	sprintf(tmpparam, "%s", aVersion);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

// Get hardware version number.
static int P3K_GetHWVersion(char *reqparam, char *respParam, char *userdata)
{
	//#HW-VERSION? <CR>
	//~nn@HW-VERSION hardware_version<CR><LF>
	DBG_InfoMsg("P3K_GetHWVersion\n");
	int s32Ret = 0;
	char aVersion[24] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetHWVersion(aVersion);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetHWVersion err\n");
		return -1;
	}
	sprintf(tmpparam, "%s", aVersion);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

// Get Device Status.
static int P3K_GetDevStatus(char *reqparam, char *respParam, char *userdata)
{
	//#DEV-STATUS? <CR>
	//~nn@DEV-STATUS 0<CR><LF>
	DBG_InfoMsg("P3K_GetDevStatus\n");
	int ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	ret = EX_GetDevStatus();
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetDevStatus err\n");
		return -1;
	}
	sprintf(tmpparam, "%d", ret);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetHWTemp(char *reqparam, char *respParam, char *userdata)
{
	//#HW-TEMP? region_id <CR>
	//~nn@HW?TEMP region_id, temperature<CR><LF>
	DBG_InfoMsg("P3K_GetHWTemp\n");
	int ret = 0;
	int region_id = 0;
	int count = 0;
	int iMode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1 || isnum(str[1]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	region_id = atoi(str[0]);
	iMode = atoi(str[1]);
	ret = EX_GetHWTemp(region_id, iMode);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetHWTemp err\n");
		return -1;
	}

	if(iMode == 0)
		sprintf(tmpparam, "%d,%dC", region_id, ret);
	else
		sprintf(tmpparam, "%d,%dF", region_id, ret);

	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

// Firmware Version Last Upgrade Date/Time.
static int P3K_GetUPGTime(char *reqparam, char *respParam, char *userdata)
{
	//#UPG-TIME? <CR>
	//~nn@UPG-TIME date,data<CR><LF>
	DBG_InfoMsg("P3K_GetUPGTime\n");
	int s32Ret = 0;
	char aDay[16] = {0};
	char aTime[16] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetUPGTime(aDay, aTime);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetUPGTime err\n");
		return -1;
	}
	sprintf(tmpparam, "%s,%s", aDay, aTime);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetAudOutput(char *reqparam, char *respParam, char *userdata)
{
	//#KDS_AUD_OUTPUT  [ list of Audio output]<CR>
	//~nn@KDS-AUD_OUTPUT [ list of Audio output]<CR><LF>
	DBG_InfoMsg("P3K_SetAudOutput\n");
	int s32Ret = 0;
	int count = 0;
	int mute_mode = 0;
	int aud[MAX_PARAM_COUNT] = {0};
	char aStr1[64] = {0};
	char aStr2[64] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	sscanf(reqparam, "[%s", aStr1);
	memcpy(aStr2, aStr1, (strlen(aStr1) - 1));
	count = P3K_PhraserParam(aStr2, strlen(aStr2), str);
	if(count < 0)
	{
		ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	s32Ret = EX_SetVidOutput(str, count);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetVidOutput err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

// Get Decoder Audio output list.
static int P3K_GetAudOutput(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-AUD_OUTPUTE ? <CR>
	//~nn@KDS-AUD_OUTPUT [ list of Audio output]<CR><LF>
	DBG_InfoMsg("P3K_GetAudOutput\n");
	int s32Ret = 0;
	char tmp[32] = {0};
	char aOutput[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetVidOutput(tmp);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetVidOutput err\n");
		return -1;
	}
	sprintf(aOutput, "[%s]", tmp);
	sprintf(tmpparam, " %s", aOutput);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

// Set Decoder OSD Display
static int P3K_SetOsdDisplay(char *reqparam, char *respParam, char *userdata)
{
	//#KDS_OSD_DISPLAY  mode<CR>
	//~nn@KDS-OSD_DISPLAY mode<CR><LF>
	DBG_InfoMsg("P3K_SetOsdDisplay\n");
	int s32Ret = 0;
	int count = 0;
	int iOsdmode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	iOsdmode = atoi(str[0]);
	s32Ret = EX_SetOsdDisplay(iOsdmode);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetOsdDisplay err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetOsdDisplay(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-OSD_DISPLAY ? <CR>
	//~nn@KDS-OSD_DISPLAYmode<CR><LF>
	DBG_InfoMsg("P3K_GetOsdDisplay\n");
	int s32Ret = 0;
	int osdmode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	osdmode = EX_GetOsdDisplay();
	if (osdmode < 0)
	{
		EX_ERR_MSG(osdmode, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetOsdDisplay err\n");
		return -1;
	}
	sprintf(tmpparam, "%d", osdmode);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

// Set Daisy Chain.
static int P3K_SetDaisyChain(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-DAISY-CHAIN daisy_state<CR>
	//~nn@KDS-DAISY-CHAIN daisy_state<CR><LF>
	DBG_InfoMsg("P3K_SetOsdDisplay\n");
	int s32Ret = 0;
	int count = 0;
	int daisy_state = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	daisy_state = atoi(str[0]);

	s32Ret = EX_SetDaisyChain(daisy_state);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetDaisyChain err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetDaisyChain(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-DAISY-CHAIN?<CR>
	//~nn@KDS-DAISY-CHAIN daisy_state<CR><LF>
	DBG_InfoMsg("P3K_GetDaisyChain\n");
	int s32Ret = 0;
	int osdmode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	osdmode = EX_GetDaisyChain();
	if (osdmode < 0)
	{
		EX_ERR_MSG(osdmode, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetDaisyChain err\n");
		return -1;
	}
	sprintf(tmpparam, "%d", osdmode);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

// Set unicast / multicast.
static int P3K_SetMethod(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-METHOD method<CR>
	//~nn@KDS-METHOD  method<CR><LF>
	DBG_InfoMsg("P3K_SetMethod\n");
	int s32Ret = 0;
	int count = 0;
	int method = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	method = atoi(str[0]);

	s32Ret = EX_SetMethod(method);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetMethod err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetMethod(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-METHOD? <CR>
	//~nn@KDS-METHOD method<CR><LF>
	DBG_InfoMsg("P3K_GetMethod\n");
	int s32Ret = 0;
	int method = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	method = EX_GetMethod();
	if (method < 0)
	{
		EX_ERR_MSG(method, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetMethod err\n");
		return -1;
	}
	sprintf(tmpparam, "%d", method);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

// Set Inactivity auto-logout time
static int P3K_SetTimeOut(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-LOGOUT-TIMEOUT time<CR>
	//~nn@KDS-LOGOUT-TIMEOUT time<CR><LF>
	DBG_InfoMsg("P3K_SetTimeOut\n");
	int s32Ret = 0;
	int count = 0;
	int iTime = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	iTime = atoi(str[0]);

	s32Ret = EX_SetTimeOut(iTime);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetTimeOut err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetTimeOut(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-LOGOUT-TIMEOUT? <CR>
	//~nn@KDS-LOGOUT-TIMEOUT time<CR><LF>
	DBG_InfoMsg("P3K_SetTimeOut\n");
	int s32Ret = 0;
	int iTime = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	iTime = EX_GetTimeOut();
	if (iTime < 0)
	{
		EX_ERR_MSG(iTime, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetTimeOut err\n");
		return -1;
	}
	sprintf(tmpparam, "%d", iTime);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

// Set Inactivity auto-logout time
static int P3K_SetStandbyTimeOut(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-LOGOUT-TIMEOUT time<CR>
	//~nn@KDS-LOGOUT-TIMEOUT time<CR><LF>
	DBG_InfoMsg("P3K_SetStandbyTimeOut\n");
	int s32Ret = 0;
	int count = 0;
	int iTime = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	iTime = atoi(str[0]);

	s32Ret = EX_SetStandbyTimeOut(iTime);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetStandbyTimeOut err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetStandbyTimeOut(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-LOGOUT-TIMEOUT? <CR>
	//~nn@KDS-LOGOUT-TIMEOUT time<CR><LF>
	DBG_InfoMsg("P3K_SetStandbyTimeOut\n");
	int iTime = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	iTime = EX_GetStandbyTimeOut();
	if (iTime < 0)
	{
		EX_ERR_MSG(iTime, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetStandbyTimeOut err\n");
		return -1;
	}
	sprintf(tmpparam, "%d", iTime);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

// Set multicast group address and TTL value.
static int P3K_SetMulticastStatus(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-MULTICAST  group_ip,ttl <CR>
	//~nn@KDS-ACTION group_ip,ttl<CR><LF>
	DBG_InfoMsg("P3K_SetMulticastStatus\n");
	int s32Ret = 0;
	int count = 0;
	int iTtl = 0;
	char ip[16] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	int isip = checkisIp(str[0]);
	if (isip == -1)
	{
		ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	memcpy(ip, str[0], strlen(str[0]));
	if (isnum(str[1]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	iTtl = atoi(str[1]);

	s32Ret = EX_SetMulticastStatus(ip, iTtl);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetMulticastStatus err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

// Set gateway network port
static int P3K_SetGatewayPort(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-GW-ETH gw_type,netw_id<CR>
	//~nn@KDS-GW-ETH gw_type,netw_id<CR><LF>
	DBG_InfoMsg("P3K_SetGatewayPort\n");
	int s32Ret = 0;
	int count = 0;
	int iGw_Type = 0;
	int iNetw_Id = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1 || isnum(str[1]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	iGw_Type = atoi(str[0]);
	iNetw_Id = atoi(str[1]);
	if (iNetw_Id < 0)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}

	s32Ret = EX_SetGatewayPort(iGw_Type, iNetw_Id);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetGatewayPort err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetGatewayPort(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-GW-ETH? gw_type<CR>
	//~nn@KDS-GW-ETH gw_type,netw_id<CR><LF>
	DBG_InfoMsg("P3K_GetGatewayPort\n");
	int s32Ret = 0;
	int iGw_Type = 0;
	int iNetw_Id = 0;
	int count = 0;
	char aVersion[24] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	iGw_Type = atoi(str[0]);

	iNetw_Id = EX_GetGatewayPort(iGw_Type);
	if (iNetw_Id < 0)
	{
		EX_ERR_MSG(iNetw_Id, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetGatewayPort err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%d", iGw_Type, iNetw_Id);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetVlanTag(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-VLAN-TAG gw_type,xxxx<CR>
	//~nn@KDS-VLAN-TAG gw_type,xxxx<CR><LF>
	DBG_InfoMsg("P3K_SetVlanTag\n");
	int s32Ret = 0;
	int count = 0;
	int iGw_Type = 0;
	int iTag = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1 || isnum(str[1]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	iGw_Type = atoi(str[0]);
	iTag = atoi(str[1]);

	s32Ret = EX_SetVlanTag(iGw_Type, iTag);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetVlanTag err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetVlanTag(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-VLAN-TAG? gw_type<CR>
	//~nn@KDS-VLAN-TAG gw_type,xxxx<CR><LF>
	DBG_InfoMsg("P3K_GetVlanTag\n");
	int s32Ret = 0;
	int iGw_Type = 0;
	int iTag = 0;
	int count = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	iGw_Type = atoi(str[0]);

	iTag = EX_GetVlanTag(iGw_Type);
	if (iTag < 0)
	{
		EX_ERR_MSG(iTag, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetVlanTag err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%d", iGw_Type, iTag);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

// Reset password
static int P3K_SetPassword(char *reqparam, char *respParam, char *userdata)
{
	//#PASS old_pass,new_pass <CR>
	//~nn@PASS old_pass,new_pass <CR><LF>
	// or
	//~nn@PASS err 004<CR><LF>
	//(if bad old password entered)
	DBG_InfoMsg("P3K_SetPassword\n");
	int ret = 0;
	int count = 0;
	char login_level[12] = {0};
	char iNew_Pass[12] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	// iOld_Pass = atoi(str[0]);
	// iNew_Pass = atoi(str[1]);
	memcpy(login_level, str[0], strlen(str[0]));
	if (strlen(str[1]) > 15)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	memcpy(iNew_Pass, str[1], strlen(str[1]));

	ret = EX_SetPassword(login_level, iNew_Pass);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetPassword err\n");
		return -1;
	}
	sprintf(tmpparam, "%s,%s", login_level, iNew_Pass);
	if (ret != 0)
	{
		memset(tmpparam, 0, sizeof(tmpparam));
		sprintf(tmpparam, "%s", "err_004");
	}
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetPassword(char *reqparam, char *respParam, char *userdata)
{
	//#PASS? login_level<CR>
	//~nn@PASS login_level password<CR><LF>
	DBG_InfoMsg("P3K_GetPassword\n");
	int s32Ret = 0;
	int count = 0;
	char aLogin_level[16] = {0};
	char ologin_Pass[32] = {0};
	int iPassWord = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	memcpy(aLogin_level, str[0], strlen(str[0]));
	iPassWord = EX_GetPassword(aLogin_level, ologin_Pass);
	if (iPassWord < 0)
	{
		EX_ERR_MSG(iPassWord, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetPassword err\n");
		return -1;
	}
	else if (iPassWord)
	{
		sprintf(tmpparam, "%s,%s", aLogin_level, ologin_Pass);
	}
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetRollback(char *reqparam, char *respParam, char *userdata)
{
	//#ROLLBACK<CR>
	//~nn@ROLLBACK ok<CR><LF>
	DBG_InfoMsg("P3K_SetRollback\n");
	int ret = 0;
	int count = 0;
	char type[16] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	ret = EX_SetRollback(type);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetRollback err\n");
		return -1;
	}
	sprintf(tmpparam, "%s", "OK");
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetLogResetEvent(char *reqparam, char *respParam, char *userdata)
{
	//#LOG-RESET?<CR>
	//~nn@LOG-RESET log_type,date,time<CR><LF>
	DBG_InfoMsg("P3K_GetLogResetEvent\n");
	int s32Ret = 0;
	char weekDay[16] = {0};
	int iLog_Type = 0;
	char date[32] = {0};
	char time[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetLogResetEvent(&iLog_Type, date, time);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetLogResetEvent err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%s,%s", iLog_Type, date, time);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

// set tr gateway mode
static int P3K_SetIRGateway(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-IR-GW mode<CR>
	//~nn@KDS-IR-GW mode<CR><LF>
	DBG_InfoMsg("P3K_SetIRGateway\n");
	int ret = 0;
	int count = 0;
	int iGW_mode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	iGW_mode = atoi(str[0]);

	ret = EX_SetIRGateway(iGW_mode);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetIRGateway err\n");
		return -1;
	}
	// sprintf(tmpparam,"%s",type);
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetIRGateway(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-IR-GW?<CR>
	//~nn@KDS-IR-GW mode<CR><LF>
	DBG_InfoMsg("P3K_GetIRGateway\n");
	int s32Ret = 0;
	int iIr_mode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	iIr_mode = EX_GetIRGateway();
	if (iIr_mode < 0)
	{
		EX_ERR_MSG(iIr_mode, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetIRGateway err\n");
		return -1;
	}
	sprintf(tmpparam, "%d", iIr_mode);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_RmEDID(char *reqparam, char *respParam, char *userdata)
{
	//#EDID-RM Index <CR>
	//~nn@#EDID-RM Index<CR><LF>
	// or
	//~nn@EDID-RM err<CR><LF>
	DBG_InfoMsg("P3K_RmEDID\n");
	int ret = 0;
	int count = 0;
	int iEDID = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	iEDID = atoi(str[0]);
	sprintf(tmpparam, "%d", iEDID);
	ret = EX_RemoveEDID(iEDID); // EX_RmEDID(iEDID);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_RemoveEDID err\n");
		return -1;
	}

	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetVideoWallStretch(char *reqparam, char *respParam, char *userdata)
{
	//#WND-STRETCH out_index,mode<CR>
	//~nn@WND-STRETCH out_index,mode<CR><LF>
	DBG_InfoMsg("P3K_SetVideoWallStretch\n");
	int s32Ret = 0;
	int count = 0;
	int index = 0;
	int iMode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1 || isnum(str[1]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	index = atoi(str[0]);
	iMode = atoi(str[1]);
	s32Ret = EX_SetVideoWallStretch(index, iMode);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetVideoWallStretch err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetVideoWallStretch(char *reqparam, char *respParam, char *userdata)
{
	//#KDS-LOGOUT-TIMEOUT? <CR>
	//~nn@KDS-LOGOUT-TIMEOUT time<CR><LF>
	DBG_InfoMsg("P3K_GetVideoWallStretch\n");
	int s32Ret = 0;
	int iMode = 0;
	int count = 0;
	int index = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	index = atoi(str[0]);
	iMode = EX_GetVideoWallStretch(index);
	if (iMode < 0)
	{
		EX_ERR_MSG(iMode, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetVideoWallStretch err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%d", index, iMode);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_SetCfgModify(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_SetCfgModify\n");
	int ret = 0;
	ret = EX_SetCfgModify(reqparam);
	if (ret < 0)
	{
		EX_ERR_MSG(ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_SetCfgModify err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_Discovery(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_Discovery\n");
	int count = 0;
	char iIP[24] = "";
	int iport = 0;
	char aflag[12] = "";
	int u32ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	memcpy(aflag, str[0], strlen(str[0]));
	memcpy(iIP, str[1], strlen(str[1]));
	if (isnum(str[2]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	iport = atoi(str[2]);
	u32ret = EX_Discovery(aflag, iIP, iport);
	if (u32ret < 0)
	{
		// EX_ERR_MSG(u32ret,reqparam,respParam);
		// strcpy(userdata,"error");
		DBG_ErrMsg("EX_Discovery err\n");
		return -1;
	}
	return 0;
}

static int P3K_BEACON(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_BEACON\n");
	int count = 0;
	int iport_id = 0;
	int istatus = 0;
	int irate = 0;
	int u32ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	if (isnum(str[0]) == -1 || isnum(str[1]) == -1 || isnum(str[2]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	iport_id = atoi(str[0]);
	istatus = atoi(str[1]);
	irate = atoi(str[2]);
	u32ret = EX_Beacon(iport_id, istatus, irate);
	if (u32ret < 0)
	{
		EX_ERR_MSG(u32ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_Beacon err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_ConfBeaconInfo(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_ConfBeaconInfo\n");
	int s32Ret = 0;
	int count = 0;
	char muticastIP[64] = "";
	int port = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam, strlen(reqparam), str);
	int isip = checkisIp(str[0]);
	if (isip == -1)
	{
		ERR_MSG(ERR_PROTOCOL_SYNTAX, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	memcpy(muticastIP, str[0], strlen(str[0]));
	if (isnum(str[1]) == -1)
	{
		ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, reqparam, respParam);
		strcpy(userdata, "error");
		return -1;
	}
	port = atoi(str[1]);
	s32Ret = EX_ConfBeaconInfo(muticastIP, port);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_ConfBeaconInfo err\n");
		return -1;
	}
	memcpy(respParam, reqparam, strlen(reqparam));
	return 0;
}

static int P3K_GetBeaconConf(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_GetBeaconConf\n");
	int s32Ret = 0;
	char iMode = 0;
	char muticastIP[64] = "";
	int port = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetBeaconConf(muticastIP, &port);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetBeaconConf err\n");
		return -1;
	}
	sprintf(tmpparam, "%s,%d", muticastIP, port);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_GetBEACON(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_GetBEACON\n");
	int s32Ret = 0;
	int port = 0;
	int status = 0;
	int rate = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetBeacon(&port, &status, &rate);
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_GetBeacon err\n");
		return -1;
	}
	sprintf(tmpparam, "%d,%d,%d", port, status, rate);
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_PhraserNTFYParam(char *param, int len, char str[][MAX_PARAM_LEN])
{
	int tmpLen = 0;
	// int s32Ret = 0;
	int i = 0;
	char *tmpdata = param;
	char *tmpdata1 = param;

	if (param == NULL || len <= 0)
	{
		return -1;
	}
	while (tmpdata != NULL)
	{
		tmpdata = strchr(tmpdata, ':');
		if (tmpdata != NULL)
		{
			tmpLen = tmpdata - tmpdata1;
			memcpy(str[i], tmpdata1, tmpLen);
			i++;
			if (len > tmpdata - param + 1)
			{
				tmpdata1 = tmpdata + 1;
				tmpdata = tmpdata + 1;
			}
			else
			{
				break;
			}
		}
	}
	memcpy(str[i], tmpdata1, strlen(tmpdata1));
	i++;
	return i;
}

static int P3K_NTFY_PROC(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_NTFY_PROC\n");
	int count = 0;
	char CMD[24] = "";
	Notify_S s_NTFYInfo = {0};
	int u32ret = 0;
	char tmpparam[4096] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
	// char strParam[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	s_NTFYInfo.iParamNum = P3K_PhraserNTFYParam(reqparam, strlen(reqparam), str);
	if(s_NTFYInfo.iParamNum > 18)
	{
		char tmpdata[24] = {0};
		sprintf(tmpdata, "ERR %03d", ERR_PARAMETER_OUT_OF_RANGE);
		strcpy(userdata, tmpdata);
		return 0;
	}
	memcpy(CMD, str[0], strlen(str[0]));
	s_NTFYInfo.NCmd = P3K_CheckNTFYCMD(CMD);
	if (s_NTFYInfo.NCmd == -10)
	{
		char tmpdata[24] = {0};
		sprintf(tmpdata, "ERR %03d", ERR_PARAMETER_OUT_OF_RANGE);
		strcpy(userdata, tmpdata);
	}
	for (count = 2; count < s_NTFYInfo.iParamNum; count++)
	{
		memcpy(s_NTFYInfo.strParam[count - 2], str[count], strlen(str[count]));
	}
	s_NTFYInfo.iParamNum -= 2;
	u32ret = EX_NTFYPhraser(&s_NTFYInfo, tmpparam);
	if (u32ret < 0)
	{
		char tmpdata[24] = {0};
		if (u32ret == -1)
		{
			sprintf(tmpdata, "ERR %03d", ERR_COMMAND_NOT_AVAILABLE);
		}
		else if (u32ret == -2)
		{
			sprintf(tmpdata, "ERR %03d", ERR_PARAMETER_OUT_OF_RANGE);
		}
		else if (u32ret == -3)
		{
			sprintf(tmpdata, "ERR %03d", ERR_WRONG_MODE);
		}
		else
		{
			strcpy(tmpdata, "ERR");
		}
		strcpy(userdata, tmpdata);
	}
	else
	{
		P3K_NTFYToStr(s_NTFYInfo.NCmd, userdata);
		for (count = 0; count < s_NTFYInfo.iParamNum; count++)
		{
			sprintf(tmpparam + strlen(tmpparam), "%s", s_NTFYInfo.strParam[count]);
		}
		memcpy(respParam, tmpparam, strlen(tmpparam));
	}
	return 0;
}

static int P3K_TESTMODE(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_TESTMODE\n");
	int s32Ret = 0;
	int port = 0;
	int status = 0;
	int rate = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};

	s32Ret = EX_TESTMODE();
	if (s32Ret < 0)
	{
		EX_ERR_MSG(s32Ret, reqparam, respParam);
		strcpy(userdata, "error");
		DBG_ErrMsg("EX_TESTMODE err\n");
		return -1;
	}
	memcpy(respParam, tmpparam, strlen(tmpparam));
	return 0;
}

static int P3K_HELP(char *reqparam, char *respParam, char *userdata)
{
	DBG_InfoMsg("P3K_HELP\n");
	int s32Ret = 0;

	s32Ret = EX_HelpCmd(respParam);
	return 0;
}

/*   P3K_SetIRGateway  */
/*P3K_GetBEACON
int P3K_SilmpleSpecReqCmdProcess(P3K_SimpleSpecCmdInfo_S * cmdreq, P3K_SimpleSpecCmdInfo_S * cmdresp)
{
	static P3K_SpecPhraserToExecute_S cliSpecialFunc[] = {

											  {"LDFW",NULL},

												{NULL,NULL}

	};
	if(cmdreq == NULL || cmdresp == NULL)
	{
		return -1;
	}
}
*/
int P3K_SilmpleReqCmdProcess(P3K_SimpleCmdInfo_S *cmdreq, P3K_SimpleCmdInfo_S *cmdresp, char *userdata)
{
	static P3K_PhraserToExecute_S cliFunc[] = {

		{"KDS-AUD", P3K_SetAudioInputMode, 1},
		{"KDS-AUD?", P3K_GetAudioInputMode, 0},
		{"X-AUD-LVL", P3K_SetAudLevel, 2},
		{"X-AUD-LVL?", P3K_GetAudLevel, 1},
		{"X-AUD-DESC?", P3K_GetAudParam, 1},
		{"X-AV-SW-MODE", P3K_SetAutoSwitchMode, 2},
		{"X-AV-SW-MODE?", P3K_GetAutoSwitchMode, 1},
		{"CPEDID", P3K_CopyEdid, 4},
		{"LOCK-EDID", P3K_SetEdidLockMode, 2},
		{"LOCK-EDID?", P3K_GetEdidLockMode, 1},
		{"HDCP-MOD", P3K_SetHDCPMode, 2},
		{"HDCP-MOD?", P3K_GetHDCPMode, 1},
		{"HDCP-STAT?", P3K_GetHDCPStatus, 2},
		{"VIEW-MOD", P3K_SetVideoWallMode, -2},
		{"VIEW-MOD?", P3K_GetVideoWallMode, 0},
		{"WND-BEZEL", P3K_SetWndBezel, 6},
		{"WND-BEZEL?", P3K_GetWndBezel, 0},
		{"VIDEO-WALL-SETUP", P3K_SetVideoWallRotaion, 2},
		{"VIDEO-WALL-SETUP?", P3K_GetVideoWallRotaion, 0},
		{"KDS-START-OVERLAY", P3K_StartOverlay, 2},
		{"KDS-STOP-OVERLAY", P3K_StopOverlay, 0},
		{"KDS-DEFINE-CHANNEL", P3K_SetChannelId, 1},
		{"KDS-DEFINE-CHANNEL?", P3K_GetChannelId, 0},
		{"KDS-CHANNEL-SELECT",P3K_SetChannleSelection,-2},
		{"KDS-CHANNEL-SELECT?",P3K_GetChannleSelection,1},
		{"KDS-ACTION", P3K_SetVideoAction, 1},
		{"KDS-ACTION?", P3K_GetVideoAction, 0},
		{"CS-CONVERT", P3K_SetVideoOutCS, 2},
		{"CS-CONVERT?", P3K_GetVideoOutCS, 1},
		{"KDS-RESOL?", P3K_GetVideoReslotion, 3},
		{"CEC-SND", P3K_SendCECMsg, 5},
		{"LOG-ACTION", P3K_SetLogAction, 2},
		{"LOG-ACTION?", P3K_GetLogAction, 0},
		{"CEC-NTFY", P3K_RecvCECNtfy, 0},
		{"CEC-GW-PORT-ACTIVE", P3K_SetCECGWMode, 1},
		{"CEC-GW-PORT-ACTIVE?", P3K_GetCECGWMode, 0},
		{"IR-STOP", P3K_StopIRMsg, -2},
		{"X-ROUTE", P3K_SetXROUTEMatch, -2},
		{"X-ROUTE?", P3K_GetXROUTEMatch, 1},
		{"UART", P3K_SetUartConf, -2},
		{"UART?", P3K_GetUartConf, 1},
		{"COM-ROUTE-ADD", P3K_AddComRoute, 5},
		{"COM-ROUTE-REMOVE", P3K_RemoveComRoute, 1},
		{"COM-ROUTE?", P3K_GetComRouteInfo, 1},
		{"KDS-MULTICAST?", P3K_GetMulticastStatus, 0},
		{"FCT-MAC", P3K_SetMacAddr, 1},
		{"NET-MAC?", P3K_GetMacAddr, -2},
		{"NAME-RST", P3K_ResetDNSName, 0},
		{"NET-DHCP", P3K_SetDHCPMode, -2},
		{"NET-DHCP?", P3K_GetDHCPMode, -2},
		{"NET-CONFIG", P3K_SetNetConf, -2},
		{"NET-CONFIG?", P3K_GetNetConf, 1},
		{"ETH-PORT", P3K_SetEthPort, 2},
		{"ETH-PORT?", P3K_GetEthPort, 1},
		{"SECUR", P3K_SetSecurity, 1},
		{"SECUR?", P3K_GetSecurity, 0},
		{"LOGIN", P3K_SetLogin, 2},
		{"LOGIN?", P3K_GetLogin, 0},
		{"LOGOUT", P3K_LogOut, 0},
		{"VERSION?", P3K_GetVersion, 0},
		{"BL-VERSION?", P3K_GetBootVersion, 0},
		{"UPGRADE", P3K_Upgrade, 0},
		{"UPGRADE-STATUS?", P3K_UpgradeStatus, 0},
		{"FCT-MODEL", P3K_SetFCTMODEL, 1},
		{"MODEL?", P3K_GetFCTMODEL, 0},
		{"FCT-SN", P3K_SetSerailNum, 1},
		{"SN?", P3K_GetSerailNum, 0},
		{"LOCK-FP", P3K_SetLockFP, 1},
		{"LOCK-FP?", P3K_GetLockFP, 0},
		{"IDV", P3K_SetIDV, 0},
		{"STANDBY?", P3K_GetStandByMode, 1},
		{"RESET", P3K_DoReset, 0},
		{"FACTORY", P3K_DoFactory, 0},
		{"BEACON-EN", P3K_BEACON, 3},
		{"BEACON-EN?", P3K_GetBEACON, 0},
		{"BEACON-INFO?", P3K_GetBeaconInfo, 1},
		{"BUILD-DATE?", P3K_GetBuildTime, 0},
		{"TIME", P3K_SetTime, 3},
		{"TIME?", P3K_GetTime, 0},
		{"TIME-LOC", P3K_SetTimeZero, 2},
		{"TIME-LOC?", P3K_GetTimeZero, 0},
		{"TIME-SRV", P3K_SetTimeServer, 3},
		{"TIME-SRV?", P3K_GetTimeServer, 0},
		{"SIGNALS-LIST?", P3K_GetSignalList, 0},
		{"PORTS-LIST?", P3K_GetPortList, 0},
		{"KDS-ACTIVE-CLNT?", P3K_GetActiveCli, 0},
		{"PORT-DIRECTION", P3K_SetAudAnalogDir, 2},
		{"PORT-DIRECTION?", P3K_GetAudAnalogDir, 1},
		{"X-PRIORITY", P3K_SetAutoSwitchPriority, -2},
		{"X-PRIORITY?", P3K_GetAutoSwitchPriority, 1},
		{"EDID-MODE", P3K_SetEDIDMode, -2},
		{"EDID-MODE?", P3K_GetEDIDMode, 1},
		{"EDID-LIST?", P3K_GetEDIDList, 0},
		{"EDID-ADD", P3K_AddEDID, 2},
		{"EDID-ACTIVE", P3K_SetActiveEDID, 2},
		{"EDID-ACTIVE?", P3K_GetActiveEDID, 1},
		{"EDID-NET-SRC", P3K_SetEDIDNetSrc, 2},
		{"EDID-NET-SRC?", P3K_GetEDIDNetSrc, 1},
		{"GTW-MSG-NUM-SEND?", P3K_GetSendMsgNum, 2},
		{"KDS-RATIO?", P3K_GetVidOutRatio, 0},
		{"KDS-DEFINE-CHANNEL-NAME", P3K_SetChannelName, 1},
		{"KDS-DEFINE-CHANNEL-NAME?", P3K_GetChannelName, 0},
		{"NAME", P3K_SetDNSName, 2},
		{"NAME?", P3K_GetDNSName, 1},
		{"X-MUTE", P3K_SetAudMute, 2},
		{"X-MUTE?", P3K_GetAudMute, 1},
		{"NET-STAT?", P3K_GetConnectionList, 0},
		{"STANDBY-VERSION?", P3K_GetStandbyVersion, 0},
		{"HW-VERSION?", P3K_GetHWVersion, 0},
		{"DEV-STATUS?", P3K_GetDevStatus, 0},
		{"HW-TEMP?", P3K_GetHWTemp, 2},
		{"KDS-AUD-OUTPUT", P3K_SetAudOutput, -2},
		{"UPG-TIME?", P3K_GetUPGTime, 0},
		{"KDS-AUD-OUTPUT?", P3K_GetAudOutput, 0},
		{"KDS-OSD-DISPLAY", P3K_SetOsdDisplay, 1},
		{"KDS-OSD-DISPLAY?", P3K_GetOsdDisplay, 0},
		{"KDS-DAISY-CHAIN", P3K_SetDaisyChain, 1},
		{"KDS-DAISY-CHAIN?", P3K_GetDaisyChain, 0},
		{"KDS-METHOD", P3K_SetMethod, 1},
		{"KDS-METHOD?", P3K_GetMethod, 0},
		{"KDS-SCALE",P3K_SetVideoOutScaler,2},
		{"KDS-SCALE?",P3K_GetVideoOutScaler,0},
		{"LOGOUT-TIMEOUT", P3K_SetTimeOut, 1},
		{"LOGOUT-TIMEOUT?", P3K_GetTimeOut, 0},
		{"KDS-MULTICAST", P3K_SetMulticastStatus, 2},
		{"KDS-GW-ETH", P3K_SetGatewayPort, 2},
		{"KDS-GW-ETH?", P3K_GetGatewayPort, 1},
		{"KDS-VLAN-TAG", P3K_SetVlanTag, 2},
		{"KDS-VLAN-TAG?", P3K_GetVlanTag, 1},
		{"GEDID", P3K_GetEdid, 2},
		{"EDID-CS", P3K_SetEdidCsMode, 2},
		{"EDID-CS?", P3K_GetEdidCsMode, 1},
		{"IMAGE-PROP", P3K_SetVideoOutMode, 2},
		{"IMAGE-PROP?", P3K_GetVideoOutMode, 1},
		{"PASS", P3K_SetPassword, 2},
		{"PASS?", P3K_GetPassword, 1},
		{"ROLLBACK", P3K_SetRollback, 0},
		{"KDS-IR-GW", P3K_SetIRGateway, 1},
		{"KDS-IR-GW?", P3K_GetIRGateway, 0},
		{"EDID-RM", P3K_RmEDID, 1},
		{"WND-STRETCH", P3K_SetVideoWallStretch, 2},
		{"WND-STRETCH?", P3K_GetVideoWallStretch, 1},
		{"KDS-CFG-MODIFY", P3K_SetCfgModify, -2},
		{"UDPNET-CONFIG?", P3K_Discovery, -2},
		{"BEACON-CONF", P3K_ConfBeaconInfo, 2},
		{"BEACON-CONF?", P3K_GetBeaconConf, 0},
		{"P3K-NOTIFY", P3K_NTFY_PROC, -2},
		{"TEST-MODE", P3K_TESTMODE, 0},
		{"IR-SND", P3K_SendIRMsg, -2},
		{"HELP", P3K_HELP, 0},
		{NULL, NULL}};
	printf("<<<<<<recv==%s %s\n", cmdreq->command, cmdreq->param);
	if (cmdreq == NULL || cmdresp == NULL)
	{
		DBG_WarnMsg("P3K cmdreq or cmd resp is NULL\n ");
		return -1;
	}
	int i = 0;
	for (i = 0;; i++)
	{
		if (cliFunc[i].cmd == NULL)
		{
			DBG_WarnMsg("cmd[ %s ]does not support\n", cmdreq->command);
			memcpy(cmdresp->command, cmdreq->command, strlen(cmdreq->command));
			memcpy(cmdresp->param, "ERR 002", strlen("ERR 002"));
			strcpy(userdata, "error");
			return 0;
		}
		if (strcasecmp(cliFunc[i].cmd, cmdreq->command) == 0)
		{
			if (cliFunc[i].ParamPhraser != NULL)
			{
				memset(cmdresp->command, 0, sizeof(cmdresp->command));
				memcpy(cmdresp->command, cmdreq->command, strlen(cmdreq->command));
				memset(cmdresp->param, 0, sizeof(cmdresp->param));
				if (cliFunc[i].paramnum == -2)
				{
					cliFunc[i].ParamPhraser(cmdreq->param, cmdresp->param, userdata);
				}
				else if (cliFunc[i].paramnum == 0)
				{
					char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
					int count = 0;
					count = P3K_PhraserParam(cmdreq->param, strlen(cmdreq->param), str);
					if (count == -1)
					{
						cliFunc[i].ParamPhraser(cmdreq->param, cmdresp->param, userdata);
					}
					else
					{
						if (count == -2)
							ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, cmdreq->param, cmdresp->param);
						else
							ERR_MSG(ERR_PROTOCOL_SYNTAX, cmdreq->param, cmdresp->param);
						strcpy(userdata, "error");
						return 0;
					}
				}
				else if (cliFunc[i].paramnum > 0)
				{
					char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] = {0};
					int count = 0;
					count = P3K_PhraserParam(cmdreq->param, strlen(cmdreq->param), str);
					if (count == cliFunc[i].paramnum)
					{
						cliFunc[i].ParamPhraser(cmdreq->param, cmdresp->param, userdata);
					}
					else
					{
						if (count == -2)
							ERR_MSG(ERR_PARAMETER_OUT_OF_RANGE, cmdreq->param, cmdresp->param);
						else
							ERR_MSG(ERR_PROTOCOL_SYNTAX, cmdreq->param, cmdresp->param);
						strcpy(userdata, "error");
						return 0;
					}
				}
			}
			break;
		}
	}
	return 0;
}

int P3K_CheckedSpeciCmd(char *cmd)
{
	static char *specCmd[] = {

		"LDFW",
		NULL,
	};

	int i = 0;
	for (i = 0;; i++)
	{
		if (specCmd[i] == NULL)
		{
			break;
		}
		if (strcasecmp(specCmd[i], cmd) == 0)
		{
			return 1;
		}
	}
	return 0;
}

/*
									{"STANDBY-TIMEOUT",P3K_SetStandbyTimeOut,1},
									{"STANDBY-TIMEOUT?",P3K_GetStandbyTimeOut,0},
									{"LOG-RESET?",P3K_GetLogResetEvent,0},
									{"STANDBY",P3K_SetStandByMode,1},
									{"KVM-USB-CTRL",P3K_SetUSBMode,1},
		{"HELP", P3K_HELP, 0},
				{"IR-SND", P3K_SendIRMsg, -2},
									{"ETH-TUNNEL?",P3K_GetEthTunnel,0},
									{"KDS-BR?",P3K_GetVideoBitRate,0},
									{"KDS-FR?",P3K_GetVideoFrameRate,0},
									{"LOG-ACTION",P3K_SetLogAction,2},
									{"LOG-ACTION?",P3K_GetLogAction,0},
									{"GTW-MSG-NUM?", P3K_GetRecvMsgNum, 2},
*/
