#include "cp3kcmd.h"
#include "QMessageBox"
Cp3kcmd::Cp3kcmd(QObject*parent)
{
    s_sendMsgNum = 0;
    s_recvMsgNum = 0;
    s_cmdList.clear();
    s_socketHandle = new SelfTcpSocket();
    connect(s_socketHandle,SIGNAL(signal_Connected()),this,SLOT(on_UserConnected()));
    connect(s_socketHandle,SIGNAL(signal_ConnectedErr()),this,SLOT(on_UserConnectErr()));
    connect(s_socketHandle,SIGNAL(signal_Disconnect()),this,SLOT(on_UserDisconnected()));
    connect(s_socketHandle,SIGNAL(signal_ReadMsg(char*,int)),this,SLOT(on_UserReadMesg(char*,int)));
}

Cp3kcmd::~Cp3kcmd()
{
    s_socketHandle->stopsocket();
    delete s_socketHandle;

}
int Cp3kcmd::ClinkToS(QString ip ,int port)
{
    s_socketHandle->startsocket(ip.toLatin1().data(),port);
    return 0;
}
int Cp3kcmd::SendCmdNonsync(QString cmd)
{
     QList<CmdReqInfo_S>::iterator it;
     int ret = 0;
    for(it=s_cmdList.begin();it != s_cmdList.end();it++)
    {
            if(0 == QString::compare(cmd,it->cmd))
            {
                if(s_socketHandle)
                {
                       qDebug() << it->cmdstr;
                     ret = s_socketHandle->sendmsg(it->cmdstr.toLatin1().data(),it->cmdstr.length());
                    if(ret > 0)
                    {

                        s_sendMsgNum += 1;
                        emit signal_SendMsgNumer(s_sendMsgNum);
                    }
                }
            }
    }
    return 0;
}
int Cp3kcmd::SendCmdSync(QString cmd,QString respInfo)
{
    int ret = 0;
    QByteArray msg;
    QList<CmdReqInfo_S>::iterator it;
   for(it=s_cmdList.begin();it != s_cmdList.end();it++)
   {
           if(0 == QString::compare(cmd,it->cmd))
           {
               if(s_socketHandle)
               {
                 ret =  s_socketHandle->sendmsgSync(it->cmdstr.toLatin1().data(),it->cmdstr.length(),msg.data());
                 respInfo = QString(msg);
                 if(ret <= 0)
                {
                    return -1;
                }
                   s_sendMsgNum += 1;
                   s_recvMsgNum +=1;
               }
           }
   }


    return 0;
}
void Cp3kcmd::AddcmdInfo(QString cmd,QString cmdstr)
{
        CmdReqInfo_S tmp ;
        tmp.cmd = cmd;
        tmp.cmdstr = cmdstr;

        s_cmdList.append(tmp);
}
void Cp3kcmd::on_UserReadMesg(char* data ,int len)
{
    QString Msg =QString(data);
    if(Msg[0]=='~'&&Msg[3]=='@')
    {
        s_recvMsgNum += 1;
    }
    emit signal_ReadMsg(Msg);

    emit signal_RecvMsgNumer(s_recvMsgNum);
}

void Cp3kcmd::on_UserConnected()
{
      qDebug() << "on_UserConnected";
      char buf[32];
      memset(buf,0,sizeof(buf));
    s_socketHandle->sendmsgSync("#\r",2,buf);
    if(strcmp(buf,"~nn@\r\n")==0)
    {
         QMessageBox::about(NULL, "提示", "以连接，P3K握手成功！");

    }
    else
    {
         QMessageBox::about(NULL, "提示", "以连接，P3K握手失败！");
    }

     //s_socketHandle->sendmsg("#\r",2);
}
void Cp3kcmd::on_UserConnectErr()
{
    //提示失败
    qDebug() << "on_UserConnected";
    QMessageBox::about(NULL, "提示", "连接失败！");
}
void Cp3kcmd::on_UserDisconnected()
{
    //提示断开连接
}
