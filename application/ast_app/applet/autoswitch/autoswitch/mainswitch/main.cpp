#include <QCoreApplication>
#include <QTextStream>
#include <QMutex>
#include <QString>
#include <QDateTime>
//#include <QMessageLogContext>
#include <QFile>
#include <QtDebug>
#include <sys/time.h>
#include "mainswitch.h"



#ifndef LOG_FILE
#define LOG_FILE    "/var/log/mainswitch-log.txt"
#endif

#define LOG_ROLLING

#define MAX_FILE_SIZE   200*1024

quint32 getCurrentTimeMs()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec*1000 + tv.tv_sec/1000);
}


void  outputMessage(QtMsgType type, const char *msg)
//void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    //Q_UNUSED(context);
    static QMutex mutex;
    mutex.lock();

    QString text;
    switch (type)
    {
        case QtDebugMsg:
            text = QString("Debug:");
            break;

        case QtWarningMsg:
            text = QString("Warning:");
            break;

        case QtCriticalMsg:
            text = QString("Critical:");
            break;

        case QtFatalMsg:
            text = QString("Fatal:");
            break;
    }

    QString currentDataTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    //qint64 msec = QDateTime::currentMSecsSinceEpoch();
	quint64 msec = getCurrentTimeMs();

    //¿ª»úÔËÐÐÊ±¼äºÁÃë
    QString currentMsec = QString("(%1)").arg(msec);
    currentDataTime += " " + currentMsec;
    QString message = QString("%1 %2 %3").arg(currentDataTime).arg(text).arg(msg);

    QFile file(LOG_FILE);
    file.open(QIODevice::WriteOnly | QIODevice::Append);

    #ifdef LOG_ROLLING
    quint64 size = file.size();
    if (size > MAX_FILE_SIZE) {
        file.close();
        file.open(QIODevice::WriteOnly);
    }
    #endif

    QTextStream text_stream(&file);
    text_stream << message << "\r\n";
    file.flush();
    file.close();
    mutex.unlock();
}


int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    //Qt5
    //qInstallMessageHandler(outputMessage);

    //Qt4
    qInstallMsgHandler(outputMessage);

    MainSwitch mainSwitch;
    mainSwitch.start();
    app.exec();
}
