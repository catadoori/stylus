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
#include "../config.h"
#if !defined (CONFIG_STYLUS_NO_VORBISENC)
#include "CVorbisEnc.h"
#include <vorbis/vorbisenc.h>

#include "Excepts.h"
#include <math.h>
#include <stdlib.h>
#include <CBufSeg.h>
#include <QFile>

#include "CApp.h"
#include "CSettings.h"
#include <QDebug>

extern CApp * App;



CVorbisEnc::CVorbisEnc()
{
}

CVorbisEnc::~CVorbisEnc()
{
    ogg_stream_clear(&Stream);
    vorbis_block_clear(&Block);
    vorbis_dsp_clear(&Dsp);
    vorbis_comment_clear(&Comment);
    vorbis_info_clear(&Info);
}




bool CVorbisEnc::Init(tInfo const & info, bool local, bool remote)
{
    CBaseEnc::Init(info, local, remote);
    vorbis_info_init(&Info);
    if (vorbis_encode_init_vbr(&Info, 2, info.In.Rate, info.Out.Quality))
       return false;

    vorbis_comment_init(&Comment);
    char const * cap = "stylus";
    vorbis_comment_add_tag(&Comment, CommentTag(ctEncoder), (char*)cap);
    vorbis_comment_add_tag(&Comment, CommentTag(ctTitle), info.Tags.Title.toUtf8().data());
    vorbis_comment_add_tag(&Comment, CommentTag(ctAlbum), info.Tags.Album.toUtf8().data());
    vorbis_comment_add_tag(&Comment, CommentTag(ctArtist), info.Tags.Artist.toUtf8().data());
    vorbis_comment_add_tag(&Comment, CommentTag(ctGenre), info.Tags.Genre.toUtf8().data());
    vorbis_comment_add_tag(&Comment, CommentTag(ctDate), info.Tags.Year.toUtf8().data());
    vorbis_comment_add_tag(&Comment, CommentTag(ctTrackNumber), info.Tags.Track.toUtf8().data());
    vorbis_comment_add_tag(&Comment, CommentTag(ctComment), info.Tags.ComposedComment.toUtf8().data());

    srand(time(NULL));
    if (vorbis_analysis_init(&Dsp, &Info) ||
        vorbis_block_init(&Dsp, &Block) ||
        ogg_stream_init(&Stream,rand()))
        return false;

    ogg_packet bitstream_header;
    ogg_packet comment_header;
    ogg_packet codebook_header;

    if (vorbis_analysis_headerout(&Dsp, &Comment, &bitstream_header,
                                  &comment_header, &codebook_header) ||
       ogg_stream_packetin(&Stream, &bitstream_header) ||
       ogg_stream_packetin(&Stream, &comment_header) ||
       ogg_stream_packetin(&Stream, &codebook_header))
          return false;


    while (ogg_stream_flush(&Stream, &Page))
        write();

    if (info.In.Format == SND_PCM_FORMAT_S16_LE)
    {
        ProcFunc = &CVorbisEnc::process_16;
        Scale = SHRT_MAX / info.Out.Scale;
    }
    else if (info.In.Format == SND_PCM_FORMAT_S32_LE)
    {
        ProcFunc = &CVorbisEnc::process_32;
        Scale = INT_MAX / info.Out.Scale;
    }
    return true;
}

void CVorbisEnc::write()
{
    CBaseEnc::write ((char*)Page.header, Page.header_len);
    CBaseEnc::write ((char*)Page.body, Page.body_len); 
}

inline void CVorbisEnc::Process(CBufSeg const * seg)
{
    float ** buffer = vorbis_analysis_buffer (&Dsp, FrameSize);
    (this->*ProcFunc) (seg, buffer);
    vorbis_analysis_wrote(&Dsp, FrameSize);
    do_vorbis();
}

void CVorbisEnc::process_16(CBufSeg const * seg, float ** buffer)
{
    tSample16 * samp = (tSample16*) seg->Audio.Data;
    tSample16 * end = samp + FrameSize;
    for (int x = 0; samp < end; ++ samp, ++x)
    {
        buffer[0][x] = samp->Left  / Scale;
        buffer[1][x] = samp->Right / Scale;
    }
}

void CVorbisEnc::process_32(CBufSeg const * seg, float ** buffer)
{
    tSample32 * samp = (tSample32*) seg->Audio.Data;
    tSample32 * end = samp + FrameSize;
    for (int x = 0; samp < end; ++ samp, ++x)
    {
        buffer[0][x] = samp->Left  / Scale;
        buffer[1][x] = samp->Right / Scale;
    }
}

void CVorbisEnc::do_vorbis()
{
    while (vorbis_analysis_blockout(&Dsp, &Block) == 1)
    {
        vorbis_analysis(&Block, NULL);
        vorbis_bitrate_addblock(&Block);
        while (vorbis_bitrate_flushpacket(&Dsp, &Packet))
        {
            ogg_stream_packetin(&Stream, &Packet);
            while (ogg_stream_pageout(&Stream, &Page))
            {
                write();
                if (ogg_page_eos(&Page))
                    break;
            }
        }
    }
}

#endif

