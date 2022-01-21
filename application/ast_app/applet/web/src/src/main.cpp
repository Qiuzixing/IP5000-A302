#ifdef NO_SSL
#define TEST_WITHOUT_SSL
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <getopt.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>

#include "debug.h"
#include "webapp.h"
//#include "process_json.h"

typedef struct ConfInfoFile
{
	string xmlfile;
	string jsonfile;
	string StartMode;
    string ModelNumber;
}ConfInfo;


int exitNow = 0;

static ConfInfoFile webconfig;


void CIVET_HELP(const char *Proname)
{
	printf("%s need [options]:\n", Proname);

    printf("	-h --help		show this help\n");
    printf("	-x --xml		set xml file path,this version not support xml ,please choose json\n");//����json�ļ��������������
    printf("	-j --json		set json file path\n");
    printf("	-m --mode		set web start mode (you can select : http/https/all)\n");
    printf("default: [%s -j ./gbwebserver.json -m http]\n", Proname);
}

const char* GetModelNumber(void)
{
    return webconfig.ModelNumber.c_str();
}

int GetConfFile(int argc, char **argv)
{
	int getopt_return_val = 0;
	int option_index = 0;

	static struct option longOpts[] =
    {
        { "help", no_argument, NULL, 'h' },
        { "xml", required_argument, NULL, 'x' },
        { "json", required_argument, NULL, 'j' },
        { "mode", required_argument, NULL, 'm' },
        { "model", required_argument, NULL, 'M' },
        { NULL, 0, NULL, 0 }
    };
	while((getopt_return_val = getopt_long(argc,argv,"hx:j:m:M:",longOpts,&option_index)) != -1)
	{
		 switch (getopt_return_val)
		 {
            case 'h':
                CIVET_HELP(argv[0]);
                return -1;
            case 'x':
                webconfig.xmlfile.append(optarg);
                break;
            case 'j':
                webconfig.jsonfile.append(optarg);
                break;
            case 'm':
                webconfig.StartMode.append(optarg);
                break;
            case 'M':
                webconfig.ModelNumber = optarg;
                break;
            default:
                CIVET_HELP(argv[0]);
                return -1;
        }
	}

	if(webconfig.xmlfile.empty())
		webconfig.xmlfile.append("./gbwebserver.xml");
	if(webconfig.jsonfile.empty())
        webconfig.jsonfile.append(DEFAULT_WEB_CONFIG_FILE);
	if(webconfig.StartMode.empty())
        ;
		//webconfig.StartMode.append("http");

	//printf("webserver : ready to get param from -j %s\n", webconfig.jsonfile.c_str());

	return 0;
}

void WebConfInit(void)
{
    // 初始化web config文件
    if(access(DEFAULT_WEB_CONFIG_FILE, F_OK) != 0)
    {
        if(COperation::AssertDirExists("/data/configs/kds-7/secure/webconfig") >= 0)
        {
            char szCmd[MAX_CMD_STR] = {0};
            sprintf(szCmd, "cp -f %s %s", WEB_CONFIG_FILE, DEFAULT_WEB_CONFIG_FILE);
            int ret = system(szCmd);
            if(ret < 0)
            {
                BC_ERROR_LOG("Init web config file failed!");
                exit(0);
            }
        }
    }

    // https证书文件
    if(access(DEFAULT_HTTPS_CERT, F_OK) != 0)
    {
        if(COperation::AssertDirExists("/data/configs/kds-7/secure/certificate_file_name") >= 0)
        {
            char szCmd[MAX_CMD_STR] = {0};
            sprintf(szCmd, "cp -f %s %s", HTTPS_CERT, DEFAULT_HTTPS_CERT);
            int ret = system(szCmd);
            if(ret < 0)
            {
                BC_ERROR_LOG("Init https cert file failed!");
            }
        }
    }
}

int main(int argc, char *argv[])
{
	GetConfFile(argc,argv);

    WebConfInit();
	ConfInfoParam p_webparam;
	bool getparamres = GetConfParamfromJson(webconfig.jsonfile.c_str(),&p_webparam);
	if(!getparamres)
	{
		printf("GetConfParamfromJson failed\n");
		return -1;
	}

    // 初始化p3k状态
    CWeb::P3kStatusInit();

    // 启动日志
    BCLog_Init(p_webparam.log_confpath);

    // mjpegstream
    pthread_t mjpegThread;
	pthread_attr_t  s_tThreadAttr;
	pthread_attr_init(&s_tThreadAttr);
	pthread_attr_setstacksize(&s_tThreadAttr, 1024*1024);
	int nret = pthread_create(&mjpegThread, &s_tThreadAttr, CWeb::MjpegStreamThread, NULL);
	if(nret < 0)
	{
		BC_INFO_LOG( "mjpeg pthread create error \n");
	}

	int err = 0;
#ifdef USE_WEBSOCKET
	if (!Civetweb_check_WEBSOCKET_feature()) {
		BC_ERROR_LOG(
		        "error: Embedded example built with websocket support, "
		        "but civetweb library build without.");
		err = 1;
	}
#endif
#ifndef TEST_WITHOUT_SSL
	if (!Civetweb_check_SSL_feature())
    {
        BC_ERROR_LOG(
        "error: Embedded example built with SSL support, "
        "but civetweb library build without.");
        err = 1;
    }
#endif
	if (err)
    {
        BC_ERROR_LOG(  "Cannot start CivetWeb - inconsistent build.");
        return EXIT_FAILURE;
    }

	bool StartRes = CWeb::Start(&p_webparam,webconfig.StartMode);
    if(!StartRes)
    {
    	BC_ERROR_LOG("Cannot start CivetWeb - mg_start failed.");
		return 0;
    }

	//CWeb::Print_ServerPort();

	// websocket demon
	CWeb::WebSocketRun(CWeb::P3kWebsocketHandle,0);

    pthread_t tP3kThread;
    pthread_create(&tP3kThread, &s_tThreadAttr, CWeb::P3kCommunicationThread, NULL);

	CWeb::HttpRun();

	//UpdateWebToConfigfile(webconfig.jsonfile.c_str(), "StartMode", "both");


	/* Wait until the server should be closed */
	while (!exitNow)
    {
        sleep(3);
    }

    pthread_join(tP3kThread, NULL);
    pthread_join(mjpegThread, NULL);
    CWeb::CloseP3kSocket();
	CWeb::Stop();

	return EXIT_SUCCESS;
}
