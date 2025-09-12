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

#include "CDetector.h"
#include "CDetTool.h"
#include "CApp.h"
#include <QComboBox>
#include <QAction>
#include "CToolButton.h"
#include <QSpinBox>
#include <QDoubleSpinBox>
#include "CToolButton.h"
#include <QMenu>

#include "CAStream.h"
#include "CGui.h"
#include "CTEdit.h"
#include "CPTool.h"
#include "CTList.h"


extern CApp * App;

CDetTool::CDetTool(QWidget * parent):
    CBaseTool(parent, "Detector")
{


    Ctrls.Go.Button = new CToolButton(this);

    Ctrls.Go.Guess = new QAction(QIcon(":/icons/ark_help.png"), "Estimate track count", this);
    Ctrls.Go.Guess->setCheckable(true);
    connect(Ctrls.Go.Guess, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));

    Ctrls.Go.Detect = new QAction(QIcon(":/icons/find.png"), "Detect tracks", this);
    Ctrls.Go.Detect->setCheckable(true);
    connect(Ctrls.Go.Detect, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));    
    Ctrls.Go.Button->setDefaultAction(Ctrls.Go.Detect);

    Ctrls.Go.ManualSplit = new QAction(QIcon(":/icons/split.png"), "Manual split without pause", this);
    Ctrls.Go.ManualSplit->setCheckable(true);
    connect(Ctrls.Go.ManualSplit, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));

    Ctrls.Go.ManualSplitPause = new QAction(QIcon(":/icons/split_pause.png"),"Manual split with pause", this);
    Ctrls.Go.ManualSplitPause->setCheckable(true);
    connect(Ctrls.Go.ManualSplitPause, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));

    connect(Ctrls.Go.Button, SIGNAL(triggered(QAction*)), this, SLOT(on_action(QAction*)));


    Ctrls.Go.SkipDrop = new QAction ("Skip stylus drop", this);
    Ctrls.Go.SkipDrop->setCheckable(true);
    Ctrls.Go.SkipDrop->setChecked(true);

    Ctrls.Go.CutStart = new QAction("Cut leading silence", this);
    Ctrls.Go.CutStart->setCheckable(true);
    Ctrls.Go.CutEnd = new QAction("Cut trailing silence", this);
    Ctrls.Go.CutEnd->setCheckable(true);

    Ctrls.Go.OutroNew = new QAction("Outro as new track", this);
    Ctrls.Go.OutroNew->setCheckable(true);
    connect(Ctrls.Go.OutroNew, SIGNAL(triggered()), this, SLOT(outro()));

    Ctrls.Go.OutroLast = new QAction("Outro to last track", this);
    Ctrls.Go.OutroLast->setCheckable(true);
    connect(Ctrls.Go.OutroLast, SIGNAL(triggered()), this, SLOT(outro_last()));

    Ctrls.Go.Build = new QAction(QIcon(":/icons/gear.png"), "Rebuild profile", this);
    connect (Ctrls.Go.Build, SIGNAL(triggered()), this, SLOT(build_prof()));

    Ctrls.Go.Stop = new QAction(QIcon(":/icons/stop.png"), "Stop!", this);
    Ctrls.Go.Stop->setVisible(false);


    Ctrls.Go.Menu = new QMenu(this);

    Ctrls.Go.Menu->addAction(Ctrls.Go.ManualSplit);
    Ctrls.Go.Menu->addAction(Ctrls.Go.ManualSplitPause);
    Ctrls.Go.Menu->addSeparator();

    Ctrls.Go.Menu->addAction(Ctrls.Go.Guess);
    Ctrls.Go.Menu->addAction(Ctrls.Go.Detect);
    Ctrls.Go.Menu->addSeparator();    
    Ctrls.Go.Menu->addAction(Ctrls.Go.SkipDrop);
    Ctrls.Go.Menu->addAction(Ctrls.Go.CutStart);
    Ctrls.Go.Menu->addAction(Ctrls.Go.CutEnd);
    Ctrls.Go.Menu->addAction(Ctrls.Go.OutroNew);
    Ctrls.Go.Menu->addAction(Ctrls.Go.OutroLast);
    Ctrls.Go.Menu->addSeparator();
    Ctrls.Go.Menu->addAction(Ctrls.Go.Build);
    Ctrls.Go.Menu->addAction(Ctrls.Go.Stop);

    Ctrls.Go.Button->setMenu(Ctrls.Go.Menu);
    addWidget(Ctrls.Go.Button);
    addSeparator();    

    Ctrls.TrackSet = new QSpinBox(this);
    Ctrls.TrackSet->setSpecialValueText(" ");
    Ctrls.TrackSet->setPrefix("Set ");
    connect(Ctrls.TrackSet, SIGNAL(valueChanged(int)), this, SLOT(set_selected(int)));
    addWidget(Ctrls.TrackSet);
    addSeparator();

    Ctrls.Mode = new QComboBox(this);
    for (char const ** str = CDetector::ModeList; *str; ++str)
        Ctrls.Mode->addItem(*str);
    Ctrls.Mode->setToolTip("Detection method");
    Ctrls.Mode->setVisible(false);
    addWidget(Ctrls.Mode);
//    addSeparator();

    Ctrls.Length = new QComboBox(this);
    Ctrls.Length->setToolTip("Minimum track length");
    for (char const ** str = CDetector::LengthList; *str; ++str)
        Ctrls.Length->addItem(*str);
    Ctrls.Length->setCurrentIndex(CDetector::TLenDefault);
    addWidget(Ctrls.Length);
    addSeparator();
    
    Ctrls.Silence = new QComboBox(this);
    Ctrls.Silence->setToolTip("Minimum silence length");
    for (char const ** str = CDetector::LengthList; *str; ++str)
        Ctrls.Silence->addItem(*str);
    Ctrls.Silence->setCurrentIndex(CDetector::PLenDefault);
    addWidget(Ctrls.Silence);
    addSeparator();

    Ctrls.Noise = new QComboBox(this);
    Ctrls.Noise->setToolTip("Vinyl noise level");
    for (char const ** str = CDetector::NoiseList; *str; ++str)
        Ctrls.Noise->addItem(*str);
    Ctrls.Noise->setVisible(false);    
    addWidget(Ctrls.Noise);
//    addSeparator();
       
    setToolTip("Detector");
}

void CDetTool::outro()
{
    if (Ctrls.Go.OutroNew->isChecked())
        Ctrls.Go.OutroLast->setChecked(false);
}

void CDetTool::outro_last()
{
    if (Ctrls.Go.OutroLast->isChecked())
        Ctrls.Go.OutroNew->setChecked(false);
}

void CDetTool::enable_options()
{
    bool on = !Ctrls.Go.ManualSplitPause->isChecked() &&
             !Ctrls.Go.ManualSplit->isChecked();
    
    Ctrls.Go.SkipDrop->setVisible(on);
    Ctrls.Go.CutStart->setVisible(on);
    Ctrls.Go.CutEnd->setVisible(on);
    Ctrls.Go.OutroNew->setVisible(on);
    Ctrls.Go.OutroLast->setVisible(on);
    Ctrls.Length->setEnabled(on);
    Ctrls.Silence->setEnabled(on);
    Ctrls.TrackSet->setEnabled(on);
}

void check_action (QAction * a, bool checked)
{
    a->setChecked(checked);
}


void CDetTool::DoAction(CGui::SpaceActions act)
{
    QAction * a;
    switch (act)
    {
        case CGui::saSplitTrack:      a = Ctrls.Go.ManualSplit; break;
        case CGui::saSplitTrackPause: a = Ctrls.Go.ManualSplitPause; break;
        case CGui::saDetect:          a = Ctrls.Go.Detect; break;
        case CGui::saQuery:           a = Ctrls.Go.Guess; break;
        default: a = 0;
    }
    if (a)
        on_action(a);
}

void CDetTool::toggled(bool)
{
    QAction * send = qobject_cast<QAction*> (sender());
    QSignalBlocker b { send };
    check_action(Ctrls.Go.ManualSplitPause, false);
    check_action(Ctrls.Go.ManualSplit, false);
    check_action(Ctrls.Go.Detect, false);
    check_action(Ctrls.Go.Guess, false);

    if (!send) return;
    check_action(send, true);
    char const * msg = "";
    if (send == Ctrls.Go.ManualSplitPause)
        msg = "Hit space bar twice to end current and begin new track.";
    else if (send == Ctrls.Go.ManualSplit)
        msg = "Hit space bar twice to split track.";
    else
        msg = "Tweaking detector parameters may be needed.";

    enable_options();
    App->Gui()->StatusMessage(msg, 6);
    Ctrls.Go.Button->setDefaultAction(send);
    Ctrls.Go.Button->setChecked(false);
}

CDetTool::~CDetTool()
{
    
}

void CDetTool::SetSides(int count, bool keep_selection)
{
    CBaseTool::SetSides(count, keep_selection);
}


void CDetTool::build_prof()
{
    App->Detector()->BuildProfile();
}

int CDetTool::run_detect()
{
    int count;
    CDetector::Modes mode = (CDetector::Modes) Ctrls.Mode->currentIndex();
    CDetector::tOption option;
    if (Ctrls.Go.SkipDrop->isChecked())  option.Add(CDetector::SkipNeedlePop);
    if (Ctrls.Go.CutStart->isChecked())  option.Add(CDetector::CutStart);
    if (Ctrls.Go.CutEnd->isChecked())    option.Add(CDetector::CutEnd);
    if (Ctrls.Go.OutroLast->isChecked()) option.Add(CDetector::OutroToLast);
    if ( Ctrls.Go.OutroNew->isChecked()) option.Add(CDetector::OutroToNew);
    
    count = App->Detector()->LocateTracks(mode, option, GetSide(), Ctrls.Length->currentIndex(),
                                          Ctrls.Silence->currentIndex(),
                                          Ctrls.Noise->currentIndex());
    return count;                                          
}

void CDetTool::guess_count()
{
    run_detect();
    QString hint;
    int count = App->Detector()->GetData().Estimated.Count;
    if (count)            
        hint = QString("Stream seems to have %1 track%2 (%3% relevance)").arg(count).
                       arg(count > 1 ? "s" : "").
                       arg(App->Detector()->GetData().Estimated.Relevance);
                       
    App->Gui()->StatusMessage(hint, 5);

}

void CDetTool::SetBuildMode(bool bv)
{
    Ctrls.Go.Button->menu()->setEnabled(!bv);
    if (bv)
    {
        Ctrls.Go.LastAction = Ctrls.Go.Button->defaultAction();
        Ctrls.Go.Button->setDefaultAction(Ctrls.Go.Stop);
        Ctrls.Go.Button->setEnabled(true);
    }
    else
    {
        Ctrls.Go.Button->setMenu(Ctrls.Go.Menu);
        Ctrls.Go.Button->setDefaultAction(Ctrls.Go.LastAction);
    }
    Ctrls.Go.Button->setChecked(false);
}


void CDetTool::find_tracks()
{
    int count = run_detect();
    CDetector::Modes mode = (CDetector::Modes) Ctrls.Mode->currentIndex();    
    if (mode == CDetector::Approach)
    {
        App->Gui()->StatusMessage(QString("%1 match%2 found.").arg(count).
                                  arg(count < 2 ? "": "es"), 5);
    }                                  
    set_trackset(count);
    if (count)
        set_selected(1);
}

void CDetTool::on_action(QAction * action)
{
    if (action == Ctrls.Go.ManualSplitPause)
    {
        App->Gui()->TEdit()->MarkPlayTime(true);
    }
    else if (action == Ctrls.Go.ManualSplit)
    {
        App->Gui()->TEdit()->MarkPlayTime(false);
    }
    else if (action == Ctrls.Go.Detect)
        find_tracks();
    else if (action == Ctrls.Go.Guess)
        guess_count();
    else if (action == Ctrls.Go.Stop) // FIXME why Ctrls.Go.Stop!?
        build_prof();
    else if (action == Ctrls.Go.Build)
        build_prof();
}


void CDetTool::set_trackset(int count)
{
    Ctrls.TrackSet->blockSignals(true);
    Ctrls.TrackSet->setMinimum(count ? 1 : 0);
    Ctrls.TrackSet->setMaximum(count);
    Ctrls.TrackSet->setEnabled(count);
    Ctrls.TrackSet->setValue(count ? 1 : 0);
    Ctrls.TrackSet->setSpecialValueText(count ? "" : " ");
    Ctrls.TrackSet->blockSignals(false);    
}

void CDetTool::side_changed(int new_side)
{
    if (App->Detector()->LoadData(new_side) == -1)
        App->Gui()->StatusWarning(QString("Could not load profile data for side %1").
                                  arg(CApp::SideToQChar(new_side)), 5);
    set_trackset(0);
    double secs = App->Gui()->TEdit()->GetSideLength(new_side);
    bool const has_profile = App->Detector()->GetData().Profile.size();
    Ctrls.Silence->setEnabled(secs != 0.0 && has_profile);
    Ctrls.Length->setEnabled(secs != 0.0 && has_profile);
    Ctrls.TrackSet->setEnabled(secs != 0.0 && has_profile);
    if (secs != 0.0 && !has_profile)
    {
        Ctrls.Go.Button->setDefaultAction(Ctrls.Go.Build);
    }
    Requery();
}

void CDetTool::Requery()
{
    Ctrls.Go.Detect->setEnabled(App->Detector()->GetData().Profile.size() &&
                             App->Gui()->TEdit()->TrackCount(GetSide()));

    Ctrls.TrackSet->setEnabled(App->Detector()->TrackSets.size());
}

void CDetTool::set_selected(int set)
{
    App->Gui()->TEdit()->SetTimings(GetSide(), set);
    App->Gui()->setWindowModified(true);
}


// -------------------------------------------------------------------------------------------


