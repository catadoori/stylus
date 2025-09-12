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
#ifndef CMBRAINZ_H
#define CMBRAINZ_H

/**
	@author Harry <harry@actionpussyclub.de>
*/

#include "../config.h"
#if !defined (CONFIG_STYLUS_NO_MBRAINZ)

#include <QThread>
#include <set>
#include<musicbrainz5/ListImpl.h>
#include<musicbrainz5/Query.h>
#include<musicbrainz5/Metadata.h>

using namespace std;
using namespace MusicBrainz5;


class CMBrainz: public QThread
{
    Q_OBJECT
    public:
        enum 
        {
            GetArtists,
            GetAlbums,
            GetTracks
        };
                    CMBrainz    (QObject * parent);
                   ~CMBrainz    ();
        void        Start       (int act, QString const & item);
        int         GetAction   () const { return Action; }

    signals:
        void        Finished    (QString const & error);
        void        Item        (int act, QString const & arg_1, QString const &arg_2,
                                          QString const & arg_3 = QString(),
                                          QString const & arg_4 = QString());
            
                           
    private:
        void        run         ();
        int         Action;
        QString     SearchItem;

        struct
        {
            int Target = 0;
            int Actual = 0;
            void Clear() { Target = Actual = 0; }
        }
        ItemCount;

        CMetadata Data;

        void        get_artists ();
        void        get_albums  ();
        void        get_tracks  ();

        template <typename ReturnEntity, typename FilterEntity, bool Lookup>
        void list_items(string const &select_this,
                        CQuery::tParamMap params = CQuery::tParamMap());

        template <typename ReturnEntity, typename FilterEntity, bool Lookup>
        void process_data();

};

#endif
#endif

