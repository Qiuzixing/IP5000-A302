/****************************************************************************
* ��Ȩ��Ϣ����C��2020�������аγ��Ƽ����޹�˾��Ȩ����
* ϵͳ���ƣ�
* �ļ����ƣ�webapp.h
* �ļ�˵�������ļ�������civetweb.h�ṩ�Ľӿڣ���װ��һЩ��ҵ���߼���
* ��    �ߣ�zp
* �汾��Ϣ��1.0
* ������ڣ�2020-9-4
* �޸ļ�¼��
* ��    ��                ��    ��                �޸���                 �޸�ժҪ
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

// 暂时(套接字相关)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MG_READ_BUFSIZE 	2*1024
#define KEY_VALUE_SIZE      256
#define MAX_KEY_VALUE_SIZE  1024

typedef struct P3kStatus
{
    struct mg_connection *p3k_conn;
    Bool16 p3k_writeok;
    int p3k_start;
}P3kStatus;

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
    // 指定下载文件存储位置，以及别名
    static bool         SaveUploadFile(struct mg_connection *conn, const char *i_pPath, const char *i_pFileNmae, struct T_FromInfo *o_tFrominfo);

    static int          UploadCertificateHandle(struct mg_connection *conn, void *cbdata);
    static int          SetCertificateHandle(struct mg_connection *conn, void *cbdata);


    // 文件传输方面
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

    // json数据传输
    static int         SetSwitchDelayHandle(struct mg_connection *conn, void *cbdata);
    static int         GetSwitchDelayHandle(struct mg_connection *conn, void *cbdata);
    static int         SetAVSignalHandle(struct mg_connection *conn, void *cbdata);
    static int         GetAVSignalHandle(struct mg_connection *conn, void *cbdata);
    static int         SetDisplaySleepHandle(struct mg_connection *conn, void *cbdata);
    static int         GetDisplaySleepHandle(struct mg_connection *conn, void *cbdata);
    static int         SetEdidHandle(struct mg_connection *conn, void *cbdata);
    static int         GetEdidHandle(struct mg_connection *conn, void *cbdata);
    static int         SetGatewayHandle(struct mg_connection *conn, void *cbdata);
    static int         GetGatewayHandle(struct mg_connection *conn, void *cbdata);
    static int         SetVersionHandle(struct mg_connection *conn, void *cbdata);
    static int         GetVersionHandle(struct mg_connection *conn, void *cbdata);
    static int         SetOsdHandle(struct mg_connection *conn, void *cbdata);
    static int         GetOsdHandle(struct mg_connection *conn, void *cbdata);
    static int         SetOverlayHandle(struct mg_connection *conn, void *cbdata);
    static int         GetOverlayHandle(struct mg_connection *conn, void *cbdata);
    static int         SetSecureHandle(struct mg_connection *conn, void *cbdata);
    static int         GetSecureHandle(struct mg_connection *conn, void *cbdata);
    static int         SetUsbHandle(struct mg_connection *conn, void *cbdata);
    static int         GetUsbHandle(struct mg_connection *conn, void *cbdata);


    // P3K
    static void        P3kStatusInit();
    static void 	   P3kWebsocketHandle(struct mg_connection *conn,char *data,size_t len);
    static void *      P3kCommunicationThread(void *arg);
    static void        CloseP3kSocket(void);

    // 测p3k用线程
    //static int        P3kTestHand();
    //static int        SetP3k(struct mg_connection *conn, void *cbdata);
private:

	static CMutex s_p3kmutex;
    static CCond s_p3kcond;
	static int s_p3kSocket;
	static struct mg_context * ctx;
    static P3kStatus s_p3kStatus;
};





#endif
