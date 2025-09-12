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

#include "CBaseTool.h"
#include <QComboBox>
#include <algorithm>

#include "CApp.h"
#include "CGui.h"
#include "CSettings.h"
#include <QDebug>
#include <QKeyEvent>

extern CApp * App;


CBaseTool::CBaseTool(QWidget * parent, char const * title):
    QToolBar(title, parent)
{
    Side = new QComboBox(parent);
    Side->setMaximumWidth(30 + fontMetrics().boundingRect("X").width());
    addWidget(Side);

    connect(Side, SIGNAL(currentIndexChanged(int)), this, SLOT(side_changed(int)));
    setFocusPolicy(Qt::StrongFocus);
}

CBaseTool::~CBaseTool()
{
 
}

QString CBaseTool::GetWavName(int side) const
{
    return App->Settings.Project.GetWavName(side);
}

QString CBaseTool::GetWavName() const
{
    return GetWavName(GetSide());
}

void CBaseTool::SetSides(int count, bool keep_selection)
{
    int sel = Side->currentIndex();
    Side->blockSignals(true);
    Side->clear();
    for (int x = 0; x < count; ++x)
        Side->addItem(CApp::SideToQChar(x));

    if (keep_selection &&  sel != -1)
        Side->setCurrentIndex(std::min(sel, count - 1));
    Side->blockSignals(false);
    OldSide = 0;
}

void CBaseTool::SetSide(int side)
{
    Side->setCurrentIndex(side);
    side_changed(side);
}


int CBaseTool::GetSide() const
{
    return Side->currentIndex();
}

QChar CBaseTool::GetSideName() const
{
    return CApp::SideToQChar(GetSide());
}

void CBaseTool::keyPressEvent(QKeyEvent * event)
{
    if (App->Gui()->KeyPressed(event->key(), this) == false)
        QToolBar::keyPressEvent(event);
}




