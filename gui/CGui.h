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
 
#ifndef CGUI_H
#define CGUI_H

#include "../config.h"

#include <QMainWindow>
#include <QDockWidget>

#include "Excepts.h"
#include "CState.h"
#include <QBasicTimer>


class QStatusBar;
class CTEdit;
class CWEdit;
class CDetTool;

class QAction;
class CAStream;
class CXRuns;
class CMeters;
class CPTool;
class CRTool;
class CEncTool;
class CNormTool;
class CMBranzDlg;
class QFrame;
class QComboBox;
class CMBrainzDlg;


class CGui: public QMainWindow
{
    Q_OBJECT
    public:
                        CGui         ();
        virtual        ~CGui         ();

        void            Requery      ();

        bool            CanClose     ();
        void            StatusMessage(QString const & msg, int time = 0, bool add = false);
        void            StatusWarning(QString const & msg, int time = 0, bool add = false);

        bool            KeyPressed   (int key, QWidget * emitter);
        void            SetRecStart  ();

        void            SetTitle     ();
        void            SetSides     (bool keep_selection = false);

        CTEdit     * TEdit   ()  const    { return _TEdit;  } // Track editor
        CWEdit     * WEdit   ()  const    { return _WEdit;  } // Wave editor
        CPTool     * PTool   ()  const    { return _PTool;  } // Play toolbar
        CRTool     * RTool   ()  const    { return _RTool;  } // Record toolbar
        CXRuns     * XRuns   ()  const    { return _XRuns;  } // XRuns window
        CMeters    * Meters  ()  const    { return _Meters; } // VU Meters
        CDetTool   * DetTool ()  const    { return _DetTool;} // Detection toolbar
        CEncTool   * EncTool ()  const    { return _EncTool;} // Encoder toolbar
        CNormTool  * NormTool()  const    { return _NormTool;}// Normalizer toolbar

        enum SpaceActions // Button role: Role of button visible in status bar
        {
            saPlay,
            saRecord,
            saSplitTrack,
            saSplitTrackPause,
            saDetect,
            saQuery
        };

        SpaceActions SpaceBarAction () const;

    public slots:
        void            ToggleMusicBrainz (bool checked);


    private:
        CTEdit      * _TEdit;
        CWEdit      * _WEdit;
        CXRuns      * _XRuns;
        CMeters     * _Meters;
        CPTool      * _PTool;
        CRTool      * _RTool;
        CDetTool    * _DetTool;
        CEncTool    * _EncTool;
        CNormTool   * _NormTool;
        QToolBar    * _SpaceToolBar;
        QComboBox   *_SpaceAction;
        struct tMBrainz
        {
             CMBrainzDlg *Form;
             QPoint       Pos;
                          tMBrainz(): Form(0) {}
        } MBrainz;

        struct tProjMenu
        {
            QMenu   *Menu;
            QAction *New;
            QAction *Rebuild;
            QAction *Open;
            QAction *Save;
            QAction *Close;
            QAction *Quit;
            QAction *Settings;
            QAction *Load;
            QAction *Load2;
        };

        struct tViewMenu
        {
            QMenu   *Menu;
        };

        struct
        {
            tProjMenu   Proj;
            tViewMenu   View;
        } Menu;


    private:
        QBasicTimer Timer;
        QToolBar *  PTools; // Project Toolbar

        void            create_mainm    ();
        void            create_statbar  ();
        void            enable_tools    (bool eanble);
        void            timerEvent      (QTimerEvent * );
        void            closeEvent      (QCloseEvent * e);


        void            show_message   (QString const & msg, int time, bool add);
        bool            maybe_save      ();
    private slots:
        void            space_action    (int);
        void            project_new     ();
        void            project_open    ();
        void            project_save    ();
        void            project_close   ();
        void            project_save_as ();
        void            project_settings();
        void            project_rebuild ();
#ifdef CONFIG_STYLUS_DEV_HELPERS
        void            load            ();
        void            load2           ();
#endif

};

#endif
