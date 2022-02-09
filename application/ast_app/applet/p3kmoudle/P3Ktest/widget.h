#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "cp3kcmd.h"
namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_ReadMessage(QString);
    void on_GetSendMsgNum(int);
    void on_GetRecvMsgNum(int);
    void on_button_link_clicked();

    void on_button_all_clicked();



    void on_tableWidget_cellDoubleClicked(int row, int column);

    void on_textBrowser_textChanged();

    void on_button_clear_clicked();

private:
    Ui::Widget *ui;
    Cp3kcmd *s_cmdMod;
    void AddCmdToTable(QString cmd);

};

#endif // WIDGET_H
