#include "mainwidget.h"
#include "videolistwidget.h"
//#include "osdlabel.h"
#include "common/global.h"
#include "common/scale_global.h"
#include "p3ktcp.h"
#include "dialog.h"

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
#include <unistd.h>

#include <QProcess>

#define RESOLUTION_CONFIG   "/sys/devices/platform/videoip/timing_info"
//#define SLEEP_IMAGE_PATH  "/data/configs/kds-7/logo/sleep_image.jpg"
#define SLEEP_IMAGE_PATH    "/share/default.jpg"

#define MENUINFO_PATH      "/data/configs/kds-7/osd/osd.json"
#define CHANNELS_LIST_PATH "/data/configs/kds-7/channel/channel_map.json"

using namespace std;

QMap<int, OSDLabel *> MainWidget::g_overlayMap;
QSet<int> MainWidget::osdIdSet;

MainWidget::MainWidget(QWidget *parent) : QWidget(parent)
    ,m_imageOverlay(NULL)
    ,m_textOverlay(NULL)
    ,m_CmdOuttime(0)
    ,m_overlayStatus(false)
    ,m_bKvmMode(false)
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

    qDebug() << "main_3:" << g_bDeviceSleepMode;

    // 初始化OSD菜单
     initOsdMeun();

    // 初始化页面切换
    initPanelStack();

    connect(m_sleepPanel,SIGNAL(sigStartSleepMode()),this,SLOT(startSleepMode()));
    connect(m_sleepPanel,SIGNAL(sigStartKVM(bool)),this,SLOT(handleKvmMsg(bool)));

    // 正常工作状态
    if(!g_bDeviceSleepMode)
    {
        //    m_panelStack->close();
        m_sleepPanel->setVisible(false);
        //    connect(&getInfoTimer,SIGNAL(timeout()),this,SLOT(isNoSignal()));
        //    this->getInfoTimer.start(1000);
    }

    qDebug() << "main_4";

    // 分割图片test
    //    QString path = "/share/image.png";
    //    segmentationPic(path);
    //    segmentationPic(path);
    //    m_sleepPanel->setGuideImage("./orderCut.png");
    //    startSleepMode(m_sleepPanel);

    // 处理p3k命令
    UdpRecv = UdpRecvThread::getInstance();
    UdpRecv->start();

    connect(UdpRecv,SIGNAL(RecvData(QByteArray)),this,SLOT(onRecvData(QByteArray)));

    // 测试OSD菜单显示
    // showOsdMeun();

    // 测试overlay显示
    //  QString filename = "overlay2_setting.json";
    //  parseOverlayJson(filename);

    QTimer::singleShot(1500,this,SLOT(slotShowOverlay()));
}

MainWidget::~MainWidget()
{

}


bool MainWidget::p3kconnected()
{
    m_p3kTcp = P3ktcp::getInstance();
    while(!m_p3kTcp->p3kConnect())
    {
        usleep(1000 * 1000);
    }

    connect(m_p3kTcp,SIGNAL(tcpRcvMsg(QByteArray)),this,SLOT(parseCmdResult(QByteArray)));

    qDebug() << "connected P3K OK";
    return true;
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

    QStringList argList = QString(datagram).split(" ").at(1).split(",");
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

        QString orderPath("./orderCut.png");
        m_sleepPanel->setSleepPicture(orderPath.toLatin1().data());
        startSleepMode();
        update();
    }
    else if(datagram.contains("CLOSE"))
    {
        qDebug() << "CLOSE VIDEOWALL";
        QString path = "/share/default.jpg";
        m_sleepPanel->setSleepPicture(path.toLatin1().data());
        startSleepMode();
        update();
    }
}

void MainWidget::setOsdDispaly(bool status)
{
    if(status)
    {
        slotHideOverlay();
        QTimer::singleShot(200,this,SLOT(showOsdMeun()));
    }
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

    // 显示光标
    this->setCursor(Qt::ArrowCursor);
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

    // 隐藏光标
    this->setCursor(Qt::BlankCursor);
}

void MainWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

void MainWidget::resizeEvent(QResizeEvent *event)
{
    qDebug() << "RESEIZE";
}

void MainWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget::mouseDoubleClickEvent(event);
}

void MainWidget::syncConfig(QString path)
{
    //qApp->exit(RESTART_APP);
    if(path.compare(RESOLUTION_CONFIG) == 0)
    {
        // 分辨率文件发生改变
        qDebug("Resolution Change!");
        QTimer::singleShot(200,this,SLOT(getResolutionFromTiming()));
    }
    else if(path.compare(SLEEP_IMAGE_PATH) == 0)
    {
        // 休眠图片发生改变
        qDebug("SLEEP Pic Change!");
        if(m_sleepPanel != NULL)
            m_sleepPanel->setSleepPicture(path.toLatin1().data());
    }
    else if(path.compare(MENUINFO_PATH) == 0)
    {
        // osd.json发生改变
        qDebug("OSD.JSON Change!");
        if(m_osdMeun == NULL)
            return;

        m_osdMeun->parseMeunJson(MENUINFO_PATH);
        m_osdMeun->setListWidgetHeight();
        m_osdMeun->setMeunFont();

        if(m_osdMeun->getdisplayStatus())
        {
            // OSD菜单显示时隐藏所有overlay
            qDebug() << "show meun hide overlay";
            slotHideOverlay();

            m_osdMeun->hide();

            QTimer::singleShot(200,this,SLOT(showOsdMeun()));
        }
        else
        {
            hideOsdMeun();
        }
    }
    else if(path.compare(CHANNELS_LIST_PATH) == 0)
    {
        // channel.json发生改变
        qDebug("Channel.JSON Change!");
        if(m_osdMeun == NULL)
            return;

        m_osdMeun->parseChannelJson();

        if(m_osdMeun->getdisplayStatus())
        {
            // OSD菜单显示时隐藏所有overlay
            qDebug() << "show meun hide overlay";
            slotHideOverlay();

            m_osdMeun->hide();

            QTimer::singleShot(200,this,SLOT(showOsdMeun()));
        }
        else
        {
            hideOsdMeun();
        }
    }
}

void MainWidget::getResolutionFromTiming()
{
    watch->removePath(RESOLUTION_CONFIG);

    QString strCmd = "cat /sys/devices/platform/videoip/timing_info";
    QProcess p;
    p.start("bash", QStringList() <<"-c" << strCmd);
    p.waitForFinished();
    QString strResult = p.readAllStandardOutput();

    if(strResult.isEmpty())
        return;

    qDebug() << "MainWidget::strResult:" << strResult;

    if(!strResult.startsWith("Not"))
    {
        // 解析输出源的分辨率应用到OSD
         QString startStr = "Capture";
         QString endStr = "Compress";

         int spos = strResult.indexOf(startStr);
         QString tmpStr = strResult.mid(spos);

         int epos = strResult.indexOf(endStr);
         tmpStr = tmpStr.left(epos);

         QStringList list = tmpStr.split(" ");
         qDebug() << "list:" << list;

         if(list.isEmpty())
             return;

         QString orderStr;
         for(int index = 0; index < list.count() ; index++)
         {
             if(list.at(index).startsWith("["))
             {
                orderStr = list.at(index);
                qDebug() << "ordStr:" << orderStr;
                break;
             }
         }

        list.clear();
        list = orderStr.split("X");

        QString widthStr = list.at(0).mid(1,list.at(0).length()-2);

        if(widthStr.isEmpty())
            return;

        int width = widthStr.toInt();
        qDebug() << "widthStr:" << widthStr;

        QString heightStr = list.at(1).mid(1,list.at(1).length()-2);
        int height = heightStr.toInt();
        qDebug() << "heightStr:" << heightStr;

        if(width != g_nScreenWidth)
        {
            float wScale = static_cast<float> (width) / g_nStdScreenWidth;
            float hScale = static_cast<float> (height) / g_nStdScreenHeight;
            g_fScaleScreen = qMin(wScale, hScale);

            g_nScreenWidth = width;
            g_nScreenHeight = height;

            resize(g_nScreenWidth,g_nScreenHeight);

            //m_osdMeun->deleteLater();
            if(m_osdMeun != NULL)
            {
                m_osdMeun->setVisible(false);

                destroyOsdAndOverlay();
                // m_osdMeun->deleteLater();
                initOsdMeun();
                initOverlay();
            }

            g_bDeviceSleepMode = false;
            update();
        }
    }
    else
    {
        if(g_nframebufferWidth != g_nScreenWidth)
        {
            float wScale = static_cast<float> (g_nframebufferWidth) / g_nStdScreenWidth;
            float hScale = static_cast<float> (g_nframebufferHeight) / g_nStdScreenHeight;
            g_fScaleScreen = qMin(wScale, hScale);

            g_nScreenWidth = g_nframebufferWidth;
            g_nScreenHeight = g_nframebufferHeight;

            resize(g_nScreenWidth,g_nScreenHeight);

            if(m_osdMeun != NULL)
            {
                hideOsdMeun();

                destroyOsdAndOverlay();

                initOsdMeun();
                initOverlay();
            }

            g_bDeviceSleepMode = true;
            update();
        }
            // 显示休眠界面，更新
            startSleepMode();
            update();
        }

    // 文件会被删除所以需要重新监控
    watch->addPath(RESOLUTION_CONFIG);
}

void MainWidget::destroyOsdAndOverlay()
{
    if(m_osdMeun != NULL)
    {
        delete m_osdMeun;
        m_osdMeun = NULL;
    }

    if(m_imageOverlay != NULL)
    {
        delete m_imageOverlay;
        m_imageOverlay = NULL;
    }

    if(m_textOverlay != NULL)
    {
        delete  m_textOverlay;
        m_textOverlay = NULL;
    }
}

void MainWidget::isNoSignal()
{
//    QString strCmd = "cat /sys/devices/platform/videoip/timing_info";
//    QProcess p;
//    p.start("bash", QStringList() <<"-c" << strCmd);
//    p.waitForFinished();
//    QString strResult = p.readAllStandardOutput();

//    if(strResult.startsWith("Not"))
//    {
//        getInfoTimer.stop();
//        qApp->exit(RESTART_APP);
//    }
}

void MainWidget::handleKvmMsg(bool enable)
{
    if(!enable)
    {
        if(m_osdMeun->getdisplayStatus())
        {
            m_bKvmMode = false;
            showOsdMeun();
        }
        freeKMControl();
    }
    else
    {
         m_bKvmMode = true;
         hideOsdMeun();
    }
}

void MainWidget::startSleepMode()
{
    qDebug() << "main_0_3";

    if(m_sleepPanel != NULL)
    {
        m_panelStack->setCurrentWidget(m_sleepPanel);
        m_sleepPanel->setVisible(true);
    }

     qDebug() << "main_0_4";

    // 隐藏 osd&overlay
    // hideOsdMeun();
    qDebug() << "main_0_5";
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
    QString path = "/share/default.jpg";
    //    QString path = "/data/configs/kds-7/logo/sleep_image.jpg";
    m_sleepPanel = new Dialog();

    QBrush brush;
    QPalette palette = m_sleepPanel->palette();
    palette.setBrush(QPalette::Window, brush);
    palette.setColor(QPalette::WindowText, QColor(QString("white")));
    m_sleepPanel->setPalette(palette);

    QFont fnt;
    fnt.setPointSize(8);
    m_sleepPanel->setFont(fnt);

    m_panelStack->addWidget(m_sleepPanel);
    m_panelStack->setCurrentWidget(m_sleepPanel);
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

void MainWidget::initOverlay()
{
    //上电解析overlay文件
    m_bFirst = true;
    parseOverlayJson("overlay1_setting.json");

    // 如果1进入常显则不去解析2
    if(!m_bFirst)
        return;

    parseOverlayJson("overlay2_setting.json");
}

//  set show overlay
void MainWidget::slotShowOverlay()
{
    // 添加文件监控
    QStringList paths ;

    paths << RESOLUTION_CONFIG;
    paths << SLEEP_IMAGE_PATH;
    paths << MENUINFO_PATH;
    paths << CHANNELS_LIST_PATH;

    watch = new QFileSystemWatcher;
    watch->addPaths(paths);
    connect(watch, SIGNAL(fileChanged(QString)), this, SLOT(syncConfig(QString)));

    initOverlay();
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

    qDebug() << "main_0_4_1_2";

    // 隐藏菜单后释放KM
    if(!m_bKvmMode)
    {
        freeKMControl();
    }

    g_bOSDMeunDisplay = false;

    // 隐藏OSD菜单时，继续显示常显Overlay
    QTimer::singleShot(200,this,SLOT(showLongDisplay()));
}

void MainWidget::showOsdMeun()
{
    if(m_osdMeun == NULL)
        return;

    m_osdMeun->resize(m_osdMeun->width(),height());
    m_osdMeun->setVisible(false);

    if(m_bKvmMode)
    {
        getKMControl();
    }

    // 防遮挡，显示在最顶层
    m_osdMeun->activateWindow();
    m_osdMeun->raise();

    moveOsdMeun(m_osdMeun->getShowPosition());
}

void MainWidget::moveOsdMeun(int position)
{
    g_bOSDMeunDisplay = true;

    if(!g_bDeviceSleepMode)
    {
        m_sleepPanel->setInfoEnable(false);
    }

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
            xpos = g_nframebufferWidth-m_osdMeun->width() - OSD_XPOS;
            ypos = (g_nframebufferHeight-m_osdMeun->height())/2;
            break;
        }
    }

    qDebug() << "xpos:" << xpos;
    qDebug() << "ypos:" << ypos;

    moveFramebuffer(position);
    m_osdMeun->move(xpos,ypos);
    setTransparency(80);
    m_osdMeun->startTimer();

    m_osdMeun->setVisible(true);
}

void MainWidget::updateOsdMenu()
{
    qDebug() << "updateMENU";
    int menuHeight = m_osdMeun->getOSDMeunHeight();
    m_osdMeun->setFixedHeight(menuHeight * g_fScaleScreen);
    // moveOsdMeun(m_osdMeun->getShowPosition());
    update();
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
             xpos = g_nframebufferWidth-text->width() - OSD_XPOS;
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

    //jsonpath = "/share/" + jsonpath;
    jsonpath = "/data/configs/kds-7/overlay/" + jsonpath;
    QByteArray path = jsonpath.toLatin1();
    qDebug() << "path:" <<path;


    // 从文件中读取
    //string path = jsonpath.toStdString();
    ifstream in(path,ios::binary);

    if(!in.is_open())
    {
        qDebug() << "open overlay.json failed";
        return;
    }

    if(reader.parse(in,root))
    {
        // 读取根节点信息

        // 读取子节点信息
        string Enable = root["genral"]["enable"].asString();
        int Timeout = root["genral"]["timeout"].asInt();
        int Transparency = root["genral"]["transparency"].asInt();

        QString enableStr = Enable.c_str();
        if(enableStr.compare("on") == 0)
        {
            m_overlayStatus = true;
        }else
        {
            m_overlayStatus = false;
        }

        m_CmdOuttime = Timeout;
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

                    if(m_bFirst)
                    {
                        // 显示
                        if(m_overlayStatus && (m_CmdOuttime == 0))
                        {
                            m_bFirst = false;
                            showOverlay(m_imageOverlay,showPos);
                        }
                    }
                    else
                    {
                        // 显示
                        if(m_overlayStatus)
                        {
                            showOverlay(m_imageOverlay,showPos);
                        }
                    }
                }
                else if(type.compare("text") == 0)
                {
                    string position = root["objects"][i]["position"].asString();
                    string caption = root["objects"][i]["caption"].asString();
                    string font = root["objects"][i]["font"].asString();
                    string size = root["objects"][i]["size"].asString();
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
                    else
                    {
                        fontsize = FONTSIZE_SMALL * g_fScaleScreen;
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

                    if(m_bFirst)
                    {
                        // 上电显示
                        if(m_overlayStatus && (m_CmdOuttime == 0))
                        {
                            m_bFirst = false;
                            showOverlay(m_textOverlay,showPos);
                        }
                    }
                    else
                    {
                        // 显示
                        if(m_overlayStatus)
                        {
                            showOverlay(m_textOverlay,showPos);
                        }
                    }
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















