#ifndef MODULEPANEL_H
#define MODULEPANEL_H

#include <QDebug>
#include <QFile>
#include <QFrame>

class ModulePanel : public QFrame
{
    Q_OBJECT
public:
    ModulePanel(QWidget *parent = NULL)
        : QFrame(parent)
    {
//        connect(dApp->signalM, SIGNAL(gotoPanel(ModulePanel *)),
//                this, SLOT(showPanelEvent(ModulePanel *)));
    }
    virtual bool isMainPanel() {return false;}
//    virtual QString moduleName() = 0;
//    virtual QWidget *toolbarBottomContent() = 0;
//    virtual QWidget *toolbarTopContent() = 0;

private slots:
//    virtual void showPanelEvent(ModulePanel *p)
//    {
//        if (p != this)
//            return;
//    }
};

#endif // MODULEPANEL_H

