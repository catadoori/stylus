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


#ifndef CTLIST_H
#define CTLIST_H

#include <QTableView>
#include <QItemDelegate>

#include "CBaseEnc.h"
#include <lame/lame.h>
#include "CAStream.h"



class QWidget;
class QComboBox;
class QTabBar;
class QVBoxLayout;
class QSortFilterProxyModel;
class QStandardItemModel;

class CTList: public QTableView
{
    Q_OBJECT
    public:
                    CTList      (QWidget * parent);
        virtual    ~CTList      (); 
        enum  // columns 
        {
            colNo,
            colStatus,
            colStart,
            colEnd,
            colDuration,
            colVolume,
            colTitle,
            colArtist,
            colGenre,
            colYear,
            colFormat,
            colRate,
            colMode,
            colComment,
            colSide,
            colConfigDir,
            ColCount = 16,
            colLast = colConfigDir
        };
        static char const * ColCaps [ColCount];
        void    AddTrack    (int side);
        void    Clear       ();
        void    RemTrack    ();
        void    TrackUp     ();
        void    TrackDown   ();
        void    TrackLR     (int current_side, int count, bool right);
        void    SetData     (int row, int col, QString const & data, bool filtered = false);

        void    SetSideData (int col, QString const & data, bool always = true);
        void    SetSideData (int side, int col, QString const & data);
        void    SetTimings  (int side, int track, int start_sample, int end_sample);
        void    FillSideData(bool also_nonempty_fields);

        bool    SetNextTime (int side, QString const & time);
        // returns true if the start time of the first track has been set.

        void    ClearSelectedTimes();
        
        bool    IsEdited    ()    const { return state() == QAbstractItemView::EditingState; }
        bool    IsSyncedPlay()    const;
        
        void     FilterOnSide(int side);
        int      TrackCount  (int side)                      const;
        int      TrackCount  ()                              const;
        QVariant GetData     (int track, int col)            const;  // from StandardModel
        void     GetData     (int track, CBaseEnc::tInfo &,
                              bool stream_info, bool chain_info) const; // from StandardModel
        void     ResizeGrid  (int first = colStart, int last = colLast);
        
        
        void     Test1       ();
    private:
        class CDelegate: public QItemDelegate
        {
            public:
                CDelegate (QObject * parent);

            private:
                QWidget *   createEditor(QWidget *parent, 
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const;
                
                void        setModelData(QWidget *editor, 
                                         QAbstractItemModel *model,
                                         const QModelIndex &index) const;

                void        fmt_changed (int fmt, QAbstractItemModel * mode,
                                         QModelIndex const & index) const;

                void        mode_changed(int mode, QAbstractItemModel * model,
                                         QModelIndex const & index) const;

                void        clear_cell  (QAbstractItemModel * model,
                                         QModelIndex const & index, int col) const ;
            

                static QComboBox *  ACombo;
        };

        int                       ColWidths [ColCount];
        QSortFilterProxyModel   * PModel; 
        QStandardItemModel      * SModel;
        CDelegate               * Delegate;

        double      get_time    (int col, int row) const;
        void        set_time    (int col, int row, double & target,
                                 double lower = 0.0, double upper = CAStream::MaxSideSecs);
        void        enum_tracks ();
        void        col_widths  (bool restore);
        void        swap_tracks (int first, int second);

        void        wheelEvent  (QWheelEvent * event);
        void        trim_time   (int col, int row, int delta, Qt::MouseButtons, bool next_row);
        void        set_duration(int row);
        void        sync_message(char const * action) const;
        

        void        filter_model(QSortFilterProxyModel & model, int side) const;
        Qt::
        AlignmentFlag alignment (int col);

        void       keyPressEvent(QKeyEvent * event);
        
    private slots:
        void        dbl_clicked (const QModelIndex & index);



};

#endif
