#ifndef CTEDIT_H
#define CTEDIT_H

#include <QDockWidget>
#include <QString>

#include "CBaseEnc.h"

class CTList;
class QTabBar;
class QVBoxLayout;
class QFrame;
class QToolButton;
class QToolBar;
class QTextStream;
class QToolButton;

class CTEdit: public QWidget
{
    Q_OBJECT
    public:
                CTEdit      (QWidget * parent);
                
        int     SideCount   ()              const;
        int     TrackCount  (int side)      const;
        int     TrackCount  ()              const;
        void    WriteTracks (QTextStream &) const;
        QString DataFile    (int side)      const;
        void    Clear       ();
        void    SetSide     (int side);
        int     GetSide     ()              const;
        QChar   GetSideChar ()              const;
        void    AddSide     ();
        void    AddSides    (int count);
        void    SetData     (int row, int col, QString const & data);
        void    SetData     (CBaseEnc::tInfo const &);
        void    SetSideData (int side, int col, QString const & data);
        void    MarkPlayTime(bool with_pause);
        void    SetVolumes  (double db);
        void    SetTimings  (int side, unsigned set_offset);
        void    ResizeGrid  ();

        void    SetSideLengths  (QString const & list);
        QString GetSideLengths  ()              const;   // String listing each side and its length in seconds
        void    SetSideLength   (int side, double secs);

        double  GetSideLength   (int side)      const;
        double  GetSideLength   ()              const;        
        void    UnSync          ();
        void    Sync            ();
        bool    SyncChecked     () const;
        void    UncheckMBrainz  ();

        bool    GetEncodeJob    (CBaseEnc::tInfo & info, int side_to_query,
                                 bool with_stream_info, bool with_rec_chain_info,
                                 bool only_selected);
        void    SetDefaults      () { defs_empty (); }                                 

    private slots:
        void    add_side        (bool refresh = true);
        void    rem_side        ();
        void    swap_side       ();
        void    side_changed    (int);
        void    add_track       ();
        void    rem_track       ();
        void    track_up        ();
        void    track_down      ();
        void    track_left      ();
        void    track_right     ();

        void    defs_empty      ();
        void    defs_all        ();
        void    clear_time_cell ();
        void    clear_time_all  ();
        void    clear_stati     ();

        void    synced          ();

#ifdef CONFIG_STYLUS_DEV_HELPERS
        void    slot_1          (int x);
#endif
    private:
        QToolBar    *TTools;
        QToolBar    *STools;
        CTList      *TList;
        QTabBar     *Tab;
        int         OldSide;
        struct 
        {
            QAction     * AddSide;
            QAction     * RemSide;
            QAction     * SwapSide;
            QAction     * MBrainz;

            QAction     * Sync;
            QAction     * AddTrack;
            QAction     * RemTrack;
            QAction     * TrackUp;
            QAction     * TrackDown;
            QAction     * TrackLeft;
            QAction     * TrackRight;
            QToolButton * Defaults;
            QAction     * DefaultsEmpty;
            QAction     * DefaultsAll;
            QAction     * ClearStatus;
            QToolButton * ClearTime;
            QAction     * ClearTimeCells;
            QAction     * ClearTimeAll;
        } Actions;
        QString track_data  (int track, int col)    const;

//        void      keyPressEvent (QKeyEvent * event);
        
};

#endif


