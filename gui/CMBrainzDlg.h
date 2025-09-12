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

#include "../config.h"

#if !defined(CMBRAINZDLG_H) && !defined(NO_MUSICBRANIZ)
#define CMBRAINZDLG_H

#include <QDialog>

/**
	@author Harry <harry@actionpussyclub.de>
*/

class QComboBox;
class CMultiCombo;
class QWidget;
class QTableWidget;
class QStatusBar;
class QToolButton;
class QRadioButton;
class QGroupBox;


#include "CMBrainz.h"
class CMBrainzDlg: public QDialog
{
    Q_OBJECT
    public:
                    CMBrainzDlg     (QWidget * parent);
                   ~CMBrainzDlg     ();
    public slots:
        void        WorkerFinished  (QString const& error);
        void        WorkerItem      (int act, QString const & arg_1, QString const &arg_2,
                                     QString const & arg_3, QString const & arg_4);
        

    signals:
        void        Closed          (bool);                                     
    private:
        struct tCtrls
        {
            CMultiCombo *Artists;
            CMultiCombo *Albums;
            QTableWidget*Tracks;
            QPushButton *GetArtists;
            QPushButton *GetAlbums;
            QPushButton *GetTracks;
            QPushButton *AddTracks;

            QPushButton *Assign;
            QGroupBox   *Options;
            struct
            {
                QRadioButton * Keep;
                QRadioButton * Rebuild;
                QRadioButton * RebuildDefaults;
            } TrackEdit;
            
        };
        tCtrls  Ctrls;

        CMBrainz  * Worker;
        QStatusBar* SBar;

        enum // Track fields
        {
            colNo,
            colSide,
            colTitle,
            colDuration
        };

        struct tTrackColumn
        {
            char const *        Caption;
            Qt::AlignmentFlag   Alignment;
            Qt::ItemFlag        Flag;

            QStringList Captions ();
        };
        static tTrackColumn const TrackColumn [];
        static int HintCount;
        

        QString Wanted;
        
        void    init            ();
        void    highlight       (CMultiCombo *, bool b);
        void    highlight_box   (CMultiCombo * box);
        void    enable_butts    ();

        void    block_signals   (QWidget * w, bool b);
        void    add_track       (QString const & name, QString const & duration);
        void    set_track_item  (int row, int col, QString const& text);
        void    show_error      ();
        void    closeEvent      (QCloseEvent * event);
        void    resizeEvent     (QResizeEvent * event);

        void    start_worker    (int action, QString const & search_item);

        int     track_count     (int side) const;
        int     side_count      () const;
        void    fill_track_side (int start_row);
        void    show_message    (QString const & msg, int delay = 5000, bool sticky = false);

    private slots:
        void    get_artists     ();
        void    get_albums      ();
        void    get_tracks      ();
        void    artist_edit     (QString const & text);
        void    album_change    (int);
        void    track_cell      (int row, int col);
        void    assign_tracks   ();
};

#endif
