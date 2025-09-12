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


#ifndef CRTOOL_H
#define CRTOOL_H

#include "CBaseTool.h"
#include <QIcon>

class CTime;
class CBufStat;

class CRTool: public CBaseTool
{
   Q_OBJECT
    public:
                CRTool      (QWidget * parent);
               ~CRTool      ();
        bool    Active      () const;
        void    SetData     ();
        void    Reset       ();
        void    Refresh     ();
        void    Requery     ();                
    public slots:        
        void    Reserve     (int res);
        void    StopGo      ();

    private:
        struct tCtrls
        {
            CTime   * Length;
            QAction * SyncPlay;
            CBufStat* BufStat;
            QAction * Enable;
            QAction * Go;
            QIcon     Recording;
            QIcon     Paused;
        };
        tCtrls  Ctrls;
    
        void      set_active    (bool b);
        void      try_init      ();
    private slots:
        void      side_changed  (int side);
        void      do_enable     (bool checked);
        void      play          ();
                   
};

#endif
