#ifndef __OPERATION_H__
#define __OPERATION_H__

#include "Civetweb_API.h"
#include "process_json.h"
#include "mutex.h"
#include <string>

using namespace std;

#define MAX_CMD_STR         512

#define UPLOAD_FILE_PATH "/tmp"
#define WEB_CONFIG_FILE "./gbwebserver.json"

// 文件传输文件默认路径
#define DEFAULT_FILE_PATH       "/data/configs/kds-7"
#define CHANNEL_FILE_PATH       "/channel/channel_map.json"
#define SLEEPIMAGE_FILE_PATH    "/display/sleep_image.png"
#define EDID_FILE_PATH          "/edid/custom_edid.bin"
#define LOG_FILE_PATH           "/log/log_file.txt"
#define OVERLAY_FILE_PATH       "/osd/overlay_image.png"
#define SECURE_FILE_PATH        "/secure/server.pem"
#define VIDEO_WALL_FILE_PATH    "/vw/video_wall_test_pattern.png"

class COperation
{
public:
    static bool SetWebSecurityMode(const char *i_pMode);
    static bool SetCertificate(const char *i_pCert);

    // 文件传输
    static bool UpdateEdidFile(const char *i_pFilePath);

    // json数据处理函数
    static bool SetJsonFile(const char *i_pJsonData, const char *i_pFile);
    static bool GetJsonFile(const char *i_pJsonFile, string& o_strContent);

    // 原5000
    static bool SetPassword(const char *i_pNewPasswd);
    static bool VerifyPassword(char *i_pUsername, char *i_pPasswd);

private:

    // 功能函数

    // 处理文件名重复
    static bool FileRename(const char *i_pFileName, const char *i_pFilePath, string &o_strRename);
    static int  FileToString(const char *i_pinFilePath, string &o_strDes);
    static int  StringToFile(const char *i_pStrBuff, const char *i_pOutFileath);

    // 原5000
    static int  SavePasswordToParam(const char *i_pUsrName);
    static int  JudgePasswdEncrypt(char *username, char *passwd);

    static CMutex s_DeviceMutex;

};

#endif // __OPERATION_H__
