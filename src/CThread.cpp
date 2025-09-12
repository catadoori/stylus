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

#include "../debug.h"

#include "CThread.h"
#include "CAStream.h"
#include "CFile.h"
#include "CAudio.h"
#include "CDetector.h"
#include "CApp.h"
#include "CBufSeg.h"
#include "CGui.h"
#include <QDebug>


#include <iostream>


extern CApp * App;


CThread::CThread(CAStream * parent, bool write_to_buff):
    QThread(0),
    Parent(parent),
    _OldBReserve(-1),
    SegmentCount(0)
{
    set_name(parent->ToDisk(), parent->ToDisk() ^ write_to_buff);
}

void CThread::Start()
{
    start();
}


CThread::~CThread()
{
    State.Add(Ending);
    DEBUG(2,1, "CThread::~CThread() " << DebugName << " [leave]");
}

#if 0
#include <pthread.h>
pthread_t tid;
int ret;
struct sched_param param;
int priority;
/* sched_priority will be the priority of the thread */
sched_param.sched_priority = priority;
/* only supported policy, others will result in ENOTSUP */

policy = SCHED_OTHER;
/* scheduling parameters of target thread */
ret = pthread_setschedparam(tid, policy, &param);

pthread_setschedparam() returns zero after completing successfully. Any other returned value indicates that an error occurred. When either of the following conditions occurs, the pthread_setschedparam() function fails and returns an error value.

#endif


void CThread::Pause(bool wait)
{
    if (!State.Has(Idle) && State.Has(Active))
    {
        State.Remove(Working);
        if (State.Has(Blocked))
        {
            Signal();
            DEBUG(DB_ENDING, 2, DebugName << " is blocked");
        }
        if (wait)
        {
            Signal();
            DEBUG(DB_ENDING, 2, "WaitForState()");
            WaitForState(Idle, Working);
        }
    }
}

void CThread::idle()
{
    State.Add(Idle);
#if 0
    CApp::USleep(10000);
#else
    msleep(25);
#endif
}



void CThread::Signal()
{
    Mutex.lock();
    Cond.wakeAll();
    Mutex.unlock();
    State.Remove(Blocked);
}

void CThread::Continue(bool wait)
{
    if (!State.Has(Working))
    {
        State.Add(Working);
        if (wait)
            WaitForState(Working, Idle);
    }
}



void CThread::WaitForState(States must_be_set, States must_be_cleared)
{
    DEBUG(DB_PAUSING, 0, __PRETTY_FUNCTION__ <<  "(" << DebugName << ") " << must_be_set << " " << must_be_cleared);
    int x = 0;
    while ((must_be_set && !State.Has(must_be_set)) || State.Has(must_be_cleared))
    {
        CApp::USleep(1000);
        if (State.Has(Blocked))
            Signal();
#if 1
        if (++x == 3000)
        {
            throw EInternal( __PRETTY_FUNCTION__, DebugName);
        }
#endif
    }
}

void CThread::End(bool wait)
{
    if (State.Has(Idle) || State.Has(Active))
    {
        State.Add(Ending);
        if (wait)
        {
            WaitForState(NoState, Ending);
        }
    }
}


bool CThread::set_priority(int prio)
{
    struct sched_param param;
    param.sched_priority = prio;
    bool ok = pthread_setschedparam (pthread_self(), SCHED_FIFO, &param) == 0;
    if (ok)
        qDebug() << "Thread '" << DebugName << "' runs with priority " << prio;
    else
        qWarning("Could not set priority for %s to %d", DebugName, prio);
    return ok;
}

void CThread::run()
{
#ifdef CONFIG_STYLUS_THREAD_PRORITY
    if (Parent->ToDisk() && IsFileThread() )
        set_priority(90);
    if (Parent->ToDisk() && IsAudioThread())
        set_priority(70);
#endif
    try
    {
        setStackSize(1024 * 1);
        State.Add(Idle);
        State.Add(Active);
        (this->*LoopFunc) ();
        State.Clear();
    }
    catch (Except & e)
    {
        State.Clear();
        qDebug() << "Exception caught in thread " << DebugName << ". Message = " << e.GetMessage();
        emit Exception();
    }
}


void CThread::wait()
{
    Mutex.lock();
    State.Add(Blocked);
    Cond.wait(&Mutex);
    Mutex.unlock();
    State.Remove(Blocked);
}

#ifdef CONFIG_STYLUS_WITH_LOCKING

int CThread::GetSegmentCount()  const
{
    QMutexLocker l (&Mutex);
    int s = SegmentCount;
    return s;
}

void CThread::SetSegmentCount  (int s)
{
    QMutexLocker l (&Mutex);
    SegmentCount = s;
}

void CThread::IncSegmentCount ()
{
    QMutexLocker l(&Mutex);
    ++SegmentCount;
}

#endif

void CThread::set_name (bool record, bool file)
{
    strcpy(DebugName, record ? "Record-": "Play-");
    strcat(DebugName, file ? "File" : "Audio");
}

#ifdef DEBUG_CONTEXT
void CThread::DumpState(int scenario, int level)
{
    DEBUG(scenario, level, DebugName << ": State = " <<  (int)State.Get());
}
#else
void CThread::DumpState(int, int)
{
}
#endif


bool CThread::IsAudioThread() const
{
    return Parent->GetAudio() == this;
}

bool CThread::IsFileThread() const
{
    return Parent->GetFile() == this;
}

