#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "debug.h"
#include "Civetweb_API.h"
#include <string.h>
#include <dirent.h>
#include <algorithm>
#include <errno.h>
#include "mutex.h"

#define OPTION_MAX_NUMBER 126			//�˺�Ϊ �ڲ�ָ���� option �б������ֵ������


static const char *CT_status_code_to_str(int status_code)
{
  switch (status_code)
  {
    case 100: return "Continue";
    case 101: return "Switching Protocols";
    case 102: return "Processing";

    case 200: return "OK";
    case 201: return "Created";
    case 202: return "Accepted";
    case 203: return "Non-Authoritative Information";
    case 204: return "No Content";
    case 205: return "Reset Content";
    case 206: return "Partial Content";
    case 207: return "Multi-Status";
    case 208: return "Already Reported";
    case 226: return "IM Used";

    case 300: return "Multiple Choices";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 303: return "See Other";
    case 304: return "Not Modified";
    case 305: return "Use Proxy";
    case 306: return "Switch Proxy";
    case 307: return "Temporary Redirect";
    case 308: return "Permanent Redirect";

    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 402: return "Payment Required";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 406: return "Not Acceptable";
    case 407: return "Proxy Authentication Required";
    case 408: return "Request Timeout";
    case 409: return "Conflict";
    case 410: return "Gone";
    case 411: return "Length Required";
    case 412: return "Precondition Failed";
    case 413: return "Payload Too Large";
    case 414: return "URI Too Long";
    case 415: return "Unsupported Media Type";
    case 416: return "Requested Range Not Satisfiable";
    case 417: return "Expectation Failed";
    case 418: return "I\'m a teapot";
    case 422: return "Unprocessable Entity";
    case 423: return "Locked";
    case 424: return "Failed Dependency";
    case 426: return "Upgrade Required";
    case 428: return "Precondition Required";
    case 429: return "Too Many Requests";
    case 431: return "Request Header Fields Too Large";
    case 451: return "Unavailable For Legal Reasons";

    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    case 504: return "Gateway Timeout";
    case 505: return "HTTP Version Not Supported";
    case 506: return "Variant Also Negotiates";
    case 507: return "Insufficient Storage";
    case 508: return "Loop Detected";
    case 510: return "Not Extended";
    case 511: return "Network Authentication Required";

    default:  return "Server Error";
  }
}



/*start the civetweb server*/
#ifdef USE_SSL_DH
//#include "openssl/dh.h"
//#include "openssl/ec.h"
//#include "openssl/ecdsa.h"
//#include "openssl/evp.h"
///#include "openssl/ssl.h"

DH * get_dh2236()
{
	static unsigned char dh2236_p[] = {
	    0x0E, 0x97, 0x6E, 0x6A, 0x88, 0x84, 0xD2, 0xD7, 0x55, 0x6A, 0x17, 0xB7,
	    0x81, 0x9A, 0x98, 0xBC, 0x7E, 0xD1, 0x6A, 0x44, 0xB1, 0x18, 0xE6, 0x25,
	    0x3A, 0x62, 0x35, 0xF0, 0x41, 0x91, 0xE2, 0x16, 0x43, 0x9D, 0x8F, 0x7D,
	    0x5D, 0xDA, 0x85, 0x47, 0x25, 0xC4, 0xBA, 0x68, 0x0A, 0x87, 0xDC, 0x2C,
	    0x33, 0xF9, 0x75, 0x65, 0x17, 0xCB, 0x8B, 0x80, 0xFE, 0xE0, 0xA8, 0xAF,
	    0xC7, 0x9E, 0x82, 0xBE, 0x6F, 0x1F, 0x00, 0x04, 0xBD, 0x69, 0x50, 0x8D,
	    0x9C, 0x3C, 0x41, 0x69, 0x21, 0x4E, 0x86, 0xC8, 0x2B, 0xCC, 0x07, 0x4D,
	    0xCF, 0xE4, 0xA2, 0x90, 0x8F, 0x66, 0xA9, 0xEF, 0xF7, 0xFC, 0x6F, 0x5F,
	    0x06, 0x22, 0x00, 0xCB, 0xCB, 0xC3, 0x98, 0x3F, 0x06, 0xB9, 0xEC, 0x48,
	    0x3B, 0x70, 0x6E, 0x94, 0xE9, 0x16, 0xE1, 0xB7, 0x63, 0x2E, 0xAB, 0xB2,
	    0xF3, 0x84, 0xB5, 0x3D, 0xD7, 0x74, 0xF1, 0x6A, 0xD1, 0xEF, 0xE8, 0x04,
	    0x18, 0x76, 0xD2, 0xD6, 0xB0, 0xB7, 0x71, 0xB6, 0x12, 0x8F, 0xD1, 0x33,
	    0xAB, 0x49, 0xAB, 0x09, 0x97, 0x35, 0x9D, 0x4B, 0xBB, 0x54, 0x22, 0x6E,
	    0x1A, 0x33, 0x18, 0x02, 0x8A, 0xF4, 0x7C, 0x0A, 0xCE, 0x89, 0x75, 0x2D,
	    0x10, 0x68, 0x25, 0xA9, 0x6E, 0xCD, 0x97, 0x49, 0xED, 0xAE, 0xE6, 0xA7,
	    0xB0, 0x07, 0x26, 0x25, 0x60, 0x15, 0x2B, 0x65, 0x88, 0x17, 0xF2, 0x5D,
	    0x2C, 0xF6, 0x2A, 0x7A, 0x8C, 0xAD, 0xB6, 0x0A, 0xA2, 0x57, 0xB0, 0xC1,
	    0x0E, 0x5C, 0xA8, 0xA1, 0x96, 0x58, 0x9A, 0x2B, 0xD4, 0xC0, 0x8A, 0xCF,
	    0x91, 0x25, 0x94, 0xB4, 0x14, 0xA7, 0xE4, 0xE2, 0x1B, 0x64, 0x5F, 0xD2,
	    0xCA, 0x70, 0x46, 0xD0, 0x2C, 0x95, 0x6B, 0x9A, 0xFB, 0x83, 0xF9, 0x76,
	    0xE6, 0xD4, 0xA4, 0xA1, 0x2B, 0x2F, 0xF5, 0x1D, 0xE4, 0x06, 0xAF, 0x7D,
	    0x22, 0xF3, 0x04, 0x30, 0x2E, 0x4C, 0x64, 0x12, 0x5B, 0xB0, 0x55, 0x3E,
	    0xC0, 0x5E, 0x56, 0xCB, 0x99, 0xBC, 0xA8, 0xD9, 0x23, 0xF5, 0x57, 0x40,
	    0xF0, 0x52, 0x85, 0x9B,
	};
	static unsigned char dh2236_g[] = {
	    0x02,
	};
	DH *dh;

	if ((dh = DH_new()) == NULL)
		return (NULL);
	dh->p = BN_bin2bn(dh2236_p, sizeof(dh2236_p), NULL);
	dh->g = BN_bin2bn(dh2236_g, sizeof(dh2236_g), NULL);
	if ((dh->p == NULL) || (dh->g == NULL)) {
		DH_free(dh);
		return (NULL);
	}
	return (dh);
}
#endif

#ifndef TEST_WITHOUT_SSL
static int init_ssl(void *ssl_context, void *user_data)
{
	/* Add application specific SSL initialization �����ض���Ӧ�ó����SSL��ʼ��*/
	struct ssl_ctx_st *ctx = (struct ssl_ctx_st *)ssl_context;

#ifdef USE_SSL_DH
	/* example from https://github.com/civetweb/civetweb/issues/347 */
	DH *dh = get_dh2236();
	if (!dh)
		return -1;
	if (1 != SSL_CTX_set_tmp_dh(ctx, dh))
		return -1;
	DH_free(dh);

	EC_KEY *ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	if (!ecdh)
		return -1;
	if (1 != SSL_CTX_set_tmp_ecdh(ctx, ecdh))
		return -1;
	EC_KEY_free(ecdh);

	BC_INFO_LOG("ECDH ciphers initialized");
#else
	BC_INFO_LOG("ssl_ctx_st = [%p]",ctx);
#endif
	return 0;
}
#endif


static int log_message(const struct mg_connection *conn, const char *message)
{
	puts(message);
	return 1;
}

//��Ϊ�첽�ظ���ԭ����Ҫ�� timeout ʱ���ú���Ӧ�ûظ�һ�� 500 ��ǰ��
static void user_connection_close(const struct mg_connection *conn)
{
	// /websocket not handle
	const struct mg_request_info * ri = BC_mg_get_request_info(conn);
	BC_INFO_LOG("ri = [%p]",ri);
	if(ri != NULL && 0 == ::strncmp(ri->request_uri,"/websocket",10))
	{
		BC_INFO_LOG("websocket close , do nothing!!!");
		return;
	}

	UserConnData * t_tauserda = (UserConnData *)BC_mg_get_user_connection_data((struct mg_connection *)conn);//��� userdata ΪNULL����ô��ʾ ��userdata�Ѿ����û�����������do nothing
	if(t_tauserda)
	{
		//send rsp to conn
		if(false == t_tauserda->Rspflag)
		{
			send_http_usererror_rsp((struct mg_connection *)conn,408);		//����ʱ
			t_tauserda->Rspflag = true;
		}

		//free userdata
		BC_INFO_LOG("free %p",t_tauserda);
		free(t_tauserda);

		//set userdat NULL
		BC_mg_set_user_connection_data((struct mg_connection *)conn,NULL);
	}

}

static void Get_User_Provide_Port(web_options*i_weboptions,Server_Start_Mode i_mode,string& o_port)
{
	bool http_port_exist = false;
	bool https_port_exist = false;
	string p_http_port_num = i_weboptions->web_allconf_http_port;
	string p_https_port_num = i_weboptions->web_allconf_https_port;
	if(0 != i_weboptions->web_allconf_http_port.length())
	{
		http_port_exist = true;
	}
	if(0 != i_weboptions->web_allconf_https_port.length())
	{
		https_port_exist = true;
	}


	if(Server_Start_Mode_Http == i_mode && http_port_exist)//http ģʽ��ֻȡhttp�˿�
	{
		o_port = p_http_port_num;
	}
	else if(Server_Start_Mode_Https == i_mode && https_port_exist)//https ģʽ��ֻȡhttps�˿�
	{
#if 1
		if(http_port_exist)
		{
			o_port = p_http_port_num + "r,";
		}
#endif
		o_port = o_port + p_https_port_num + "s";
	}
	else if(Server_Start_Mode_all == i_mode)//http and https ģʽ�������˿ڶ�ȡ
	{
		if(http_port_exist)
		{
			o_port = p_http_port_num;
		}
		if(https_port_exist)
		{
			if(http_port_exist)
			{
				o_port = o_port + ",";
			}
			o_port = o_port + p_https_port_num + "s";
		}
	}
	else if(Server_Start_Mode_auto == i_mode)//auto ģʽ�������û��ṩ�Ķ˿ں��Զ����
	{
		o_port = p_http_port_num;
		if(http_port_exist)
		{
			o_port = o_port + ",";
		}
		if(https_port_exist)
		{
			o_port = o_port + p_https_port_num;
		}
	}
	else
	{
		BC_ERROR_LOG("Unexpected startup mode");
	}
	BC_INFO_LOG("Confirm start port : %s",o_port.c_str());
}
static void Print_Option_List(const char ** list,int count)
{
	int loop = 0;
	for(;loop < count;loop+=2)
	{
		BC_INFO_LOG("{%s,%s}",list[loop],list[loop+1]);
	}
}
struct mg_context *ServerStart(web_options*weboptions,Server_Start_Mode Server_mode,void *user_data)
{
	//��μ��
	if(weboptions == NULL)
	{
		BC_ERROR_LOG("Param weboptions is NULL");
		return NULL;
	}
	//option ��û�� default ֵ��ѡ��ʹ���߱��븳ֵ���������������
	if(0 == weboptions->web_allconf_documentroot.length())
	{
		BC_ERROR_LOG("Param weboptions document_root is NULL");
		return NULL;
	}
	if((Server_Start_Mode_all == Server_mode && 0 ==  weboptions->web_httpsconf_cerm.length() ) ||
			(Server_Start_Mode_Https == Server_mode && 0 ==  weboptions->web_httpsconf_cerm.length()))
	{
		BC_ERROR_LOG("Param weboptions cerm file path is NULL");
		return NULL;
	}
	string web_port = "";
	Get_User_Provide_Port(weboptions,Server_mode,web_port);
	//option ָ������
	const char *options[OPTION_MAX_NUMBER] = {0};
	int option_loop = 0;
#if !defined(NO_FILES)
	options[option_loop++] = "document_root";
    options[option_loop++] = weboptions->web_allconf_documentroot.c_str();
#endif
	if(web_port.length())
	{
		options[option_loop++] = "listening_ports";
		options[option_loop++] = web_port.c_str();
	}
	options[option_loop++] = "request_timeout_ms";
	options[option_loop++] = "10000";
	options[option_loop++] = "error_log_file";
	options[option_loop++] = "error.log";
	if(weboptions->web_allconf_enable_auth_domain_check.length())
	{
		options[option_loop++] = "enable_auth_domain_check";
		options[option_loop++] = weboptions->web_allconf_enable_auth_domain_check.c_str();
	}
	if(weboptions->web_allconf_authentication_domain.length())
	{
		options[option_loop++] = "authentication_domain";
		options[option_loop++] = weboptions->web_allconf_authentication_domain.c_str();
	}

#ifdef USE_WEBSOCKET
	options[option_loop++] = "enable_websocket_ping_pong";
	if(weboptions->web_wsconf_enable_pingpong.length())
	{
		options[option_loop++] = weboptions->web_wsconf_enable_pingpong.c_str();
	}
	else
	{
		options[option_loop++] = "no";
	}

	options[option_loop++] = "websocket_timeout_ms";
	if(weboptions->web_wsconf_pingpong_timeout_ms.length())
	{
		options[option_loop++] = weboptions->web_wsconf_pingpong_timeout_ms.c_str();
	}
	else
	{
		options[option_loop++] = "3600000";
	}
#endif
#ifndef TEST_WITHOUT_SSL
	options[option_loop++] = "ssl_certificate";
    options[option_loop++] = weboptions->web_httpsconf_cerm.c_str();
	options[option_loop++] = "ssl_protocol_version";
	options[option_loop++] = "3";
	options[option_loop++] = "ssl_cipher_list";
#ifdef USE_SSL_DH
	options[option_loop++] = "ECDHE-RSA-AES256-GCM-SHA384:DES-CBC3-SHA:AES128-SHA:AES128-GCM-SHA256";
#else
	options[option_loop++] = "DES-CBC3-SHA:AES128-SHA:AES128-GCM-SHA256";
#endif
#endif
	if(weboptions->web_allconf_thread_num.length())
	{
		options[option_loop++] = "num_threads";
		options[option_loop++] = weboptions->web_allconf_thread_num.c_str();
	}
	if(weboptions->web_allconf_enable_keepalive.length())
	{
		options[option_loop++] = "enable_keep_alive";
		options[option_loop++] = weboptions->web_allconf_enable_keepalive.c_str();
	}
	if(weboptions->web_allconf_keepalive_timeout.length())
	{
		options[option_loop++] = "keep_alive_timeout_ms";
		options[option_loop++] = weboptions->web_allconf_keepalive_timeout.c_str();
	}


	//set options finish , ready start
	if(OPTION_MAX_NUMBER < option_loop)
	{
		BC_ERROR_LOG("Too many options,can't start civetweb");
		return NULL;
	}
	Print_Option_List(options,option_loop);

	struct mg_callbacks callbacks;
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.init_ssl = init_ssl;
	callbacks.log_message = log_message;
//	callbacks.connection_close = user_connection_close;
	return mg_start(&callbacks,user_data,options);
}



/* List and printf all listening ports */
void List_All_ServerPort(struct mg_context *ctx)
{
	struct mg_server_port ports[32];
	int port_cnt, n;
	memset(ports, 0, sizeof(ports));
	port_cnt = mg_get_server_ports(ctx, 32, ports);
	BC_INFO_LOG("%i listening ports:", port_cnt);

	for(n = 0; n < port_cnt && n < 32; n++)
    {
        const char *proto = ports[n].is_ssl ? "https" : "http";
        const char *host;

        if ((ports[n].protocol & 1) == 1)
        {
            /* IPv4 */
            host = "127.0.0.1";
            BC_INFO_LOG( "You can Browse files at %s://%s:%i", proto, host, ports[n].port);
        }

        if ((ports[n].protocol & 2) == 2)
        {
            /* IPv6 */
            host = "[::1]";
            BC_INFO_LOG( "You can Browse files at %s://%s:%i", proto, host, ports[n].port);
        }
    }
}

/*stop the civetweb server*/
void ServerStop(struct mg_context *ctx)
{
	mg_stop(ctx);
}



/*add uri process handler*/
void AddURIProcessHandler(struct mg_context *ctx,struct uri_list * list,int list_size)
{
	int loop = 0;
	for(;loop < list_size;loop++)
	{
		if(list[loop].uri == NULL || list[loop].handler == NULL)
		{
			BC_WARN_LOG("ADD URI handler error : list[%d]",loop);
			continue;
		}
		mg_set_request_handler(ctx,list[loop].uri ,list[loop].handler,list[loop].user_data);
	}

}

static list<ws_clients> gb_clientconnlist;
static CMutex g_WsCliListMutex;


/*
	websocket��������ʾ�����������յ�websocket�����ݺ�͸������������ִ��
*/
#ifdef USE_WEBSOCKET

mg_websocket_handler my_websocket_handler = NULL;

/*
	��websocket client list �����������
*/
static bool DeleteWSClientInList(const struct mg_connection *conn)
{
	struct mg_context *ctx = mg_get_context(conn);
	mg_lock_context(ctx);
	list<ws_clients>::iterator ws_clientIterator;
	{
		CMutexLocker locker(&g_WsCliListMutex);
		for(ws_clientIterator = gb_clientconnlist.begin();ws_clientIterator!=gb_clientconnlist.end();++ws_clientIterator)
		{
			if(ws_clientIterator->conn == conn)
			{
				BC_INFO_LOG("earse conn from list [%p]",ws_clientIterator->conn);
				ws_clientIterator->conn = NULL;
				ws_clientIterator->local_auth = WEBSVR_AUTH_UNKNOW;
				gb_clientconnlist.erase(ws_clientIterator);
				break;
			}
		}
	}
	mg_unlock_context(ctx);
	if(gb_clientconnlist.end() == ws_clientIterator)
	{
		BC_ERROR_LOG("error: don't find this we_clientconn in list!!!!");
		return false;
	}
	else
	{
		BC_INFO_LOG( "Delete [%p] In List",conn);
		return true;
	}
}

int SetWebsocketLocalAuthSucess(struct mg_connection *conn)
{
	CMutexLocker locker(&g_WsCliListMutex);
	list<ws_clients>::iterator ws_clientIterator;
	for(ws_clientIterator = gb_clientconnlist.begin();ws_clientIterator!=gb_clientconnlist.end();++ws_clientIterator)
	{
		if(ws_clientIterator->conn == conn)
		{
			ws_clientIterator->local_auth = WEBSVR_AUTH_SUCCESS;
			return 1;
		}
	}
	return 0;
}
int SetWebsocketLDAPAuthSucess(struct mg_connection *conn)
{
	CMutexLocker locker(&g_WsCliListMutex);
	list<ws_clients>::iterator ws_clientIterator;
	for(ws_clientIterator = gb_clientconnlist.begin();ws_clientIterator!=gb_clientconnlist.end();++ws_clientIterator)
	{
		if(ws_clientIterator->conn == conn)
		{
			ws_clientIterator->ldap_auth = WEBSVR_AUTH_SUCCESS;
			return 1;
		}
	}
	return 0;
}


void Send_Data_To_CurWebsocket(struct mg_connection *conn, const char *data, size_t len)
{
	mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT, (const char *)data, len);
}


void Send_Data_To_ALLWebsocket(const char *data,size_t len)
{
	list<ws_clients>::iterator ws_clientIterator;
	for(ws_clientIterator = gb_clientconnlist.begin();ws_clientIterator!=gb_clientconnlist.end();++ws_clientIterator)
	{
		/*������֤ģʽ ��ʱȡ��һ*/
		if(ws_clientIterator->local_auth == WEBSVR_AUTH_SUCCESS || ws_clientIterator->ldap_auth == WEBSVR_AUTH_SUCCESS)
		{
			mg_websocket_write(ws_clientIterator->conn, MG_WEBSOCKET_OPCODE_TEXT, (const char *)data, len);
		}
	}
}



int WebSocketConnectHandler(const struct mg_connection *conn, void *cbdata)
{
	struct mg_context *ctx = mg_get_context(conn);
	mg_lock_context(ctx);

	ws_clients WebscketTempConn;
	WebscketTempConn.conn = (struct mg_connection *)conn;
	WebscketTempConn.local_auth = WEBSVR_AUTH_UNKNOW;
	WebscketTempConn.ldap_auth = WEBSVR_AUTH_UNKNOW;
	{
		CMutexLocker locker(&g_WsCliListMutex);
		gb_clientconnlist.push_back(WebscketTempConn);
	}

	#if 0
	list<ws_clients>::iterator ws_clientIterator;
	for(ws_clientIterator = gb_clientconnlist.begin();ws_clientIterator!=gb_clientconnlist.end();++ws_clientIterator)
	{
		if(conn == ws_clientIterator->conn)
		{
			reject = 0;
			break;
		}
	}
	#endif
	mg_unlock_context(ctx);
	return 0;
}


void WebSocketReadyHandler(struct mg_connection *conn, void *cbdata)
{
   	SetWebsocketLocalAuthSucess(conn);
}


int WebsocketDataHandler(struct mg_connection *conn,
                     int bits,
                     char *data,
                     size_t len,
                     void *cbdata)
{
	switch (((unsigned char)bits) & 0x0F)
	{
		case MG_WEBSOCKET_OPCODE_CONTINUATION:
			break;
		case MG_WEBSOCKET_OPCODE_TEXT:
	    {

	        int nlen = len+1;
	        char * real_data = (char *)malloc(nlen);
	        memset(real_data,0,nlen);
	        ::memcpy(real_data,data,len);

			if(my_websocket_handler)
			{
				my_websocket_handler(conn,real_data,nlen);
			}
			else
			{
				BC_ERROR_LOG("websocket data handler is NULL");
			}

			memset(real_data,0,nlen);
			free(real_data);
	        break;
	    }

		case MG_WEBSOCKET_OPCODE_BINARY:
			BC_INFO_LOG("binary");
			break;
		case MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE:
			BC_INFO_LOG("close");
			break;
		case MG_WEBSOCKET_OPCODE_PING:
			BC_INFO_LOG("ping");
			break;
		case MG_WEBSOCKET_OPCODE_PONG:
			BC_INFO_LOG("websocket recv pong from [%p]",conn);
			break;
		default:
			BC_INFO_LOG("unknown(%1xh)", ((unsigned char)bits) & 0x0F);
			break;
	}
	return 1;
}


void WebSocketCloseHandler(const struct mg_connection *conn, void *cbdata)
{
	DeleteWSClientInList(conn);
}

#endif

/*set websocket handler*/
void SetWebsocketHandler(struct mg_context *ctx,
			                         mg_websocket_handler handler,
			                         void *cbdata)
{
	my_websocket_handler = handler;
	mg_set_websocket_handler(ctx,
	                         "/websocket",
	                         WebSocketConnectHandler,
	                         WebSocketReadyHandler,
	                         WebsocketDataHandler,
	                         WebSocketCloseHandler,
	                         cbdata);
}


/*check IPV6 feature*/
int Civetweb_check_IPV6_feature()
{
	return mg_check_feature(8);
}

/*check WEBSOCKET feature*/
int Civetweb_check_WEBSOCKET_feature()
{
	return mg_check_feature(16);
}

/*check ssl feature*/
int Civetweb_check_SSL_feature()
{
	return mg_check_feature(2);
}


/*get file size*/
unsigned long get_file_size(const char *path)
{
	unsigned long filesize = 0;
	struct stat statbuff;
	if(stat(path, &statbuff) < 0){
		return filesize;
	}else{
		filesize = statbuff.st_size;
	}
	return filesize;
}

#if 0
static void bin2str(char *to, const unsigned char *p, size_t len)
{
    static const char *hex = "0123456789abcdef";

    for (; len--; p++) {
        *to++ = hex[p[0] >> 4];
        *to++ = hex[p[0] & 0x0f];
    }
    *to = '\0';
}
#endif

void Parse_String_Json(const char* i_pSrc, const char* i_pFlag, string &o_string)
{
    if(NULL == i_pSrc || NULL == i_pFlag)
    {
        return;
    }

    string strsrc = i_pSrc;
    o_string.clear();
    int nend = -1;

    int nstart = strsrc.find(i_pFlag);
    if(nstart >= 0)
    {
        nstart += ::strlen(i_pFlag);
        o_string = strsrc.substr(nstart);
        nend = o_string.find("\"");
        if(nend >= 0)
        {
            o_string.erase(nend);
        }
    }

}


/*set formdata handler*/
int Set_Formdata_Handler(struct mg_connection *conn,struct mg_form_data_handler *fdh)
{
	return mg_handle_form_request(conn,fdh);
}

void * BC_mg_get_user_connection_data(struct mg_connection *conn)
{
	return mg_get_user_connection_data(conn);
}
void  BC_mg_set_user_connection_data(struct mg_connection *conn,void * data)
{
	mg_set_user_connection_data(conn,data);
}

const struct mg_request_info *BC_mg_get_request_info(const struct mg_connection * conn)
{
	return mg_get_request_info(conn);
}
bool BC_CheckRequestMethod(const struct mg_connection * conn,const char * p_Method)
{
	const struct mg_request_info *ri = mg_get_request_info(conn);
	if(0 == ::strcasecmp(ri->request_method, p_Method))
	{
		return true;
	}
	return false;
}


/*get time*/
long long get_time_ms()
{
    int err = 0;
    struct timespec ts;
    err = ::clock_gettime(CLOCK_MONOTONIC, &ts);
    if (err < 0)
    {
        return -1;
    }
    return (long long)ts.tv_sec * 1000 + (long long)ts.tv_nsec / 1000000;
}



/*
	struct mg_request_info {
	const char *request_method;   "GET", "POST", etc
	const char *request_uri;     URL-decoded URI (absolute or relative,
	                            as in the request)
	const char *local_uri;        URL-decoded URI (relative). Can be NULL
	                             if the request_uri does not address a
	                               resource at the server host.
//#if defined(MG_LEGACY_INTERFACE)  2017-02-04, deprecated 2014-09-14
	const char *uri;              Deprecated: use local_uri instead
//#endif
	const char *http_version;  E.g. "1.0", "1.1"
	const char *query_string;  URL part after '?', not including '?', or
	                             NULL
	const char *remote_user;  Authenticated user, or NULL if no auth
	                             used
	char remote_addr[48];      Client's IP address as a string.

	long long content_length; Length (in bytes) of the request body,
	                             can be -1 if no length was given.
	int remote_port;           Client's port
	int is_ssl;                1 if SSL-ed, 0 if not
	void *user_data;          User data pointer passed to mg_start()
	void *conn_data;          Connection-specific user data

	int num_headers; Number of HTTP headers
	struct mg_header
	    http_headers[MG_MAX_HEADERS];  Allocate maximum headers

	struct mg_client_cert *client_cert; Client certificate information

	const char *acceptedWebSocketSubprotocol; websocket subprotocol,
	                                            accepted during handshake
};
*/

void Print_Request_Info(const struct mg_connection *conn)
{
	const struct mg_request_info *req_info = mg_get_request_info(conn);
	printf("request_method : %s\n",req_info->request_method);
	printf("request_uri : %s\n",req_info->request_uri);
	printf("local_uri : %s\n",req_info->local_uri);
	printf("http_version : %s\n",req_info->http_version);
	printf("query_string : %s\n",req_info->query_string);
	printf("remote_user : %s\n",req_info->remote_user);
	printf("remote_addr : %s\n",req_info->remote_addr);

	printf("content_length : %lld\n",req_info->content_length);
	printf("remote_port : %d\n",req_info->remote_port);
	printf("is_ssl : %d\n",req_info->is_ssl);
	printf("num_headers : %d\n",req_info->num_headers);
	int loop = 0;
	int num = req_info->num_headers >  MG_MAX_HEADERS ? MG_MAX_HEADERS : req_info->num_headers;
	for(;loop < num;loop++)
	{
		printf("%s<--->%s\n",req_info->http_headers[loop].name,req_info->http_headers[loop].value);
	}


	printf("acceptedWebSocketSubprotocol : %s\n",req_info->acceptedWebSocketSubprotocol);
}

void mg_close_cur_connection(struct mg_connection *conn)
{
	mg_close_connection(conn);
}


int BC_mg_write(struct mg_connection *conn, const void *buf, size_t len)
{
	return mg_write(conn,buf,len);
}



void send_http_ok_rsp(struct mg_connection *conn)
{
	mg_printf(conn,"%s","HTTP/1.1 200 OK\r\n"
							"Server: Apache-Coyote/1.1\r\n"
							"Content-Length: 0\r\n\r\n");
}

void send_http_error_rsp(struct mg_connection *conn)
{
	mg_printf(conn,"%s","HTTP/1.1 500 Internal Server Error\r\n"
							"Server: Apache-Coyote/1.1\r\n"
							"Content-Length: 0\r\n"
							"Connection: close\r\n\r\n");
}

void send_http_usererror_rsp(struct mg_connection *conn,int status)
{
	mg_printf(conn,"HTTP/1.1 %d %s\r\n%s",status,CT_status_code_to_str(status),
							"Server: Apache-Coyote/1.1\r\n"
							"Content-Length: 0\r\n"
							"Connection: close\r\n\r\n");
}

void send_http_usererror_body_rsp(struct mg_connection *conn,int status, const char* body)
{
	mg_printf(conn,"HTTP/1.1 %d %s\r\n"
					"Server: Apache-Coyote/1.1\r\n"
					"Content-Length: %d\r\n"
					"Connection: close\r\n\r\n"
					"%s\r\n\r\n",
					status,
					CT_status_code_to_str(status),
					body ? strlen(body) : 0,
					body ? body : "{}"
				);
}


void send_http_pwd_fail_rsp(struct mg_connection *conn)
{
	mg_printf(conn,"%s","HTTP/1.1 401 Unauthorized\r\n"
							"Server: Apache-Coyote/1.1\r\n"
							"Content-Length: 0\r\n\r\n");
}

static int Sendhtml(struct mg_connection *conn,const char * htmlpath)
{
	int filesize = 0;
	char buf[1024] = {0};
    struct stat st;
    int n;
    FILE *fp;
    if (stat(htmlpath, &st) == 0 && (fp = fopen(htmlpath, "rb")) != NULL)
    {
        while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
        {
            int write_ret = mg_write(conn, buf, n);
            if(write_ret <= 0)
            {
            	BC_ERROR_LOG("write error");
            	fclose(fp);
				return -1;
            }
            ::memset(buf,0,sizeof(buf));
            filesize = filesize + write_ret;
            //usleep(100);
        }
        fclose(fp);
        mg_write(conn, "\r\n", 2);
    }
    else
    {
		BC_ERROR_LOG("send file error");
		return -1;
    }
	return filesize;
}

int send_html_file(struct mg_connection *conn,const char * htmlpath)
{
	unsigned long filesize = get_file_size(htmlpath);
	BC_INFO_LOG("%s size is %lu",htmlpath,filesize);
	if(filesize == 0)
	{
		return -1;
	}
	mg_printf(conn, "HTTP/1.1 200 OK\r\n"
					"Content-Type: text/html\r\n"
					"Connection: close\r\n"
					"Content-Length: %lu\r\n"
					"Accept-Ranges: bytes\r\n\r\n",filesize);

	return Sendhtml(conn,htmlpath);
}


static int SendPic(mg_connection *conn, const char *path)
{
	char buf[1024] = {0};
    struct stat st;
    int n;
    FILE *fp;
    if (stat(path, &st) == 0 && (fp = fopen(path, "rb")) != NULL)
    {
        mg_printf(conn, "--w00t\r\nContent-Type: image/jpeg\r\n"
                "Content-Length: %lu\r\n\r\n", (unsigned long) st.st_size);
        while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
        {
            int write_ret = mg_write(conn, buf, n);
            if(write_ret <= 0)
            {
            	BC_ERROR_LOG("write error");
            	fclose(fp);
				return 0;
            }
            ::memset(buf,0,sizeof(buf));
            //usleep(100);
        }
        fclose(fp);
        mg_write(conn, "\r\n", 2);
    }
    else
    {
		BC_ERROR_LOG("send file error");
    }
	return 1;
}

int send_jpg_file(struct mg_connection *conn,const char * picpath)
{
	mg_printf(conn, "%s",
								"HTTP/1.1 200 OK\r\n" "Cache-Control: no-cache\r\n"
								"Pragma: no-cache\r\nExpires: Thu, 01 Dec 1994 16:00:00 GMT\r\n"
								"Connection: close\r\nContent-Type: multipart/x-mixed-replace; "
								"boundary=--w00t\r\n\r\n");
	SendPic(conn,picpath);
	return 0;
}

int sendjpgstream(struct mg_connection *conn,const char * picpath)
{
	/*��������Ҫ��ľ���һ��ʵʱ�ԣ�����Ϊ�˿������͸�ǰ�ˣ��������buf���ĺܴ�*/
	char buf[200 * 1024] = {0};
    struct stat st;
    int n;
    FILE *fp;
    if (stat(picpath, &st) == 0 && (fp = fopen(picpath, "rb")) != NULL)
    {
        mg_printf(conn, "--w00t\r\nContent-Type: image/jpeg\r\n"
                "Content-Length: %lu\r\n\r\n", (unsigned long) st.st_size);
        while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
        {
            int write_ret = mg_write(conn, buf, n);
            if(write_ret <= 0)
            {
				return 0;
            }
            //usleep(100);
        }
        fclose(fp);
        mg_write(conn, "\r\n", 2);
    }
    else
    {
		return 0;
    }
	return 1;
}

int send_jpg_stream_file(struct mg_connection *conn,const char * picpath,int time_ms)
{
	mg_printf(conn, "%s",
						"HTTP/1.1 200 OK\r\n"
						"Server: Apache-Coyote/1.1\r\n"
						"connection: close\r\n"
						"Content-Type: multipart/x-mixed-replace; boundary=--w00t\r\n\r\n");
	int result = 0;
	int sleeptime = time_ms*1000;
	while(1)
	{
		result = sendjpgstream(conn,picpath);
		if(result <= 0)
		{
			break;
		}
		::usleep(sleeptime);
	}

	return 1;
}



int send_chunk_file_Body(struct mg_connection *conn,const char * filepath)
{
	char buf[2000] = {0};
    struct stat st;
    int n;
    FILE *fp;
    if (stat(filepath, &st) == 0 && (fp = fopen(filepath, "rb")) != NULL)
    {
        while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
        {
            int write_ret = mg_send_chunk(conn, buf, n);
            if(write_ret < 0)
            {
            	BC_ERROR_LOG("mg_send_chunk error");
            	fclose(fp);
				return 0;
            }
            ::memset(buf,0,sizeof(buf));
        }
        fclose(fp);
        mg_send_chunk(conn,"",0);
    }
    else
    {
		BC_ERROR_LOG("open file error");
    }
	return 1;
}

void send_chunk_file(struct mg_connection *conn,const char * pathname,const char * filename)
{
	mg_printf(conn,"HTTP/1.1 200 OK\r\n"
							"Server: Apache-Coyote/1.1\r\n"
							"Content-Disposition: attachment;filename=\"%s\"\r\n"
							"Content-Type: application/octet-stream\r\n"
							"Transfer-Encoding: chunked\r\n\r\n",filename);
	send_chunk_file_Body(conn,(const char *)pathname);
}



int send_json_data(struct mg_connection *conn,char * jsondata)
{
	mg_printf(conn, "%s",
								"HTTP/1.1 200 OK\r\n"
								"Cache-Control: no-cache\r\n"
								"Connection: close\r\n"
								"Content-Type: application/json;charset=utf-8\r\n\r\n");
	mg_printf(conn,"%s",jsondata);
	return 0;
}




void mg_send_status(struct mg_connection *conn,int status)
{
	mg_printf(conn,"HTTP/1.1 %d %s\r\n",status,CT_status_code_to_str(status));

}


const char *mg_get_header_value(const struct mg_connection *conn,const char *name)
{
	return mg_get_header(conn,name);
}



int mg_get_var_from_querystring(const struct mg_connection *conn, const char *name,char *dst, size_t dst_len)
{
	const struct mg_request_info *req_info = mg_get_request_info(conn);

  	int len = mg_get_var(req_info->query_string, req_info->query_string == NULL ? 0 :
                    strlen(req_info->query_string), name, dst, dst_len);
  	return len;
}


//#define va_copy(x,y) x = y
static int ns_avprintf(char **buf, size_t size, const char *fmt, va_list ap)
{
  va_list ap_copy;
  int len;

  va_copy(ap_copy, ap);
  len = vsnprintf(*buf, size, fmt, ap_copy);
  va_end(ap_copy);

  if (len < 0)
  {
    // eCos and Windows are not standard-compliant and return -1 when
    // the buffer is too small. Keep allocating larger buffers until we
    // succeed or out of memory.
    //eCos��Windows�����ϱ�׼�����������̫С���򷵻�-1�� �����������Ļ�������ֱ���ɹ����ڴ治��Ϊֹ��
    *buf = NULL;
    while (len < 0)
    {
      if (*buf) free(*buf);
      size *= 2;
      if ((*buf = (char *)malloc(size)) == NULL) break;
      va_copy(ap_copy, ap);
      len = vsnprintf(*buf, size, fmt, ap_copy);
      va_end(ap_copy);
    }
  }
  else if (len > (int) size)
  {
    // Standard-compliant code path. Allocate a buffer that is large enough.
    //���ϱ�׼�Ĵ���·���� �����㹻��Ļ�������
    if ((*buf = (char *)malloc(len + 1)) == NULL)
    {
      len = -1;
    }
    else
    {
      va_copy(ap_copy, ap);
      len = vsnprintf(*buf, len + 1, fmt, ap_copy);
      va_end(ap_copy);
    }
  }

  return len;
}

int mg_printf_data(struct mg_connection *conn,bool send_header, const char *fmt, ...)
{
	va_list ap;
	int len;
	char mem[IOBUF_SIZE], *buf = mem;
	if(send_header)
	mg_printf(conn,"%s","Transfer-Encoding: chunked\r\n\r\n");

	va_start(ap, fmt);
	len = ns_avprintf(&buf, sizeof(mem), fmt, ap);
	va_end(ap);

	if(len >= 0)
	{
    	mg_send_chunk(conn, buf, len);
  	}
  	if(buf != mem && buf != NULL)
  	{
  		BC_WARN_LOG("IOBUF_SIZE is error,so Re-malloc and free success");
    	free(buf);
  	}
  	else
  	{
		//BC_INFO_LOG("IOBUF_SIZE is enough");
  	}

  	return len;
}

void BC_mg_send_header(struct mg_connection *conn, const char *name, const char *value)
{
	mg_printf(conn,"HTTP/1.1 %d %s\r\n", 200, CT_status_code_to_str(200));
	mg_printf(conn,"%s: %s\r\n",name,value);
}

int ACCESS_FILE_EXIT_R_OK(const char * file)
{
	if(access(file, F_OK) != 0)
	{
		BC_WARN_LOG("WARING file [%s]! not exist",file);
		return -1;
	}
	if(access(file, R_OK) != 0)
	{
		BC_WARN_LOG("WARING file [%s]! without read permission",file);
		return -2;
	}
	return 0;
}

int ACCESS_FILE_EXIT_W_OK(const char * file)
{
	if(access(file, F_OK) != 0)
	{
		BC_WARN_LOG("WARING file [%s]! not exist",file);
		return -1;
	}
	if(access(file, W_OK) != 0)
	{
		BC_WARN_LOG("WARING file [%s]! without write permission",file);
		return -2;
	}
	return 0;
}

int File2String(const char *i_pinFilePath, string &o_strDes)
{
	FILE *fp    = NULL;
	o_strDes   = "";
	if(0 != ACCESS_FILE_EXIT_R_OK(i_pinFilePath))
	{
		return -2;
	}

	fp = fopen(i_pinFilePath, "r");
	if(fp==NULL)
	{
		BC_ERROR_LOG("fopen %s error!!!",i_pinFilePath);
		return -1;
	}
	char sTmp[1024]		= {0};
	int nRet			= 0;
	while(1)
	{
		nRet = fread(sTmp,1,sizeof(sTmp),fp);
		if(nRet<= 0)
		{
			break;
		}
		o_strDes.append(sTmp,nRet);
	}
	fclose(fp);
	return o_strDes.length();
}
int StringtoFile(string i_pStrBuff, const char *i_pOutFileath)
{
	FILE *fp	= NULL;

	fp = fopen(i_pOutFileath,"w+");
	if(fp==NULL)
	{
		BC_ERROR_LOG("fopen %s error!!!",i_pOutFileath);
		return -1;
	}
	int nRet = fwrite(i_pStrBuff.c_str(),1,i_pStrBuff.length(),fp);
	fflush(fp);
	fclose(fp);
	return nRet;
}

int BC_GET_Request_Body(struct mg_connection *conn, void *buf, size_t len)
{
	return mg_read(conn,buf,len);
}



long long STore_body_to_file(struct mg_connection *conn,const char * i_store_file)
{
	if(NULL == i_store_file)
	{
		return -1;
	}
	long long bodyres = mg_store_body(conn,i_store_file);
	if(bodyres <= 0)
	{
		//BC_ERROR_LOG("mg_store_body error");
		return -2;
	}

	if(ACCESS_FILE_EXIT_R_OK(i_store_file) == -1)
	{
		return -3;
	}
	BC_INFO_LOG("mg_store_body <%s> success <%lld>\n",i_store_file,bodyres);
	return bodyres;
}








int My_System(const char * cmdstring)
{
	//BC_INFO_LOG("%s",cmdstring);
    pid_t pid;
    int status;

    if(cmdstring == NULL)
    {
		BC_ERROR_LOG("input is NULL!");
        return -1;
    }
    if((pid = fork())<0)
    {
    	perror("fork error:");
        return -1;
    }
    else if(pid == 0)
    {
        execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
        _exit(127);
    }
    else
    {
        while(waitpid(pid, &status, 0) < 0)
        {
            if(errno != EINTR)
            {
                status = -1;
                break;
            }
        }
    }

    return status;
}




int Get_Line_From_File(const char * i_filename,string& o_storebuf,int i_cnt)
{
	FILE * fp;
	int i=0;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen(i_filename, "r");
	if (fp == NULL)
	{
		perror("fopen :");
	 	return -1;
	}

	if(i_cnt <= 0)
	{
		BC_WARN_LOG("param i_cnt is < 0");
		return -2;
	}

	while((read = getline(&line, &len, fp)) != -1)
	{
		++i;
		if(i >= i_cnt)
		{
			break;
		}
	}
 	if(i_cnt > i)
 	{
 		BC_WARN_LOG("file not exist %d line",i_cnt);	//û����һ�ж��˳�����������������Դ�ӡ����
 		if(fclose(fp) == EOF)
		{
			perror("close ");
		}
		if(line)
		{
			free(line);
		}
 		return -3;
 	}
	if(line)
	{
		//memcpy(pOutputBuf,line,strlen(line));
		o_storebuf = line;
		free(line);
		if(fclose(fp) == EOF)
		{
			perror("close ");
		}
		return 0;
	}

	return -4;
}


