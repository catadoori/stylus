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


#include "CGui.h"
#include "CTEdit.h"
#include "CWEdit.h"
#include "CProjSet.h"

#include <QMainWindow>
#include <QStatusBar>
#include <QMenuBar>
#include <QListWidget>
#include <QVBoxLayout>
#include <QDialog>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include "Excepts.h"

#include "CXRuns.h"
#include "CMeters.h"
#include "CPTool.h"
#include "CRTool.h"
#include "CDetTool.h"
#include "CEncTool.h"
#include "CNormTool.h"
#include <iostream>
#include <QCloseEvent>
#include <QComboBox>
#include "CMBrainzDlg.h"


#include "CApp.h"
#include "Excepts.h"
#include "CAStream.h"
#include "CXRuns.h"

extern CApp * App;


#define NO_WEDIT


CGui::CGui()

{
    _TEdit = new CTEdit(this);
#ifndef NO_WEDIT
    _WEdit = new CWEdit(this);
#endif
    _XRuns = new CXRuns(this);
    _Meters = new CMeters(this);
    _PTool = new CPTool(this);
    _RTool = new CRTool(this);
    _DetTool = new CDetTool(this);
    _EncTool = new CEncTool(this);
    _NormTool = new CNormTool(this);
    _SpaceToolBar = new QToolBar("Space actions", this);

    create_mainm();

    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(Qt::TopToolBarArea, DetTool());
    addToolBar(Qt::TopToolBarArea, NormTool());
    addToolBar(Qt::TopToolBarArea, EncTool());

    _SpaceAction = new QComboBox(_SpaceToolBar);
    connect (_SpaceAction, SIGNAL(currentIndexChanged(int)), this, SLOT(space_action(int)));
    _SpaceToolBar->addWidget(_SpaceAction);
#if 1
    QFont f (font());
    f.setPointSizeF(8);
    _SpaceAction->setFont(f);
#endif
    _SpaceAction->addItem(QIcon(":/icons/player_play.png"), "");
    _SpaceAction->addItem(QIcon(":/icons/rec_rec.png"), "");
    _SpaceAction->addItem(QIcon(":/icons/split.png"), "");
    _SpaceAction->addItem(QIcon(":/icons/split_pause.png"), "");
    _SpaceAction->addItem(QIcon(":/icons/find.png"), "");
    _SpaceAction->addItem(QIcon(":/icons/ark_help.png"), "");
    _SpaceAction->setToolTip("Space bar action");
    addToolBar(Qt::TopToolBarArea, _SpaceToolBar);

#ifndef NO_WEDIT
    setCentralWidget(WEdit());
#else
    setCentralWidget(TEdit());
#endif
    addToolBar(Qt::BottomToolBarArea, PTool());
    addToolBar(Qt::BottomToolBarArea, RTool());


    addToolBarBreak(Qt::BottomToolBarArea);

    addDockWidget(Qt::LeftDockWidgetArea, Meters());
#ifndef NO_WEDIT
    addDockWidget(Qt::TopDockWidgetArea, TEdit());
#else

    addDockWidget(Qt::RightDockWidgetArea, XRuns());
    XRuns()->setVisible(false);

#endif
    setWindowIcon(QIcon(":/icons/pencil.png"));
    show();
#ifndef NO_WEDIT
    resize(800,600);
#else
    resize(850, 500);
#endif

}

CGui::~CGui()
{

}


void CGui::ToggleMusicBrainz(bool checked)
{
#if !defined (CONFIG_STYLUS_NO_MBRAINZ)
    if (checked)
    {
        MBrainz.Form = new CMBrainzDlg(this);
        MBrainz.Form->show();
        if (!MBrainz.Pos.isNull())
            MBrainz.Form->move(MBrainz.Pos);
        connect(MBrainz.Form, SIGNAL(Closed(bool)), this, SLOT(ToggleMusicBrainz(bool)));
    }
    else
    {
        if (MBrainz.Form)
        {
            MBrainz.Pos = MBrainz.Form->pos();
            MBrainz.Form->close();
        }

        delete MBrainz.Form;
        MBrainz.Form = 0;
        TEdit()->UncheckMBrainz();
    }
#endif
}


void CGui::create_mainm()
{
    Menu.Proj.Menu = menuBar()->addMenu("&Project");
    Menu.Proj.New = Menu.Proj.Menu->addAction(QIcon(":/icons/filenew.png"), "&New");
    connect(Menu.Proj.New, SIGNAL(triggered()), this, SLOT(project_new()));

    Menu.Proj.Rebuild = Menu.Proj.Menu->addAction(QIcon(":/icons/rerecord.png"), "&Rebuild");
    connect(Menu.Proj.Rebuild, SIGNAL(triggered()), this, SLOT(project_rebuild()));
    Menu.Proj.Rebuild->setStatusTip("Re - record audio using track data from (remotely) existing files.");

    Menu.Proj.Open = Menu.Proj.Menu->addAction(QIcon(":/icons/fileopen.png"), "&Open");
    connect(Menu.Proj.Open, SIGNAL(triggered()), this, SLOT(project_open()));

    Menu.Proj.Save = Menu.Proj.Menu->addAction(QIcon(":/icons/filesave.png"), "&Save");
    connect(Menu.Proj.Save, SIGNAL(triggered()), this, SLOT(project_save()));
    Menu.Proj.Close = Menu.Proj.Menu->addAction(QIcon(":/icons/remove.png"), "&Close");
    connect(Menu.Proj.Close, SIGNAL(triggered()), this, SLOT(project_close()));

    Menu.Proj.Settings = Menu.Proj.Menu->addAction(QIcon(":/icons/settings2.png"), "&Settings");
    connect(Menu.Proj.Settings, SIGNAL(triggered()), this, SLOT(project_settings()));

    Menu.Proj.Menu->addSeparator();
    Menu.Proj.Quit = Menu.Proj.Menu->addAction(QIcon(":/icons/exit.png"), "&Quit");
    connect(Menu.Proj.Quit, SIGNAL(triggered()), this, SLOT(close()));

#ifdef CONFIG_STYLUS_DEV_HELPERS
    Menu.Proj.Load = Menu.Proj.Menu->addAction("Load 16");
    connect(Menu.Proj.Load, SIGNAL(triggered()), this, SLOT(load()));
    Menu.Proj.Load2 = Menu.Proj.Menu->addAction("Load 32");
    connect(Menu.Proj.Load2, SIGNAL(triggered()), this, SLOT(load2()));
#endif

    Menu.View.Menu = menuBar()->addMenu("&View");

    Menu.View.Menu->addAction(Meters()->toggleViewAction());
    Menu.View.Menu->addAction(XRuns()->toggleViewAction());
    Menu.View.Menu->addSeparator();

    Menu.View.Menu->addAction(RTool()->toggleViewAction());
    Menu.View.Menu->addAction(PTool()->toggleViewAction());
    Menu.View.Menu->addAction(DetTool()->toggleViewAction());
    Menu.View.Menu->addAction(NormTool()->toggleViewAction());
    Menu.View.Menu->addAction(EncTool()->toggleViewAction());
    Menu.View.Menu->addAction(_SpaceToolBar->toggleViewAction());

    PTools = addToolBar("Project");
    PTools->setFocusPolicy(Qt::NoFocus);
    PTools->addAction(Menu.Proj.New);
    PTools->addAction(Menu.Proj.Open);
    PTools->addAction(Menu.Proj.Save);
    PTools->addAction(Menu.Proj.Close);
    PTools->addSeparator();
    PTools->addAction(Menu.Proj.Settings);
#ifdef CONFIG_STYLUS_DEV_HELPERS
    PTools->addSeparator();
    PTools->addAction(Menu.Proj.Load);
    PTools->addAction(Menu.Proj.Load2);
#endif

    create_statbar();
}

CGui::SpaceActions CGui::SpaceBarAction() const
{
    return (SpaceActions) _SpaceAction->currentIndex();
}
void CGui::create_statbar()
{
    statusBar()->showMessage(" ");
//    statusBar()->insertPermanentWidget(0, _SpaceAction);
}

void CGui::space_action(int widget)
{
    SpaceActions sa = SpaceActions(widget);
    QWidget * w;
    switch (sa)
    {
        case saRecord:          w = RTool();   break;
        case saSplitTrack:      w = DetTool(); DetTool()->ManualSplit()->setChecked(true); break;
        case saSplitTrackPause: w = DetTool(); DetTool()->ManualSplitPause()->setChecked(true); break;
        case saQuery:           w = DetTool(); DetTool()->Guess()->setChecked(true); break;
        case saDetect:          w = DetTool(); DetTool()->Detect()->setChecked(true);break;

        case saPlay:
        default:            w = PTool(); break; 
    }
    _SpaceAction->clearFocus();
    w->setFocus();
}


void CGui::project_new() 
{
    App->NewProject();
    Timer.start(60, this);
}


void CGui::project_open()
{
    QString name;

    name = QFileDialog::getOpenFileName(this, "Open project", App->Settings.
                                        Project.GetLBasePath(), "Stylus project (*.stylus)");
    if (!name.isEmpty())
    {
        if (App->OpenProject(name))
            Timer.start(60, this);
    }
    space_action(0);
}

void CGui::project_save()
{
    if (App->Settings.Project.GetFile().isEmpty())
        project_save_as();
    else
        App->SaveProject();
    StatusMessage(QString("Saved file '%1'").arg(App->Settings.Project.GetFile()), 2);
}

void CGui::project_save_as()
{
    CSettings::CProject & s = App->Settings.Project;
    s.SetFile(QFileDialog::getSaveFileName(this, "Save current project as",
                                   s.GetPath(), "Stylus project (*.stylus)"));
    if (!s.GetFile().isEmpty())
    {
        char const * ext = "stylus";
        QFileInfo i (s.GetFile());
        if (i.suffix() != ext) 
            s.SetFile(s.GetFile() + QString('.') + ext);
        App->SaveProject();
    }
}

void CGui::project_close()
{
    if (App->CloseProject(true))
        Timer.stop();
}

void CGui::project_settings()
{
    App->SetSettings(false, true);
}

void CGui::project_rebuild()
{
    QString src_dir;
#if !defined (CONFIG_STYLUS_DEV_HELPERS)
    src_dir = QFileDialog::getExistingDirectory(this, "Select directory where tracks reside in:",
                                                App->Settings.Project.GetRBasePath());
#else
    src_dir = "/home/harry/rec/music/stylus/pil/live_in_tokyo";
#endif
    if (src_dir.isEmpty())
        ;
    else
        App->NewProject(src_dir);
}

#ifdef CONFIG_STYLUS_DEV_HELPERS
void CGui::load()
{
    App->OpenProject("/home/harry/rec/music/stylus/johnny_cash/american_vi__ain_t_no_grave/s.stylus");
     Timer.start(60, this);
    space_action(0);

}

void CGui::load2()
{
    App->OpenProject("/home/harry/rec/music/stylus/damned/rockfield_files/s.stylus");
     Timer.start(60, this);
    space_action(0);

}
#endif


void CGui::enable_tools(bool enable)
{
    PTool()->setEnabled(enable);
    PTool()->Requery();

    RTool()->setEnabled(PTool()->isEnabled());
    RTool()->Requery();

    DetTool()->setEnabled(enable);
    DetTool()->Requery();

    NormTool()->setEnabled(enable);
    NormTool()->Requery();

    EncTool()->setEnabled(enable);
}

void CGui::SetSides(bool keep_selection)
{
    PTool()->SetSides(TEdit()->SideCount(), keep_selection);
    RTool()->SetSides(TEdit()->SideCount(), keep_selection);
    DetTool()->SetSides(TEdit()->SideCount(), keep_selection);
    NormTool()->SetSides(TEdit()->SideCount(), keep_selection);
}

void CGui::Requery()
{
    bool enabled = !App->Settings.Project.GetName().isEmpty();
    enable_tools(enabled);    
    TEdit()->setEnabled(enabled);
    Menu.Proj.Save->setEnabled(enabled);
    Menu.Proj.Close->setEnabled(enabled);
    Menu.Proj.Settings->setEnabled(enabled);
    Meters()->setEnabled(PTool()->isEnabled() || RTool()->isEnabled());
    _SpaceAction->setEnabled(enabled);
    EncTool()->Reset();
    Meters()->Refresh();
}

void CGui::StatusMessage(QString const & msg, int time, bool add)
{
    statusBar()->setFont(font());
    statusBar()->setPalette(palette());
    show_message(msg, time, add);
}

void CGui::StatusWarning(QString const & msg, int time, bool add)
{
    QFont f;
    f.setBold(true);
    statusBar()->setFont(f);
    QPalette p;
    p.setColor(QPalette::Active, QPalette::WindowText, QColor(Qt::darkRed));
    statusBar()->setPalette(p);
    show_message(msg, time, add);
}

void CGui::show_message(QString const & msg, int time, bool add)
{
    QString str = statusBar()->currentMessage();
    if (add && !str.isEmpty())
        str = (str + ", ") +  msg;
    else
        str = msg;
    statusBar()->showMessage(str, time * 1000);
}

void CGui::SetTitle()
{
    setWindowTitle(QString("%1 [*] - stylus").arg(App->Settings.Project.GetName()));
    setWindowModified(false);
}


void CGui::timerEvent(QTimerEvent * event)
{

    if(event->timerId() == Timer.timerId())
    {
        Meters()->Refresh();
        PTool()->Refresh();
        RTool()->Refresh();
    }
    else
        QWidget::timerEvent(event);
}

bool CGui::CanClose() 
{
    if (RTool()->Active())
    {
        QString msg = QString("Can't close. Currently side %1 is beeing recorded.").
                              arg(CApp::SideToQChar(RTool()->GetSide()));
        QMessageBox::information(this, "stylus", msg);
        return false;
    }
    return true;
}

void CGui::closeEvent(QCloseEvent * e)
{
    if (!CanClose())
        e->ignore();
    else if (isWindowModified())
    {
        if (maybe_save())
            e->accept();
        else
            e->ignore();
    }
}

bool CGui::maybe_save()
{
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, "stylus", "Do you want to save your changes?",
                      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        App->SaveProject();
     else if (ret == QMessageBox::Cancel)
        return false;
     return true;
}

bool CGui::KeyPressed(int key, QWidget * /* emitter */ )
{
    bool handled = false;
    if (key == 32)
    {
        handled = true;
        if (SpaceBarAction() == saPlay)
            PTool()->Go();
        else if (SpaceBarAction() >= saSplitTrack &&
                 SpaceBarAction() <= saQuery)
        {
            DetTool()->DoAction(SpaceBarAction());
        }
        else if (SpaceBarAction() == saRecord)
        {
            double pos = App->Rec()->GetPosSec();
            RTool()->StopGo();
            if (pos == 0.0)
            {
               _SpaceAction->setCurrentIndex(int(saPlay));
            }
        }
    }
    return handled;
}

void CGui::SetRecStart()
{
    _SpaceAction->setCurrentIndex(int(saRecord));
}
