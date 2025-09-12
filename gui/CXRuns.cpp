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
#include "CXRuns.h"
#include <QLabel>
#include <QSpinBox>
#include <QListWidget>


#include <QWidget>
#include <QHBoxLayout>
#include <QFrame>
#include "CApp.h"
#include "CAStream.h"

extern CApp * App;

CXRuns::CXRuns(QWidget * parent):
    QDockWidget("XRuns", parent),
    Plays(0)

{
    Frame = new QFrame(this);
    RCount = new QLabel(this);
    QFont f = RCount->font();
    f.setBold(true);
    RCount->setFont(f);
    PCount = new QLabel(this);
    
    PrePlayLabel = new QLabel(this);
    PrePlayLabel->setText("Pre sec");
    
    PrePlay = new QSpinBox(this);
    PrePlay->setMaximumWidth(35);
    PrePlay->setValue(4);
    
    List = new QListWidget(this);

    QHBoxLayout* pre = new QHBoxLayout;
    pre->addWidget(PrePlayLabel);
    pre->addWidget(PrePlay);
    pre->addSpacing(1);
    
    QVBoxLayout* vl = new QVBoxLayout;
    vl->addWidget(PCount);
    vl->addWidget(RCount);
    vl->addWidget(List);
    vl->addLayout(pre);
    vl->setSpacing(0);
    setMaximumWidth(100);
    setMinimumWidth(40);
    connect (List, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(play_xrun(QListWidgetItem*)));
    
    
    Frame->setLayout(vl);
    setWidget(Frame);
    set_counts();
}

void CXRuns::set_counts()
{
    QString str;
    str =  "Rec: %1";
    RCount->setText(str.arg(List->count()));
    str =  "Play: %1";
    PCount->setText(str.arg(Plays));
    
    
}

void CXRuns::Add(int segment, bool record)
{
    QString str;
    if (record)
    {
        double seconds = CAStream::SegmentsToSec(segment);
        CAStream::SecsToTimeString(seconds, str);
        List->addItem(str);
    }
    else
        ++Plays;
    set_counts();        
}


void CXRuns::Clear()
{
    Plays = 0;
    List->clear();
    set_counts();
}

CXRuns::~CXRuns()
{
}


void CXRuns::play_xrun(QListWidgetItem* item)
{
    if (App->Play())
    {
        double secs = CAStream::TimeStringToSecs(item->text());
        secs -= PrePlay->value();
        App->Play()->Cue(secs, false);
    }
}
