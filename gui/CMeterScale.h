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
#ifndef CMETERSCALE_H
#define CMETERSCALE_H

/**
	@author Harry <harry@actionpussyclub.de>
*/

#include <QWidget>

class CMeterScale: public QWidget
{
    Q_OBJECT
    public:
                    CMeterScale (QWidget * parent);
        virtual    ~CMeterScale ();
        void        paintEvent  (QPaintEvent * event);
        void        resizeEvent (QResizeEvent * event);

        static int const FineDBs;
        static int const Ticks;
        static int const Offset;
        static double const MinScale;

        static double PixelPerDbFine;
};

#endif
