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


#ifndef CMP3ENC_H
#define CMP3ENC_H

#include "CBaseEnc.h"
#include <lame/lame.h>


#include "CBufSeg.h"

struct id3_tag;
class QByteArray;


class CMp3Enc: public CBaseEnc
{
    public:
                        CMp3Enc     ();
        virtual        ~CMp3Enc     ();
                bool    Init        (tInfo const &, bool local, bool remote);
                void    Process     (CBufSeg const * seg);
                bool    Flush       ();

                bool    ReadTags    (QString const & file_name, tInfo & out);

    private:
        struct tTags
        {
            QByteArray Title;
            QByteArray Artist;
            QByteArray Album;
            QByteArray Year;
            QByteArray Genre;
            QByteArray Comment;
            QByteArray ComposedComment;
            QByteArray RecChain;
            QByteArray Track;
            tTags & operator = (CBaseEnc::tInfo::tTags const & rhs);
        } Tags;
    
        typedef void (CMp3Enc::*tProcFunc)(CBufSeg const*);
        lame_global_struct* Lame;
        CBufSeg             Left;
        CBufSeg             Right;
        tProcFunc           ProcFunc;
        int                 InSize;    // size of one input chunk in bytes 
        unsigned char       Mp3Buf[LAME_MAXMP3BUFFER];
        
        
        void    process_16 (CBufSeg const* in_samples);
        void    process_32 (CBufSeg const* in_samples);

        void    str_from_frame(id3_tag const * tag, char const * field, QString & text);

};

#endif
