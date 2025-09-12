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

#ifndef CTRACKS_H
#define CTRACKS_H

/**
	@author Harry <harry@actionpussyclub.de>
*/

#include <vector>
using namespace std;

class CTracks
{
    public:
        struct tTrack
        {
            tTrack (): Start(0), End(0), Next(0) {}
            tTrack (unsigned s, unsigned e, unsigned n): Start(s), End(e), Next(n) {}

            unsigned    Start;
            unsigned    End;
            unsigned    Next;

            bool        operator == (tTrack const & rhs) const;
            bool        operator != (tTrack const & rhs) const { return ! operator == (rhs); }

        };
    
        typedef vector<tTrack>       tVec;
        typedef tVec::iterator       tIter;
        typedef tVec::const_iterator tConstIter;
        
        tConstIter      Begin           ()  const    { return Tracks.begin(); }
        tConstIter      End             ()  const    { return Tracks.end();   }
        
                        CTracks         ();
                       ~CTracks         ();
        bool            operator ==     (CTracks const & rhs) const;
                                        
        unsigned        GetScanStart    ()  const;
        unsigned        GetTrackStart   ()  const;

        void            Clear           ();     
        void            Insert          (unsigned start, unsigned min_pos, unsigned next);
        void            UpdateLast      (unsigned min_pos, unsigned next);
        /*
            Sets or overrides these values of Tracks' last record. If Tracks is empty, an empty
            record will be inserted.
        */
        int             Count           ()  const;
        void            Dump            (); // FIXME temporär
    private:
        tVec  Tracks;
};

#endif
