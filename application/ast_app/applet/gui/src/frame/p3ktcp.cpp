#include "p3ktcp.h"
#include "common/global.h"

#include <QProcess>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//P3ktcp::P3ktcp(QObject *parent)
//{
//    m_TcpConn = new QTcpSocket();

//    // 端口为5000
//    m_Port = 5000;
//    // 本地环回IP
//    m_IP = QHostAddress::LocalHost;

//    connect(m_TcpConn, SIGNAL(readyRead()), this, SLOT(onReadPendingDatagrams()));
//}

//P3ktcp::~P3ktcp()
//{
//    if(m_TcpConn != nullptr)
//    {
//        m_TcpConn->close();
//        m_TcpConn->deleteLater();
//        m_TcpConn = 0;
//    }
//}

//QString P3ktcp::getIPAdress()
//{
//    QString strResult;
//    QString strCmd = "e lmparam g MY_IP";
//    QProcess *p = new QProcess();
//    p->start("bash", QStringList() <<"-c" << strCmd);
//    if(p->waitForFinished())
//    {
//        strResult = p->readAllStandardOutput();
//        qDebug() << "getKMControl finished";
//        p->deleteLater();
//    }
//    if(!strResult.isEmpty())
//    {
//        return strResult;
//    }
//}

//void P3ktcp::p3kConnect(QHostAddress ip,int port)
//{
//    if(m_TcpConn == nullptr)
//        return;

//    qDebug() << "IP:" << ip;
//    m_TcpConn->connectToHost(ip,port);
//    if(m_TcpConn->waitForConnected(3000))
//    {
//        qDebug() << "P3K Connected";

//        // 发送登录信息
//        QString strCmd = "";
//        QByteArray byteCmd = strCmd.toLatin1();
//        sendCmdToP3k(byteCmd);

//        // 等待返回信息
//        m_TcpConn->waitForReadyRead();
//    }
//}


//bool P3ktcp::IsConnected()
//{
//    if(m_TcpConn != nullptr && m_TcpConn->state() == QAbstractSocket::ConnectedState)
//        return true;
//    else
//        return false;
//}

//void P3ktcp::closeP3kConnect()
//{
//    if(m_TcpConn != nullptr)
//    m_TcpConn->close();
//}

//bool P3ktcp::sendCmdToP3k(const char *lpBuf)
//{
//    if(lpBuf != nullptr)

//    if(IsConnected() && m_TcpConn->isValid())
//        m_TcpConn->write(lpBuf,strlen(lpBuf));

//}

//void P3ktcp::onReadPendingDatagrams()
//{
//    QByteArray dategrams;
//    dategrams.resize(m_TcpConn->readAll().size());
//    dategrams = m_TcpConn->readAll();
//}

//void P3ktcp::parseCmdResult(QByteArray datagram)
//{
//    if(datagram.isEmpty())
//        return;

//    qDebug()<<"datagram:" << datagram;
//    // '#'号开头为接收的命令，'~nn@'开头为返回数据

//    if(datagram.startsWith("#"))
//    {
//        datagram = datagram.mid(1);
//    }
//    else if(datagram.startsWith("~nn@"))
//    {
//        datagram = datagram.mid(4);
//    }

//    QStringList argList = QString(datagram).split(" ");

//    if(argList.at(0) == "KDS_OSD_DISPLAY")
//    {
//        //发送信号到主界面，根据mode参数设置OSD显示状态
//        emit setOsdDisplay(argList.at(1).toInt());
//    }
//    else if(argList.at(0) == "KDS_OSD_DISPLAY?")
//    {
//        // 返回命令？获取OSD显示状态，发送给P3K

//    }
//    else if(argList.at(0) == "KDS_START_DISPLAY")
//    {
//        // 开始overlay
//        emit startOverlay(argList.at(1),argList.at(2).toInt());
//    }
//    else if(argList.at(0) == "KDS_STOP_OVERLAY")
//    {
//        // 停止overlay
//        emit stopOverlay();
//    }
//}

#define UDP_PORT 5588

UdpRecvThread *UdpRecvThread::UdpRecv=NULL;

int UdpRecvThread::UdpRecv_fd = -1;
UdpRecvThread::UdpRecvThread(QObject *parent)
    : QThread(parent)
{

}

void UdpRecvThread::run()
{
    struct sockaddr_in server;
    struct sockaddr_in client;
    socklen_t addrlen;
    char buf[1024];

    int UdpRecv_fd = socket(AF_INET,SOCK_DGRAM,0);
    if(UdpRecv_fd == -1)
    {
        qDebug() << "CREAT UDPSOCKET FAILED";
        return;
    }

    printf("UDP SOCKET: %d\n",UdpRecv_fd);
    qDebug() << "UDP SOCKET:" << UdpRecv_fd;

    //
    memset(&server,0,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(UDP_PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");//htonl(INADDR_ANY);

    // 端口重用
    int opt = 1;
    setsockopt(UdpRecv_fd, SOL_SOCKET,SO_REUSEADDR, (const void *)&opt, sizeof(opt) );


    int ret = bind(UdpRecv_fd,(struct sockaddr *)&server,sizeof (server));
    if(ret == -1)
    {
        qDebug() << "UDPSOCKET BIND FAILED";
        return;
    }

    int retval;
    fd_set readfds;
    struct timeval tv;
    qDebug() << "LISENING.......";
    while(1)
    {
        // 设置等待时间
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(UdpRecv_fd, &readfds);

        retval = select(UdpRecv_fd+1, &readfds, NULL, NULL, &tv);
        if(FD_ISSET(UdpRecv_fd,&readfds)) {
            addrlen = sizeof (client);
            int recvNum = recvfrom(UdpRecv_fd,buf,sizeof (buf),0,(sockaddr *)&client,&addrlen);
            if(recvNum == -1)
            {
                printf("recv failed\n");
                return;
            }
            printf("recv is %s\n",buf);
            QByteArray data(buf);
            if(data.startsWith("GET"))
            {
                char replybuf[16] = {0};
                if(g_bOSDMeunDisplay)
                    strcpy(replybuf,"1");
                else
                    strcpy(replybuf,"0");

                int ret = sendto(UdpRecv_fd,replybuf,sizeof(replybuf),0,(struct sockaddr*)&client,sizeof(client));
                if(recvNum > 0)
                {
                    printf("send finished: %d\n",ret);
                }
            }
            emit RecvData(data);
            memset(&buf,0,sizeof (buf));
        }
    }
}

