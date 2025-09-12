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
#include "CMeters.h"
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "CMeter.h"
#include "CMeterScale.h"
#include <QSpacerItem>

#include "CApp.h"
#include "CAStream.h"

extern CApp * App;

#include <QDebug>
CMeters::CMeters(QWidget * parent):
   QDockWidget("VU Meters",parent),
   Show(1)
{
    Frame = new QFrame(this);
    Scale = new CMeterScale(Frame);
    Play = new CMeter(Frame, "Play");
    Record = new CMeter(Frame, "Rec");
    Record->hide();
    QHBoxLayout * hl = new QHBoxLayout;
    hl->addWidget(Play);
    hl->addWidget(Record);
    QVBoxLayout * vl = new QVBoxLayout;

    vl->addSpacing(Play->BarGeometry().y());
    ScaleTopSpace = dynamic_cast<QSpacerItem*> (vl->itemAt(0));
    vl->addWidget(Scale);
    vl->addSpacing(0);
    ScaleBottomSpace = dynamic_cast<QSpacerItem*> (vl->itemAt(2));
    hl->addLayout(vl);
    // FIXME hl->setMargin(0);
    Frame->setLayout(hl);

    setWidget(Frame);
    setMinimumWidth(50);
}


CMeters::~CMeters()
{
}


QSize CMeters::sizeHint() const
{
    return QSize(80,20);
}

void CMeters::Refresh()
{

    if (App->Play())
        Play->Refresh();
    if (App->Rec())
        Record->Refresh();
}

void CMeters::resizeEvent(QResizeEvent * e)
{
    if (width() < 1)
    {
        Scale->setVisible(false);
    }
    else
    {
        Scale->setVisible(true);
        ScaleTopSpace->changeSize(0, Play->BarGeometry().y());
        ScaleBottomSpace->changeSize(0, Frame->geometry().height() -
                                        Play->BarGeometry().height() -
                                        Play->BarGeometry().y() );

        Frame->layout()->invalidate();
    }
    update();
    QDockWidget::resizeEvent(e);
}

void CMeters::Toggle(bool init)
{
   if (init)
       Show = 0; 
   if (++Show == 4 || ! App->Rec())
       Show = 1;

    Play->setVisible(Show & 1);
    Record->setVisible(Show & 2);
}
