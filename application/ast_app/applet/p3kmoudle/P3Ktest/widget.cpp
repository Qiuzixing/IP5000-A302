#include "widget.h"
#include "ui_widget.h"
#include <QFile>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QMessageBox>
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
   // ui->tableWidget->setStyleSheet("");
    this->setWindowTitle(QString("P3K-CMD"));
    s_cmdMod = new Cp3kcmd();
    connect(s_cmdMod,SIGNAL(signal_ReadMsg(QString)),this,SLOT(on_ReadMessage(QString)));
    connect(s_cmdMod,SIGNAL(signal_SendMsgNumer(int)),this,SLOT(on_GetSendMsgNum(int)));
    connect(s_cmdMod,SIGNAL(signal_RecvMsgNumer(int)),this,SLOT(on_GetRecvMsgNum(int)));

    #if 1
    QString filename = QObject::tr("cmd.json");

     //qDebug() << filename;
    QFile file(filename);

   if( false == file.open(QIODevice::ReadOnly| QIODevice::Text))
   {
         qDebug() << filename;
   }
    QByteArray data = file.readAll();
    file.close();
    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(data, &parseJsonErr);
    if (!(parseJsonErr.error == QJsonParseError::NoError)) {
        QMessageBox::about(NULL, "提示", "cmd.json配置文件错误！");
    }
    else
    {
        QJsonValue arrayValue = document.object().value(QStringLiteral("cmd"));

         QJsonArray array = arrayValue.toArray();
        for(int i = 0;i<array.size();i++)
        {


            QJsonObject key = array.at(i).toObject();
              //添加到cmd
            QString cmd =key["name"].toString();
            QString cmdStr= key["str"].toString();

            AddCmdToTable(cmd);
            s_cmdMod->AddcmdInfo(cmd,cmdStr);
        }
        //显示

    }
    #else
    QString str = QString("KDE_AUD");
    QString str2 = QString("KDE_AUD?");
    QString str1= QString("#KDS-AUD 1\r");
    QString str3 = QString("#KDS-AUD?\r");
    AddCmdToTable(str);
    AddCmdToTable(str2);
    s_cmdMod->AddcmdInfo(str,str1);
    s_cmdMod->AddcmdInfo(str2,str3);
    #endif
}

Widget::~Widget()
{
    delete ui;
    delete s_cmdMod;
}

void Widget::AddCmdToTable(QString cmd)
{
  int count =  ui->tableWidget->rowCount();
  ui->tableWidget->insertRow(count);
  ui->tableWidget->setItem(count,0,new QTableWidgetItem(cmd));
  ui->tableWidget->item(count,0)->setTextAlignment(Qt::AlignCenter);

}
void Widget::on_ReadMessage(QString msg)
{
    ui->textBrowser->append(msg);

}
void Widget::on_GetSendMsgNum(int number)
{
    QString str = QString("%1").arg(number);
    ui->lineEdit_send->setText(str);
}
void Widget::on_GetRecvMsgNum(int number)
{
     QString str = QString("%1").arg(number);
     ui->lineEdit_recv->setText(str);
}
void Widget::on_button_link_clicked()
{
    //执行连接
   QString ipAddr =  ui->lineEdit_ip->text();
   if(ipAddr.isEmpty() == true)
   {
        return ;
   }
   qDebug() << ipAddr;
    s_cmdMod->ClinkToS(ipAddr,5000);
    return ;
}

void Widget::on_button_all_clicked()
{
    //执行所有命令
     int count =  ui->tableWidget->rowCount();
     //qDebug() << count;
     QString cmd ;
     int i = 0;
     for(i= 0;i<count;i++)
     {

            cmd = ui->tableWidget->item(i,0)->text();
            // qDebug() << cmd;
            if(s_cmdMod)
            {

                s_cmdMod->SendCmdNonsync(cmd);
                QEventLoop loop;
                QTimer::singleShot(20, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
                loop.exec();
            }
     }
}

void Widget::on_tableWidget_cellDoubleClicked(int row, int column)
{
       //qDebug() << row << column;
      QString cmd ;
      if(s_cmdMod)
      {
           cmd = ui->tableWidget->item(row,0)->text();
          s_cmdMod->SendCmdNonsync(cmd);
      }
}

void Widget::on_textBrowser_textChanged()
{
    ui->textBrowser->moveCursor(QTextCursor::End);
}

void Widget::on_button_clear_clicked()
{
      ui->textBrowser->clear();
}
