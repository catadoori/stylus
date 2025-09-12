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
#ifndef CFLACENC_H
#define CFLACENC_H

#include "../config.h"
#if !defined(CONFIG_STYLUS_NO_FLACENC)
#include "CBaseEnc.h"

#include <FLAC/stream_encoder.h>
#include <FLAC/metadata.h>

/**
	@author Harry <harry@actionpussyclub.de>
*/

class CFlacEnc: public CBaseEnc
{
    public:
                        CFlacEnc    ();
        virtual        ~CFlacEnc    ();
                bool    Init        (tInfo const &, bool local, bool remote);
                void    Process     (CBufSeg const * seg);
                bool    Flush       ();

    private:
        FLAC__StreamEncoder * Encoder;
        FLAC__StreamMetadata* MetaData;


        static
        FLAC__StreamEncoderWriteStatus  write_callback (FLAC__StreamEncoder const * encoder,
                                                        const FLAC__byte buffer[],
                                                        size_t bytes,
                                                        unsigned samples,
                                                        unsigned current_frame,
                                                        void *client_data);
        static
        FLAC__StreamEncoderSeekStatus   seek_callback (const FLAC__StreamEncoder *encoder,
                                                       FLAC__uint64 byte_offset, void *client_data);
        static
        FLAC__StreamEncoderTellStatus   tell_callback (const FLAC__StreamEncoder *encoder,
                                                       FLAC__uint64 *byte_offset, void *client_data);


        typedef void (CFlacEnc::*tProcFunc)(CBufSeg const*);
        tProcFunc   ProcFunc;
        FLAC__int32*Buffer;
        qreal       Scale;
        void        set_comment (eCommentTags tag, QString const & data);
        void        process_16  (CBufSeg const *);
        void        process_32  (CBufSeg const *);

};


#endif
#endif
