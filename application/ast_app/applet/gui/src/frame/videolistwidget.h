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

class QLabel;
class QString;
class QListView;
class QStringListModel;
class QModelIndex;

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

    void startTimer(){osdOverTimer.start(m_overTime);};
    void stopTimer(){osdOverTimer.stop();}

    int getOSDMeunHeight();
    int parseOSDPos(QString positon);

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

    void setPageChannels(QAction *action);
    int getPageChannels(){return m_pageChannels;}

    void setOverTime(int msec){m_overTime = msec;}

    // 解析频道json文件
    void  parseChannelJson();
    void  parseMeunJson(QString jsonpath);

    QString restoreString();

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);

    void loadChannel(QStringList channelList);
    QFont loadFontSize();
    QString omittedString(QString i_str ,QString o_str); // 只显示前八个字符


    void initLayout();
    void initPageNumList();
    void initConnect();
    void initButtonStyle();
    void styleSheetControl(QPushButton *button);


    void updateListWidget(int pageChannels);

private:
    QLineEdit *m_inputEdit;
    QListWidget *m_listWidget;
    QStringList m_channelList;
    QStringList m_sreachList;
    QGridLayout *m_mainLayout;

    static QMap<int,QString> channelMap;
    static QSet<int> channelId;

    int m_pageChannels; //  页面频道数
    int m_currentPage;
    int m_maxChannels;

    int m_showPosition;
    int m_FontSize;

    QFont m_font;

    QMenu *pageNumList;

    QPushButton *m_Select;
    QPushButton *m_Search;
    QPushButton *m_Page_up;
    QPushButton *m_Page_down;
    QPushButton *m_Apply;
    QPushButton *m_Exit;

    bool m_onSreachMode; // 搜索模式标记

    QTimer  osdOverTimer; // 定时器
    int  m_overTime;     // 超时时间
};


#endif // VIDEOLISTWIDGET_H
