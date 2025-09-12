/***************************************************************************
 *   Copyright (C) 2006 by Harry   *
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
#ifndef CTHREAD_H
#define CTHREAD_H
#include "../config.h"

/**
	@author Harry <harry@actionpussyclub.de>
*/

#include "CState.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class CAStream;
class CBufSeg;

class CThread: public QThread
{
    Q_OBJECT
    public:
        enum States
        {
            NoState   = 0x00,
            Working   = 0x01,
            Idle      = 0x02,
            Active    = 0x04,
            Blocked   = 0x08,
            Ending    = 0x10,
        };
        CState <States> State;
                            CThread         (CAStream * parent, bool write_to_buff);
                           ~CThread         ();
                void        Start           ();
                void        End             (bool wait);

                bool        Open            (char const * device = 0);
                bool        Close           ();
                
                void        Signal          ();
                void        Pause           (bool wait = false);
                void        Continue        (bool wait = false);
                /*
                    Continues a paused thread. If it has the Sleeping flag set, a call
                    to this function has no effect. 
                */
#ifdef CONFIG_STYLUS_WITH_LOCKING
                int         GetSegmentCount ()  const;
                void        SetSegmentCount (int s);
                void        IncSegmentCount ();
#else
                int         GetSegmentCount ()  const           { return SegmentCount; }
                void        SetSegmentCount (int s)             { SegmentCount = s;    }
                void        IncSegmentCount ()                  { ++SegmentCount;      }
#endif
                
                void        WaitForState    (States set, States cleared = NoState);
                // Blocks the calling thread until State has the desired state(s) where

                void        DumpState       (int scenario, int level);

                bool        IsFileThread    () const;
                bool        IsAudioThread   () const;

    signals:
                void        BufferReserve   (int percent);
                void        Exception       ();
    protected:
        CAStream        *Parent;
        QMutex mutable   Mutex;
        QWaitCondition   Cond;
        int              _OldBReserve;

        int         SegmentCount;

                typedef void (CThread::*tLoopFunc)();
                tLoopFunc    LoopFunc;

                void         run                ();

                void         pause              ();
                void         idle               ();
                void         wait               ();
                bool         set_priority       (int prio);
    
// #ifdef DEBUG_CONTEXT
        char DebugName [20];
        void set_name (bool record, bool file);
// #endif
                
};


#endif

