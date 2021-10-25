#include "videolistwidget.h"
#include "common/global.h"
#include "common/scale_global.h"
#include "p3ktcp.h"

#include "json/json.h"
#include <string>
#include <iostream>
#include <fstream>

#include <QKeyEvent>
#include <QListView>
#include <QStringListModel>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QFontMetrics>
#include <QFile>
#include <QLabel>
#include <QWidgetAction>

#define MENUINFO_PATH      "/data/configs/kds-7/osd/osd.json"
#define CHANNELS_LIST_PATH "/data/configs/kds-7/channel/channel_map.json"

using namespace std;

QMap<int,QString> OSDMeun::channelMap;
QSet<int> OSDMeun::channelId;

OSDMeun::OSDMeun(QWidget *parent)
    :QWidget(parent)
    ,m_currentPage(1)
    ,m_onSreachMode(false)
{
    // 解析菜单参数
    parseMeunJson(MENUINFO_PATH);

    // 初始化菜单布局
    initLayout();

    // 加载频道列表
     parseChannelJson();

    // 注册连接函数
    initConnect();

    // P3K
    m_p3ktcp = P3ktcp::getInstance();
}

void OSDMeun::focusOutEvent(QFocusEvent *e)
{

}

void OSDMeun::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();
    int count = m_listWidget->count();
    QModelIndex currentIndex = m_listWidget->currentIndex();

    if (Qt::Key_Down == key) {
        // 按向下方向键时，移动光标选中下一个完成列表中的项
        int row = currentIndex.row() + 1;
        if (row >= count) {
            row = 0;
        }
        QModelIndex index = m_listWidget->model()->index(row, 0);
        m_listWidget->setCurrentIndex(index);
    } else if (Qt::Key_Up == key) {
        // 按向下方向键时，移动光标选中上一个完成列表中的项
        int row = currentIndex.row() - 1;
        if (row < 0) {
            row = count - 1;
        }
        QModelIndex index = m_listWidget->model()->index(row, 0);
        m_listWidget->setCurrentIndex(index);
    } else if (Qt::Key_Escape == key) {
        // 按下Esc键时，隐藏完成列表
    } else if (Qt::Key_Enter == key || Qt::Key_Return == key) {
        // 按下回车键时，使用完成列表中选中的项，并隐藏完成列表
        if (currentIndex.isValid()) {
            QString text = m_listWidget->currentIndex().data().toString();
            // 选中视频源item
            m_onSreachMode = false;
            m_inputEdit->clear();
            emit selectItem(text);
        }
    }

    QWidget::keyPressEvent(e);
}

void OSDMeun::initPageNumList()
{
    pageNumList = new QMenu(this);
    pageNumList->setFixedSize(g_nButtonWidth * g_fScaleScreen,g_nButtonHeight * g_fScaleScreen);

    QWidgetAction *action_1 = new QWidgetAction(pageNumList);
    QWidgetAction *action_2 = new QWidgetAction(pageNumList);
    QWidgetAction *action_3 = new QWidgetAction(pageNumList);


    action_1->setText("5");
    action_2->setText("6");
    action_3->setText("7");


    QLabel *label_1 = new QLabel("5");
    QLabel *label_2 = new QLabel("6");
    QLabel *label_3 = new QLabel("7");


    label_1->setAlignment(Qt::AlignCenter);
    label_2->setAlignment(Qt::AlignCenter);
    label_3->setAlignment(Qt::AlignCenter);


    action_1->setDefaultWidget(label_1);
    action_2->setDefaultWidget(label_2);
    action_3->setDefaultWidget(label_3);



    pageNumList->addAction(action_1);
    pageNumList->addAction(action_2);
    pageNumList->addAction(action_3);


    if(g_nScreenWidth < 3840)
    {
        QWidgetAction *action_4 = new QWidgetAction(pageNumList);
        QWidgetAction *action_5 = new QWidgetAction(pageNumList);
        QWidgetAction *action_6 = new QWidgetAction(pageNumList);

        action_4->setText("5");
        action_5->setText("6");
        action_6->setText("7");

        QLabel *label_4 = new QLabel("8");
        QLabel *label_5 = new QLabel("9");
        QLabel *label_6 = new QLabel("10");

        label_4->setAlignment(Qt::AlignCenter);
        label_5->setAlignment(Qt::AlignCenter);
        label_6->setAlignment(Qt::AlignCenter);

        action_4->setDefaultWidget(label_4);
        action_5->setDefaultWidget(label_5);
        action_6->setDefaultWidget(label_6);


        pageNumList->addAction(action_4);
        pageNumList->addAction(action_5);
        pageNumList->addAction(action_6);
    }

    connect(pageNumList,SIGNAL(triggered(QAction *)),this,SLOT(setPageChannels(QAction *)));
}

void OSDMeun::initLayout()
{
    m_Select = new QPushButton("Video Select",this);
    m_Select->setFixedSize(g_nOsdMenuWidth * g_fScaleScreen,g_nButtonHeight * g_fScaleScreen);

    m_Search = new QPushButton("Setting",this);
    m_Search->setFixedSize(g_nButtonWidth * g_fScaleScreen,g_nButtonHeight * g_fScaleScreen);

    m_Page_up = new QPushButton("Page Up",this);
    m_Page_up->setFixedSize(g_nButtonWidth * g_fScaleScreen,g_nButtonHeight * g_fScaleScreen);

    m_Page_down = new QPushButton("Page Down",this);
    m_Page_down->setFixedSize(g_nButtonWidth * g_fScaleScreen,g_nButtonHeight * g_fScaleScreen);

    m_Apply = new QPushButton("Apply",this);
    m_Apply->setFixedSize(g_nButtonWidth * g_fScaleScreen,g_nButtonHeight * g_fScaleScreen);

    m_Exit = new QPushButton("Exit",this);
    m_Exit->setFixedSize(g_nButtonWidth * g_fScaleScreen,g_nButtonHeight * g_fScaleScreen);



    m_inputEdit = new QLineEdit(this);
    m_inputEdit->setFixedSize(g_nButtonWidth * g_fScaleScreen,g_nButtonHeight * g_fScaleScreen);


    m_listWidget = new QListWidget(this);
    m_listWidget->setFixedSize(g_nOsdMenuWidth * g_fScaleScreen,(m_pageChannels * g_nItemRowHeight) * g_fScaleScreen);
    m_listWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 设置字体大小
    m_font = loadFontSize();
    m_listWidget->setFont(m_font);

    // 初始化设置页面频道数
    initPageNumList();

    // 初始化界面大小
    initButtonStyle();


    m_Search->setMenu(pageNumList);


    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    layout->addWidget(m_inputEdit);
    layout->addWidget(m_Search);
    layout->setStretch(2,1);

    m_mainLayout = new QGridLayout(this);
    m_mainLayout->setContentsMargins(0,0,0,0);
    m_mainLayout->setSpacing(0);
    //m_mainLayout->setVerticalSpacing(0);

    m_mainLayout->addWidget(m_Select,0,0,1,2);
    m_mainLayout->addLayout(layout,1,0,1,2);
    m_mainLayout->addWidget(m_listWidget,2,0,1,2);
    m_mainLayout->addWidget(m_Page_up,3,0,1,1);
    m_mainLayout->addWidget(m_Page_down,3,1,1,1);
    m_mainLayout->addWidget(m_Apply,4,0,1,1);
    m_mainLayout->addWidget(m_Exit,4,1,1,1);

    this->setLayout(m_mainLayout);
    qDebug() << "setLayout  finished";
}

void OSDMeun::styleSheetControl(QPushButton *button)
{
    button->setStyleSheet("color:white");
}

void OSDMeun::initButtonStyle()
{
    styleSheetControl(m_Select);
    styleSheetControl(m_Search);
    styleSheetControl(m_Page_up);
    styleSheetControl(m_Page_down);
    styleSheetControl(m_Apply);
    styleSheetControl(m_Exit);
    pageNumList->setStyleSheet("color:white");
}

void OSDMeun::initConnect()
{
    //  输入框
    connect(m_inputEdit, SIGNAL(textChanged(const QString &)), this, SLOT(setCompleter(const QString &)));
    connect(m_listWidget, SIGNAL(clicked(const QModelIndex &)), this, SLOT(completeText(const QModelIndex &)));

    //  翻页
    connect(m_Page_up,SIGNAL(clicked()),this,SLOT(pageUpClicked()));
    connect(m_Page_down,SIGNAL(clicked()),this,SLOT(pageDownClicked()));

    connect(&osdOverTimer,SIGNAL(timeout()),this,SLOT(exitClicked()));

    connect(m_Exit,SIGNAL(clicked()),this,SLOT(exitClicked()));
    qDebug() << "connect   finished";
}


void OSDMeun::setCompleter(const QString &text)
{
    qDebug() << "text:" << text;

    m_onSreachMode = true;

    // 清空搜索链表
    m_sreachList.clear();

    // 如果完整的完成列表中的某个单词包含输入的文本，则加入要显示的完成列表串中
    foreach(QString word, m_channelList)
    {
        qDebug() << "word:" << word;
        if (word.contains(text, Qt::CaseInsensitive))
        {
            m_sreachList << word;
        }
    }
    qDebug() << "words:" << m_sreachList;

    loadChannel(m_sreachList);

    if(text.isEmpty())
    {
        m_onSreachMode = false;
        loadChannel(m_channelList);
    }

    if (m_listWidget->count() == 0)
        return;
}


QString OSDMeun::omittedString(QString i_str ,QString o_str)
{
    qDebug() << "i_str.length():" <<i_str.length() << "i_str:" << i_str;
    if(i_str.length() <= 8)
    {
        o_str = i_str;
        return o_str;
    }
    //  计算前八个字符的像素宽度
    QString tmpstr = i_str.left(8);
    o_str = tmpstr + "...";
    return o_str;
}

QString OSDMeun::restoreString()
{
    QString orderStr;
    // 定位选中的行，结合当前页面模式和位置定位原字符串
    int strPos = (m_currentPage-1) * m_pageChannels + m_listWidget->currentRow();

    if(strPos < 0)
        return NULL;

    if(m_onSreachMode)
    {
        if(!m_sreachList.isEmpty())
        orderStr = m_sreachList.at(strPos);
    }
    else
    {
        if(!m_channelList.isEmpty())
            orderStr = m_channelList.at(strPos);
    }

    qDebug() << "restoreString:" << orderStr;
    return orderStr;
}

void OSDMeun::updateListWidget(int pageChannels)
{
    if(pageChannels < 5 || pageChannels >10)
    return;

    m_listWidget->setFixedHeight(g_nItemRowHeight * g_fScaleScreen * pageChannels);
    //  发送信号通知更新菜单高度
    emit updateOsdMenu();
}

int OSDMeun::parseOSDPos(QString positon)
{
    if(positon.isEmpty())
        return TOP_MID;

    QString str = positon;
    qDebug() << "parseOSDPos::str:" << str;

    int pos = -1;
    qDebug() << "parseOSDPos::positon:" << positon;
    if(str.compare("top_center",Qt::CaseInsensitive) == 0)
    {
        pos = TOP_MID;
    }
    else if(str.compare("top_left",Qt::CaseInsensitive) == 0)
    {
        pos = TOP_LEFT;
    }
    else if(str.compare("top_right",Qt::CaseInsensitive) == 0)
    {
        pos = TOP_RIGTH;
    }
    else if(str.compare("bottom_left",Qt::CaseInsensitive) == 0)
    {
        pos = BOTTOM_LEFT;
    }
    else if(str.compare("bottom_center",Qt::CaseInsensitive) == 0)
    {
        pos = BOTTOM_MID;
    }
    else if(str.compare("bottom_right",Qt::CaseInsensitive) == 0)
    {
        pos = BOTTOM_RIGHT;
    }
    else if(str.compare("left",Qt::CaseInsensitive) == 0)
    {
        pos = LEFT_MID;
    }
    else if(str.compare("center",Qt::CaseInsensitive) == 0)
    {
        pos = CENTER;
    }
    else if(str.compare("right",Qt::CaseInsensitive) == 0)
    {
        pos = RIGHT_MID;
    }

    return pos;
}

void OSDMeun::parseMeunJson(QString jsonpath)
{
    if(jsonpath.isEmpty())
        return;

    Json::Reader reader;
    Json::Value root;


    // 从文件中读取
   // string path = jsonpath.toStdString();
    ifstream in(MENUINFO_PATH,ios::binary);

    if(!in.is_open())
    {
        qDebug() << "openfile failed";
        return;
    }
    qDebug() << "Start Parse OSDjson";

    if(reader.parse(in,root))
    {
        // 获取节点信息
        int timeout = root["channel_menu"]["timeout_sec"].asInt();
        string pos = root["channel_menu"]["position"].asString();
        string size = root["channel_menu"]["font_size"].asString();
        int channelnum = root["channel_menu"]["max_channels_per_page"].asInt();
        int maxchannelnum = root["channel_menu"]["max_channels"].asInt();

        cout << "size:" << size << endl;

        QString str = size.c_str();
        qDebug() << "str:" << str;
        int fontsize;
        if(str.compare("small") == 0)
        {
            fontsize = Small;
        }
        else if(str.compare("medium") == 0)
        {
            fontsize = Mid;
        }
        else if(str.compare("large") == 0)
        {
            fontsize = Big;
        }

        QString Strpos = pos.c_str();
        qDebug() << "Strpos:" << Strpos;
        m_showPosition = parseOSDPos(Strpos);

        cout << "timeout:" << timeout << endl;
        cout << "pos:" << pos << endl;
        cout << "fontsize:" << fontsize << endl;
        cout << "channelnum:" << channelnum << endl;
        cout << "maxchannelnum:" << maxchannelnum << endl;

        m_overTime = timeout * 1000;        // 转为秒数

        m_FontSize = fontsize;
        if(g_nScreenWidth == 3840)
        {
            if(channelnum > 7)
                m_pageChannels = 7;
            else
                m_pageChannels = channelnum;
        }
        else
        {
            m_pageChannels = channelnum;
        }

        m_maxChannels = maxchannelnum;

        qDebug() << "m_overTime:" << m_overTime;
        qDebug() << "m_showPosition:" << m_showPosition;
        qDebug() << "m_FontSize:" << m_FontSize;
        qDebug() << "m_pageChannels:" << m_pageChannels;
        qDebug() << "m_maxChannels:" << m_maxChannels;

        // 这里应该根据页面频道数更新QListWidget的高度， 更新后同时 发送信号通知菜单更新
        //updateListWidget(m_pageChannels);
    }
}

QFont OSDMeun::loadFontSize()
{
    QFont font;
    switch (m_FontSize)
    {
        case Small:
        {
            m_FontSize = 10 * ((float)g_nScreenWidth/g_nStdScreenWidth);
            font.setPointSize(m_FontSize);
            font.setBold(true);
            font.setWeight(50 * g_fScaleScreen);
            break;
        }
        case Mid:
        {
            m_FontSize = 15 * ((float)g_nScreenWidth/g_nStdScreenWidth);
            font.setPointSize(m_FontSize);
            font.setBold(true);
            font.setWeight(50 * g_fScaleScreen);
            break;
        }
        case Big:
        {
            m_FontSize = 20 * ((float)g_nScreenWidth/g_nStdScreenWidth);
            font.setPointSize(m_FontSize);
            font.setBold(true);
            font.setWeight(50 * g_fScaleScreen);
            break;
        }
    }

    // 抗锯齿
    font.setStyleStrategy(QFont::PreferAntialias);//QFont::PreferAntialias	0x0080	antialias if possible.
    font.setStyleStrategy(QFont::NoAntialias);//QFont::NoAntialias	0x0100	don't antialias the fonts.
    return font;
}

void OSDMeun::parseChannelJson()
{
    Json::Reader reader;
    Json::Value root;


    // 从文件中读取
    //string path = jsonpath.toStdString();
    ifstream in(CHANNELS_LIST_PATH,ios::binary);

    if(!in.is_open())
    {
        qDebug() << "openfile failed";
        return;
    }

    qDebug() << "list_1";

    int channelId = 0;
    string channelName;

    if(reader.parse(in,root))
    {
        // 读取根节点信息

        // 读取子节点信息

        // 读取数组信息
         qDebug() << "list_2";
        if(root["channels_list"].isArray())
        {
            int arraySize = root["channels_list"].size();
            qDebug() << "list_3" << arraySize;
            //  最大频道数
            if(arraySize > m_maxChannels)
                arraySize = m_maxChannels;

            for (int i =0 ;i < arraySize; i++)
            {
                channelId = root["channels_list"][i]["id"].asInt();
                channelName = root["channels_list"][i]["name"].asString();


                qDebug() << "id is" << channelId;
                qDebug() << "channelname is" << channelName.c_str();

                if(!channelMap.contains(channelId))
                {
                    channelMap[channelId] = channelName.c_str();
                }
            }
        }

        if(channelMap.isEmpty())
        {
            qDebug() << "channelMap is NULL" << channelId;
            return;
        }

        // 放入链表
        QMap<int,QString>::iterator it = channelMap.begin();
        while(it != channelMap.end())
        {
            m_channelList <<  it.value();
            it++;
        }
        qDebug() << "list_4";
        // 排序
        QSet<QString> set =  m_channelList.toSet();
        QList<QString> list = QList<QString>::fromSet(set);


        qSort(list);
        m_channelList = list;
        qDebug() << "m_channelList = " << m_channelList;

        qDebug() << "list_5";
        // 频道加载
        loadChannel(m_channelList);
    }
}


void OSDMeun::loadChannel(QStringList channelList)
{
    if(channelList.isEmpty())
    {
        // 清除元素
        for(int index = m_listWidget->count() - 1; index >= 0; index--)
        {
            delete m_listWidget->item(index);
        }
        return;
    }


    int totalChannel = channelList.count();

    int totalpages = (totalChannel % m_pageChannels)?(totalChannel/m_pageChannels + 1):totalChannel/m_pageChannels;

    qDebug() << "load_1" << totalpages;

    if(m_currentPage < totalpages)
    {
        m_Page_down->setEnabled(true);
    }
    else
    {
        m_Page_down->setEnabled(false);
    }

    if(m_currentPage > 1)
    {
        m_Page_up->setEnabled(true);
    }
    else
    {
        m_Page_up->setEnabled(false);
    }

    // 清除元素
    for(int index = m_listWidget->count() - 1; index >= 0; index--)
    {
        delete m_listWidget->item(index);
    }

    qDebug() << "load_2" << m_listWidget->count();

    // 防止越界
    if(m_currentPage > totalpages)
    {
        m_currentPage = totalpages;
    }

    int startindex = (m_currentPage - 1) * m_pageChannels;
    int endindex = (m_currentPage == totalpages)?totalChannel : m_currentPage*m_pageChannels;

//    // 设置 字体大小
//    m_font = loadFontSize();
    qDebug() << "load_3" <<  endindex;

    int rowIndex = 0;
    for(int index = startindex; index < endindex; index++)
    {
        QString channelName = channelList.at(index);
        QString displayStr;
        channelName = omittedString(channelName,displayStr);
//        qDebug() << "displayStr:" << displayStr;
        QListWidgetItem *item = new QListWidgetItem(m_listWidget);
        item->setSizeHint(QSize(200 * g_fScaleScreen,30 * g_fScaleScreen));
        item->setText(channelName);
        item->setTextColor(Qt::white);
        m_listWidget->insertItem(rowIndex,item);      
    }
   // m_listWidget->setFont(m_font);
    qDebug() << "load_4" <<  m_listWidget->count();
}

void OSDMeun::completeText(const QModelIndex &index)
{
    QString text = index.data().toString();
    qDebug() << "text:" << text;
    QString itemtext = restoreString();
    qDebug() << "itemtext:" << itemtext;


    QMap<int,QString>::iterator it = channelMap.begin();
    while(it != channelMap.end())
    {
        qDebug() << "it.value():" << it.value();
        if(it.value().compare(itemtext) == 0)
        {
            m_channelID = it.key();
            break;
        }
        it++;
    }
    qDebug() << "channelid:" << m_channelID;

    g_nChannelId = m_channelID;

    // 获取了点击的频道id, 设置频道切换或发送信号
    QString strCmd = QString("#KDS-CHANNEL-SELECT %1\r").arg(m_channelID);
    QByteArray byteCmd = strCmd.toLatin1();
    if(m_p3ktcp->sendCmdToP3k(byteCmd))
    {
        qDebug("Set ChannelID YES");
    }
}

void OSDMeun::pageUpClicked()
{
    if(m_onSreachMode)
    {
        m_currentPage--;
        loadChannel(m_sreachList);
    }
    else
    {
        m_currentPage--;
        loadChannel(m_channelList);
    }
}

void  OSDMeun::pageDownClicked()
{
    if(m_onSreachMode)
    {
        m_currentPage++;
        loadChannel(m_sreachList);
    }
    else
    {
        m_currentPage++;
        loadChannel(m_channelList);
    }
}

void OSDMeun::exitClicked()
{
    // 发送信号隐藏菜单
    osdOverTimer.stop();
    emit hideOsdMenu();
}

void OSDMeun::setPageChannels(QAction *action)
{
    int nums = action->text().toInt();
    qDebug() << "pagenums:" << nums;
    m_pageChannels = nums;

    if(m_onSreachMode)
    {
        loadChannel(m_sreachList);
    }
    else
    {
        loadChannel(m_channelList);
    }

    m_listWidget->setFixedHeight(g_nItemRowHeight * g_fScaleScreen * m_pageChannels);
    //  发送信号通知更新菜单高度
    emit updateOsdMenu();
}

 int OSDMeun::getOSDMeunHeight()
 {
     return g_nButtonHeight*4 + g_nItemRowHeight * m_pageChannels;
 }
