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

#include "CDetector.h"
#include "../debug.h"
#include "CApp.h"
#include "CGui.h"
#include "CPTool.h"
#include "CDetTool.h"
#include "CSettings.h"
#include "CAStream.h"
#include "CAudio.h"
#include "CFile.h"
#include "CBufSeg.h"
#include <QObject>
// #include <math.h>

#include "CTEdit.h"

#include <QDebug>

#include <iomanip>

#include <iostream> // FIXME: nur für Dump()
#include <QFile>


extern CApp * App;


char const *  CDetector::ModeList[] =
{
    "Approach",
    "Static",
    "Average",
    "Dynamic",
    NULL
};

char const * CDetector::LengthList[] =
{
    "Very short",
    "Short",
    "Medium",
    "Long",
    "Very long",
    NULL
};

char const * CDetector::NoiseList[] =
{
    "Low",
    "Mid",
    "High",
    NULL    
};

int const CDetector::TrackLengths[] = { 30, 60, 120, 180, 210 };
double const CDetector::SilenceLengths[] = { 0.2, 0.5, 1, 2, 5 };



void CDetector::tAvgLine::Add(unsigned val)
{
    if (val > App->Settings.GetVinylNoise())
    {
        if (++Count >= MinCount)
        {
            Sum += val;
            Val = unsigned(Sum / ++Samples);
        }
    }
    
}

CDetector::CDetector():
    ProfileStream(0)
{
    Data.Thres = ~0;
}


CDetector & CDetector::operator = (CDetector const & rhs)
{
    Data = rhs.Data;
    return *this;
}

void CDetector::Clear()
{
    State.Clear();
    Data.Profile.clear();
    Data.AvgLine.Clear();
    Data.AvgLine.MinCount = CAStream::SecToSegments(30);
    Data.Music.Set(~0, ~0);
    Data.DroppedPop = 0;
    Data.Profile.clear();
}


CDetector::~CDetector()
{
}



void CDetector::AddSegment(tPSample const & sample)
{
    Data.Profile.push_back(sample);
    calc_avg_rms(100); // 20
//    calc_thres(20, 1);
}

void CDetector::calc_avg_rms(unsigned el_count)
{
    long long sum = 0;
    // example: Data.Profile.size() == 9, call for calc_avg_rms(3)
    // 0  1  2  3  4  5  6  7  8  
    //    ^        ^        ^   
    //    |        |        |
    //  begin     dst      end

    unsigned width = 2 * el_count;
    tProfile::iterator end(Data.Profile.end() - 1);
    tProfile::iterator begin(end - width);
    if (begin < Data.Profile.begin())
        begin = Data.Profile.begin();

    tProfile::iterator dst = end - el_count;
    if (dst < begin)
        dst = begin;

    int count = 0;
    for (; begin <= end; begin += 5)
    {
        unsigned val = (begin->RMS.Left + begin->RMS.Right) >> 1;
        if (val)
        {
            sum += val;
            ++count;
        }
    }
    if (count)
    {
        dst->AvgRMS = sum / count;
        Data.AvgLine.Add(dst->AvgRMS);
    }
    dst->AvgLine = Data.AvgLine.Val;
}


void CDetector::calc_thres(unsigned width, unsigned step)
{
    CSettings & settings = App->Settings;
    tProfile::iterator dst, curr, thres_start;
    
    // adapt empty AvgLine - values to closest value
    for (dst = Data.Profile.begin(); dst != Data.Profile.end() && dst->AvgLine == 0; ++dst)
       ;
    thres_start = dst;
    (thres_start - 1)->Thres = dst->AvgLine / 4;
    for (curr = Data.Profile.begin(); curr < dst; ++curr)
        curr->AvgLine = dst->AvgLine;
        
    for (dst = Data.Profile.end() - 1; dst != Data.Profile.begin() && dst->AvgLine == 0; --dst)
        ;
    for (curr = Data.Profile.end() - 1; curr > dst; --curr)
        curr->AvgLine = dst->AvgLine;

    if (Mode == Dynamic)
    {
        for (dst = thres_start; dst != Data.Profile.end(); ++dst)
        {
            curr = dst - width;
            unsigned avg = 0;
            while (curr <= dst)
            {
                avg += curr->AvgRMS;
                curr += step;                
            }
            avg /= width / step;
             avg *= 15;
            avg += settings.GetVinylNoise();
            if (avg  < (dst - 1)->Thres )
                dst->Thres = avg;
            else
                dst->Thres = (dst - 1)->Thres;
        }
    }
}


int  CDetector::set_mu_start()
{
    if (Data.Music.GetFirst() == ~0)
    {
        tProfile::iterator curr = Data.Profile.begin();
        int limit = App->Settings.GetVinylNoise();
        for (; curr != Data.Profile.end(); ++curr)
        {
            if (curr->AvgRMS >= limit )
            {
                Data.Music.SetFirst(distance(Data.Profile.begin(), curr));
                break;
            }
        }
    }
   
    if (Option & SkipNeedlePop)
    {
        unsigned needed = CAStream::SecToSegments(2.0) + Data.Music.GetFirst();
        if (Data.Profile.size() > needed)
        {
            tProfile::iterator start = Data.Profile.begin() + Data.Music.GetFirst();
            tProfile::iterator peak = start + CAStream::SecToSegments(0.30);
            
            if (peak->AvgRMS > start->AvgRMS * 3)
            {
                tProfile::iterator end = peak +  CAStream::SecToSegments(0.40);
                while (peak->AvgRMS > start->AvgRMS && peak < end)
                    ++peak;
                
                Data.DroppedPop = distance(start, peak);
                Data.Music.SetFirst(distance(Data.Profile.begin(), peak));
            }
            State.Remove(CalcStart);
        }
    }
    return Data.Music.GetFirst();
}

int CDetector::set_mu_end()
{
    int ret;
    ret = Data.Profile.size();
    if (Option & SkipEndPop)
    {
        tProfile::reverse_iterator iter = Data.Profile.rbegin();
        for (; iter != Data.Profile.rend(); ++iter)
        {
            if (iter->AvgRMS > iter->Thres) break;
            --ret;
        }
    }
    Data.Music.SetLast(ret);
    return ret;
}



int CDetector::LocateTracks(Modes mode, tOption option, int side,
                            int track_len, int pause_len, int /* lp_noise */ )
{
    Mode = mode;
    Option = option;
    TrackSets.clear();
    PauseLen = pause_len;
    TrackRule.SetFirst(CAStream::SecToSegments(TrackLengths[track_len]));
    PauseRule.SetFirst(CAStream::SecToSegments(SilenceLengths[pause_len]));
    std::map <int, int> track_relevance;
    int scans = 0;
    if (Mode == Approach)
    {
        int thres = App->Settings.GetVinylNoise();
        for(; thres < Data.AvgLine.Val / 2; thres += 100)
        {
            ++scans;
            for (tProfile::iterator iter = Data.Profile.begin(); iter != Data.Profile.end(); ++iter)
                iter->Thres = thres;
            locate_tracks();
            if (App->Gui()->TEdit()->TrackCount(side) == Tracks.Count())
            {
                if (find (TrackSets.begin(), TrackSets.end(), Tracks) == TrackSets.end())
                {
                    TrackSets.push_back(Tracks);
                }
            }
            else
            {                
            }
            std::map<int, int>::iterator iter;
            iter = track_relevance.find(Tracks.Count());
            if (iter == track_relevance.end())
                track_relevance.insert(pair<int, int>(Tracks.Count(), 1));
            else
                ++ iter->second;
//break;
        }
    }
    std::map<int, int>::iterator iter, best;
    for (iter = best = track_relevance.begin(); iter != track_relevance.end(); ++iter)
    {
        if (iter->second > best->second)
            best = iter;
    }
    if (best != track_relevance.end())
    {
        Data.Estimated.Count = best->first;
        Data.Estimated.Relevance = best->second * 100 / scans;
    }
    return TrackSets.size();
}

void CDetector::locate_tracks()
{
    Tracks.Clear();

    int needle_drop_offs = 0;
    int silence_end = 0;
    for (; silence_end != -1; )
    {
        int curr = Tracks.GetScanStart();
        int track_start = Option & CutStart ? curr : Tracks.GetTrackStart();
        int min_pos, min_val;
        int data_end = Data.Profile.size();
        for(;;)
        {
            int track_end = next_silence_begin(curr);
            silence_end = next_silence_end(track_end, min_pos, min_val);
#if 0
            double ts, te, se, mp; ts = CAStream::SegmentsToSec(track_start); 
            te = CAStream::SegmentsToSec(track_end);
            se = CAStream::SegmentsToSec(silence_end);
            mp = CAStream::SegmentsToSec(min_pos);
#endif            
            if (silence_end == -1)
               min_pos = track_end;
            if (needle_drop_offs == 0 && Option & SkipNeedlePop &&  Tracks.Count() == 0  &&
                track_end - track_start > CAStream::SecToSegments(2.5) &&
                silence_end - track_end >  PauseLen ? PauseRule.GetFirst() : 0 )
            {
                needle_drop_offs = silence_end;
            }
            else if (track_end - track_start - needle_drop_offs > TrackRule.GetFirst() &&  // a regular track: long enough 
                    (silence_end - track_end > PauseRule.GetFirst() ||  // with either pause following
                     silence_end == -1))                                // or last track
            {
                Tracks.Insert(needle_drop_offs ? needle_drop_offs : track_start,
                              Option & CutEnd ? track_end : min_pos, silence_end);
                needle_drop_offs = 0;
                break;
            }
            else
            {
                if (data_end - track_start < TrackRule.GetFirst() && // last track
                    Option & DoOutros)
                {
                    data_end = set_mu_end();
                    track_end = min_pos = silence_end = data_end;
                    if (Option & OutroToLast)
                        Tracks.UpdateLast(min_pos, silence_end);
                    else 
                        Tracks.Insert(track_start, min_pos, silence_end);

                    silence_end = -1;
                    break;
                }
            }
            curr = silence_end;
            if (curr == -1 ) break;
        }
    }

}

int CDetector::next_silence_begin(int start_pos)
{
    int ret = Data.Profile.size() ;
    tProfile::iterator iter = Data.Profile.begin() + start_pos;
    for (; iter != Data.Profile.end(); ++iter)
    {
        if (iter->AvgRMS < iter->Thres)
        {
            ret = distance(Data.Profile.begin(), iter);
            break;
        }
    }
    return ret;
}

int CDetector::next_silence_end(int start_pos, int & min_pos, int & min_val)
{
    int ret = -1;
    tProfile::iterator iter = Data.Profile.begin() + start_pos;
    tProfile::iterator min = iter;
    for (; iter != Data.Profile.end(); ++iter)
    {
        if (iter->AvgRMS < min->AvgRMS)
            min = iter;
        if (iter->AvgRMS > iter->Thres)
        {
            ret = distance(Data.Profile.begin(), iter);
            min_pos = distance(Data.Profile.begin(), min);
            min_val = min->AvgRMS;
            break;
        }
    }
    return ret;
}

#ifdef CONFIG_STYLUS_DEV_HELPERS
void CDetector::DumpData()
{
    if (!Data.Profile.size()) return;
    char const sep = '\t';
    QString f_name = App->Settings.Project.GetPath() + "data.csv";
    ofstream file(f_name.toLatin1());
    int segments = 0;
    file << "Time\tAvgRms\tAvgLine\tThres\tRms\tLPeak\tRPeak\n";
    for (tProfile::const_iterator iter = Data.Profile.begin();
         iter != Data.Profile.end()-1; ++iter, ++segments)
    {
#if 1
        file << setw(9) << fixed << setprecision(3)<< CAStream::SegmentsToSec(segments) << sep <<
                setw(6) << iter->AvgRMS  << sep <<                                 // 2
                setw(6) << iter->AvgLine << sep << 
//                iter->Peak.Left << sep <<                               // 3
//                iter->Peak.Right << sep <<                              // 4
//                iter->RMS.Left << sep <<                                // 5
//                iter->RMS.Right << sep <<                               // 6
                setw(6) << iter->Thres  << sep <<                
                setw(6) << ((iter->RMS.Left + iter->RMS.Right) >> 1) <<
                setw(6) << iter->Peak.Left << sep <<
                setw(6) << iter->Peak.Right << sep <<
                 '\n';
#else
        file << ((iter->RMS.Left + iter->RMS.Right) >> 1) << '\n';
#endif
    }
#if 0    
    qDebug() << __PRETTY_FUNCTION__;
    std::cout << "Some profile info:"
                 "\n  Data.AvgLine.Val           = " << Data.AvgLine.Val <<
                 "\n  Data.Music.First (Seg) = " << Data.Music.GetFirst() <<
                 "\n  Data.Music.First (Sec  = " << CAStream::SegmentsToSec(Data.Music.GetFirst()) <<
                 "\n  Data.DroppedPop (Seg)  = " << Data.DroppedPop <<
                 "\n  Data.DroppedPop (Sec)  = " << CAStream::SegmentsToSec(Data.DroppedPop) <<
                  
                 std::endl;
#endif

#if 0
    file.close();
    file.open("levels.csv");
    file << "Level\tCount\n";
    for (int x = 0; x < SHRT_MAX; ++x)
    {
        file << setw(7) << x << sep << setw(8) << Data.Levels[x] << "\n";
    }
#endif    
}
#endif

bool CDetector::SaveData(int side)
{
#ifdef CONFIG_STYLUS_DEV_HELPERS    
    DumpData();
#endif    
    QFile file(file_name(side));
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
        return false;
    QDataStream out (&file);
    out << Data.AvgLine.Val << Data.Music.GetFirst() << Data.DroppedPop;
    for (tProfile::const_iterator iter = Data.Profile.begin();
         iter != Data.Profile.end(); ++iter)
    {
        out.writeRawData((char*) & (*iter), sizeof(tPSample));
    }
    file.close();
    return file.error() == 0;
}

int CDetector::LoadData(int side)
{
    Clear();

    QFile file(file_name(side));
    if (!file.open(QFile::ReadOnly))
        return -1;
    QDataStream in(&file);
    unsigned music_first;
    in >> Data.AvgLine.Val >> music_first >> Data.DroppedPop;
    Data.Music.SetFirst(music_first);

    Data.Profile.clear();
    while (!in.atEnd())
    {
        tPSample sample;
        in.readRawData((char*) &sample, sizeof(tPSample));
        Data.Profile.push_back(sample);
    }
    return file.error() ? -1 : Data.Profile.size();
}

QString CDetector::file_name(int side) const
{
    return App->Settings.Project.GetPath()  + "profile." +
           CApp::SideToQChar(side);
}



// -----------------------------------------------------------------------------------------

void CDetector::BuildProfile()
{
    if (!ProfileStream)
    {
        ProfileStream = new CProfileStream;
        connect(ProfileStream->GetFile(), SIGNAL(finished()), this, SLOT(build_finish()),
                Qt::QueuedConnection);
        connect(ProfileStream->GetFile(), SIGNAL(Progress(int)), this, SLOT(build_work(int)),
                Qt::QueuedConnection);
    }
    if (ProfileStream->IsRunning())
        ProfileStream->GetFile()->End(true);
    else
    {
        App->Gui()->DetTool()->SetBuildMode(true);
        ProfileStream->Start(App->Gui()->PTool()->GetWavName(App->Gui()->DetTool()->GetSide()));
    }
}

void CDetector::build_finish()
{
   *App->Detector() =  * ProfileStream->GetDetector();

    App->Gui()->DetTool()->Requery();
    App->Gui()->StatusMessage("Saving profile data..");
    if (!ProfileStream->SaveProfileData(App->Gui()->DetTool()->GetSide(), true, true))
        App->Gui()->StatusWarning("Could not save profile data.", 5);
    else
    {
        App->Gui()->StatusMessage("Profile data saved.", 3);
        if (ProfileStream->GetFile()->GetSegmentCount() < ProfileStream->GetSegmentCount())
            App->Gui()->StatusWarning("Profile rebuild was stopped, data is not complete.", 5);
        else
            App->Gui()->setWindowModified(true);
            
    }
    delete ProfileStream;
    ProfileStream = 0;
    App->Gui()->DetTool()->SetBuildMode(false);
}

void CDetector::build_work(int progress)
{
    QString s = QString("Rebuilding profile info: %1%").arg(progress);
    App->Gui()->StatusMessage(s, 1);
}


// -----------------------------------------------------------------------------------------
