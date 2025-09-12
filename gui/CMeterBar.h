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
#ifndef CMETERBAR_H
#define CMETERBAR_H

/**
	@author Harry <harry@actionpussyclub.de>
*/

#include <QWidget>

class QPixmap;

class CMeterBar: public QWidget
{
    Q_OBJECT
    public:
                    CMeterBar   (QWidget * parent);
        virtual    ~CMeterBar   ();

        void        Refresh     (double rms, double peak);
        void        resizeEvent (QResizeEvent * event);
        void        paintEvent  (QPaintEvent * event);
        void        mouseDoubleClickEvent(QMouseEvent * e);

    private:
        int Rms;
        int Peak;
        int PeakHold;
        int ObjNr;
        int TickCount;
        
        static int       ObjCount;
        static QPixmap * BarPixmap;
        static QPixmap * ClearPixmap;
        static QPixmap * PeakPixmap;

};

#endif
