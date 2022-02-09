#ifndef GUIDEPANEL_H
#define GUIDEPANEL_H

#include "modules/modulepanel.h"

#include <QPixmap>
#include <QImage>
#include <QFileSystemWatcher>
#include <QLabel>

class SleepPanel : public ModulePanel
{
    Q_OBJECT
public:
    explicit SleepPanel(const QString &pic, QWidget *parent = NULL);

//    QString moduleName();
//    QWidget *toolbarBottomContent();
//    QWidget *toolbarTopContent();

	void setGuideImage(const QString &file);
protected:
    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *event);

private:
	//QPixmap m_scaledPixmap;
	QImage	m_scaledImage;
	QString m_bgPath;
};

#endif // GUIDEPANEL_H

