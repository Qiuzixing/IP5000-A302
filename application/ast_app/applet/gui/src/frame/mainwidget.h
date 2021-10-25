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

struct info_hdr {
    unsigned int type;
    unsigned int data_len;
    char data[];
};

struct static_info {
    char IP[STR_BUF_SIZE];
    char FW[STR_BUF_SIZE];
    char RemoteIP[STR_BUF_SIZE];
    char ID[STR_BUF_SIZE];
};

struct rinfo_hidden {
    int     hidden;  // 1:hide,0:not hide
};

struct runtime_info {
    char str[STR_BUF_SIZE];
};

struct gui_show_info {
    unsigned int show_text;		//1: show, 0: hide
    char picture_name[STR_BUF_SIZE];
};

struct gui_action_info {
  unsigned int action_type;
  union {
    char buf[STR_BUF_SIZE];
    unsigned int show_dialog; //1: show, 0: hide
    unsigned int refresh_node; //1: refresh, 0: no refresh
    struct gui_show_info show_info;
  } ub;
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
    void parseCmdResult(QByteArray datagram);

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
    void initInfoLandIndoR();
    void parseMsg(QByteArray &data, unsigned int type);
    void updateInfoR();
    void updateInfoL();
    void updateGUI();

private:
    QStackedWidget  *m_panelStack;
    SleepPanel *m_sleepPanel;

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

    struct static_info sInfo;
    struct info_hdr hdr;
    struct runtime_info rInfo;
    struct rinfo_hidden rinfoHidden;
    struct gui_action_info guiActionInfo;
    bool bRinfoDlgHidden;

    QTcpSocket *m_tcpSocket;
    bool m_bKvmMode;

    int m_nVideoWall_H;
    int m_nVideoWall_V;

    int m_nVideoWall_ID;
    int m_nVideoWall_R;
};

#endif // MAINWIDGET_H
