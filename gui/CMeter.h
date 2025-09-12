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
#ifndef CMETER_H
#define CMETER_H

/**
	@author Harry <harry@actionpussyclub.de>
*/

#include "types.h"
#include <QWidget>
#include <QDebug>

class CMeterLED;
class CMeterBar;
class CMeterInfo;
class QLabel;
class QVBoxLayout;

class CMeter: public QWidget
{
    Q_OBJECT
    public:
                    CMeter      (QWidget * parent, char const * usage);
        virtual     ~CMeter     ();
        QRect       BarGeometry () const;
        void        Refresh     ();
        tMeterData* GetData     ()                   { return &Data; }
        
    private:
        struct tChannel
        {
            CMeterLED   * LED;
            CMeterBar   * Bar;
        };
        tChannel   Left;
        tChannel   Right;
        QLabel*    Label;
        tMeterData Data;

        QVBoxLayout*init_channel(tChannel & channel);
};

#endif
