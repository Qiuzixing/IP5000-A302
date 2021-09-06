/****************************************************************************
* °æÈ¨ĞÅÏ¢£º£¨C£©2020£¬ÉîÛÚÊĞ°Î³¬¿Æ¼¼ÓĞÏŞ¹«Ë¾°æÈ¨ËùÓĞ
* ÏµÍ³Ãû³Æ£º
* ÎÄ¼şÃû³Æ£ºwebapp.h
* ÎÄ¼şËµÃ÷£º¸ÃÎÄ¼şÊÇÀûÓÃcivetweb.hÌá¹©µÄ½Ó¿Ú£¬·â×°ÁËÒ»Ğ©¼òµ¥ÒµÎñÂß¼­£¬
* ×÷    Õß£ºzp
* °æ±¾ĞÅÏ¢£º1.0
* Éè¼ÆÈÕÆÚ£º2020-9-4
* ĞŞ¸Ä¼ÇÂ¼£º
* ÈÕ    ÆÚ                °æ    ±¾                ĞŞ¸ÄÈË                 ĞŞ¸ÄÕªÒª
****************************************************************************/

#ifndef __WEBAPP_H__
#define __WEBAPP_H__


#include "Civetweb_API.h"
#include "operation.h"
#include "process_json.h"
#include "mutex.h"
#include "cond.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// æš‚æ—¶(å¥—æ¥å­—ç›¸å…³)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MG_READ_BUFSIZE 	2*1024
#define MAX_PARAM_LEN       128
#define KEY_VALUE_SIZE      256
#define MAX_KEY_VALUE_SIZE  1024

typedef struct P3kStatus
{
    struct mg_connection *p3k_conn;
    Bool16 p3k_writeok;
    int p3k_start;
}P3kStatus;

typedef struct conn_state
{
    time_t last_poll;
    int request_type;
    FILE* fp;
    char var[MAX_PARAM_LEN];
    char boundary[MAX_PARAM_LEN];
    long long flen;
    unsigned int last_mjpeg_seq;
}conn_state;

enum {
    request_default = 0,
    request_fast_cgi,
    request_mjpeg,
    request_upload_logo,
    request_upload_bg_pic,
    request_dl_bmp,
    request_dl_jpg
};


class CWeb
{
public:
	static bool			Start(ConfInfoParam * p_webparam,string Server_mode);
	static void         Stop();
    static void         HttpRun();
	static void			WebSocketRun(mg_websocket_handler handler,void *cbdata);

	static void			Print_ServerPort();

	static int 			file_found(const char *key,const char *filename,char *path,size_t pathlen,void *user_data);
	static int 			file_get(const char *key,const char *value,size_t valuelen,void *user_data);
	static int 			file_store(const char *path, long long file_size, void *user_data);
    // æŒ‡å®šä¸‹è½½æ–‡ä»¶å­˜å‚¨ä½ç½®ï¼Œä»¥åŠåˆ«å
    static bool         SaveUploadFile(struct mg_connection *conn, const char *i_pPath, const char *i_pFileNmae, struct T_FromInfo *o_tFrominfo);

    static int          UploadCertificateHandle(struct mg_connection *conn, void *cbdata);
    static int          SetCertificateHandle(struct mg_connection *conn, void *cbdata);


    // æ–‡ä»¶ä¼ è¾“æ–¹é¢
    static int         UploadChannelMapHandle(struct mg_connection *conn, void *cbdata);
    static int         DownChannelMapHandle(struct mg_connection *conn, void *cbdata);
    static int         ShowSleepImageHandle(struct mg_connection *conn, void *cbdata);
    static int         UpdateSleepImageHandle(struct mg_connection *conn, void *cbdata);
    static int         UpdateEdidHandle(struct mg_connection *conn, void *cbdata);
    static int         DownloadLogFileHandle(struct mg_connection *conn, void *cbdata);
    static int         UpdateOverlayImageHandle(struct mg_connection *conn, void *cbdata);
    //static int         TransmitPresetsFileHandle(struct mg_connection *conn, void *cbdata);
    static int         UploadSecurHandle(struct mg_connection *conn, void *cbdata);
    static int         DownSecurHandle(struct mg_connection *conn, void *cbdata);
    static int         UploadVideoWallHandle(struct mg_connection *conn, void *cbdata);
    static int         DownVideoWallHandle(struct mg_connection *conn, void *cbdata);
    static int         UploadUpgradeHandle(struct mg_connection *conn, void *cbdata);

    // jsonæ•°æ®ä¼ è¾“
    static int         JsonDataHandle(struct mg_connection *conn, void *cbdata);

    // P3K
    static void        P3kStatusInit();
    static void 	   P3kWebsocketHandle(struct mg_connection *conn,char *data,size_t len);
    static void *      P3kCommunicationThread(void *arg);
    static void        CloseP3kSocket(void);

    // åŸ5000
    static int         ActionReqHandler(struct mg_connection *conn, void *cbdata);
    static int         StreamReqHandler(struct mg_connection *conn, void *cbdata);
    static int         UploadLogoReqHandler(struct mg_connection *conn, void *cbdata);
    static int         UploadBgReqHandler(struct mg_connection *conn, void *cbdata);
    static int         CapturebmpReqHandler(struct mg_connection *conn, void *cbdata);
    static int         CapturejpgReqHandler(struct mg_connection *conn, void *cbdata);
    static void *      MjpegStreamThread(void *param);

private:
    static conn_state* get_state(struct mg_connection * conn);
    static long long get_time_ms(void);
    static int SendBmpPic(struct mg_connection *conn, const char *path);
    static int send_bmp_file(struct mg_connection *conn,const char * picpath);
    static void update_jpg_preview_file(void);

	static CMutex s_p3kmutex;
    static CCond s_p3kcond;
	static int s_p3kSocket;
	static struct mg_context * ctx;
    static P3kStatus s_p3kStatus;

    static CMutex s_AliveStreamMutex;
    static CMutex s_MjpegUsrCntMutex;
    static CMutex s_MjpegMutex;
    static CMutex s_BmpMutex;
    static int s_mjpegUsrCnt;
    static int s_KeepAliveWorker;
    static int s_mjpegSeq;
    static int s_mjpegEnable;
    static long long s_LastUpdataTime;
    static int s_mjpegIntevalMs;
};





#endif

