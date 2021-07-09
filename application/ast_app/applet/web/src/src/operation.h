#ifndef __OPERATION_H__
#define __OPERATION_H__

#include "process_json.h"
#include "mutex.h"
#include <string>

using namespace std;

#define MAX_CMD_STR         512

#define UPLOAD_FILE_PATH "/tmp"
#define WEB_CONFIG_FILE "./gbwebserver.json"

// 文件传输文件默认路径
#define DEFAULT_FILE_PATH       "/opt/configs/kds-n-6X/usr/etc"
#define CHANNEL_FILE_PATH       "/channel/channel_map.json"
#define SLEEPIMAGE_FILE_PATH    "/display/sleep_image.png"
#define EDID_FILE_PATH          "/edid/custom_edid.bin"
#define LOG_FILE_PATH           "/log/log_file.txt"
#define OVERLAY_FILE_PATH       "/osd/overlay_image.png"
#define SECURE_FILE_PATH        "/secure/server.pem"
#define VIDEO_WALL_FILE_PATH    "/vw/video_wall_test_pattern.png"

// json文件路径
#define JSON_SWITCH_DELAY_PATH  "/switch/auto_switch_delays"
#define JSON_AV_SIGNAL_PATH     "/av_signal"
#define JSON_DISPLAY_SLEEP_PATH "/display/display_sleep"
#define JSON_EDID_PATH          "/edid/edid.json"
#define JSON_GATEWAY_PATH       "/gateway"
#define JSON_VERSION_PATH       "/version/version.json"
#define JSON_OSD_PATH           "/osd/overlay.json"
#define JSON_OVERLAY_PATH       "/overlay/overlay"
#define JSON_SECURE_PATH        "/secure/security_setting.json"
#define JSON_USB_PATH           "/usb/km_usb"

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
private:

    // 功能函数

    // 处理文件名重复
    static bool FileRename(const char *i_pFileName, const char *i_pFilePath, string &o_strRename);
    static int  FileToString(const char *i_pinFilePath, string &o_strDes);
    static int  StringToFile(const char *i_pStrBuff, const char *i_pOutFileath);
    static CMutex s_DeviceMutex;

};

#endif // __OPERATION_H__