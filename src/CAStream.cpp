/***************************************************************************
 *   Copyright (C) 2006 by Harry   *
 *   harry@actionpussyclub.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later v ersion.                                   *
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
#include "../debug.h"
#include "CAStream.h"
#include "Excepts.h"

#include "CAudio.h"
#include "CFile.h"
#include "CBuffer.h"
#include "CBufSeg.h"
#include "CDetector.h"
#include "CNormalizer.h"
#include "CSettings.h"
#include "CScanFile.h"

#include "CApp.h"
#include <math.h>

#include <QTime>
#include <QDebug>
#include <QFile>


#include "CMp3Enc.h"
#if !defined(CONFIG_STYLUS_NO_VORBISENC)
#include "CVorbisEnc.h"
#endif
#include "CFlacEnc.h"

extern CApp * App;

double const CAStream::MaxSideSecs = 60 * 45.0;

CAStream::CAStream():
    Callback(0),
    File(0),
    Audio(0),
    Buffer(0),
    Master(0),
    Detector(0),
    Normalizer(0),
    Segment(0),
    Good(false),
    _ToDisk(false),
    SegmentCount(0),
    XRuns(0),
    Volume(0),
    MeterData(0),
    calc_rms(0)

{
}


CAStream::CAStream(bool rec, bool create_thds, bool with_audio):
    Callback(0),
    File(0),
    Audio(0),
    Buffer(0),
    Master(0),
    Detector(0),
    Normalizer(0),
    Segment(0),
    ActiveStream(0),
    Good(false),
    _ToDisk(rec),
    SegmentCount(0),
    XRuns(0),
    Volume(0),
    MeterData(0),
    calc_rms(0)

{
    if (with_audio)
        Audio = new CAudio(this, ToDisk());
    File = new CFile(this, !ToDisk());
    Detector = new CDetector;
    Normalizer = new CNormalizer;
    if (create_thds)
    {
        Audio->Start();
        File->Start();
    }
    if (MeterData)
        MeterData->Clear();
}



CAStream::~CAStream()
{
    if (Audio) Audio->End(true);
    if (File) File->End(true);
    free();

    delete Audio;
    delete File;
    delete Detector;
    delete Normalizer;
    delete Buffer;
}



void CAStream::free()
{
    if (Audio)
    {
        Audio->Pause(true);
        Audio->SetSegmentCount(0);
    }
    if (File)
    {
        File->Pause(true);
        File->SetSegmentCount(0);
    }
    SetSegmentCount(0);
}

void CAStream::Reset(bool to_disk)
{
    End();
    free();
    _ToDisk = to_disk;
    Master = this;
}

void CAStream::RecoverFromXRun()
{
    Audio->SetSegmentCount(0);
    File->SetSegmentCount(0);
    SetSegmentCount(0);

}

void CAStream::Init(QString const & file, int buff_seg_count, CAStream * master,
                    bool to_disk, tMeterData * meter)
{
    Good = false;
    End();
    free();

    _ToDisk = to_disk;
    MeterData = meter;

    if (!Buffer)
        Buffer = new CBuffer;
    if (ToDisk())
    {
        Src = Audio;
        Dst = File;
    }
    else
    {
        Src = File;
        Dst = Audio;
    }

    Volume = 1.0;

    if (MeterData)
        MeterData->Clear();

    CSettings::CAudio const & set = App->Settings.Audio;
    FileName = file;
    Segment = 0;
    Master = master ? master : this;

    File->Open(0); // File gets file name via this->GetFileName()

    Buffer->Init(buff_seg_count, set.GetSamplesPerSegment(), set.GetBytesPerSample());
    WorkSegment.Init(set.GetSamplesPerSegment() * set.GetBytesPerSample());
    SetActiveStream(this);
    Buffer->Reset(0);
    set_rms_func();

//    Audio->Open("default");
//    Audio->Open("nonsense");
#if 0
    Audio->Open("plughw:0,0");
#else
    // Audio->SetDevice("plughw:0,0");    
    Audio->SetDevice("default");    
#endif
    Audio->DoSignals(true);
    GetDetector()->Clear();
    GetNormalizer()->Clear();
    Good = true;

}


void CAStream::InitDetector()
{
    delete Detector;
    Detector = new CDetector;
}

void CAStream::set_rms_func()
{
    switch (App->Settings.Audio.GetBytesPerSample())
    {
        case 4:
            calc_rms = calc_rms_16;
            set_vol  = set_vol_16;
            break;
        case 8:
            calc_rms = calc_rms_32;
            set_vol = set_vol_32;
            break;
        default:
            throw EInternal(__PRETTY_FUNCTION__);
    }

}

#if 0
void CAStream::RunProfBuild()
{
    GetDetector()->Init();
    File->StartScan();
}
#endif


bool CAStream::BufferBleedsOff() const
{
    return GetSegmentCount() - Dst->GetSegmentCount() <= Buffer->GetCapacity(); // TODO Capacity auf Segmente
}

bool CAStream::IsSyncdSlave () const
{
    bool ret = IsSlave() && Audio->GetSegmentCount() >= Master->FirstBufferSegment();
    return ret;
}


void CAStream::Start()
{
    if (!IsGood())
        return;
    Audio->DoSignals(IsPaused());
    if (!IsRunning())
    {
        File->Continue();
        Audio->Continue();
    }
}

void CAStream::Continue()
{
    if (IsPaused())
    {
        File->Continue();
        Audio->Continue();
    }
}

bool CAStream::CreateFile(QString const & f)
{
    FileName = f;
    return (ToDisk()) ? File->Open(0) : true;
}

bool CAStream::OpenFile(QString const & f)
{
    FileName = f;
    if (ToDisk())
        return true;
    else
    {
        return File->Open(0);
    }
}

void CAStream::DeleteFile()
{
    File->Close();
    if (!QFile::remove(FileName))
        throw EFile("File could not be removed", FileName);
    
}


void CAStream::Pause()
{
    Audio->Pause(true);

}

void CAStream::End()
{
    Audio->Pause(true);
    Audio->Close();


    if (ToDisk())  // allow buffer content to be written to disk
    {
        while (File->GetSegmentCount() != Audio->GetSegmentCount())
        {
            File->Signal();
            CApp::USleep(1000);
        }
    }
    File->Pause(true);
    File->Close();
    free();

}


bool CAStream::Cue(QString const & seconds, bool relative)
{
    double sec = TimeStringToSecs(seconds);
    return  Cue(sec, relative);
}

bool CAStream::Cue(double seconds, bool relative)
{
    int segments =  SecToSegments(seconds);
    bool ret = cue(segments, relative);
    return ret;
}


bool CAStream::IsRunning () const
{
    bool ret;
    if (ToDisk())
        ret = Audio->State.Has(CThread::Working);
    else if (Audio == 0)
        ret = File->State.Has(CThread::Working) &&
              File->GetSegmentCount() <= Master->GetSegmentCount();
    else
        ret = Audio->State.Has(CThread::Working) &&
              Audio->GetSegmentCount() < Master->GetSegmentCount() &&
              File->GetSegmentCount() <= Master->GetSegmentCount();
    return ret;
}


void CAStream::ProcessSegment(bool detector)
{
     if (!Segment)
        return;
    tPSample sample;
    if (detector)
        calc_rms(Segment, sample, GetDetector(), GetNormalizer());
    else
        calc_rms(Segment, sample, 0, 0);

    if (MeterData)
    {
        MeterData->Peak.Left = ushort_to_db(sample.Peak.Left);
        MeterData->Peak.Right = ushort_to_db(sample.Peak.Right);
#if 0        
        MeterData->RMS.Left = MeterData->Peak.Left;  // TODO RMS wird gar nicht anzgeigt
        MeterData->RMS.Right = MeterData->Peak.Right;
#else
        MeterData->RMS.Left = ushort_to_db(sample.RMS.Left);
        MeterData->RMS.Right = ushort_to_db(sample.RMS.Right);
        
#endif
    }
}

double CAStream::ushort_to_db (short in)
{
    double db = 20.0 * log10(double(in) / double (0x8000));
    return db;
}

bool CAStream::IsPaused() const
{
    return Audio->State.Has(CThread::Idle);
}

bool  CAStream::IsAtEnd() const
{
    return ToDisk() || Audio->GetSegmentCount() >= GetSegmentCount();
}

bool CAStream::IsAtBegin() const
{
    return Audio->GetSegmentCount() == 0;
}

int CAStream::BufferReserve() const
{
    int diff;
    if (Good)
    {
        CBuffer * buffer = (!IsSlave() || IsSyncdSlave()) ? Master->Buffer : Buffer;
        diff = Src->GetSegmentCount() - Dst->GetSegmentCount();
        diff *= 100;
        diff /= buffer->GetCapacity();

        if (ToDisk())
            diff = 100 - diff;
    }
    else
        diff = 100;
    return diff;
}

int CAStream::BufferReserve2() const
{
    CBuffer * buffer = (!IsSlave() || IsSyncdSlave()) ? Master->Buffer : Buffer;
    int c = buffer->GetCapacity();
    int d =  Src->GetSegmentCount() - Dst->GetSegmentCount();
    int ret = c - d;
    if (ret < 0)
        ret = 0;
    ret /= buffer->GetSegSize();
    return ret;
}


CBuffer * CAStream::DuplexPbBuffer () const
{
    CBuffer * ret;
    if (IsSyncdSlave())
        ret = GetMaster()->GetBuffer();
    else
        ret = GetBuffer();
    return ret;     
}

double  CAStream::GetLength() const
{
    return SegmentsToSec(GetSegmentCount());
}

double CAStream::Length(QString const & file_name)
{
    double ret = 0;
    CAStream as(false, false, false);
    try
    {
        as.OpenFile(file_name);
        ret = as.GetLength();
    }
    catch (Except &e) {}
    return ret;
}


int CAStream::GetPlayedSample() const
{
    return ToDisk() ? 0 : Audio->GetSegmentCount(); 
}

int CAStream::GetAudioSampleCount() const
{
    return Audio->GetSegmentCount();
}

QString CAStream::WavInfo() const
{
    QString length;
    SecsToTimeString(GetLength(), length);
    CSettings::CAudio const & set = App->Settings.Audio;
    QString ret = QString("%1 minutes at %2 Hz / %3 bits").arg(length).
                                                           arg(set.GetRate()).
                                                           arg(set.GetBitsPerSample());
    return ret;
}


void CAStream::GetPos(double & seconds) const
{
    int sample = Audio->GetSegmentCount();
    seconds = SegmentsToSec(sample);
}

void  CAStream::GetLength(QString & time_string) const
{
     SecsToTimeString(GetLength(), time_string);
}

void CAStream::GetPos(QString & time_string) const
{
    int sample = Audio->GetSegmentCount();
    double secs = SegmentsToSec(sample);
    SecsToTimeString(secs, time_string);
}

void CAStream::SecsToTimeString(double secs, QString & time_string, bool tenth)
{
    QChar fill('0');
    int time;
    int isecs, fracs, digits;
    int fac;
    if (tenth)
    {
        digits = 1;
        fac = 10;
    }
    else
    {
        digits = 2;
        fac = 100;
    }

    time = int (secs * fac + 0.5);
    isecs = time / fac;
    QTime t =  QTime(0, 0).addSecs(isecs);
    fracs = time - isecs * fac;
    time_string = QString("%1:%2.%3").arg(t.minute(), 2, 10, fill).
                                      arg(t.second(), 2, 10, fill).
                                      arg(fracs, digits, 10, fill);
}

void CAStream::SegsToTimeString(unsigned segments, QString & time_string, bool tenth)
{
    SecsToTimeString(SegmentsToSec(segments), time_string, tenth);
}

double CAStream::TimeStringToSecs(QString const & input)
{
    double min, sec;
    min = input.mid(0, 2).toDouble();
    sec = input.mid(3, -1).toDouble();
    sec += (min * 60);
    return sec;

}


double CAStream::GetPosSec() const
{
    return SegmentsToSec(Audio->GetSegmentCount());
}

int CAStream::FirstBufferSegment() const
{
    return SegmentCount - Buffer->GetCapacity();
}

int CAStream::TimeStringToSegments(QString const & input)
{
    double secs = TimeStringToSecs(input);
    int ret = SecToSegments(secs);
    return ret;
}

int CAStream::SecToSegments(double secs)
{
    double ret;
    ret = double(App->Settings.Audio.GetRate())
         / App->Settings.Audio.GetSamplesPerSegment()
         * secs;
    return qRound(ret);
}

double CAStream::SegmentsToSec(unsigned segments)
{
    double ret;
    if (segments == 0)
        ret = 0;
    else
    {
        ret = ((double) segments)
       * App->Settings.Audio.GetSamplesPerSegment()
       / App->Settings.Audio.GetRate();

    }
    return ret;
}

bool CAStream::SaveProfileData (int side, bool detector, bool normalizer)
{
    bool ret = true;
    if (detector && GetDetector())
        ret = ret && GetDetector()->SaveData(side);
    if (normalizer && GetNormalizer())
        ret = ret && GetNormalizer()->SaveData(side);
    return ret;
}


bool CAStream::cue(int new_pos, bool relative)
{
    DEBUG(DB_GENERAL, 0, __PRETTY_FUNCTION__);
    bool ret = false;
    if (IsGood() && !ToDisk())
    {
        CFile * file = (CFile*) Src;
        bool bleeding_buffer = BufferBleedsOff();
        bool paused = Dst->State.Has(CThread::Idle); // Cueing was requested while pb paused
        Src->Pause(false);
        Dst->Pause(true);
        int old_pos = Dst->GetSegmentCount();
        if (relative)
            new_pos += old_pos;

        int upper_bound =  Master->GetSegmentCount();
        if (new_pos >= 0 && new_pos <= upper_bound)
        {
            int last = Master->Src->GetSegmentCount();
            int first = last - Master->Buffer->GetCapacity();
            bool can_use_master = new_pos >= first && new_pos <= last;

            if (!IsSlave() || !can_use_master)
                file->Seek(new_pos, bleeding_buffer);

            Src->SetSegmentCount(new_pos);
            Dst->SetSegmentCount(new_pos);
            Buffer->Reset(new_pos);
            Audio->DoSignals(paused);
        }
        Src->Continue(false);
        if (!paused)
            Dst->Continue(true);
        ret = true;
    }
    return ret;
}


void CAStream::calc_rms_32 (CBufSeg * seg, tPSample & sample, CDetector * detector,
                            CNormalizer * normalizer)
{
    CSettings::CAudio const & sa = App->Settings.Audio;
    tSample64 sum;

    tSample32 const * current = (tSample32*) seg->Audio.Data;
    tSample32 const * end = current + sa.GetSamplesPerSegment();
    tSample32 copy;

    for (; current != end; ++current)
    {
        seg->Lock();
        copy = *current;
        seg->Unlock();
        copy.Left >>= 16;
        copy.Right >>= 16;
        sum.Left += copy.Left * copy.Left;
        sum.Right += copy.Right * copy.Right;
        short rect_left = abs(copy.Left);
        short rect_right = abs(copy.Right);
        if (normalizer)
            normalizer->CountLevel(max(rect_left, rect_right));

        if (rect_left > sample.Peak.Left) sample.Peak.Left = rect_left;
        if (rect_right > sample.Peak.Right) sample.Peak.Right = rect_right;
    }
    double samp_per_seg = sa.GetSamplesPerSegment();
    sample.RMS.Left = qRound(sqrt(sum.Left / samp_per_seg));
    sample.RMS.Right= qRound(sqrt(sum.Right / samp_per_seg));
    if (detector)
    {
        detector->AddSegment(sample);
    }

}

void CAStream::calc_rms_16 (CBufSeg * seg, tPSample & sample, CDetector * detector,
                            CNormalizer * normalizer)
{
    CSettings::CAudio const & sa = App->Settings.Audio;
    tSample64 sum;
    tSample16 const * current = (tSample16*) seg->Audio.Data;
    tSample16 const * end = current + sa.GetSamplesPerSegment();

    for (; current != end; ++current)
    {
        sum.Left += current->Left * current->Left;
        sum.Right += current->Right * current->Right;
        short rect_left = abs(current->Left);
        short rect_right = abs(current->Right);
        if (normalizer)
            normalizer->CountLevel(max(rect_left, rect_right));
        if (rect_left > sample.Peak.Left) sample.Peak.Left = rect_left;
        if (rect_right > sample.Peak.Right) sample.Peak.Right = rect_right;
    }
    double samp_per_seg = sa.GetSamplesPerSegment();    
    sample.RMS.Left = qRound(sqrt(sum.Left / samp_per_seg));
    sample.RMS.Right= qRound(sqrt(sum.Right / samp_per_seg));
    if (detector)
    {
        detector->AddSegment(sample);
    }

}

CBufSeg * CAStream::ChangeVolume(CBufSeg * in_segment)
{
    CBufSeg * ret;
    if (GetVolume() == 1.0)
        ret = in_segment;
    else
    {
        in_segment->Lock();
        WorkSegment = *in_segment;
        in_segment->Unlock();
        set_vol(&WorkSegment, GetVolume());
        ret = &WorkSegment;

    }
    return ret;
}

void CAStream::set_vol_16(CBufSeg * segment, double const & volume)
{
    tSample16 * sample = (tSample16*) segment->Audio.Data;
    tSample16 * end = sample + App->Settings.Audio.GetSamplesPerSegment();

    int l, r;
    for (; sample != end; ++sample)
    {
        l = qRound(sample->Left * volume);
        r = qRound(sample->Right * volume);
        sample->SetLeft(l);
        sample->SetRight(r);
    }
}

void CAStream::set_vol_32(CBufSeg * segment, double const & volume)
{
    tSample32 * sample = (tSample32*) segment->Audio.Data;
    tSample32 * end = sample + App->Settings.Audio.GetSamplesPerSegment();
    qint64 l, r;
    for (; sample != end; ++sample)
    {
        l = qRound64(sample->Left * volume);
        r = qRound64(sample->Right * volume);
        sample->SetLeft(l);
        sample->SetRight(r);
    }
}

// -------------------------------------------------------------------------------------------

CProfileStream::CProfileStream()
{
    File = new CScanFile(this, App->Settings.Audio.GetBytesPerSegment());
    Detector = new CDetector;
    Normalizer = new CNormalizer;
    Segment = 0;
    Master = this;
    Buffer = new CBuffer;
}

CProfileStream::~CProfileStream()
{

}

void CProfileStream::Start(QString const & file)
{
    CSettings::CAudio const & set = App->Settings.Audio;
    FileName = file;
    File->Open(0);
    if (!Buffer->GetSegCount())
        Buffer->Init(1, set.GetSamplesPerSegment(), set.GetBytesPerSample());
    Buffer->Reset(0);
    GetDetector()->Clear();
    set_rms_func();
    File->State.Add(CThread::Working);
    File->Start();
}

// -------------------------------------------------------------------------------------------

void CEncoderStream::ProcessSegment(bool /* dummy */)
{
    Encoder->Process(Segment);
}

CEncoderStream::~CEncoderStream()
{
    delete Encoder;
}

CEncoderStream::CEncoderStream(int format):
    Encoder(0)
{
    File = new CScanFile(this);

    switch (format)
    {
        case CBaseEnc::ecMp3:   Encoder = new CMp3Enc; break;
#if !defined (CONFIG_STYLUS_NO_VORBISENC)
        case CBaseEnc::ecVorbis:Encoder = new CVorbisEnc; break;
#endif
#if !defined (CONFIG_STYLUS_NO_FLACENC)
        case CBaseEnc::ecFlac:  Encoder = new CFlacEnc; break;
#endif
    }
    Master = this;
    Buffer = new CBuffer;
}

void CEncoderStream::Start(CBaseEnc::tInfo & info, bool local, bool remote)
{
    CSettings::CAudio const & set = App->Settings.Audio;
    ENSURE(Encoder);
    Encoder->Init(info, local, remote);
    CScanFile* f = dynamic_cast<CScanFile*>(File);
    f->SetReadCount(Encoder->GetFrameSize() * set.GetBytesPerSample());

    FileName = info.In.File;
    Master = this;
    File->Open(0);
    File->Seek(info.In.Start, false);
    File->SetSegmentCount(0);
    SetSegmentCount(info.In.Length);
    Buffer = new CBuffer;
    Buffer->Init(1, Encoder->GetFrameSize(), set.GetBytesPerSample());
    File->State.Add(CThread::Working);
    File->Start();
}

bool CEncoderStream::IsAtEnd() const
{
    return File->EoF() || File->GetSegmentCount() >= GetSegmentCount();
}


