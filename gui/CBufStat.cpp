/***************************************************************************
 *   Copyright (C) 2007 by Harry   *
 *   harry@actionpussyclub.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "CBufStat.h"
#include <QPainter>
#include <QEvent>

#include <QDebug>

CBufStat::CBufStat(QWidget * parent, bool rec):
    QWidget(parent)

{
    Used = QColor (rec ? Qt::darkRed : Qt::darkGreen);
    Free = QColor(rec ? Qt::red : Qt::green);
    Window = QPalette().color(QPalette::Window);
    setMinimumWidth(5);
    Refresh(0);
}


CBufStat::~CBufStat()
{
}

void CBufStat::Refresh(int grade)
{
    Grade = grade;
#if 0
    update();
#else    
    repaint();
#endif    

}

void CBufStat::paintEvent(QPaintEvent *)
{
    int free = ((height() * Grade) / 100) ;
    int used = height() - free;
    QPainter painter(this);
    painter.setBrush(isEnabled() ? Used : Window);
    painter.setPen(QPen(Used, 0.0, Qt::NoPen));
    painter.drawRect(0, 0, width(), used);
    painter.setBrush(isEnabled() ? Free : Window);
    painter.drawRect(0, height() - free, width(), free);
}


QSize CBufStat::sizeHint () const
{
    QSize ret(10, 0);
    return ret;
}

