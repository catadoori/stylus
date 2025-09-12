/****************************************************************************
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

#include "CAudio.h"
#include "Excepts.h"

#include "CApp.h"
#include "CAStream.h"
#include "CSettings.h"
#include "CBuffer.h"
#include "CFile.h"
#include "CDetector.h"
#include "CBufSeg.h"

#include "../debug.h"
#include <QDebug>


extern CApp * App;



CAudio::CAudio(CAStream * parent, bool write_buffer):
    CThread(parent, write_buffer),
    Device(0),
    Handle(0),
    HWParams(0),
    SWParams(0),
    _SignalSegment(1),
    _Between(false)
{
 
    LoopFunc = write_buffer ? (tLoopFunc) &CAudio::write_buff : (tLoopFunc) &CAudio::read_buff;
}


CAudio::~CAudio()
{
    Close(); 
}

void CAudio::check_snd(int result, Except::Contexts con, char const * explanation)
{
    if (result < 0)
        throw Except(con, QString().fromLocal8Bit(snd_strerror(result)), explanation);
}



bool CAudio::Open(char const * device)
{
#if 0
    {
        static char *device = "default";            /* playback device */
        unsigned char buffer[16*1024];              /* some random data */

        int err;
        unsigned int i;
        snd_pcm_t *handle;
        snd_pcm_sframes_t frames;

        for (i = 0; i < sizeof(buffer); i++)
            buffer[i] = random() & 0xff;

        if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
            printf("Playback open error: %s\n", snd_strerror(err));
            exit(EXIT_FAILURE);
        }
        if ((err = snd_pcm_set_params(handle,
                                      SND_PCM_FORMAT_U8,
                                      SND_PCM_ACCESS_RW_INTERLEAVED,
                                      1,
                                      48000,
                                      1,
                                      500000)) < 0) {   /* 0.5sec */
            printf("Playback open error: %s\n", snd_strerror(err));
            exit(EXIT_FAILURE);
        }

        for (i = 0; i < 16; i++) {
            frames = snd_pcm_writei(handle, buffer, sizeof(buffer));
            if (frames < 0)
                frames = snd_pcm_recover(handle, frames, 0);
            if (frames < 0) {
                printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
                break;
            }
            if (frames > 0 && frames < (long)sizeof(buffer))
                printf("Short write (expected %li, wrote %li)\n", (long)sizeof(buffer), frames);
        }

        /* pass the remaining samples, otherwise they're dropped in close */
        err = snd_pcm_drain(handle);
        if (err < 0)
            printf("snd_pcm_drain failed: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
    }
#endif
    int err;
    if (device)
        Device = device;
    if (!Device)
        Device = "default";
    snd_pcm_stream_t stream = Parent->ToDisk() ?  SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK;
    err = snd_pcm_open(&Handle, Device, stream, 0);
    check_snd(err, Except::Audio, "Could not open pcm device");

    set_hw_par();


    err = snd_pcm_prepare(Handle);
    check_snd(err, Except::Audio, "could not prepare");
    set_sw_par();

    if (0)
    {
        unsigned char buffer[32*1024];              /* some random data */
        unsigned int i;
        for (i = 0; i < sizeof(buffer); i++)
            buffer[i] = random() & 0xff;

        snd_pcm_sframes_t frames;
        for (i = 0; i < 4; i++) {
            frames = snd_pcm_writei(Handle, buffer, 512);
            if (frames < 0)
                frames = snd_pcm_recover(Handle, frames, 0);
            if (frames < 0) {
                printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
                break;
            }
            if (frames > 0 && frames < (long)sizeof(buffer))
                printf("Short write (expected %li, wrote %li)\n", (long)sizeof(buffer), frames);
        }
    }
    return true;
}

bool CAudio::Close()
{
    if (Handle)
    {
        if (snd_pcm_close(Handle))
            throw EInternal(__PRETTY_FUNCTION__);
        Handle = 0;
    }

    if (SWParams)
    {
        snd_pcm_sw_params_free(SWParams);
        SWParams = 0;
    }

    if (HWParams)
    {
        snd_pcm_hw_params_free(HWParams);
        HWParams = 0;
    }
    return true;

}


void CAudio::set_hw_par()
{

    CSettings::CAudio & set = App->Settings.Audio;
    unsigned  frag_size = set.GetSamplesPerSegment();
    int err;
    unsigned int val;
    snd_pcm_uframes_t period_size;
    snd_pcm_hw_params_malloc(&HWParams);

    err = snd_pcm_hw_params_any(Handle, HWParams);
    check_snd(err, Except::Audio, "Could not get predefines");

    err = snd_pcm_hw_params_set_access(Handle, HWParams, set.GetAccess());
    check_snd(err, Except::Audio, "Could not set access mode mode");

    err = snd_pcm_hw_params_set_format(Handle, HWParams, set.GetFormat());
    check_snd(err, Except::Audio, "Could not set format");

    err = snd_pcm_hw_params_set_subformat(Handle, HWParams, SND_PCM_SUBFORMAT_STD);
    check_snd(err, Except::Audio, "Could not set subformat");

    val = set.GetRate();

    err = snd_pcm_hw_params_set_rate_near(Handle, HWParams, &val, 0);
    check_snd(err, Except::Audio, "could not set rate");

    err = snd_pcm_hw_params_set_channels(Handle, HWParams, 2);
    check_snd(err, Except::Audio, "could not set channels");

    period_size = frag_size;

    err = snd_pcm_hw_params_set_period_size_near(Handle, HWParams, &period_size, 0);
    check_snd(err, Except::Audio, snd_strerror(err));

    val = 12;

    err = snd_pcm_hw_params_set_periods_near(Handle, HWParams, &val, 0);
    check_snd(err, Except::Audio, "Could not set periods");

    err = snd_pcm_hw_params(Handle, HWParams);
    check_snd(err, Except::Audio, "could not set hw params");

    snd_pcm_uframes_t uframes;
    err = snd_pcm_hw_params_get_buffer_size(HWParams, &uframes);
    check_snd(err, Except::Audio, "could not get buffer size");

    err = snd_pcm_hw_params_get_period_size(HWParams, &uframes, NULL);
    check_snd(err, Except::Audio, "could not get period size");
    if (uframes != frag_size)
        qWarning() << __PRETTY_FUNCTION__ << ": period size changed from " << frag_size <<
                                             " to " << uframes;
    set.SetSamplesPerSegment(uframes);

    set.SetCanPause(snd_pcm_hw_params_can_pause(HWParams));
    set.SetCanPause(false);

}


void CAudio::set_sw_par()
{
    CSettings::CAudio & sa = App->Settings.Audio;
    int err;
    snd_pcm_uframes_t uframes;
    snd_pcm_sw_params_malloc(&SWParams);

    err = snd_pcm_sw_params_current(Handle, SWParams);
    check_snd(err, Except::Audio, "could not get sw params");

    err = snd_pcm_sw_params_get_boundary(SWParams, &uframes);
    check_snd(err, Except::Audio, "could not get boundary");

    /* start playing when one period has been written */
    err = snd_pcm_sw_params_set_start_threshold(Handle, SWParams, sa.GetSamplesPerSegment());
    check_snd(err, Except::Audio, "could not set start threshold");

    /* disable underrun reporting */
#if 0
    err = snd_pcm_sw_params_set_stop_threshold(Handle, SWParams, uframes);
    check_snd(err, Except::AudioInit);
#endif

    uframes = 0;
    /* play silence when there is an underrun */
    err = snd_pcm_sw_params_set_silence_size(Handle, SWParams, uframes);
    check_snd(err, Except::Audio, "could not set silence size");

    err = snd_pcm_sw_params(Handle, SWParams);
    check_snd(err, Except::Audio, "could not set sw params");
}


void CAudio::write_buff()
{
    CSettings::CAudio & sa = App->Settings.Audio;
    while (!State.Has(Ending))
    {
        if (State.Has(Working))
        {
            State.Remove(Idle);
            if (Parent->Callback)
                Parent->Callback(Parent);

            CBufSeg * seg = Parent->GetBuffer()->GetNewSegment();
            int actual = snd_pcm_readi(Handle, seg->Audio.Data, sa.GetSamplesPerSegment());

            IncSegmentCount();
            Parent->SetWorkSegment(seg);          // actual work is done by file thread
            Parent->SetSegmentCount(GetSegmentCount());
            handle_xrun(actual, sa.GetSamplesPerSegment(), true);

            Parent->GetFile()->Signal();
        }
        else
            idle();
    }
}


void CAudio::read_buff() // and write to pcm
{

    CSettings::CAudio const & sa = App->Settings.Audio;
    CBufSeg * out_seg;
    int err;
    while (! State.Has(Ending))
    {
        if (State.Has(Working))
        {
            State.Remove(Idle);
            DoSignals(false);
            if (Parent->Callback)
                Parent->Callback(Parent);
            int rhs_count = Parent->GetMaster()->GetFile()->GetSegmentCount();
            if (Handle && rhs_count && GetSegmentCount() < rhs_count)
            {
                CBuffer * buffer = Parent->DuplexPbBuffer();
                CBufSeg * in_seg = buffer->GetSegment(GetSegmentCount());
                out_seg = Parent->ChangeVolume(in_seg);
                err = snd_pcm_writei(Handle, out_seg->Audio.Data, sa.GetSamplesPerSegment());
                if (Parent->IsSyncdSlave() && err < 0)
                    msleep(3);
                handle_xrun(err, sa.GetSamplesPerSegment(), false);
                IncSegmentCount();
                if (Parent->IsSyncdSlave())
                    Parent->GetFile()->SetSegmentCount(Parent->GetMaster()->GetSegmentCount());
                Parent->GetFile()->Signal();
            }
            else
#if 0
                CApp::USleep(3000);
#else
                msleep(3);
#endif
        }
        else
           idle();
    }
}

void CAudio::Pause(bool wait)
{
    CThread::Pause(wait);
    Close();
}

void CAudio::Continue(bool wait)
{
    if (!Handle)
       Open();
    CThread::Continue(wait);
}

void CAudio::DoSignals(bool was_paused)
{
    if (!Parent->GetSegmentCount())
        return;
    if (GetSegmentCount() == 0)
    {
        if (_SignalSegment != GetSegmentCount())
        {
            emit AtBegin(was_paused);
           _SignalSegment = GetSegmentCount();
           _Between = false;
        }
    }
    else if (GetSegmentCount() > Parent->GetActiveStream()->GetSegmentCount())
    {
        if (_SignalSegment != GetSegmentCount())
        {
            emit AtEnd(was_paused);
            _SignalSegment = GetSegmentCount();
            _Between = false;
        }
    }
    else if (!_Between)
    {
        emit Between(was_paused);
        _Between = true;
        _SignalSegment = 1;
    }
}

void CAudio::handle_xrun (int actual, int target, bool record)
{
    if (actual < 0)
    {
        if (record && actual == -EAGAIN)
        {
            emit XRun(Parent->GetMaster()->GetAudioSampleCount(), record);
            return;
        }
        if (actual == -EPIPE)
        {
            actual = snd_pcm_prepare(Handle);
            emit XRun(Parent->GetMaster()->GetAudioSampleCount(), record);
            if (actual < 0)
            {
                actual = snd_pcm_prepare(Handle);
                check_snd(actual, Except::Audio, "cant recover from underrun, prepare failed");
            }

        }
        else if (actual == -ESTRPIPE)
        {
            while ((actual = snd_pcm_resume(Handle)) == -EAGAIN)
                CApp::USleep(1000);
            if (actual < 0)
            {
                actual = snd_pcm_prepare(Handle);
                check_snd(actual, Except::Audio, "cant recover from suspend");
            }
        }
    }
    else if (actual != target)
        emit XRun(Parent->GetMaster()->GetAudioSampleCount(), record);
}


