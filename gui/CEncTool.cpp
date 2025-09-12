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


#include "CEncTool.h"
#include "CMp3Enc.h"
#include <QAction>
#include <QToolButton>
#include "CApp.h"
#include "CGui.h"
#include "CTEdit.h"
#include "CTList.h"

#include "CFile.h"
#include "CAStream.h" // FIXME auf CEncoderStream �ndern

#include <QDebug>
#include <QMenu>
#include <QDir>
#include <QMessageBox>

extern CApp * App;

CEncTool::CEncTool(QWidget * parent):
    QToolBar("Encoder", parent),
    Encoder(0)
{
    Ctrls.Go.But  = new QToolButton(this);
    QMenu * m = new QMenu(this);
    Ctrls.Go.Local = new QAction( "Create locally", this);
    Ctrls.Go.Local->setCheckable(true);
    Ctrls.Go.Local->setChecked(true);
    m->addAction(Ctrls.Go.Local);

    Ctrls.Go.CleanLocal = new QAction("Clean up local directory", this);
    Ctrls.Go.CleanLocal->setCheckable(true);
    m->addAction(Ctrls.Go.CleanLocal);
    m->addSeparator();
    

    Ctrls.Go.Remote = new QAction("Create remote", this);
    Ctrls.Go.Remote->setCheckable(true);
    m->addAction(Ctrls.Go.Remote);

    Ctrls.Go.CleanRemote = new QAction("Clean up remote directory", this);
    Ctrls.Go.CleanRemote->setCheckable(true);
    m->addAction(Ctrls.Go.CleanRemote);
    m->addSeparator();

    m->addSeparator();
    Ctrls.Go.Side = new QAction("Only current side", this);
    Ctrls.Go.Side->setCheckable(true);
    m->addAction(Ctrls.Go.Side);

    Ctrls.Go.Track = new QAction("Only selected track", this);
    Ctrls.Go.Track->setCheckable(true);
    m->addAction(Ctrls.Go.Track);
    m->addSeparator();
    
    Ctrls.Go.StreamInfo = new QAction("Stream info in file name", this);
    Ctrls.Go.StreamInfo->setCheckable(true);
    m->addAction(Ctrls.Go.StreamInfo);

    Ctrls.Go.RecChainInfo = new QAction("Recording chain in file name", this);
    Ctrls.Go.RecChainInfo->setCheckable(true);
    m->addAction(Ctrls.Go.RecChainInfo);
    
    Ctrls.Go.But->setMenu(m);
    Ctrls.Go.Stop = new QAction(QIcon(":/icons/stop.png"), "Stop encoding", this);
    Ctrls.Go.Start = new QAction(QIcon(":/icons/kscd.png"), "Start encoding", this);

    Ctrls.Go.But->setDefaultAction(Ctrls.Go.Start);
    addWidget(Ctrls.Go.But);
    connect(Ctrls.Go.But, SIGNAL(triggered(QAction*)), this, SLOT(go_pressed(QAction*)));
    setToolTip("Encoder toolbar");
    go_pressed(Ctrls.Go.CleanLocal);
    go_pressed(Ctrls.Go.Remote);
}

CEncTool::~CEncTool()
{

}

void CEncTool::Reset()
{
    if (Encoder && Encoder->GetFile())
    {
        disconnect(Encoder->GetFile(), 0, 0, 0);
        go_pressed(Ctrls.Go.Stop);
    }
}

void CEncTool::go_pressed(QAction* act)
{
    if (act == Ctrls.Go.Start)
    {
        if (!Ctrls.Go.Remote->isChecked() && ! Ctrls.Go.Local->isChecked())
        {
            App->Gui()->StatusWarning("Encoder: No destination directory selected.", 5);
            return;
        }
        if (Ctrls.Go.CleanLocal->isChecked() && clean_up(false))
            Ctrls.Go.CleanLocal->setChecked(false);
        if (Ctrls.Go.CleanRemote->isChecked() && clean_up(true))
            Ctrls.Go.CleanRemote->setChecked(false);
        
        Ctrls.Go.But->setDefaultAction(Ctrls.Go.Stop);
        encode();
    }
    else if (act == Ctrls.Go.Stop)
    {
        Ctrls.Go.But->setDefaultAction(Ctrls.Go.Start);
        Encoder->GetFile()->End(true);
    }
    else if (act == Ctrls.Go.Remote)
    {
        Ctrls.Go.CleanRemote->setEnabled(Ctrls.Go.Remote->isChecked());
        if (!Ctrls.Go.CleanRemote->isEnabled())
            Ctrls.Go.CleanRemote->setChecked(false);
    }
    else if (act == Ctrls.Go.Local)
    {
        Ctrls.Go.CleanLocal->setEnabled(Ctrls.Go.Local->isChecked());
        if (!Ctrls.Go.CleanLocal->isEnabled())
            Ctrls.Go.CleanLocal->setChecked(false);
    }
    else if (act == Ctrls.Go.Track)
    {
        if (Ctrls.Go.Track->isChecked())
            Ctrls.Go.Side->setChecked(false);
    }
    else if (act == Ctrls.Go.Side)
    {
        if (Ctrls.Go.Side->isChecked())
            Ctrls.Go.Track->setChecked(false);
    }
 }


void CEncTool::encode()
{
    int side = (Ctrls.Go.Side->isChecked() ||
               Ctrls.Go.Track->isChecked()) ? App->Gui()->TEdit()->GetSide() : -1;
    if (App->Gui()->TEdit()->GetEncodeJob(TrackInfo, side, Ctrls.Go.StreamInfo->isChecked(),
                                          Ctrls.Go.RecChainInfo->isChecked(),
                                          Ctrls.Go.Track->isChecked()) == false)
    {
        Ctrls.Go.But->setDefaultAction(Ctrls.Go.Start);
        return;
    }

    Encoder = new CEncoderStream(TrackInfo.Out.Format);
    
    connect(Encoder->GetFile(), SIGNAL(finished()), this, SLOT(finished()), Qt::QueuedConnection);
    connect(Encoder->GetFile(), SIGNAL(terminated()), this, SLOT(terminated()), Qt::QueuedConnection);
    connect(Encoder->GetFile(), SIGNAL(Progress(int)), this, SLOT(progress(int)), Qt::QueuedConnection);
    connect(Encoder->GetFile(), SIGNAL(Exception()), this, SLOT(worker_exception()), Qt::QueuedConnection);

    try
    {
        Encoder->Start(TrackInfo, Ctrls.Go.Local->isChecked(), Ctrls.Go.Remote->isChecked());
    }
    catch (Except &e)
    {
        go_pressed(Ctrls.Go.Stop);
        Ctrls.Go.But->setChecked(false);
        App->Gui()->TEdit()->SetData(TrackInfo.Offs, CTList::colStatus, "Err");
        App->Gui()->StatusWarning(e.GetMessage(), 5);
    }
}

bool CEncTool::clean_up(bool remote)
{
    QString path = remote ? App->Settings.Project.GetRBasePath() : App->Settings.Project.GetLBasePath();
    path += App->Settings.Project.GetSubPath();
    CApp::CheckAccess(path,true, true, true);
    QDir dir;
    dir.setPath(path);
    int err = 0;
    int deleted = 0;
    
    for (CBaseEnc::tEncoder const  * e = CBaseEnc::Encoder; e->Name; ++e)
    {
        QString const mask = QString("???_*.%1").arg(e->Ext);
        dir.setNameFilters(QStringList(mask));
        QStringList li = dir.entryList();
        QString fname;
        for (QStringList::ConstIterator it = li.begin(); it != li.end(); ++it)
        {
            fname = dir.path() + "/" + *it;
            App->Gui()->StatusMessage(QString ("Deleting file '%1'...").arg(fname), 5);
            QFile f (fname);
            if (!f.remove())
            {
                App->Gui()->StatusWarning(QString ("Could not delete file '%1'.").arg(fname), 5);
                ++err;
            }
            else
                ++deleted;
        }
    }
    if (err)
        QMessageBox::warning(App->Gui(), "stylus", QString("Could not delete %1 file(s) from '%1'.").arg(dir.path()));
    else
        App->Gui()->StatusMessage(QString("%1 old target file(s) were deleted from '%2'.").arg(deleted).arg(path), 5);
    return err == 0;
}    



void CEncTool::finished()
{
    bool finis = Encoder->IsAtEnd();    
    if (finis)
    {
        finis &= Encoder->GetEncoder()->Flush();
    }
    QString stat = finis ?  "F" : "Err";
    App->Gui()->TEdit()->SetData(TrackInfo.Offs, CTList::colStatus, stat);
    delete Encoder;
    Encoder = 0;
    if (finis)
        encode();
    else
        Ctrls.Go.But->setDefaultAction(Ctrls.Go.Start);            
}

void CEncTool::progress(int percent)
{
    App->Gui()->TEdit()->SetData(TrackInfo.Offs, CTList::colStatus, QString("%1%").arg(percent));
}

void CEncTool::worker_exception()
{
    App->Gui()->StatusWarning("Worker error in CEncTool",5);
    
}

void CEncTool::terminated()
{
    delete Encoder;
    Encoder = 0;
}

