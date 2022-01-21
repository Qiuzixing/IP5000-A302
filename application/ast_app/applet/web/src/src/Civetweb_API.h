/****************************************************************************
* ��Ȩ��Ϣ����C��2020�������аγ��Ƽ����޹�˾��Ȩ����
* ϵͳ���ƣ�
* �ļ����ƣ�webserver.h
* �ļ�˵�������ļ�������civetweb.h�ṩ�Ľӿڣ���װ��һЩ���õĹ��ܣ�
			���ĳЩ������Ҫ�Զ��壬��������ʹ��civetweb.h�еĽӿ�
* ��    �ߣ�zp
* �汾��Ϣ��1.0
* ������ڣ�2020-9-4
* �޸ļ�¼��
* ��    ��                ��    ��                �޸���                 �޸�ժҪ
****************************************************************************/

#ifndef _CIVETWEB_API_H_
#define _CIVETWEB_API_H_

#include "civetweb.h"
#include <vector>
#include <list>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>


using namespace std;


#define WEBSOCKET_RECV_BUF 1025


#define ASSERT(x)                                                              \
	{                                                                          \
		if (!(x)) {                                                            \
			fprintf(stderr,                                                    \
			        "Assertion failed in line %u\n",                           \
			        (unsigned)__LINE__);                                       \
		}                                                                      \
	}

// define AUTH Mode  ��֤ģʽ
enum websvr_auth_e
{
	WEBSVR_AUTH_DISABLE	= 0,		//No verification
	WEBSVR_AUTH_LOCAL 	= 1,		//Local verification
    WEBSVR_AUTH_LDAP 	= 2,		//ldap verification
};

// ������֤���
enum websvr_auth__res
{
	WEBSVR_AUTH_UNKNOW 	= 0,
	WEBSVR_AUTH_SUCCESS	= 1,
	WEBSVR_AUTH_FAIL 	= 2,
};


typedef struct t_ws_client
{
	struct mg_connection *	conn;
	websvr_auth__res 		local_auth;
	websvr_auth__res 		ldap_auth;
} ws_clients;







typedef struct user_conn_data
{
	bool Rspflag;			//��ʶ�� conn��û�б��ظ���true���ظ��ˣ�false��δ�ظ���
}UserConnData;


/***********************************************************************
 * �������� : ServerStart
 * ��    �� :
 * ������� :
 * �������� : start civetweb server
 * ��    �� : callbacks This structure needs to be passed to mg_start(), to let civetweb know
   						which callbacks to invoke. For a detailed description, see
   						https://github.com/civetweb/civetweb/blob/master/docs/UserManual.md
 * ��	�� : user_data user data
 * ��	�� : options   Configure civetweb related attributes.
 						Commonly used attributes are: document_directory, listening port, certificate location
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/


#ifdef USE_SSL_DH
#include "openssl/dh.h"
#include "openssl/ec.h"
#include "openssl/ecdsa.h"
#include "openssl/evp.h"
#include "openssl/ssl.h"
#endif
/*
	1.allconf represents the configuration required for both http and https,
			and httpsconf represents the configuration specifically required for https
	2.THE default value means that if the user does not assign a value to these parameters, the default value will be enabled.
	3.Pay special attention to the default value of NULL means that the user must give a value, otherwise it will return NULL and the startup will fail
*/
typedef struct web_options
{
	string web_allconf_documentroot;					/*default: NULL,User must assign*/
	string web_allconf_http_port;						/*default: "8080"*/
	string web_allconf_https_port;
	string web_httpsconf_cerm;							/*default: NULL,Certificate The user must give parameters when enabling https*/
	string web_allconf_enable_auth_domain_check;		/*default: "yes"*/
	string web_allconf_authentication_domain;			/*default: "mydomain.com"*/
	string web_allconf_thread_num;						/*default: "5"��Number of threads opened*/
	string web_allconf_enable_keepalive;				/*default: "no"��keep alive*/
	string web_allconf_keepalive_timeout;				/*default: "500"��keep alive timeout,ms*/
	string web_wsconf_enable_pingpong;					/*default: no*/
	string web_wsconf_pingpong_timeout_ms;				/*default: 3600000*/
}web_options;
enum Server_Start_Mode
{
	Server_Start_Mode_Http = 0,
	Server_Start_Mode_Https = 1,
	Server_Start_Mode_all = 2,
	Server_Start_Mode_auto = 3,
};
struct mg_context *ServerStart(web_options*options,Server_Start_Mode Server_mode,void *user_data);


/***********************************************************************
 * �������� : ServerStop
 * ��    �� :
 * ������� :
 * �������� : Shut down civetweb server
 * ��    �� : ctx �����������Ĺ���
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
void ServerStop(struct mg_context *ctx);


/***********************************************************************
 * �������� : List_All_ServerPort
 * ��    �� :
 * ������� :
 * �������� : ��ӡ��ǰ�󶨼��������ж˿ڣ�һ������ ServerStart ֮�󣬴�ӡ��ѯ�����Ķ˿ڶԲ���
 * ��    �� : ctx �����������Ĺ���
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
void List_All_ServerPort(struct mg_context *ctx);


/***********************************************************************
 * �������� : AddURIProcessHandler
 * ��    �� :
 * ������� :
 * �������� : ����URI�Ĵ�������
 * ��    �� : ctx �����������Ĺ���
 * ��    �� : uri_list ��Դ��ʶ���б���civetweb ��web��ͨ�Ż�������ͨ����Դ����ʶ�ģ�һ��uri��Ӧһ������������
 					�˴���list ���� uri       <-��Ӧ-> handler �ı���
			 mg_request_handler �յ���uriʱ�Ļص���������
 * ��    �� : list_size list�ĳ�Ա��������ʹ�� ARRAY_SIZE(x) ���
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

struct uri_list
{
	const char * uri;
	mg_request_handler handler;
	void * user_data;
};
void AddURIProcessHandler(struct mg_context *ctx,struct uri_list * list,int list_size);

#ifdef USE_WEBSOCKET
/***********************************************************************
 * �������� : SetWebsocketHandler
 * ��    �� :
 * ������� :
 * �������� : websocket ��غ�����˵�����£�
 			1.mg_websocket_handler ��һ���������ͣ����ڴ���websocket�����ݵĻص��������˻ص�����������͸��API��Ϣ
 			2.SetWebsocketHandler ��websocket���ú������������û��Զ�������ݴ����ص�����������ȥ��
 			3.SetWebsocketConnVal ��������Щwebsocket�ͻ�������Ч�ġ���Ϊ�������е�web�ͻ��˶�����Ч���ӣ�
 									����û��ͨ����֤�ģ��Ϳ���SetWebsocketConnVal(conn,1,0);
 									�������û�о���������֤��web�Ͳ�������ѡ��㷢����ʱ�յ����ݡ�
 			4.Send_Data_To_CurWebsocket 	�����Ե�ǰ���ӵ�websocket�������� ��
 			5.Send_Data_To_ALLWebsocket		������websocket�㷢����

 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/

typedef void (*mg_websocket_handler)(struct mg_connection *conn,char *data,size_t len);

/*
	����websocket �ͻ��� �Ƿ�ͨ����֤��
	1 ��ʾ�ɹ�
	0 ��ʾʧ��
*/
int SetWebsocketLocalAuthSucess(struct mg_connection *conn);
int SetWebsocketLDAPAuthSucess(struct mg_connection *conn);



void SetWebsocketHandler(struct mg_context *ctx,
			                         mg_websocket_handler handler,
			                         void *cbdata);
/*
	����Ϣ�������� conn ��������֪ͨ��ǰ��һ��web������ͨ����֤�ˡ�
*/
void Send_Data_To_CurWebsocket(struct mg_connection *conn, const char *data, size_t len);

/*
	�㷢��Ϣʱ��ֻ�� ͨ����֤����ldap_auth != 0�� ��web�����յ��������㷢����Ϣ
*/
void Send_Data_To_ALLWebsocket(const char *data,size_t len);


#endif 	/*USE_WEBSOCKET*/

/***********************************************************************
 * �������� : Civetweb_check_IPV6_feature
 * ��    �� :
 * ������� :
 * �������� : ��⵱ǰ��������û�п��� IPV6 ���� ��ע�⣺�ú���ֻ���ڶ����� USE_IPV6 ������Ч��⣬����˺�������ֵ��Ч��
 * ��    �� :
 * �� �� ֵ : 0 ��ʾδ���ã���0 ��ʾ������
 * �޸�����           �޸���	      �޸�����
 example��
 	#ifdef USE_IPV6
		if (!Civetweb_check_IPV6_feature()) {
			printf(
			        "error: Embedded example built with IPv6 support, "
			        "but civetweb library build without.");
		}
	#endif
***********************************************************************/
int Civetweb_check_IPV6_feature();

/***********************************************************************
 * �������� : Civetweb_check_WEBSOCKET_feature
 * ��    �� :
 * ������� :
 * �������� : ��⵱ǰ��������û�п��� WEBSOCKET ���� ��ע�⣺�ú���ֻ���ڶ����� USE_WEBSOCKET ������Ч��⣬����˺�������ֵ��Ч��
 * ��    �� :
 * �� �� ֵ : 0 ��ʾδ���ã���0 ��ʾ������
 * �޸�����           �޸���	      �޸�����
 example��
 	#ifdef USE_WEBSOCKET
		if (!Civetweb_check_WEBSOCKET_feature()) {
			printf(
			        "error: Embedded example built with websocket support, "
			        "but civetweb library build without.");
		}
	#endif
***********************************************************************/
int Civetweb_check_WEBSOCKET_feature();



/***********************************************************************
 * �������� : Civetweb_check_SSL_feature
 * ��    �� :
 * ������� :
 * �������� : ��⵱ǰ��������û�йر� ssl ����sslĬ���ǿ����� ��ע�⣺�ú���ֻ����û�ж��� NO_SSL ������Ч��⣬����˺�������ֵ��Ч��
 * ��    �� :
 * �� �� ֵ : 0 ��ʾδ����ssl���񣬷�0 ��ʾ�ѿ���ssl����
 * �޸�����           �޸���	      �޸�����
 example��
 	#ifndef NO_SSL
		if (!Civetweb_check_SSL_feature()) {
			printf(
			        "error: Embedded example built with SSL support, "
			        "but civetweb library build without.");
		}
	#endif
***********************************************************************/
int Civetweb_check_SSL_feature();

/***********************************************************************
 * �������� : get_file_size
 * ��    �� :
 * ������� :
 * �������� : ��ȡ�ļ���С
 * ��    �� : path  ��·�����ļ���
 * �� �� ֵ : 	�ļ���С
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
unsigned long get_file_size(const char *path);

/***********************************************************************
 * �������� : get_time_ms
 * ��    �� :
 * ������� :
 * �������� : ��ȡʱ�䣬��λ����
 * ��    �� :
 * �� �� ֵ : 	��ǰʱ��
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
long long get_time_ms();

/***********************************************************************
 * �������� : ACCESS_FILE_EXIT_R_OK
 * ��    �� :
 * ������� :
 * �������� :	�ж��ļ��Ƿ���ڣ��ҿɶ�
 * ��    �� :
 * �� �� ֵ : 	0 �����ҿɶ�
 				-1 ������
 				-2 ���ɶ�
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
int ACCESS_FILE_EXIT_R_OK(const char * file);

/***********************************************************************
 * �������� : ACCESS_FILE_EXIT_W_OK
 * ��    �� :
 * ������� :
 * �������� :	�ж��ļ��Ƿ���ڣ��ҿ�д
 * ��    �� :
 * �� �� ֵ : 	0 �����ҿ�д
 				-1 ������
 				-2 ����д
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
int ACCESS_FILE_EXIT_W_OK(const char * file);

/***********************************************************************
 * �������� : StringtoFile
 * ��    �� :
 * ������� :
 * �������� :	��i_pStrBuff ������д�� i_pOutFileath�ļ���
 * ��    �� :
 * �� �� ֵ : 	д����ֽ���
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
int StringtoFile(string i_pStrBuff, const char *i_pOutFileath);

/***********************************************************************
 * �������� : File2String
 * ��    �� :
 * ������� :
 * �������� :	��i_pinFilePath �����ݶ��� o_strDes��
 * ��    �� :
 * �� �� ֵ : 	�������ֽ���
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
int File2String(const char *i_pinFilePath, string &o_strDes);


void Parse_String_Json(const char* i_pSrc, const char* i_pFlag, string &o_string);


#if 0
/* Stringify binary data. Output buffer must be twice as big as input,
 * because each byte takes 2 bytes in string representation */
static void bin2str(char *to, const unsigned char *p, size_t len);
#endif
/***********************************************************************
 * �������� : Set_Formdata_Handler
 * ��    �� :
 * ������� :
 * �������� : ��web�ϴ����������ļ���form-data��ʽ��ʱ���ô˺����������������ݵ��ļ��У�
 * ��    �� : conn  ��ǰ����
 * ��    �� : fdh  	�ļ����ݴ��������ṹ�壬��ϸ������ο�civetweb.h �й���struct mg_form_data_handler��˵��
 * �� �� ֵ : 	�ļ���С
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
int Set_Formdata_Handler(struct mg_connection *conn,struct mg_form_data_handler *fdh);



/***********************************************************************
 * �������� : BC_mg_get_user_connection_data
 * ��    �� :
 * ������� :
 * �������� : conn ����һ�� void* userdata�����Թ��û���һЩ˽����Ϣ�� conn �У��ú������ǻ�ȡconn�е�userdata��
 				�� GB_set_user_connection_data �ӿڳ���ʹ��
 * ��    �� :   conn  ��ǰ����
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
void * BC_mg_get_user_connection_data(struct mg_connection *conn);

/***********************************************************************
 * �������� : BC_mg_set_user_connection_data
 * ��    �� :
 * ������� :
 * �������� : conn ����һ�� void* userdata�����Թ��û���һЩ˽����Ϣ�� conn �У��ú������ǻ�ȡconn�е�userdata��
 				�� GB_get_user_connection_data �ӿڳ���ʹ��
 * ��    �� :   conn  ��ǰ����
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
void  BC_mg_set_user_connection_data(struct mg_connection *conn,void * data);

/***********************************************************************
 * �������� : BC_mg_get_request_info
 * ��    �� :
 * ������� :
 * �������� : ��ȡconn�е� mg_request_info�������Ա�а����� request_uri http_headers��������Ϣ
 * ��    �� :   conn  ��ǰ����
 * �� �� ֵ : ��ע���������ֵ�� const ��Ҳ����˵����������õ�ֵ�ı䡣
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
const struct mg_request_info *BC_mg_get_request_info(const struct mg_connection * conn);

/***********************************************************************
 * �������� : BC_CheckRequestMethod
 * ��    �� :
 * ������� :
 * �������� : check method
 * ��    �� :   conn  ��ǰ����
 * �� �� ֵ : true: �յ���������p_Method��false���յ���������p_Method��
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
bool BC_CheckRequestMethod(const struct mg_connection * conn,const char * p_Method);



/***********************************************************************
 * �������� : Print_Request_Info
 * ��    �� :
 * ������� :
 * �������� : ��ӡ������Ϣ����ӡ����Ϣ�ܶ࣬����ֻ���ڵ��ԡ�
 * ��    �� :   conn  ��ǰ����
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
void Print_Request_Info(const struct mg_connection *conn);




/***********************************************************************
 * �������� : mg_close_cur_connection
 * ��    �� :
 * ������� :
 * �������� : �رյ�ǰ�����ӡ�
 * ��    �� :   conn  ��ǰ����
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
void mg_close_cur_connection(struct mg_connection *conn);


/***********************************************************************
 * �������� : BC_mg_printf
 * ��    �� :
 * ������� :
 * �������� : �Զ���ظ����ݸ�ǰ�ˡ�BC_mg_printf �� mg_printf ��һ��������
 				�ұȽ�ϲ��ʹ�� mg_printf�����Դ�����mg_printf ���ֵĴ����ܶࡣ
 * ��    �� :   conn  ��ǰ����
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
#define BC_mg_printf(conn,format, args...) do\
{ \
	mg_printf(conn,format,##args); \
}while(0)

/*
	Send data to the client.
   Return:
    0   when the connection has been closed
    -1  on error
    >0  number of bytes written on success
*/
int BC_mg_write(struct mg_connection *conn, const void *buf, size_t len);



/***********************************************************************
 * �������� : mg_send_status
 * ��    �� :
 * ������� :
 * �������� : �����ĸ�webһ����Ӧ������ OK��200�� ������ error��400�� ����������Ҫ��web�������ݡ�
 			  �ú�����Ƴ�����Ϊ����Ӧ mongoose��mongoose������ӿڣ������������һ����
 * ��    �� : status  һ��״ֵ̬ �� 200 400 ֮���
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
void mg_send_status(struct mg_connection *conn,int status);


/***********************************************************************
 * �������� : send_http_ok_rsp
 * ��    �� :
 * ������� :
 * �������� : �ظ�webһ�� ������OK ����Ӧ��
 * ��    �� :   conn  ��ǰ����
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
void send_http_ok_rsp(struct mg_connection *conn);



/***********************************************************************
 * �������� : send_http_error_rsp
 * ��    �� :
 * ������� :
 * �������� : �ظ�webһ�� ������error ����Ӧ��
 * ��    �� :   conn  ��ǰ����
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
void send_http_error_rsp(struct mg_connection *conn);

/***********************************************************************
 * �������� : send_http_usererror_rsp
 * ��    �� :
 * ������� :
 * �������� : �ظ�webһ�� �û��Զ��� error ����Ӧ��
 * ��    �� :   conn  ��ǰ����
 * ��    �� :   status  ״̬��
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
void send_http_usererror_rsp(struct mg_connection *conn,int status);

void send_http_usererror_body_rsp(struct mg_connection *conn,int status, const char* body);

/***********************************************************************
 * �������� : send_http_pwd_fail_rsp
 * ��    �� :
 * ������� :
 * �������� : �ظ�webһ�� ������У�鲻ͨ�� ����Ӧ��
 * ��    �� :   conn  ��ǰ����
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
void send_http_pwd_fail_rsp(struct mg_connection *conn);



/***********************************************************************
 * �������� : send_html_file
 * ��    �� :
 * ������� :
 * �������� : �ϴ�HTMLҳ���web��ʾ
 * ��    �� : htmlpathpath  ������·����HTML�ļ���
 * ������
 * �� �� ֵ : 	�ļ���С
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
int send_html_file(struct mg_connection *conn,const char * htmlpath);
/***********************************************************************
 * �������� : send_jpg_file
 * ��    �� :
 * ������� :
 * �������� : �ϴ�һ��JPGͼƬ��web��ʾ
 * ��    �� : picpath  ������·�����ļ���
 * �� �� ֵ : 	�ļ���С
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
int send_jpg_file(struct mg_connection *conn,const char * picpath);
/***********************************************************************
 * �������� : send_jpg_stream_file
 * ��    �� :
 * ������� :
 * �������� : ���� ���������Ҹ����ӻ�һֱ������������������Ͽ�����������
 * ��    �� : picpath  ��·����jpg�ļ���
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
int send_jpg_stream_file(struct mg_connection *conn,const char * picpath,int time_ms);

/***********************************************************************
 * �������� : send_chunk_file_Body
 * ��    �� :
 * ������� :
 * �������� : ��chunk�ĸ�ʽ�����������͵��ļ������ر�ע�⣬�ýӿ�ֻ�Ƿ���body���֣�header���ֱ�����ʹ�����Զ���
 				������ᶨ�� header���֣�����ֱ��ʹ�������send_chunk_file�ӿڡ�
 * ��    �� : filepath  ������·�����ļ���
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
int send_chunk_file_Body(struct mg_connection *conn,const char * filepath);


/***********************************************************************
 * �������� : send_http_file
 * ��    �� :
 * ������� :
 * �������� : ��chunk�ĸ�ʽ�����������͵��ļ������ر�ע�⣬�ýӿ��Ѿ���װ�� OK ����Ӧͷ������Ҫʹ�øýӿڣ���ֹ
 				ʹ�������Զ���������Ӧͷ���ͣ�ʹ����Ҫ�Զ�����Ӧͷ��ʹ��send_chunk_file_Body�ӿ� ��
 * ��    �� : pathname  ������·�����ļ�����ʹ����Ҫ���͵��ļ���
 			filename 	�ļ���ʶ�����㽫�����ļ����͸�ǰ��ʱ��ǰ�˲���֪������ļ���ʲô���֣������ͨ���Ĳ�����֪ǰ�ˣ�
 * ʾ�� ��
 			send_http_file(conn,"/1.c","3.c");
 			//�����ص��ļ�1.c ���͸�ǰ�ˣ�������ǰ������ļ���־��3.c(ע������ֻ�Ǳ�־Ϊ3.c������ǰ�����Ϊʲô��������������)
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
void send_chunk_file(struct mg_connection *conn,const char * pathname,const char * filename);

/***********************************************************************
 * �������� : send_json_data
 * ��    �� :
 * ������� :
 * �������� : �ϴ�json ����
 * ��    �� : jsondata  json��ʽ���ַ���(ʹ���߸�����εļ�⣬�ýӿ�Ĭ��jsondata����json����)
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
int send_json_data(struct mg_connection *conn,char * jsondata);




/***********************************************************************
 * �������� : mg_get_header_value
 * ��    �� :
 * ������� :
 * �������� : ��ȡ header ��value ��header������ name:value �ļ�ֵ����ʽ���ڵ�
 * ��    �� : name  header name
 * �� �� ֵ : 	name���ڣ��򷵻� value�������ڣ��򷵻�NULL
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
const char *mg_get_header_value(const struct mg_connection *conn,const char *name);



/***********************************************************************
 * �������� : mg_get_var_from_querystring
 * ��    �� :
 * ������� :
 * �������� : �� query_string �л�ȡ����name��Ӧ�� Valer��ֵ��������dst��
 				�ú�������� mg_get_var ����
 * ��    �� : name  ����
 * ��    �� : dst   �洢value��buf
 * ��    �� : dst_len  dst��size
 * �� �� ֵ : 	dstΪnull ���� dst_len Ϊ0	����װ����				:����-2
 				nameΪnull								:����-1
 				����										:���� value����

 * �޸�����           �޸���	      �޸�����
***********************************************************************/
int mg_get_var_from_querystring(const struct mg_connection *conn, const char *name,char *dst, size_t dst_len);




/***********************************************************************
 * �������� : mg_printf_data
 * ��    �� :
 * ������� :
 * �������� : �Զ�����chunk header���ҽ�fmt �ṩ�Ĳ�����chunk��ʽ����
 				�ú���������Ŀ��ֻ�ǵ�����ģ�� mongoose �е�ͬ������
 * ����		��send_header �Ƿ��� header��Transfer-Encoding: chunked
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
#define IOBUF_SIZE 8192
int mg_printf_data(struct mg_connection *conn,bool send_header, const char *fmt, ...);


/***********************************************************************
 * �������� : BC_mg_send_header
 * ��    �� :
 * ������� :
 * �������� : �Զ�������Ӧ�У�HTTP/1.1 200 OK\r\n
 				Ȼ������ʹ�ô����header ������name :value\r\n
 				�磺	HTTP/1.1 200 OK\r\nConnection :close\r\n
 				�ú���������Ŀ��ֻ�ǵ�����ģ�� mongoose �е�mg_send_header����
 * �� �� ֵ :
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
void BC_mg_send_header(struct mg_connection *conn, const char *name, const char *value);

/***********************************************************************
 * �������� : BC_GET_Request_Body
 * ��    �� :
 * ������� :
 * �������� : ������� �洢��buf��

 * ����		��buf �洢��
 * ����		��len   buf�Ĵ�С
 * �� �� ֵ :		0 	��ʾû�����ݿɶ���
 				<0 	��ʾ����
 				>0	��ʾbuf����˶����ֽ�
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
int BC_GET_Request_Body(struct mg_connection *conn, void *buf, size_t len);

/***********************************************************************
 * �������� : STore_body_to_file
 * ��    �� :
 * ������� :
 * �������� : ������� body��һ������ post���󣩴洢���ļ���i_store_file����

 * ����		��store_file body��浽����ļ���
 * �� �� ֵ :		body�ĳ���	��ʧ�ܷ��� <= 0
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
long long STore_body_to_file(struct mg_connection *conn,const char * i_store_file);







/***********************************************************************
 * �������� : My_System
 * ��    �� :
 * ������� :
 * �������� : ִ��shellָ�
 * ��    �� :   cmdstring Ҫִ�е�shell
 * �� �� ֵ : ʧ�ܷ���-1���ɹ�����ִ��״̬��
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
int My_System(const char * cmdstring);



/***********************************************************************
 * �������� : Get_Line_From_File
 * ��    �� :
 * ������� :
 * �������� : ���ļ��л�ȡ �� i_cnt �е����ݡ�
 * ��    �� :
 	i_filename Ŀ���ļ�
	o_storebuf ��ȡ�����ݴ浽���buf
	i_cnt Ҫ��ȡ�ڼ��� ��> 0��
 * �� �� ֵ :
	 0 �ɹ�������һ�����ݣ��Ҵ浽�� o_storebuf
	-1 �ļ���ʧ��
	-2 ���� i_cnt <= 0
	-3 �ļ�û����һ��
	-4 δ֪�Ĵ���
 * �޸�����           �޸���	      �޸�����
***********************************************************************/
int Get_Line_From_File(const char * i_filename,string& o_storebuf,int i_cnt);


#endif
