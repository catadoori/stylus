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
#ifndef CXRUNS_H
#define CXRUNS_H

/**
	@author Harry <harry@actionpussyclub.de>
*/

#include <QDockWidget>

class QWidget;
class QLabel;
class QSpinBox;
class QListWidget;
class QFrame;
class QListWidgetItem;

class CXRuns: public QDockWidget
{
    Q_OBJECT
    public:
                 CXRuns       (QWidget * parent);
                ~CXRuns       ();
        void    Refresh       ();
        void    Clear         ();
    public slots:
        void    Add           (int sample, bool record);

    private:
        QFrame      * Frame;
        QLabel      * RCount;
        QLabel      * PCount;
        QLabel      * PrePlayLabel;
        QSpinBox    * PrePlay;
        QListWidget * List;
        int           Plays;
        void          set_counts     ();
    private slots:      
        void          play_xrun      (QListWidgetItem * xrun);

        
};

#endif
