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
#include "audioswitch.h"
#include "utilities.h"


int AudioSwitch::currentAudioPort = 0;
int AudioSwitch::currentMode = AUDIO_MODE_AUTO_FILO                                                                                                               ;
int AudioSwitch::manualPort = -1;
int AudioSwitch::audioSwitchTime = 0;
bool AudioSwitch::isAnalogInput = false;
QString AudioSwitch::output;

AudioSwitch::AudioSwitch(QObject *parent)
	: QObject(parent)
{
	audioPlugInTimer = new QTimer(this);
    audioPlugInTimer->setSingleShot(true);
    connect(audioPlugInTimer, SIGNAL(timeout()),
            this, SLOT(audioPlugInTimeout()));

    audioPlugOutTimer = new QTimer(this);
    audioPlugOutTimer->setSingleShot(true);
    connect(audioPlugOutTimer, SIGNAL(timeout()),
            this, SLOT(audioPlugOutTimeout()));
}

//QStringList audioList = {"dante", "analog", "hdmi", "no"};
QStringList audioList;
//QStringList modeList = {"filo", "priority", "manual"};
QStringList modeList;
QStringList switchModeList;

AudioSwitch::~AudioSwitch()
{
	if (0 != audioPlugInTimer) {
		delete audioPlugInTimer;
		audioPlugInTimer = 0;
	}

	if (0 != audioPlugOutTimer) {
		delete audioPlugOutTimer;
		audioPlugOutTimer = 0;
	}
}

void AudioSwitch::init()
{
    audioList.append("dante");
    audioList.append("analog");
    audioList.append("hdmi");
    audioList.append("no");

    modeList.append("filo");
    modeList.append("priority");
    modeList.append("manual");

    switchModeList.append("last_connected");
    switchModeList.append("priority");
    switchModeList.append("manual");
}

void AudioSwitch::start()
{
    sock = create_unixsocket(MAIN_AUDIO_SOCKET_NAME);
    assert(-1 != sock);


    sockNotifier = new QSocketNotifier(sock, QSocketNotifier::Read, this);
    connect(sockNotifier, SIGNAL(activated(int)),
            this, SLOT(sockReadyRead()));

    init();

    parseConfigFile();
    qDebug() << "Audioswitch start.";
}

void AudioSwitch::audioPlugInTimeout()
{
    audioPlugInTimer->stop();
    qDebug() << "Plug in timer timout.";
    if (runList.size() > 0) {
        int index = runList.at(0);
        switchToSource(index);
        qDebug() << "currentPort:" << currentAudioPort;
    }
}

void AudioSwitch::audioPlugOutTimeout()
{
    audioPlugOutTimer->stop();
    qDebug() << __PRETTY_FUNCTION__;
    if (runList.size() > 0) {
        int prevSource = runList.at(0);
        switchToSource(prevSource);

        if (prevSource < audioList.size()) {
            qDebug() << "Switch back to" << audioList.at(prevSource);
        }
    }
}


//switch to source audio.
bool AudioSwitch::switchToSource(int index)
{
    bool ret = true;

    QString cmd;
    cmd.sprintf("ipc @m_lm_set s audio_in:%d", index);
    QProcess::execute(cmd);
    qDebug() << "Run command:" << cmd;
    qDebug() << "Switch to audio" << audioList.at(index);

    currentAudioPort = index;
    return ret;
}

//audio plug in.
//event-audio plugin {dante|analog|hdmi}
bool AudioSwitch::eventPlugIn(const QString &args)
{
    bool ret = false;

    QString str = args.trimmed().toLower();
    QRegExp exp("^(dante|analog|hdmi)$");
    if (-1 == exp.indexIn(str)) {
        qDebug() << "eventPlugIn arg error:" << args;
        return ret;
    }

    int index = audioList.indexOf(str);
    int source = index;
    if ((index >= 0) && (index < AUDIO_INPUT_MAX)) {
        if (runList.contains(index)) {
            qDebug() << args << "aleady plug in." ;
        } else {
             qDebug() << "Audio plug in" << args;

             runList.removeAll(source);
             plugInList.removeAll(source);
             plugInList.prepend(source);
             qDebug() << "plugInList:" << plugInList;

             if (AUDIO_MODE_AUTO_FILO == currentMode) {
                 qDebug() << "auto mode FILO.";
                 runList.prepend(source);    //auto mode:FILO
                 audioPlugInTimer->start(audioSwitchTime);
             } else if (AUDIO_MODE_AUTO_PRIORITY == currentMode) {
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
                     if (takeFirstItem == source) {
                         qDebug() << "first priority";
                         audioPlugInTimer->start(audioSwitchTime);
                     } else {
                         qDebug() << "low priority no switch.";
                     }
                 }
             } else if (AUDIO_MODE_MANUAL == currentMode) {
                 qDebug() << "mannual mode.";
                 qDebug() << "currentAudioPort" << currentAudioPort;
             }
        }
        ret = true;
    }

    return ret;
}

//audio plug out
bool AudioSwitch::eventPlugOut(const QString &args)
{
    bool ret = false;
    qDebug() << __PRETTY_FUNCTION__;
    QString str = args.trimmed().toLower();
    QRegExp exp("^(dante|analog|hdmi)$");
    if (-1 == exp.indexIn(str)) {
        qDebug() << "eventPlugOut arg error:" << args;
        return ret;
    }

    int num = audioList.indexOf(str);
    int index = num;

    qDebug() << "args:" << args;
    qDebug() << "index:" << index;
    qDebug() << "runList:" << runList;
    plugInList.removeAll(index);
    qDebug() << "plugInList:" << plugInList;
    if (runList.size() > 0) {
        int topItem = runList.at(0);
        runList.removeAll(index);

        qDebug() << "topItem:" << topItem;
        if ((AUDIO_MODE_AUTO_FILO == currentMode)
                || (AUDIO_MODE_AUTO_PRIORITY == currentMode)) {
            if (topItem == index) {
                qDebug() << "switch back to audiox?.";
                if (runList.size() > 0) {
                    audioPlugOutTimer->start(audioSwitchTime);
                }
            }
        }
    }

    return ret;
}

bool AudioSwitch::isValidType(const QString &s)
{
    bool ret = false;
    if ((s.toLower() == "audio-event")
       || (s.toLower() == "set")
       || (s.toLower() == "get"))
    {
        ret = true;
    }

    return ret;
}

bool AudioSwitch::parseLocalJsonConfig()
{
    bool ret = false;

    priorityList.append(AUDIO_DANTE);
    priorityList.append(AUDIO_ANALOG);
    priorityList.append(AUDIO_HDMI);
    qDebug() << "priorityList:" << priorityList;
    return ret;
}

bool AudioSwitch::parseConfigFile()
{
    bool ret = false;
    Json::Reader reader;
    Json::Value root;

    QFile file(AUDIO_CONFIG_FILE_PATH);
    if (file.exists()) {
        //从文件中读取，保证当前文件存在
        std::ifstream in(AUDIO_CONFIG_FILE_PATH, std::ios::binary);
        if(!in.is_open()) {
            qDebug() << "open file error:" << AUDIO_CONFIG_FILE_PATH;
            return ret;
        }

        if (reader.parse(in, root)) {
            if (root.isObject() && root.isMember("audio_setting")) {
                Json::Value switchJson = root["audio_setting"];
                qDebug() << "analog_direction isMember: " << switchJson.isMember("analog_direction");
                qDebug() << "switch_mode isMember: " << switchJson.isMember("switch_mode");
                qDebug() << "priority isMember: " << switchJson.isMember("priority");
                qDebug() << "source_select isMember: " << switchJson.isMember("source_select");
                qDebug() << "destination_select isMember: " << switchJson.isMember("destination_select");

                if (switchJson.isMember("destination_select")) {
                    Json::Value dstJson = switchJson["destination_select"];
                    QStringList dstList;
                    for (int cnt = 0; cnt < dstJson.size(); ++cnt) {
                        QString s = dstJson[cnt].asCString();
                        dstList.append(s);
                    }
                    output = dstList.join(" ");
                    qDebug() << "destination_select:" << dstList;
                    setCurrentOutput(output);
                }

                if (switchJson.isMember("analog_direction")) {
                    QString analogDirection = switchJson["analog_direction"].asCString();
                    qDebug() << "analog_direction:" << analogDirection;
                    QString analogCmd;
                    if (analogDirection.toLower() == "in") {
                        isAnalogInput = true;
                        analogCmd.sprintf("ipc @m_lm_set s set_gpio_val:1:70:1");
                    } else {
                        analogCmd.sprintf("ipc @m_lm_set s set_gpio_val:1:70:0");
                    }

                    QProcess::execute(analogCmd);
                    qDebug() << "execute cmd:" << analogCmd;
                }
                qDebug() << "isAnalogInput:" << isAnalogInput;

                if (switchJson.isMember("switch_mode")) {
                    QString switchModeStr = switchJson["switch_mode"].asCString();
                    qDebug() << "switch_mode:" << switchModeStr;
                    if (switchModeList.contains(switchModeStr)) {
                        currentMode = switchModeList.indexOf(switchModeStr);
                    }
                }
                qDebug() << "currentMode:" << currentMode;

                if (switchJson.isMember("source_select")) {
                    QString portStr = switchJson["source_select"].asCString();
                    qDebug () << "source_select str:" << portStr;
                    if (portStr.toLower() == "none") {
                        portStr = "no";
                    }
                    if (audioList.contains(portStr)) {
                        manualPort = audioList.indexOf(portStr);
                    }

                    if (-1 == manualPort) {
                        qDebug() << "default manual port.";
                        manualPort = priorityList.at(0);
                    }
                    switchToSource(manualPort);
                    qDebug () << "source_select:" << manualPort;
                }

                if (switchJson.isMember("priority")) {
                    Json::Value priorJson = switchJson["priority"];
                    quint32 cnt = 0;
                    bool foundError = false;
                    for (cnt = 0; cnt < priorJson.size(); ++cnt) {
                        QString s = priorJson[cnt].asCString();
                        if (!audioList.contains(s)) {
                            foundError = true;
                            break;
                        }
                    }

                    if (!foundError) {
                        for (cnt = 0; cnt < priorJson.size(); ++cnt) {
                            QString s = priorJson[cnt].asCString();
                            priorityList.append(audioList.indexOf(s));
                            //priorityList.append(root[cnt].asInt());
                        }
                        qDebug() << "priority:" << priorityList;
                    } else {
                        priorityList.append(AUDIO_DANTE);
                        priorityList.append(AUDIO_ANALOG);
                        priorityList.append(AUDIO_HDMI);
                        qDebug() << "default priority:" << priorityList;;
                    }
                }

                
            } else {
                qDebug() << "invalid json file" << AUDIO_CONFIG_FILE_PATH;
            }
        } else {
            qDebug() << "parse json file error" << AUDIO_CONFIG_FILE_PATH;
        }
    } else {
        qDebug() << AUDIO_CONFIG_FILE_PATH << "config file not exists";
        qDebug() << "default currentMode:" << currentMode;
        priorityList.append(AUDIO_DANTE);
        priorityList.append(AUDIO_ANALOG);
        priorityList.append(AUDIO_HDMI);
        qDebug() << "default priority:" << priorityList;
        if (-1 == manualPort) {
            qDebug() << "default manual port.";
            manualPort = priorityList.at(0);
            switchToSource(manualPort);
		}
        qDebug () << "default source select:" << manualPort;
    }

    return ret;
}

//sconfig --audio-mode {FILO|priority|manual}
bool AudioSwitch::setCurrentMode(const QString &args)
{
    bool ret = false;
    qDebug() << __PRETTY_FUNCTION__;
    //QStringList modeList = {"filo", "priority", "manual"};
    QString str = args.trimmed().toLower();
    QRegExp exp("^(filo|priority|manual)$");
    if (-1 == exp.indexIn(str)) {
        qDebug() << "setCurrentMode arg error:" << args;
        return ret;
    }
    int mode = modeList.indexOf(str);
    if (-1 != mode) {
        if ((AUDIO_MODE_AUTO_FILO == mode)
            && (currentMode != mode)) {
            if ((AUDIO_MODE_MANUAL == currentMode) 
                && plugInList.contains(manualPort)) {
                runList = plugInList;
                runList.removeAll(manualPort);
                runList.prepend(manualPort);
                qDebug() << "auto mode filo runList:" << runList;
            } else {
                runList = plugInList;
                qDebug() << "auto mode filo rebalance audio runList:" << runList;
                if (runList.size() > 0) {
                    int takeFirstItem = runList.at(0);
                    switchToSource(takeFirstItem);
                }
            }
        } 

        if ((AUDIO_MODE_AUTO_PRIORITY == mode) 
            && (currentMode != mode)) {
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
                qDebug() << "current runList:" << runList;

                if (runList.size() > 0) {
                    int takeFirstItem = runList.at(0);
                    switchToSource(takeFirstItem);
                }
        }
        

        if ((AUDIO_MODE_MANUAL == mode) 
            && (currentMode != mode)) {
            if (-1 != manualPort) {
                qDebug() << "default manual port.";
                manualPort = priorityList.at(0);
            }
            switchToSource(manualPort);
        }

        if (currentMode != mode) {
            currentMode = mode;
        }

        qDebug() << "currentPort:" << currentAudioPort;
        qDebug() << "currentMode:" << currentMode;
    }

    return ret;
}

//sconfig --audio-analog {in|out}
bool AudioSwitch::setAudioAnalog(const QString &args)
{
    bool ret = false;
    qDebug() << __PRETTY_FUNCTION__;
    QString str = args.trimmed().toLower();
    QRegExp exp("^(in|out)$");
    if (-1 == exp.indexIn(str)) {
        qDebug() << "setAudioAnalog args:" << args;
        return ret;
    }

    QString cmd;
    if (str.toLower() == "in") {
        isAnalogInput = true;
        str.sprintf("ipc @m_lm_set s set_gpio_val:1:70:1");
    } else {
        isAnalogInput = false;
        str.sprintf("ipc @m_lm_set s set_gpio_val:1:70:0");
    }
    QProcess::execute(cmd);
    qDebug() << "run cmd:" << cmd;

    qDebug() << "isAnalogInput:" << isAnalogInput;
    return ret;
}

//sconfig --show audio-analog
bool AudioSwitch::getAudioAnalog(sockaddr_un &cliaddr, socklen_t len)
{
    bool ret = false;
    QString msg;
    if (isAnalogInput) {
        msg = "in";
    } else {
        msg = "out";
    }
    qDebug() << "direction:" << msg;
    int n = sendto(sock, qPrintable(msg), msg.length(), 0, (struct sockaddr *)&cliaddr, len);
    if (n > 0) {
        ret = true;
    }
    return ret;
}

//sconfig --audio-input {dante|analog|hdmi|no}
bool AudioSwitch::setCurrentInput(const QString &args)
{
    bool ret = false;
    qDebug() << __PRETTY_FUNCTION__;
    QString str = args.trimmed().toLower();
    QRegExp exp("^(dante|analog|hdmi|no)$");
    if (-1 == exp.indexIn(str)) {
        qDebug() << "setCurrentInput arg error:" << args;
        return ret;
    }

    int source = audioList.indexOf(args);
    switchToSource(source);
    manualPort = source;

    return ret;
}

//sconfig --audio-output {dante|analog|hdmi|lan|no}
bool AudioSwitch::setCurrentOutput(const QString &args)
{
    bool ret = true;
    qDebug() << __PRETTY_FUNCTION__;
    QString cmd;
    QString outputStr;
    bool prefix = false;
    if (args.contains("dante")) {
        outputStr += "0";
        prefix = true;
    }

    if (args.contains("analog")) {
        if (prefix) {
            outputStr += ":1";
        } else {
            outputStr += "1";
        }
        prefix = true;
    }

    if (args.contains("hdmi")) {
        if (prefix) {
            outputStr += ":2";
        } else {
            outputStr += "2";
        }
        prefix = true;
    }

    if (args.contains("lan")) {
        if (prefix) {
            outputStr += ":3";
        } else {
            outputStr += "3";
        }
    }

    if (args.toLower() == "no") {
        outputStr = "";
    }
    qDebug() << "args:" << args;
    qDebug() << "outputStr:" << outputStr;

    cmd.sprintf("ipc @m_lm_set s audio_out:%s", qPrintable(outputStr));
    QProcess::execute(cmd);
    qDebug() << "Run command:" << cmd;
    if (args.toLower() == "no") {
    	output = "";
    } else {
    	output = args;
	}
    return ret;
}


bool AudioSwitch::setPriority(const QString &args)
{
    bool ret = false;
    qDebug() << __PRETTY_FUNCTION__;
    QStringList argsList = args.trimmed().split(" ");
    if (argsList.size() != AUDIO_INPUT_MAX) {
        qDebug() << "setPriority args error.";
        return false;
    }
    QList<quint32> slist;
    foreach (QString s, argsList) {
        int index = audioList.indexOf(s);
        if (-1 != index) {
            slist.append(index);
        }
    }

    if (slist.size() == AUDIO_INPUT_MAX) {
        priorityList = slist;
    }

    qDebug() << "priorityList:" << priorityList;

    return ret;
}

bool AudioSwitch::parseMessage(const QString &s, QString &type, QString &cmd, QString &args, quint32 &code)
{
    Q_UNUSED(cmd);
    bool ret = true;
    QString str(s);
    QStringList strlist;
    strlist = str.trimmed().split(" ");

    qDebug() << "strlist:" << strlist;
    //qDebug() << "size:" << strlist.size();

    if (strlist.size() < MESSAGE_SIZE) {
        ret = false;
    } else {
        type = strlist.at(0).toLower();
        //qDebug() << "type:" << type;
        strlist.removeFirst();
        if (isValidType(type)) {
            QString cmdstr = strlist.at(0).toLower();
            cmd = cmdstr;
            if (type == "audio-event") {
                if (cmdstr.toLower() == "plugin") {
                    code = EVENT_PLUGIN_AUDIO;
                } else if (cmdstr.toLower() == "plugout") {
                    code = EVENT_PLUGOUT_AUDIO;
                } else {
                    ret = false;
                }
            }

            if (type == "set") {
                if (cmdstr.toLower() == "audio-mode") {
                    code = SET_AUDIO_WORK_MODE;
                } else if (cmdstr.toLower() == "audio-input") {
                    code = SET_AUDIO_CURRENT_INPUT;
                } else if (cmdstr.toLower() == "audio-priority") {
                    code = SET_AUDIO_PRIORITY;
				} else if (cmdstr.toLower() == "audio-output") {
                	code = SET_AUDIO_OUTPUT;
                } else if (cmdstr.toLower() == "audio-analog") {
                    code = SET_AUDIO_ANALOG;
                }
            }

            if (type == "get") {
                if (cmdstr.toLower() == "audio-mode") {
                    code = GET_AUDIO_WORK_MODE;
                } else if (cmdstr.toLower() == "audio-input") {
                    code = GET_AUDIO_CURRENT_INPUT;
                } else if (cmdstr.toLower() == "audio-priority") {
                    code = GET_AUDIO_PRIORITY;
				} else if (cmdstr.toLower() == "audio-output") {
                	code = GET_AUDIO_CURRENT_OUTPUT;
                } else if (cmdstr.toLower() == "audio-analog") {
                    code = GET_AUDIO_ANALOG;
                }
            }

            //qDebug() << "code:" << hex << code;

            strlist.removeFirst();
            args = strlist.join(" ");
            //qDebug() << "args:" << args;
        } else {
           ret = false;
        }
    }

    return ret;
}

bool AudioSwitch::getPriority(struct sockaddr_un &cliaddr, socklen_t len)
{
    bool ret = false;
    //QStringList sourceList = {"dante", "analog", "hdmi"};
    QString msg;
    for (int cnt = 0; (cnt < priorityList.size()) && (cnt < audioList.size()); cnt++) {
        if (cnt != 0) {
            msg.append(" ");
        }
        QString s;
        //s.sprintf("HDMI%u", priorityList.at(cnt) + 1);
        s = audioList.at(priorityList.at(cnt));
        msg.append(s);
    }
    qDebug() << "message:" << msg;
    int n = sendto(sock, qPrintable(msg), msg.length(), 0, (struct sockaddr *)&cliaddr, len);
    if (n > 0) {
        ret = true;
    }
    return ret;
}

bool AudioSwitch::getCurrentInput(struct sockaddr_un &cliaddr, socklen_t len)
{
    bool ret = false;
    //QStringList sourceList = {"dante", "analog", "hdmi"};
    QString msg;
    if (currentAudioPort < audioList.size()) {
        msg = audioList.at(currentAudioPort);
    } else {
        msg = "dante";
    }
    //msg.sprintf("HDMI%u", (currentPort + 1));

    int n = sendto(sock, qPrintable(msg), msg.length(), 0, (struct sockaddr *)&cliaddr, len);
    if (n > 0) {
        ret = true;
    }
    return ret;
}

bool AudioSwitch::getCurrentOutput(struct sockaddr_un &cliaddr, socklen_t len)
{
    bool ret = false;
    
    QString msg;
    msg = output;

    int n = sendto(sock, qPrintable(msg), msg.length(), 0, (struct sockaddr *)&cliaddr, len);
    if (n > 0) {
        ret = true;
    }
    return ret;
}


bool AudioSwitch::getCurrentMode(struct sockaddr_un &cliaddr, socklen_t len)
{
    bool ret = false;
    QString msg;
    if (AUDIO_MODE_AUTO_FILO == currentMode) {
        msg.sprintf("FILO");
    } else if (AUDIO_MODE_AUTO_PRIORITY == currentMode) {
        msg.sprintf("priority");
    } else if (AUDIO_MODE_MANUAL == currentMode) {
        msg.sprintf("manual");
    }

    int n = sendto(sock, qPrintable(msg), msg.length(), 0, (struct sockaddr *)&cliaddr, len);
    if (n > 0) {
        ret = true;
    }

    qDebug() << "mode:" << msg;
    return ret;
}

bool AudioSwitch::handlerMessage(quint32 code, const QString &args, struct sockaddr_un &cliaddr, socklen_t len)
{
    bool ret = false;

    switch (code) {
    case EVENT_PLUGIN_AUDIO: {
        ret = eventPlugIn(args);
        break;
    }

    case EVENT_PLUGOUT_AUDIO: {
        ret = eventPlugOut(args);
        break;
    }

    case SET_AUDIO_WORK_MODE: {
        ret = setCurrentMode(args);
        break;
    }

    case SET_AUDIO_CURRENT_INPUT: {
        ret = setCurrentInput(args);
        break;
    }

    case SET_AUDIO_PRIORITY: {
        ret = setPriority(args);
        break;
    }

    case GET_AUDIO_WORK_MODE: {
        ret = getCurrentMode(cliaddr, len);
        break;
    }

    case GET_AUDIO_CURRENT_INPUT: {
        ret = getCurrentInput(cliaddr, len);
        break;
    }

    case GET_AUDIO_PRIORITY: {
        ret = getPriority(cliaddr, len);
        break;
    }

	 case SET_AUDIO_OUTPUT: {
     	ret = setCurrentOutput(args);
     	break;
     }

     case GET_AUDIO_CURRENT_OUTPUT: {
        ret = getCurrentOutput(cliaddr, len);
        break;
    }

    case SET_AUDIO_ANALOG: {
        ret = setAudioAnalog(args);
        break;
    }

    case GET_AUDIO_ANALOG: {
        ret = getAudioAnalog(cliaddr, len);
        break;
    }


    default:
        break;
    }
    return ret;
}

void AudioSwitch::sockReadyRead()
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

