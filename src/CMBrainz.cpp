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

#include "CMBrainz.h"
#include <musicbrainz5/Query.h>
#include <musicbrainz5/Artist.h>
#include <musicbrainz5/Release.h>
#include <musicbrainz5/ReleaseList.h>
#include <musicbrainz5/ReleaseGroup.h>
#include <musicbrainz5/ReleaseGroupList.h>
#include <musicbrainz5/MediumList.h>
#include <musicbrainz5/TrackList.h>
#include <musicbrainz5/Track.h>
#include <musicbrainz5/Recording.h>
#include <musicbrainz5/Medium.h>


// search: http://musicbrainz.org/ws/2/recording/?query=release:%22The%20Rockfield%20Files%22%20AND%20artist:%22The%20Damned%22?inc=recordings
// browse releases: https://musicbrainz.org/ws/2/release-group?artist=77d21c13-846f-4f48-9546-873949eff6ae&limit=100


#if !defined (CONFIG_STYLUS_NO_MBRAINZ)
#include "CAStream.h"

CMBrainz::CMBrainz(QObject * parent):
    QThread(parent)
{
}

CMBrainz::~CMBrainz()
{
}

void CMBrainz::Start(int act, QString const & item)
{
    Action = act;
    SearchItem = item;
    start();
}

void CMBrainz::run()
{
    try
    {
        switch (Action)
        {
            case GetArtists: get_artists(); break;
            case GetAlbums: get_albums(); break;
            case GetTracks: get_tracks(); break;
        }
            emit Finished(QString());
    }
    catch (std::exception const & e)
    {
        emit Finished(e.what());
    }
 }


template <typename ReturnEntity, typename FilterEntity, bool Lookup>
void CMBrainz::list_items(string const &select_this, CQuery::tParamMap extra_params)
{
    CQuery::tParamMap params;
    params["limit"] = "100";
    params.insert(extra_params.begin(), extra_params.end());

    std::string lu_entity;

    if (Lookup)
        //lu_entity = FilterEntity::GetElementName() + "=" + select_this;
        lu_entity = select_this;
    else
        params[FilterEntity::GetElementName()] = select_this;

    MusicBrainz5::CQuery query(__FUNCTION__);
    ItemCount.Clear();

    do
    {
        Data = query.Query(ReturnEntity::GetElementName(), lu_entity, "", params);
        process_data<ReturnEntity, FilterEntity, Lookup>();
        params["offset"] = std::to_string(ItemCount.Actual);
    }
    while (ItemCount.Actual < ItemCount.Target);
}

void CMBrainz::get_artists()
{
    CQuery::tParamMap params;
    QString what = QString("\"%1\"").arg(SearchItem);
    params["query"] = what.toUtf8().toStdString();
    list_items<CArtist, CArtist, false>(SearchItem.toStdString(), params);
}

template <>
void CMBrainz::process_data<CArtist, CArtist, false>()
{
    CArtistList * list = Data.ArtistList();
    if (ItemCount.Target == 0)
        ItemCount.Target = list->Count();

    for (auto x = 0; x < list->NumItems(); ++x)
    {
        auto item = list->Item(x);
        emit Item(Action, QString::fromStdString(item->ID()),
                  QString::fromUtf8(item->Name().c_str()),
                  QString::fromUtf8(item->Disambiguation().c_str()));
        ++ItemCount.Actual;
    }
}

void CMBrainz::get_albums()
{
    // https://musicbrainz.org/ws/2/release/534e54df-5b95-4afe-92d9-31bb6417fd97?inc=media
    CQuery::tParamMap params;
    params["inc"] = "media";
    list_items<CRelease, CArtist, false>(SearchItem.toStdString(), params);
}

template <>
void CMBrainz::process_data<CRelease, CArtist, false>()
{
    CReleaseList * list = Data.ReleaseList();
    if (ItemCount.Target == 0)
        ItemCount.Target = list->Count();

    for (auto x = 0; x < list->NumItems(); ++x)
    {
        auto item = list->Item(x);
        auto media = item->MediumList();
        for (int x = 0; media && x < media->Count(); ++x)
        {
            auto medium = media->Item(x);
            emit Item(Action, QString::fromStdString(item->ID()),
                      QString::fromUtf8(item->Title()),
                      QString::fromStdString(medium->Format()),
                      QString::fromStdString(item->Date()));
        }
        ++ItemCount.Actual;
    }
}


void CMBrainz::get_tracks()
{
     CQuery::tParamMap params;
    params["inc"] = "recordings";
    list_items<CRelease, CRelease, true>(SearchItem.toStdString(), params);

    // https://musicbrainz.org/ws/2/release/31365b17-0224-475c-8419-c589ce72d44f?inc=recordings
    // https://musicbrainz.org/ws/2/release/035aae28-0ce9-4198-820f-c0ceac24eae3?inc=recordings
}

template <>
void CMBrainz::process_data<CRelease, CRelease, true>()
{
    CRelease * release = Data.Release();
    auto media = release->MediumList();

    for (int m = 0; m < media->Count(); ++m)
    {
        auto medium = media->Item(m);
        auto tl = medium->TrackList();
        for (int t = 0; t < tl->Count(); ++t)
        {
            auto track = tl->Item(t);
            auto r = track->Recording();

            QString duration;
            CAStream::SecsToTimeString(r->Length() / 1000, duration);
            emit Item(Action, QString::fromUtf8(r->Title()), duration);
        }
    }
}

#endif
