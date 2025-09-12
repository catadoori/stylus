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

#include "CGui.h"
#include <QComboBox>
#include "CPTool.h"
#include "CRTool.h"
#include "CApp.h"
#include "CAStream.h"
#include "CTime.h"
#include "CBufStat.h"
#include <QAction>
#include <math.h>
#include <QDebug>
#include "CTEdit.h"


extern CApp * App;


CPTool::CPTool(QWidget * parent):
    CBaseTool(parent, "Playback"),
    Pos(0.0),
    ReadAfterWrite(false)
{
//    connect(Side, SIGNAL(currentIndexChanged(int)), this, SLOT(side_changed(int))); 
    Ctrls.Length = new CTime(parent);
    addWidget(Ctrls.Length);
    addSeparator();
    Ctrls.BufStat = new CBufStat(this, false);
    addWidget(Ctrls.BufStat);
    addSeparator();
    Ctrls.Playing.addFile(":/icons/player_play.png");
    Ctrls.Paused.addFile(":/icons/player_pause.png");
    Ctrls.Begin = new QAction (QIcon(":/icons/player_start.png"), "Begin", this);
    Ctrls.Rwd = new QAction(QIcon(":/icons/player_rew.png"), "Rewind", this);
    Ctrls.Go = new QAction(Ctrls.Playing, "", this);
    Ctrls.Fwd = new QAction(QIcon(":/icons/player_fwd.png"), "Forward", this);
    Ctrls.End = new QAction(QIcon(":/icons/player_end.png"), "End", this);
    Ctrls.Position = new QSlider(Qt::Horizontal, this);
    Ctrls.Position->setMinimumWidth(100);
    connect(Ctrls.Position, SIGNAL(valueChanged(int)), this, SLOT(pos_changed(int)));
    Ctrls.Reload = new QAction(QIcon(":/icons/reload.png"), "Update length", this);
    Ctrls.Reload->setEnabled(false);

    Ctrls.Volume = new QSlider(Qt::Horizontal, this);
    Ctrls.Volume->setMinimumWidth(20);
    Ctrls.Volume->setMinimum(-360); Ctrls.Volume->setMaximum(180);

    Ctrls.Time = new CTime(this);

    addAction(Ctrls.Begin);
    addAction(Ctrls.Rwd);
    addAction(Ctrls.Go);
    addAction(Ctrls.Fwd);
    addAction(Ctrls.End);
    addAction(Ctrls.Reload);

    addSeparator();
    Ctrls.Volume->setMaximumWidth(50);
    addWidget(Ctrls.Volume);
    addSeparator();
    addWidget(Ctrls.Position);
    addSeparator();
    addWidget(Ctrls.Time);
    connect(Ctrls.Go, SIGNAL(triggered()), this, SLOT(Go()));
    connect(Ctrls.Begin, SIGNAL(triggered()), this, SLOT(begin()));
    connect(Ctrls.Fwd, SIGNAL(triggered()), this, SLOT(fwd()));
    connect(Ctrls.Rwd, SIGNAL(triggered()), this, SLOT(rwd()));
    connect(Ctrls.End, SIGNAL(triggered()), this, SLOT(end()));
    connect(Ctrls.Reload, SIGNAL(triggered()), this, SLOT(Requery()));

    connect(Ctrls.Volume, SIGNAL(valueChanged(int)), this, SLOT(vol_changed(int)));
    vol_changed(0);
    set_tooltip(true);

}

CPTool::~CPTool()
{
}

void CPTool::SetData()
{
    double pos = App->Play()->GetPosSec();
    Ctrls.Time->Set(pos);
    Pos = pos;
}

void CPTool::SetVolume(double db)
{
    Ctrls.Volume->setValue(qRound(db * 10));
}

void CPTool::Reset()
{
    Ctrls.Go->setIcon(Ctrls.Playing);
}

void CPTool::SetRAW(bool read_after_write)
{
    ReadAfterWrite = read_after_write;
    if (ReadAfterWrite)
        App->Play()->SetActiveStream(App->Play()->GetMaster());
    else
    {
        App->Play()->SetActiveStream(App->Play());
        Ctrls.Position->setMaximum(qRound(App->Play()->GetLength() * 10));
    }
}

void CPTool::Requery()
{
    bool e = isEnabled();
    double l  = 0.0;
    if (e)
    {
        if (App->Play()->IsSlave() && App->Play()->GetMaster())
        {
            Ctrls.Position->setMaximum(qRound(App->Play()->GetMaster()->GetLength() * 10));
            App->Play()->SetSegmentCount(App->Rec()->GetSegmentCount());
        }
        if (App->Play() && App->Play()->IsGood())
            l = App->Play()->GetLength();
        else
            l = CAStream::Length(GetWavName());
        e = l > 0.0;
    }
    Ctrls.Rwd->setEnabled(e);
    Ctrls.Fwd->setEnabled(e);
    Ctrls.Begin->setEnabled(e);
    Ctrls.End->setEnabled(e);
    Ctrls.Go->setEnabled(e);
    Ctrls.Volume->setEnabled(e);
    Ctrls.Position->setEnabled(e);
    Ctrls.Time->setEnabled(e);
    Ctrls.Reload->setEnabled(e && App->Play()->IsSlave() && !ReadAfterWrite);

    Ctrls.BufStat->Refresh(0);
    Ctrls.BufStat->setEnabled(e);
    Ctrls.Length->setEnabled(e);
    Ctrls.Position->setMaximum(qRound(l * 10));    
    Ctrls.Length->Set(l);
    Ctrls.Length->update();
    vol_changed(Ctrls.Volume->value());
    EnableReload();
    
}

void CPTool::EnableReload()
{
    Ctrls.Reload->setEnabled(App->Gui()->RTool()->Active() &&  App->Play()->IsSlave());
}

void CPTool::Refresh()
{
    static double old_pos = 0.0;
    Ctrls.Time->update();
    
    if (ReadAfterWrite && App->Play()->IsSlave() && App->Play()->GetMaster())
    { 
        Pos = Ctrls.Position->maximum();
        App->Play()->SetSegmentCount(App->Rec()->GetSegmentCount());
    }
    if (fabs(Pos - old_pos) > 0.1)
    {
        Ctrls.Position->blockSignals(true);
        Ctrls.Position->setValue(Pos * 10);
        Ctrls.Position->blockSignals(false);
        Pos = old_pos;            
    }
}



void CPTool::side_changed(int)
{
    App->InitStreams(true, false);
    Requery();
}

void CPTool::Go()
{
    if (!Ctrls.Go->isEnabled()) return;
    if (App->Play()->IsGood())
    {
        if (App->Play()->IsRunning())
        {
            App->Play()->Pause();
            Ctrls.Go->setIcon(Ctrls.Playing);
            set_tooltip(true);
        }
        else
        {
            if (App->Play()->IsAtEnd() && !App->Play()->IsSyncdSlave() )
                App->Play()->Cue(0, false);
            try
            {
                App->Play()->Continue();
                Ctrls.Go->setIcon(Ctrls.Paused);
                set_tooltip(false);
            }
            catch (Except &e)
            {
                App->Gui()->StatusWarning(e.GetMessage(), 5);                
            }
        }
    }
}

void CPTool::set_tooltip(bool play)
{
    Ctrls.Go->setToolTip(play ? "Play" : "Pause");
}

void CPTool::pos_changed(int new_pos)
{
    SetRAW(false);
    double secs = new_pos / 10.0;
    Pos = secs;
    App->Play()->Cue(secs, false);
    SetData();
}


void CPTool::vol_changed(int db)
{
    Ctrls.Volume->setToolTip(QString("Volume is %1 db").arg(Ctrls.Volume->value() / double(10)));
    double v = exp10 (0.1 * double(db) / 20 );
    App->Play()->SetVolume(v);
}

void CPTool::fwd()
{
    App->Play()->Cue(10.0, true);
    SetRAW(false);    
    SetData();
}

void CPTool::rwd()
{
    App->Play()->Cue(-10.0, true);
    SetRAW(false);    
    SetData();
}

void CPTool::begin()
{
    App->Play()->Cue(0, false);
    SetRAW(false);    
    SetData();
    
}

void CPTool::end()
{
    App->Play()->Cue(App->Play()->GetMaster()->GetLength(), false);
    SetRAW(false);    
    SetData();
}

void CPTool::AtBegin(bool stream_was_paused)
{
    Between (stream_was_paused);
    Ctrls.Rwd->setEnabled(false);
    Ctrls.Begin->setEnabled(false);
}

void CPTool::AtEnd(bool stream_was_paused)
{
    Between (stream_was_paused);
    Ctrls.Fwd->setEnabled(false);
    Ctrls.End->setEnabled(false);
    if (!stream_was_paused)
    {
        Ctrls.Go->setIcon(Ctrls.Playing);
        App->Play()->Pause();
        set_tooltip(true);
    }        
}    


void CPTool::Between(bool /* stream_was_paused */ )
{
    Ctrls.Fwd->setEnabled(true);
    Ctrls.End->setEnabled(true);
    Ctrls.Rwd->setEnabled(true);
    Ctrls.Begin->setEnabled(true);
}

void CPTool::Reserve(int res)
{
    Ctrls.BufStat->Refresh(res);
}



