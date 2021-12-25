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
#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent, Qt::CustomizeWindowHint | Qt::FramelessWindowHint),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    QRect screen=QApplication::desktop()->screenGeometry(0);
    screen_width = screen.width();
    screen_height = screen.height();
    printf("Width x Height:%d x %d\n",screen_width, screen_height);
    setFixedSize(screen_width, screen_height);

    // SLEEP图片
    picture_name.clear();
    picture_name.append("");

    // 启动参数
    qslArg = QApplication::arguments();

    option = 0;
    if (qslArg.contains(QString(BAROPTION), Qt::CaseInsensitive)) {
        bool ok;
        int idx = 1 + QApplication::arguments().indexOf(QString(BAROPTION));
        option = QApplication::arguments().at(idx).toUInt(&ok, 16);
        qDebug() << "UI_OPTION:" << option;
    }

    if (qslArg.contains(QString(BARUI), Qt::CaseInsensitive))
    {
        //Support hostname_prefix
        if(QApplication::arguments().contains(QString(BARHOSTNAME_PREFIX), Qt::CaseInsensitive)) {
            int idx = 1 + QApplication::arguments().indexOf(QString(BARHOSTNAME_PREFIX));
            BARHOSTNAMEHEAD.insert(0, QApplication::arguments().at(idx));
        } else {
            //Use default hostname_prefix
            BARHOSTNAMEHEAD.insert(0, "ast");
        }
        /* Bruce170509. -Tx- is now contained in BARHOSTNAME_PREFIX. */
        /*BARHOSTNAMEHEAD.append("-Tx-");*/

        //Bruce121001. Support multicast_ip_prefix
        if(QApplication::arguments().contains(QString(BARMULTICAST_IP_PREFIX), Qt::CaseInsensitive)) {
            int idx = 1 + QApplication::arguments().indexOf(QString(BARMULTICAST_IP_PREFIX));
            MULTICAST_IP_PREFIX.insert(0, QApplication::arguments().at(idx));
        } else {
            //Use default multicast_ip
            MULTICAST_IP_PREFIX.insert(0, "225.0.10");
        }

#if 0//todo, implement keyboard input to auto search, using QStringListModel
        QCompleter *pCompleter = new QCompleter(this);
        QStringListModel *pModel = new QStringListModel();
        pModel->setStringList(this->qslNodeList);
        pCompleter->setModel(pModel);
        pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        pCompleter->setCompletionMode(QCompleter::PopupCompletion);
        cbNodeSelect->setCompleter(pCompleter);
#endif
        this->installEventFilter(this);
        dlgNode = new NodeDialog(this);
        dlgNode->cbNodeSelect->installEventFilter(this);
        dlgNode->btConnect->installEventFilter(this);
        dlgNode->btRefresh->installEventFilter(this);

        //cursor.setShape(Qt::ArrowCursor); //Qt::WaitCursor
        //QApplication::setOverrideCursor(QCursor(cursor));
        //dlgNode->show();
        qDebug() << "GUI feature on";
    }
    else
    {
        qDebug() << "GUI feature off";
        dlgNode = NULL;
    }
    strcpy((char*)sInfo.FW, "Unknown");
    strcpy((char*)sInfo.IP, "Unknown");
    strcpy((char*)sInfo.RemoteIP, "Unknown");
    strcpy((char*)sInfo.ID, "Unknown");
    //bConnectedToHost = false;
    /* Connect to MsgD */
    StartMsgDConnection();

    qDebug() << "Connect to MsgD";

    infoL = new QLabel(this);
    infoL->setAlignment(Qt::AlignBottom);
    infoL->setText(QString("Initial..."));

    infoC = new QLabel(this);
    infoC->setAlignment(Qt::AlignBottom);
    infoC->clear();

    infoR = new QLabel(this);
    infoR->setAlignment(Qt::AlignBottom);
    infoR->setText(QString("FW: %1\nLocal IP: %2\nRemote IP: %3\nID: %4")
                   .arg((char*)sInfo.FW)
                   .arg((char*)sInfo.IP)
                   .arg((char*)sInfo.RemoteIP)
                   .arg((char*)sInfo.ID));
    /*
    ** Bruce150820. Hide infoX messages by default.
    ** Link manager control whether to show it or not.
    */

    infoL->setVisible(false);
    infoC->setVisible(false);
    infoR->setVisible(false);

    infoOSD = new QLabel(this);
    OSDFont.setPointSize(40);
    //OSDFont.setStyleStrategy(QFont::NoAntialias);
    OSDFont.setWeight(QFont::Bold);
    infoOSD->setFont(OSDFont);
    /* Set QPalette */
    OSDPalette.setColor(QPalette::WindowText, QColor(0xFF00FF00));
    OSDPalette.setColor(QPalette::Window, QColor(0xFF9D9B90));
    infoOSD->setPalette(OSDPalette);

    infoOSD->setAlignment(Qt::AlignHCenter);
    infoOSD->setText(QString(""));

    lay_bottom = new QHBoxLayout;
    lay_bottom->insertWidget(0, infoL, 0, Qt::AlignLeft);
    lay_bottom->insertWidget(1, infoC, 0, Qt::AlignHCenter);
    lay_bottom->insertWidget(2, infoR, 0, Qt::AlignRight);

    lay_OSD = new QHBoxLayout;
    lay_OSD->insertWidget(0, infoOSD, 0, Qt::AlignHCenter);

    layout = new QVBoxLayout;
    //layout->addStretch(1);
    layout->addLayout(lay_bottom, 0);

    overScanV = new QVBoxLayout;
    overScanH = new QHBoxLayout;
    overScanH->addSpacing(H_OVERSCANBORDER(screen_width));
    overScanH->addLayout(layout);
    overScanH->addSpacing(H_OVERSCANBORDER(screen_width));
    //overScanV->addSpacing(V_OVERSCANBORDER(screen_height));
    overScanV->addLayout(overScanH);
    overScanV->addSpacing(V_OVERSCANBORDER(screen_height));

    backgroundV = new QVBoxLayout(this);
    backgroundH = new QHBoxLayout;
    backgroundV->setContentsMargins(0, 0, 0, 0);
    backgroundV->addLayout(lay_OSD, 0);
    backgroundH->addLayout(overScanV);
    backgroundV->addLayout(backgroundH, 0);
    setLayout(backgroundV);
    setWindowFlags(Qt::SplashScreen);

    qDebug() << "setLayout finished";
}

Dialog::~Dialog()
{
    //delete ui;
    if (dlgNode != NULL)
        delete dlgNode;

    delete infoL;
    delete infoR;
    delete infoC;
    delete lay_bottom;
    delete layout;
    delete tcpSocket;
}

void Dialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Dialog::setSelectedNode(QString qstr)
{
    qsSelected = qstr;
}


// 该函数没有被实际应用
bool Dialog::changeTargetHost(QString qstr)
{
    QProcess ch_select, multicast_ip;
    QStringList arguments, arguments1;
    QChar *ch = qstr.data();
    //QByteArray m_ip = "225.0.";
    QByteArray m_ip = "";

    arguments << BARSAVE.data() << BARCHSELECT.data() << qstr;
    ch_select.start(BARASTPARAM.data(), arguments);
    if (!ch_select.waitForFinished())
        return false;

    //m_ip will be 225.0.10B0.B1B2B3
    m_ip.insert(0, MULTICAST_IP_PREFIX.data());
    m_ip.append(ch[0]);
    m_ip.append(".");
    m_ip.append(ch[1]);
    m_ip.append(ch[2]);
    m_ip.append(ch[3]);
    arguments1 << BARSAVE.data() << BARMULTICAST_IP.data() << m_ip.data();
    multicast_ip.start(BARASTPARAM.data(), arguments1);
    if (!multicast_ip.waitForFinished())
        return false;

    return true;
}

bool Dialog::reconnectToHost(QString qstr)
{
    QProcess reconnect;
    QStringList arguments;

    arguments << BARDASHONE << BARRECONNECT + "::" + qstr;
    reconnect.start(BARASTSENDEVENT.data(), arguments);

    if (reconnect.waitForFinished())
    {
        return true;
    }
    else
        return false;
}

void Dialog::runNodeList()
{
    QProcess qpNodeList;
    //QByteArray barRet;

    //nodeList.start(BARNLPROGNAME.data());
    //nodeList.waitForFinished();
    qpNodeList.startDetached(BARNLPROGNAME.data());
    QTimer::singleShot(RUNNODEWAITINTERVAL, this, SLOT(setupNodeSelectComboBox()));
    /*if (nodeList.waitForFinished())
        barRet = nodeList.readAllStandardOutput();
    else
        barRet = BARERROR;
    return barRet;*/
}

QStringList Dialog::readNodesFromFile(void)
{
    QFile file(BARFILENAME.data());
    QStringList result;

    result << QString(BARERROR);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTimer::singleShot(FILEWAITINTERVAL, this, SLOT(setupNodeSelectComboBox()));
        return result;
    }
    else
        result.clear();

    while (!file.atEnd())
    {
        result << file.readLine();
    }
    return result;
}

QStringList Dialog::parseNodes(void)
{
    //QByteArray bytes = in;
    QStringList lines;
    QStringList filtered;
    QStringList qslTemp;
    QStringList eofm;
    QString qstTemp;

    //qDebug() << "called by timer";
    lines << readNodesFromFile();

    if (lines.contains(QString(BARERROR), Qt::CaseInsensitive) )
    {
        //qDebug() << BARERROR << endl;
        return lines;
    }

    eofm = lines.filter(BAREOF.data());
    if (eofm.size() == 0 )
    {
        //No eof mark, means the list file is still generating
        //so set timer to read again
        qDebug() << "can't find oef mark";
        eofm << BARERROR;
        QTimer::singleShot(FILEWAITINTERVAL, this, SLOT(setupNodeSelectComboBox()));
        return eofm;
    }
    //filter out the lines that don't contain "ast-gateway"
    filtered = lines.filter(BARHOSTNAMEHEAD.data());

    //no host found, might be the output file not ready
    //need to set timer to read again
    if (filtered.size() == 0)
    {
        qDebug() << "nothing in nodes.txt, will need to read again.";
        lines << BARERROR;
        return lines;
    }

    //get rid of the unwanted words other than "ast-gatewayxxxx" in the lines
    for (int i = 0; i < filtered.size(); ++i)
    {
        qstTemp = filtered.at(i).toLocal8Bit().constData();
        lines.clear();
        lines = qstTemp.split(QRegExp("\\s+"));
        qslTemp << lines.filter(BARHOSTNAMEHEAD.data());
    }
    filtered.clear();
    filtered = qslTemp;
    //get rid of "ast-gateway" prefix, leave only 4 digits of host id
    filtered.replaceInStrings(QString(BARHOSTNAMEHEAD), QString(""), Qt::CaseInsensitive);
    //for (int i = 0; i < filtered.size(); ++i)
        //qDebug() << "Host=" << filtered.at(i).toLocal8Bit().constData();

    if (option & UIO_CONVERT_TO_HEX) {
        qslTemp.clear();
        for (int i = 0; i < filtered.size(); i++) {
            QString tmp;
            int value;
            bool ok;

            tmp = filtered.at(i).toLocal8Bit().constData();
            value = tmp.toInt(&ok, 2);
            /*
            ** Ignore strings other then 0000~1111.
            */
            if ((tmp.size() != 4)
                || (!ok)
                || (value < 0)
                || (value > 15)) {
                //Don't translate
                qslTemp << tmp;
            } else {
                //Got a valid value
                qslTemp << tmp.setNum(value, 16).toUpper();
            }
        }
        filtered.clear();
        filtered = qslTemp;
    }

    return filtered;
}

bool Dialog::setupNodeSelectComboBox(void)
{
    QStringList nodes;

    //runNodeList();
    nodes << parseNodes();
    //check previous execution of node_list for error
    if (nodes.contains(QString(BARERROR), Qt::CaseSensitive)) {
        SetComponentWaitForNodeData(false);
        return false;
    }

    qslNodeList.clear();
    qslNodeList << nodes;
    qslNodeList.sort();

    //add nodes to combobox
    dlgNode->cbNodeSelect->clear();
    dlgNode->cbNodeSelect->addItems(qslNodeList);
    //because QComboBox enforce even 1 item sets to current text
    //so we must update the current text to the member string just to be safe
    //qsSelected = dlgNode->cbNodeSelect->currentText();
    //enable components after node list is ready
    SetComponentWaitForNodeData(false);
    return true;
}

bool Dialog::eventFilter(QObject *obj, QEvent *event)
{

	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = (QKeyEvent *)event;
		if (keyEvent->key() == Qt::Key_Escape) {
			qDebug() << "Escape Event!!" << endl;
			return true;
		}
	}

    /*
    ** Bruce141103. bug fix. segment fault on adding new variable into Dialog object.
    ** dlgNode is not yet constructed and eventFilter() is triggerred somehow.
    */
    if (!dlgNode) {
        //qDebug() << "Incoming event, but dlgNode is NULL\n";
        goto out;
    }

    if (obj == dlgNode->cbNodeSelect)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = (QKeyEvent *)event;
            if ((keyEvent->key() == Qt::Key_Return) ||
				(keyEvent->key() == Qt::Key_Enter)  ||
				(keyEvent->key() == Qt::Key_Space))
            {
                //qDebug() << "enter pressed" << endl;
                ConnectByEvent();
            }
        }
        else if (event->type() == QEvent::MouseButtonPress)
        {
        }
        else
        {
            //qDebug() << "what the hell is these events??" << endl;
        }
        return false;
    }
    else if(obj == dlgNode->btConnect)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            ConnectByEvent();
        }
        else if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = (QKeyEvent *)event;
            if ((keyEvent->key() == Qt::Key_Return) ||
				(keyEvent->key() == Qt::Key_Enter)  ||
				(keyEvent->key() == Qt::Key_Space))
            {
                //qDebug() << "enter pressed" << endl;
                ConnectByEvent();
            }
        }
        return false;
    }
    else if(obj == dlgNode->btRefresh)
    {
        if(event->type() == QEvent::MouseButtonPress)
        {
            //if (bConnectedToHost)
            {
                SetComponentWaitForNodeData(true);
                runNodeList();
            }
        }
        else if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = (QKeyEvent *)event;
            if ((keyEvent->key() == Qt::Key_Return) ||
				(keyEvent->key() == Qt::Key_Enter)  ||
				(keyEvent->key() == Qt::Key_Space))
            {
                SetComponentWaitForNodeData(true);
                runNodeList();
            }
        }
        return false;
    }

out:
    // pass the event on to the parent class
    return QDialog::eventFilter(obj, event);
}

void Dialog::SetComponentWaitForNodeData(bool bWait)
{
    if(!bNodeDlgHidden)
    {
        if (bWait)
        {
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        }
        else
        {
            QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
        }
    }
    dlgNode->btConnect->setDisabled(bWait);
    dlgNode->btRefresh->setDisabled(bWait);
    dlgNode->cbNodeSelect->setDisabled(bWait);
}

void Dialog::ConnectByEvent()
{
    //if(bConnectedToHost && !qslNodeList.isEmpty())
    if(!dlgNode->cbNodeSelect->currentText().isEmpty())
    {
        QString target = dlgNode->cbNodeSelect->currentText();
        //if(qslNodeList.contains(qsSelected))
        //{
            //changeTargetHost(qsSelected);
            //reconnectToHost();
        //}
        //else
            //qDebug() << "qsSelected=" << qsSelected;

        if (option & UIO_CONVERT_TO_HEX) {
            bool ok;
            int value;

            value = target.toInt(&ok, 16);
            if ((target.size() == 1)
              && (ok)
              && (value >= 0)
              && (value <= 15))
            {
                target.setNum(value, 2);
                while (target.size() < 4)
                    target.prepend("0");
            }
        }
        qDebug() << "Connect:" << target;

        /* Bruce170508. A7 don't need to set astparam. */
        //changeTargetHost(target);
        reconnectToHost(target);
    }
}

void Dialog::StartMsgDConnection()
{
    tcpSocket = new QTcpSocket(this);

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMsgD()));
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(connectedMsgD()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayMsgDErr(QAbstractSocket::SocketError)));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnectedMsgD()));
    Connect2MsgD();
}

void Dialog::Connect2MsgD()
{
    hdr.data_len = 0;
    tcpSocket->abort();
    tcpSocket->connectToHost(QString(MSGD_IP), MSGD_FE_PORT);
    /* Any errors will be fired through tcpSocket signal */

}

void Dialog::disconnectedMsgD()
{
#if 0 /* Bruce171122. See no reason to enable dialog under this case?! */
    if (qslArg.contains(QString(BARUI), Qt::CaseInsensitive))
    {
        setNodeDialogVisible(true);
    }
#endif
}

void Dialog::dbgMsg(const QString &s)
{
#if 0
    showMessage(s, Qt::AlignHCenter | Qt::AlignTop, QColor(Qt::red));
#else
    //qDebug() << s;
    QString errMSG;
    errMSG = s;//meaningless just to get rid of compilation variable unused warning
#endif
}

void Dialog::paintEvent(QPaintEvent *p)
{
    QPainter painter(this);
    if (!picture_name.isEmpty())
        painter.drawPixmap(0, 0,screen_width,screen_height,QPixmap(picture_name));
}

void Dialog::readMsgD()
{
    qint64 r = 0;


    if (hdr.data_len == 0) {

        if (tcpSocket->bytesAvailable() < (int)sizeof(struct info_hdr))
            return;

        r = tcpSocket->read((char*)&hdr, sizeof(struct info_hdr));
        if (r <= 0) {
            dbgMsg(tr("something wrong"));
            //BruceToDo.
            return;
        }

        if (hdr.type != INFOTYPE_RT
         && hdr.type != INFOTYPE_ST
         && hdr.type != INFOTYPE_OSD
         && hdr.type != INFOTYPE_GUI_ACTION)
        {
            dbgMsg(tr("wrong type"));
            //BruceToDo.
        }
    }

    if (tcpSocket->bytesAvailable() < hdr.data_len)
        return;


	dbgMsg(QString("hdr.data_len: %1").arg(hdr.data_len));

    QByteArray d(hdr.data_len, '\0');
    r = tcpSocket->read(d.data(), hdr.data_len);
    //reset hdr.data_len
    hdr.data_len = 0;
    if (r <= 0) {
        dbgMsg(tr("something wrong"));
        return;
    }

    // 判断是否是KVM热键信息
    QString str(d.constData());
    //  qDebug() << "str:" << str;
   if(str.isEmpty())
       return;

   if(str.compare("Hotkey3") == 0)
   {
       // GUI取得KM控制权,同时激活OSD菜单
       qDebug() << "--GUI GET KM--";
       emit sigStartKVM(false);
       //m_bKvmMode = false;
       //showOsdMeun();
       return;
   }
   else if(str.compare("Hotkey4") == 0)
   {
       // GUI释放KM控制权，同时隐藏OSD菜单
       qDebug() << "--GUI FREE KM--";
       emit sigStartKVM(true);
       // m_bKvmMode = true;
       // hideOsdMeun();
       return;
   }

	dbgMsg(QString("d.constData: %1").arg(d.constData()));
	dbgMsg(QString("d.size: %1").arg(d.size()));

    parseMsg(d,hdr.type);

    if (tcpSocket->bytesAvailable()) {
        readMsgD();
    }
}

void Dialog::connectedMsgD()
{
    qint64 r;
    struct info_hdr hdr;
#if 0
    QHostAddress addr = tcpSocket->localAddress();
    if ( ! (addr == QHostAddress::Null) ) {
        strcpy((char*)sInfo.IP,
               addr.toString().toAscii().data());

        updateInfoR();
    }
#endif

    //bConnectedToHost = true;
    hdr.type = CMD_GET_ST;
    hdr.data_len = 0;

    r = tcpSocket->write((char*)&hdr, sizeof(hdr));
    if (r <= 0) {
        dbgMsg(tr("something wrong"));
        return;
    }

}

void Dialog::displayMsgDErr(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        dbgMsg(QString("MsgD disconnected"));
        break;
    case QAbstractSocket::HostNotFoundError:
        dbgMsg(QString("The host was not found. Please check the \n"
                       "host name and port settings.\n"));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        dbgMsg(QString("The connection was refused by the peer. \n"
                       "Make sure the MsgD is running, \n"
                       "and check that the host name and port \n"
                       "settings are correct.\n"));
        break;
    default:
        dbgMsg(QString("The following error occurred: \n%1.")
                                 .arg(tcpSocket->errorString()));
    }

}

void Dialog::parseMsg(QByteArray &data, unsigned int type)
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
        memcpy(&osdInfo, data.constData(), data.size());
        updateInfoOSD();
        dbgMsg(tr("OSD done"));
        break;
    case INFOTYPE_GUI_ACTION:
        //memcpy(&guiActionInfo, data.constData(), data.size());
        memcpy(&guiActionInfo, data.constData(), sizeof(guiActionInfo));
        updateGUI();
        break;
    default:
        dbgMsg(tr("wrong type?!"));
    }

}

void Dialog::updateInfoR()
{
    infoR->setText(QString("FW: %1\nLocal IP: %2\nRemote IP: %3\nID: %4")
                   .arg((char*)sInfo.FW)
                   .arg((char*)sInfo.IP)
                   .arg((char*)sInfo.RemoteIP)
                   .arg((char*)sInfo.ID));

    emit sigUpdateDeviceInfo(infoR);
}

void Dialog::updateInfoL()
{
    infoL->setText(QString("%1").arg((char*)rInfo.str));
}

void Dialog::updateInfoOSD()
{
    /* Set Font type */
    OSDFont.setPointSize(osdInfo.font_size);
    //OSDFont.setStyleStrategy(QFont::NoAntialias);
    OSDFont.setWeight(QFont::Bold);
    //OSDFont.setFamily(QString("Arial"));
    infoOSD->setFont(OSDFont);
    //infoOSD->setAutoFillBackground(true);
    //infoOSD->setFrameShape(QFrame::Box);
    //infoOSD->setFrameShadow(QFrame::Raised);
    //infoOSD->setLineWidth(3);
    //infoOSD->setMargin(8);
	if ((osdInfo.font_color & 0x0F000000) == 0) {
		infoOSD->setAlignment(Qt::AlignLeft | Qt::AlignTop);
		lay_OSD->setAlignment(Qt::AlignLeft | Qt::AlignTop);
		osdInfo.font_color |= 0x0F000000;
	} else {
		infoOSD->setAlignment(Qt::AlignHCenter);
		lay_OSD->setAlignment(Qt::AlignHCenter);
	}

	/* Set QPalette */
    OSDPalette.setColor(QPalette::WindowText, QColor((QRgb)osdInfo.font_color));
    infoOSD->setPalette(OSDPalette);

    infoOSD->setText(QString("%1").arg((char*)osdInfo.str));
#if 0 /* WTF is this?! */
    if(infoOSD->text().isEmpty() && OSDFont.pointSize() == 30 && osdInfo.font_color == 0xff00ff00)
    {
        if(QApplication::arguments().contains(QString(BARUI), Qt::CaseInsensitive))
            setNodeDialogVisible(true);
    }
    else
    {
        if(QApplication::arguments().contains(QString(BARUI), Qt::CaseInsensitive))
            setNodeDialogVisible(false);
    }
#endif
}

void Dialog::updateGUI()
{
    if(QApplication::arguments().contains(QString(BARUI), Qt::CaseInsensitive))
    {
        if(guiActionInfo.action_type == ACTION_GUI_CHANGES)
        {
            if(guiActionInfo.ub.show_dialog == GUI_SHOW_DIALOG)
            {
                setNodeDialogVisible(true);
            }
            else
            {
                setNodeDialogVisible(false);
            }
        }
        else if(guiActionInfo.action_type == ACTION_NODE_REFRESH)
        {
            if(guiActionInfo.ub.refresh_node == GUI_REFRESH_NODE)
            {
                initNodeList();
            }
        }

		/* always check show_text setting for necessary ifnormation display */
		if (guiActionInfo.action_type == ACTION_GUI_SHOW_PICTURE) {
			if (guiActionInfo.ub.show_info.show_text == GUI_SHOW_TEXT) {

                // GUI sleep
                emit sigStartSleepMode();

				infoL->setVisible(true);
				infoC->setVisible(true);
				infoR->setVisible(true);
			} else {
				infoL->setVisible(false);
				infoC->setVisible(false);
				infoR->setVisible(false);
			}
		}
    }
    else if(guiActionInfo.action_type == ACTION_GUI_SHOW_PICTURE)
    {
        if(guiActionInfo.ub.show_info.show_text == GUI_SHOW_TEXT)
        {
            // GUI sleep
            emit sigStartSleepMode();

            infoL->setVisible(true);
            infoC->setVisible(true);
            infoR->setVisible(true);
        }
        else
        {
            infoL->setVisible(false);
            infoC->setVisible(false);
            infoR->setVisible(false);
        }

        picture_name.clear();
        picture_name.append(guiActionInfo.ub.show_info.picture_name);
        qDebug() << "Qt GUI picture_name=" << picture_name;
        update();
    }
}

void Dialog::setSleepPicture(char *path)
{
    picture_name.clear();
    picture_name.append(path);
    qDebug() << "Qt GUI picture_name=" << picture_name;
    update();
}

void Dialog::setInfoEnable(bool enable)
{
    infoL->setVisible(enable);
    infoC->setVisible(enable);
    infoR->setVisible(enable);
}

QLabel* Dialog::getDeviceInfo()
{
    return infoR;
}

void Dialog::setNodeDialogVisible(bool bVisible)
{
    dlgNode->setVisible(bVisible);
    if(bVisible)
    {
        bNodeDlgHidden = false;
        QApplication::setOverrideCursor(QCursor( Qt::ArrowCursor ));
    }
    else
    {
        bNodeDlgHidden = true;
        QApplication::setOverrideCursor(QCursor( Qt::BlankCursor ));
    }
}

void Dialog::showEvent ( QShowEvent *e)
{
    e->type();//no purpose but to avoid build variable unused warning
    //if (qslArg.contains(QString(BARUI), Qt::CaseInsensitive))
    //{
        /*
        showing dialog in parent's showEvent rather than in parent's constructor
        because moveEvent needs to get parent's geometry after parent is ready and
        shown, so the result of rect won't deviate
        */
        //dlgNode->show();
    //}
}

void Dialog::switchDialog(QWidget *w)
{
    if (w == dlgNode)
    {
        qDebug() << "switching to tabwidget option";
        dlgNode->setVisible(false);
        //set option tabdialog visible
    }
    else
    {
        qDebug() << "which widget??";
    }
}

void Dialog::initNodeList()
{
    SetComponentWaitForNodeData(true);
    runNodeList();
}

/*
>>>>>
name:ast-gateway0000
IP:169.254.6.138
type:H
status:s_srv_on
--------------------------------------------------
name:ast-client821EBEA2BA96
IP:169.254.9.87
type:C
status:s_srv_on
--------------------------------------------------
<<<<<
*/

NodeDialog::NodeDialog(QWidget *parent)
    : QDialog(parent, Qt::CustomizeWindowHint|
              Qt::WindowStaysOnTopHint|
              Qt::WindowTitleHint|
              Qt::MSWindowsFixedSizeDialogHint)
{
    /*cbNodeSelect = new QComboBox(this);
    //cbNodeSelect->setGeometry(QRect(240, 30, 201, 22));
    cbNodeSelect->setEditable(true);
    btConnect = new QPushButton(QString("Ok"), this);
    //btConnect->setGeometry(cbNodeSelect->geometry().right(), cbNodeSelect->geometry().top(), 45, cbNodeSelect->geometry().height());
    btRefresh = new QPushButton(QString("Refresh"), this);
    //btRefresh->setGeometry(btConnect->geometry().right(), btConnect->geometry().top(), btConnect->geometry().width(), btConnect->geometry().height());
    tabLayout = new QHBoxLayout;
    tabLayout->addWidget(cbNodeSelect);
    tabLayout->addWidget(btConnect);
    tabLayout->addWidget(btRefresh);
    setLayout(tabLayout);*/

    /*Qt::WindowFlags flags = this->windowFlags();
    Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
    Qt::WindowFlags rejectFlag = Qt::WindowSystemMenuHint;
    flags = flags & (~helpFlag);
    flags = flags & (~rejectFlag);
    this->setWindowFlags(flags);*/
    //this->setWindowFlags(Qt::FramelessWindowHint);
    //this->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);

    myParent = (Dialog *)parent;
    installEventFilter(this);
    QFont fnt(QApplication::font());
    fnt.setPointSize(11);
    setFont(fnt);
    setWindowTitle(NODEDIALOGTITLE);
#if ENABLE_OPTION_TAB
    btOption = new QPushButton(this);
    btOption->setIcon(QIcon(":/icons/option.png"));
    btOption->installEventFilter(this);
#endif
    lb = new QLabel(COMBOBOXNAME, this);
    cbNodeSelect = new QComboBox(this);
    connect(this, SIGNAL(optionSelected(QWidget*)), parent, SLOT(switchDialog(QWidget*)) );
    connect(this, SIGNAL(dialogReady()), parent, SLOT(initNodeList()) );
    connect(cbNodeSelect, SIGNAL(editTextChanged(QString)), this, SLOT(on_cbNodeSelect_editTextChanged(QString)));
    connect(cbNodeSelect, SIGNAL(activated(QString)), this, SLOT(on_cbNodeSelect_activated(QString)));
    //mnuBar = new QMenuBar(this);
    //mnuOption = new QMenu(QString("Options"));
    //mnuOption->installEventFilter(this);
    //mnuBar->addMenu(mnuOption);
    cbNodeSelect->setEditable(true);
    btConnect = new QPushButton(QString("Connect"), this);
    btRefresh = new QPushButton(QString("Refresh"), this);
    mainLayout = new QVBoxLayout;
    topLayout = new QHBoxLayout;
    centerLayout = new QHBoxLayout;
    centerLayout2 = new QVBoxLayout;
    bottomLayout = new QHBoxLayout;
    topLayout->insertWidget(0, lb, 0, Qt::AlignLeft);
    cbNodeSelect->setMinimumContentsLength(25);
    centerLayout->insertWidget(0, cbNodeSelect, 0, Qt::AlignLeft);
    bottomLayout->insertWidget(0, btConnect, 0, Qt::AlignRight);
    bottomLayout->insertWidget(1, btRefresh, 0, Qt::AlignRight);
#if ENABLE_OPTION_TAB
    bottomLayout->insertWidget(2, btOption, 0, Qt::AlignRight);
    setTabOrder(btRefresh, btOption);
#endif
    //btConnect->setFocus();
    setTabOrder(btConnect, btRefresh);
    bottomLayout->setAlignment(Qt::AlignRight);
    //mainLayout->addLayout(mnuLayout, 1);
    //mainLayout->addSpacing(40);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(centerLayout);
    centerLayout2->addSpacing(40);
    mainLayout->addLayout(centerLayout2);
    mainLayout->addLayout(bottomLayout);
    //do not allow user to resize this dialog
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(mainLayout);
}

NodeDialog::~NodeDialog()
{
    delete mainLayout;
    delete topLayout;
    delete centerLayout;
    delete centerLayout2;
    delete bottomLayout;
    delete cbNodeSelect;
    delete btConnect;
    delete btRefresh;
    delete lb;
}

void NodeDialog::moveEvent(QMoveEvent *e)
{
    /*
    if(ptDlg == QPoint(0, 0))
        ptDlg = e->oldPos();
    //always move back to the original point
    if (ptDlg != e->pos())
        move(ptDlg);
    */
    e->type();//no purpose but to avoid build variable unused warning
    scr = this->parentWidget()->geometry();
    scr = QRect(scr.left(), scr.top(), scr.width(), scr.height());
    //y starts from 2/3 of screen position
    move( QPoint(scr.center().x(), scr.height() - (scr.height()/3*2) ) - QPoint(rect().center()) );
}

void NodeDialog::on_cbNodeSelect_editTextChanged(QString qstr)
{
    qDebug() << qstr;
}

void NodeDialog::on_cbNodeSelect_activated(QString qstr)
{
    myParent->setSelectedNode(qstr);
    qDebug() << qstr;
}

bool NodeDialog::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = (QKeyEvent *)e;
		if (keyEvent->key() == Qt::Key_Escape) {
			qDebug() << "Escape Event!!" << endl;
			return true;
		}
	}

    if (obj == btOption)
    {
        if (e->type() == QEvent::MouseButtonPress)
        {
            //qDebug() << "option pressed";
            emit optionSelected(this);
        }
        return false;
    }
    else
    {
        return QDialog::eventFilter(obj, e);
    }
}

void NodeDialog::showEvent ( QShowEvent *e)
{
    e->type();
    //emit dialogReady();
}
