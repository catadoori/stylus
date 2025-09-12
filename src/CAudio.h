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
#ifndef CAUDIO_H
#define CAUDIO_H

/**
	@author Harry <harry@actionpussyclub.de>
*/

#include "CThread.h"
#include "Excepts.h"
#include <alsa/asoundlib.h>

class CAStream;
class CBuffer;

class CAudio: public CThread
{
    Q_OBJECT

    signals:

        void        AtBegin     (bool was_paused);
        void        AtEnd       (bool was_paused);
        void        Between     (bool was_paused);
        void        XRun        (int sample, bool record);

    public:
                    CAudio      (CAStream * parent, bool write_to_buff);
                   ~CAudio      ();

        bool        Open        (char const * device_name = 0);
        void        SetDevice   (char const * device_name = 0) { Device = device_name;  }
        bool        Close       ();
        void        Test        (char * data, int s);
        void        DoSignals   (bool stream_is_paused);
        void        Pause       (bool wait = false);
        void        Continue    (bool wait = false);
    private:
        // for opening the sound device:
        void        open        (char const * device_name);
        void        check_snd   (int result, Except::Contexts con, char const * explanation =0);
        void        handle_xrun (int actual_result, int target_sample_count, bool record);

        void        set_hw_par  ();  
        void        set_sw_par  ();

        void        read_buff   ();
        void        write_buff  ();


        char const *         Device;
        snd_pcm_t*           Handle;
        snd_pcm_hw_params_t* HWParams;
        snd_pcm_sw_params_t* SWParams;
        int                 _SignalSegment;
        bool                _Between;

};

#endif
