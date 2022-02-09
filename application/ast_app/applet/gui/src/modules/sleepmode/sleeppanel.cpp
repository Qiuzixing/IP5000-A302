#include "sleeppanel.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QDebug>


SleepPanel::SleepPanel(const QString &pic, QWidget *parent)
    : ModulePanel(parent)
{
    setMouseTracking(true);

    setGuideImage(pic);
}

void SleepPanel::setGuideImage(const QString &file)
{
    if (file.isEmpty())
		return;

    m_bgPath = file;
	m_scaledImage = QImage(m_bgPath);
	update();
}

void SleepPanel::paintEvent(QPaintEvent *event)
{
	if (m_scaledImage.isNull())
		return;

    QPainter *painter = new QPainter(this);
    // painter->setRenderHint(QPainter::Antialiasing);

    //painter.drawPixmap(this->rect(), m_scaledPixmap);
    
    painter->drawImage(this->rect(), m_scaledImage, m_scaledImage.rect());
    painter->end();
    qDebug() << "SLEEP PAINTER FINISHED";
}

void SleepPanel::mouseReleaseEvent(QMouseEvent *event)
{

}
