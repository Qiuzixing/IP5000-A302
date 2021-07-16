#include <QSocketNotifier>
#include <QStringList>
#include <QString>
#include <QFile>
#include <QTimer>
#include <QProcess>
#include <QtDebug>
#include <iostream>
#include <fstream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <json/json.h>
#include "mainswitch.h"
#include "utilities.h"


quint32 MainSwitch::currentPort = INPUT_HDMI1;
quint32 MainSwitch::currentMode = MODE_AUTO_FILO;
quint32 MainSwitch::prevPort = INPUT_HDMI1;
quint32 MainSwitch::prevMode = MODE_AUTO_FILO;
int MainSwitch::manualPort = INPUT_HDMI1;

Json::Value rootJson;


MainSwitch::MainSwitch(QObject *parent)
    : QObject(parent),
      isSignalValid(std::string("000")),
      plugInIntervalTime(0),
      signalLossSwitchingTime(0),
      plugOutIntervalTime(0),
      manualOverrideInactiveSignalTime(0)
{
    plugInTimer = new QTimer(this);
    plugInTimer->setSingleShot(true);
    connect(plugInTimer, SIGNAL(timeout()),
            this, SLOT(plugInTimeout()));

    signalLossTimer = new QTimer(this);
    signalLossTimer->setSingleShot(true);
    connect(signalLossTimer, SIGNAL(timeout()),
            this, SLOT(signalLossTimeout()));

    plugOutTimer = new QTimer(this);
    plugOutTimer->setSingleShot(true);
    connect(plugOutTimer, SIGNAL(timeout()),
            this, SLOT(plugOutTimeout()));
}


MainSwitch::~MainSwitch()
{
    if (0 != plugInTimer) {
        plugInTimer->stop();
        delete plugInTimer;
        plugInTimer = 0;
    }

    if (0 != plugOutTimer) {
        plugOutTimer->stop();
        delete plugOutTimer;
        plugOutTimer = 0;
    }

    if (0 != signalLossTimer) {
        signalLossTimer->stop();
        delete signalLossTimer;
        signalLossTimer = 0;
    }
}


void MainSwitch::start()
{
    sock = create_unixsocket(MAIN_SOCKET_NAME);
    assert(-1 != sock);

    sockNotifier = new QSocketNotifier(sock, QSocketNotifier::Read, this);
    connect(sockNotifier, SIGNAL(activated(int)),
            this, SLOT(sockReadyRead()));

    writeDefaultJsonConfig();

    parseConfigFile();
    qDebug() << "Mainswitch start.";
}

void MainSwitch::writeJsonConfig()
{
    std::string str;
    str = rootJson.toStyledString();

    std::ofstream ofs(DEFAULT_CONFIG_FILE_PATH);
    ofs << str;
    ofs.close();
}

void MainSwitch::writeDefaultJsonConfig()
{
    QFile file(DEFAULT_CONFIG_FILE_PATH);

    if (!file.exists()) {

        rootJson["Auto Switch Delays"]["signal loss switching"] = 10; //10s
        rootJson["Auto Switch Delays"]["cable unplug"] = 0;
        rootJson["Auto Switch Delays"]["cable plugin"] = 0;
        rootJson["Auto Switch Delays"]["manual-override inactive-signal"] = 10;
        rootJson["Auto Switch"]["mode"] = MODE_AUTO_FILO;
        rootJson["Auto Switch"]["priority"].append(INPUT_HDMI1);
        rootJson["Auto Switch"]["priority"].append(INPUT_HDMI2);
        rootJson["Auto Switch"]["priority"].append(INPUT_HDMI3);
        rootJson["Auto Switch"]["manual port"] = -1;

        std::string str;
        str = rootJson.toStyledString();

        std::ofstream ofs(DEFAULT_CONFIG_FILE_PATH);
        ofs << str;
        ofs.close();

        qDebug() << "write default configure.";
    }

    parseLocalJsonConfig();
}

bool MainSwitch::parseLocalJsonConfig()
{
    bool ret = false;
    Json::Reader reader;
    Json::Value root;

    //从文件中读取，保证当前文件存在
    std::ifstream in(DEFAULT_CONFIG_FILE_PATH, std::ios::binary);
    if(!in.is_open()) {
        qDebug() << "open file error:" << DEFAULT_CONFIG_FILE_PATH;
        return ret;
    }

    if (reader.parse(in, rootJson)) {
        if (rootJson.isObject() && rootJson.isMember("Auto Switch Delays")) {
            Json::Value autoSwitchJson;
            autoSwitchJson = rootJson["Auto Switch Delays"];
            qDebug() << "signal loss switching isMember: " << autoSwitchJson.isMember("signal loss switching");
            qDebug() << "cable unplug isMember: " << autoSwitchJson.isMember("cable unplug");
            qDebug() << "cable plugin isMember: " << autoSwitchJson.isMember("cable plugin");
            qDebug() << "manual-override inactive-signal isMember: " << autoSwitchJson.isMember("manual-override inactive-signal");
            if (autoSwitchJson.isMember("signal loss switching")) {
                signalLossSwitchingTime = autoSwitchJson["signal loss switching"].asInt();
                signalLossSwitchingTime *= 1000; //ms
            }

            if (autoSwitchJson.isMember("cable unplug")) {
                plugOutIntervalTime = autoSwitchJson["cable unplug"].asInt();
                plugOutIntervalTime *= 1000; //ms
            }

            if (autoSwitchJson.isMember("cable plugin")) {
                plugInIntervalTime = autoSwitchJson["cable plugin"].asInt();
                plugInIntervalTime *= 1000; //ms
            }

            if (autoSwitchJson.isMember("manual-override inactive-signal")) {
                manualOverrideInactiveSignalTime = autoSwitchJson["manual-override inactive-signal"].asInt();
                manualOverrideInactiveSignalTime *= 1000; //ms
            }
            qDebug() << "default signalLossSwitchingTime:" << signalLossSwitchingTime/1000 << "s";
            qDebug() << "default plugOutIntervalTime:" << plugOutIntervalTime/1000 << "s";
            qDebug() << "default plugInIntervalTime:" << plugInIntervalTime/1000 << "s";
            qDebug() << "default manualOverrideInactiveSignalTime:" << manualOverrideInactiveSignalTime/1000 << "s";

            Json::Value switchJson = rootJson["Auto Switch"];
            qDebug() << "mode isMember: " << switchJson.isMember("mode");
            qDebug() << "priority isMember: " << switchJson.isMember("priority");
            qDebug() << "manual port isMember: " << switchJson.isMember("mannual port");
            if (switchJson.isMember("mode")) {
                currentMode = switchJson["mode"].asInt();
            }
            qDebug() << "currentMode:" << currentMode;

            if (switchJson.isMember("priority")) {
                Json::Value root = switchJson["priority"];
                for (quint32 cnt = 0; cnt < root.size(); ++cnt) {
                    priorityList.append(root[cnt].asInt());
                }
            }
            qDebug() << "priority:" << priorityList;
            if (switchJson.isMember("manual port")) {
                manualPort = switchJson["mannual port"].asInt();
                if (-1 == manualPort) {
                    qDebug() << "default manual port.";
                    manualPort = priorityList.at(0);
                }
                qDebug () << "manualPort" << manualPort;
            }
        } else {
            qDebug() << "invalid json file" << CONFIG_FILE_PATH;
        }
    }

    return ret;
}


bool MainSwitch::isValidType(const QString &s)
{
    bool ret = false;
    if ((s.toLower() == "event")
       || (s.toLower() == "set")
       || (s.toLower() == "get"))
    {
        ret = true;
    }

    return ret;
}

bool MainSwitch::parseConfigFile()
{
    bool ret = false;
    Json::Reader reader;
    Json::Value root;

    QFile file(CONFIG_FILE_PATH);
    if (file.exists()) {
        //从文件中读取，保证当前文件存在
        std::ifstream in(CONFIG_FILE_PATH, std::ios::binary);
        if(!in.is_open()) {
            qDebug() << "open file error:" << CONFIG_FILE_PATH;
            return ret;
        }

        if (reader.parse(in, root)) {
            if (root.isObject() && root.isMember("Auto Switch Delays")) {
                Json::Value autoSwitchJson;
                autoSwitchJson = root["Auto Switch Delays"];
                qDebug() << "signal loss switching isMember: " << autoSwitchJson.isMember("signal loss switching");
                qDebug() << "cable unplug isMember: " << autoSwitchJson.isMember("cable unplug");
                qDebug() << "cable plugin isMember: " << autoSwitchJson.isMember("cable plugin");
                qDebug() << "manual-override inactive-signal isMember: " << autoSwitchJson.isMember("manual-override inactive-signal");
                if (autoSwitchJson.isMember("signal loss switching")) {
                    signalLossSwitchingTime = autoSwitchJson["signal loss switching"].asInt();
                    signalLossSwitchingTime *= 1000; //ms
                }

                if (autoSwitchJson.isMember("cable unplug")) {
                    plugOutIntervalTime = autoSwitchJson["cable unplug"].asInt();
                    plugOutIntervalTime *= 1000; //ms
                }

                if (autoSwitchJson.isMember("cable plugin")) {
                    plugInIntervalTime = autoSwitchJson["cable plugin"].asInt();
                    plugInIntervalTime *= 1000; //ms
                }

                if (autoSwitchJson.isMember("manual-override inactive-signal")) {
                    manualOverrideInactiveSignalTime = autoSwitchJson["manual-override inactive-signal"].asInt();
                    manualOverrideInactiveSignalTime *= 1000; //ms
                }
                qDebug() << "signalLossSwitchingTime:" << signalLossSwitchingTime/1000 << "s";
                qDebug() << "plugOutIntervalTime:" << plugOutIntervalTime/1000 << "s";
                qDebug() << "plugInIntervalTime:" << plugInIntervalTime/1000 << "s";
                qDebug() << "manualOverrideInactiveSignalTime:" << manualOverrideInactiveSignalTime/1000 << "s";
            } else {
                qDebug() << "invalid json file" << CONFIG_FILE_PATH;
            }
        }
    } else {
        qDebug() << CONFIG_FILE_PATH << "config file not exists";
    }

    return ret;
}

bool MainSwitch::parseMessage(const QString &s, QString &type, QString &cmd, QString &args, quint32 &code)
{
    Q_UNUSED(cmd);
    bool ret = true;
    QString str(s);
    QStringList strlist;
    strlist = str.trimmed().split(" ");

    qDebug() << "strlist:" << strlist;
    qDebug() << "size:" << strlist.size();

    if (strlist.size() < MESSAGE_SIZE) {
        ret = false;
    } else {
        type = strlist.at(0).toLower();
        qDebug() << "type:" << type;
        strlist.removeFirst();
        if (isValidType(type)) {
            QString cmdstr = strlist.at(0).toLower();
            cmd = cmdstr;
            if (type == "event") {
                if (cmdstr.toLower() == "plugin") {
                    code = EVENT_PLUGIN_HDMI;
                } else if (cmdstr.toLower() == "plugout") {
                    code = EVENT_PLUGOUT_HDMI;
                } else if (cmdstr.toLower() == "signal-valid") {
                    code = EVENT_SIGNAL_VALID;
                } else if (cmdstr.toLower() == "signal-invalid") {
                    code = EVENT_SIGNAL_INVALID;
                } else {
                    ret = false;
                }
            }

            if (type == "set") {
                if (cmdstr.toLower() == "mode") {
                    code = SET_WORK_MODE;
                } else if (cmdstr.toLower() == "input") {
                    code = SET_CURRENT_INPUT;
                } else if (cmdstr.toLower() == "priority") {
                    code = SET_PRIORITY;
                }
            }

            if (type == "get") {
                if (cmdstr.toLower() == "mode") {
                    code = GET_WORK_MODE;
                } else if (cmdstr.toLower() == "input") {
                    code = GET_CURRENT_INPUT;
                } else if (cmdstr.toLower() == "priority") {
                    code = GET_PRIORITY;
                }
            }

            qDebug() << "code:" << hex << code;

            strlist.removeFirst();
            args = strlist.join(" ");
            qDebug() << "args:" << args;
        } else {
           ret = false;
        }
    }

    return ret;
}

//HDMI plug in.
bool MainSwitch::eventPlugIn(const QString &args)
{
    bool ret = false;
    QString str = args.trimmed().toUpper();
    QRegExp exp("^HDMI[1-3]$");
    if (-1 == exp.indexIn(str)) {
        qDebug() << "eventPlugIn arg error:" << args;
        return ret;
    }

    int index = str.mid(4).toUInt();
    int source = index - 1;
    if ((index > 0) && (index <= HDMI_INPUT_MAX)) {
        if (runList.contains(index - 1)) {
            qDebug() << args << "aleady plug in." ;
        } else {
             qDebug() << "Plug in" << args;

             runList.removeAll(source);
             plugInList.removeAll(source);
             plugInList.prepend(source);
             qDebug() << "plugInList:" << plugInList;

             if (MODE_AUTO_FILO == currentMode) {
                 qDebug() << "auto mode FILO.";
                 runList.prepend(source);    //auto mode:FILO

                 if (plugInIntervalTime > 0) {
                     plugInTimer->start(plugInIntervalTime);
                 } else {
                     plugInTimer->stop();
                     switchToSource(source);
                 }
             } else if (MODE_AUTO_PRIORITY == currentMode) {
                 qDebug() << "auto priority mode.";
                 qDebug() << "runList:" << runList;
                 qDebug() << "plugInList:" << plugInList;
                 QList<quint32> currentList;
                 for (int n = 0; n < priorityList.size(); ++n) {
                     quint32 item = priorityList.at(n);
                     if (plugInList.contains(item)) {
                         currentList.append(item);
                     }
                 }
                 runList = currentList;
                 qDebug() << "runList:" << runList;

                 if (runList.size() > 0) {
                     int takeFirstItem = runList.at(0);
                     QString sstr;
                     sstr.sprintf("HDMI%u", source + 1);
                     if (takeFirstItem == source) {
                         qDebug() << "first priority" << sstr;
                         if (plugInIntervalTime > 0) {
                             plugInTimer->start(plugInIntervalTime);
                         } else {
                             plugInTimer->stop();
                             switchToSource(source);
                         }
                     } else {
                         qDebug() << sstr << "low priority no switch.";
                     }
                 }
             } else if (MODE_MANUAL == currentMode) {
                 qDebug() << "mannual mode.";
                 if (manualPort == source) {
                     #if 0
                     if (plugInIntervalTime > 0) {
                         plugInTimer->start(plugInIntervalTime);
                     } else {
                         plugInTimer->stop();
                         switchToSource(source);
                     }
                     #endif
                 }
             }


        }
        ret = true;
    }

    return ret;
}

void MainSwitch::plugInTimeout()
{
    plugInTimer->stop();

    qDebug() << "Plug in timer timout.";
    if (runList.size() > 0) {
        int index = runList.at(0);
        switchToSource(index);
        qDebug() << "currentPort:" << currentPort;
    }
}

//
bool MainSwitch::switchToSourceForce(int index)
{
    bool ret = true;

    QString cmd;
    //cmd.sprintf("plug in hdmi%d", (index + 1));
	cmd.sprintf("ipc @m_lm_set s set_input_source:16:%d", index);
    QProcess::execute(cmd);
    qDebug() << "Run command:" << cmd;
    qDebug() << "Force switch to HDMI" << (index + 1);

    currentPort = index;

    return ret;
}

//switch to HDMIn source.
bool MainSwitch::switchToSource(int index)
{
    bool ret = true;

    QString cmd;
    //cmd.sprintf("plug in hdmi%d", (index + 1));
	//ipc @m_lm_set s set_input_source:16:0
	cmd.sprintf("ipc @m_lm_set s set_input_source:16:%d", index);
    QProcess::execute(cmd);
    qDebug() << "Run command:" << cmd;
    qDebug() << "Switch to HDMI" << (index + 1);

    currentPort = index;
    if (currentMode == MODE_MANUAL_OVERRIDE) {
        if (manualOverrideInactiveSignalTime > 0) {
            signalLossTimer->start(manualOverrideInactiveSignalTime);
            qDebug() << "MODE_MANUAL_OVERRIDE:start timer manualOverrideInactiveSignalTime:" << manualOverrideInactiveSignalTime;
        } else {
            qDebug() << "manualOverrideInactiveSignalTime error, time is 0.";
        }

    } else {
        if (signalLossSwitchingTime > 0) {
            signalLossTimer->start(signalLossSwitchingTime);
            qDebug() << "start timer signalLossSwitchingTime:" << signalLossSwitchingTime;
        } else {
            qDebug() << "signalLossSwitchingTime error, time is 0.";
        }
    }

    return ret;
}

void MainSwitch::plugOutTimeout()
{
    plugOutTimer->stop();

    if (runList.size() > 0) {
        quint32 prevSource = runList.at(0);
        switchToSource(prevSource);
        QString str;
        str.sprintf("HDMI%d", (prevSource + 1));
        qDebug() << "Switch back to" << str;
    }
}

//HDMI plug out
bool MainSwitch::eventPlugOut(const QString &args)
{
    bool ret = false;
    qDebug() << __PRETTY_FUNCTION__;
    QString str = args.trimmed().toUpper();
    QRegExp exp("^HDMI[1-3]$");
    if (-1 == exp.indexIn(str)) {
        qDebug() << "eventPlugOut arg error:" << args;
        return ret;
    }

    int num = str.mid(4).toUInt();
    int index = num - 1;

    qDebug() << "args:" << args;
    qDebug() << "index:" << index;
    qDebug() << "runList:" << runList;
    isSignalValid.reset(index);
    plugInList.removeAll(index);
    qDebug() << "plugInList:" << plugInList;
    if (runList.size() > 0) {
        int topItem = runList.at(0);
        runList.removeAll(index);

        qDebug() << "topItem:" << topItem;
        if ((MODE_AUTO_FILO == currentMode)
                || (MODE_AUTO_PRIORITY == currentMode)) {
            if (topItem == index) {
                qDebug() << "switch back to HDMIx?.";
                if (runList.size() > 0) {
                    if (plugOutIntervalTime > 0) {
                        plugOutTimer->start(plugOutIntervalTime);
                    } else {
                        quint32 prevSource = runList.at(0);
                        switchToSource(prevSource);
                        qDebug() << "switch back to" << prevSource;
                    }
                } else {
					switchToSourceForce(priorityList.at(0));
				}
            }
        } else if ((MODE_MANUAL_OVERRIDE == currentMode)) {
            if (index == manualPort) {
                currentMode = prevMode;
                qDebug() << "Switch mode back to" << prevMode;

                QString sstr;
                sstr.sprintf("HDMI%d", prevPort + 1);
                qDebug() << "Switch source back to" << sstr;
                switchToSource(prevPort);
            }
        }
    }

    return ret;
}

//valid signal loss timeout
void MainSwitch::signalLossTimeout()
{
    signalLossTimer->stop();
    qDebug() << __PRETTY_FUNCTION__;
    qDebug() << "currentMode:" << currentMode;
    qDebug() << "currentPort:" << currentPort;
    if (isSignalValid.test(currentPort)) {
        qDebug() << "current port signal valid,no switch.";
    } else {
        if ((MODE_AUTO_FILO == currentMode)
                || (MODE_AUTO_PRIORITY == currentMode)) {
            int pos = runList.indexOf(currentPort);
            if (pos != -1) {
                if (runList.size() == (pos + 1)) {
                    if (currentPort != priorityList.at(0)) {
                        switchToSourceForce(priorityList.at(0));
                    } else {
                        qDebug() << "No switch, already at default port.";
                    }
                } else {
                    QString sstr;
                    sstr.sprintf("HDMI%d", runList.at(pos + 1) + 1);
                    qDebug() << "Switch back to" << sstr;
                    switchToSource(runList.at(pos + 1));
                }
            }
        } else if (MODE_MANUAL_OVERRIDE == currentMode) {
            currentMode = prevMode;
            qDebug() << "switch mode back to" << prevMode;
            QString sstr;
            sstr.sprintf("HDMI%d", prevPort + 1);
            qDebug() << "switch source back to" << sstr;
            switchToSource(prevPort);
        }
    }
}

bool MainSwitch::eventSignalValid(const QString &args)
{
    bool ret = false;
    qDebug() << __PRETTY_FUNCTION__;
    QString str = args.trimmed().toUpper();
    QRegExp exp("^HDMI[1-3]$");
    if (-1 == exp.indexIn(str)) {
        qDebug() << "source arg error:" << args;
        return ret;
    }

    int index = str.mid(4).toUInt();
    quint32 source = index - 1;
    qDebug() << "current index:" << source;
    qDebug() << "currentPort" << currentPort;
    if (source != currentPort) {
        qDebug() << "HDMI source state invalid.";
        //assert(0);
    }
    isSignalValid.set(source);

    return ret;
}

bool MainSwitch::eventSignalInvalid(const QString &args)
{
    bool ret = false;
    qDebug() << __PRETTY_FUNCTION__;
    QString str = args.trimmed().toUpper();
    QRegExp exp("^HDMI[1-3]$");
    if (-1 == exp.indexIn(str)) {
        qDebug() << "eventSignalInvalid arg error:" << args;
        return ret;
    }
    qDebug() << "args:" << args;

    int index = str.mid(4).toUInt();
    quint32 source = index - 1;
    qDebug() << "current index:" << source;
    qDebug() << "currentPort" << currentPort;
    if (source != currentPort) {
        qDebug() << "HDMI not current port invalid.";
        //assert(0);
    } else {
        if (currentMode == MODE_MANUAL_OVERRIDE) {
            if (manualOverrideInactiveSignalTime > 0) {
                signalLossTimer->start(manualOverrideInactiveSignalTime);
                qDebug() << "MODE_MANUAL_OVERRIDE:start timer manualOverrideInactiveSignalTime:" << manualOverrideInactiveSignalTime;
                ret = false;
            } else {
                qDebug() << "manualOverrideInactiveSignalTime args error.";
            }

        } else {
            if (signalLossSwitchingTime > 0) {
                signalLossTimer->start(signalLossSwitchingTime);
                qDebug() << "start timer signalLossSwitchingTime:" << signalLossSwitchingTime;
                ret = true;
            } else {
                qDebug() << "signalLossSwitchingTime args error.";
            }
        }
    }
    isSignalValid.reset(source);

    return ret;
}


//set current work mode
bool MainSwitch::setCurrentMode(const QString &args)
{
    bool ret = false;
    if (args.toLower() == "filo") {
        if (MODE_AUTO_FILO != currentMode) {
            currentMode = MODE_AUTO_FILO;
            qDebug() << "runList:" << runList;
            qDebug() << "plugInList:" << plugInList;
            runList = plugInList;
            qDebug() << "rebalance runlist:" << runList;

            if (runList.size() > 0) {
                quint32 topItem = runList.at(0);
                if (topItem != currentPort) {
                    switchToSource(topItem);
                }
            }
        }
    } else if (args.toLower() == "priority") {
        currentMode = MODE_AUTO_PRIORITY;
        qDebug() << "runList:" << runList;
        qDebug() << "plugInList:" << plugInList;
        QList<quint32> currentList;
        for (int n = 0; n < priorityList.size(); ++n) {
            quint32 item = priorityList.at(n);
            if (plugInList.contains(item)) {
                currentList.append(item);
            }
        }
        runList = currentList;
        qDebug() << "runList:" << runList;
        if (runList.size() > 0) {
            quint32 topItem = runList.at(0);
            if (topItem != currentPort) {
                switchToSource(topItem);
            }
        } else {
            qDebug() << "Switch to default high priority port " << priorityList.at(0);
            switchToSourceForce(priorityList.at(0));
        }
    } else if (args.toLower() == "manual") {
        currentMode = MODE_MANUAL;
        qDebug() << "Force switch to manual port" << manualPort;
        switchToSourceForce(manualPort);
    }

    if (rootJson["Auto Switch"]["mode"] != currentMode) {
        rootJson["Auto Switch"]["mode"] = currentMode;
        writeJsonConfig();
    }
    return ret;
}


//switch to HDMIn
bool MainSwitch::setCurrentInput(const QString &args)
{
    bool ret = false;
    qDebug() << __PRETTY_FUNCTION__;
    QString str = args.trimmed().toUpper();
    QRegExp exp("^HDMI[1-3]$");
    if (-1 == exp.indexIn(str)) {
        qDebug() << "setCurrentInput arg error:" << args;
        return ret;
    }

    int num = str.mid(4).toUInt();
    if (currentMode == MODE_MANUAL) {
        switchToSourceForce(num - 1);
        if (rootJson["Auto Switch"]["manual port"] != currentMode) {
            rootJson["Auto Switch"]["manual port"] = currentMode;
            writeJsonConfig();
        }
    } else {
        prevMode = currentMode;
        prevPort = currentPort;
        qDebug() << "currentMode:" << currentMode;
        qDebug() << "currentPort:" << currentPort;
        currentMode = MODE_MANUAL_OVERRIDE;
        switchToSource(num - 1);
    }
    return ret;
}

bool MainSwitch::setPriority(const QString &args)
{
    bool ret = false;
    QStringList argsList = args.trimmed().split(" ");
    if (argsList.size() != HDMI_INPUT_MAX) {
        qDebug() << "setPriority args error.";
        return false;
    }

    QList<quint32> slist;
    foreach (QString s, argsList) {
        quint32 n = s.mid(4).toUInt() - 1;
        slist.append(n);
    }
    if (slist.contains(INPUT_HDMI1)
         && slist.contains(INPUT_HDMI2)
            && slist.contains(INPUT_HDMI3)) {
        priorityList = slist;

        rootJson["Auto Switch"]["priority"].resize(0);
        for (int cnt = 0; cnt < priorityList.size(); ++cnt) {
            rootJson["Auto Switch"]["priority"].append(priorityList.at(cnt));
        }
        writeJsonConfig();
    }

    qDebug() << "current priorityList" << priorityList;
    return ret;
}

bool MainSwitch::getCurrentInput(struct sockaddr_un &cliaddr, socklen_t len)
{
    bool ret = false;
    QString msg;
    msg.sprintf("HDMI%u", (currentPort + 1));

    int n = sendto(sock, qPrintable(msg), msg.length(), 0, (struct sockaddr *)&cliaddr, len);
    if (n > 0) {
        ret = true;
    }
    return ret;
}

bool MainSwitch::getCurrentMode(struct sockaddr_un &cliaddr, socklen_t len)
{
    bool ret = false;
    QString msg;
    if (MODE_AUTO_FILO == currentMode) {
        msg.sprintf("FILO");
    } else if (MODE_AUTO_PRIORITY == currentMode) {
        msg.sprintf("priority");
    } else if (MODE_MANUAL == currentMode) {
        msg.sprintf("manual");
    } else if (MODE_MANUAL_OVERRIDE == currentMode) {
        if (MODE_AUTO_FILO == prevMode) {
             msg.sprintf("FILO");
        } else if (MODE_AUTO_PRIORITY == prevMode) {
            msg.sprintf("priority");
        } else {
            msg.sprintf("unknow");
        }
    }
    int n = sendto(sock, qPrintable(msg), msg.length(), 0, (struct sockaddr *)&cliaddr, len);
    if (n > 0) {
        ret = true;
    }

    qDebug() << "mode:" << msg;
    return ret;
}

bool MainSwitch::getPriority(struct sockaddr_un &cliaddr, socklen_t len)
{
    bool ret = false;
    QString msg;
    for (int cnt = 0; cnt < priorityList.size(); cnt++) {
        if (cnt != 0) {
            msg.append(" ");
        }
        QString s;
        s.sprintf("HDMI%u", priorityList.at(cnt) + 1);
        msg.append(s);
    }
    int n = sendto(sock, qPrintable(msg), msg.length(), 0, (struct sockaddr *)&cliaddr, len);
    if (n > 0) {
        ret = true;
    }
    return ret;
}

bool MainSwitch::handlerMessage(quint32 code, const QString &args, struct sockaddr_un &cliaddr, socklen_t len)
{
    bool ret = false;

    switch (code) {
    case EVENT_PLUGIN_HDMI: {
        ret = eventPlugIn(args);
        break;
    }

    case EVENT_PLUGOUT_HDMI: {
        ret = eventPlugOut(args);
        break;
    }

    case EVENT_SIGNAL_VALID: {
        ret = eventSignalValid(args);
        break;
    }

    case EVENT_SIGNAL_INVALID: {
        ret = eventSignalInvalid(args);
        break;
    }

    case SET_WORK_MODE: {
        ret = setCurrentMode(args);
        break;
    }

    case SET_CURRENT_INPUT: {
        ret = setCurrentInput(args);
        break;
    }

    case SET_PRIORITY: {
        ret = setPriority(args);
        break;
    }

    case GET_WORK_MODE: {
        ret = getCurrentMode(cliaddr, len);
        break;
    }

    case GET_CURRENT_INPUT: {
        ret = getCurrentInput(cliaddr, len);
        break;
    }

    case GET_PRIORITY: {
        ret = getPriority(cliaddr, len);
        break;
    }

    default:
        break;
    }
    return ret;
}

void MainSwitch::sockReadyRead()
{
    char buf[BUFSIZ] = {0};
    struct sockaddr_un claddr;
    ssize_t numBytes = 0;
    socklen_t len = 0;
    len = sizeof(struct sockaddr_un);
    numBytes = recvfrom(sock, buf, BUFSIZ, 0,
                        (struct sockaddr *) &claddr, &len);
    if (-1 == numBytes) {
        qDebug() << "recvfrom error.";
    } else {
        buf[numBytes] = '\0';
        QString str(buf);
        qDebug() << "from" << claddr.sun_path << ":" << str;

        QString type;
        QString cmd;
        QString args;
        quint32 code;
        bool ok = parseMessage(str, type, cmd, args, code);
        if (ok) {
            qDebug() << "type:" << type;
            qDebug() << "command:" << cmd;
            qDebug() << "argument:" << args;
            qDebug() << "code:" << hex << code;
            ok = handlerMessage(code, args, claddr, len);
            qDebug() << "handlerMessage" << ok;
        } else {
            qDebug() << "parser message error." << str;
        }
    }
}
