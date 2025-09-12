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
#include "CMeter.h"
#include <QPixmap>
#include <QLinearGradient>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

#include "CMeterLED.h"
#include "CMeterBar.h"



CMeter::CMeter(QWidget * parent, char const * usage):
    QWidget(parent)
{
    QVBoxLayout* vl = new QVBoxLayout;
    vl->setSpacing(3);

// FIXME    vl->setMargin(2);
    QHBoxLayout * hl = new QHBoxLayout;
    Label = new QLabel;
    Label->setText(usage);
    Label->setMargin(1);
    QFont f = Label->font(); f.setPointSizeF(7.0);
    Label->setFont(f);
    Label->setMaximumHeight(Label->fontMetrics().height());
    Label->setAlignment(Qt::AlignCenter);
 // FIXME   hl->setMargin(2);
    hl->setSpacing(3);
    hl->addLayout(init_channel(Left));
    hl->addLayout(init_channel(Right));
    vl->addLayout(hl);
    vl->addWidget(Label);

    setLayout(vl);

}

QRect CMeter::BarGeometry() const
{
    return Left.Bar->geometry();
}

QVBoxLayout * CMeter::init_channel(tChannel & ch)
{
   ch.LED = new CMeterLED(this);
   ch.Bar = new CMeterBar(this);

   QVBoxLayout * ret = new QVBoxLayout;
   ret->addWidget(ch.LED);
   ret->addWidget(ch.Bar);
   return ret;
}

void CMeter::Refresh()
{
     Left.Bar->Refresh(Data.RMS.Left, Data.Peak.Left);
     Right.Bar->Refresh(Data.RMS.Right, Data.Peak.Right);
}


CMeter::~CMeter()
{

}

#if 0
void CMeter::resizeEvent(QResizeEvent * event)
{
   QWidget::resizeEvent(event);
}
#endif

#if 0
void CMeter::paintEvent(QPaintEvent * event)
{
#if 1
    QPixmap pm (size());
    QPainter p (&pm);
    QLinearGradient gr(0, 0, 0, height());

    gr.setColorAt(1, QColor(Qt::darkGreen));
    gr.setColorAt(0.4, QColor(Qt::darkYellow));
    gr.setColorAt(0, QColor(Qt::red));

    p.fillRect(0, 0, width(), height(), gr);

    QPainter p1(this);
    p1.drawPixmap(0,0,pm);
    
    
#else
    QPixmap pm (size());
    QPainter p (&pm);
    QLinearGradient gr(0, 0, 0, height());

    gr.setColorAt(1, QColor(Qt::darkGreen));
    gr.setColorAt(0.4, QColor(Qt::darkYellow));
    gr.setColorAt(0, QColor(Qt::red));

    p.fillRect(0, 0, width(), height(), gr);

    QPainter p1(this);
    p1.drawPixmap(0,0,pm);

#endif
}
#endif

#if 0

+-----------------------+  <-- CMeters::Frame
|         #####  #####  |  <-- CMeterLED
|   0  - +----+  +----+ | 
|      - |    |  |    | |                   |
|      - |    |  |    | |                   |
|      - |    |  |    | |
|  -5 -- |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
| -10 -- |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
| -15 -- |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
| -20 -- |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
| -30 -- |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
| -40 -- |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
| -50 -- |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |
|      - |    |  |    | |                  |
|      - |    |  |    | |                  |
| -60 -- +----+  +----+ | 
|         12.3    12.3  |
|          -- Play --   |
+-----------------------+
         |            | 
         |            |
         +-- CMeter --+   
            




#endif
