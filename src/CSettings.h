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
#ifndef CSETTINGS_H
#define CSETTINGS_H

#include "../config.h"
#include "CRange.h"
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <QString>

/**
	@author Harry <harry@actionpussyclub.de>
*/


#include "CState.h"


class CSettings
{
    public:
        class CAudio
        {
            private:
                snd_pcm_format_t    Format;
                snd_pcm_access_t    Access;
                unsigned int        Rate;
                unsigned int        SamplesPSeg;  // size of one segment in samples
                unsigned int        BitsPSam;
                bool                CanPause;
                CAudio &            operator = (CAudio const & rhs);
            public:
                                    CAudio      ();
                snd_pcm_format_t    GetFormat   ()  const   { return Format;    }
                snd_pcm_access_t    GetAccess   ()  const   { return Access;    }
                unsigned int        GetRate     ()  const   { return Rate;      }


                 
                size_t  GetSamplesPerSegment    ()  const   { return SamplesPSeg  ; }
                void    SetSamplesPerSegment    (int size)  { SamplesPSeg = size  ; }

                size_t  GetBytesPerSample       ()  const;
                size_t  GetBytesPerSec          ()  const;
                size_t  GetBytesPerSegment      ()  const;
                size_t  GetBitsPerSample        ()  const   { return BitsPSam;  }
                bool    GetCanPause             ()  const   { return CanPause;  }
                void    Set                     (snd_pcm_format_t format, unsigned int rate);

                void    SetCanPause             (bool b)    { CanPause = b;     }
        };
        class CProject
        {
            public:
                                CProject        ();
                void            Clear           ();                              
                void            SetName         (QString const & n) { Name = n;         }
                void            SetArtist       (QString const & a) { Artist = a;       }
                void            SetAlbum        (QString const & a) { Album = a;        }
                
                void            SetPath         (QString const & p) { Path = p;         }
                void            SetFile         (QString const & f) { File = f;         }
                void            SetLBasePath    (QString const & p) { LBasePath = p;    }
                void            SetRBasePath    (QString const & p) { RBasePath = p;    }

                QString const & GetName         ()  const           { return Name;      }
                QString const & GetArtist       ()  const           { return Artist;    }
                QString const & GetAlbum        ()  const           { return Album;     }
                
                QString const & GetPath         ()  const           { return Path;      }
                QString const & GetFile         ()  const           { return File;      }
                QString         GetQualFile     ()  const           { return Path + File;}
                QString         GetWavName      (int side) const;
                QString const & GetLBasePath    ()  const           { return LBasePath; }
                QString const & GetRBasePath    ()  const           { return RBasePath; }
                QString         GetSubPath      ()  const;


            private:
                QString Name;
                QString Artist;
                QString Album;
                QString Path;
                QString File;
                QString WavName;
                QString LBasePath;  // Local base path
                QString RBasePath;  // Remote base path
            public:
                struct tTarget
                {
                    QString Format;
                    QString Mp3Mode;
                    int     Mp3Rate;
                    double  OggQuality;
                    int     FlacMode;
                    QString Genre;
                    int     Year;
                    QString Comment;
                    QString ConfigDir;
                    QString RecChain;
                    bool    UseRecChain;
                            tTarget(): UseRecChain(false) {}
                };
                tTarget Target;
        };
        class CGui
        {
            public:
                struct tFontSize
                {
                     double Application  ()  const;
                     double Times        ()  const;
                     double TEdit        ()  const;
                } FontSizes;
        };

    public:
                        CSettings     ();
                       ~CSettings     ();
                               
        CProject        Project;
        CAudio          Audio;
        CGui            Gui;
        void            Dump          ();
        unsigned        GetInputNoise ()         const { return InputNoise; }
        /*
            returns a value repesenting the total noise of the audio input chain, i. e. the
            cardrigide's noise + amplifier's preamp noise + sound card adc noise.
        */

        unsigned        GetVinylNoise ()         const { return VinylNoise; }
        /*
            returns a value repesenting the noise signal level when the disk is running
            and the stylus is located on the groove but no music is played yet.
        */
        

    private:
        char const *    FileName;
        unsigned        InputNoise;
        unsigned        VinylNoise;
        CSettings &     operator =   (CSettings const & rhs);
                
};

#endif


/*
    A sample is ment to be data of all available channels. E. g. for 16 bit stereo mode
    GetBytesPSam() == 4
     
*/
