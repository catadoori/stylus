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
#include "CFlacEnc.h"
#if !defined (CONFIG_STYLUS_NO_FLACENC)

#include "CApp.h"
#include "Excepts.h"
#include "CBufSeg.h"
#include "CVorbisEnc.h"

extern CApp * App;

CFlacEnc::CFlacEnc():
    Encoder(0),
    MetaData(0),    
    Buffer(0)

{

}


CFlacEnc::~CFlacEnc()
{
    if (MetaData)
        FLAC__metadata_object_delete(MetaData);
    if (Encoder)
        FLAC__stream_encoder_delete(Encoder);
    delete [] Buffer;        
}

void CFlacEnc::set_comment(eCommentTags tag, QString const & data)
{
    FLAC__StreamMetadata_VorbisComment_Entry entry;
    FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair( &entry,
          CommentTag(tag), data.toUtf8().constData());
    FLAC__metadata_object_vorbiscomment_append_comment(MetaData, entry, true);
}


bool CFlacEnc::Init(tInfo const & info, bool local, bool remote)
{
    CBaseEnc::Init(info, local, remote);
    CSettings::CAudio const & sa = App->Settings.Audio;
    bool ret;
    Encoder = FLAC__stream_encoder_new();
    ret = Encoder != NULL;
    ret = ret && FLAC__stream_encoder_set_channels(Encoder, 2);
    unsigned bits = sa.GetBitsPerSample();
    switch (bits)
    {
        case 16:
            ProcFunc = &CFlacEnc::process_16;
            Scale = info.Out.Scale;            
            break;
        case 32:
            ProcFunc = &CFlacEnc::process_32;
            Scale = info.Out.Scale / (1 << 8);
            bits = 24;
            break;
        default:
            ENSURE(false);
    }

    Buffer = new FLAC__int32 [GetFrameSize()*2];
    ret = ret && FLAC__stream_encoder_set_bits_per_sample(Encoder, bits);
    ret = ret && FLAC__stream_encoder_set_sample_rate(Encoder, sa.GetRate());
    ret = ret &&  FLAC__stream_encoder_set_compression_level(Encoder, info.Out.Quality);
    ret = ret && FLAC__stream_encoder_set_total_samples_estimate(Encoder, info.In.Length);
    ret = ret && FLAC__stream_encoder_set_verify(Encoder, true);

    MetaData = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
    if (ret == false || MetaData == NULL)
        return false;
            
    set_comment(ctTrackNumber, info.Tags.Track);
    set_comment(ctTitle, info.Tags.Title);
    set_comment(ctComment, info.Tags.ComposedComment);
    set_comment(ctGenre, info.Tags.Genre);
    set_comment(ctArtist, info.Tags.Artist);
    set_comment(ctAlbum, info.Tags.Album);
    set_comment(ctDate, info.Tags.Year);

    ret = FLAC__stream_encoder_set_metadata(Encoder, &MetaData, 1);
    FLAC__StreamEncoderInitStatus stat;
    stat = FLAC__stream_encoder_init_stream(Encoder, write_callback, seek_callback,
                                            tell_callback, NULL, (void*)this);
    ret = ret && stat == FLAC__STREAM_ENCODER_INIT_STATUS_OK;
    return ret;
}



void CFlacEnc::process_16(CBufSeg const * seg)
{
    tSample16 * samp = (tSample16*) seg->Audio.Data;
    tSample16 * end = samp + FrameSize;
    for (int x = 0; samp < end; ++ samp, x+= 2)
    {

        Buffer[x] = clipped_int(samp->Left * Scale, SHRT_MIN, SHRT_MAX);
        Buffer[x+1] = clipped_int(samp->Right * Scale, SHRT_MIN, SHRT_MAX);
    }
}

void CFlacEnc::process_32(CBufSeg const * seg)
{
    tSample32 * samp = (tSample32*) seg->Audio.Data;
    tSample32 * end = samp + FrameSize;
    
    int const bit_24_max = INT_MAX >> 8;
    int const bit_24_min = INT_MIN >> 8;
    for (int x = 0; samp < end; ++ samp, x+= 2)
    {
        Buffer[x] = clipped_int(samp->Left * Scale, bit_24_min, bit_24_max);
        Buffer[x+1] = clipped_int(samp->Right * Scale, bit_24_min, bit_24_max);
    }
}

inline void CFlacEnc::Process(CBufSeg const * seg)
{
    FLAC__StreamEncoderState state;
    (this->*ProcFunc)(seg);
    if (FLAC__stream_encoder_process_interleaved(Encoder, Buffer, GetFrameSize()) == false)
    {
        state = FLAC__stream_encoder_get_state(Encoder);
        state = state;
    }
}

bool CFlacEnc::Flush()
{
    return FLAC__stream_encoder_finish(Encoder);

}


FLAC__StreamEncoderWriteStatus
CFlacEnc::write_callback (FLAC__StreamEncoder const*,  const FLAC__byte buffer[],
                    size_t bytes, unsigned, unsigned, void *client_data)
{
    CFlacEnc *  obj = (CFlacEnc*) client_data;
    obj->write((char*)buffer, bytes);
    return FLAC__STREAM_ENCODER_WRITE_STATUS_OK; // write may throw exception
}


FLAC__StreamEncoderSeekStatus
CFlacEnc::seek_callback (const FLAC__StreamEncoder *, FLAC__uint64 offs, void *client)
{
    CFlacEnc * obj = (CFlacEnc*) client;
    obj->seek(offs);
    return FLAC__STREAM_ENCODER_SEEK_STATUS_OK; // write seek throws exception
}

FLAC__StreamEncoderTellStatus
CFlacEnc::tell_callback (const FLAC__StreamEncoder *, FLAC__uint64 *offs, void *client)
{
    CFlacEnc * obj = (CFlacEnc*) client;
    obj->tell((qint64*)offs);
    return FLAC__STREAM_ENCODER_TELL_STATUS_OK;
}
#endif
