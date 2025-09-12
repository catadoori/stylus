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
#include "CBuffer.h"
#include "CBufSeg.h"
#include "Excepts.h"

#include "CThread.h"
#include <QDebug>

CBuffer::CBuffer():

    SegCount(0),
    SegSize(0),
    FirstSegment(0),
    Head(-1),
    Segment(0)
{
}


void CBuffer::Init(int segments, int seg_size, int bytes_per_sample)
{
    SegCount = segments;
    SegSize = seg_size;
    delete [] Segment;
    Segment = new  CBufSeg [segments]; 
    for (int x = 0; x < segments; ++x)
    {
        Segment[x].Init(SegSize * bytes_per_sample);
    }
//    Capacity = SegCount * SegSize;
    Capacity = SegCount;
}


CBuffer::~CBuffer()
{
    delete [] Segment;
}

CBufSeg  * CBuffer::GetNewSegment()
{
#ifdef CONFIG_STYLUS_WITH_LOCKING
    QMutexLocker l (&Mutex);
#endif
    ++Head;
    if (Head == SegCount)
        Head = 0;
    CBufSeg * seg = Segment + Head;
    return seg;
}


char * CBuffer::GetAudioData(int _seg)
{
#ifdef CONFIG_STYLUS_WITH_LOCKING
    QMutexLocker l (&Mutex);
#endif
    char * ret;
    int segment = (_seg - FirstSegment) % SegCount;

    CBufSeg & seg = Segment[segment];
    ret = seg.Audio.Data;
    return ret;
}

CBufSeg * CBuffer::GetSegment(int _seg)
{
#ifdef CONFIG_STYLUS_WITH_LOCKING
    QMutexLocker l (&Mutex);
#endif
    // int offs = ((start_sample - FirstSample) / SegSize) % SegCount;
    int offs = (_seg - FirstSegment)  % SegCount;
    return Segment + offs;
}


void CBuffer::Reset(int first_segment)
{
#ifdef CONFIG_STYLUS_WITH_LOCKING
    QMutexLocker l (&Mutex);
#endif
    Head = -1;
    FirstSegment = first_segment;
}




