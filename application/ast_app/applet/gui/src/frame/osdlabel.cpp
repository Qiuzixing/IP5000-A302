#include "osdlabel.h"
#include "common/global.h"
#include "common/scale_global.h"

#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QObject>

#include <QScreen>
#include <QPixmap>
#include <QDesktopWidget>
#include <QApplication>
//#include <QGuiApplication>
#include <QImage>

OSDLabel::OSDLabel(const QString& text, quint8 fontSize, quint32 fontColor, QWidget *parent)
    : QFrame(parent)
    ,m_bLongDisplay(false)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10* g_fScaleScreen, 2* g_fScaleScreen,
                               10* g_fScaleScreen, 2* g_fScaleScreen);

    textLabel = new QLabel(text);
    layout->addWidget(textLabel);
    QFont font;

    font.setPointSize(fontSize);
    setFont(font);

    QPalette palette;
    int a = (fontColor >> 24) & 0xff;
    int r = (fontColor >> 16) & 0xff;
    int g = (fontColor >> 8) & 0xff;
    int b = (fontColor) & 0xff;
    if (!a) a = 0xff;

    r = setRGB(r);
    g = setRGB(g);
    b = setRGB(b);

    m_coverBrush = QColor(0, 0, 0, 0);

    qDebug() << "g_Transparency:" << g_Transparency;

    palette.setColor(QPalette::WindowText, QColor(r, g, b, a));
    setPalette(palette);

    connect(&displayerTimer,SIGNAL(timeout()),this,SLOT(overTimer()));
}


OSDLabel::OSDLabel(const QString &imagePath,int width,int height,QWidget *parent)
    :QFrame(parent)
    ,imagePath(imagePath)
    ,m_bLongDisplay(false)
{
    QPixmap pix;
    if(pix.load(imagePath))
    {
        qDebug() << "load finished";
    }

    int widthR = pix.width() * g_fScaleScreen;
    int heightR = pix.height() * g_fScaleScreen;

    qDebug() << "widthR:" << widthR << "heightR:" << heightR << "g_fScaleScreen:" << g_fScaleScreen;

    // image
    QRect imageRect;
    imageRect.setX(0);
    imageRect.setY(0);
    imageRect.setWidth(widthR);
    imageRect.setHeight(heightR);
    imageLabel = new QLabel(this);
    imageLabel->setGeometry(imageRect);

    QPixmap fixpix = pix.scaled(widthR, heightR, Qt::KeepAspectRatio, Qt::SmoothTransformation); // ???????????????
    imageLabel->setPixmap(fixpix);

    connect(&displayerTimer,SIGNAL(timeout()),this,SLOT(overTimer()));
}

OSDLabel::~OSDLabel()
{

}

void OSDLabel::startTimer()
{
    displayerTimer.start(m_overTime);
}

void OSDLabel::stopTimer()
{
    displayerTimer.stop();
}

void OSDLabel::overTimer()
{
    qDebug() << "overlay timeout hide";
    this->hide();
    //emit osdOvertime();
    displayerTimer.stop();
}

void OSDLabel::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QRect insideRect;
    insideRect.setRect(this->rect().x(),
                       this->rect().y(),
                       this->rect().width(),
                       this->rect().height());
    QPainterPath ip;
    ip.addRoundedRect(insideRect, 10 * g_fScaleScreen, 10 * g_fScaleScreen);
    p.setClipPath(ip);

    p.fillRect(0, 0, width(), height(), m_coverBrush);
    p.end();
}

QString OSDLabel::text() const
{
    return textLabel->text();
}

void OSDLabel::setText(const QString &text)
{
    textLabel->setText(text);
}
