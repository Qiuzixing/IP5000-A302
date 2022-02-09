/* Change this if the SERVER_NAME environment variable does not report
	the true name of your web server. */
#if 1
#define SERVER_NAME cgiServerName
#endif
#if 0
#define SERVER_NAME "www.boutell.com"
#endif

#if 0//steven
/* You may need to change this, particularly under Windows;
	it is a reasonable guess as to an acceptable place to
	store a saved environment in order to test that feature. 
	If that feature is not important to you, you needn't
	concern yourself with this. */

#ifdef WIN32
#define SAVED_ENVIRONMENT "c:\\cgicsave.env"
#else
#define SAVED_ENVIRONMENT "/tmp/cgicsave.env"
#endif /* WIN32 */
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cgic.h"

#if 1//steven
static  unsigned long crc_table[256] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};

#define DO1(byte_data) crc = crc_table[((int)crc ^ (byte_data)) & 0xff] ^ (crc >> 8);
#endif

void HandleSubmit();
#if 0//steven
void ShowForm();
void CookieSet();
void Name();
void Address();
void Hungry();
void Temperature();
void Frogs();
void Color();
void Flavors();
void NonExButtons();
void RadioButtons();
#endif
void File();
#if 0//steven
void Entries();
void Cookies();
void LoadEnvironment();
void SaveEnvironment();
#endif

void cgiOnCalled(void)
{
	system("/usr/local/bin/ast_send_event -1 e_stop_link");
}

int cgiMain() {
	HandleSubmit();
	return 0;
}

void HandleSubmit()
{
	File();
}

#if 0//steven
void Name() {
	char name[81];
	cgiFormStringNoNewlines("name", name, 81);
	fprintf(cgiOut, "Name: ");
	cgiHtmlEscape(name);
	fprintf(cgiOut, "<BR>\n");
}
	
void Address() {
	char address[241];
	cgiFormString("address", address, 241);
	fprintf(cgiOut, "Address: <PRE>\n");
	cgiHtmlEscape(address);
	fprintf(cgiOut, "</PRE>\n");
}

void Hungry() {
	if (cgiFormCheckboxSingle("hungry") == cgiFormSuccess) {
		fprintf(cgiOut, "I'm Hungry!<BR>\n");
	} else {
		fprintf(cgiOut, "I'm Not Hungry!<BR>\n");
	}
}
	
void Temperature() {
	double temperature;
	cgiFormDoubleBounded("temperature", &temperature, 80.0, 120.0, 98.6);
	fprintf(cgiOut, "My temperature is %f.<BR>\n", temperature);
}
	
void Frogs() {
	int frogsEaten;
	cgiFormInteger("frogs", &frogsEaten, 0);
	fprintf(cgiOut, "I have eaten %d frogs.<BR>\n", frogsEaten);
}

char *colors[] = {
	"Red",
	"Green",
	"Blue"
};

void Color() {
	int colorChoice;
	cgiFormSelectSingle("colors", colors, 3, &colorChoice, 0);
	fprintf(cgiOut, "I am: %s<BR>\n", colors[colorChoice]);
}	 

char *flavors[] = {
	"pistachio",
	"walnut",
	"creme"
};

void Flavors() {
	int flavorChoices[3];
	int i;
	int result;	
	int invalid;
	result = cgiFormSelectMultiple("flavors", flavors, 3, 
		flavorChoices, &invalid);
	if (result == cgiFormNotFound) {
		fprintf(cgiOut, "I hate ice cream.<p>\n");
	} else {	
		fprintf(cgiOut, "My favorite ice cream flavors are:\n");
		fprintf(cgiOut, "<ul>\n");
		for (i=0; (i < 3); i++) {
			if (flavorChoices[i]) {
				fprintf(cgiOut, "<li>%s\n", flavors[i]);
			}
		}
		fprintf(cgiOut, "</ul>\n");
	}
}

char *ages[] = {
	"1",
	"2",
	"3",
	"4"
};

void RadioButtons() {
	int ageChoice;
	char ageText[10];
	/* Approach #1: check for one of several valid responses. 
		Good if there are a short list of possible button values and
		you wish to enumerate them. */
	cgiFormRadio("age", ages, 4, &ageChoice, 0);

	fprintf(cgiOut, "Age of Truck: %s (method #1)<BR>\n", 
		ages[ageChoice]);

	/* Approach #2: just get the string. Good
		if the information is not critical or if you wish
		to verify it in some other way. Note that if
		the information is numeric, cgiFormInteger,
		cgiFormDouble, and related functions may be
		used instead of cgiFormString. */	
	cgiFormString("age", ageText, 10);

	fprintf(cgiOut, "Age of Truck: %s (method #2)<BR>\n", ageText);
}

char *votes[] = {
	"A",
	"B",
	"C",
	"D"
};

void NonExButtons() {
	int voteChoices[4];
	int i;
	int result;	
	int invalid;

	char **responses;

	/* Method #1: check for valid votes. This is a good idea,
		since votes for nonexistent candidates should probably
		be discounted... */
	fprintf(cgiOut, "Votes (method 1):<BR>\n");
	result = cgiFormCheckboxMultiple("vote", votes, 4, 
		voteChoices, &invalid);
	if (result == cgiFormNotFound) {
		fprintf(cgiOut, "I hate them all!<p>\n");
	} else {	
		fprintf(cgiOut, "My preferred candidates are:\n");
		fprintf(cgiOut, "<ul>\n");
		for (i=0; (i < 4); i++) {
			if (voteChoices[i]) {
				fprintf(cgiOut, "<li>%s\n", votes[i]);
			}
		}
		fprintf(cgiOut, "</ul>\n");
	}

	/* Method #2: get all the names voted for and trust them.
		This is good if the form will change more often
		than the code and invented responses are not a danger
		or can be checked in some other way. */
	fprintf(cgiOut, "Votes (method 2):<BR>\n");
	result = cgiFormStringMultiple("vote", &responses);
	if (result == cgiFormNotFound) {	
		fprintf(cgiOut, "I hate them all!<p>\n");
	} else {
		int i = 0;
		fprintf(cgiOut, "My preferred candidates are:\n");
		fprintf(cgiOut, "<ul>\n");
		while (responses[i]) {
			fprintf(cgiOut, "<li>%s\n", responses[i]);
			i++;
		}
		fprintf(cgiOut, "</ul>\n");
	}
	/* We must be sure to free the string array or a memory
		leak will occur. Simply calling free() would free
		the array but not the individual strings. The
		function cgiStringArrayFree() does the job completely. */	
	cgiStringArrayFree(responses);
}

void Entries()
{
	char **array, **arrayStep;
	fprintf(cgiOut, "List of All Submitted Form Field Names:<p>\n");
	if (cgiFormEntries(&array) != cgiFormSuccess) {
		return;
	}
	arrayStep = array;
	fprintf(cgiOut, "<ul>\n");
	while (*arrayStep) {
		fprintf(cgiOut, "<li>");
		cgiHtmlEscape(*arrayStep);
		fprintf(cgiOut, "\n");
		arrayStep++;
	}
	fprintf(cgiOut, "</ul>\n");
	cgiStringArrayFree(array);
}

void Cookies()
{
	char **array, **arrayStep;
	char cname[1024], cvalue[1024];
	fprintf(cgiOut, "Cookies Submitted On This Call, With Values (Many Browsers NEVER Submit Cookies):<p>\n");
	if (cgiCookies(&array) != cgiFormSuccess) {
		return;
	}
	arrayStep = array;
	fprintf(cgiOut, "<table border=1>\n");
	fprintf(cgiOut, "<tr><th>Cookie<th>Value</tr>\n");
	while (*arrayStep) {
		char value[1024];
		fprintf(cgiOut, "<tr>");
		fprintf(cgiOut, "<td>");
		cgiHtmlEscape(*arrayStep);
		fprintf(cgiOut, "<td>");
		cgiCookieString(*arrayStep, value, sizeof(value));
		cgiHtmlEscape(value);
		fprintf(cgiOut, "\n");
		arrayStep++;
	}
	fprintf(cgiOut, "</table>\n");
	cgiFormString("cname", cname, sizeof(cname));	
	cgiFormString("cvalue", cvalue, sizeof(cvalue));	
	if (strlen(cname)) {
		fprintf(cgiOut, "New Cookie Set On This Call:<p>\n");
		fprintf(cgiOut, "Name: ");	
		cgiHtmlEscape(cname);
		fprintf(cgiOut, "Value: ");	
		cgiHtmlEscape(cvalue);
		fprintf(cgiOut, "<p>\n");
		fprintf(cgiOut, "If your browser accepts cookies (many do not), this new cookie should appear in the above list the next time the form is submitted.<p>\n"); 
	}
	cgiStringArrayFree(array);
}
#endif

#if CGI_USE_TMPFS
#define NEW_FW_FORMAT 1
int do_async_update=0;
FILE *info_out = NULL;
#else
#define NEW_FW_FORMAT 0
#endif

#if NEW_FW_FORMAT
int update_fw(char *fname, int file_size)
{
	cgiFilePtr file = NULL;
	FILE *fp=NULL;
	char buffer[1024];
	int got;
	int len, ret = 0;

	if (cgiFormFileOpen(fname, &file) != cgiFormSuccess) {
		fprintf(info_out, "Could not open the file.\n");
		return 1;
	}

	fp = fopen("/dev/shm/fw.tar.gz", "w");
	if (fp == NULL) {
		fprintf(info_out, "failed to open /dev/shm/fw.tar.gz\n");
		ret = 1;
		goto done;
	}

	while (cgiFormFileRead(file, buffer, sizeof(buffer), &got) ==
		cgiFormSuccess)
	{
		len = fwrite(buffer, 1, got, fp);
		if (len != got)
		{
			fprintf(info_out, "failed to write to /dev/shm/fw.tar.gz\n");
			ret = 1;
			goto done;
		}
	}

	fflush(fp);

done:

	if (fp != NULL)
		fclose(fp);
	if (file != NULL)
		cgiFormFileClose(file);

	return ret;
}

int cgiOnClose(void)
{
	if (do_async_update)
		system("/usr/local/bin/update_fw.sh > /dev/shm/uf 2>/dev/shm/uf_e");
}


void html_update_fw_page(void)
{
	FILE *page_fp;
	char buffer[1024];
	size_t got;

	/*
	//Looks like "Location" doesn't work for busybox httpd. :(
	//cgiHeaderLocation("/update_fw.html");
	*/

	cgiHeaderContentType("text/html");
	fprintf(cgiOut, "<HTML><HEAD>\n");
	fprintf(cgiOut, "<TITLE></TITLE>\n");
	fprintf(cgiOut, "<meta http-equiv=\"refresh\" content=\"0;URL=/update_fw.html\">\n");
	fprintf(cgiOut, "</HEAD><BODY></BODY></HTML>\n");

#if 0	
	page_fp = fopen("/www/update_fw.html", "r");
	if (page_fp == NULL)
		return;

	while ((got = fread(buffer, 1, 1024, page_fp)) > 0)
	{
		fwrite(buffer, 1, got, cgiOut);
	}
#endif
}

void File(void)
{
	FILE *rfp = NULL;
	char tempBuf[1024];
	int size;

	system("/usr/local/bin/ast_send_event -1 e_stop_link");
	
	system("rm -f /www/fw_size_total.js");
	system("rm -f /www/fw_size_remain.js");
	info_out = fopen("/www/update_fw_info.txt", "w");
	if (info_out == NULL)
		goto done;
	//fprintf(info_out, "Content-type: text/plain\r\n\r\n");
	rfp = fopen("/www/update_fw_result.txt", "w");
	if (rfp == NULL) {
		fprintf(info_out, "Can't open update_fw_result.txt??\n");
		goto done;
	}

	if (cgiFormFileName("fwfile", tempBuf, 1024) == cgiFormNotFound) {
		fprintf(info_out, "No firmware file was uploaded.\n");
		fprintf(rfp, "FAILED");
		goto done;
	}

	fprintf(info_out, "firmware file name: %s\n", tempBuf);
	
	cgiFormFileSize("fwfile", &size);
	fprintf(info_out, "firmware file size: %d bytes\n", size);

	if (update_fw("fwfile", size)) {
		fprintf(info_out, "Upgrade firmware Failed\n");
		fprintf(rfp, "FAILED");
	} else {
		fprintf(info_out, "Decompressing firmware...\n");
		fprintf(rfp, "Please Wait...");
		do_async_update=1;
	}
	goto done;

done:
	html_update_fw_page();

	if (info_out)
		fclose(info_out);
	if (rfp)
		fclose(rfp);

}

#else //#if NEW_FW_FORMAT
int cgiOnClose(void)
{
	return 0;
}

#ifdef AST1500_HOST
#define HEADER_STRING "ast1500host"
#else
#define HEADER_STRING "ast1500client"
#endif
#define BUFFER_SIZE 4096//the block size used by file system
void File()
{
	cgiFilePtr file = NULL;
	char tempBuf[1024];
	int size;
	int got;
	FILE *fd = NULL;
	int len;
	int size_written = 0;
	cgiFormResultType result;
	unsigned long ulType, ulSize;
	unsigned long crc = 0;
	int ch, i, j;
	char *inb;
	int inLength;

	cgiHeaderContentType("text/html");
	
	if (cgiFormFileName("fwfile", tempBuf, 1024) == cgiFormNotFound) {
		printf("<p>No firmware file was uploaded.<p>\n");
		goto done;
	}
	fprintf(cgiOut, "firmware file name: ");
	cgiHtmlEscape(tempBuf);
	fprintf(cgiOut, "<p>\n");
	
	cgiFormFileSize("fwfile", &size);
	fprintf(cgiOut, "firmware file size: %d bytes<p>\n", size);
	
//	cgiFormFileContentType("fwfile", buffer, sizeof(buffer));
//	fprintf(cgiOut, "The alleged content type of the file was: ");
//	cgiHtmlEscape(buffer);
//	fprintf(cgiOut, "<p>\n");


	//verify firmware file
	crc = crc ^ 0xffffffffL;
	if (cgiFormFileOpen("fwfile", &file) != cgiFormSuccess) {
		fprintf(cgiOut, "Could not open firmware file.<p>\n");
		goto done;
	}
	inLength = file->bufferLength;
	if (size != inLength) {
		fprintf(cgiOut, "wrong buffer length (%d).<p>\n", inLength);
		goto done;
	}
	
	inb = file->buffer;
	if (strncmp(inb, HEADER_STRING, strlen(HEADER_STRING)) != 0)
	{
		fprintf(cgiOut, "not valid firmware file (%s)\n", HEADER_STRING);
		goto done;
	}
	for (i = 0; i < 16; i++)
	{
		ch = *inb++;
		DO1(ch);
	}
//	fprintf(cgiOut, "CRC_1 = %08X<p>\n", crc);
	
	for (i = 1; i < 3; i++)
	{
		ulType = *((unsigned long *)inb);
		if (ulType != i)
		{
			fprintf(cgiOut, "firmware file corrupted (%dth type mismatch)\n", i);
			goto done;
		}
		for (j = 0; j < 4; j++)
		{
			ch = *inb++;
			DO1(ch);
		}
		ulSize = *((unsigned long *)inb);
		fprintf(cgiOut, "ulSize = %d<p>\n", ulSize);
		ulSize = (ulSize + 3) & 0xFFFFFFFC;
		if (size < ulSize)
		{
			fprintf(cgiOut, "firmware file corrupted (less than %dth data size)\n", i);
			goto done;
		}
		for (j = 0; j < 4; j++)
		{
			ch = *inb++;
			DO1(ch);
		}
//		fprintf(cgiOut, "CRC_%d = %08X<p>\n", 3 + 3 * (i - 1), crc);
		for (j = 0; j < ulSize; j++)
		{
			ch = *inb++;
			DO1(ch);
		}
//		fprintf(cgiOut, "CRC_%d = %08X<p>\n", 4 + 3 * (i - 1), crc);
		size -= ulSize;
	}
	crc = crc ^ 0xffffffffL;
	if (crc != *((unsigned long *)inb))
	{
		fprintf(cgiOut, "CRC mismatched (%08X) (%08X)\n", crc, *((unsigned long *)inb));
		goto done;
	}

	//write to Flash
	inb = file->buffer;
	inb += 16;
	for (i = 1; i < 3; i++)
	{
		if (i == 1)
			fd = fopen("/dev/mtdblkkernel", "r+");
		else if (i == 2)
			fd = fopen("/dev/mtdblkrootfs", "r+");
		if (fd == NULL)
		{
			if (i == 1)
				fprintf(cgiOut, "failed to open /dev/mtdblkkernel\n");
			else if (i == 2)
				fprintf(cgiOut, "failed to open /dev/mtdblkrootfs\n");
			goto done;
		}
		
		inb += 4;
		ulSize = *((unsigned long *)inb);
		ulSize = (ulSize + 3) & 0xFFFFFFFC;
		inb += 4;
		while (ulSize)
		{
			if (ulSize >= BUFFER_SIZE)
				got = BUFFER_SIZE;
			else
				got = ulSize;
			len = fwrite(inb, 1, got, fd);
			if (len != got)
			{
				fprintf(cgiOut, "failed to write to /dev/mtdblkkernel\n");
				break;
			}
			inb += got;
			ulSize -= got;
		}
		fclose(fd);
		fd = NULL;
	}
	printf("<p>Upgrade firmware successfully.\nPlease restart the system.\n");

done:
	if (fd)
		fclose(fd);
}
#endif //#if NEW_FW_FORMAT


#if 0//steven
void ShowForm()
{
	fprintf(cgiOut, "<!-- 2.0: multipart/form-data is required for file uploads. -->");
	fprintf(cgiOut, "<form method=\"POST\" enctype=\"multipart/form-data\" ");
	fprintf(cgiOut, "	action=\"");
	cgiValueEscape(cgiScriptName);
	fprintf(cgiOut, "\">\n");
	fprintf(cgiOut, "<p>\n");
	fprintf(cgiOut, "Text Field containing Plaintext\n");
	fprintf(cgiOut, "<p>\n");
	fprintf(cgiOut, "<input type=\"text\" name=\"name\">Your Name\n");
	fprintf(cgiOut, "<p>\n");
	fprintf(cgiOut, "Multiple-Line Text Field\n");
	fprintf(cgiOut, "<p>\n");
	fprintf(cgiOut, "<textarea NAME=\"address\" ROWS=4 COLS=40>\n");
	fprintf(cgiOut, "Default contents go here. \n");
	fprintf(cgiOut, "</textarea>\n");
	fprintf(cgiOut, "<p>\n");
	fprintf(cgiOut, "Checkbox\n");
	fprintf(cgiOut, "<p>\n");
	fprintf(cgiOut, "<input type=\"checkbox\" name=\"hungry\" checked>Hungry\n");
	fprintf(cgiOut, "<p>\n");
	fprintf(cgiOut, "Text Field containing a Numeric Value\n");
	fprintf(cgiOut, "<p>\n");
	fprintf(cgiOut, "<input type=\"text\" name=\"temperature\" value=\"98.6\">\n");
	fprintf(cgiOut, "Blood Temperature (80.0-120.0)\n");
	fprintf(cgiOut, "<p>\n");
	fprintf(cgiOut, "Text Field containing an Integer Value\n");
	fprintf(cgiOut, "<p>\n");
	fprintf(cgiOut, "<input type=\"text\" name=\"frogs\" value=\"1\">\n");
	fprintf(cgiOut, "Frogs Eaten\n");
	fprintf(cgiOut, "<p>\n");
	fprintf(cgiOut, "Single-SELECT\n");
	fprintf(cgiOut, "<br>\n");
	fprintf(cgiOut, "<select name=\"colors\">\n");
	fprintf(cgiOut, "<option value=\"Red\">Red\n");
	fprintf(cgiOut, "<option value=\"Green\">Green\n");
	fprintf(cgiOut, "<option value=\"Blue\">Blue\n");
	fprintf(cgiOut, "</select>\n");
	fprintf(cgiOut, "<br>\n");
	fprintf(cgiOut, "Multiple-SELECT\n");
	fprintf(cgiOut, "<br>\n");
	fprintf(cgiOut, "<select name=\"flavors\" multiple>\n");
	fprintf(cgiOut, "<option value=\"pistachio\">Pistachio\n");
	fprintf(cgiOut, "<option value=\"walnut\">Walnut\n");
	fprintf(cgiOut, "<option value=\"creme\">Creme\n");
	fprintf(cgiOut, "</select>\n");
	fprintf(cgiOut, "<p>Exclusive Radio Button Group: Age of Truck in Years\n");
	fprintf(cgiOut, "<input type=\"radio\" name=\"age\" value=\"1\">1\n");
	fprintf(cgiOut, "<input type=\"radio\" name=\"age\" value=\"2\">2\n");
	fprintf(cgiOut, "<input type=\"radio\" name=\"age\" value=\"3\" checked>3\n");
	fprintf(cgiOut, "<input type=\"radio\" name=\"age\" value=\"4\">4\n");
	fprintf(cgiOut, "<p>Nonexclusive Checkbox Group: Voting for Zero through Four Candidates\n");
	fprintf(cgiOut, "<input type=\"checkbox\" name=\"vote\" value=\"A\">A\n");
	fprintf(cgiOut, "<input type=\"checkbox\" name=\"vote\" value=\"B\">B\n");
	fprintf(cgiOut, "<input type=\"checkbox\" name=\"vote\" value=\"C\">C\n");
	fprintf(cgiOut, "<input type=\"checkbox\" name=\"vote\" value=\"D\">D\n");
	fprintf(cgiOut, "<p>File Upload:\n");
	fprintf(cgiOut, "<input type=\"file\" name=\"file\" value=\"\"> (Select A Local File)\n");
	fprintf(cgiOut, "<p>\n");
	fprintf(cgiOut, "<p>Set a Cookie<p>\n");
	fprintf(cgiOut, "<input name=\"cname\" value=\"\"> Cookie Name\n");
	fprintf(cgiOut, "<input name=\"cvalue\" value=\"\"> Cookie Value<p>\n");
	fprintf(cgiOut, "<input type=\"submit\" name=\"testcgic\" value=\"Submit Request\">\n");
	fprintf(cgiOut, "<input type=\"reset\" value=\"Reset Request\">\n");
	fprintf(cgiOut, "<p>Save the CGI Environment<p>\n");
	fprintf(cgiOut, "Pressing this button will submit the form, then save the CGI environment so that it can be replayed later by calling cgiReadEnvironment (in a debugger, for instance).<p>\n");
	fprintf(cgiOut, "<input type=\"submit\" name=\"saveenvironment\" value=\"Save Environment\">\n");
	fprintf(cgiOut, "</form>\n");
}

void CookieSet()
{
	char cname[1024];
	char cvalue[1024];
	/* Must set cookies BEFORE calling cgiHeaderContentType */
	cgiFormString("cname", cname, sizeof(cname));	
	cgiFormString("cvalue", cvalue, sizeof(cvalue));	
	if (strlen(cname)) {
		/* Cookie lives for one day (or until browser chooses
			to get rid of it, which may be immediately),
			and applies only to this script on this site. */	
		cgiHeaderCookieSetString(cname, cvalue,
			86400, cgiScriptName, SERVER_NAME);
	}
}

void LoadEnvironment()
{
	if (cgiReadEnvironment(SAVED_ENVIRONMENT) != 
		cgiEnvironmentSuccess) 
	{
		cgiHeaderContentType("text/html");
		fprintf(cgiOut, "<head>Error</head>\n");
		fprintf(cgiOut, "<body><h1>Error</h1>\n");
		fprintf(cgiOut, "cgiReadEnvironment failed. Most "
			"likely you have not saved an environment "
			"yet.\n");
		exit(0);
	}
	/* OK, return now and show the results of the saved environment */
}

void SaveEnvironment()
{
	if (cgiWriteEnvironment(SAVED_ENVIRONMENT) != 
		cgiEnvironmentSuccess) 
	{
		fprintf(cgiOut, "<p>cgiWriteEnvironment failed. Most "
			"likely %s is not a valid path or is not "
			"writable by the user that the CGI program "
			"is running as.<p>\n", SAVED_ENVIRONMENT);
	} else {
		fprintf(cgiOut, "<p>Environment saved. Click this button "
			"to restore it, playing back exactly the same "
			"scenario: "
			"<form method=POST action=\"");
		cgiValueEscape(cgiScriptName);
		fprintf(cgiOut, "\">" 
			"<input type=\"submit\" "
			"value=\"Load Environment\" "
			"name=\"loadenvironment\"></form><p>\n");
	}
}
#endif
