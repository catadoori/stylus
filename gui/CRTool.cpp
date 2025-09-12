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


#include "CRTool.h"
#include <QAction>
#include "CAStream.h"
#include "CApp.h"
#include "CTime.h"
#include "CPTool.h"
#include <QDebug>
#include "CBufStat.h"
#include <QComboBox>
#include "CAStream.h"
#include <QMessageBox>
#include "CGui.h"
#include "CTEdit.h"
#include "CXRuns.h"
#include "CDetector.h"
#include "CDetTool.h"
#include "CNormalizer.h"
#include "CNormTool.h"
#include "CAudio.h"

extern CApp * App;


CRTool::CRTool(QWidget * parent):
    CBaseTool(parent, "Record")
{
    Ctrls.Length = new CTime(parent);
    addWidget(Ctrls.Length);
    Ctrls.SyncPlay = new QAction(QIcon(":/icons/speaker_rec.png"), "Play current recording", this);
    connect(Ctrls.SyncPlay, SIGNAL(triggered()), this, SLOT(play()));
    addAction(Ctrls.SyncPlay);

    addSeparator();
    
    Ctrls.BufStat = new CBufStat(this, true);
    addWidget(Ctrls.BufStat);
    addSeparator();
    
    Ctrls.Recording.addFile(":/icons/rec_go.png");
    Ctrls.Paused.addFile(":/icons/rec_pause.png");
    Ctrls.Go = new QAction(Ctrls.Recording, "Go / Pause", this);
    Ctrls.Enable = new QAction(QIcon(":/icons/rec_rec.png"), "Toggle recording", this);
    Ctrls.Enable->setCheckable(true);

    addAction(Ctrls.Enable);
    addAction(Ctrls.Go);
    addSeparator();
    connect(Ctrls.Go, SIGNAL(triggered()), this, SLOT(StopGo()));
    connect (Ctrls.Enable, SIGNAL(toggled(bool)), this, SLOT(do_enable(bool)));
//    connect(Side, SIGNAL(currentIndexChanged(int)), this, SLOT(side_changed(int))); 
    
    set_active(false);
}
    
    
CRTool::~CRTool ()
{

}

void CRTool::SetData()
{
    double pos = App->Rec()->GetPosSec();
    Ctrls.Length->Set(pos);
}

void CRTool::Reserve(int res)
{
    Ctrls.BufStat->Refresh(res);
}


void CRTool::Reset()
{
    Ctrls.Go->setIcon(Ctrls.Recording);
}

bool CRTool::Active() const
{
    return Ctrls.Enable->isChecked();
}

void CRTool::StopGo ()
{
    if (App->Rec()->IsRunning())
    {
        App->Rec()->Pause();
        Ctrls.Go->setIcon(Ctrls.Recording);
    }
    else
    {
        App->Rec()->Continue();
        Ctrls.Go->setIcon(Ctrls.Paused);
    }
}

void CRTool::set_active(bool b)
{
    Ctrls.Enable->blockSignals(true);
    Ctrls.Enable->setChecked(b);
    Ctrls.BufStat->setEnabled(b);
    Ctrls.Go->setEnabled(b);
    Ctrls.SyncPlay->setEnabled(b);
    Ctrls.Enable->blockSignals(false);
}

void CRTool::do_enable (bool checked)
{
    if (checked)
    {
        CAStream s(false, false, false);
        try
        {
            CApp::CheckAccess(GetWavName(), true, true);           // Check path
            if (CApp::CheckAccess(GetWavName(), false, false, false))
            {
                try
                {
                    s.OpenFile(GetWavName());
                }
                catch (EFile &e) {}
                if (s.GetLength())
                {
                    QString info = QString("File %1 already contains data (%2). It will be "
                                        "overwritten. Do you want to continue?")
                                        .arg(GetWavName()).arg(s.WavInfo());
                    if (QMessageBox::warning(this, "Stylus - overwrite file?", info,
                        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
                    {
                        s.DeleteFile();
                        try_init();
                    }
                    else
                    {
                        set_active(false);
                        Requery();
                    }
                }
                else
                {
                    try_init();
                }
            }
            else
                try_init();
        }
        catch (Except &e)
        {
            set_active(false);
            Requery();
            throw;                    
        }
    }
    else
    {
        if (App->Rec()->IsRunning())
            StopGo();
        App->Rec()->Pause();
        set_active(false);
        App->Gui()->TEdit()->SetSideLength(GetSide(), App->Rec()->GetLength());        
        App->Rec()->End();

        if (GetSide() == App->Gui()->PTool()->GetSide())
            App->InitStreams(true, false);

        App->Rec()->SaveProfileData(GetSide(), true, true);
        if (GetSide() == App->Gui()->DetTool()->GetSide())
           * App->Detector()  = * App->Rec()->GetDetector();

        if (GetSide() == App->Gui()->NormTool()->GetSide())
           * App->Normalizer()  = * App->Rec()->GetNormalizer();
        Requery();
    }
    App->Gui()->PTool()->EnableReload();
    
}

void CRTool::try_init()
{
    try
    {
#if 0        
        setFocusPolicy(Qt::StrongFocus);
        setFocus();
#endif        
        App->Gui()->SetRecStart();
        App->InitStreams(false, true);
        set_active(true);
        App->Gui()->XRuns()->Clear();
    }
    catch (Except &e)
    {
        App->Gui()->StatusWarning(e.GetMessage(), 5);
    }
}

void CRTool::Refresh()
{
    if (Active())
    {
        Ctrls.Length->Set(App->Rec()->GetPosSec());
        Ctrls.Length->update();
    }
}

void CRTool::Requery()
{
   Ctrls.Length->Set(CAStream::Length(GetWavName()));
   Ctrls.Length->Clear();
   Ctrls.Length->update();

}

void CRTool::side_changed(int new_side)
{
    if (Active())
    {
        Side->blockSignals(true);
        Side->setCurrentIndex(OldSide);
        QString msg = QString("Currently side %1 is set to be recorded. To change the "
                                "active side please first deactivate record mode.").
                                arg(GetSideName());
        QMessageBox::information(this, "Stylus - Can't change recording side", msg);
        Side->blockSignals(false);
        return;
    }
    else
    {
        Requery();
    }
    OldSide = new_side;
}

void CRTool::play()
{
    if (GetSide() != App->Gui()->PTool()->GetSide())
        App->Gui()->PTool()->SetSide(GetSide());

    App->Gui()->PTool()->SetRAW(true);    
    App->Gui()->PTool()->Requery();
    App->Play()->Cue(App->Rec()->GetPosSec(), false);
    if (App->Play()->IsPaused())
        App->Gui()->PTool()->Go();
}

