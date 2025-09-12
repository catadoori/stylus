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
#ifndef CNORMTOOL_H
#define CNORMTOOL_H

#include "../config.h"
#include "CBaseTool.h"

/**
	@author Harry <harry@actionpussyclub.de>
*/

class QDoubleSpinBox;
class QAction;

class CNormTool: public CBaseTool
{
    Q_OBJECT
    public:
                CNormTool   (QWidget * parent);
               ~CNormTool   ();
        void    Requery     ();               
    private:
        struct tCtrls
        {
            QDoubleSpinBox * Level;
            QDoubleSpinBox * Clipped;
            QAction        * Volume;
            QAction        * CommitVol;
        } Ctrls;
    private slots:
            void    level_changed   (double db);
            void    clipped_changed (double ppm);
            void    vol_toggle      ();
            void    vol_commit      ();
    virtual void    side_changed    (int new_side);
        
            
                   
};



#endif
