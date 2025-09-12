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
#include "CNormTool.h"
#include <QDoubleSpinBox>

#include "CApp.h"
#include "CNormalizer.h"
#include "CGui.h"
#include "CPTool.h"
#include "CTEdit.h"

extern CApp * App;

CNormTool::CNormTool(QWidget * parent):
    CBaseTool (parent, "Normalizer")
{
    Ctrls.Level = new QDoubleSpinBox(this);
    Ctrls.Level->setMinimum(-6);
    Ctrls.Level->setMaximum(+24);
    Ctrls.Level->setSuffix(" db");
    Ctrls.Level->setDecimals(1);
    Ctrls.Level->setSingleStep(0.1);
    connect (Ctrls.Level, SIGNAL(valueChanged(double)), this, SLOT(level_changed(double)));
    addWidget(Ctrls.Level);
    
    Ctrls.Clipped = new QDoubleSpinBox(this);
    Ctrls.Clipped->setMaximum(10000);
    Ctrls.Clipped->setSuffix(" ppm");
    Ctrls.Clipped->setDecimals(1);
    Ctrls.Clipped->setSingleStep(0.5);
    connect(Ctrls.Clipped, SIGNAL(valueChanged(double)), this, SLOT(clipped_changed(double)));
    addWidget(Ctrls.Clipped);

    Ctrls.Volume = new QAction(QIcon(":/icons/speaker_r.png"), "Use level for playback", this);
    Ctrls.Volume->setCheckable(true);
    addAction(Ctrls.Volume);
    connect(Ctrls.Volume, SIGNAL(triggered()), this, SLOT(vol_toggle()));

    Ctrls.CommitVol = new QAction(QIcon(":/icons/player_playlist.png"), "Commit level to tracks", this);
    addAction(Ctrls.CommitVol);
    connect(Ctrls.CommitVol, SIGNAL(triggered()), this, SLOT(vol_commit()));
    setToolTip("Normalizer");
}


CNormTool::~CNormTool()
{
}

void CNormTool::level_changed(double db)
{
    double ppm = App->Normalizer()->ClippingAtGain(db);
    Ctrls.Clipped->blockSignals(true);
    Ctrls.Clipped->setValue(ppm);
    Ctrls.Clipped->blockSignals(false);
    if (Ctrls.Volume->isChecked())
        App->Gui()->PTool()->SetVolume(db);
}

void CNormTool::clipped_changed(double ppm)
{
    double db = App->Normalizer()->GainAtClipping(ppm);
    Ctrls.Level->blockSignals(true);
    Ctrls.Level->setValue(db);
    Ctrls.Level->blockSignals(false);
    if (Ctrls.Volume->isChecked())
        App->Gui()->PTool()->SetVolume(db);

}

void CNormTool::vol_toggle()
{
    double val = Ctrls.Volume->isChecked() ? Ctrls.Level->value() : 0.0;
    App->Gui()->PTool()->SetVolume(val);
}

void CNormTool::vol_commit()
{
    App->Gui()->TEdit()->SetVolumes(Ctrls.Level->value());
}

void CNormTool::side_changed (int new_side)
{
    if (App->Normalizer()->LoadData(new_side) == false)
        App->Gui()->StatusWarning(QString("Could not load level data for side %1").
                                  arg(CApp::SideToQChar(new_side)), 5);
    else if (App->Normalizer()->HasData())
    {
        clipped_changed(Ctrls.Clipped->value());
    }
    Requery();
}

void CNormTool::Requery()
{
    bool has_data = App->Normalizer()->HasData();
    Ctrls.Clipped->setEnabled(has_data);
    Ctrls.Level->setEnabled(has_data);
#if 1  
    if (has_data)
    {
        //clipped_changed(0.0);
    }
    else
    {
        Ctrls.Level->setValue(0.0);
        Ctrls.Clipped->setValue(0.0);
    }
#endif    
}

