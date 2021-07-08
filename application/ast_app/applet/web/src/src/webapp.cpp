#include "webapp.h"
#include "debug.h"
#include "Civetweb_API.h"
#include "stringsplit.h"

#define UP_CHANNEL_URL      "/upload/channel"
#define UP_EDID_URL         "/upload/edid"
#define UP_SLEEPIMAGE_URL   "/upload/sleepimage"
#define UP_OVERLAY_URL      "/upload/overlay_image"
#define UP_SECURE_URL       "/upload/secure"
#define UP_TESTPATTERN_URL  "/upload/testpattern"

#define DOWN_LOGFILE_URL    "/log"
#define DOWN_CHANNEL_URL    "/channel/channel_map"
#define DOWN_SECURE_URL     "/secure"
#define DOWN_TESTPATTERN_URL "/vw/video_wall_test_pattern"

// json文件传输
#define GET_SWITCH_DELAY    "/switch/auto_switch_delays"
#define SET_SWITCH_DELAY    "/switch/set_auto_switch_delays"
#define GET_AV_SIGNAL       "/av_signal"
#define SET_AV_SIGNAL       "/set_av_signal"
#define GET_DISPLAY_SLEEP   "/display/display_sleep"
#define SET_DISPLAY_SLEEP   "/display/set_display_sleep"
#define GET_EDID            "/edid/edid"
#define SET_EDID            "/edid/set_edid"
#define GET_GATEWAY         "/gateway"
#define SET_GATEWAY         "/set_gateway"
#define GET_VERSION         "/version/version"
#define SET_VERSION         "/version/set_version"
#define GET_OSD             "/overlay/overlay"
#define SET_OSD             "/overlay/set_overlay"
#define GET_OVERLAY         "/overlay/overlay"
#define SET_OVERLAY         "/overlay/set_overlay"
#define GET_SECURE          "/secure/security_setting"
#define SET_SECURE          "/secure/set_security_setting"
#define GET_USB             "/usb/km_usb"
#define SET_USB             "/usb/set_km_usb"


typedef struct T_FromInfo
{
    char filename[KEY_VALUE_SIZE];
    char filepath[KEY_VALUE_SIZE];
}T_FromInfo;

CMutex CWeb::s_p3kmutex;
CCond  CWeb::s_p3kcond;
P3kStatus CWeb::s_p3kStatus;
struct mg_context * CWeb::ctx = NULL;
int CWeb::s_p3kSocket = -1;

bool CWeb::Start(ConfInfoParam * p_webparam,string Server_mode)
{
	/* Start CivetWeb web server */
	BC_INFO_LOG("server will start with %s",Server_mode.c_str());
	web_options options;
	options.web_allconf_documentroot = p_webparam->web_rootpath;
	options.web_allconf_http_port = p_webparam->web_httpport;
	options.web_allconf_https_port = p_webparam->web_httpsport;
	options.web_httpsconf_cerm = p_webparam->web_cermpath;
	options.web_allconf_thread_num = p_webparam->web_numthread;
	options.web_allconf_enable_keepalive = p_webparam->web_keepaliveenable;
	options.web_allconf_keepalive_timeout = p_webparam->web_keepalivetimems;

	options.web_wsconf_enable_pingpong = p_webparam->web_ws_pingpongenable;
	options.web_wsconf_pingpong_timeout_ms = p_webparam->web_ws_pingpongtimems;

	Server_Start_Mode pStart_Mode;
    if(!Server_mode.empty())
    {
        if(Server_mode == "all")
    	{
    		pStart_Mode = Server_Start_Mode_all;
    	}
    	else if(Server_mode == "https")
    	{
    		pStart_Mode = Server_Start_Mode_Https;
    	}
    	else
    	{
    		pStart_Mode = Server_Start_Mode_Http;
    	}
    }
    else
    {
        if(p_webparam->web_startmode == "both")
        {
            pStart_Mode = Server_Start_Mode_all;
        }
        else if(p_webparam->web_startmode == "https")
        {
            pStart_Mode = Server_Start_Mode_Https;
        }
        else
        {
            pStart_Mode = Server_Start_Mode_Http;
        }
    }

	//pStart_Mode = Server_Start_Mode_all;
	printf("pStart_Mode = %d\n", pStart_Mode);
	ctx = ServerStart(&options,pStart_Mode,NULL);
	if(NULL == ctx)
	{
		return false;
	}
	return true;
}

void CWeb::Stop()
{
	ServerStop(ctx);
	ctx = NULL;
}


void CWeb::WebSocketRun(mg_websocket_handler handler,void *cbdata)
{
	SetWebsocketHandler(ctx,handler,cbdata);
}


void CWeb::HttpRun()
{
	/*uri handler list demon*/
	struct uri_list UriHandleList[] =
	{
        //{"/setcert$", SetCertificateHandle, NULL},
        //{"/update$", UploadCertificateHandle, NULL},
        {UP_CHANNEL_URL, UploadChannelMapHandle, NULL},
        {DOWN_CHANNEL_URL, DownChannelMapHandle, NULL},
        {"/preview", ShowSleepImageHandle, NULL}, // 暂时
        {UP_SLEEPIMAGE_URL, UpdateSleepImageHandle, NULL},
        {UP_EDID_URL, UpdateEdidHandle, NULL},
        {DOWN_LOGFILE_URL, DownloadLogFileHandle, NULL},
        {UP_OVERLAY_URL, UpdateOverlayImageHandle, NULL},
        {UP_SECURE_URL, UploadSecurHandle, NULL},
        {DOWN_SECURE_URL, DownSecurHandle, NULL},
        {UP_TESTPATTERN_URL, UploadVideoWallHandle, NULL},
        {DOWN_TESTPATTERN_URL, DownVideoWallHandle, NULL},
        {SET_SWITCH_DELAY, SetSwitchDelayHandle, NULL},
        {GET_SWITCH_DELAY, GetSwitchDelayHandle, NULL},
        {SET_AV_SIGNAL, SetAVSignalHandle, NULL},
        {GET_AV_SIGNAL, GetAVSignalHandle, NULL},
        {SET_DISPLAY_SLEEP, SetDisplaySleepHandle, NULL},
        {GET_DISPLAY_SLEEP, GetDisplaySleepHandle, NULL},
        {SET_EDID, SetEdidHandle, NULL},
        {GET_EDID, GetEdidHandle, NULL},
        {SET_GATEWAY, SetGatewayHandle, NULL},
        {GET_GATEWAY, GetGatewayHandle, NULL},
        {SET_VERSION, SetVersionHandle, NULL},
        {GET_VERSION, GetVersionHandle, NULL},
        {SET_OSD, SetOsdHandle, NULL},
        {GET_OSD, GetOsdHandle, NULL},
        {SET_OVERLAY, SetOverlayHandle, NULL},
        {GET_OVERLAY, GetOverlayHandle, NULL},
        {SET_SECURE, SetSecureHandle, NULL},
        {GET_SECURE, GetSecureHandle, NULL},
        {SET_USB, SetUsbHandle, NULL},
        {GET_USB, GetUsbHandle, NULL},
	};
	AddURIProcessHandler(ctx,UriHandleList,ARRAY_SIZE(UriHandleList));
}

void CWeb::Print_ServerPort()
{
	List_All_ServerPort(ctx);
}

int CWeb::file_found(const char *key,const char *filename,char *path,size_t pathlen,void *user_data)
{
	struct mg_connection *conn = (struct mg_connection *)user_data;
    struct T_FromInfo *pFrom = (struct T_FromInfo *)mg_get_user_connection_data(conn);

	BC_INFO_LOG("the key is %s  the filename is %s",key,filename);
    if (filename && *filename)
    {
        if(strlen(pFrom->filename) == 0)
        {
	        snprintf(path, pathlen, "%s/%s", pFrom->filepath, filename);
            strncpy(pFrom->filename, filename, strlen(filename));
        }
        else
        {
            snprintf(path, pathlen, "%s/%s", pFrom->filepath, pFrom->filename);
        }
        BC_INFO_LOG("file path is %s",path);
        mg_set_user_connection_data(conn, (void*)path);
	    return MG_FORM_FIELD_STORAGE_STORE;
    }
    return MG_FORM_FIELD_HANDLE_ABORT;
}
int CWeb::file_get(const char *key,const char *value,size_t valuelen,void *user_data)
{
	struct mg_connection *conn = (struct mg_connection *)user_data;

}

int CWeb::file_store(const char *path, long long file_size, void *user_data)
{
	struct mg_connection *conn = (struct mg_connection *)user_data;
    BC_INFO_LOG("field_stored path %s  file_size  %lld ", path, file_size);

    return 0;
}

bool CWeb::SaveUploadFile(struct mg_connection *conn, const char *i_pPath, const char *i_pFileNmae, struct T_FromInfo *o_tFrominfo)
{
    memset(o_tFrominfo, 0, sizeof(struct T_FromInfo));
    strncpy(o_tFrominfo->filepath, i_pPath, strlen(i_pPath));
    if((i_pFileNmae != NULL) && (strlen(i_pFileNmae) != 0))
    {
        strncpy(o_tFrominfo->filename, i_pFileNmae, strlen(i_pFileNmae));
    }
    mg_set_user_connection_data(conn,(void *)o_tFrominfo);

    struct mg_form_data_handler fdh = {file_found, file_get, file_store, 0};
    fdh.user_data = (void *)conn;

    int ret = Set_Formdata_Handler(conn, &fdh);
    if(ret < 0)
    {
        BC_ERROR_LOG("CWeb::UploadFile Set_Formdata_Handler <%d>", ret);
        return false;
    }

    return true;
}


int CWeb::UploadCertificateHandle(struct mg_connection *conn, void *cbdata)
{
    struct mg_form_data_handler fdh = {file_found, file_get, file_store, 0};
    fdh.user_data = (void *)conn;
    int ret = mg_handle_form_request(conn, &fdh);

    string strFileName = (char *)mg_get_user_connection_data(conn);
    printf("UploadCertificateHandle strFileName is <%s>\n", strFileName.c_str());

    send_http_ok_rsp(conn);
    return 1;
}

int CWeb::SetCertificateHandle(struct mg_connection *conn, void *cbdata)
{
    if(!COperation::SetCertificate(SECURE_FILE_PATH))
    {
       printf("set error");
    }

    send_http_ok_rsp(conn);
    return 1;
}


// 文件传输
int CWeb::UploadChannelMapHandle(struct mg_connection *conn, void *cbdata)
{
    const struct mg_request_info *pRequest = mg_get_request_info(conn);

    if(strcmp(pRequest->request_method, "POST") == 0)
    {

        struct T_FromInfo tFrom;
        if(!SaveUploadFile(conn, DEFAULT_FILE_PATH "/channel", "channel_map.json", &tFrom))
        //if(!SaveUploadFile(conn, "/home", NULL, &tFrom))
        {
            BC_INFO_LOG("TransmitChannelMapHandle upload file error");
        }
        else
        {
            BC_INFO_LOG("TransmitChannelMapHandle upload file OK");
            send_http_ok_rsp(conn);
            return 1;
        }
    }
    send_http_error_rsp(conn);

    return 1;
}

int CWeb::DownChannelMapHandle(struct mg_connection *conn, void *cbdata)
{
    const struct mg_request_info *pRequest = mg_get_request_info(conn);

    if(strcmp(pRequest->request_method, "GET") == 0)
    {
        send_chunk_file(conn, CHANNEL_FILE_PATH, "channel_map.json");
        //send_chunk_file(conn, "/home/1.txt", "1.txt");
    }
    else
    {
        send_http_error_rsp(conn);
    }

    return 1;
}

int CWeb::ShowSleepImageHandle(struct mg_connection *conn, void *cbdata)
{
    string strFile = DEFAULT_FILE_PATH;
    strFile += SLEEPIMAGE_FILE_PATH;

    mg_send_file(conn, strFile.c_str());
    return 1;
}

int CWeb::UpdateSleepImageHandle(struct mg_connection *conn, void *cbdata)
{
    struct T_FromInfo tFrom;
    if(!SaveUploadFile(conn, DEFAULT_FILE_PATH "/display", "sleep_image.png", &tFrom))
    {
        BC_INFO_LOG("UpdateSleepImageHandle upload file error");
    }
    else
    {
        BC_INFO_LOG("UpdateSleepImageHandle upload file OK");
        send_http_ok_rsp(conn);
        return 1;
    }
    send_http_error_rsp(conn);

    return 1;
}

int CWeb::UpdateEdidHandle(struct mg_connection *conn, void *cbdata)
{
    struct T_FromInfo tFrom;
    if(!SaveUploadFile(conn, "/tmp", NULL, &tFrom))
    {
        BC_INFO_LOG("UpdateSleepImageHandle upload file error");
    }
    else
    {
        char aTmpName[MAX_KEY_VALUE_SIZE] = {0};
        sprintf(aTmpName, "%s/%s", tFrom.filepath, tFrom.filename);

        if(COperation::UpdateEdidFile(aTmpName))
        {
            send_http_ok_rsp(conn);
            return 1;
        }
    }
    send_http_error_rsp(conn);

    return 1;
}

int CWeb::DownloadLogFileHandle(struct mg_connection *conn, void *cbdata)
{
    send_chunk_file(conn, "/log/log_file.txt", "log_file.txt");

    return 1;
}

int CWeb::UpdateOverlayImageHandle(struct mg_connection *conn, void *cbdata)
{
    struct T_FromInfo tFrom;
    if(!SaveUploadFile(conn, DEFAULT_FILE_PATH "/osd", "overlay_image.png", &tFrom))
    {
        BC_INFO_LOG("UpdateOverlayImageHandle upload file error");
    }
    else
    {
        BC_INFO_LOG("UpdateOverlayImageHandle upload file OK");
        send_http_ok_rsp(conn);
        return 1;
    }
    send_http_error_rsp(conn);

    return 1;
}


/*
int CWeb::TransmitPresetsFileHandle(struct mg_connection *conn, void *cbdata)
{
    // 获取请求模式
    const struct mg_request_info *pRequest = mg_get_request_info(conn);
    if(strcmp(pRequest->request_method, "GET") == 0)
    {
        mg_send_file(conn, "/opt/configs/kds-n-6X/usr/etc/presets/preset1.json");

        return 1;
    }

    if(strcmp(pRequest->request_method, "POST") == 0)
    {
        struct mg_form_data_handler fdh = {file_found, file_get, file_store, 0};
        fdh.user_data = (void *)conn;

        int ret = Set_Formdata_Handler(conn, &fdh);
        if(ret < 0)
        {
            BC_ERROR_LOG("CWeb::TransmitPresetsFileHandle Set_Formdata_Handler <%d>", ret);
        }

        // 获取下载的文件路径
        string strFileName = (char *)mg_get_user_connection_data(conn);

        // 将获取到的文件移动到指定位置(函数)

        // 返回成功响应
        send_http_ok_rsp(conn);

        return 1;
    }

    send_http_error_rsp(conn);

    return 1;
}
*/

int CWeb::UploadSecurHandle(struct mg_connection *conn, void *cbdata)
{
    const struct mg_request_info *pRequest = mg_get_request_info(conn);

    if(strcmp(pRequest->request_method, "POST") == 0)
    {

        struct T_FromInfo tFrom;
        if(!SaveUploadFile(conn, "/secure", "server.pem", &tFrom))
        {
            BC_INFO_LOG("TransmitSecurHandle upload file error");
        }
        else
        {
            BC_INFO_LOG("TransmitSecurHandle upload file OK");
            send_http_ok_rsp(conn);
            return 1;
        }
    }
    send_http_error_rsp(conn);

    return 1;
}

int CWeb::DownSecurHandle(struct mg_connection *conn, void *cbdata)
{
    const struct mg_request_info *pRequest = mg_get_request_info(conn);

    if(strcmp(pRequest->request_method, "GET") == 0)
    {
        send_chunk_file(conn, "/secure/server.pem", "server.pem");
    }
    else
    {
        send_http_error_rsp(conn);
    }

    return 1;
}

int CWeb::UploadVideoWallHandle(struct mg_connection *conn, void *cbdata)
{
    const struct mg_request_info *pRequest = mg_get_request_info(conn);

    if(strcmp(pRequest->request_method, "POST") == 0)
    {

        struct T_FromInfo tFrom;
        if(!SaveUploadFile(conn, DEFAULT_FILE_PATH "/vw", "video_wall_test_pattern.png", &tFrom))
        {
            BC_INFO_LOG("TransmitVideoWallHandle upload file error");
        }
        else
        {
            BC_INFO_LOG("TransmitVideoWallHandle upload file OK");
            send_http_ok_rsp(conn);
            return 1;
        }
    }
    send_http_error_rsp(conn);

    return 1;
}

int CWeb::DownVideoWallHandle(struct mg_connection *conn, void *cbdata)
{
    const struct mg_request_info *pRequest = mg_get_request_info(conn);

    if(strcmp(pRequest->request_method, "GET") == 0)
    {
        send_chunk_file(conn, DEFAULT_FILE_PATH "/vw/video_wall_test_pattern.png", "video_wall_test_pattern.png");
    }
    else
    {
        send_http_error_rsp(conn);
    }

    return 1;
}

int CWeb::SetSwitchDelayHandle(struct mg_connection *conn, void *cbdata)
{
    // 获取前端来的数据
    size_t sLen = 0;
    char szContent[MG_READ_BUFSIZE] = {0};
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_SWITCH_DELAY_PATH;

    sLen = BC_GET_Request_Body(conn, szContent, MG_READ_BUFSIZE);
    BC_INFO_LOG("SetSwitchDelayHand request body is <%s>", szContent);
    if(sLen < 0)
    {
        BC_INFO_LOG("SetSwitchDelayHand get body is failed");
    }
    else
    {
        if(COperation::SetJsonFile(szContent, strFile.c_str()))
        //if(COperation::SetJsonFile(szContent, "./auto_switch_delays"))
        {
            send_http_ok_rsp(conn);
            return 1;
        }
    }
    send_http_error_rsp(conn);

    return 1;
}

int CWeb::GetSwitchDelayHandle(struct mg_connection *conn, void *cbdata)
{
    string strConfigInfo = "";
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_SWITCH_DELAY_PATH;

    if(COperation::GetJsonFile(strFile.c_str(), strConfigInfo))
    //if(COperation::GetJsonFile("./auto_switch_delays", strConfigInfo))
    {
        BC_INFO_LOG("GetSwitchDelayHand strConfigInfo is <%s>", strConfigInfo.c_str());
        BC_mg_send_header(conn, "Content-Type", "application/json");
        mg_printf_data(conn, true, "%s", strConfigInfo.c_str());
        mg_printf(conn, "%x\r\n\r\n", 0x00);
    }
    else
    {
        send_http_error_rsp(conn);
    }

    return 1;
}

int CWeb::SetAVSignalHandle(struct mg_connection *conn, void *cbdata)
{
    size_t sLen = 0;
    char szContent[MG_READ_BUFSIZE] = {0};
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_AV_SIGNAL_PATH;

    sLen = BC_GET_Request_Body(conn, szContent, MG_READ_BUFSIZE);
    BC_INFO_LOG("SetAVSignalHandle request body is <%s>", szContent);
    if(sLen < 0)
    {
        BC_INFO_LOG("SetAVSignalHandle get body is failed");
    }
    else
    {
        if(COperation::SetJsonFile(szContent, strFile.c_str()))
        {
            send_http_ok_rsp(conn);
            return 1;
        }
    }
    send_http_error_rsp(conn);

    return 1;
}

int CWeb::GetAVSignalHandle(struct mg_connection *conn, void *cbdata)
{
    string strConfigInfo = "";
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_AV_SIGNAL_PATH;
    if(COperation::GetJsonFile(strFile.c_str(), strConfigInfo))
    {
        BC_INFO_LOG("GetAVSignalHandle strConfigInfo is <%s>", strConfigInfo.c_str());
        BC_mg_send_header(conn, "Content-Type", "application/json");
        mg_printf_data(conn, true, "%s", strConfigInfo.c_str());
        mg_printf(conn, "%x\r\n\r\n", 0x00);
    }
    else
    {
        send_http_error_rsp(conn);
    }

    return 1;
}

int CWeb::SetDisplaySleepHandle(struct mg_connection *conn, void *cbdata)
{
    size_t sLen = 0;
    char szContent[MG_READ_BUFSIZE] = {0};
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_DISPLAY_SLEEP_PATH;

    sLen = BC_GET_Request_Body(conn, szContent, MG_READ_BUFSIZE);
    BC_INFO_LOG("SetDisplaySleepHandle request body is <%s>", szContent);
    if(sLen < 0)
    {
        BC_INFO_LOG("SetDisplaySleepHandle get body is failed");
    }
    else
    {
        if(COperation::SetJsonFile(szContent, strFile.c_str()))
        {
            send_http_ok_rsp(conn);
            return 1;
        }
    }
    send_http_error_rsp(conn);

    return 1;
}

int CWeb::GetDisplaySleepHandle(struct mg_connection *conn, void *cbdata)
{
    string strConfigInfo = "";
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_DISPLAY_SLEEP_PATH;

    if(COperation::GetJsonFile(strFile.c_str(), strConfigInfo))
    {
        BC_INFO_LOG("GetDisplaySleepHandle strConfigInfo is <%s>", strConfigInfo.c_str());
        BC_mg_send_header(conn, "Content-Type", "application/json");
        mg_printf_data(conn, true, "%s", strConfigInfo.c_str());
        mg_printf(conn, "%x\r\n\r\n", 0x00);
    }
    else
    {
        send_http_error_rsp(conn);
    }

    return 1;
}

int CWeb::SetEdidHandle(struct mg_connection *conn, void *cbdata)
{
    size_t sLen = 0;
    char szContent[MG_READ_BUFSIZE] = {0};
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_EDID_PATH;

    sLen = BC_GET_Request_Body(conn, szContent, MG_READ_BUFSIZE);
    BC_INFO_LOG("SetEdidHandle request body is <%s>", szContent);
    if(sLen < 0)
    {
        BC_INFO_LOG("SetEdidHandle get body is failed");
    }
    else
    {
        if(COperation::SetJsonFile(szContent, strFile.c_str()))
        {
            send_http_ok_rsp(conn);
            return 1;
        }
    }
    send_http_error_rsp(conn);

    return 1;
}

int CWeb::GetEdidHandle(struct mg_connection *conn, void *cbdata)
{
    string strConfigInfo = "";
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_EDID_PATH;

    if(COperation::GetJsonFile(strFile.c_str(), strConfigInfo))
    {
        BC_INFO_LOG("GetEdidHandle strConfigInfo is <%s>", strConfigInfo.c_str());
        BC_mg_send_header(conn, "Content-Type", "application/json");
        mg_printf_data(conn, true, "%s", strConfigInfo.c_str());
        mg_printf(conn, "%x\r\n\r\n", 0x00);
    }
    else
    {
        send_http_error_rsp(conn);
    }

    return 1;
}

int CWeb::SetGatewayHandle(struct mg_connection *conn, void *cbdata)
{
    size_t sLen = 0;
    char szContent[MG_READ_BUFSIZE] = {0};
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_GATEWAY_PATH;

    sLen = BC_GET_Request_Body(conn, szContent, MG_READ_BUFSIZE);
    BC_INFO_LOG("SetGatewayHandle request body is <%s>", szContent);
    if(sLen < 0)
    {
        BC_INFO_LOG("SetGatewayHandle get body is failed");
    }
    else
    {
        if(COperation::SetJsonFile(szContent, strFile.c_str()))
        {
            send_http_ok_rsp(conn);
            return 1;
        }
    }
    send_http_error_rsp(conn);

    return 1;
}

int CWeb::GetGatewayHandle(struct mg_connection *conn, void *cbdata)
{
    string strConfigInfo = "";
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_GATEWAY_PATH;

    if(COperation::GetJsonFile(strFile.c_str(), strConfigInfo))
    {
        BC_INFO_LOG("GetGatewayHandle strConfigInfo is <%s>", strConfigInfo.c_str());
        BC_mg_send_header(conn, "Content-Type", "application/json");
        mg_printf_data(conn, true, "%s", strConfigInfo.c_str());
        mg_printf(conn, "%x\r\n\r\n", 0x00);
    }
    else
    {
        send_http_error_rsp(conn);
    }

    return 1;
}

int CWeb::SetVersionHandle(struct mg_connection *conn, void *cbdata)
{
    size_t sLen = 0;
    char szContent[MG_READ_BUFSIZE] = {0};
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_VERSION_PATH;

    sLen = BC_GET_Request_Body(conn, szContent, MG_READ_BUFSIZE);
    BC_INFO_LOG("SetVersionHandle request body is <%s>", szContent);
    if(sLen < 0)
    {
        BC_INFO_LOG("SetVersionHandle get body is failed");
    }
    else
    {
        if(COperation::SetJsonFile(szContent, strFile.c_str()))
        {
            send_http_ok_rsp(conn);
            return 1;
        }
    }
    send_http_error_rsp(conn);

    return 1;
}

int CWeb::GetVersionHandle(struct mg_connection *conn, void *cbdata)
{
    string strConfigInfo = "";
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_VERSION_PATH;

    if(COperation::GetJsonFile(strFile.c_str(), strConfigInfo))
    {
        BC_INFO_LOG("GetVersionHandle strConfigInfo is <%s>", strConfigInfo.c_str());
        BC_mg_send_header(conn, "Content-Type", "application/json");
        mg_printf_data(conn, true, "%s", strConfigInfo.c_str());
        mg_printf(conn, "%x\r\n\r\n", 0x00);
    }
    else
    {
        send_http_error_rsp(conn);
    }

    return 1;
}

int CWeb::SetOsdHandle(struct mg_connection *conn, void *cbdata)
{
    size_t sLen = 0;
    char szContent[MG_READ_BUFSIZE] = {0};
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_OSD_PATH;

    sLen = BC_GET_Request_Body(conn, szContent, MG_READ_BUFSIZE);
    BC_INFO_LOG("SetOsdHandle request body is <%s>", szContent);
    if(sLen < 0)
    {
        BC_INFO_LOG("SetOsdHandle get body is failed");
    }
    else
    {
        if(COperation::SetJsonFile(szContent, strFile.c_str()))
        {
            send_http_ok_rsp(conn);
            return 1;
        }
    }
    send_http_error_rsp(conn);

    return 1;
}

int CWeb::GetOsdHandle(struct mg_connection *conn, void *cbdata)
{
    string strConfigInfo = "";
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_OSD_PATH;

    if(COperation::GetJsonFile(strFile.c_str(), strConfigInfo))
    {
        BC_INFO_LOG("GetOsdHandle strConfigInfo is <%s>", strConfigInfo.c_str());
        BC_mg_send_header(conn, "Content-Type", "application/json");
        mg_printf_data(conn, true, "%s", strConfigInfo.c_str());
        mg_printf(conn, "%x\r\n\r\n", 0x00);
    }
    else
    {
        send_http_error_rsp(conn);
    }

    return 1;
}

int CWeb::SetOverlayHandle(struct mg_connection *conn, void *cbdata)
{
    size_t sLen = 0;
    char szContent[MG_READ_BUFSIZE] = {0};
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_OVERLAY_PATH;

    sLen = BC_GET_Request_Body(conn, szContent, MG_READ_BUFSIZE);
    BC_INFO_LOG("SetOverlayHandle request body is <%s>", szContent);
    if(sLen < 0)
    {
        BC_INFO_LOG("SetOverlayHandle get body is failed");
    }
    else
    {
        if(COperation::SetJsonFile(szContent, strFile.c_str()))
        {
            send_http_ok_rsp(conn);
            return 1;
        }
    }
    send_http_error_rsp(conn);

    return 1;
}

int CWeb::GetOverlayHandle(struct mg_connection *conn, void *cbdata)
{
    string strConfigInfo = "";
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_OVERLAY_PATH;

    if(COperation::GetJsonFile(strFile.c_str(), strConfigInfo))
    {
        BC_INFO_LOG("GetOverlayHandle strConfigInfo is <%s>", strConfigInfo.c_str());
        BC_mg_send_header(conn, "Content-Type", "application/json");
        mg_printf_data(conn, true, "%s", strConfigInfo.c_str());
        mg_printf(conn, "%x\r\n\r\n", 0x00);
    }
    else
    {
        send_http_error_rsp(conn);
    }

    return 1;
}

int CWeb::SetSecureHandle(struct mg_connection *conn, void *cbdata)
{
    size_t sLen = 0;
    char szContent[MG_READ_BUFSIZE] = {0};
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_SECURE_PATH;

    sLen = BC_GET_Request_Body(conn, szContent, MG_READ_BUFSIZE);
    BC_INFO_LOG("SetSecureHandle request body is <%s>", szContent);
    if(sLen < 0)
    {
        BC_INFO_LOG("SetSecureHandle get body is failed");
    }
    else
    {
        if(COperation::SetJsonFile(szContent, strFile.c_str()))
        {
            send_http_ok_rsp(conn);
            return 1;
        }
    }
    send_http_error_rsp(conn);

    return 1;
}

int CWeb::GetSecureHandle(struct mg_connection *conn, void *cbdata)
{
    string strConfigInfo = "";
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_SECURE_PATH;

    if(COperation::GetJsonFile(strFile.c_str(), strConfigInfo))
    {
        BC_INFO_LOG("GetSecureHandle strConfigInfo is <%s>", strConfigInfo.c_str());
        BC_mg_send_header(conn, "Content-Type", "application/json");
        mg_printf_data(conn, true, "%s", strConfigInfo.c_str());
        mg_printf(conn, "%x\r\n\r\n", 0x00);
    }
    else
    {
        send_http_error_rsp(conn);
    }

    return 1;
}

int CWeb::SetUsbHandle(struct mg_connection *conn, void *cbdata)
{
    size_t sLen = 0;
    char szContent[MG_READ_BUFSIZE] = {0};
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_USB_PATH;

    sLen = BC_GET_Request_Body(conn, szContent, MG_READ_BUFSIZE);
    BC_INFO_LOG("SetUsbHandle request body is <%s>", szContent);
    if(sLen < 0)
    {
        BC_INFO_LOG("SetUsbHandle get body is failed");
    }
    else
    {
        if(COperation::SetJsonFile(szContent, strFile.c_str()))
        {
            send_http_ok_rsp(conn);
            return 1;
        }
    }
    send_http_error_rsp(conn);

    return 1;
}

int CWeb::GetUsbHandle(struct mg_connection *conn, void *cbdata)
{
    string strConfigInfo = "";
    string strFile = DEFAULT_FILE_PATH;
    strFile += JSON_USB_PATH;

    if(COperation::GetJsonFile(strFile.c_str(), strConfigInfo))
    {
        BC_INFO_LOG("GetUsbHandle strConfigInfo is <%s>", strConfigInfo.c_str());
        BC_mg_send_header(conn, "Content-Type", "application/json");
        mg_printf_data(conn, true, "%s", strConfigInfo.c_str());
        mg_printf(conn, "%x\r\n\r\n", 0x00);
    }
    else
    {
        send_http_error_rsp(conn);
    }

    return 1;
}

void CWeb::P3kStatusInit()
{
    s_p3kStatus.p3k_conn = NULL;
    s_p3kStatus.p3k_writeok = false;
    s_p3kStatus.p3k_start = 1;
}

void CWeb::P3kWebsocketHandle(struct mg_connection *conn, char *data, size_t len)
{
    BC_INFO_LOG("websocket get data is <%s>", data);
    if(s_p3kSocket < 0)
    {
        BC_INFO_LOG("p3k socket error");
        return;
    }

	int maxfd;
	fd_set Writefds;

	maxfd = s_p3kSocket + 1;
	FD_ZERO(&Writefds);
	FD_SET(s_p3kSocket, &Writefds);

	int nret = select(maxfd, NULL, &Writefds, NULL, NULL);
	if(nret < 0)
	{
		BC_INFO_LOG("P3kWebsocketHandle select failed");
	}

	if(nret > 0)
	{
		if(FD_ISSET(s_p3kSocket,&Writefds))
		{
		    s_p3kmutex.Lock();
            if(s_p3kStatus.p3k_start == 1)
            {
                int WriteSize = write(s_p3kSocket, data, len - 1);
    			if(WriteSize < 0)
    			{
    				BC_INFO_LOG("P3kWebsocketHandle send data failed");
    			}
                s_p3kStatus.p3k_start = 0;
                s_p3kStatus.p3k_conn = conn;
                s_p3kmutex.Unlock();
            }
            else
            {
                s_p3kcond.Wait(&s_p3kmutex, 1500);
                while(s_p3kStatus.p3k_writeok)
                {
        			//int WriteSize = write(s_p3kSocket, "#KDS_AUD-OUTPUT?\r", strlen("#KDS_AUD-OUTPUT?\r"));
        			int WriteSize = write(s_p3kSocket, data, len - 1);
        			if(WriteSize < 0)
        			{
        				BC_INFO_LOG("P3kWebsocketHandle send data failed");
        			}
                    s_p3kStatus.p3k_writeok = false;
                    s_p3kStatus.p3k_conn = conn;

                    s_p3kmutex.Unlock();
                }
            }
		}
	}

}

void *CWeb::P3kCommunicationThread(void *arg)
{
    int nret;
	int maxfd = -1;
	fd_set Readfds;
	struct timeval timeout = {3, 0};
	struct sockaddr_in tServerAddr;
    struct sockaddr_in tClientAddr;

	// 初始化sockaddr结构体
    memset(&tServerAddr, 0, sizeof(tServerAddr));
    tServerAddr.sin_family = AF_INET;
    tServerAddr.sin_port = htons(5000);
    tServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    memset(&tClientAddr, 0, sizeof(tClientAddr));
    tClientAddr.sin_family = AF_INET;
    tClientAddr.sin_port = htons(5001);
    tClientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// 创建套接字
    s_p3kSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(s_p3kSocket < 0)
    {
        BC_ERROR_LOG("socket create error");
        return NULL;
    }

    int op = 1;
    nret = setsockopt(s_p3kSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&op, sizeof(op));
    if(nret < 0)
    {
        BC_ERROR_LOG("socket ReuseAddr error");
        return NULL;
    }

    int nBufSize = 0;
    nret = setsockopt(s_p3kSocket, SOL_SOCKET, SO_SNDBUF, (const char*)&nBufSize, sizeof(nBufSize));
    if(nret < 0)
    {
        BC_ERROR_LOG("socket NO Delay error");
        return NULL;
    }

    // bind
    nret = bind(s_p3kSocket, (struct sockaddr *)&tClientAddr, sizeof(tClientAddr));
    if(nret < 0)
    {
        BC_ERROR_LOG("socket bind error");
        return NULL;
    }

	// 连接服务器
    nret = connect(s_p3kSocket, (struct sockaddr *)&tServerAddr, sizeof(tServerAddr));
    if(nret < 0)
    {
        BC_ERROR_LOG("connect error");
        return NULL;
    }

	// 初始化p3k通信
	char aFlag[KEY_VALUE_SIZE] = {0};
	char aRecv[KEY_VALUE_SIZE] = {0};

    strncpy(aFlag, "#LOGIN admin,33333\r", strlen("#LOGIN admin,33333\r"));
	int len = write(s_p3kSocket, aFlag, strlen(aFlag));
    if(len < 0)
    {
    	BC_INFO_LOG("p3k init write faild");
    	close(s_p3kSocket);
        return NULL;
    }

    len = read(s_p3kSocket, aRecv, KEY_VALUE_SIZE);
    if(len < 0)
    {
        BC_INFO_LOG("p3k init read faild");
    	close(s_p3kSocket);
        return NULL;
    }
	else
	{
	    BC_INFO_LOG("aRecv is <%s>", aRecv);
		if(strncmp(aRecv, "~01@LOGIN admin,33333,ok\r", strlen("~01@LOGIN admin,33333,ok\r")) != 0)
		{
			BC_INFO_LOG("p3k init is check failed");
	    	close(s_p3kSocket);
	        return NULL;
		}
	}

    FD_ZERO(&Readfds);
	FD_SET(s_p3kSocket, &Readfds);
	while(1)
	{
		char aBuff[MG_READ_BUFSIZE] = {0};
		maxfd = s_p3kSocket + 1;

		nret = select(maxfd, &Readfds, NULL, NULL, NULL);
		if(nret < 0)
		{
			BC_INFO_LOG("p3k read error");
			close(s_p3kSocket);
			return NULL;
		}

	    if(FD_ISSET(s_p3kSocket,&Readfds))
        {
            int ret = read(s_p3kSocket, aBuff, MG_READ_BUFSIZE);
			if(ret < 0)
			{
				BC_INFO_LOG("p3k read data failed");
				continue;
			}
            else if(ret == 0)
            {
                BC_INFO_LOG("p3k is close");
                close(s_p3kSocket);
                return NULL;
            }
			else
			{
			    s_p3kmutex.Lock();
			    BC_INFO_LOG("p3k read data is <%s> ret is <%d>", aBuff, ret);
                Send_Data_To_CurWebsocket(s_p3kStatus.p3k_conn, aBuff, ret);
				//Send_Data_To_ALLWebsocket(aBuff, (size_t)ret);

                s_p3kStatus.p3k_writeok = true;
                s_p3kcond.Signal();
				s_p3kmutex.Unlock();
			}
        }
	}
}


void CWeb::CloseP3kSocket()
{
    close(s_p3kSocket);
}


