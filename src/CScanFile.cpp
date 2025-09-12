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

#include "CScanFile.h"
#include "CAStream.h"
#include "CBuffer.h"
#include "CApp.h"
#include <QDebug>

extern CApp * App;

CScanFile::~CScanFile()
{

}

CScanFile::CScanFile(CAStream * parent, int read_count):
   CFile(parent),
   ReadCount(read_count)
{
    LoopFunc = (tLoopFunc) &CScanFile::scan;
}


void CScanFile::scan()
{
    CBufSeg * seg = Parent->GetBuffer()->GetNewSegment();
    int progress = -1;
    while (!State.Has(Ending))
    {
        if (State.Has(Working))
        {            
            State.Remove(Idle);
            if (Stream.good() && GetSegmentCount() <= Parent->GetSegmentCount())
            {
                Stream.read(seg->Audio.Data, ReadCount);
                IncSegmentCount();
                Parent->SetWorkSegment(seg);
                Parent->ProcessSegment(true);
                if (Parent->GetSegmentCount())
                {
                    int p =  GetSegmentCount() / (Parent->GetSegmentCount() / 100);
                    if (p != progress)
                    {
                        progress = p;
                        emit Progress(progress);
                    }
               }
            }
            else
                State.Add(Ending);
        }
        else
            idle();
    }
}

