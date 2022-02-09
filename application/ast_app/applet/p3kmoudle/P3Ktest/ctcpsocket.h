#ifndef CTCPSOCKET_H
#define CTCPSOCKET_H

#include <QTcpSocket>
#include <QAbstractSocket>
#include <QTimer>

class SelfTcpSocket : public QObject
{
    Q_OBJECT
public:
    SelfTcpSocket(QObject*parent=0);
    ~SelfTcpSocket();

   void startsocket(char*ip,int port);
   void stopsocket();

   int sendmsg(char*data,int len);
   int sendmsgSync(char*data,int len,char*readata);
signals:

   void signal_Connected();
   void signal_ConnectedErr();
   void signal_ReadMsg(char*,int);
   void signal_Disconnect();
private:
   QTcpSocket*s_tcpsocket;
   QTimer *s_time;
   bool s_stopStat;
   bool s_connectStat;
   int s_port;
   int s_syncFlag;
   char s_ip[32];
private slots:
   void on_Connected();
   void on_connectedErr(QAbstractSocket::SocketError);
   void on_disconnected();
   void on_ReadMsg();
   void on_connectServer();

};



#endif // CTCPSOCKET_H
