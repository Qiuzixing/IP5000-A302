#include "mainwidget.h"
#include "videolistwidget.h"
//#include "osdlabel.h"
#include "common/global.h"
#include "common/scale_global.h"
#include "p3ktcp.h"

#include "json/json.h"
#include <string>
#include <iostream>
#include <fstream>

#include <QLabel>
#include <QProcess>
#include <QDebug>
#include <QRect>
#include <QApplication>
#include <QDesktopWidget>
#include <QSet>
#include <QList>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QImageReader>

#include <stdlib.h>

#include <QProcess>

#define RESOLUTION_CONFIG   "/sys/devices/platform/videoip/timing_info"

using namespace std;

QMap<int, OSDLabel *> MainWidget::g_overlayMap;
QSet<int> MainWidget::osdIdSet;

MainWidget::MainWidget(QWidget *parent) : QWidget(parent)
    ,m_imageOverlay(NULL)
    ,m_textOverlay(NULL)
    ,m_CmdOuttime(-1)
{
    // 鼠标跟踪
    setMouseTracking(true);
    // 背景透明
    setAttribute(Qt::WA_TranslucentBackground);

    qDebug() << "ScreenWidth:" << g_nScreenWidth << "ScreenHeight:" << 	g_nScreenHeight;

    // 设置尺寸、无边框
    setFixedSize(g_nScreenWidth,g_nScreenHeight);
    setWindowFlags(Qt::FramelessWindowHint);

    qDebug() << "main_0";

    // 添加文件监控
    QStringList paths ;
    paths << RESOLUTION_CONFIG;
    watch = new QFileSystemWatcher;
    watch->addPaths(paths);
    //qDebug() << "watch list:" << watch->addPaths(paths);
    connect(watch, SIGNAL(fileChanged(QString)), this, SLOT(syncConfig(QString)));

    qDebug() << "main_3:" << g_bDeviceSleepMode;

     initOsdMeun();

    // 初始化页面切换
    initPanelStack();

    // 正常工作状态
    if(!g_bDeviceSleepMode)
    {
        //switchOSDMeun();
        m_panelStack->close();
        m_sleepPanel->close();
        connect(&getInfoTimer,SIGNAL(timeout()),this,SLOT(isNoSignal()));
        this->getInfoTimer.start(1000);
    }
    else
    {
         qDebug() << "main_0_2";
         startSleepMode(m_sleepPanel);
    }

    qDebug() << "main_4";

    bRinfoDlgHidden = false;
    // 初始化sInfo结构体
    strcpy((char*)sInfo.FW, "Unknown");
    strcpy((char*)sInfo.IP, "Unknown");
    strcpy((char*)sInfo.RemoteIP, "Unknown");
    strcpy((char*)sInfo.ID, "Unknown");
    // 设备连接
    StartMsgDConnection();

    // 创建显示信息
    initInfoLandIndoR();

    // 处理p3k命令
    UdpRecv = UdpRecvThread::getInstance();
    UdpRecv->start();

    connect(UdpRecv,SIGNAL(RecvData(QByteArray)),this,SLOT(onRecvData(QByteArray)));

    m_p3kTcp = P3ktcp::getInstance();
    connect(m_p3kTcp,SIGNAL(tcpRcvMsg(QByteArray)),this,SLOT(parseCmdResult(QByteArray)));
}

MainWidget::~MainWidget()
{
    m_tcpSocket->close();
    m_tcpSocket = 0;
    m_tcpSocket->deleteLater();
}

void MainWidget::initInfoLandIndoR()
{
    QString str = QString("Initializing transmitter search...");
    quint8 size = 10;
    quint32 color = 0xffffff;
    infoL = new OSDLabel(str,size,color,this);

    infoR = new OSDLabel(str,size,color,this);

    if (bRinfoDlgHidden == true)
    {
        infoR->setText(QString("FW: %1\nLocal IP: %2\nRemote IP: %3\n")
                .arg((char*)sInfo.FW)
                .arg((char*)sInfo.IP)
                .arg((char*)sInfo.RemoteIP));
    }
    else
    {
        infoR->setText(QString("FW: %1\nLocal IP: %2\nRemote IP: %3\nID: %4")
                .arg((char*)sInfo.FW)
                .arg((char*)sInfo.IP)
                .arg((char*)sInfo.RemoteIP)
                .arg((char*)sInfo.ID));
    }

    infoL->setVisible(true);
    infoR->setVisible(true);

    infoL->adjustSize();
    infoL->show();

    int xpos = OSD_XPOS;
    int ypos = g_nframebufferHeight-infoL->height() - OSD_YPOS;
    infoL->move(xpos,ypos);

    infoR->adjustSize();
    infoR->show();

    xpos = g_nframebufferWidth-infoR->width() - OSD_XPOS;
    ypos = g_nframebufferHeight-infoR->height() - OSD_YPOS;
    infoR->move(xpos,ypos);

    moveFramebuffer(CENTER);
}


void MainWidget::parseCmdResult(QByteArray datagram)
{
    if(datagram.isEmpty())
    {
        qDebug()<<"Datagram Is Null";
        return;
    }

    qDebug()<<"datagram:" << datagram;
    // '#'号开头为接收的命令，'~nn@'开头为返回数据

    if(datagram.startsWith("~01@"))
    {
        datagram = datagram.mid(4);
    }

//    QStringList argList = QString(datagram).split(" ").at(1).split(",");

//    if(datagram.contains("VIEW-MOD") && argList.size() == 3)
//    {
//        if(argList.at(0).toInt() == 15)
//        {
//            m_nVideoWall_H = argList.at(1).toInt();
//            m_nVideoWall_V = argList.at(2).toInt();
//        }
//    }
//    else if(datagram.contains("VIDEO-WALL-SETUP") && argList.size() == 2)
//    {
//        m_nVideoWall_ID = argList.at(0).toInt();
//        m_nVideoWall_R = argList.at(1).toInt();
//    }
}

void MainWidget::onRecvData(QByteArray data)
{
    QString datagram(data);
    qDebug() << "Recv :" << datagram;
    if(datagram.contains("SET"))
    {
        // 设置OSD显示状态，参数为mode
        qDebug() << "SHOW OSD :";
        QStringList argList = datagram.split(" ");

        if(argList.size() < 2)
        {
            qDebug() << "too few arg";
            return;
        }

        bool bshow;
        int mode = argList.at(1).toInt();
        if(mode == 0)
            bshow = false;
        else
            bshow = true;

         setOsdDispaly(bshow);
    }
    else if(datagram.contains("START"))
    {
        // 开始显示overlay
        qDebug() << "SHOW OVERLAY :";
        QStringList argList = datagram.split(" ");

        if(argList.size() < 2)
        {
            qDebug() << "too few arg";
            return;
        }

        QString filename = argList.at(1);

        // 测试用之后可删
        if(filename.compare("overlay") == 0)
        {
            filename = "overlay2_setting.json";
        }

        if(!argList.at(2).isEmpty())
        {
            int timeout = argList.at(2).toInt();
            m_CmdOuttime = timeout;
        }
        // 解析overlay
        parseOverlayJson(filename);
    }
    else if(datagram.contains("STOP"))
    {
        qDebug() << "STOP OVERLAY :";
        // 停止显示overlay
        slotHideOverlay();
    }
    else if(datagram.contains("OPEN"))
    {
        qDebug() << "OPEN VIDEOWALL";
        QStringList argList = datagram.split(" ");
        if(argList.size() < 5)
        {
            qDebug() << "too few arg";
            return;
        }

        m_nVideoWall_H = argList.at(1).toUInt();
        m_nVideoWall_V = argList.at(2).toUInt();

        m_nVideoWall_ID = argList.at(3).toUInt();
        m_nVideoWall_R = argList.at(4).toUInt();

        QString path = "/data/configs/kds-7/vw/video_wall_test_pattern.png";
        segmentationPic(path);
        m_sleepPanel->setGuideImage("./orderCut.png");
        startSleepMode(m_sleepPanel);
    }
    else if(datagram.contains("CLOSE"))
    {
        qDebug() << "CLOSE VIDEOWALL";
        QString path = "/data/configs/kds-7|/logo/sleep_image.png";
        m_sleepPanel->setGuideImage(path);
        startSleepMode(m_sleepPanel);
    }
}

void MainWidget::setOsdDispaly(bool status)
{
    if(status)
        showOsdMeun();
    else
        hideOsdMeun();
}

void MainWidget::focusOutEvent(QFocusEvent *e)
{

}

void MainWidget::keyPressEvent(QKeyEvent *e)
{
    QWidget::keyPressEvent(e);
}

void MainWidget::getKMControl()
{
    QString strCmd = "e e_stop_kmoip";
    QProcess *p = new QProcess();
    p->start("bash", QStringList() <<"-c" << strCmd);
    if(p->waitForFinished())
    {
        m_bKvmMode = false;
        delete p;
        qDebug() << "getKMControl finished";
    }
}

void MainWidget::freeKMControl()
{
    QString strCmd = "e e_start_kmoip";
    QProcess *p = new QProcess();
    p->start("bash", QStringList() <<"-c" << strCmd);
    if(p->waitForFinished())
    {
        m_bKvmMode = true;
        delete p;
        qDebug() << "freeKMControl finished";
    }
}

void MainWidget::StartMsgDConnection()
{
    m_tcpSocket = new QTcpSocket(this);

    connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(readMsgD()));
    connect(m_tcpSocket, SIGNAL(connected()), this, SLOT(connectedMsgD()));
    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayMsgDErr(QAbstractSocket::SocketError)));
    connect(m_tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnectedMsgD()));
    m_tcpSocket->abort();
    m_tcpSocket->connectToHost(QString(MSGD_IP), MSGD_FE_PORT);
}


void MainWidget::connectedMsgD()
{
    qDebug() << "Connected MSGD Finished!!!";
}

void MainWidget::readMsgD()
{
    qDebug() << "Recv Device Msg";
    qint64 r = 0;

    hdr.data_len = 0;

    if (hdr.data_len == 0) {

        if (m_tcpSocket->bytesAvailable() < (int)sizeof(struct info_hdr))
            return;

        r = m_tcpSocket->read((char*)&hdr, sizeof(struct info_hdr));
        if (r <= 0) {
            qDebug() << "Msg format err";
            return;
        }

        qDebug() << "TYPE:" << hdr.type;

        if (hdr.type != INFOTYPE_RT
         && hdr.type != INFOTYPE_ST
         && hdr.type != INFOTYPE_OSD
         && hdr.type != INFOTYPE_GUI_ACTION
         && hdr.type != INFOTYPE_RINFO_HIDDEN)
        {
            qDebug() << "Err Type";
            //BruceToDo.
        }
    }

    if (m_tcpSocket->bytesAvailable() < hdr.data_len)
    {
        char buf[100] = {0};
        //r = m_tcpSocket->read(buf,m_tcpSocket->bytesAvailable());
        QByteArray datagram = m_tcpSocket->readAll();
        if (r <= 0) {
            qDebug() << "Msg format err2";
            return;
        }

        qDebug() << "data too few:" << datagram;
        return;
    }

    QByteArray d(hdr.data_len, '\0');
    r = m_tcpSocket->read(d.data(), hdr.data_len);
    //reset hdr.data_len
    hdr.data_len = 0;
    if (r <= 0) {
        return;
    }

    qDebug() << "Recv Client Msg:" << d.constData();
    QString str(d.constData());
       qDebug() << "str:" << str;

       if(str.isEmpty())
           return;

       if(str.compare("Hotkey3") == 0)
       {
           m_bKvmMode = false;
           showOsdMeun();
       }
       else if(str.compare("Hotkey4") == 0)
       {
           m_bKvmMode = true;
           hideOsdMeun();
       }

    parseMsg(d,hdr.type);

    if (m_tcpSocket->bytesAvailable()) {
        readMsgD();
    }
}

void MainWidget::parseMsg(QByteArray &data, unsigned int type)
{
    unsigned int s = 0;

    switch (type) {
    case INFOTYPE_RT:
        s = (data.size()<MAX_STR_LEN)?(data.size()):(MAX_STR_LEN);
        memset(rInfo.str, '\0', MAX_STR_LEN);
        strncpy(rInfo.str, data.constData(), s);
        updateInfoL();
        break;
    case INFOTYPE_ST:
        memcpy(&sInfo, data.constData(), data.size());
        updateInfoR();
        break;
    case INFOTYPE_OSD:
//        memcpy(&osdInfo, data.constData(), data.size());
//        updateInfoOSD();
//        dbgMsg(tr("OSD done"));
        break;
    case INFOTYPE_GUI_ACTION:
        //memcpy(&guiActionInfo, data.constData(), data.size());
        memcpy(&guiActionInfo, data.constData(), sizeof(guiActionInfo));
        updateGUI();
        break;
    case INFOTYPE_RINFO_HIDDEN:
        memcpy(&rinfoHidden, data.constData(), data.size());
        if (rinfoHidden.hidden == 1)
        {
            bRinfoDlgHidden = true;
        }
        else
        {
            bRinfoDlgHidden = false;
        }
        updateInfoR();
        qDebug() <<"rinfo hidden done";
        break;
    default:
        qDebug() <<"wrong type?!";
    }

}

void MainWidget::updateInfoR()
{
    qDebug() << "updateInfoR_FW:" << (char*)sInfo.FW;
    qDebug() << "updateInfoR_IP:" << (char*)sInfo.IP;
    qDebug() << "updateInfoR_RemoteIP:" << (char*)sInfo.RemoteIP;

    if (bRinfoDlgHidden == true)
    {
        infoR->setText(QString("FW: %1\nLocal IP: %2\nRemote IP: %3\n")
                .arg((char*)sInfo.FW)
                .arg((char*)sInfo.IP)
                .arg((char*)sInfo.RemoteIP));
    }
    else
    {
        infoR->setText(QString("FW: %1\nLocal IP: %2\nRemote IP: %3\nID: %4")
                .arg((char*)sInfo.FW)
                .arg((char*)sInfo.IP)
                .arg((char*)sInfo.RemoteIP)
                .arg((char*)sInfo.ID));

    }
}

void MainWidget::updateInfoL()
{

    qDebug() << "updateInfoL_STR:" << (char*)rInfo.str;
    infoL->setText(QString("%1").arg((char*)rInfo.str));
    infoL->adjustSize();
}

void MainWidget::updateGUI()
{
    if(guiActionInfo.action_type == ACTION_GUI_SHOW_PICTURE)
    {
        if(guiActionInfo.ub.show_info.show_text == GUI_SHOW_TEXT)
        {
            infoL->setVisible(true);
            infoR->setVisible(true);
        }
        else
        {
            infoL->setVisible(false);
            infoR->setVisible(false);
        }
        update();
    }
}

void MainWidget::disconnectedMsgD()
{
    qDebug() << "MSGD Disconnected!!!";
}

void  MainWidget::displayMsgDErr(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        qDebug() << "MsgD disconnected";
        break;
    case QAbstractSocket::HostNotFoundError:
        qDebug() << "The host was not found. Please check the \n"
                       "host name and port settings.\n";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        qDebug() << "The connection was refused by the peer. \n"
                       "Make sure the MsgD is running, \n"
                       "and check that the host name and port \n"
                       "settings are correct.\n";
        break;
    default:
        qDebug() << "The following error occurred: ";
    }
}

void MainWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

void MainWidget::resizeEvent(QResizeEvent *event)
{
    qDebug() << "RESEIZE";
//    if(m_osdMeun != NULL)
//    showOsdMeun(m_osdMeun->getShowPosition());
}

void MainWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);
}

void MainWidget::syncConfig(QString)
{
    qApp->exit(RESTART_APP);
}

void MainWidget::isNoSignal()
{
    QString strCmd = "cat /sys/devices/platform/videoip/timing_info";
    QProcess p;
    p.start("bash", QStringList() <<"-c" << strCmd);
    p.waitForFinished();
    QString strResult = p.readAllStandardOutput();

    if(strResult.startsWith("Not"))
    {
        getInfoTimer.stop();
        qApp->exit(RESTART_APP);
    }
}

void MainWidget::startSleepMode(ModulePanel *panel)
{
    qDebug() << "main_0_3";
    QPointer<ModulePanel> p(panel);
    if (p.isNull())
    {
        return;
    }
     qDebug() << "main_0_4";

    // 隐藏 osd&overlay
    // hideOsdMeun();
    qDebug() << "main_0_5";

    m_panelStack->setCurrentWidget(panel);
   // showOsdMeun(m_osdMeun->getShowPosition());
}

void MainWidget::switchOSDMeun()
{
    if(m_osdMeun == NULL)
        return;

    m_panelStack->setCurrentWidget(m_osdMeun);
}

void MainWidget::initPanelStack()
{
    qDebug() << "main_0_1";
    m_panelStack = new QStackedWidget(this);
    m_panelStack->setObjectName("PanelStack");

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_panelStack);

    // 初始化OSD菜单，添加菜单页面
//    initOsdMeun();
//    m_panelStack->addWidget(m_osdMeun);

    // 添加休眠页面
    QString path = "/data/configs/kds-7|/logo/sleep_image.png";
    //QString path = "/logo/sleep_image.png";
    m_sleepPanel = new SleepPanel(path);
    m_panelStack->addWidget(m_sleepPanel);
}

void MainWidget::initOsdMeun()
{
   qDebug() << "main_1_1";
   m_osdMeun = new OSDMeun(this);
   m_osdMeun->setStyleSheet("background-color:rgb(169,169,169);");
   int menuHeight = m_osdMeun->getOSDMeunHeight();
   m_osdMeun->setFixedSize(g_nOsdMenuWidth * g_fScaleScreen, menuHeight * g_fScaleScreen);
   m_osdMeun->move(-m_osdMeun->width(),0);
   m_osdMeun->setVisible(false);
    qDebug() << "main_1_2" << menuHeight;

   connect(m_osdMeun,SIGNAL(hideOsdMenu()),this,SLOT(hideOsdMeun()));
   connect(m_osdMeun,SIGNAL(updateOsdMenu()),this,SLOT(updateOsdMenu()));
   qDebug() << "main_1_3";
}

//  set show overlay
void MainWidget::slotShowOverlay()
{

}

void MainWidget::slotHideOverlay()
{
    qDebug() << "overlay hide";
    if(m_imageOverlay != NULL)
        m_imageOverlay->hide();

    if(m_textOverlay != NULL)
        m_textOverlay->hide();
}


void MainWidget::hideOsdMeun()
{
    if(m_osdMeun == NULL)
        return;
    qDebug() << "main_0_4_1";
    qDebug() << "Meun Hide";
    m_osdMeun->move(-this->width(),(this->height() - m_osdMeun->height())/2);
    m_osdMeun->setVisible(false);
    // 隐藏OSD菜单时，继续显示常显Overlay
    qDebug() << "main_0_4_1_2";

    if(!m_bKvmMode)
    {
        freeKMControl();
    }

    g_bOSDMeunDisplay = false;
    showLongDisplay();
}

void MainWidget::showOsdMeun()
{
    if(m_osdMeun == NULL)
        return;

    m_osdMeun->resize(m_osdMeun->width(),height());
    m_osdMeun->setVisible(true);
    moveOsdMeun(m_osdMeun->getShowPosition());
}

void MainWidget::moveOsdMeun(int position)
{
    // OSD菜单显示时隐藏所有overlay
    qDebug() << "show meun hide overlay";
    slotHideOverlay();
    g_bOSDMeunDisplay = true;

    int xpos = 0;
    int ypos = 0;

    // 移动到指定位置
    switch (position)
    {
        case CENTER:
        {
            xpos = (g_nframebufferWidth-m_osdMeun->width())/2;
            ypos = (g_nframebufferHeight-m_osdMeun->height())/2;
            break;
        }
        case TOP_LEFT:
        {
            xpos = OSD_XPOS;
            ypos = OSD_YPOS;
            break;
        }
        case TOP_MID:
        {
            xpos = (g_nframebufferWidth-m_osdMeun->width())/2;
            ypos = OSD_YPOS;
            break;
        }
        case TOP_RIGTH:
        {
            xpos = g_nframebufferWidth-m_osdMeun->width() - OSD_XPOS;
            ypos = OSD_YPOS;
            break;
        }
        case BOTTOM_LEFT:
        {
            xpos = OSD_XPOS;
            ypos = g_nframebufferHeight-m_osdMeun->height() - OSD_YPOS;
            break;
        }
        case BOTTOM_MID:
        {
            xpos = (g_nframebufferWidth-m_osdMeun->width())/2;
            ypos = g_nframebufferHeight-m_osdMeun->height();
            break;
        }
        case BOTTOM_RIGHT:
        {
            xpos = g_nframebufferWidth-m_osdMeun->width() - OSD_XPOS;
            ypos = g_nframebufferHeight-m_osdMeun->height() - OSD_YPOS;
            break;
        }
        case LEFT_MID:
        {
            xpos = OSD_XPOS;
            ypos = (g_nframebufferHeight-m_osdMeun->height())/2;
            break;
        }
        case RIGHT_MID:
        {
            xpos = (g_nframebufferWidth-m_osdMeun->width())/2 - OSD_XPOS;
            ypos = (g_nframebufferHeight-m_osdMeun->height())/2;
            break;
        }
    }

    qDebug() << "xpos:" << xpos;
    qDebug() << "ypos:" << ypos;

    m_osdMeun->move(xpos,ypos);

    moveFramebuffer(position);
    setTransparency(80);
    m_osdMeun->startTimer();
}

void MainWidget::updateOsdMenu()
{
    qDebug() << "updateMENU";
    int menuHeight = m_osdMeun->getOSDMeunHeight();
    m_osdMeun->setFixedHeight(menuHeight * g_fScaleScreen);
    moveOsdMeun(m_osdMeun->getShowPosition());
    //update();
}

void MainWidget::showLongDisplay()
{
   if(m_imageOverlay != NULL)
   {
       if(m_imageOverlay->isLongDisplay())
       {
           showOverlay(m_imageOverlay,m_imageOverlay->getShowPos());
           m_imageOverlay->show();
       }
   }
   else if(m_textOverlay != NULL)
   {
       if(m_textOverlay->isLongDisplay())
       {
           showOverlay(m_textOverlay,m_textOverlay->getShowPos());
           m_textOverlay->show();
       }
   }
}


void MainWidget::showOverlay(OSDLabel *overlay,int position)
{
    // 隐藏菜单
    if(m_osdMeun != NULL && g_bOSDMeunDisplay)
    {
        m_osdMeun->move(-this->width(),(this->height() - m_osdMeun->height())/2);
        m_osdMeun->setVisible(false);
        g_bOSDMeunDisplay = false;
    }


     int xpos = 0;
     int ypos = 0;

     qDebug() << "main_2_2";

     OSDLabel *text = overlay;
     text->adjustSize();
     text->show();

     switch (position)
     {
         case CENTER:
         {
             xpos = (g_nframebufferWidth-text->width())/2;
             ypos = (g_nframebufferHeight-text->height())/2;
             break;
         }
         case TOP_LEFT:
         {
             xpos = OSD_XPOS;
             ypos = OSD_YPOS;
             break;
         }
         case TOP_MID:
         {
             xpos = (g_nframebufferWidth-text->width())/2;
             ypos = OSD_YPOS;
             break;
         }
         case TOP_RIGTH:
         {
             xpos = g_nframebufferWidth-text->width() - OSD_XPOS;
             ypos = OSD_YPOS;
             break;
         }
         case BOTTOM_LEFT:
         {
             xpos = OSD_XPOS;
             ypos = g_nframebufferHeight-text->height() - OSD_YPOS;
             break;
         }
         case BOTTOM_MID:
         {
             xpos = (g_nframebufferWidth-text->width())/2;
             ypos = g_nframebufferHeight-text->height();
             break;
         }
         case BOTTOM_RIGHT:
         {
             xpos = g_nframebufferWidth-text->width() - OSD_XPOS;
             ypos = g_nframebufferHeight-text->height() - OSD_YPOS;
             break;
         }
         case LEFT_MID:
         {
             xpos = OSD_XPOS;
             ypos = (g_nframebufferHeight-text->height())/2;
             break;
         }
         case RIGHT_MID:
         {
             xpos = (g_nframebufferWidth-text->width())/2 - OSD_XPOS;
             ypos = (g_nframebufferHeight-text->height())/2;
             break;
         }
     }

     qDebug() << "xpos:" << xpos;
     qDebug() << "ypos:" << ypos;

     text->move(xpos,ypos);

     moveFramebuffer(position);
     qDebug() << "m_Transparency" << m_Transparency;
     setTransparency(m_Transparency);

     // 非常显，启动定时器
     if(!text->isLongDisplay())
     {
        qDebug() << "start timer:";
        text->setoverTime();
        text->startTimer();
     }

   //  将解析信息用QOSDLabel加载
    qDebug() << "main_2_3";
}

// 移动frame buffer的位置
void MainWidget::moveFramebuffer(int align)
{
    QString Cmdstr = QString("ast_send_event -1 \"e_osd_position::%1::0::0\"").arg(align);
    qDebug() << "move Cmdstr:" <<Cmdstr;
    QProcess *p = new QProcess();
    p->start("bash", QStringList() <<"-c" << Cmdstr);
    if(p->waitForFinished())
    {
        delete p;
        qDebug() << "move frame buffer finished";
    }
}

void MainWidget::setTransparency(int Transparency)
{
    Transparency = ((float)((Transparency*1.0)/100)*31);
    QString Cmdstr = QString("ast_send_event -1 \"e_osd_on_pic::0::9999::%1::1::n:: ::0\"").arg(Transparency);
    qDebug() << "setTransparency Cmdstr:" <<Cmdstr;
    QProcess *p = new QProcess();
    p->start("bash", QStringList() <<"-c" << Cmdstr);
    if(p->waitForFinished())
    {
        delete p;
        qDebug() << "setTransparency finished";
    }
}

void MainWidget::parseOverlayJson(QString jsonpath)
{
    if(jsonpath.isEmpty())
        return;

    qDebug() << "jsonpath:" <<jsonpath;
    Json::Reader reader;
    Json::Value root;

    jsonpath = "/share/" + jsonpath;
    //jsonpath = "/overlay/" + jsonpath;
    QByteArray path = jsonpath.toLatin1();
    qDebug() << "path:" <<path;


    // 从文件中读取
    //string path = jsonpath.toStdString();
    ifstream in(path,ios::binary);

    if(!in.is_open())
    {
        qDebug() << "openfile failed";
        return;
    }

    if(reader.parse(in,root))
    {
        // 读取根节点信息

        // 读取子节点信息
        string Enable = root["genral"]["enable"].asString();
        int Timeout = root["genral"]["timeout"].asInt();
        int Transparency = root["genral"]["transparency"].asInt();

        m_Transparency = Transparency;
        qDebug() << "m_Transparency" << m_Transparency;

        cout << "Enable:" << Enable << endl;
        cout << "Timeout:" << Timeout << endl;
        cout << "Transparency:" << Transparency << endl;

        // 读取数组信息
        if(root["objects"].isArray())
        {
            for(int i=0; i < root["objects"].size(); i++)
            {
                string type = root["objects"][i]["type"].asString();
                if(type.compare("image") == 0)
                {
                    string position = root["objects"][i]["position"].asString();
                    int width = root["objects"][i]["width"].asInt();
                    int height = root["objects"][i]["height"].asInt();
                    string path = root["objects"][i]["path"].asString();

                    cout << "position:" << position << endl;
                    cout << "width:" << width << endl;
                    cout << "height:" << height << endl;
                    cout << "path:" << path << endl;

                    // 参数转换
                    QString posStr = position.c_str();
                    int showPos = parseOverlayPos(posStr);

                    QString filepath = path.c_str();

                    // 释放前一个内存
                    if(m_imageOverlay != NULL)
                    {
                        m_imageOverlay->deleteLater();
                        m_imageOverlay = NULL;
                    }

                    // 组装 overlay
                    m_imageOverlay = new OSDLabel(filepath,width,height,this);
                    m_imageOverlay->setShowPos(showPos);
                    if(m_CmdOuttime == 0)
                    {
                        m_imageOverlay->setLongDisplay(true);
                    }
                    else if(m_CmdOuttime == -1)
                    {
                        m_imageOverlay->setoverTime();
                    }
                    else
                    {
                        int overtime = m_CmdOuttime * 1000;
                        m_imageOverlay->setoverTime(overtime);
                    }

                    // 互斥
                    if(m_textOverlay != NULL)
                        m_textOverlay->hide();

                    // 显示
                    showOverlay(m_imageOverlay,showPos);
                }
                else if(type.compare("text") == 0)
                {
                    string position = root["objects"][i]["position"].asString();
                    string caption = root["objects"][i]["caption"].asString();
                    string font = root["objects"][i]["font"].asString();
                    string size = root["objects"][i]["szie"].asString();
                    string color = root["objects"][i]["color"].asString();

                    cout << "position:" << position << endl;
                    cout << "caption:" << caption << endl;
                    cout << "font:" << font << endl;
                    cout << "size:" << size << endl;
                    cout << "color:" << color << endl;

                    QString str = size.c_str();
                    quint8 fontsize;
                    if(str.compare("small") == 0)
                    {
                        fontsize = FONTSIZE_SMALL * g_fScaleScreen;
                    }
                    else if(str.compare("medium") == 0)
                    {
                        fontsize = FONTSIZE_MID * g_fScaleScreen;
                    }
                    else if(str.compare("large") == 0)
                    {
                        fontsize = FONTSIZE_BIG * g_fScaleScreen;
                    }

                    // 参数转换
                    QString displayStr = caption.c_str();
                    qDebug() << "displayStr:" <<displayStr;

                    QString colorStr = color.c_str();
                    QByteArray byteColor = colorStr.toLatin1();
                    QDataStream in(byteColor);

                    quint32 fontcolor = colorStr.toUInt(NULL,16);

                    QString posStr = position.c_str();
                    int showPos = parseOverlayPos(posStr);

                    qDebug() << "parseOverlayJson::fontcolor:" << hex << fontcolor;
                    qDebug() << "parseOverlayJson::fontsize:" << fontsize;
                    qDebug() << "parseOverlayJson::displayStr:" << displayStr;
                    qDebug() << "parseOverlayJson::showPos:" << showPos;

                    // 释放前一个内存
                    if(m_textOverlay != NULL)
                    {
                        m_textOverlay->deleteLater();
                        m_textOverlay = NULL;
                    }

                    // 组装 overlay
                    m_textOverlay = new OSDLabel(displayStr,fontsize,fontcolor,this);
                    m_textOverlay->setShowPos(showPos);

                    if(m_CmdOuttime == 0)
                    {
                        m_textOverlay->setLongDisplay(true);
                    }
                    else if(m_CmdOuttime == -1)
                    {
                        m_textOverlay->setoverTime();
                    }
                    else
                    {
                        int overtime = m_CmdOuttime * 1000;
                        m_textOverlay->setoverTime(overtime);
                    }

                    // 互斥
                    if(m_imageOverlay != NULL)
                        m_imageOverlay->hide();

                    // 显示
                    showOverlay(m_textOverlay,showPos);
                }
            }
        }
    }
}

int MainWidget::parseOverlayPos(QString positon)
{
    if(positon.isEmpty())
        return TOP_MID;

    if(positon.compare("top_center") == 0)
    {
        qDebug() << "parseOverlayPos::positon:" << positon;
        return TOP_MID;
    }
    else if(positon.compare("top_left") == 0)
    {
        return TOP_LEFT;
    }
    else if(positon.compare("top_right") == 0)
    {
        return TOP_RIGTH;
    }
    else if(positon.compare("bottom_left") == 0)
    {
        return BOTTOM_LEFT;
    }
    else if(positon.compare("bottom_center") == 0)
    {
        return BOTTOM_MID;
    }
    else if(positon.compare("bottom_right") == 0)
    {
        return BOTTOM_RIGHT;
    }
    else if(positon.compare("left") == 0)
    {
        return LEFT_MID;
    }
    else if(positon.compare("center") == 0)
    {
        return CENTER;
    }
    else if(positon.compare("right") == 0)
    {
        return RIGHT_MID;
    }
}

void MainWidget::segmentationPic(QString picpath)
{
    // 判断图片路径是否存在
    QImage image;
    QImage orderImage;
    QImage saveImage;
    QImageReader reader;

    reader.setFileName(picpath);
    QSize imageSize = reader.size();
    imageSize.scale(this->width(),this->height(),Qt::IgnoreAspectRatio);
    reader.setScaledSize(imageSize); // 缩放图片适应屏幕
    image = reader.read();

    int column = 4;
    int row = 4;
    int out_id = 5;
    int rotation = 0;

    column = m_nVideoWall_V;
    row = m_nVideoWall_H;

    out_id = m_nVideoWall_ID;
    rotation = m_nVideoWall_R;

    // 确定目标区域的宽高, 起始坐标
     int width = this->width()/column;
     int height = this->height()/row;

     int hpos,vpos;

     if(out_id <= column)
     {
         hpos = this->width()/column * (out_id - 1);
         vpos = 0;
     }
     else
     {
        // 确定行列位置
        int R = (out_id % column) ? (out_id/column + 1) : (out_id/column);
        int C = (out_id % column) ? (out_id%column) : column;
        qDebug() << "orderArea R:" << R;
        qDebug() << "orderArea C:" << C;


        // 起始坐标
        hpos = this->width()/column * (C - 1);
        vpos = this->height()/row * (R - 1);
        qDebug() << "orderArea hpos:" << hpos;
        qDebug() << "orderArea vpos:" << vpos;
     }

    // 切割目标区域
    orderImage = image.copy(hpos,vpos,width,height);

    // 旋转
    QMatrix matrix;
    matrix.rotate(rotation);
    orderImage = orderImage.transformed(matrix,Qt::FastTransformation);

    if(orderImage.save("orderCut.png","PNG"))
    {
        qDebug() << "cut finished";       
    }
    else
    {
        qDebug() << "cut failed";
    }
}















