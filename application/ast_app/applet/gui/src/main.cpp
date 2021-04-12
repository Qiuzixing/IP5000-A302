/*
 * Copyright (c) 2004-2012 ASPEED Technology Inc.
 * Based on:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <QtGui/QApplication>
#include "dialog.h"
#include <QtPlugin>


int main(int argc, char *argv[])
{
    Q_IMPORT_PLUGIN(qjpeg)

    QApplication a(argc, argv);
    //For static link program, Q_INIT_RESOURCE is a must
    Q_INIT_RESOURCE(resource);
    Dialog w;
    QBrush brush;
    QPalette palette = w.palette();
    //QPixmap px(":/jpg/background.jpg");
    if(QApplication::arguments().contains(QString(BARUI), Qt::CaseInsensitive))
    {
        brush = QBrush(QColor(QString("royalblue")));
    }
    else
    {
        brush = QBrush(QColor(QString("black")));
    }
    palette.setBrush(QPalette::Window, brush);
    palette.setColor(QPalette::WindowText, QColor(QString("white")));
    w.setPalette(palette);
    QFont fnt(QApplication::font());
    fnt.setPointSize(8);
    w.setFont(fnt);
    //w.setWindowTitle(QString("HOST Selection"));
    //w.setFixedSize(640, 480); //Moved to Dialog constructor
    w.show();
    return a.exec();
}


