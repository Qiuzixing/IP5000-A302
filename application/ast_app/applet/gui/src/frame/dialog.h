/*
 * Copyright (c) 2004-2012 ASPEED Technology Inc.
 * Based on:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QVBoxLayout>
#include <QtDebug>
#include <QLabel>
#include <QProcess>
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtNetwork/QTcpSocket>
#include <QKeyEvent>
#include <QScreenCursor>
#include <QPushButton>
#include <QHostAddress>
#include <QFile>
#include <QCompleter>
#include <QItemSelectionModel>
#include <QStringListModel>
#include <QCursor>
#include <QTimer>
#include <QTabWidget>
#include <QDesktopWidget>
#include <QMenu>
#include <QMenuBar>
#include <QImage>
#include <QPixmap>
#include "../../msgd/msgd.h"

#define ENABLE_OPTION_TAB 0

const QByteArray BARUI = "-ui_feature";
const QByteArray BAROPTION = "-option";
const QByteArray BARHOSTNAME_PREFIX = "-hostname_prefix";
const QByteArray BARMULTICAST_IP_PREFIX = "-multicast_ip_prefix";
const QByteArray BARNLPROGNAME = "/usr/local/bin/run_list.sh";
const QByteArray BARFILENAME = "nodes.txt";
const QByteArray BAREOF = "<<<<<";
const QByteArray BARERROR = "file_read_error";
const QByteArray BARTITLEHEAD = "name:";
const QByteArray BARASTPARAM = "/usr/local/bin/astparam";
const QByteArray BARSAVE = "s";
const QByteArray BARCHSELECT = "ch_select";
const QByteArray BARMULTICAST_IP = "multicast_ip";
const QByteArray BARASTSENDEVENT = "/usr/local/bin/ast_send_event";
const QByteArray BARDASHONE = "-1";
const QByteArray BARRECONNECT = "e_reconnect";
const int RUNNODEWAITINTERVAL = 4 * 1000;//1000 is 1 second
const int FILEWAITINTERVAL = 1 * 1000;//1000 is 1 second
#define V_OVERSCANBORDER(height) ((int)((height)*5/100/2))
#define H_OVERSCANBORDER(width)  ((int)((width)*17.5/100/2))
const QString NODEDIALOGTITLE = "Host Selection";
const QString COMBOBOXNAME = "Node List:";

/*
** UI option defines
*/
#define UIO_CONVERT_TO_HEX   (1 << 0)

class NodeDialog; //forward declaration

namespace Ui {
    class Dialog;
}

class Dialog : public QDialog {
    Q_OBJECT
public:
    Dialog(QWidget *parent = 0);
    ~Dialog();
    void setSelectedNode(QString);
    void setSleepPicture(char *path);
    void setDeviceInfoStatus(bool status){m_bDeviceInfoDispaly = status;}
    QLabel* getDeviceInfo();

signals:
    void sigStartSleepMode();
    void sigStartKVM(bool enable);
    void sigUpdateDeviceInfo(QLabel* info);

protected:
    void changeEvent(QEvent*);
    bool eventFilter(QObject*, QEvent*);
    void showEvent ( QShowEvent*);
private:
    Ui::Dialog *ui;
    bool bNodeDlgHidden;
    QStringList qslArg;
    QStringList qslNodeList;
    QString qsSelected;
    struct static_info sInfo;
    struct runtime_info rInfo;
    struct info_hdr hdr;
    struct osd_info osdInfo;
    struct gui_action_info guiActionInfo;
    QTcpSocket *tcpSocket;
    NodeDialog *dlgNode;
    QLabel *infoL;

    QLabel *infoR;
    QLabel *infoC;
    QHBoxLayout *lay_bottom;
    QVBoxLayout *layout;
    QVBoxLayout *overScanV;
    QHBoxLayout *overScanH;
    QVBoxLayout *backgroundV;
    QHBoxLayout *backgroundH;
    QHBoxLayout *lay_OSD;
    QCursor cursor;
    QLabel *infoOSD;
    QFont OSDFont;
    QPalette OSDPalette;
	int screen_width;
	int screen_height;
	unsigned int option;
	QString picture_name;

	QByteArray BARHOSTNAMEHEAD;
    QByteArray MULTICAST_IP_PREFIX;

    bool m_bDeviceInfoDispaly;

    QStringList readNodesFromFile(void);
    QStringList parseNodes(void);
    void updateInfoOSD(void);
    void setNodeDialogVisible(bool);
    void ConnectByEvent(void);
    void SetComponentWaitForNodeData(bool);
    void runNodeList(void);
    void StartMsgDConnection(void);
    void dbgMsg(const QString&);
    void Connect2MsgD(void);
    void parseMsg(QByteArray&, unsigned int);
    void updateInfoR(void);
    void updateInfoL(void);
    void updateGUI(void);
    bool changeTargetHost(QString);
    bool reconnectToHost(QString);
	void paintEvent(QPaintEvent*);

private slots:
    void readMsgD(void);
    void connectedMsgD(void);
    void displayMsgDErr(QAbstractSocket::SocketError socketError);
    bool setupNodeSelectComboBox(void);
    void disconnectedMsgD(void);//socket signal disconnected doesn't seem to emit at all

public slots:
    void switchDialog(QWidget*);
    void initNodeList(void);
};

/*
class LinkTab : public QWidget
{
     Q_OBJECT

public:
     LinkTab(QWidget *parent = 0);
     ~LinkTab();
     QComboBox *cbNodeSelect;
     QPushButton *btOk;
     QPushButton *btRefresh;
     QHBoxLayout *tabLayout;
signals:
    void editTextChanged(QString);
    void activated(QString);
};
*/

class NodeDialog : public QDialog
 {
     Q_OBJECT

 public:
     NodeDialog(QWidget *parent = 0);
     ~NodeDialog();
 private slots:
     void on_cbNodeSelect_editTextChanged(QString );
     void on_cbNodeSelect_activated(QString );

 public:
     QVBoxLayout *mainLayout;
     //QHBoxLayout *mnuLayout;
     QHBoxLayout *topLayout;
     QHBoxLayout *centerLayout;
     QVBoxLayout *centerLayout2;
     QHBoxLayout *bottomLayout;
     QComboBox *cbNodeSelect;
     QPushButton *btConnect;
     QPushButton *btRefresh;
     QPushButton *btOption;
     QLabel *lb;
 private:
     QRect scr;
     Dialog *myParent;
     //QPixmap pmIcon;
     //QMenu *mnuOption;
     //QMenuBar *mnuBar;
 protected:
     //bool eventFilter(QObject*, QEvent*);
     void moveEvent(QMoveEvent*);
     bool eventFilter(QObject*, QEvent*);
     void showEvent ( QShowEvent*);
 signals:
     void optionSelected(QWidget*);
     void dialogReady(void);
 };
#endif // DIALOG_H
