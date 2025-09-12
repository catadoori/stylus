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

#include "CTime.h"
#include "CSettings.h"

#include <QPainter>
#include <QTime>
#include <QDebug>
#include "CApp.h"

extern CApp * App;

CTime::CTime(QWidget * parent):
    QWidget(parent),
    Msh(true)
{
    Format = "%1:%2.%3";      
    setAutoFillBackground(true);
    if (App->Settings.Gui.FontSizes.Times() != 0.0)
    {
        QFont f = font();
        f.setPointSizeF(App->Settings.Gui.FontSizes.Times());
        setFont(f);
    }        
    Set(0);
    Clear();
    setMinimumWidth(fontMetrics().boundingRect("XX:XX.XX").width());
}

CTime::~CTime()
{
}

void CTime::paintEvent(QPaintEvent * )
{
    QPainter painter (this);
    if (Time != OldTime || OldTime == 0.0)
    {
        QChar fill('0');
        if (Msh)
        {
            QTime time = QTime(0, 0, 0).addMSecs (Time * 1000);
            String = Format.arg(time.minute(), 2, 10, fill).
                            arg(time.second(), 2, 10, fill).
                            arg(time.msec() / 10, 2, 10, fill);
        }
        else
        {
            String = QString("%1s").arg(Time, 0, 'f', 2);
        }
        OldTime = Time;
    }
    painter.drawText(rect(), Qt::AlignVCenter, String);        
}
        
void  CTime::mouseDoubleClickEvent  (QMouseEvent *)
{
    Msh = !Msh;
}

