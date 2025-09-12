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
#ifndef CPROJSET_H
#define CPROJSET_H

#include <alsa/asoundlib.h>
#include <QDialog>
#include <lame/lame.h>

class QWidget;


#include <QLineEdit>
#include <QLabel>
class QComboBox;
class QSpinBox;
class QCheckBox;
class QDoubleSpinBox;

/**
	@author Harry <harry@actionpussyclub.de>
*/



class CProjSet: public QDialog
{
    Q_OBJECT
    public:
                            CProjSet    (QWidget * parent, bool basepath_change, bool read_current = true);
        virtual            ~CProjSet    ();

        snd_pcm_format_t    GetFormat   () const;
        unsigned            GetRate     () const;
        QString             GetName     () const    { return Name->text();     }
        QString             GetPath     () const    { return Path->text();     }  
        QString             GetLBasePath() const    { return LBasePath->text();}
        QString             GetRBasePath() const    { return RBasePath->text();}
        QString             GetAlbum    () const    { return _Album;           }
        QString             GetArtist   () const    { return Artist->text();   }

        struct tTarget
        {
            QString     GetFormat   () const;
            unsigned    GetMp3Rate  () const;
            QString     GetMp3Mode  () const;
            double      GetOggQual  () const;
            int         GetFlacMode () const;
            QString     GetGenre    () const;
            QString     GetComment  () const     { return Comment->text();     }
            QString     GetRecChain () const;
            bool        WithRecChain() const;
            int         GetYear     () const;
            QString     GetConfigDir() const;

            QComboBox*  Format;
            QComboBox*  Mp3Mode;
            QLabel   *  Mp3RateCaption;
            QComboBox*  Mp3Rate;
            QDoubleSpinBox* OggQuality;
            QSpinBox*   FlacMode;
            QComboBox*  Genre;
            QSpinBox*   Year;
            QLineEdit*  Comment;
            QComboBox*  RecChain;
            QCheckBox*  _WithRecChain;
            QComboBox*  ConfigDir;
        };
        tTarget Target;


        int                 GetOutFmt   () const;
        unsigned            GetOutRate  () const;

        static void         FillFormat      (QComboBox * box);
        static void         LameFillGenre   (QComboBox * box);
        static void         LameFillBitrate (QComboBox * box);
        static void         LameFillQuality (QComboBox * box);
        static void         LameFillVbrMode (QComboBox * box);
        
        static void         SetMp3Combos    (QComboBox * mode, QComboBox * rate, int rate_data);
        static
        QDoubleSpinBox *    CreateOggSpinBox(QWidget * parent);
        static
        QSpinBox *          CreateFlacBox   (QWidget * parent);
        static
        QComboBox *         CreateConfigBox (QWidget * parent);

        static int          VbrMode         (QString const & in);
        static vbr_mode     LameVbrMode     (QString const & in);
        static int          GetFormat       (QString const & in);
        
        static QString      rate_string    (int rate);

    private:
        static char const * LameVbrModes[];
        static char const * ConfigDirs[];
        static int  const   SampleRates[];
        QLineEdit*          Name;
        QLineEdit*          Artist;
        QLineEdit*          Album;
        QLineEdit*          Path;
        QLineEdit*          LBasePath;
        QLineEdit*          RBasePath;
        QComboBox*          Format;
        QComboBox*          Rate;
        QString             _Album;
        bool                BasePathChange;

        static void         lame_genre_callb (int nr, char const * genre, void*);

        void                read_current();
        
    private slots:
        void                ldir_select     ();
        void                rdir_select     ();
        void                mp3_mode_changed(int mode);
        void                accept          ();
        void                edited          ();
        void                target_chain    (int state);

};


#endif
