#ifndef CP3KCMD_H
#define CP3KCMD_H
#include <QString>
#include <QObject>
#include <QList>
#include "ctcpsocket.h"
typedef struct _CmdReqInfo_S
{
    QString cmd;
    QString cmdstr;
}CmdReqInfo_S;
class Cp3kcmd :public QObject
{
    Q_OBJECT
    public:
        Cp3kcmd(QObject*parent=0);
        ~Cp3kcmd();
       //连接IP
        int ClinkToS(QString ip ,int port);
        //命令发送字符串同步

        int SendCmdSync(QString cmd,QString respInfo);
        //命令发送异步
        int SendCmdNonsync(QString cmd);
        void AddcmdInfo(QString cmd,QString cmdstr);
    signals:
        //命令消息接收异步
       void signal_ReadMsg(QString);
        //发送消息统计
       void signal_SendMsgNumer(int);
        //接收消息统计
       void signal_RecvMsgNumer(int);
    private:
        QList<CmdReqInfo_S> s_cmdList;
        SelfTcpSocket *s_socketHandle;
        int s_sendMsgNum;
        int s_recvMsgNum;
    private slots:
        void on_UserReadMesg(char* ,int);
        void on_UserConnected();
        void on_UserConnectErr();
        void on_UserDisconnected();

};

#endif // CP3KCMD_H
