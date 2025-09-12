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

#ifndef CBASEENC_H
#define CBASEENC_H

#include <QString>

#include <lame/lame.h>

class CBufSeg;
class QFile;

class CBaseEnc
{
    public:
        enum eCommentTags
        {
            ctEncoder, ctTitle, ctAlbum, ctArtist, ctGenre,
            ctDate, ctTrackNumber, ctComment
        };

        static char const* CommentTag (eCommentTags e)
        {
            return CommentTags[e];
        }
        enum  // Encoders
        {
            ecMp3,
            ecVorbis,
            ecFlac,
            ecNone
        };
        struct tEncoder
        {
            char const * Name;
            char const * Ext;
            double       Cbr; 
            double       Vbr;
        };
        static tEncoder const Encoder[];
        struct tInfo
        {
            int     Side;
            int     Track;
            int     Offs;
            bool    Selected;
            struct tIn
            {
                QString  File;
                int      Start;    // in samples
                int      Length;   // in samples
                int      Rate;
                int      Format;
            };

            struct tOut
            {
                int      Format;
                QString  File;
                QString  LPath;
                QString  RPath;
                QString  ConfigDir;
                QString  SubPath;
                vbr_mode Mp3Mode;
                double   Quality;
                float    Scale;
                int      EncQuality;
                int      NormQuality(); // Normalized "Quality"
            };
            struct tTags
            {
                QString Title;
                QString Artist;
                QString Album;
                QString Year;
                QString Genre;
                QString Comment;
                QString ComposedComment;
                QString RecChain;
                QString Track;
                void    ComposeComment ();
                void    SplitComment   ();

            };
            tIn     In;
            tOut    Out;
            tTags   Tags;
        };
                        CBaseEnc    ();
        virtual        ~CBaseEnc    ();
        virtual bool    Init        (tInfo const &, bool create_local, bool create_remote);
        virtual void    Process     (CBufSeg const * seg) = 0;
        virtual bool    Flush       () = 0;

        virtual bool    ReadTags    (QString const & file, tInfo & output);

                int     GetFrameSize()                      const   { return FrameSize; }


    protected:
        static char const * CommentTags[];
        int     FrameSize;
        QFile  *Local;
        QFile  *Remote;

        static int          clipped_int (qreal in, int lower_limit, int upper_limit);
        void                init_file   (bool local, bool remote);
        void                open_file   (QFile * file, QString const & base_p, QString const & config_p,
                                         QString const & sub_p, QString const & name);
        void                open_files  (tInfo const & info);
        void                write       (char * data, int len);
        void                seek        (qint64 pos);
        void                tell        (qint64 *)  const;
        virtual char const* recchain_tag()  const                { return "TXXX"; }

        static QString const RecChainTag;
};

#endif


