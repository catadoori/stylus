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

#ifndef CTIME_H
#define CTIME_H

#include <QWidget>

/**
	@author Harry <harry@actionpussyclub.de>
*/


class CTime:public QWidget
{
    public:                
                CTime                   (QWidget * parent);
               ~CTime                   ();
        void     Set                    (double const & seconds) { Time = seconds; }
        void     Clear                  ()                       { OldTime = 0.0; }
        void     paintEvent             (QPaintEvent * e);
        void     mouseDoubleClickEvent  (QMouseEvent * e);
                    
    private:
        bool    Msh;  // Format mm:ss.h or ssss.h
        double  Time;
        double  OldTime;
        QString Format;
        QString String;

};

#endif
