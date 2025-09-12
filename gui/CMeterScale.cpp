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
#include "CMeterScale.h"
#include <QPainter>
#include <QDebug>

int const CMeterScale::FineDBs = 30; // 5 db step
int const CMeterScale::Ticks = 50;
int const CMeterScale::Offset = 4;
double const CMeterScale::MinScale = -70.0;

double CMeterScale::PixelPerDbFine;


CMeterScale::CMeterScale(QWidget * parent):
    QWidget(parent)
{
    QFont f = font();
    f.setPointSizeF(6.0);
    setFont(f);
    setMaximumWidth(5 + 5 + fontMetrics().boundingRect("XX").width());
}


CMeterScale::~CMeterScale()
{
}

 
void CMeterScale::resizeEvent(QResizeEvent * )
{
    PixelPerDbFine = double(height()) / Ticks;
}

void CMeterScale::paintEvent(QPaintEvent * )
{
    QPainter painter(this);
    int db = 0;
    int ticklength;
    for (int tick = 0; tick  <= Ticks; ++tick)
    {
        int ypos = qRound(PixelPerDbFine * tick);
        if ((tick && (db <= FineDBs && ((db % 5) == 0))) || ((db > FineDBs) && ((db % 10) == 0)))
        {
            painter.drawText(10, Offset + ypos, QString("%1").arg(db));
            ticklength = 5;
        }
        else
            ticklength = 2;
        painter.drawLine (0, ypos, ticklength, ypos);
        
        if (tick >= FineDBs)
            db += 2;
        else
            ++db;

    }
}
