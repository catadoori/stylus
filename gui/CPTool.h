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



#ifndef CPTOOL_H
#define CPTOOL_H

#include "CBaseTool.h"
#include <QIcon>
#include <QSlider>


class QIcon;
class CTime;
class CBufStat;

class CPTool: public CBaseTool
{
    Q_OBJECT
    public:
                CPTool      (QWidget * parent);
               ~CPTool      ();
        void    Refresh     ();
        void    SetRAW      (bool read_after_write);
    public slots:
        void    AtBegin     (bool stream_was_paused);
        void    AtEnd       (bool stream_was_paused);
        void    Between     (bool stream_was_paused);
        void    Reserve     (int res);
        void    Go          ();
        void    SetData     ();
        void    SetVolume   (double db);
        void    Reset       ();
        void    Requery     ();        
        void    EnableReload();

    private:
        struct tCtrls
        {
            CTime     *Length;
            CBufStat  *BufStat;
            QAction   *Begin;
            QAction   *Rwd;
            QAction   *Go;
            QAction   *Fwd;
            QAction   *End;
            QAction   *Reload;
            QSlider   *Volume;
            QSlider   *Position;
            CTime     *Time;
            QIcon      Playing;
            QIcon      Paused;
        };
        tCtrls  Ctrls;
   
    private:
        double    Pos;
        bool      ReadAfterWrite;
        void      set_tooltip   (bool play_on_click);


    private slots:
        void      side_changed  (int side);
        void      vol_changed   (int value_in_db);
        void      pos_changed   (int seconds);
        void      begin         ();
        void      fwd           ();
        void      rwd           ();
        void      end           ();
};


#endif
