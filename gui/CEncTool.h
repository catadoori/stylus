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


#ifndef CENCTOOL_H
#define CENCTOOL_H

#include <QToolBar>
class QAction;
class QToolButton;

#include "CBaseEnc.h"

class CEncoderStream;

class CEncTool: public QToolBar
{
    Q_OBJECT
    public:
                CEncTool    (QWidget * parent);
               ~CEncTool    ();
        void	Reset       ();
    private:
        struct tCtrls
        {
            struct tGo
            {
                QToolButton *But;
                QAction     *Start;
                QAction     *Stop;
                QAction     *Local;
                QAction     *CleanLocal;
                QAction     *Remote;
                QAction     *CleanRemote;
                QAction     *StreamInfo;
                QAction     *RecChainInfo;
                QAction     *Side;
                QAction     *Track;
            };
            tGo Go;
            
        };

        tCtrls          Ctrls;
        CEncoderStream *Encoder;
        CBaseEnc::tInfo TrackInfo;

        void    encode  ();
        bool    clean_up(bool remote);
    private slots:
        void    go_pressed      (QAction*);
        void    finished        ();
        void    terminated      ();
        void    progress        (int percentage);
        void    worker_exception();
};

#endif
