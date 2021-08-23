#ifndef MAINSWITCH_H
#define MAINSWITCH_H

#include <QList>
#include <QObject>
#include <cstddef>
#include <bitset>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>


class QSocketNotifier;
class QTimer;


#define MAIN_SOCKET_NAME    "/tmp/mainswitch"

#define MESSAGE_SIZE    3

#define HDMI_INPUT_MAX  3

#define CONFIG_FILE_PATH    "/data/configs/kds-6x/switch/auto_switch_delays.json"
#define CONFIG_FILE_AUTO_SETTING	"/data/configs/kds-6x/switch/auto_switch_setting.json"
//#define DEFAULT_CONFIG_FILE_PATH    "default.json"//"/data/configs/kds-sw3-en-6x/switch/default.json" //"default.json"

enum InputHdmi
{
    INPUT_HDMI1 = 0,
    INPUT_HDMI2,
    INPUT_HDMI3,
};

enum WorkMode
{
    MODE_AUTO_FILO,  // first in last out mode
    MODE_AUTO_PRIORITY,     //priority mode
    MODE_MANUAL,     //manual mode
    MODE_MANUAL_OVERRIDE //manual-override mode
};

enum MessageCode
{
    EVENT_PLUGIN_HDMI = 0x100,
    EVENT_PLUGOUT_HDMI,
    EVENT_SIGNAL_VALID,
    EVENT_SIGNAL_INVALID,
    SET_WORK_MODE,
    SET_PRIORITY,
    SET_CURRENT_INPUT,
    GET_WORK_MODE,
    GET_PRIORITY,
    GET_CURRENT_INPUT,
};


#if 0
enum EventHdmi
{
    EVENT_PLUGIN_HDMI1 = 0x200,
    EVENT_PLUGIN_HDMI2,
    EVENT_PLUGIN_HDMI3,
    EVENT_PLUGOUT_HDMI1,
    EVENT_PLUGOUT_HDMI2,
    EVENT_PLUGOUT_HDMI3,
    EVENT_SIGNAL_VALID_HDMI1,
    EVENT_SIGNAL_VALID_HDMI2,
    EVENT_SIGNAL_VALID_HDMI3,
    EVENT_SIGNAL_INVALID_HDMI1,
    EVENT_SIGNAL_INVALID_HDMI2,
    EVENT_SIGNAL_INVALID_HDMI3,
};
#endif

class MainSwitch : public QObject
{
    Q_OBJECT

public:
    MainSwitch(QObject *parent = 0);

    ~MainSwitch();

    void start();

    void init();

    void writeDefaultJsonConfig();

    void writeJsonConfig();

    bool parseLocalJsonConfig();

    bool isValidType(const QString &s);

    bool parseMessage(const QString &s, QString &type, QString &cmd, QString &args, quint32 &code);

    bool handlerMessage(quint32 code, const QString &args, struct sockaddr_un &cliaddr, socklen_t len);

    bool eventPlugIn(const QString &args);

    bool eventPlugOut(const QString &args);

    bool eventSignalValid(const QString &args);

    bool eventSignalInvalid(const QString &args);

    bool parseConfigFile();

    bool switchToSource(int index);

    bool switchToSourceForce(int index);

    bool setCurrentMode(const QString &args);

    bool setCurrentInput(const QString &args);

    bool setPriority(const QString &args);

    bool getCurrentMode(struct sockaddr_un &cliaddr, socklen_t len);

    bool getCurrentInput(struct sockaddr_un &cliaddr, socklen_t len);

    bool getPriority(struct sockaddr_un &cliaddr, socklen_t len);

private slots:
    void sockReadyRead();   //socket read ready

    void plugInTimeout();   //HDMI plug in timeout

    void plugOutTimeout();  //HDMI plug out timeout

    void signalLossTimeout();   //valid signal loss timeout

private:
    int sock;   //unix domain socket /tmp/mainswitch
    QSocketNotifier *sockNotifier;
    QTimer *plugInTimer;
    QTimer *signalLossTimer;
    QTimer *plugOutTimer;
    QList<quint32> eventList;
    std::bitset<HDMI_INPUT_MAX> isSignalValid;
    QList<quint32> plugInList;
    QList<quint32> runList;
    QList<quint32> priorityList;
    quint32 plugInIntervalTime;
    quint32 signalLossSwitchingTime;
    quint32 plugOutIntervalTime;
    quint32 manualOverrideInactiveSignalTime;
    static quint32 currentMode;
    static quint32 prevMode;
    static quint32 currentPort;
    static quint32 prevPort;
    static int manualPort;
};



#endif // MAINSWITCH_H
