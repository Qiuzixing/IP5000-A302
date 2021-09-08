#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QKeyEvent>
#include <QStackedWidget>
#include <QTcpSocket>
#include "osdlabel.h"

#include "modules/sleepmode/sleeppanel.h"

class QFileSystemWatcher;
class QTimer;
class QLabel;
class OSDMeun;
class UdpRecvThread;

struct info_hdr {
    unsigned int type;
    unsigned int data_len;
    char data[];
};

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

public slots:
    void hideOsdMeun();
    void showOsdMeun();
    void moveOsdMeun(int position);
    void updateOsdMenu();

    void slotShowOverlay();
    void slotHideOverlay();

    void isNoSignal();

    void startSleepMode(ModulePanel *panel);

    void syncConfig(QString);

    void getKMControl();
    void freeKMControl();

    void segmentationPic(QString picpath);

    void onRecvData(QByteArray data);

    void parseOverlayJson(QString jsonpath);

    void connectedMsgD();
    void readMsgD();
    void disconnectedMsgD();
    void displayMsgDErr(QAbstractSocket::SocketError socketError);
protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);

    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    void initOsdMeun();
    void initPanelStack();
    void initOverlayLabelConnect(OSDLabel *label);
    void StartMsgDConnection();

private:
    QStackedWidget  *m_panelStack;
    SleepPanel *m_sleepPanel;

    QLabel *imageLabel;
    OSDMeun *m_osdMeun;

    OSDLabel *m_imageOverlay;
    OSDLabel *m_textOverlay;

    static QMap<int, OSDLabel *> g_overlayMap;
    static QSet<int> osdIdSet;

    QTimer osdMeunTimer;
    QTimer getInfoTimer;

    QFileSystemWatcher *watch;

    int m_Transparency; // 透明度
    int m_CmdOuttime;

    UdpRecvThread* UdpRecv;

    struct info_hdr hdr;
    QTcpSocket *m_tcpSocket;
};

#endif // MAINWIDGET_H
