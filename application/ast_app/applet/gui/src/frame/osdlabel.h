#ifndef OSDLABEL_H
#define OSDLABEL_H

#include <QFrame>
#include <QLabel>
#include <QTimer>

#define DEFAULET_TINEOUT 20*1000

#define FONTSIZE_BIG 30
#define FONTSIZE_MID 20
#define FONTSIZE_SMALL 15

class OSDLabel : public QFrame
{
    Q_OBJECT
public:
    // 显示文本的构造函数
    explicit OSDLabel(const QString& text,
                      quint8 fontSize,
                      quint32 fontColor,    //ARGB8888
                      QWidget *parent = NULL);

    // 显示图像的构造函数
    explicit OSDLabel(const QString &imagePath,
                      int width,
                      int height,
                      QWidget *parent = NULL);

    ~OSDLabel();

    QString text() const;
    void startTimer();
    void stopTimer();
    void setoverTime(int msec = DEFAULET_TINEOUT){m_overTime = msec;}

    void setLongDisplay(bool status){ m_bLongDisplay = status;}
    bool isLongDisplay(){return m_bLongDisplay;}

    void setShowPos(int pos){m_showPos = pos;}
    int getShowPos(){return m_showPos;}

public slots:

signals:
    void osdOvertime();

public Q_SLOTS:
    void setText(const QString &text);
    void overTimer();

protected:
	void paintEvent(QPaintEvent *e);

private:
    QLabel  *textLabel;
    QLabel  *imageLabel;
	QColor m_coverBrush;

    QImage QLabelImage;
    QString imagePath;

    int   m_overTime;
    QTimer displayerTimer;

    bool m_bLongDisplay; // 是否常显 TRUE - 常显
    int m_showPos;
};


#endif // OSDLABEL_H

