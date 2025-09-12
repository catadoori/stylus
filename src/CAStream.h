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
#ifndef CASTREAM_H
#define CASTREAM_H
#include <QString>
#include <vector>
#include <types.h>
#include "CBufSeg.h"

#include "CBaseEnc.h"

/** 
	@author Harry <harry@actionpussyclub.de>
*/

class CAudio;
class CFile;
class CBuffer; 
class CThread;
class CDetector;
class CNormalizer;

class CAStream
{
    public:
                        CAStream            ();
                        CAStream            (bool record, bool create_thds, bool with_audio);
        virtual        ~CAStream            ();

        bool            ToDisk              () const          { return _ToDisk;     }

        void            Init                (QString const & file, int buffer_size,
                                             CAStream * master, bool to_disk,
                                             tMeterData * = 0);
        /*
            Initializes the stream.
            Play: Take audio settings from file, if App.Settings.Audio was not set
            Record: Take audio settings always from App.Settings.Audio.
        */

        void            InitDetector        ();

        void            Reset               (bool to_disk);
        void            RecoverFromXRun     ();

        void            Start               ();
        void            Pause               ();
        void            Continue            ();
        void            End                 ();  // Ends both running or paused threads.

        bool            Cue                 (QString const & seconds, bool relative);
        bool            Cue                 (double seconds, bool relative);
        /*
            Tries to change the current play position of the stream. if relative == true,
            the new position will be relative to the current one.
            Returns the success of operation and always false on recording streams. 
        */
        bool            Cue                 (int pos)           { return cue(pos, false);}

        bool            IsRunning           () const; // Is at least one thread running?
        bool            IsPaused            () const;
        bool            IsAtEnd             () const;
        bool            IsAtBegin           () const;
        bool            IsGood              () const            { return Good;}

        static
        double const    MaxSideSecs;  
        static double   Length              (QString const & file_name);
        double          GetLength           () const;
        int             GetLengthInt        () const  { return int(GetLength() + 0.5);  }
        void            GetLength           (QString & out) const;

        int             GetPlayedSample     () const;
        // for play back: returns the current position

        int             GetAudioSampleCount () const;


        void            GetPos              (double & seconds)  const;
        void            GetPos              (QString & seconds) const;
        double          GetPosSec           () const;
        int             GetPosIntSec        () const  { return int (GetPosSec() + 0.5);  }

          // return current position relative to stream start in seconds

        int             BufferReserve       () const;
          // returns remaining capacity reserve of the underlying buffer.
          // both on play and record 100 means the buffer could drop off
          // a lot and 0 means over- / underruns are likely to occur.

        int             BufferReserve2      () const;
          // returns the number of preceeding segments (from the current play position
          // available in Buffer.

        int             GetSegmentCount     () const          { return SegmentCount; }
        void            SetSegmentCount     (int s)           { SegmentCount = s;    }
          // SegmentCount stores the number of segments available for the stream.
          // For playing, this is the number stored in the audio file
          // For record, this is an volatile value.


        int             FirstBufferSegment  () const;
          // returns the offset of the first sample of the first segment of the
          // Buffer. (BufferStartSample() + Buffer->GetCapacity() == GetSampleCount())

        bool            BufferBleedsOff     () const;
          // returns weather the stream is currently playing back from the buffer only.
          // this happens when the end of the stream is reached. The file thread is may
          // already have died and audio is fed from the remaining buffer content.

        bool            IsSlave             () const          { return this != Master;}
        bool            IsSyncdSlave        () const;
          // returns true if the stream is a slave (has a master) that currently
          // gets its audio data from the master's buffer

        virtual void    ProcessSegment      (bool to_detector);
          // Called by a CFile object.
          // 1. Calculates RMS and Peak values for the current audio segment.
          // 2. Caluclates the VU meter power as a RMS of the current and some
          //    previous peak values


        QString const & GetFileName         () const          { return FileName;    }
        void            SetFileName         (QString const &f){ FileName = f;       }


        QString         WavInfo             () const;

        static double   TimeStringToSecs    (QString const & input);
        static int      TimeStringToSegments(QString const & input);

        static void     SecsToTimeString    (double seconds, QString & output, bool tenth = false);
        static void     SegsToTimeString    (unsigned segments, QString & output, bool tenth = false); 
        static int      SecToSegments       (double seconds);
        static double   SegmentsToSec       (unsigned segments);

        CBufSeg *       ChangeVolume        (CBufSeg * input_segment);
        void            SetVolume           (double v)        { Volume = v; }
        double          GetVolume           () const          { return Volume; }


        // Helpers for file handling:
        bool            CreateFile          (QString const & file_on_disk); 
        bool            OpenFile            (QString const & file_on_dikt);
        void            DeleteFile          ();

        void            RunProfBuild        ();
        bool            SaveProfileData     (int side, bool detector_data, bool normalizer_data);

        CFile *         GetFile             ()  const  { return File;           }
        CAudio *        GetAudio            ()  const  { return Audio;          }
        CBuffer *       GetBuffer           ()  const  { return Buffer;         }
        CDetector *     GetDetector         ()  const  { return Detector;       }
        CNormalizer *   GetNormalizer       ()  const  { return Normalizer;     } 
        CAStream *      GetMaster           ()  const  { return Master;         }

        CAStream *      GetActiveStream     ()  const  { return ActiveStream;   }
        void            SetActiveStream     (CAStream * s) { ActiveStream = s;  }


        typedef void (*tCallback) (CAStream const * stream);
        tCallback       Callback;
        void            SetCallback         (tCallback cb) { Callback = cb; }
        void            SetWorkSegment      (CBufSeg  * segment)  { Segment = segment; }
        CBuffer *       DuplexPbBuffer      ()  const;
        /*
            returns the appropriate Buffer to access for data fetch. 
            For master streams always the own buffer is returned whereas
            for slave streams Master's buffer is returned if audio data can be
            fetched from it. Ohterwise the own buffer is returned.
        */


    protected:

        CFile          *File;
        CAudio         *Audio;
        CBuffer        *Buffer;
        CAStream       *Master;
        CDetector      *Detector;
        CNormalizer    *Normalizer;
        CBufSeg *       Segment;
        CBufSeg         WorkSegment;
        CAStream *      ActiveStream;

        bool            Good;
        bool            _ToDisk;
        int             SegmentCount;
        QString         FileName;

        CThread *       Dst;
        CThread *       Src;
        int             XRuns;
        double          Volume;
        tMeterData *    MeterData;

        void            free                ();
        bool            cue                 (int samples, bool relative);
        void            set_rms_func        ();
        void           (*calc_rms)          (CBufSeg * in, tPSample & out, CDetector *,
                                             CNormalizer *);
        static void     calc_rms_16         (CBufSeg * in, tPSample & out, CDetector *,
                                             CNormalizer *);
        static void     calc_rms_32         (CBufSeg * in, tPSample & out, CDetector *,
                                             CNormalizer *);
        /*
            calc_rms calls the appropiate function, either calc_rms_16 or calc_rms32.
            A call the calc_rms calculates and sets the VU and Peak members of segment->Profile
        */

               void     (*set_vol)          (CBufSeg * segment, double const & volume);
        static void     set_vol_16          (CBufSeg * segment, double const & volume);
        static void     set_vol_32          (CBufSeg * segment, double const & volume);

        static double   ushort_to_db        (short unsigned_input);
        /*
            Calculates the db - representation (loss) of unsingned_input.
            Conversion is for example:
                input     ret        meaning
                0x8000      0        0 db
                0x4000     -6       -6 db
        */
};

class CProfileStream: public CAStream
{
    public:
                CProfileStream  ();
               ~CProfileStream  ();

        void    Start           (QString const & filename);
};

class CEncoderStream: public CAStream
{
    public:
                 CEncoderStream  (int format);
                ~CEncoderStream  ();

        void        Start           (CBaseEnc::tInfo & info, bool local, bool remote);
        void        ProcessSegment  (bool dummy);
        bool        IsAtEnd         ()    const;
        CBaseEnc*   GetEncoder      ()   { return Encoder; }

    private:
        CBaseEnc * Encoder;

};

#endif
