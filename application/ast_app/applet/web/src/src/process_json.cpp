#include "process_json.h"
#include "debug.h"
/*
	{
	    "date":{
			"year":2020
			"mon":9
			"day":27
	    }
	    "student":"xxx"
	    "num":66666
	    "age":18
	}
*/
void GetValueFromJsonstream()
{
	string msg = "{ \"date\":{\"year\":2020,\"mon\":9,\"day\":27},\"student\":\"xxx\",\"num\":66666,\"age\":18}";
	Json::Reader reader;
  	Json::Value root;

  	if(reader.parse(msg,root))
  	{
  		Json::Value date = root["date"];
  		int date_y = date["year"].asInt();
  		int date_m = date["mon"].asInt();
  		int date_d = date["day"].asInt();

  		string name = root["student"].asString();
  		int age = root["age"].asInt();
  		int num = root["num"].asInt();
  		cout << "[" << date_y << ":" << date_m << ":" << date_d << "] : " << name << "-" << num << "-" << age << endl;
  		return;
  	}
  	cout << "parse faild\n";
}


//从字符串中读取JSON（内容复杂些）
void readStrProJson()
{
    string strValue = "{\"name\":\"json\",\"array\":[{\"cpp\":\"jsoncpp\"},{\"java\":\"jsoninjava\"},{\"php\":\"support\"}]}";
    /*
    //json内容如下：
    {
    "name": "json″,
    "array": [
        {
            "cpp": "jsoncpp"
        },
        {
            "java": "jsoninjava"
        },
        {
            "php": "support"
        }
    ]
    }
    */


    Json::Reader reader;
    Json::Value value;

    if (reader.parse(strValue, value))
    {
        string out = value["name"].asString();
        cout << out << endl;
        const Json::Value arrayObj = value["array"];
        for (unsigned int i = 0; i < arrayObj.size(); i++)
        {
            if (!arrayObj[i].isMember("cpp"))
                continue;
            out = arrayObj[i]["cpp"].asString();
            cout << out;
            if (i != (arrayObj.size() - 1))
                cout << endl;
        }
    }
}

/*
{
	"webconfig":{
		"ROOT":"./www",
		"PORT":"8888,9999s",
		"CERM":"./www/server.pem"
	},
	"LOG":"./www/log4cpp.conf"
}
*/
int ReadStrFromFile(const char* filename,WEBCONF * conf)
{
	// 解析json用Json::Reader
	cout << filename << endl;
	Json::Reader reader;
	// Json::Value是一种很重要的类型，可以代表任意类型。如int, string, object, array...
	Json::Value root;
	/*以输入的当方式打开is，is作为输入源了*/
	std::ifstream is;
	is.open (filename, std::ios::binary );
	if (!is.is_open())
	{
		cout << "open json file failed." << endl;
		return -1;
	}

	cout <<"111111111\n";
	/*把is里面的内容输出到root*/
	if (reader.parse(is, root))
	{
		cout <<"222222222\n";
		std::string code;
		if (root["webconfig"].isNull())//访问节点，按名称访问对象值,如果不存在，就返回一个空值.
		{
		  	cout <<  "can't find webconfig" << endl;
		  	return -1;
		}
		// 访问节点，如果key存在，则返回成员defaultValue
		// 否则
		Json::Value webconf = root["webconfig"];
		string rootpath = webconf.get("ROOT", "/www").asString();
		cout << rootpath << endl;

		string port = webconf.get("PORT", "80,443s").asString();
		cout << port << endl;

		string cermpath = webconf.get("CERM", "/www/server.pem").asString();
		cout << cermpath << endl;

		string logpath = root.get("LOG", "/www/log4cpp.conf").asString();
		cout << logpath << endl;

	}
	is.close();
	return 0;
}

/*
{
	"webconfig":{
		"DOCUMENTROOT":"./www",
		"WEBPORT":"8888,9999s",
		"CERM":"./www/server.pem"
	}
	"LOG":"./www/log4cpp.conf",
	"ldap":{
		"ldap_uri":"ldaps://192.168.17.225:636",
		"ldap_base_dn":"dc=my-domain,dc=com",
		"ldap_bind_dn":"cn=jeremy,ou=People,dc=my-domain,dc=com",
		"ldap_attr":"uid",
		"ldap_ca_pem":"./www/ca.pem"
	},
	"auth_mode":"2"
}
*/

bool GetConfParamfromJson(const char * i_jsonfile,ConfInfoParam * i_webparam)
{
	Json::Reader reader;
	Json::Value root;
	std::ifstream is;
	is.open (i_jsonfile, std::ios::binary );
	if (!is.is_open())
	{
		cout << "open json file failed." << endl;
		return false;
	}
	bool flag = false;
	if (reader.parse(is, root))
	{
		if (root["webconfig"].isNull())
		{
		  	cout <<  "can't find webconfig" << endl;
		  	return -1;
		}
		/*
			The startup mode of the web server will often change,
			so we generally specify the startup mode setting in the command line parameters,not in the json file
		*/
		#if 0
		Json::Value webconf = root["webconfig"];
		i_webparam->web_rootpath = webconf.get("DOCUMENTROOT", "/www").asString();
		i_webparam->web_httpport = webconf.get("HttpPort", "80").asString();
		i_webparam->web_httpsport = webconf.get("HttpsPort", "443").asString();
		i_webparam->web_cermpath = webconf.get("CERM", "/www/server.pem").asString();
		i_webparam->web_numthread = webconf.get("NumThreads", "20").asString();
		i_webparam->web_ws_pingpongenable = webconf.get("WsPingPongEnable", "no").asString();
		i_webparam->web_ws_pingpongtimems = webconf.get("WsPingPongTimeMs", "3600000").asString();

		i_webparam->log_confpath = root.get("LOG", "/www/log4cpp.conf").asString();

		Json::Value ldapconf = root["ldap"];
		i_webparam->ldap_URI = ldapconf.get("ldap_uri", "ldaps://192.168.17.225:636").asString();
		i_webparam->ldap_BaseDN = ldapconf.get("ldap_base_dn", "dc=my-domain,dc=com").asString();
		i_webparam->ldap_BindDN = ldapconf.get("ldap_bind_dn", "cn=jeremy,ou=People,dc=my-domain,dc=com").asString();
		i_webparam->ldap_Attr = ldapconf.get("ldap_attr", "uid").asString();
		i_webparam->ldap_CaPem = ldapconf.get("ldap_ca_pem", "/www/ca.pem").asString();

		i_webparam->auth_mode = root.get("auth_mode", "0").asString();

		#else
		//this->m_strDeviceName        = jsDeviceInfo["DeviceName"].asString();
		Json::Value webconf = root["webconfig"];
		i_webparam->web_rootpath = root["webconfig"]["DOCUMENTROOT"].asString();
		i_webparam->web_rootpath = webconf["DOCUMENTROOT"].asString();
		i_webparam->web_httpport = webconf["HttpPort"].asString();
		i_webparam->web_httpsport = webconf["HttpsPort"].asString();
		//"StartMode":"https_olny",
		i_webparam->web_startmode = webconf["StartMode"].asString();
		i_webparam->web_cermpath = webconf["CERM"].asString();
		i_webparam->web_numthread = webconf["NumThreads"].asString();
		i_webparam->web_ws_pingpongenable = webconf["WsPingPongEnable"].asString();
		i_webparam->web_ws_pingpongtimems = webconf["WsPingPongTimeMs"].asString();

		i_webparam->log_confpath = root["LOG"].asString();

		Json::Value ldapconf = root["ldap"];
		i_webparam->ldap_URI = ldapconf["ldap_uri"].asString();
		i_webparam->ldap_BaseDN = ldapconf["ldap_base_dn"].asString();
		i_webparam->ldap_BindDN = ldapconf["ldap_bind_dn"].asString();
		i_webparam->ldap_Attr = ldapconf["ldap_attr"].asString();
		i_webparam->ldap_CaPem = ldapconf["ldap_ca_pem"].asString();

		i_webparam->auth_mode = root["auth_mode"].asString();

		#endif
		flag = true;
#if 1
		{
			cout << "root:" << root << endl;
			cout << "webconf:" << webconf << endl;
			cout << "i_webparam->web_rootpath:" << i_webparam->web_rootpath << endl;
			cout << "i_webparam->web_httpport:" << i_webparam->web_httpport << endl;
			cout << "i_webparam->web_httpsport:" << i_webparam->web_httpsport << endl;
			cout << "i_webparam->web_startmode:" << i_webparam->web_startmode << endl;
			cout << "i_webparam->web_cermpath:" << i_webparam->web_cermpath << endl;
			cout << "i_webparam->web_numthread:" << i_webparam->web_numthread << endl;
			cout << "i_webparam->web_ws_pingpongenable:" << i_webparam->web_ws_pingpongenable << endl;
			cout << "i_webparam->web_ws_pingpongtimems:" << i_webparam->web_ws_pingpongtimems << endl;
			cout << "i_webparam->log_confpath:" << i_webparam->log_confpath << endl;
			cout << "i_webparam->auth_mode:" << i_webparam->auth_mode << endl;
		}
#endif
	}


	is.close();
	if(flag == false)
		cout << "open json file sucess,but can't parse this json file!!!" << endl;
	return flag;
}


bool UpdateWebToConfigfile(const char * i_jsonfile, const char *i_option, string i_startmode)
{
	Json::Reader reader;
	Json::Value root;
	std::ifstream in;
	in.open (i_jsonfile, std::ios::binary );
	if (!in.is_open())
	{
		cout << "open json file failed." << endl;
		return false;
	}
	bool flag = false;
	if (reader.parse(in, root))
	{
		if (root["webconfig"].isNull())
		{
		  	cout <<  "can't find webconfig" << endl;
		  	return false;
		}
		root["webconfig"][i_option] = i_startmode.c_str();

	}
	in.close();

#if 1
	std::ofstream ou;
	ou.open (i_jsonfile, std::ios::trunc );
	if (!ou.is_open())
	{
		cout << "open json file failed." << endl;
		return false;
	}
	ou << root << endl;
	//cout << "root:" << root << endl;
#endif

	return true;
}