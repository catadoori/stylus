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

#ifndef CBASETOOL_H
#define CBASETOOL_H

#include <QToolBar>


class QComboBox;

class CBaseTool: public QToolBar
{
    Q_OBJECT
    public:
                CBaseTool   (QWidget * parent, char const * title);
               ~CBaseTool   ();
        int     GetSide     ()          const;
        QChar   GetSideName ()          const;
        void    SetSides    (int count, bool keep_selection = false);
        void    SetSide     (int side);
        QString GetWavName  (int side)  const;
        QString GetWavName  ()          const;
        
    
    protected:
        QComboBox * Side;
        int         OldSide;

        void        keyPressEvent(QKeyEvent * event);
    private slots:
    virtual  void   side_changed (int new_side) = 0;

};


#endif
