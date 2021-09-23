/* Change this if the SERVER_NAME environment variable does not report
    the true name of your web server. */
#define SERVER_NAME cgiServerName

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "cgic.h"

#define FILE_TEMP_PATH              "/dev/shm"
#define FILE_DEST_PATH              "/data"
#define FILE_SIZE_LIMIT             (8*1024)
#define FILE_NAME_LDAP_CA_PEM       "ldap_ca.pem"
#define FILE_NAME_802_CA_PEM        "802_ca.pem"
#define FILE_NAME_802_CLI_PEM       "802_client.pem"
#define FILE_NAME_802_CLI_KEY_PEM   "802_client_key.pem"

#define CGI_DEBUG   0

int upload_ok = 0;

void HandleSubmit();
void File();

#if CGI_DEBUG
FILE *pLogFile = NULL;
#define debug(format,args...) fprintf(pLogFile, format, ##args)
#else
#define debug(format,args...) do {} while(0)
#endif

void cgiOnCalled(void)
{
#if CGI_DEBUG
    pLogFile = fopen("/tmp/cgi.log", "w");
#endif
    return;
}

int cgiMain()
{
    HandleSubmit();
    return 0;
}

void HandleSubmit()
{
    File();
}

int do_async_update = 0;

int save_file(char *fname, const char* tmp_path)
{
    cgiFilePtr file = NULL;
    FILE *fp = NULL;
    char buffer[1024];
    int got;
    int len, ret = 0;

    if (cgiFormFileOpen(fname, &file) != cgiFormSuccess)
    {
        return 1;
    }

    fp = fopen(tmp_path, "w");

    if (fp == NULL)
    {
        ret = 1;
        goto done;
    }

    while (cgiFormFileRead(file, buffer, sizeof(buffer), &got) ==
            cgiFormSuccess)
    {
        len = fwrite(buffer, 1, got, fp);

        if (len != got)
        {
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
    if (!upload_ok)
    {
        cgiHeaderStatus(400, "Bad Request");
    }

    return 0;
}

void File(void)
{
    char tempBuf[1024];
    char destPath[1024];
    char tmpPath[1024];
    char sysCmd[1024];
    int size = 0;
    int ret  = 0;
    tempBuf[1023]  = '\0';
    destPath[1023] = '\0';
    tmpPath[1023]  = '\0';
    sysCmd[1023]   = '\0';

    if (cgiFormFileName("file", tempBuf, 1024) == cgiFormNotFound)
    {
        debug("cgiFormNotFound\n");
        return;
    }

    debug("File Name: %s\n", tempBuf);

    if (
        (0 == strcasecmp(FILE_NAME_LDAP_CA_PEM, tempBuf))
        || (0 == strcasecmp(FILE_NAME_802_CA_PEM, tempBuf))
        || (0 == strcasecmp(FILE_NAME_802_CLI_PEM, tempBuf))
        || (0 == strcasecmp(FILE_NAME_802_CLI_KEY_PEM, tempBuf))
    )
    {
        snprintf(tmpPath, 1023, "%s/%s", FILE_TEMP_PATH, tempBuf);
        snprintf(destPath, 1023, "%s/%s", FILE_DEST_PATH, tempBuf);
    }
    else
    {
        debug("No support for the file:%s\n", tempBuf);
        return;
    }

    cgiFormFileSize("file", &size);
    debug("File size:%d\n", size);

    if (size > FILE_SIZE_LIMIT)
    {
        return;
    }

    if (0 == save_file("file", tmpPath))
    {
        snprintf(sysCmd, 1023, "mv %s %s", tmpPath, destPath);
        (void)system(sysCmd);
        debug("Rename from %s to %s\n", tmpPath, destPath);
        upload_ok = 1;
    }

    cgiHeaderContentType("text/html");
}
