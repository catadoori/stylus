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
#include "CMeterBar.h"
#include "CMeterScale.h"
#include <QLinearGradient>
#include <QPainter>
#include <QPixmap>
#include <QDebug>

#include "CApp.h"
#include "CGui.h"
#include "CMeters.h"

extern CApp * App;


int CMeterBar::ObjCount = 0;

QPixmap * CMeterBar::BarPixmap = 0;
QPixmap * CMeterBar::ClearPixmap = 0;
QPixmap * CMeterBar::PeakPixmap = 0;

// #define COMBI_METER

CMeterBar::CMeterBar(QWidget * parent):
    QWidget(parent),
    Rms(0x8000),
    Peak(0x8000),
    PeakHold(0x8000),
    TickCount(0)
{
    setMaximumWidth(50);
    setMinimumWidth(5);
    setAutoFillBackground(true);    
    ObjNr = ObjCount;
    if (ObjCount++ == 0)
    {
        BarPixmap = new QPixmap();
        ClearPixmap = new QPixmap();
#ifdef COMBI_METER        
        PeakPixmap = new QPixmap();
#endif        
    }
}

CMeterBar::~CMeterBar()
{
    if (--ObjCount == 0)
    {
        delete BarPixmap; BarPixmap = 0;
        delete ClearPixmap; ClearPixmap = 0;
        delete PeakPixmap; PeakPixmap = 0;
    }
}

void CMeterBar::Refresh(double rms, double peak)
{
//     rms = -10; peak = -5;
#ifndef COMBI_METER
    rms = peak;
#endif
    if (rms < CMeterScale::MinScale)
        rms = CMeterScale::MinScale;
    if (rms < -CMeterScale::FineDBs)
        rms -= (rms + CMeterScale::FineDBs) / 2;
    Rms = qRound(CMeterScale::PixelPerDbFine * - rms);
    
    if (peak < -CMeterScale::FineDBs)
        peak -= (peak + CMeterScale::FineDBs) / 2;

    int new_peak = Peak = qRound(CMeterScale::PixelPerDbFine * - peak);

    if (new_peak > Peak)
    {
        PeakHold = Peak;
        TickCount = 0;
    }
    else
    {
        if (new_peak < PeakHold)
        {
            PeakHold = new_peak;
            TickCount = 0;
        }
        else if (TickCount ++ == 40)
        {
            TickCount = 0;
            PeakHold = new_peak;        
        }
    }
#if 0
    update();
#else
    repaint();
#endif
}

void CMeterBar::resizeEvent(QResizeEvent * )
{
    if (ObjNr == 0)
    {
        QPixmap pm(size());           
        *BarPixmap = pm;
        *ClearPixmap = pm;
        
        QPainter barpainter(BarPixmap);
        QLinearGradient gr(0, 0, 0, height());
        gr.setColorAt(1, QColor(Qt::darkGreen));
        gr.setColorAt(0.4, QColor(Qt::darkYellow));
        gr.setColorAt(0, QColor(Qt::darkRed));
        barpainter.fillRect(0, 0, width(), height(), gr);

#ifdef COMBI_METER
        *PeakPixmap = pm;
        QPainter peakpainter(PeakPixmap);
        gr.setColorAt(1, QColor(Qt::darkGreen));
        gr.setColorAt(0.4, QColor(Qt::yellow));
        gr.setColorAt(0, QColor(Qt::red));
        peakpainter.fillRect(0, 0, width(), height(), gr);
#endif

        QPainter clearpainter(ClearPixmap);
#if 1
        clearpainter.fillRect(rect(), QColor(Qt::darkGray));
#else        
        ClearPixmap->fill();
        clearpainter.fillRect(rect(), QBrush(QColor(Qt::black), Qt::Dense3Pattern));
#endif        
    }
}

void CMeterBar::paintEvent(QPaintEvent * )
{
    if (!isEnabled())
        return;
    QPainter painter (this);
#ifndef COMBI_METER   
    painter.drawPixmap(0, Rms, *BarPixmap, 0, Rms, -1, -1);
    painter.drawPixmap(0, 0, *ClearPixmap, 0, height() - Rms, -1, -1);
    painter.drawPixmap(0, PeakHold, *BarPixmap, 0, PeakHold, width(), 3);
#else
    painter.drawPixmap(0, 0, *ClearPixmap, 0, height() - Rms, 0, 0);
    painter.drawPixmap(0, Rms, *BarPixmap, 0, Rms, -1, -1);
    painter.drawPixmap(0, Peak, *PeakPixmap, 0, Peak, 0, Rms - Peak);
    painter.drawPixmap(0, PeakHold, *PeakPixmap, 0, PeakHold, width(), 3);
#endif
}

void CMeterBar::mouseDoubleClickEvent(QMouseEvent * )
{
    App->Gui()->Meters()->Toggle(false);
}

