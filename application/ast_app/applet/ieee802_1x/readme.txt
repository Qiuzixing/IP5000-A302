移植：把IP5000_802X_MODULE/prefix/移植到开发板上，如移植到 / 目录下，
进入到开发板的/prefix/usr/local/bin下，根据需求，执行如下命令。
-u : 用户名
-p : 密码
-l : log4cpp的配置文件
-d : 802_1x的工作目录，/prefix/usr/local/bin/，该目录下有wpa_supplicant_802_1x，daemon，802_1X_TEST 程序，检查是否有这三个执行文件
-c : CA证书存放路径
-t : 客户端证书存放路径
-k : 客户端私钥存放路径
-m : 选择认证方式


运行802_1X_TEST

md5:
sudo ./802_1X_TEST -u steve -p testing -l /prefix/usr/local/bin/sample_log.conf -d /prefix/usr/local/bin/ -m md5

gtc:
sudo ./802_1X_TEST -u steve -p testing -l /prefix/usr/local/bin/sample_log.conf -d /prefix/usr/local/bin/ -m gtc

tls:
sudo ./802_1X_TEST -c /certs/ca.pem -t /certs/client.pem  -k /certs/client.key -l /prefix/usr/local/bin/sample_log.conf -d /prefix/usr/local/bin/ -m tls

ttls:
sudo ./802_1X_TEST  -u steve -p testing -t /certs/client.pem -l /prefix/usr/local/bin/sample_log.conf  -d /prefix/usr/local/bin/ -m ttls

peap_mschapv2:
sudo ./802_1X_TEST -u steve -p testing -t /certs/client.pem -l /prefix/usr/local/bin/sample_log.conf -d /prefix/usr/local/bin/ -m peap_mschapv2

peap_gtc:
sudo ./802_1X_TEST -u steve -p testing -t /certs/client.pem -l /prefix/usr/local/bin/sample_log.conf  -d /prefix/usr/local/bin/ -m peap_gtc


