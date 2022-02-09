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


P3ktcp *P3ktcp::m_p3kMod=NULL;

P3ktcp::P3ktcp(QObject *parent)
    : QObject(parent)
{
    m_TcpConn = new QTcpSocket();

    // 端口为6001
    m_Port = 6001;

    // 本地环回IP
    m_IP = QHostAddress::LocalHost;

    connect(m_TcpConn, SIGNAL(readyRead()), this, SLOT(onReadPendingDatagrams()));
}

P3ktcp::~P3ktcp()
{
    if(m_TcpConn != NULL)
    {
        m_TcpConn->close();
        m_TcpConn->deleteLater();
        m_TcpConn = 0;
    }
}

bool P3ktcp::p3kConnect()
{
    if(m_TcpConn == NULL)
        return false;

    qDebug() << "IP:" << m_IP;
    qDebug() << "m_Port:" << m_Port;

    m_TcpConn->connectToHost(m_IP,m_Port);
    if(m_TcpConn->waitForConnected(3000))
    {
        qDebug() << "P3K Connected";

        // 发送登录信息
        QString strCmd = "#LOGIN admin,admin\r";
        QByteArray byteCmd = strCmd.toLatin1();
        sendCmdToP3k(byteCmd);
        return true;
    }
    else
    {
        return false;
        qDebug("P3K Connect failed");
    }
}


bool P3ktcp::IsConnected()
{
    if(m_TcpConn != NULL && m_TcpConn->state() == QAbstractSocket::ConnectedState)
        return true;
    else
        return false;
}

void P3ktcp::closeP3kConnect()
{
    if(m_TcpConn != NULL)
    m_TcpConn->close();
}

bool P3ktcp::sendCmdToP3k(const char *lpBuf)
{
    qDebug() << "SendCmd："<< lpBuf;
    if(lpBuf == NULL)
        return false;

    if(IsConnected() && m_TcpConn->isValid())
    {
        int ret = m_TcpConn->write(lpBuf,strlen(lpBuf));
        if(ret == -1)
        {
            qDebug("Send P3kcmd failed-write;");
            return false;
        }
        return true;
    }
    qDebug("Send P3kcmd failed-connect;");
    return false;
}

void P3ktcp::onReadPendingDatagrams()
{
    qDebug("-----P3ktcpRCV_MSG-----");
    QByteArray datagrams;
    datagrams = m_TcpConn->readAll();

    emit tcpRcvMsg(datagrams);
}

#define UDP_PORT 6003

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

    qDebug() << "UDP SOCKET:" << UdpRecv_fd;

    // 初始化地址端口
    memset(&server,0,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(UDP_PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");//htonl(INADDR_ANY);

    // 端口重用
    int opt = 1;
    setsockopt(UdpRecv_fd, SOL_SOCKET,SO_REUSEADDR, (const void *)&opt, sizeof(opt) );

    // 绑定
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
            if(data.contains("GET_OSD_DISPALY"))
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
            else if(data.contains("GET_CHANNEL_ID"))
            {
                char replybuf[16] = {0};
                QString str = QString("%1").arg(g_nChannelId);
                QByteArray strResult = str.toLatin1();
                strcpy(replybuf,strResult);
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

