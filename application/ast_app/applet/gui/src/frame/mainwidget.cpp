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
#include <QWSServer>

#include <stdlib.h>
#include <unistd.h>

#include <QProcess>

#define KM_USB_PATH         "/data/configs/kds-7/usb/km_usb.json"
#define RESOLUTION_CONFIG   "/sys/devices/platform/videoip/timing_info"
//#define SLEEP_IMAGE_PATH  "/data/configs/kds-7/logo/sleep_image.jpg"
#define SLEEP_IMAGE_PATH    "/share/default.jpg"

#define MENUINFO_PATH       "/data/configs/kds-7/osd/osd.json"
#define CHANNELS_LIST_PATH  "/data/configs/kds-7/channel/channel_map.json"

#define OVERLAY_TEXT_PATH   "/data/configs/kds-7/overlay/overlay1_setting.json"
#define OVERLAY_IMAGE_PATH   "/data/configs/kds-7/overlay/overlay2_setting.json"

#define TRANS_BACKGROUND    "/usr/local/bin/mono.jpg"

#define I_WEB_PIPE          "/tmp/overlay_f"

#define CMD_START_KM        "e e_start_kmoip"
#define CMD_STOP_KM         "e e_stop_kmoip"

#define CMD_TIMING          "cat /sys/devices/platform/videoip/timing_info"
#define CMD_SCREEN          "cat /sys/devices/platform/display/screen"

#define ACADEMY_4K          3656

using namespace std;
int g_Transparency = 255;
bool g_bDeviceInfoDisplay = false;

QMap<int, OSDLabel *> MainWidget::g_overlayMap;
QSet<int> MainWidget::osdIdSet;

MainWidget::MainWidget(QWidget *parent) : QWidget(parent)
    ,m_imageOverlay(NULL)
    ,m_textOverlay(NULL)
    ,m_background(NULL)
    ,m_CmdOuttime(-1)
    ,m_overlayStatus(false)
    ,m_bKvmMode(true)
    ,m_isSnedQueryCmd(false)
    ,m_bMoveMeunFlag(false)
    ,m_bMoveInfoFlag(false)
    ,m_bInfoCmdHide(false)
{
    // ????????????
    setAttribute(Qt::WA_TranslucentBackground);

    qDebug() << "ScreenWidth:" << g_nScreenWidth << "ScreenHeight:" << 	g_nScreenHeight;

    // ????????????????????????
    setFixedSize(g_nScreenWidth,g_nScreenHeight);
    setWindowFlags(Qt::FramelessWindowHint);

    qDebug() << "main_0";

    qDebug() << "main_3:" << g_bDeviceSleepMode;

    // ?????????OSD??????
    initOsdMeun();

    // ?????????????????????
    initPanelStack();

    // ???????????????????????????
    initDeviceInfo();

    connect(m_sleepPanel,SIGNAL(sigStartSleepMode()),this,SLOT(startSleepMode()));
    connect(m_sleepPanel,SIGNAL(sigStartKVM(bool)),this,SLOT(handleKvmMsg(bool)));
    connect(m_sleepPanel,SIGNAL(sigUpdateDeviceInfo(QLabel*)),this,SLOT(slotUpdateDeviceInfo(QLabel *)));

    qDebug() << "main_4";

    //    ????????????test
    //    QString path = "/share/image.png";
    //    segmentationPic(path);
    //    segmentationPic(path);
    //    m_sleepPanel->setGuideImage("./orderCut.png");
    //    startSleepMode(m_sleepPanel);

    //  ?????????????????????
    m_background = new OSDLabel(TRANS_BACKGROUND,300,300,this);
    m_background->setVisible(false);

    // ??????p3k??????
    UdpRecv = UdpRecvThread::getInstance();
    UdpRecv->start();

    connect(UdpRecv,SIGNAL(RecvData(QByteArray)),this,SLOT(onRecvData(QByteArray)));

    QTimer::singleShot(2000,this,SLOT(slotShowOverlay()));

    connect(&getInfoTimer,SIGNAL(timeout()),this,SLOT(getResolutionFromTiming()));
    this->getInfoTimer.start(1000);
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
    // '#'??????????????????????????????'~nn@'?????????????????????

    if(datagram.startsWith("~01@"))
    {
        datagram = datagram.mid(4);
    }

    QStringList argList = QString(datagram).split(" ");
    if(argList.at(0).contains("KDS-CHANNEL-SELECT") && m_isSnedQueryCmd)
    {
        m_isSnedQueryCmd = false;
        int id = argList.at(1).split(",").at(1).toUInt();
        qDebug() << "id:" << id;
        m_osdMeun->setChannelLocation(id);
    }
}

void MainWidget::onRecvData(QByteArray data)
{
    QString datagram(data);
    qDebug() << "Recv :" << datagram;
    if(datagram.contains("SET"))
    {
        // ??????OSD????????????????????????mode;(??????????????????????????????????????????????????????)
        qDebug() << "SHOW OSD :";
        QStringList argList = datagram.split(" ");

        if(argList.size() < 2)
        {
            qDebug() << "too few arg";
            return;
        }

        qDebug() << "argList:" << argList;

        bool bshow;
        int mode = argList.at(2).toInt();
        qDebug() << "DeviceInfo_mode:" << mode;
        if(mode == 0)
        {
            bshow = false;
        }
        else if(mode == 2)
        {
            bshow = true;
        }
        else
        {
            return ;
        }

         setDeviceInfoDispaly(bshow);
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

void MainWidget::setDeviceInfoDispaly(bool status)
{
    if(m_deviceInfo == NULL)
        return;

    // ????????????????????????????????????
    if(status)
    {
        qDebug() << "g_bDeviceSleepMode:" <<g_bDeviceSleepMode;
        // ??????????????????,???????????????????????????
        if(!g_bDeviceSleepMode)
        {
            // ??????OSD???OVERLAY
            m_osdMeun->parseMeunJson(MENUINFO_PATH);
            hideOsdMeun(false);
            slotHideOverlay();

            if(!g_bDeviceInfoDisplay)
            {
                m_bMoveInfoFlag = true;
                QTimer::singleShot(200,this,SLOT(moveDisplayArea()));

                QTimer::singleShot(500,this,SLOT(showDeviceInfo()));
            }
            else
            {
                // ?????????????????????????????????????????????,????????????????????????
                if(!m_osdMeun->getDeviceInfoDisplayStatus())
                {
                    DeviceInfoTimer.stop();
                    int timeout = m_osdMeun->getDeviceInfoTimerout();
                    DeviceInfoTimer.start(timeout);
                }
                else
                {
                    // ???????????????????????????
                    DeviceInfoTimer.stop();
                }
            }
        }
    }
    else
    {
        qDebug() << "g_bDeviceSleepMode:" <<g_bDeviceSleepMode;
        if(!g_bDeviceSleepMode)
        {
            slotHideDeviceInfo();
        }
    }
}

void MainWidget::showDeviceInfo()
{
    qDebug() << "hide DeivceInfo";
    m_deviceInfo->setVisible(true);
    g_bDeviceInfoDisplay = true;

    // ????????????????????????
    qDebug() << "m_deviceInfo->width():" << m_deviceInfo->width();
    qDebug() << "m_deviceInfo->height():" << m_deviceInfo->height();

    int xpos = g_nframebufferWidth- m_deviceInfo->width() - 4*OSD_XPOS;
    int ypos = g_nframebufferHeight- m_deviceInfo->height() - OSD_YPOS;

    qDebug() << "xpos:" << xpos;
    qDebug() << "ypos:" << ypos;

    m_deviceInfo->move(xpos,ypos);

    // ?????????????????????
    moveFramebuffer(BOTTOM_RIGHT);

    if(!m_osdMeun->getDeviceInfoDisplayStatus())
    {
        int timeout = m_osdMeun->getDeviceInfoTimerout();
        qDebug() << "timeout:" << ypos;

        DeviceInfoTimer.stop();
        DeviceInfoTimer.start(timeout);
    }

    if(m_background)
    {
        m_background->setVisible(false);
    }
}

void MainWidget::slotHideDeviceInfo(bool isStartOverlay)
{
    qDebug() << "hide DeivceInfo";

//    m_sleepPanel->setDeviceInfoStatus(false);

    DeviceInfoTimer.stop();
    m_deviceInfo->setVisible(false);
    m_deviceInfo->repaint();
    g_bDeviceInfoDisplay = false;

    if(isStartOverlay)
    {
        if(!m_osdMeun->getDeviceInfoDisplayStatus())
        {
            // ?????????????????????????????????OVERLAY????????????
            QTimer::singleShot(500,this,SLOT(showLongDisplay()));
        }
        else
        {
            // ???????????????????????????????????????OVERLAY????????????
            m_osdMeun->parseMeunJson(MENUINFO_PATH);
            if(!m_osdMeun->getdisplayConfig())
            {
                QTimer::singleShot(500,this,SLOT(showLongDisplay()));
            }
        }
    }
}

void MainWidget::focusOutEvent(QFocusEvent *e)
{
    QWidget::focusOutEvent(e);
}

void MainWidget::keyPressEvent(QKeyEvent *e)
{
    QWidget::keyPressEvent(e);
}

void MainWidget::getKMControl()
{
    QProcess p;
    p.start("bash", QStringList() <<"-c" << CMD_STOP_KM);
    if(p.waitForFinished())
    {
        m_bKvmMode = false;
        qDebug() << "getKMControl finished";
    }
}

void MainWidget::freeKMControl()
{
    QProcess p;
    p.start("bash", QStringList() <<"-c" << CMD_START_KM);
    if(p.waitForFinished())
    {
        m_bKvmMode = true;
        qDebug() << "freeKMControl finished";
    }
}

void MainWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

void MainWidget::resizeEvent(QResizeEvent *event)
{
    qDebug() << "RESEIZE";
}

void MainWidget::syncConfig(QString path)
{
    //qApp->exit(RESTART_APP);
    if(path.compare(RESOLUTION_CONFIG) == 0)
    {
        // ???????????????????????????
        qDebug("Resolution Change!");
//        watch->removePath(RESOLUTION_CONFIG);
//        hideOsdMeun(false);
//        watch->addPath(RESOLUTION_CONFIG);
    }
    else if(path.compare(SLEEP_IMAGE_PATH) == 0)
    {
        // ????????????????????????
        qDebug("SLEEP Pic Change!");
        if(m_sleepPanel != NULL)
            m_sleepPanel->setSleepPicture(path.toLatin1().data());
    }
    else if(path.compare(MENUINFO_PATH) == 0)
    {
        // osd.json????????????
        qDebug("OSD.JSON Change!");
        qDebug() << "g_bOSDMeunDisplay:" << g_bOSDMeunDisplay;
        if(g_bOSDMeunDisplay)
        {
            // Display??????P3K?????????????????????????????????????????????????????????OSD??????
            hideOsdMeun(false);

            updateOsdMenuData();

            if(!g_bDeviceSleepMode)
            {
                m_bMoveMeunFlag = true;
                QTimer::singleShot(200,this,SLOT(moveDisplayArea()));
            }

            QTimer::singleShot(500,this,SLOT(showOsdMeun()));
        }
    }
    else if(path.compare(CHANNELS_LIST_PATH) == 0)
    {
        // channel.json????????????
        qDebug("Channel.JSON Change!");

        qDebug() << "g_bOSDMeunDisplay:" << g_bOSDMeunDisplay;
        if(g_bOSDMeunDisplay)
        {
             updateChannelList();
        }
    }
    else if(path.compare(OVERLAY_TEXT_PATH) == 0)
    {
         parseOverlayText();
    }
    else if(path.compare(OVERLAY_IMAGE_PATH) == 0)
    {
         QTimer::singleShot(300,this,SLOT(parseOverlayImage()));
    }
}

void MainWidget::parseOverlayText()
{
    parseOverlayJson("overlay1_setting.json");
}
void MainWidget::parseOverlayImage()
{
    parseOverlayJson("overlay2_setting.json");
}

void MainWidget::updateOsdMenuData()
{
    if(m_osdMeun == NULL)
        return;

    m_osdMeun->parseMeunJson(MENUINFO_PATH);
    m_osdMeun->parseChannelJson();
    m_osdMeun->setListWidgetHeight();
    m_osdMeun->setMeunFont();
}

void MainWidget::updateChannelList()
{
    if(m_osdMeun == NULL)
        return;

    m_osdMeun->parseChannelJson();
}

void MainWidget::getResolutionFromTiming()
{
    QProcess p;
    p.start("bash", QStringList() <<"-c" << CMD_TIMING);
    p.waitForFinished();
    QString strResult = p.readAllStandardOutput();

    p.start("bash", QStringList() <<"-c" << CMD_SCREEN);
    p.waitForFinished();
    QString strResultScreeenInfo = p.readAllStandardOutput();

    // qDebug() << "MainWidget::strResult:" << strResult;

    if(strResult.contains("Timing Table:") && strResultScreeenInfo.contains("decode"))
    {
        // ????????????????????????????????????OSD
         QString startStr = "Timing Table:";
         QString endStr = "Pixel Rate:";

         int spos = strResult.indexOf(startStr);
         QString tmpStr = strResult.mid(spos);

         int epos = strResult.indexOf(endStr);
         tmpStr = tmpStr.left(epos);

         QStringList list = tmpStr.split(" ");
//         qDebug() << "list:" << list;

         if(list.isEmpty())
             return;

         QString orderStr = list.at(4);
         if(orderStr.startsWith("["))
         {
             list.clear();
             list = orderStr.split("X");

             QString widthStr = list.at(0).mid(1,list.at(0).length()-2);
             int width = widthStr.toInt();
//             qDebug() << "widthStr:" << widthStr;

             QString heightStr = list.at(1).mid(1,list.at(1).length()-2);
             int height = heightStr.toInt();
//             qDebug() << "heightStr:" << heightStr;

             g_bDeviceSleepMode = false;
             if(width != g_nScreenWidth)
             {

                 m_sleepPanel->setSleepGUIHide("/share/");

                 getInfoTimer.stop();
                 float wScale = static_cast<float> (width) / g_nStdScreenWidth;
                 float hScale = static_cast<float> (height) / g_nStdScreenHeight;
                 g_fScaleScreen = qMin(wScale, hScale);

                 g_nScreenWidth = width;
                 g_nScreenHeight = height;

                 this->setFixedSize(g_nScreenWidth,g_nScreenHeight);

                 qDebug() << "getResolutionFromTiming:hideOsdMeun_1";
                 m_deviceInfo->setVisible(false);
                 g_bDeviceInfoDisplay = false;

                 hideOsdMeun(false);
                 slotHideOverlay();

                 QTimer::singleShot(1000,this,SLOT(slotDelayReinit()));
            }
        }
    }
    else if(strResult.startsWith("Not") && strResultScreeenInfo.contains("message"))
    {
        if(g_nScreenWidth != g_nframebufferWidth)
        {
            g_bDeviceSleepMode = true;

            getInfoTimer.stop();
            float wScale = static_cast<float> (g_nframebufferWidth) / g_nStdScreenWidth;
            float hScale = static_cast<float> (g_nframebufferHeight) / g_nStdScreenHeight;
            g_fScaleScreen = qMin(wScale, hScale);

            g_nScreenWidth = g_nframebufferWidth;
            g_nScreenHeight = g_nframebufferHeight;

            this->setFixedSize(g_nScreenWidth,g_nScreenHeight);

            qDebug() << "getResolutionFromTiming:hideOsdMeun_2";
            m_deviceInfo->setVisible(false);
            g_bDeviceInfoDisplay = false;

            hideOsdMeun(false);
            slotHideOverlay();

            QTimer::singleShot(1000,this,SLOT(slotDelayReinit()));
        }
    }
}

void MainWidget::slotDelayReinit()
{
    destroyOsdAndOverlay();
    reInit();

    getInfoTimer.start(1000);
}

void MainWidget::reInit()
{
    initOsdMeun();
    initDeviceInfo();
    initOverlay();
    update();
}

void MainWidget::destroyOsdAndOverlay()
{
    if(m_osdMeun != NULL)
    {
        m_osdMeun->setVisible(false);

        delete m_osdMeun;
        m_osdMeun = NULL;
    }

    if(m_imageOverlay != NULL)
    {
        m_imageOverlay->setVisible(false);

        delete m_imageOverlay;
        m_imageOverlay = NULL;
    }

    if(m_textOverlay != NULL)
    {
        m_textOverlay->setVisible(false);

        delete  m_textOverlay;
        m_textOverlay = NULL;
    }
}

void MainWidget::handleKvmMsg(bool enable)
{
    // ??????OSD????????????????????????????????????????????????
    if(!enable)
    {
        m_bKvmMode = false;

        // hide overlay & deviceInfo
        if(!g_bDeviceSleepMode)
            slotHideDeviceInfo(false);

        slotHideOverlay();

        m_osdMeun->parseMeunJson(MENUINFO_PATH);
        m_osdMeun->parseChannelJson();
        m_osdMeun->setListWidgetHeight();
        m_osdMeun->setMeunFont();

        if(!g_bDeviceSleepMode)
        {
            m_bMoveMeunFlag = true;
            QTimer::singleShot(200,this,SLOT(moveDisplayArea()));
        }

        QTimer::singleShot(500,this,SLOT(showOsdMeun()));

        // ????????????????????????id, ?????????????????????????????????
        QString strCmd = QString("#KDS-CHANNEL-SELECT? video\r");
        QByteArray byteCmd = strCmd.toLatin1();

        qDebug() << "channel_byteCmd:" << byteCmd;
        if(m_p3kTcp->sendCmdToP3k(byteCmd))
        {
            m_isSnedQueryCmd = true;
            qDebug("Send Get ChannelID CMD Yes");
        }
    }
}

void MainWidget::slotUpdateDeviceInfo(QLabel *info)
{
//    m_deviceInfo->setText(info->text());

//    QFont font;
//    int fontsize = 15 * ((float)g_nScreenWidth/g_nStdScreenWidth);
//    font.setPointSize(fontsize);
//    font.setBold(true);
//    font.setWeight(50 * g_fScaleScreen);

//    m_deviceInfo->setFont(font);

//    update();
}

void MainWidget::startSleepMode()
{
  //  g_bDeviceSleepMode = true;
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

    // ?????????OSD???????????????????????????
    //    initOsdMeun();
    //    m_panelStack->addWidget(m_osdMeun);

    // ??????????????????
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
    n_bFirstInfo = false;
    if(m_osdMeun->getDeviceInfoDisplayStatus() && m_osdMeun->getdisplayConfig())
    {
        showDeviceInfo();
        n_bFirstInfo = true;
    }
    else
    {
        if(DeviceInfoTimer.isActive())
        {
            showDeviceInfo();
            n_bFirstInfo = true;
        }

    }

    //????????????overlay??????
    m_bFirst = true;
    parseOverlayJson("overlay1_setting.json");

    // ??????1???????????????????????????2
    if(!m_bFirst)
        return;

    parseOverlayJson("overlay2_setting.json");

    m_bFirst = false;
    n_bFirstInfo = false;
}

void MainWidget::initDeviceInfo()
{
    m_deviceInfo = m_sleepPanel->getDeviceInfo();

    QFont font;
    int fontsize = 15 * g_fScaleScreen;
    font.setPointSize(fontsize);

    m_deviceInfo->setFont(font);

    connect(&DeviceInfoTimer,SIGNAL(timeout()),this,SLOT(slotHideDeviceInfo()));
}

//  set show overlay
void MainWidget::slotShowOverlay()
{
    // ??????????????????
    QStringList paths ;

    paths << MENUINFO_PATH << CHANNELS_LIST_PATH << RESOLUTION_CONFIG << OVERLAY_TEXT_PATH << OVERLAY_IMAGE_PATH;

    watch = new QFileSystemWatcher;
    watch->addPaths(paths);
    connect(watch, SIGNAL(fileChanged(QString)), this, SLOT(syncConfig(QString)));

    initOverlay();
}

void MainWidget::slotHideOverlay()
{
    qDebug() << "overlay hide";
    if(m_imageOverlay != NULL)
    {
        m_imageOverlay->setVisible(false);
        m_imageOverlay->repaint();
    }
    else if(m_textOverlay != NULL)
    {
        m_textOverlay->setVisible(false);
        m_textOverlay->repaint();
    }
}


void MainWidget::hideOsdMeun(bool isStartOverlay)
{
    if(m_osdMeun == NULL)
        return;
    qDebug() << "Meun Hide";

    // ???????????????
    m_osdMeun->stopTimer();
    m_osdMeun->setVisible(false);
    m_osdMeun->repaint();
    g_bOSDMeunDisplay = false;

    // ?????????????????????KM
    freeKMControl();

    // ??????OSD??????????????????????????????Overlay
    if(isStartOverlay)
        QTimer::singleShot(400,this,SLOT(showLongDisplay()));
}

void MainWidget::showOsdMeun()
{
    if(m_osdMeun == NULL)
        return;

    m_osdMeun->setVisible(true);
    g_bOSDMeunDisplay = true;

    getKMControl();

    // ??????????????????????????????
    m_osdMeun->activateWindow();
    m_osdMeun->raise();

    moveOsdMeun(m_osdMeun->getShowPosition());
}

void MainWidget::moveOsdMeun(int position)
{
    int xpos = 0;
    int ypos = 0;

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
            ypos = g_nframebufferHeight-m_osdMeun->height();
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
            ypos = g_nframebufferHeight-m_osdMeun->height();
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

    m_osdMeun->move(xpos,ypos);

    // ?????????????????????
    moveFramebuffer(position);
    setTransparency(20);

    qDebug() << "m_osdMeun::width:" << m_osdMeun->width()
             << "m_osdMeun::height:" << m_osdMeun->height()
             << "g_fScaleScreen:" << g_fScaleScreen;

    m_osdMeun->startTimer();
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
    // info???????????????????????????overlay ?????????:meun > info > overlay
    if((m_osdMeun != NULL) &&(m_osdMeun->getdisplayConfig()) &&(m_osdMeun->getDeviceInfoDisplayStatus()))
    {
        if(!g_bDeviceSleepMode)
        {
            showDeviceInfo();
        }
        return;
    }


   if(m_imageOverlay != NULL)
   {
       if(m_imageOverlay->isLongDisplay())
       {
           qDebug() << "showLongDisplay::m_imageOverlay";
           showOverlay(m_imageOverlay,m_imageOverlay->getShowPos());
           return;
       }
   }
   else if(m_textOverlay != NULL)
   {
       if(m_textOverlay->isLongDisplay())
       {
           qDebug() << "showLongDisplay::m_textOverlay";
           showOverlay(m_textOverlay,m_textOverlay->getShowPos());
           return;
       }
   }
}

void MainWidget::hideOsdAndInfo()
{
    // ???????????? & ????????????
    hideOsdMeun(false);

    if(!g_bDeviceSleepMode)
        slotHideDeviceInfo(false);

    update();
}


void MainWidget::showOverlay(OSDLabel *overlay,int position)
{
    qDebug() << "showOverlay";
    // ???????????? & ????????????
//    hideOsdMeun(false);

//    if(!g_bDeviceSleepMode)
//        slotHideDeviceInfo(false);

     int xpos = 0;
     int ypos = 0;

     qDebug() << "main_2_2";

     OSDLabel *text = overlay;
     text->adjustSize();
     text->setVisible(true);

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
             ypos = g_nframebufferHeight-text->height()- OSD_YPOS;
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

     if(m_background)
     {
         m_background->setVisible(false);
     }

     // ???????????????????????????
//     if(!text->isLongDisplay())
//     {
//        qDebug() << "start timer:";
//        text->setoverTime();
//        text->startTimer();
//     }

   //  ??????????????????QOSDLabel??????
    qDebug() << "main_2_3";
}

// ??????frame buffer?????????
void MainWidget::moveFramebuffer(int align)
{
    QString Cmdstr = QString("ast_send_event -1 \"e_osd_position::%1::0::0\"").arg(align);
    qDebug() << "move Cmdstr:" <<Cmdstr;
    QProcess p;
    p.start("bash", QStringList() <<"-c" << Cmdstr);
    if(p.waitForFinished())
    {
        qDebug() << "move frame buffer finished";
    }
}

void MainWidget::setTransparency(int Transparency)
{
    Transparency = 100 - Transparency;
    Transparency = ((float)((Transparency*1.0)/100)*31);

    QString Cmdstr = QString("ast_send_event -1 \"e_osd_on_pic::0::9999::%1::1::n:: ::0\"").arg(Transparency);
    qDebug() << "setTransparency Cmdstr:" <<Cmdstr;
    QProcess p;
    p.start("bash", QStringList() <<"-c" << Cmdstr);
    if(p.waitForFinished())
    {
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

    jsonpath = "/data/configs/kds-7/overlay/" + jsonpath;
    QByteArray path = jsonpath.toLatin1();
    qDebug() << "path:" <<path;

    // ??????
    CFileMutex lock(path);
    lock.Init();
    lock.Lock();
#if 1
    // ??????????????????
    //string path = jsonpath.toStdString();
    ifstream in(path,ios::binary);

    if(!in.is_open())
    {
        qDebug() << "open overlay.json failed";
        return;
    }

    if(reader.parse(in,root))
    {
        // ?????????????????????

        // ?????????????????????
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

        // ???????????????0???????????????
        m_CmdOuttime = 0;

        m_Transparency = Transparency;
        qDebug() << "m_Transparency" << m_Transparency;

        cout << "Enable:" << Enable << endl;
        cout << "Timeout:" << Timeout << endl;
        cout << "Transparency:" << Transparency << endl;

        float Transparency_set = ((float)((100 - m_Transparency)*1.0)/100);
        qDebug() << "Transparency_set:" << Transparency_set;

        g_Transparency = (float)(((100 - m_Transparency)*1.0)/100)*255;
        qDebug() << "g_Transparency:" << g_Transparency;

        // ??????????????????
        if(root["objects"].isArray())
        {
            for(int i=0; i < root["objects"].size(); i++)
            {
                string type = root["objects"][i]["type"].asString();
                if(type.compare("image") == 0)
                {
                    // off????????????
                    if(!m_overlayStatus)
                    {
                        if(m_imageOverlay != NULL)
                        {
                            m_imageOverlay->setVisible(false);

                            delete m_imageOverlay;
                            m_imageOverlay = NULL;
                        }
                        in.close();
                        lock.UnLock();
                        return;
                    }

                    string position = root["objects"][i]["position"].asString();
                    int width = root["objects"][i]["width"].asInt();
                    int height = root["objects"][i]["height"].asInt();
                    string path = root["objects"][i]["path"].asString();

                    cout << "position:" << position << endl;
                    cout << "width:" << width << endl;
                    cout << "height:" << height << endl;
                    cout << "path:" << path << endl;

                    // ????????????
                    QString posStr = position.c_str();
                    int showPos = parseOverlayPos(posStr);

                    QString filepath = path.c_str();

                    // ?????????????????????
                    if(m_imageOverlay != NULL)
                    {
                        m_imageOverlay->setVisible(false);

                        delete m_imageOverlay;
                        m_imageOverlay = NULL;
                    }

                    // ?????? overlay
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

                    // ??????
                    if(m_textOverlay != NULL)
                    {
                        m_textOverlay->setVisible(false);

                        delete m_textOverlay;
                        m_textOverlay= NULL;
                    }

                    if(m_bFirst)
                    {
                        // ??????
                        if(m_overlayStatus && (m_CmdOuttime == 0))
                        {
                            m_bFirst = false;

                            if(!n_bFirstInfo)
                                showOverlay(m_imageOverlay,showPos);
                        }
                    }
                    else
                    {
                        // ??????
                        if(m_overlayStatus)
                        {
                            hideOsdAndInfo();
                            if(!g_bDeviceSleepMode)
                            {
                                QTimer::singleShot(200,this,SLOT(moveDisplayArea()));
                            }

                            QTimer::singleShot(500,this,SLOT(slotShowImageOverlay()));
                        }
                    }
                }
                else if(type.compare("text") == 0)
                {
                    // off????????????
                    if(!m_overlayStatus)
                    {
                        if(m_textOverlay != NULL)
                        {
                            m_textOverlay->setVisible(false);

                            delete m_textOverlay;
                            m_textOverlay = NULL;
                        }
                        in.close();
                        lock.UnLock();
                        return;
                    }

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

                    // ????????????
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

                    // ?????????????????????

                    if(m_textOverlay != NULL)
                    {
                        m_textOverlay->setVisible(false);

                        delete m_textOverlay;
                        m_textOverlay = NULL;
                    }

                    // ?????? overlay
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

                    // ??????
                    if(m_imageOverlay != NULL)
                    {
                        m_imageOverlay->setVisible(false);

                        delete  m_imageOverlay;
                        m_imageOverlay = NULL;
                    }

                    if(m_bFirst)
                    {
                        // ????????????
                        if(m_overlayStatus && (m_CmdOuttime == 0))
                        {
                            m_bFirst = false;

                            if(!n_bFirstInfo)
                                showOverlay(m_textOverlay,showPos);
                        }
                    }
                    else
                    {
                        // ??????
                        if(m_overlayStatus)
                        {
                            hideOsdAndInfo();
                            if(!g_bDeviceSleepMode)
                            {
                                QTimer::singleShot(200,this,SLOT(moveDisplayArea()));
                            }
                            QTimer::singleShot(500,this,SLOT(slotShowTextOverlay()));
                            //showOverlay(m_textOverlay,showPos);
                        }
                    }
                }
            }
        }
    }
    else
    {
        std::cout << "reader error:" << reader.getFormatedErrorMessages() << std::endl;
    }
    in.close();
    // ??????
    lock.UnLock();

#endif
}

void MainWidget::moveDisplayArea()
{
    if(m_bMoveMeunFlag)
    {
        qDebug() << "MOVE m_bMoveMeunFlag BACKGROUND";
        m_bMoveMeunFlag = false;
        showOverlay(m_background,m_osdMeun->getShowPosition());
        return;
    }

    if(m_bMoveInfoFlag)
    {
        qDebug() << "MOVE m_bMoveInfoFlag BACKGROUND";
        m_bMoveInfoFlag = false;
        showOverlay(m_background,BOTTOM_RIGHT);
        return;
    }

    if(m_textOverlay == NULL)
    {
        qDebug() << "MOVE m_textOverlay BACKGROUND";
        if(m_imageOverlay)
            showOverlay(m_background,m_imageOverlay->getShowPos());
    }
    else if(m_imageOverlay == NULL)
    {
        qDebug() << "MOVE m_imageOverlay BACKGROUND";
        if(m_textOverlay)
            showOverlay(m_background,m_textOverlay->getShowPos());
    }
}

void MainWidget::slotShowImageOverlay()
{
    if(m_imageOverlay != NULL)
    {
        showOverlay(m_imageOverlay,m_imageOverlay->getShowPos());
    }
}

void MainWidget::slotShowTextOverlay()
{
    if(m_textOverlay != NULL)
    {
        showOverlay(m_textOverlay,m_textOverlay->getShowPos());
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
    // ??????????????????????????????
    QImage image;
    QImage orderImage;
    QImage saveImage;
    QImageReader reader;

    reader.setFileName(picpath);
    QSize imageSize = reader.size();
    imageSize.scale(this->width(),this->height(),Qt::IgnoreAspectRatio);
    reader.setScaledSize(imageSize); // ????????????????????????
    image = reader.read();

    int column = 4;
    int row = 4;
    int out_id = 5;
    int rotation = 0;

    column = m_nVideoWall_V;
    row = m_nVideoWall_H;

    out_id = m_nVideoWall_ID;
    rotation = m_nVideoWall_R;

    // ???????????????????????????, ????????????
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
        // ??????????????????
        int R = (out_id % column) ? (out_id/column + 1) : (out_id/column);
        int C = (out_id % column) ? (out_id%column) : column;
        qDebug() << "orderArea R:" << R;
        qDebug() << "orderArea C:" << C;


        // ????????????
        hpos = this->width()/column * (C - 1);
        vpos = this->height()/row * (R - 1);
        qDebug() << "orderArea hpos:" << hpos;
        qDebug() << "orderArea vpos:" << vpos;
     }

    // ??????????????????
    orderImage = image.copy(hpos,vpos,width,height);

    // ??????
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















