/****************************************************************************
* 版权信息：（C）2020，深圳市拔超科技有限公司版权所有
* 系统名称：
* 文件名称：webserver.h
* 文件说明：该文件是利用civetweb.h提供的接口，封装了一些常用的功能，
			如果某些功能需要自定义，可以自行使用civetweb.h中的接口
* 作    者：zp
* 版本信息：1.0
* 设计日期：2020-9-4
* 修改记录：
* 日    期                版    本                修改人                 修改摘要
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

// define AUTH Mode  验证模式
enum websvr_auth_e
{
	WEBSVR_AUTH_DISABLE	= 0,		//No verification
	WEBSVR_AUTH_LOCAL 	= 1,		//Local verification
    WEBSVR_AUTH_LDAP 	= 2,		//ldap verification
};

// 定义验证结果
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
	bool Rspflag;			//标识该 conn有没有被回复（true：回复了，false：未回复）
}UserConnData;


/***********************************************************************
 * 函数名称 : ServerStart
 * 作    者 :
 * 设计日期 :
 * 功能描述 : start civetweb server
 * 参    数 : callbacks This structure needs to be passed to mg_start(), to let civetweb know
   						which callbacks to invoke. For a detailed description, see
   						https://github.com/civetweb/civetweb/blob/master/docs/UserManual.md
 * 参	数 : user_data user data
 * 参	数 : options   Configure civetweb related attributes.
 						Commonly used attributes are: document_directory, listening port, certificate location
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
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
	string web_allconf_thread_num;						/*default: "5"，Number of threads opened*/
	string web_allconf_enable_keepalive;				/*default: "no"，keep alive*/
	string web_allconf_keepalive_timeout;				/*default: "500"，keep alive timeout,ms*/
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
 * 函数名称 : ServerStop
 * 作    者 :
 * 设计日期 :
 * 功能描述 : Shut down civetweb server
 * 参    数 : ctx 服务器上下文勾柄
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
***********************************************************************/
void ServerStop(struct mg_context *ctx);


/***********************************************************************
 * 函数名称 : List_All_ServerPort
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 打印当前绑定监听的所有端口，一般用于 ServerStart 之后，打印查询启动的端口对不对
 * 参    数 : ctx 服务器上下文勾柄
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
***********************************************************************/
void List_All_ServerPort(struct mg_context *ctx);


/***********************************************************************
 * 函数名称 : AddURIProcessHandler
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 添加URI的处理函数
 * 参    数 : ctx 服务器上下文勾柄
 * 参    数 : uri_list 资源标识符列表。civetweb 与web的通信基本都是通过资源符标识的，一个uri对应一个处理函数。
 					此处的list 就是 uri       <-对应-> handler 的表格。
			 mg_request_handler 收到该uri时的回调处理函数
 * 参    数 : list_size list的成员数，可以使用 ARRAY_SIZE(x) 求得
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
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
 * 函数名称 : SetWebsocketHandler
 * 作    者 :
 * 设计日期 :
 * 功能描述 : websocket 相关函数，说明如下：
 			1.mg_websocket_handler 是一个函数类型，用于处理websocket的数据的回调函数。此回调函数常用来透传API消息
 			2.SetWebsocketHandler 是websocket设置函数，将上面用户自定义的数据处理回调函数设置下去。
 			3.SetWebsocketConnVal 是设置哪些websocket客户端是有效的。因为不是所有的web客户端都是有效连接，
 									比如没有通过验证的，就可以SetWebsocketConnVal(conn,1,0);
 									这样这个没有经过身份验证的web就不会在你选择广发数据时收到数据。
 			4.Send_Data_To_CurWebsocket 	单独对当前连接的websocket发送数据 。
 			5.Send_Data_To_ALLWebsocket		对所有websocket广发数据

 * 返 回 值 :
 * 修改日期           修改人	      修改内容
***********************************************************************/

typedef void (*mg_websocket_handler)(struct mg_connection *conn,char *data,size_t len);

/*
	设置websocket 客户端 是否通过验证。
	1 表示成功
	0 表示失败
*/
int SetWebsocketLocalAuthSucess(struct mg_connection *conn);
int SetWebsocketLDAPAuthSucess(struct mg_connection *conn);



void SetWebsocketHandler(struct mg_context *ctx,
			                         mg_websocket_handler handler,
			                         void *cbdata);
/*
	回消息给单个的 conn 。适用于通知当前的一个web，“你通过验证了”
*/
void Send_Data_To_CurWebsocket(struct mg_connection *conn, const char *data, size_t len);

/*
	广发消息时，只有 通过验证（即ldap_auth != 0） 的web才能收到服务器广发的消息
*/
void Send_Data_To_ALLWebsocket(const char *data,size_t len);


#endif 	/*USE_WEBSOCKET*/

/***********************************************************************
 * 函数名称 : Civetweb_check_IPV6_feature
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 检测当前服务器有没有开启 IPV6 服务 （注意：该函数只有在定义了 USE_IPV6 才能有效检测，否则此函数返回值无效）
 * 参    数 :
 * 返 回 值 : 0 表示未启用，非0 表示已启用
 * 修改日期           修改人	      修改内容
 example：
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
 * 函数名称 : Civetweb_check_WEBSOCKET_feature
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 检测当前服务器有没有开启 WEBSOCKET 服务 （注意：该函数只有在定义了 USE_WEBSOCKET 才能有效检测，否则此函数返回值无效）
 * 参    数 :
 * 返 回 值 : 0 表示未启用，非0 表示已启用
 * 修改日期           修改人	      修改内容
 example：
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
 * 函数名称 : Civetweb_check_SSL_feature
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 检测当前服务器有没有关闭 ssl 服务，ssl默认是开启的 （注意：该函数只有在没有定义 NO_SSL 才能有效检测，否则此函数返回值无效）
 * 参    数 :
 * 返 回 值 : 0 表示未开启ssl服务，非0 表示已开启ssl服务
 * 修改日期           修改人	      修改内容
 example：
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
 * 函数名称 : get_file_size
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 获取文件大小
 * 参    数 : path  带路径的文件名
 * 返 回 值 : 	文件大小
 * 修改日期           修改人	      修改内容
***********************************************************************/
unsigned long get_file_size(const char *path);

/***********************************************************************
 * 函数名称 : get_time_ms
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 获取时间，单位毫秒
 * 参    数 :
 * 返 回 值 : 	当前时间
 * 修改日期           修改人	      修改内容
***********************************************************************/
long long get_time_ms();

/***********************************************************************
 * 函数名称 : ACCESS_FILE_EXIT_R_OK
 * 作    者 :
 * 设计日期 :
 * 功能描述 :	判断文件是否存在，且可读
 * 参    数 :
 * 返 回 值 : 	0 存在且可读
 				-1 不存在
 				-2 不可读
 * 修改日期           修改人	      修改内容
***********************************************************************/
int ACCESS_FILE_EXIT_R_OK(const char * file);

/***********************************************************************
 * 函数名称 : ACCESS_FILE_EXIT_W_OK
 * 作    者 :
 * 设计日期 :
 * 功能描述 :	判断文件是否存在，且可写
 * 参    数 :
 * 返 回 值 : 	0 存在且可写
 				-1 不存在
 				-2 不可写
 * 修改日期           修改人	      修改内容
***********************************************************************/
int ACCESS_FILE_EXIT_W_OK(const char * file);

/***********************************************************************
 * 函数名称 : StringtoFile
 * 作    者 :
 * 设计日期 :
 * 功能描述 :	把i_pStrBuff 的内容写到 i_pOutFileath文件中
 * 参    数 :
 * 返 回 值 : 	写入的字节数
 * 修改日期           修改人	      修改内容
***********************************************************************/
int StringtoFile(string i_pStrBuff, const char *i_pOutFileath);

/***********************************************************************
 * 函数名称 : File2String
 * 作    者 :
 * 设计日期 :
 * 功能描述 :	把i_pinFilePath 的内容读到 o_strDes中
 * 参    数 :
 * 返 回 值 : 	读出的字节数
 * 修改日期           修改人	      修改内容
***********************************************************************/
int File2String(const char *i_pinFilePath, string &o_strDes);


void Parse_String_Json(const char* i_pSrc, const char* i_pFlag, string &o_string);


#if 0
/* Stringify binary data. Output buffer must be twice as big as input,
 * because each byte takes 2 bytes in string representation */
static void bin2str(char *to, const unsigned char *p, size_t len);
#endif
/***********************************************************************
 * 函数名称 : Set_Formdata_Handler
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 当web上传的数据是文件（form-data格式）时，用此函数处理（保存数据到文件中）
 * 参    数 : conn  当前连接
 * 参    数 : fdh  	文件数据处理函数结构体，详细介绍请参考civetweb.h 中关于struct mg_form_data_handler的说明
 * 返 回 值 : 	文件大小
 * 修改日期           修改人	      修改内容
***********************************************************************/
int Set_Formdata_Handler(struct mg_connection *conn,struct mg_form_data_handler *fdh);



/***********************************************************************
 * 函数名称 : BC_mg_get_user_connection_data
 * 作    者 :
 * 设计日期 :
 * 功能描述 : conn 中有一个 void* userdata，可以供用户绑定一些私人信息到 conn 中，该函数就是获取conn中的userdata。
 				与 GB_set_user_connection_data 接口成套使用
 * 参    数 :   conn  当前连接
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
***********************************************************************/
void * BC_mg_get_user_connection_data(struct mg_connection *conn);

/***********************************************************************
 * 函数名称 : BC_mg_set_user_connection_data
 * 作    者 :
 * 设计日期 :
 * 功能描述 : conn 中有一个 void* userdata，可以供用户绑定一些私人信息到 conn 中，该函数就是获取conn中的userdata。
 				与 GB_get_user_connection_data 接口成套使用
 * 参    数 :   conn  当前连接
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
***********************************************************************/
void  BC_mg_set_user_connection_data(struct mg_connection *conn,void * data);

/***********************************************************************
 * 函数名称 : BC_mg_get_request_info
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 获取conn中的 mg_request_info。这个成员中包含了 request_uri http_headers等请求信息
 * 参    数 :   conn  当前连接
 * 返 回 值 : 请注意这个返回值是 const ，也就是说不允许将获得的值改变。
 * 修改日期           修改人	      修改内容
***********************************************************************/
const struct mg_request_info *BC_mg_get_request_info(const struct mg_connection * conn);

/***********************************************************************
 * 函数名称 : BC_CheckRequestMethod
 * 作    者 :
 * 设计日期 :
 * 功能描述 : check method
 * 参    数 :   conn  当前连接
 * 返 回 值 : true: 收到的请求是p_Method；false：收到的请求不是p_Method。
 * 修改日期           修改人	      修改内容
***********************************************************************/
bool BC_CheckRequestMethod(const struct mg_connection * conn,const char * p_Method);



/***********************************************************************
 * 函数名称 : Print_Request_Info
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 打印请求信息，打印的信息很多，建议只用于调试。
 * 参    数 :   conn  当前连接
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
***********************************************************************/
void Print_Request_Info(const struct mg_connection *conn);




/***********************************************************************
 * 函数名称 : mg_close_cur_connection
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 关闭当前的连接。
 * 参    数 :   conn  当前连接
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
***********************************************************************/
void mg_close_cur_connection(struct mg_connection *conn);


/***********************************************************************
 * 函数名称 : BC_mg_printf
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 自定义回复数据给前端。BC_mg_printf 与 mg_printf 是一个函数，
 				我比较喜欢使用 mg_printf，所以代码中mg_printf 出现的次数很多。
 * 参    数 :   conn  当前连接
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
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
 * 函数名称 : mg_send_status
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 单纯的给web一个响应，比如 OK（200） 或者是 error（400） 服务器不需要给web回送数据。
 			  该函数设计初衷是为了适应 mongoose（mongoose有这个接口，所以这里仿造一个）
 * 参    数 : status  一个状态值 如 200 400 之类的
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
***********************************************************************/
void mg_send_status(struct mg_connection *conn,int status);


/***********************************************************************
 * 函数名称 : send_http_ok_rsp
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 回给web一个 服务器OK 的响应。
 * 参    数 :   conn  当前连接
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
***********************************************************************/
void send_http_ok_rsp(struct mg_connection *conn);



/***********************************************************************
 * 函数名称 : send_http_error_rsp
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 回给web一个 服务器error 的响应。
 * 参    数 :   conn  当前连接
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
***********************************************************************/
void send_http_error_rsp(struct mg_connection *conn);

/***********************************************************************
 * 函数名称 : send_http_usererror_rsp
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 回给web一个 用户自定义 error 的响应。
 * 参    数 :   conn  当前连接
 * 参    数 :   status  状态码
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
***********************************************************************/
void send_http_usererror_rsp(struct mg_connection *conn,int status);



/***********************************************************************
 * 函数名称 : send_http_pwd_fail_rsp
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 回给web一个 服务器校验不通过 的响应。
 * 参    数 :   conn  当前连接
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
***********************************************************************/
void send_http_pwd_fail_rsp(struct mg_connection *conn);



/***********************************************************************
 * 函数名称 : send_html_file
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 上传HTML页面给web显示
 * 参    数 : htmlpathpath  带绝对路径的HTML文件名
 * 参数：
 * 返 回 值 : 	文件大小
 * 修改日期           修改人	      修改内容
***********************************************************************/
int send_html_file(struct mg_connection *conn,const char * htmlpath);
/***********************************************************************
 * 函数名称 : send_jpg_file
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 上传一张JPG图片给web显示
 * 参    数 : picpath  带绝对路径的文件名
 * 返 回 值 : 	文件大小
 * 修改日期           修改人	      修改内容
***********************************************************************/
int send_jpg_file(struct mg_connection *conn,const char * picpath);
/***********************************************************************
 * 函数名称 : send_jpg_stream_file
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 推送 码流，并且该连接会一直保持推送码流，不会断开，所以慎用
 * 参    数 : picpath  带路径的jpg文件名
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
***********************************************************************/
int send_jpg_stream_file(struct mg_connection *conn,const char * picpath,int time_ms);

/***********************************************************************
 * 函数名称 : send_chunk_file_Body
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 以chunk的格式推送任意类型的文件，请特别注意，该接口只是发送body部分，header部分必须由使用者自定义
 				如果不会定义 header部分，可以直接使用下面的send_chunk_file接口。
 * 参    数 : filepath  带绝对路径的文件名
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
***********************************************************************/
int send_chunk_file_Body(struct mg_connection *conn,const char * filepath);


/***********************************************************************
 * 函数名称 : send_http_file
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 以chunk的格式推送任意类型的文件，请特别注意，该接口已经封装了 OK 的响应头，所以要使用该接口，禁止
 				使用者再自定义其他响应头发送；使用者要自定义响应头请使用send_chunk_file_Body接口 。
 * 参    数 : pathname  带绝对路径的文件名（使用者要推送的文件）
 			filename 	文件标识名（你将本地文件推送给前端时，前端并不知道这个文件叫什么名字，你可以通过改参数告知前端）
 * 示例 ：
 			send_http_file(conn,"/1.c","3.c");
 			//将本地的文件1.c 推送给前端，并告诉前端这个文件标志是3.c(注意这里只是标志为3.c，但是前端想存为什么名字是它的自由)
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
***********************************************************************/
void send_chunk_file(struct mg_connection *conn,const char * pathname,const char * filename);

/***********************************************************************
 * 函数名称 : send_json_data
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 上传json 数据
 * 参    数 : jsondata  json格式的字符串(使用者负责入参的检测，该接口默认jsondata就是json数据)
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
***********************************************************************/
int send_json_data(struct mg_connection *conn,char * jsondata);




/***********************************************************************
 * 函数名称 : mg_get_header_value
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 获取 header 的value ，header都是以 name:value 的键值对形式存在的
 * 参    数 : name  header name
 * 返 回 值 : 	name存在，则返回 value；不存在，则返回NULL
 * 修改日期           修改人	      修改内容
***********************************************************************/
const char *mg_get_header_value(const struct mg_connection *conn,const char *name);



/***********************************************************************
 * 函数名称 : mg_get_var_from_querystring
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 从 query_string 中获取参数name对应的 Valer的值，保存在dst中
 				该函数类比于 mg_get_var 函数
 * 参    数 : name  名字
 * 参    数 : dst   存储value的buf
 * 参    数 : dst_len  dst的size
 * 返 回 值 : 	dst为null 或者 dst_len 为0	或者装不下				:返回-2
 				name为null								:返回-1
 				正常										:返回 value长度

 * 修改日期           修改人	      修改内容
***********************************************************************/
int mg_get_var_from_querystring(const struct mg_connection *conn, const char *name,char *dst, size_t dst_len);




/***********************************************************************
 * 函数名称 : mg_printf_data
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 自动添加chunk header，且将fmt 提供的参数以chunk格式发送
 				该函数创建的目的只是单纯的模仿 mongoose 中的同名函数
 * 参数		：send_header 是否发送 header：Transfer-Encoding: chunked
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
***********************************************************************/
#define IOBUF_SIZE 8192
int mg_printf_data(struct mg_connection *conn,bool send_header, const char *fmt, ...);


/***********************************************************************
 * 函数名称 : BC_mg_send_header
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 自动添加响应行：HTTP/1.1 200 OK\r\n
 				然后添加使用传入的header 参数：name :value\r\n
 				如：	HTTP/1.1 200 OK\r\nConnection :close\r\n
 				该函数创建的目的只是单纯的模仿 mongoose 中的mg_send_header函数
 * 返 回 值 :
 * 修改日期           修改人	      修改内容
***********************************************************************/
void BC_mg_send_header(struct mg_connection *conn, const char *name, const char *value);

/***********************************************************************
 * 函数名称 : BC_GET_Request_Body
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 将请求的 存储在buf中

 * 参数		：buf 存储器
 * 参数		：len   buf的大小
 * 返 回 值 :		0 	表示没有数据可读了
 				<0 	表示出错
 				>0	表示buf里读了多少字节
 * 修改日期           修改人	      修改内容
***********************************************************************/
int BC_GET_Request_Body(struct mg_connection *conn, void *buf, size_t len);

/***********************************************************************
 * 函数名称 : STore_body_to_file
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 将请求的 body（一般用于 post请求）存储在文件（i_store_file）中

 * 参数		：store_file body会存到这个文件中
 * 返 回 值 :		body的长度	，失败返回 <= 0
 * 修改日期           修改人	      修改内容
***********************************************************************/
long long STore_body_to_file(struct mg_connection *conn,const char * i_store_file);







/***********************************************************************
 * 函数名称 : My_System
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 执行shell指令。
 * 参    数 :   cmdstring 要执行的shell
 * 返 回 值 : 失败返回-1；成功返回执行状态码
 * 修改日期           修改人	      修改内容
***********************************************************************/
int My_System(const char * cmdstring);



/***********************************************************************
 * 函数名称 : Get_Line_From_File
 * 作    者 :
 * 设计日期 :
 * 功能描述 : 从文件中获取 第 i_cnt 行的内容。
 * 参    数 :
 	i_filename 目标文件
	o_storebuf 读取的内容存到这个buf
	i_cnt 要读取第几行 （> 0）
 * 返 回 值 :
	 0 成功读到这一行数据，且存到了 o_storebuf
	-1 文件打开失败
	-2 参数 i_cnt <= 0
	-3 文件没有这一行
	-4 未知的错误
 * 修改日期           修改人	      修改内容
***********************************************************************/
int Get_Line_From_File(const char * i_filename,string& o_storebuf,int i_cnt);


#endif
