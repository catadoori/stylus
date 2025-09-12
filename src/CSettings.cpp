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
#include "CSettings.h"
#include "../stylus_paths.h"
#include "CAStream.h"
#include "CApp.h"

#include <iostream>
using namespace std;

CSettings::CSettings():
    InputNoise(15),
    VinylNoise(100)
{

}

CSettings::~CSettings()
{
}


double CSettings::CGui::tFontSize::Application() const { return 10.0; }
double CSettings::CGui::tFontSize::Times      () const { return  0.0; }
double CSettings::CGui::tFontSize::TEdit      () const { return  0.0; }

CSettings::CAudio::CAudio():
    Format(SND_PCM_FORMAT_S16_LE),
    Access(SND_PCM_ACCESS_RW_INTERLEAVED),
    Rate(0),
    SamplesPSeg(512),
    BitsPSam(0),
    CanPause(false)
{

}




size_t CSettings::CAudio::GetBytesPerSample() const
{
    size_t ret = (BitsPSam  >> (3 - 1)) ; //  3 = 8 bits per byte, 1 = two channels for stereo
    return ret;  
}

size_t CSettings::CAudio::GetBytesPerSegment() const
{
    size_t ret = SamplesPSeg * (BitsPSam >> (3 - 1));
    return ret;
}

size_t CSettings::CAudio::GetBytesPerSec() const
{
    size_t ret =  Rate * GetBytesPerSample();
    return ret;
}

void CSettings::CAudio::Set(snd_pcm_format_t format, unsigned int rate)
{
    if ( !rate) return;
    Rate = rate;
    Format = format;
    BitsPSam = snd_pcm_format_width(format); 
}
 

void CSettings::Dump()
{
    char const * const sep = "    ";
    cout << "Settings:" << endl 
         << sep << "Alsa: library version " << SND_LIB_VERSION_STR << "\n"
         << sep << sep << "Format:           " << snd_pcm_format_name(Audio.GetFormat()) << endl
         << sep << sep << "Access mode:      " << snd_pcm_access_name(Audio.GetAccess()) << endl
         << sep << sep << "Sampling rate:    " << Audio.GetRate() << endl
         << sep << sep << "Has hw pause:     " << Audio.GetCanPause() << endl
         << sep << sep << "Bits per sample:  " << Audio.GetBitsPerSample() << endl
         << sep << sep << "Bytes per sample: " << Audio.GetBytesPerSample() << endl
         << sep << sep << "Bytes per sec:    " << Audio.GetBytesPerSec()
         << endl << endl;
}

CSettings::CProject::CProject()
{
    LBasePath = LOCAL_BASE_PATH;
    RBasePath = REMOTE_BASE_PATH; 
}

void CSettings::CProject::Clear()
{
    Name.clear();
    Artist.clear();
    Album.clear();
    Path.clear();
    File.clear();
    
}

QString CSettings::CProject::GetWavName(int side) const
{
    QString ret = Path;
    ret += CApp::SideToQChar(side);
    ret += ".wav";
    return ret;
}

QString CSettings::CProject::GetSubPath() const
{
    return Path.right(Path.length() - LBasePath.length());

}
