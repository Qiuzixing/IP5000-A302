#include <QtGui/QApplication>
#include "dialog.h"
//#include "mainwindow.h"
#include "frame/mainwidget.h"
#include <QtPlugin>
#include <QProcess>
#include <QDebug>
#include "common/scale_global.h"
#include "common/global.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QWSServer>

int     g_nScreenWidth  = 1920;
int     g_nScreenHeight = 1080;
float   g_fScaleScreen  = 1.0;

bool g_bDeviceSleepMode = true;
bool g_bOSDMeunDisplay = false;
int g_nChannelId = -1;


void CalcScreenScale(int nWidth, int nHeight, float &fScale)
{
    float wScale = static_cast<float> (nWidth) / g_nStdScreenWidth;
    float hScale = static_cast<float> (nHeight) / g_nStdScreenHeight;
    fScale = qMin(wScale, hScale);
}

void getResolutionFromTiming()
{
    QString strCmd = "cat /sys/devices/platform/videoip/timing_info";
    QProcess p;
    p.start("bash", QStringList() <<"-c" << strCmd);
    p.waitForFinished();
    QString strResult = p.readAllStandardOutput();

    qDebug() << "strResult:" << strResult;
    QString NoSignals = "Not Available";
    if(strResult.startsWith("Not") || strResult.isEmpty())
    {
        // 当前没有输出，重启软件进入休眠模式，设定OSD软件分辨率为720P
        g_bDeviceSleepMode = true;
        qDebug() << "g_bDeviceSleepMode:" << g_bDeviceSleepMode;
        qDebug() << "SLEEP";
        QRect screen = QApplication::desktop()->screenGeometry(0);
        g_nScreenWidth = screen.width();
        g_nScreenHeight = screen.height();

        return;
    }

    // 解析输出源的分辨率应用到OSD
     QString startStr = "Timing Table:";
     QString endStr = "Pixel Rate:";

     int spos = strResult.indexOf(startStr);
     QString tmpStr = strResult.mid(spos);

     int epos = strResult.indexOf(endStr);
     tmpStr = tmpStr.left(epos);

     QStringList list = tmpStr.split(" ");
     qDebug() << "list:" << list;

    QString orderStr = list.at(4);
    if(orderStr.startsWith("["))
    {
        list.clear();
        list = orderStr.split("X");

        QString widthStr = list.at(0).mid(1,list.at(0).length()-2);
        int width = widthStr.toInt();
        qDebug() << "widthStr:" << widthStr;

        QString heightStr = list.at(1).mid(1,list.at(1).length()-2);
        int height = heightStr.toInt();
        qDebug() << "heightStr:" << heightStr;

        g_nScreenWidth = width;
        g_nScreenHeight = height;
        g_bDeviceSleepMode = false;
    }

    return;
}


int main(int argc, char *argv[])
{
    Q_IMPORT_PLUGIN(qjpeg)

    QApplication a(argc, argv);
    //For static link program, Q_INIT_RESOURCE is a must
    Q_INIT_RESOURCE(resource);
    qDebug() << "BEGIN:";

#ifdef Q_OS_LINUX
    QWSServer::setCursorVisible(false);
#endif

    //  获取分辨率，设定工作模式
    getResolutionFromTiming();
    CalcScreenScale(g_nScreenWidth, g_nScreenHeight, g_fScaleScreen);    

#ifdef Q_OS_LINUX
    QWSServer::setBackground(QColor(0,0,0));
#endif


    // Dialog w;
    MainWidget w;
    w.p3kconnected();

    //QFont fnt(QApplication::font());
    //fnt.setPointSize(8);
    //w.setFont(fnt);


    qDebug() << "g_nScreenWidth:" << g_nScreenWidth
             << "g_nScreenHeight:" << g_nScreenHeight
             << "g_fScaleScreen:" <<g_fScaleScreen;

    w.show();

    // 重启
    int retval = a.exec();
    if (RESTART_APP == retval)
    {
        qDebug() << "restart mainosd.";
        QStringList args;
#ifdef Q_OS_LINUX
        args << "-qws";
#endif
        QProcess::startDetached(qApp->applicationFilePath(), args);
    }

    return retval;
}




