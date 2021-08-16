#ifndef P3KTCP_H
#define P3KTCP_H

#include <QTcpSocket>
#include <QByteArray>
#include <QQueue>
#include <QHostAddress>
#include <QUdpSocket>
#include <QThread>

class P3ktcp : public QObject
{
    Q_OBJECT
public:
    static P3ktcp *getInstance()
    {
        if(m_p3kMod==NULL){
             m_p3kMod = new P3ktcp();
        }

        return m_p3kMod;
    }
    ~P3ktcp();
    // 发送指令到P3K
    bool sendCmdToP3k(const char *lpBuf);

signals:
    void tcpRcvMsg(QByteArray msg);

public slots:
    // 接收命令返回
    void onReadPendingDatagrams();

protected:
    // 连接or关闭
    void p3kConnect(QHostAddress ip,int port);
    void closeP3kConnect();

    // 检测连接
    bool IsConnected();
    void parseCmdResult(QByteArray datagram);

    QString getIPAdress();
private:
    P3ktcp(QObject *parent = 0);
    static P3ktcp *m_p3kMod;

    QTcpSocket *m_TcpConn;
    QHostAddress m_IP;
    int m_Port;

    QQueue<QByteArray> *m_CmdQueue;
};



class UdpRecvThread : public QThread
{
    Q_OBJECT
public:
    static UdpRecvThread *getInstance()
    {
        if(UdpRecv==NULL){
             UdpRecv = new UdpRecvThread;
        }

        return UdpRecv;
    }

signals:
    void RecvData(QByteArray data);

protected:
    void run();

private:
    UdpRecvThread(QObject *parent = 0);

    static UdpRecvThread *UdpRecv;
    static int UdpRecv_fd;
};


#endif // P3KRCP_H
