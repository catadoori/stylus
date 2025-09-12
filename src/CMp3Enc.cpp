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



#include "CMp3Enc.h"
#include <lame/lame.h>
#include <alsa/asoundlib.h>

#include "Excepts.h"
#include "CBufSeg.h"
#include <QFile>


#include <id3tag.h>
#include <QDebug>

CMp3Enc::CMp3Enc():
    CBaseEnc(),
    Lame(0)
{

}

CMp3Enc::~CMp3Enc()
{
    if (Lame)
        lame_close(Lame);
}

CMp3Enc::tTags & CMp3Enc::tTags::operator = (CBaseEnc::tInfo::tTags const & rhs)
{
    Title = rhs.Title.toLatin1();
    Artist = rhs.Artist.toLatin1();
    Album = rhs.Album.toLatin1();
    Year = rhs.Year.toLatin1();
    Genre = rhs.Genre.toLatin1();
    Comment = rhs.Comment.toLatin1();
    ComposedComment = rhs.ComposedComment.toLatin1();
    RecChain = rhs.RecChain.toLatin1();
    Track = rhs.Track.toLatin1();
    return *this;
}

bool CMp3Enc::Init(tInfo const & info, bool local, bool remote)
{
    CBaseEnc::Init(info, local, remote);

    Tags = info.Tags;
    Lame = lame_init();
    if (Lame == 0)
        throw EEncoder("could not initialize liblame");

    if (lame_set_in_samplerate(Lame, info.In.Rate) ||
        lame_set_out_samplerate(Lame, 48000) ||
        lame_set_brate(Lame, info.Out.Quality) ||
        lame_set_VBR(Lame, info.Out.Mp3Mode) ||
        lame_set_quality(Lame, info.Out.EncQuality) ||
        lame_set_scale_left(Lame, info.Out.Scale) ||
        lame_set_scale_right(Lame, info.Out.Scale) ||
        ((info.Out.Mp3Mode == vbr_abr) ?
            lame_set_VBR_mean_bitrate_kbps(Lame, info.Out.Quality) : 0) ||
        ((info.Out.Mp3Mode == vbr_rh) ? lame_set_VBR_q(Lame, info.Out.Quality) : 0))
       throw EEncoder("could not set parameters");

    id3tag_init (Lame);
    id3tag_set_title(Lame, Tags.Title.constData());
    id3tag_set_artist(Lame, Tags.Artist.constData());
    id3tag_set_album(Lame, Tags.Album.constData());
    id3tag_set_year(Lame, Tags.Year.constData());
    id3tag_set_comment(Lame, Tags.ComposedComment.constData());
    id3tag_set_genre(Lame, Tags.Genre.constData());
    id3tag_set_track(Lame, Tags.Track.constData());

    if (lame_init_params(Lame) == -1)
        throw EEncoder("error on parameter initializing.");

    FrameSize = lame_get_framesize(Lame);
    if (FrameSize == 0)
        throw EEncoder("framesize == 0");

    if (info.In.Format == SND_PCM_FORMAT_S16_LE)
    {
        ProcFunc = &CMp3Enc::process_16;
        InSize = FrameSize * 4;
    }
    else if (info.In.Format == SND_PCM_FORMAT_S32_LE)
    {
        InSize = FrameSize * 8;
        ProcFunc = &CMp3Enc::process_32;
        Left.Init(InSize / 2);
        Right.Init(InSize / 2);
    }
    return true;
}

inline void CMp3Enc::Process(CBufSeg const * seg)
{
    (this->*ProcFunc) (seg);
}

bool CMp3Enc::Flush()
{
    int bytes = lame_encode_flush(Lame, Mp3Buf, sizeof(Mp3Buf));
    if (Local)
        Local->write((char*)Mp3Buf, bytes);
    if (Remote)
        Remote->write((char*)Mp3Buf, bytes);
        
    return bytes >= 0; 
}




void CMp3Enc::process_16(CBufSeg const * seg)
{
    int bytes = lame_encode_buffer_interleaved(Lame, (short int*)seg->Audio.Data, FrameSize,
                                                Mp3Buf, sizeof(Mp3Buf));
    write((char*)Mp3Buf, bytes);
}

void CMp3Enc::process_32(CBufSeg const * seg)
{
    tSample32 * samp = (tSample32*) seg->Audio.Data;
    tSample32 * end = samp + FrameSize;
    int *l, *left, *r, *right;
    left = l = (int*) Left.Audio.Data;
    right = r = (int*) Right.Audio.Data;

    for (; samp < end; ++ samp, ++l, ++r)
    {
        *l = samp->Left;
        *r = samp->Right;
    }
    int bytes = lame_encode_buffer_int(Lame, left, right, FrameSize, Mp3Buf, sizeof(Mp3Buf));
    write((char*)Mp3Buf, bytes);
}

void CMp3Enc::str_from_frame(id3_tag const * tag, char const * field, QString & text)
{
    id3_frame * frame = id3_tag_findframe(tag, field, 0);
    if (frame)
    {
        QString s;
        int count = frame->nfields;
        if (count > 1)
        {
            id3_field const * fld =  &frame->fields[count - 1];
            id3_ucs4_t const * ucs4 = NULL;
            
            switch (fld->type)
            {
                case ID3_FIELD_TYPE_STRINGLIST: ucs4 = id3_field_getstrings(fld, 0); break;
                case ID3_FIELD_TYPE_STRINGFULL: ucs4 = id3_field_getfullstring(fld); break;
                default: break;
            }
            if (ucs4)
            {

                id3_latin1_t * lat = id3_ucs4_latin1duplicate(ucs4);
                s = (char const*)lat;
                free(lat);
            }
            text = s.toLatin1();
        }
    }    
}


bool CMp3Enc::ReadTags(QString const & file_name, tInfo  & out)
{
    CBaseEnc::ReadTags(file_name, out);
    id3_file * file = id3_file_open(file_name.toLatin1(), ID3_FILE_MODE_READONLY);
    if (file == NULL)
        throw EFileOpen(file_name);

    id3_tag * tag = id3_file_tag(file);
    if (tag)
    {
        str_from_frame(tag, ID3_FRAME_TITLE, out.Tags.Title);
        str_from_frame(tag, ID3_FRAME_ARTIST, out.Tags.Artist);
        str_from_frame(tag, ID3_FRAME_ALBUM, out.Tags.Album);
        str_from_frame(tag, ID3_FRAME_YEAR, out.Tags.Year);
        str_from_frame(tag, ID3_FRAME_COMMENT, out.Tags.Comment);
        out.Tags.SplitComment();
        str_from_frame(tag, ID3_FRAME_TRACK, out.Tags.Track);
//        str_from_frame(tag, recchain_tag(), out.Tags.RecChain);
        str_from_frame(tag, ID3_FRAME_GENRE, out.Tags.Genre);
        QString s(out.Tags.Genre);

        id3_ucs4_t const * ucs = id3_genre_index(s.toInt());
        id3_latin1_t * lat = id3_ucs4_latin1duplicate(ucs);
        out.Tags.Genre = (char const*)lat;
        free(lat);
    }
    id3_file_close(file);
    return true;
}
