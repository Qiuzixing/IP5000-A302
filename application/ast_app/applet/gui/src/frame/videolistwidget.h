#ifndef VIDEOLISTWIDGET_H
#define VIDEOLISTWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QStringList>
#include <QFont>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include <QMenu>
#include <QAction>

#include <QReadWriteLock>

class QLabel;
class QString;
class QListView;
class QStringListModel;
class QModelIndex;
class P3ktcp;

class OSDMeun : public QWidget
{
    Q_OBJECT
public:
    enum OSD_MODE
    {
      OSD_MODE_OFF=0,
      OSD_MODE_ON
    };

    OSDMeun(QWidget *parent = 0);

    int getCurrentPage(){return m_currentPage;}
    int getShowPosition(){return m_showPosition;}
    int getdisplayStatus(){return m_displayStatus;}
    int getdisplayConfig(){return m_displayConfig;}
    int getDeviceInfoTimerout(){return m_deviceTimeout;}
    bool getDeviceInfoDisplayStatus() {return m_bInfoLongDisplay;}

    void startTimer(){osdOverTimer.start(m_overTime);};
    void stopTimer(){osdOverTimer.stop();}
    void updateTimer(){osdOverTimer.stop();
                       osdOverTimer.start(m_overTime);}

    int getOSDMeunHeight();
    int parseOSDPos(QString positon);

    // 解析频道json文件
    void  parseChannelJson();
    void  parseMeunJson(QString jsonpath);

    void setListWidgetHeight();
    void setMeunFont();
    void setButtonFont();
    int  setdisplayStatus(bool cfg){m_displayStatus = cfg;}

    void hideSettingPage();
    void setChannelLocation(int id);

signals:
    void selectItem(const QString &text);
    void hdieMeun();

    void updateOsdMenu();
    void hideOsdMenu();

public slots:
    void setCompleter(const QString &text); // 动态的显示完成列表
    void completeText(const QModelIndex &index); // 点击完成列表中的项，使用此项自动完成输入的单词

    void pageUpClicked();
    void pageDownClicked();
    void exitClicked();
    void applyClicked();

    void setPageChannels(QAction *action);
    int getPageChannels(){return m_pageChannels;}

    void setOverTime(int msec){m_overTime = msec;}

    QString restoreString();

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);

    void updateListWidget(int pageChannels);

private:
    void loadChannel(QStringList channelList);
    QFont loadFontSize();
    QString omittedString(QString i_str ,QString o_str); // 只显示前八个字符


    void initLayout();
    void initPageNumList();
    void initConnect();
    void initButtonStyle();
    void styleSheetControl(QPushButton *button);

    void writeOsdJson(int pageChannels);
    void setControlsHide();

private:
    QLineEdit *m_inputEdit;
    QListWidget *m_listWidget;
    QStringList m_channelList;
    QStringList m_sreachList;
    QGridLayout *m_mainLayout;

    P3ktcp* m_p3ktcp;

    static QMap<int,QString> channelMap;
    static QSet<int> channelId;

    int m_pageChannels; //  页面频道数
    int m_currentPage;
    int m_totalPage;
    int m_maxChannels;
    int m_channelID;
    bool m_displayStatus;
    bool m_displayConfig;
    int m_deviceTimeout;

    int m_showPosition;
    int m_FontSize;

    QFont m_font;

    QMenu *pageNumList;
    QList<QAction*> list;

    QLabel *m_Select;
    QLabel *m_Search;
    QPushButton *m_Page_up;
    QPushButton *m_Page_down;
    QPushButton *m_Apply;
    QPushButton *m_Exit;

    bool m_onSreachMode; // 搜索模式标记
    bool m_needSelected;

    QTimer  osdOverTimer; // 定时器
    int  m_overTime;     // 超时时间

    bool m_bInfoLongDisplay; // 标记info是否常显 默认false;

    QReadWriteLock m_lock;
};


#endif // VIDEOLISTWIDGET_H
