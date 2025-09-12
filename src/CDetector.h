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
#ifndef CDETECTOR_H
#define CDETECTOR_H
#include "../config.h"


#include "types.h"
#include "CState.h"
#include "CRange.h"

#include <vector>
#include <algorithm>

#include <QString>
#include <QObject>

#include "CTracks.h"


using namespace std;

/**
	@author Harry <harry@actionpussyclub.de>
*/


class CBufSeg;
class CAStream;
class CProfileStream;

class CDetector: public QObject
{
    Q_OBJECT
    public:

        enum  Modes
        {
            Approach,
            Static,
            Average,
            Dynamic
        };

        enum
        {
           TLenDefault = 2,
           PLenDefault = 2
        };
        
        static char const * ModeList[];
        static char const * LengthList[];
        static char const * NoiseList[];

        enum Options
        {
            NoOption        = 0x000,
            SkipNeedlePop   = 0x001, // For recording start: detect needle drop pop
            SkipEndPop      = 0x002, // For recording end: detect real end of music
            OutroToLast     = 0x004, // An outro on record end belongs to last track
            OutroToNew      = 0x008, // An outro be handled as a new track
            DoOutros        = OutroToLast | OutroToNew,
            CutStart        = 0x010, // Cut leading silence from track
            CutEnd          = 0x020  // Cut trailing silence from track
        };
        typedef CState <Options> tOption;
        tOption Option;

        enum States
        {
            NoState     = 0x000,
            CalcStart   = 0x002,  // Real start of music is to be calculated
            GotOutro    = 0x004   // Outro has been detected as the last track
        };
        CState <States> State;
           
        CTracks         Tracks;
        typedef
        vector <CTracks>tTrackSets;
        tTrackSets      TrackSets;
                        CDetector     ();
                       ~CDetector     ();

        CDetector &     operator=     (CDetector const & rhs);                           
        void            Init          ();

        
        void            AddSegment    (tPSample const & sample);


        int             LocateTracks  (Modes mode = Approach,
                                       tOption options = tOption(),
                                       int side_to_check = 0,
                                       int track_len = TLenDefault,
                                       int pause_len = PLenDefault, int lp_noise = 0);

#ifdef CONFIG_STYLUS_DEV_HELPERS
        void            DumpData      ();
#endif        
        bool            SaveData      (int side);
        int             LoadData      (int side);

        void            BuildProfile  ();
        void            Clear         ();

    private:
        typedef vector <tPSample> tProfile;

        static int    const TrackLengths   [];
        static double const SilenceLengths [];

        struct tAvgLine
        {
            double Sum;
            int    Count;     // Number of probed samples
            int    Samples;   // Number of valid samples
            int    MinCount;
            int    Val;
            int    LastVal;
                   tAvgLine  (): Sum(0), Count(0), MinCount(0), Val(0), LastVal(0) {}
            void   Add       (unsigned value);
            bool   IsUsable  ()  const  { return Count >= MinCount; }
            void   Clear     ()         { Count = MinCount = Val = LastVal = Samples = 0; Sum = 0; }
        };


        struct tData
        {
            tProfile        Profile;
            tAvgLine        AvgLine;
            unsigned        Thres;
            CRange<int>     Music;
            int             DroppedPop; // Stylus drop pop removed by ... segments.
            struct
            {
                int Count;
                int Relevance;
            } Estimated;
        } Data;

        Modes       Mode;
        CRange<int> TrackRule;
        CRange<int> PauseRule;
        int         PauseLen;

    public:
        tData const&    GetData         ()          const { return Data;     }

    private:
        CProfileStream *ProfileStream;
    private slots:
        void            build_finish();
        void            build_work  (int progress);
    private:
        void            calc_avg_rms(unsigned count);
        /*
            1. Calculates the arithmetic rms average of count elements (including dst) and
               stores the result in the AvgRMS member of the last element of the vector
            2. Calculates the average line, its value is stored in dst.AvgLine
        */

        void            calc_thres  (unsigned sample_count, unsigned step);
        /*
                                             
        */
        void            locate_tracks ();

        int             set_mu_start();
        /*
            Scans Data.Profile for the beginning of the recorded audio. This is the position
            where the stylus is set on the record. Data.Music.First will then hold the
            first segment of the real music stream. Data.Music.DroppedPop holds the number
            of the skipped segments.
            If TD's settings have the SkipNeedlePop flag set, the pop when the needle is dropped
            will be tried to be cut out.
            Returns Data.Music.First
        */

        int             set_mu_end  ();
        /*
            Scans Data.Profile for the end of the music. This may be the end of the record.
            If a "pure" record player without end detection and shutdown is used, this
            function will skip the "endless" plops generated from the sylus backdrops.
            Sets and returns Data.Music.Last
        */

        int             next_silence_begin (int start_pos);
        int             next_silence_end   (int start_pos,
                                            int & min_pos, int & min_val);
        QString         file_name          (int side) const;


};

#endif
