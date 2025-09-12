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
#ifndef CFILE_H
#define CFILE_H
#include "../debug.h"
#include "CThread.h"
#include <fstream>
#include "Excepts.h"

/**
	@author Harry <harry@actionpussyclub.de>
*/

class CAStream;
class CFile: public CThread 
{
    Q_OBJECT    
    public:

                    CFile       (CAStream * parent, bool write_to_buff);
                    CFile       (CAStream * parent);
                   ~CFile       ();
        bool        Open        (char const * file_name);
        bool        Close       ();
        bool        EoF         () { return Stream.eof(); }
        void        Seek        (int target_segment, bool clear_stream);

        /* Taken from gramofile/bplaysrc/bplay.c */
        /* Definitions for Microsoft WAVE format */

        /* it's in chunks like .voc and AMIGA iff, but my source say there
           are in only in this combination, so I combined them in one header;
           it works on all WAVE-file I have
        */
        struct tWavHead
        {
          unsigned int   main_chunk;    /* 'RIFF' */
          unsigned int   length;        /* Length of rest of file */
          unsigned int   chunk_type;    /* 'WAVE' */

          unsigned int   sub_chunk;     /* 'fmt ' */
          unsigned int   sc_len;        /* length of sub_chunk, =16 (rest of chunk) */
          unsigned short format;        /* should be 1 for PCM-code */
          unsigned short modus;         /* 1 Mono, 2 Stereo */
          unsigned int   sample_fq;     /* frequence of sample */
          unsigned int   byte_p_sec;
          unsigned short byte_p_spl;    /* samplesize; 1 or 2 bytes */
          unsigned short bit_p_spl;     /* 8, 12 or 16 bit */

          unsigned int   data_chunk;    /* 'data' */
          unsigned int   data_length;   /* samplecount (lenth of rest of block?)*/
        };

        static char const * TagRiff;
        static char const * TagWave;
        static char const * TagFmt;
        static char const * TagData;
    signals:
        void        Progress    (int percentage);
        void        BufferXRun  ();

    protected:


            tWavHead    WavHead;
            std::
            fstream     Stream;

        void    open            ();
        bool    write_header    ();
        bool    read_header     ();

        void    read_buff       ();
        void    write_buff      ();
        void    sleep           ();

};
#endif
