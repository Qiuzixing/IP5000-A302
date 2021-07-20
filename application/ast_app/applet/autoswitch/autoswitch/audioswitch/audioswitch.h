#ifndef AUDIOSWITCH_H
#define AUDIOSWITCH_H

#include <QObject>
#include <QList>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define MAIN_AUDIO_SOCKET_NAME    "/tmp/audioswitch"

#ifndef MESSAGE_SIZE
#define MESSAGE_SIZE    3
#endif

#ifndef AUDIO_CONFIG_FILE_PATH
#define AUDIO_CONFIG_FILE_PATH  "/data/configs/kds-6x/audio/audio_setting.json"
#endif

#ifndef AUDIO_INPUT_MAX
#define AUDIO_INPUT_MAX 3
#endif

class QSocketNotifier;
class QTimer;


enum AudioCode
{
    EVENT_PLUGIN_AUDIO = 0x200,
    EVENT_PLUGOUT_AUDIO,
    SET_AUDIO_WORK_MODE,
    SET_AUDIO_CURRENT_INPUT,
    SET_AUDIO_PRIORITY,
    GET_AUDIO_WORK_MODE,
    GET_AUDIO_CURRENT_INPUT,
    GET_AUDIO_PRIORITY,
    SET_AUDIO_OUTPUT,
    GET_AUDIO_CURRENT_OUTPUT,
};

enum AudioMode
{
    AUDIO_MODE_AUTO_FILO,  // first in last out mode
    AUDIO_MODE_AUTO_PRIORITY,     //priority mode
    AUDIO_MODE_MANUAL,     //manual mode
};

enum AudioSource
{
    AUDIO_DANTE,
    AUDIO_ANALOG,
    AUDIO_HDMI
};

class AudioSwitch : public QObject
{
	Q_OBJECT

public:
	AudioSwitch(QObject *parent = 0);

	~AudioSwitch();

	void start();

    void init();


    bool parseLocalJsonConfig();

    bool parseMessage(const QString &s, QString &type, QString &cmd, QString &args, quint32 &code);

    bool handlerMessage(quint32 code, const QString &args, struct sockaddr_un &cliaddr, socklen_t len);

    bool eventPlugIn(const QString &args);

    bool eventPlugOut(const QString &args);

    bool isValidType(const QString &s);

    bool parseConfigFile();

    bool switchToSource(int index);

    bool setCurrentMode(const QString &args);

    bool setCurrentInput(const QString &args);
	
	bool setCurrentOutput(const QString &args);


    bool setPriority(const QString &args);

    bool getCurrentMode(struct sockaddr_un &cliaddr, socklen_t len);

    bool getCurrentInput(struct sockaddr_un &cliaddr, socklen_t len);
	
	bool getCurrentOutput(struct sockaddr_un &cliaddr, socklen_t len);

    bool getPriority(struct sockaddr_un &cliaddr, socklen_t len);

private slots:
    void sockReadyRead();   //socket read ready

    void audioPlugInTimeout();   //audio plug in timeout

    void audioPlugOutTimeout();  //audio plug out timeout


private:
	int sock;   //unix domain socket /tmp/audioswitch
    QSocketNotifier *sockNotifier;
    QTimer *audioPlugInTimer;
    QTimer *audioPlugOutTimer;
    QList<quint32> plugInList;
    QList<quint32> runList;
    QList<quint32> priorityList;
    static bool isAnalogInput;
    static QString output;
    static int audioSwitchTime;
    static int currentAudioPort;
    static int currentMode;
    static int manualPort;
};




#endif
