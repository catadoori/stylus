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
#include "CSwapSides.h"
#include "CApp.h"
#include <QLabel>
#include <QTabBar>
#include <QComboBox>
#include <QGridLayout>
#include <QDialogButtonBox>

CSwapSides::CSwapSides(QWidget * parent, int a, int count):
    QDialog(parent, Qt::WindowTitleHint)
{
    QString msg = QString("Swap tracks of side %1 with tracks of side:"). arg(CApp::SideToQChar(a));
    QLabel * l = new QLabel(msg, this);
    Sides = new QComboBox(this);
    Sides->setMaximumWidth(fontMetrics().maxWidth() * 3);
    for (int x = 0; x < count; ++x)
    {
        if (x != a)
            Sides->addItem(CApp::SideToQChar(x));
    }
    
    QGridLayout * lay = new QGridLayout;
    
    lay->addWidget(l, 0, 0);
    lay->addWidget(Sides, 0, 1);

    QDialogButtonBox * butts;
    butts = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                 Qt::Horizontal, this);
    lay->addWidget(butts, 1, 0);
    setLayout (lay);

    connect(butts, SIGNAL(accepted()), this, SLOT(accept()));
    connect(butts, SIGNAL(rejected()), this, SLOT(reject()));
    
    
}

int CSwapSides::SelectedSide() const
{
    if (Sides->currentText().length() == 1)
    {
        QChar c = Sides->currentText().at(0);
        return  CApp::QCharToSide(c);
    }
    else
        return -1;    
}


CSwapSides::~CSwapSides()
{
}

