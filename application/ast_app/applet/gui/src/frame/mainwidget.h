#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QKeyEvent>
#include <QStackedWidget>
#include <QTcpSocket>
#include <QHBoxLayout>
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

    void showLongDisplay();
    void setOsdDispaly(bool status);

    void switchOSDMeun();

    void getResolutionFromTiming();
    bool p3kconnected();
public slots:
    void hideOsdMeun();
    void showOsdMeun();
    void moveOsdMeun(int position);
    void updateOsdMenu();

    void slotShowOverlay();
    void slotHideOverlay();

    void isNoSignal();

    void startSleepMode();
    void handleKvmMsg(bool enable);

    void syncConfig(QString);

    void getKMControl();
    void freeKMControl();

    void segmentationPic(QString picpath);

    void onRecvData(QByteArray data);
    void parseCmdResult(QByteArray datagram);

    void parseOverlayJson(QString jsonpath);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);

    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    void initOsdMeun();
    void initPanelStack();

private:
    QStackedWidget  *m_panelStack;
    Dialog          *m_sleepPanel;

    QLabel *imageLabel;
    OSDMeun *m_osdMeun;

    OSDLabel *m_imageOverlay;
    OSDLabel *m_textOverlay;

    QLabel *infoL;
    QLabel *infoR;

    static QMap<int, OSDLabel *> g_overlayMap;
    static QSet<int> osdIdSet;

    QTimer osdMeunTimer;
    QTimer getInfoTimer;

    QFileSystemWatcher *watch;

    int m_Transparency; // 透明度
    int m_CmdOuttime;

    UdpRecvThread* UdpRecv;
    P3ktcp *m_p3kTcp;

    QTcpSocket *m_tcpSocket;
    bool m_bKvmMode;

    int m_nVideoWall_H;
    int m_nVideoWall_V;

    int m_nVideoWall_ID;
    int m_nVideoWall_R;
};

#endif // MAINWIDGET_H
