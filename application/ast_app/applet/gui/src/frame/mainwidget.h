#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QKeyEvent>
#include <QStackedWidget>
#include <QTcpSocket>
#include <QHBoxLayout>
#include <QReadWriteLock>
#include <QGraphicsOpacityEffect>
#include "osdlabel.h"
#include "p3ktcp.h"

#include "modules/sleepmode/sleeppanel.h"

#define STR_BUF_SIZE 64
#define MAX_STR_LEN (STR_BUF_SIZE - 1)
#define V_OVERSCANBORDER(height) ((int)((height)*5/100/2))
#define H_OVERSCANBORDER(width)  ((int)((width)*17.5/100/2))

class QFileSystemWatcher;
class QTimer;
class QLabel;
class OSDMeun;
class UdpRecvThread;
class Dialog;


class MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

    void showOverlay(OSDLabel *overlay,int position);
    int parseOverlayPos(QString positon);

    void moveFramebuffer(int align);
    void setTransparency(int Transparency);


    void setDeviceInfoDispaly(bool status);
    void showDeviceInfo();

    void switchOSDMeun();

    bool p3kconnected();
public slots:  
    void getResolutionFromTiming();
     void showLongDisplay();

    void hideOsdMeun(bool isStartOverlay = true);
    void showOsdMeun();
    void moveOsdMeun(int position);
    void updateOsdMenu();

    void reInit();

    void slotShowOverlay();
    void slotHideOverlay();

    void destroyOsdAndOverlay();

    void startSleepMode();
    void handleKvmMsg(bool enable);

    void syncConfig(QString);

    void getKMControl();
    void freeKMControl();

    void segmentationPic(QString picpath);

    void onRecvData(QByteArray data);
    void parseCmdResult(QByteArray datagram);

    int parseKMJsonGetTimeout();

    void parseOverlayJson(QString jsonpath);

    void parseOverlayText();
    void parseOverlayImage();
    void updateOsdMeun();
    void updateChannelList();

    void slotUpdateDeviceInfo(QLabel *info);
    void slotHideDeviceInfo(bool isStartOverlay = true);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);

    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    void initOsdMeun();
    void initOverlay();
    void initPanelStack();
    void initDeviceInfo();

private:
    QStackedWidget  *m_panelStack;
    Dialog          *m_sleepPanel;

    QLabel *imageLabel;
    OSDMeun *m_osdMeun;

    OSDLabel *m_imageOverlay;
    OSDLabel *m_textOverlay;

    QLabel *infoL;
    QLabel *m_deviceInfo;

    static QMap<int, OSDLabel *> g_overlayMap;
    static QSet<int> osdIdSet;

    QTimer osdMeunTimer;
    QTimer getInfoTimer;
    QTimer DeviceInfoTimer;

    QFileSystemWatcher *watch;

    int m_Transparency; // 透明度
    int m_CmdOuttime;
    bool m_overlayStatus;

    bool m_bFirst;
    bool m_isSnedQueryCmd;

    UdpRecvThread* UdpRecv;
    P3ktcp *m_p3kTcp;

    QTcpSocket *m_tcpSocket;
    bool m_bKvmMode;

    int m_nVideoWall_H;
    int m_nVideoWall_V;

    int m_nVideoWall_ID;
    int m_nVideoWall_R;

    QReadWriteLock m_lock;
    QGraphicsOpacityEffect *m_opacityEffect;
};

#endif // MAINWIDGET_H
