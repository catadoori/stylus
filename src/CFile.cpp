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
#include "CFile.h"
#include "Excepts.h"

#include <iostream>

#include "CSettings.h"
#include "CBuffer.h"
#include "CAudio.h"
#include "CApp.h"
#include "CAStream.h"
#include "CDetector.h"
#include "../debug.h"
#include "CBufSeg.h"

#include <QDebug>


char const * CFile::TagRiff = "RIFF";
char const * CFile::TagWave = "WAVE";
char const * CFile::TagFmt = "fmt ";
char const * CFile::TagData = "data";

extern CApp * App;

extern bool _Pause1;


extern bool __Pause1;
extern bool __Pause2;

bool CFile::write_header()
{
    tWavHead header;
    size_t data_len = Parent->GetSegmentCount() * App->Settings.Audio.GetBytesPerSegment();
    memcpy(&(header.main_chunk), TagRiff, 4);
    header.length = sizeof(tWavHead) - 8 + sizeof(data_len);
    memcpy(&(header.chunk_type), TagWave, 4);
    memcpy(&(header.sub_chunk), TagFmt, 4);
    header.sc_len = 16;
    header.format = 1;
    header.modus = 2;
    header.sample_fq = App->Settings.Audio.GetRate();
    header.byte_p_sec = App->Settings.Audio.GetBytesPerSec();
    header.byte_p_spl = App->Settings.Audio.GetBytesPerSample();
    header.bit_p_spl = App->Settings.Audio.GetBitsPerSample();
    memcpy(&(header.data_chunk), TagData, 4);
    header.data_length = data_len;
    Stream.seekg(0, std::ios::beg);
    Stream.write((char*)&header, sizeof(header)); 
    Stream << std::flush;
    return Stream.good();
}


bool CFile::read_header()
{
    CSettings::CAudio & set = App->Settings.Audio;

    Stream.read((char*) &WavHead, sizeof(tWavHead));
    int count = Stream.gcount();
    if (count != sizeof(tWavHead))
        throw EFileOpen(Parent->GetFileName(), "could not read wav - WavHead.");
    if ((WavHead.main_chunk != *((unsigned int*) TagRiff)) ||
        (WavHead.chunk_type != *((unsigned int*) TagWave)) ||
        (WavHead.sub_chunk != *((unsigned int*) TagFmt)) ||
        (WavHead.data_chunk != *((unsigned int*) TagData)))
        throw EFileOpen(Parent->GetFileName(), "seems to be no wav file");

    if (!set.GetRate() || !set.GetFormat())
    {
       // set.Set(WavHead.modus, WavHead.sample_fq, WavHead.bit_p_spl );
        snd_pcm_format_t format;
        switch (WavHead.bit_p_spl)
        {
            case 16: format = SND_PCM_FORMAT_S16_LE; break;
            case 32: format = SND_PCM_FORMAT_S32_LE; break;
            default: throw EAudio("input file has unknown format type");
        }
        set.Set(format, WavHead.sample_fq);
    }
    Parent->SetSegmentCount(WavHead.data_length / set.GetBytesPerSegment());
   return true;
}


CFile::CFile(CAStream * parent):
    CThread(parent, false)
{

}

CFile::CFile(CAStream * parent, bool write_buffer):
    CThread(parent, write_buffer)
{
    LoopFunc = write_buffer ? (tLoopFunc) &CFile::write_buff : (tLoopFunc) &CFile::read_buff;
}


CFile::~CFile()
{
    if (Stream.is_open())
        Close();
}


bool CFile::Open(char const * /* file_name */ )
{
    bool ret;
    Close();
    SegmentCount = 0;
    CApp::CheckAccess(Parent->GetFileName(), Parent->ToDisk(), Parent->ToDisk());
    std::ios::openmode open_mode = Parent->ToDisk() ? std::ios::out : std::ios::in;
    Stream.open(Parent->GetFileName().toLatin1(), open_mode | std::ios::binary);
    ret = Stream.is_open();
    if (!ret)
        throw EFileOpen(Parent->GetFileName());
    if (!Parent->ToDisk() && Stream.eof())
        throw EFileOpen(Parent->GetFileName(), "file is empty");

    if (Parent->ToDisk())
    {
        if (!write_header())
            throw EFileWrite(Parent->GetFileName(), "could not write header");
    }
    else if (!read_header())
        throw EFileRead(Parent->GetFileName(), "could not read header");

    return ret;
}

bool CFile::Close()
{
    bool ret = true;
    if (Stream.is_open())
    {
        if (Parent->ToDisk())
        {
            write_header();
        }
        Stream.close();
        ret = !Stream.is_open();
        Stream.clear();
    }
    if (!ret)
        throw Except(Except::General, Parent->GetFileName(), "could not close file");

    return ret;
}

void CFile::Seek(int segment, bool clear_stream)
{
    if (!Stream.is_open())
        Open(Parent->GetMaster()->GetFileName().toLatin1());
    if (clear_stream)
        Stream.clear();
    int byte_offs = sizeof(tWavHead) + segment * App->Settings.Audio.GetBytesPerSegment();
    Stream.seekg(byte_offs, std::ios::beg);
    if (!Stream.good())
        throw Except(Except::General, Parent->GetFileName(), "could not do file seek");
}


void CFile::write_buff() // write data from file to buffer
{
    CSettings::CAudio const & sa = App->Settings.Audio;
    unsigned samples;
    CBufSeg * seg;

    while (!State.Has(Ending))
    {
        if (State.Has(Working))
        {
            State.Remove(Idle);
            if (Stream.good() && !Parent->IsSyncdSlave() )
            {
                CBufSeg * seg = Parent->GetBuffer()->GetNewSegment();
                Stream.read(seg->Audio.Data, sa.GetBytesPerSegment());
                samples = Stream.gcount() / sa.GetBytesPerSample();

                if (samples)
                    IncSegmentCount();
            }
            else
               CApp::USleep(3000);// msleep(3);

            seg = Parent->DuplexPbBuffer()->GetSegment(Parent->GetAudio()->GetSegmentCount());
            Parent->SetWorkSegment(seg);
            Parent->ProcessSegment(false);
            sleep();
        }
        else
            idle();

    }
}


void CFile::read_buff() // (for writing file)
{
    CSettings::CAudio const & sa = App->Settings.Audio;
    while (!State.Has(Ending))
    {
        if (State.Has(Working))
        {
            State.Remove(Idle);

            if (Stream.good())
            {
                if (GetSegmentCount() != Parent->GetAudio()->GetSegmentCount())
                {
                    CBufSeg * seg = Parent->GetBuffer()->GetSegment(GetSegmentCount());
                    Stream.write(seg->Audio.Data, sa.GetBytesPerSegment());
                    Parent->ProcessSegment(true);  // the segement to process is set by audio thread.
                    IncSegmentCount();
                    sleep();
                }
                else if (!State.Has(Ending))
                    wait();
            }
            else
                idle();
        }
        else
            idle();
    }
}

void CFile::sleep()
{
    int r = Parent->BufferReserve();
#if 0
    if (Parent->ToDisk() && r < 50)
        qDebug() << r << ": " << Parent->GetAudio()->GetSampleCount() << " / " << GetSampleCount();
#endif
    if (r < 0 && Parent->ToDisk())
    {
        emit BufferXRun();
    }
    else if (r != _OldBReserve)
    {
        _OldBReserve = r;
        emit BufferReserve(r);
    }
    if (r >= 100)
    {
        while (r > 100)
        {
            msleep(30);
            r = Parent->BufferReserve();
        }
        wait();
    }
}


