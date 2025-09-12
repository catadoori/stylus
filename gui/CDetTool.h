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

#ifndef CDETTOOL_H
#define CDETTOOL_H

#include "CBaseTool.h"
#include "CDetector.h"
#include "../config.h"
#include "CGui.h"


class QComboBox;
class QAction;
class QDoubleSpinBox;
class QSpinBox;
class QMenu;
class CToolButton;


class CDetTool: public CBaseTool
{
    Q_OBJECT
    public:
                CDetTool    (QWidget * parent);
               ~CDetTool    ();
        void    Requery     ();
        
        void    SetSides    (int count, bool keep_selection);
        void    SetBuildMode(bool profile_building_view);
        int     MinLength   ()              const; // Minimum Track Length (Ctrls.Length)
        double  Silence     ()              const; // Intertrack silence in s (Ctrls.Silence)
        int     Noise       ()              const; //

        void    DoAction    (CGui::SpaceActions a);

        CDetector::
        Modes   Mode        ()              const; // Detection mode to be applied

        

        
        QAction*ManualSplit     () const { return Ctrls.Go.ManualSplit; }
        QAction*ManualSplitPause() const { return Ctrls.Go.ManualSplitPause; }
        QAction*Detect          () const { return Ctrls.Go.Detect;           }
        QAction*Guess           () const { return Ctrls.Go.Guess;            }

    private:
        struct tGo
        {
            QMenu       *Menu;
            QAction     *LastAction;
            
            CToolButton *Button;
            QAction     *Guess;
            QAction     *Detect;
            QAction     *ManualSplit;
            QAction     *ManualSplitPause;
            QAction     *SkipDrop;
            QAction     *CutStart;
            QAction     *CutEnd;
            QAction     *OutroNew;
            QAction     *OutroLast;
            QAction     *Build;
            QAction     *Stop;
        };
        struct tCtrls
        {

            tGo              Go;
            QSpinBox       * TrackSet;
            QComboBox      * Mode;
            QComboBox      * Length;
            QComboBox      * Silence;
            QComboBox      * Noise; // FIXME
            QComboBox      * Type; // FIXME

        } Ctrls;
        

        void    set_trackset    (int count);
        int     run_detect      ();
        void    set_build_view  (bool bv);
        void    enable_options  ();
        void    guess_count     ();
        void    find_tracks     ();

        
        
    private slots:
        void    build_prof      ();        
        void    on_action       (QAction *);
        void    toggled         (bool checked);

        void    side_changed    (int new_side);
        void    outro           ();
        void    outro_last      ();
        void    set_selected    (int set_nr);

};
        

#endif
