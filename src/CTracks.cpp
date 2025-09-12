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
#include "CTracks.h"
#include "Excepts.h"

#include <iostream>     // FIXME temporär
#include <iomanip>      // FIXME temporär
#include "CAStream.h"   // FIXME temporär


CTracks::CTracks()
{
}


CTracks::~CTracks()
{
}


bool CTracks::tTrack::operator == (CTracks::tTrack const & rhs) const
{
 //   return Start == rhs.Start && End == rhs.End;
    return Start == rhs.Start;
}

bool CTracks::operator == (CTracks const & r) const
{
    if (Tracks.size() != r.Tracks.size())
        return false;
    tVec::const_iterator rhs, lhs;
    
    lhs = Tracks.begin();
    rhs = r.Tracks.begin();
    for (; lhs != Tracks.end(); ++ lhs, ++rhs)
        if ( * lhs != * rhs)
            return false;
    return true;
}

unsigned CTracks::GetScanStart() const
{
    unsigned ret = 0;
    for (tVec::const_iterator iter = Tracks.begin(); iter != Tracks.end(); ++iter)
    {
        if (!iter->Next) break;
        ret = iter->Next + 1;
    }
    return ret;
}

unsigned CTracks::GetTrackStart() const
{
    unsigned ret = 0;
    for (tVec::const_iterator iter = Tracks.begin(); iter != Tracks.end(); ++iter)
    {
        if (!iter->End) break;
        ret = iter->End + 1;
    }
    return ret;
}


void CTracks::Insert(unsigned start, unsigned end, unsigned next)
{
    Tracks.push_back(tTrack(start, end, next));
}

void CTracks::UpdateLast(unsigned end, unsigned next)
{
    if (Tracks.empty())
        throw EInternal(__FUNCTION__);
    else
    {    
        tVec::reference last = Tracks.back();
        last.Next = next;
        last.End = end;
    }
}

void CTracks::Clear()
{
    Tracks.clear();            
}

int CTracks::Count() const
{
    return Tracks.size();
}

void CTracks::Dump()
{
    std::cout << "\nLocated tracks: " << endl;

    for (tIter iter = Tracks.begin(); iter != Tracks.end(); ++iter)
    {
        std::cout << "  " << setw(2) << distance(Tracks.begin(), iter) +1 << "  "
                  << setw(7) << CAStream::SegmentsToSec(iter->Start) << "  "
                  << setw(7) << CAStream::SegmentsToSec(iter->End) << "  "
                  << setw(7) << CAStream::SegmentsToSec(iter->Next) << "  " <<  std::endl;
    }  
}


