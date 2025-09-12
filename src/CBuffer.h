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
#ifndef CBUFFER_H
#define CBUFFER_H

#include "../debug.h"
#include "../config.h"
#include <unistd.h>

/**
	@author Harry <harry@actionpussyclub.de>
*/

class CBufSeg;

#ifdef CONFIG_STYLUS_WITH_LOCKING
#include <QMutex>
#endif        



class CBuffer
{
    private:
        int          SegCount;     // Number of segments
        int          SegSize;      // Number of samples stored in every segment

        int          FirstSegment; // Offset of the first segment in Buffer
        int          Head;         // Pointer to the current segment     
        CBufSeg*     Segment; 
        int          Capacity;     // Capacity of whole buffer (sum of segments) in segments
#ifdef CONFIG_STYLUS_WITH_LOCKING
        QMutex       Mutex;
#endif
    public:

                    CBuffer       ();
                   ~CBuffer       ();
        void        Init          (int segments, int samples, int bytes_p_sample);

        CBufSeg *   GetNewSegment ();
        /*
            Aquires a new segment from the ring buffer. The returned pointer can
            be used for reading from or writing to the segement.
        */

        char *      GetAudioData    (int segmennt);
        /*
            retuns the data block that contains the requested segment
        */
        CBufSeg *   GetSegment      (int segment);

        int         GetSegCount     () const                         { return SegCount;   }
        int         GetSegSize      () const                         { return SegSize;    }

        int         GetCapacity     () const                         { return Capacity;   }
        int         GetFirstSegment () const                         { return FirstSegment;}

        void        Reset           (int first_segment);

};

#endif
