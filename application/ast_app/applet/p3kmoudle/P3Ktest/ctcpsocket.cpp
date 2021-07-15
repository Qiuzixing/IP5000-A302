#include "ctcpsocket.h"

SelfTcpSocket::SelfTcpSocket(QObject*parent):
   QObject(parent)
{
    s_tcpsocket=NULL;
    s_stopStat=true;
    s_connectStat=false;
    s_syncFlag =false;
    int type1=qRegisterMetaType<QAbstractSocket::SocketError>("SocketError");
    s_port=0;
    memset(s_ip,0,sizeof(s_ip));
}
SelfTcpSocket::~SelfTcpSocket()
{
    s_stopStat=true;
    if(s_tcpsocket != NULL)
    {
        s_tcpsocket->close();
        s_time->stop();
       delete  s_tcpsocket;
       delete  s_time;
       s_tcpsocket = NULL;
    }
   // delete s_tcpsocket;
}
void SelfTcpSocket::startsocket(char*ip,int port)
{
    s_stopStat=false;
    if(ip)
    memcpy(s_ip,ip,sizeof(s_ip));
    s_port=port;
  //  memcpy(s_ip,ip,sizeof(s_ip));
   #if 1
    if(s_tcpsocket != NULL)
    {
        s_tcpsocket->close();
        s_time->stop();
       delete  s_tcpsocket;
       delete  s_time;
       s_tcpsocket = NULL;
    }
    if (!s_tcpsocket)
    {
       int type1=qRegisterMetaType<QAbstractSocket::SocketError>("Socket Error");
        s_tcpsocket = new QTcpSocket(this);
        s_time =new QTimer(this);
       //  qDebug() << "11111111111111112222";
       // s_tcpsocket->setSocketOption(QAbstractSocket::KeepAliveOption,1);
        s_tcpsocket->setSocketOption(QAbstractSocket::LowDelayOption,1);
        s_tcpsocket->connectToHost(s_ip, s_port);
       // s_connectStat = s_tcpsocket->waitForConnected(500);
        connect(s_tcpsocket, SIGNAL(readyRead()), this, SLOT(on_ReadMsg()));
        connect(s_tcpsocket, SIGNAL(connected()), this, SLOT(on_Connected()));
        connect(s_tcpsocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(on_connectedErr(QAbstractSocket::SocketError)));
        connect(s_tcpsocket, SIGNAL(disconnected()), this, SLOT(on_disconnected()));

      //  qDebug() << "11111111111111111111";

        connect(s_time, SIGNAL(timeout()), this, SLOT(on_connectServer()));
        s_time->setInterval(3000);
        s_time->start();
      //  qDebug() << "11111111333333331111111";
       // moveToThread(this);
    }
 #endif

  //  this->start();
}
void SelfTcpSocket::stopsocket()
{
    s_stopStat=true;
    if(s_tcpsocket != NULL)
    {
        s_tcpsocket->close();
        s_time->stop();
       delete  s_tcpsocket;
       delete  s_time;
       s_tcpsocket = NULL;
    }
}

int SelfTcpSocket::sendmsg(char*data,int len)
{
    int ret=0;
   // qDebug() << "1111111111";
   if(s_connectStat==true)
    {
        qDebug() << data;
        if(data)
        {
            ret=s_tcpsocket->write(data,len);
            s_tcpsocket->flush();
        }
    }
    return ret;
}
int SelfTcpSocket::sendmsgSync(char*data,int len,char*readata)
{
    int ret = 0;
    int i =0;
    if(s_connectStat != true)
    {
       return -1;
    }
    s_syncFlag = true;
    this->sendmsg(data,len);
    for(i = 0;i<10;i++)
    {
        s_tcpsocket->waitForReadyRead(500);
        QByteArray msgdata=s_tcpsocket->readAll();

        memcpy(readata,msgdata.data(),msgdata.length());
        ret = msgdata.length();
        break;

    }
    s_syncFlag =false;
    return ret;
}
void SelfTcpSocket::on_Connected()
{

     qDebug() << "on_Connected";
     s_connectStat=true;
   // connect(s_tcpsocket,SIGNAL(disconnected()),this,SLOT(on_disconnected()));
    emit signal_Connected();
}
void SelfTcpSocket::on_connectedErr(QAbstractSocket::SocketError code)
{
     qDebug() << "on_connectedErr";
    emit signal_ConnectedErr();
}
void SelfTcpSocket::on_disconnected()
{
    s_connectStat=false;
     qDebug() << "on_disconnected";
    emit signal_Disconnect();
}
void SelfTcpSocket::on_ReadMsg()
{
     if(s_syncFlag == false)
     {
        QByteArray msgdata=s_tcpsocket->readAll();
        emit signal_ReadMsg(msgdata.data(),msgdata.length());
     }
}
void SelfTcpSocket::on_connectServer()
{
    int type1=qRegisterMetaType<QAbstractSocket::SocketError>("SocketError");
   // qDebug() << "timer ";
    if (!s_connectStat)
    {
       qDebug() << "s_tcpsocket reconnect";
       s_tcpsocket->abort();
       s_tcpsocket->connectToHost(s_ip, s_port);
      //  s_connectStat = s_tcpsocket->waitForConnected(3000);
    }
}

