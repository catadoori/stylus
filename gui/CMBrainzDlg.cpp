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
#include "CMBrainzDlg.h"
#if !defined(CONFIG_STYLUS_NO_MBRAINZ)

#include <QTableWidget>
#include <QGridLayout>
#include <QLabel>

#include "CApp.h"
#include "CSettings.h"
#include <QPushButton>
#include <QStatusBar>
#include <QHeaderView>
#include <QScrollBar>
#include "CMultiCombo.h"
#include <QToolButton>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include "CGui.h"
#include "CTEdit.h"
#include "CTList.h"



extern CApp * App;


int CMBrainzDlg::HintCount = 0;

CMBrainzDlg::tTrackColumn const CMBrainzDlg::TrackColumn[] =
{
  {  "No",       Qt::AlignRight,  Qt::ItemIsEnabled },
  {  "Side",     Qt::AlignCenter, Qt::ItemIsEnabled },
  {  "Name",     Qt::AlignLeft,   Qt::ItemIsEnabled },
  {  "Duration", Qt::AlignRight,  Qt::ItemIsEnabled },
  {  0,          Qt::AlignRight,  Qt::ItemIsEnabled }
};

CMBrainzDlg::CMBrainzDlg(QWidget * parent):
    QDialog(parent)
{
    Worker = new CMBrainz(this);
    connect(Worker, SIGNAL(Finished(QString const&)),
            this, SLOT(WorkerFinished(QString const&)), Qt::QueuedConnection);
    connect(Worker, SIGNAL(Item(int, QString const&, QString const&, QString const&, QString const&)),
            this, SLOT(WorkerItem(int, QString const&, QString const&, QString const&, QString const&)),
            Qt::QueuedConnection);


    SBar = new QStatusBar(this);
    SBar->setSizeGripEnabled(false);
    char const * art_cols [] = { "Artist", "Disambiguation", 0 };
    Ctrls.Artists = new CMultiCombo(this, 2, art_cols);
    Ctrls.Artists->setEditable(true);
    connect(Ctrls.Artists, SIGNAL(editTextChanged(QString)), this, SLOT(artist_edit(QString)));

    char const * alb_cols [] = { "Album", "Medium", "Release", 0};
    Ctrls.Albums = new CMultiCombo(this, 3, alb_cols);

    Ctrls.Tracks = new QTableWidget(this);
    Ctrls.Tracks->horizontalScrollBar()->setVisible(false);
    QIcon icon(":/icons/gear.png");

    Ctrls.GetArtists = new QPushButton(icon, "", this);
    Ctrls.GetArtists->setToolTip("Get artists");
    connect (Ctrls.GetArtists, SIGNAL(clicked()), this, SLOT(get_artists()));

    Ctrls.GetAlbums = new QPushButton(icon, "", this);
    Ctrls.GetAlbums->setToolTip("Get albums");
    connect (Ctrls.GetAlbums, SIGNAL(clicked()), this, SLOT(get_albums()));

    Ctrls.GetTracks = new QPushButton(icon, "", this);
    connect (Ctrls.GetTracks, SIGNAL(clicked()), this, SLOT(get_tracks()));

    Ctrls.AddTracks = new QPushButton(QIcon(":/icons/add.png"), "", this);
    Ctrls.AddTracks->setToolTip("Add fetched tracks to list");
    Ctrls.AddTracks->setCheckable(true);

    Ctrls.Assign = new QPushButton("Assign tracks to track editor", this);
    connect(Ctrls.Assign, SIGNAL(pressed()), this, SLOT(assign_tracks()));

    Ctrls.Tracks->setColumnCount(4);

    QStringList sl;
    for (tTrackColumn const * t = TrackColumn; t->Caption; ++t)
        sl.append(t->Caption);

    connect (Ctrls.Tracks, SIGNAL(cellDoubleClicked (int, int)),
             this, SLOT(track_cell(int, int)));

    Ctrls.Tracks->setHorizontalHeaderLabels(sl);
    Ctrls.Tracks->setAlternatingRowColors(true);
    Ctrls.Tracks->verticalHeader()->setVisible(false);

    QGridLayout * l = new QGridLayout;

    l->addWidget(new QLabel("Artist", this), 0, 0);
    l->addWidget(new QLabel("Album", this), 1, 0);
    l->addWidget(new QLabel("Tracks", this), 2, 0);

    l->addWidget(Ctrls.Artists, 0, 1);
    l->addWidget(Ctrls.Albums, 1, 1);
    l->addWidget(Ctrls.Tracks, 2, 1);

    l->addWidget(Ctrls.GetArtists, 0, 2);
    l->addWidget(Ctrls.GetAlbums, 1, 2);

    QVBoxLayout * vl = new QVBoxLayout;
    vl->addWidget(Ctrls.GetTracks);
    vl->addWidget(Ctrls.AddTracks);
    l->addLayout(vl, 2, 2);

    QHBoxLayout * hl = new QHBoxLayout;
    hl->addWidget(Ctrls.Assign);

    Ctrls.Options = new QGroupBox("Process track editor items", this);
    QHBoxLayout * hl2 = new QHBoxLayout;
    Ctrls.TrackEdit.Keep = new QRadioButton("keep", this);
    Ctrls.TrackEdit.Rebuild = new QRadioButton("rebuild", this);
    Ctrls.TrackEdit.RebuildDefaults = new QRadioButton("rebuild with defaults", this);
    Ctrls.TrackEdit.RebuildDefaults->setChecked(true);
    hl2->addWidget(Ctrls.TrackEdit.Keep);
    hl2->addWidget(Ctrls.TrackEdit.Rebuild);
    hl2->addWidget(Ctrls.TrackEdit.RebuildDefaults);
    Ctrls.Options->setLayout(hl2);
    hl->addWidget(Ctrls.Options);

    l->addLayout(hl, 3, 0, 1, 3);

    l->addWidget(SBar, 4, 0, 3, 3);

    setLayout(l);
    resize(QSize(500, 400));
    init();
    setWindowTitle(QString("Music Brainz - %1").arg(App->Gui()->windowTitle()));
}

void CMBrainzDlg::init()
{

    Ctrls.Artists->clear();
    Ctrls.Artists->setEditText(App->Settings.Project.GetArtist());

    Ctrls.Albums->clear();
    Ctrls.Albums->AddRow("", App->Settings.Project.GetAlbum());
    connect(Ctrls.Albums, SIGNAL(activated(int)), this, SLOT(album_change(int)));
    enable_butts();
}


CMBrainzDlg::~CMBrainzDlg()
{
}

void CMBrainzDlg::resizeEvent(QResizeEvent * e)
{
    int table_w = Ctrls.Tracks->geometry().width() - 4;
    if (Ctrls.Tracks->verticalScrollBar()->isVisible())
        table_w -= Ctrls.Tracks->verticalScrollBar()->width();

    int const char_w = fontMetrics().maxWidth();
    int const col_no_w = char_w * 2;
    int const col_side_w = col_no_w;
    int const col_dur_w = char_w * 4;

    Ctrls.Tracks->setColumnWidth(colNo, col_no_w);
    Ctrls.Tracks->setColumnWidth(colSide, col_side_w);
    Ctrls.Tracks->setColumnWidth(colDuration, col_dur_w);
    Ctrls.Tracks->setColumnWidth(colTitle, table_w - col_no_w - col_side_w - col_dur_w);

    QDialog::resizeEvent(e);
}

void CMBrainzDlg::show_message(QString const & _msg, int delay, bool _sticky)
{
    static bool sticky = false;
    QString msg;
    if (sticky)
        msg = SBar->currentMessage() + _msg;
    else
        msg = _msg;
    SBar->showMessage(msg, delay);
    sticky = _sticky;
}

void CMBrainzDlg::WorkerFinished(QString const &error)
{
    int idx;
    Worker->wait();
    Ctrls.Artists->Resize();
    Ctrls.Albums->Resize();

    if (!error.isEmpty())
    {
        show_message(QString("Error: %1").arg(error), 10000);
        setCursor(Qt::ArrowCursor);
    }
    else
    {
        SBar->clearMessage();
        if (Worker->GetAction() == CMBrainz::GetArtists)
        {
            Ctrls.Artists->Sort(0);
            setCursor(Qt::ArrowCursor);
            if (!Ctrls.Artists->ItemIsValid())
            {
                show_message(QString("No entries for '%1' found.").arg(Wanted));
            }
            else
            {
                idx = Ctrls.Artists->HasOneValue(Wanted, 0);
                if (idx != -1)
                {
                    Ctrls.Artists->setCurrentIndex(idx);
                    Ctrls.Albums->setEditText(App->Settings.Project.GetAlbum());
                    idx = Ctrls.Artists->HasOneValue(Wanted, 1);
                    if (idx == -1)
                    {
                        show_message(QString("Multiple entries for artist '%1' found. ")
                                    .arg(Wanted), 5000, true);

                    }
                    get_albums();
                }
                else
                {
                }
            }
        }
        else if (Worker->GetAction() == CMBrainz::GetAlbums)
        {
            Ctrls.Albums->Sort(0);
            setCursor(Qt::ArrowCursor);
            idx = Ctrls.Albums->HasOneValue(Wanted, 0);
            if (idx != -1)
            {
                Ctrls.Albums->setCurrentIndex(idx);
                get_tracks();
            }
        }
        else
        {
            fill_track_side(0);
            if (HintCount++ == 0)
                show_message("Double click into side column to assign sides.");
            setCursor(Qt::ArrowCursor);
            QResizeEvent re (size(), size());
            resizeEvent(&re);
        }
    }
    //highlight_box(Ctrls.Artists);
    //highlight_box(Ctrls.Albums);
    block_signals(Ctrls.Artists, false);
    enable_butts();
}


void CMBrainzDlg::WorkerItem(int /* action */, QString const & arg_1, QString const &arg_2,
                             QString const & arg_3, QString const& arg_4)
{
    switch (Worker->GetAction())
    {
        case CMBrainz::GetArtists: Ctrls.Artists->AddRow(arg_1, arg_2, arg_3); break;
        case CMBrainz::GetAlbums: Ctrls.Albums->AddRow(arg_1, arg_2, arg_3, arg_4); break;
        case CMBrainz::GetTracks: add_track(arg_1, arg_2);    break;
    }
}

void CMBrainzDlg::set_track_item(int row, int col, QString const& text)
{
    QTableWidgetItem * i = new QTableWidgetItem(text);
    tTrackColumn const & rec = TrackColumn[col];
    i->setTextAlignment(rec.Alignment);
    i->setFlags(rec.Flag);
    Ctrls.Tracks->setItem(row, col, i);
}

void CMBrainzDlg::add_track(QString const & name, QString const & duration)
{
    int count = Ctrls.Tracks->rowCount();
    Ctrls.Tracks->setRowCount(count + 1);
    set_track_item(count, 0, QString("%1").arg(count+1));
    set_track_item(count, 1, QString());
    set_track_item(count, 2, name);
    set_track_item(count, 3, duration);
    Ctrls.Tracks->setRowHeight(count, fontMetrics().height() +5);
}


void CMBrainzDlg::closeEvent(QCloseEvent *)
{
    if (Worker->isRunning())
    {
        show_message("Waiting for music brainz worker to end...", 0);
        QCoreApplication::processEvents();
    }
    Worker->wait();
    emit Closed(false);
}


void CMBrainzDlg::block_signals(QWidget * w, bool b)
{
    w->blockSignals(b);
}

void CMBrainzDlg::highlight(CMultiCombo * box, bool b)
{
    QFont f (font());
    f.setBold(b);
    box->setFont(f);
}

void CMBrainzDlg::enable_butts()
{
    Ctrls.Albums->setEnabled(Ctrls.Artists->ItemIsValid());
    Ctrls.GetAlbums->setEnabled(Ctrls.Artists->ItemIsValid());
    Ctrls.GetTracks->setEnabled(Ctrls.Albums->ItemIsValid());
    Ctrls.Assign->setEnabled(Ctrls.Tracks->rowCount() > 0);
    Ctrls.Options->setEnabled(Ctrls.Tracks->rowCount() > 0);
}

void CMBrainzDlg::highlight_box(CMultiCombo * box)
{
    highlight(box, box->ItemIsValid());
}

void CMBrainzDlg::artist_edit( QString const & )
{
    highlight(Ctrls.Artists, false);
    Ctrls.Albums->clear();
    Ctrls.Tracks->setRowCount(0);
    enable_butts();
}

void CMBrainzDlg::album_change(int)
{
    Ctrls.GetTracks->setFocus();
    if (!Ctrls.AddTracks->isChecked())
    {
        Ctrls.Tracks->setRowCount(0);
        enable_butts();
    }
}


void CMBrainzDlg::start_worker(int action, QString const & item)
{
    setCursor(Qt::WaitCursor);
    Worker->Start(action, item);
}

void CMBrainzDlg::get_artists()
{
    if (Worker->isRunning())
        QApplication::beep();
    else
    {
        Wanted = Ctrls.Artists->currentText();
        block_signals(Ctrls.Artists, true);
        show_message("Getting artists...", 0);
        Ctrls.Artists->clear();
        Ctrls.Artists->setEditText(Wanted);
        start_worker(CMBrainz::GetArtists, Wanted);
    }
}

void CMBrainzDlg::get_albums()
{
    if (Worker->isRunning() || !Ctrls.Artists->ItemIsValid())
        QApplication::beep();
    else
    {
        Wanted = App->Settings.Project.GetAlbum();
        QString s = Ctrls.Artists->ID();
        show_message("Getting albums...", 0);
        Ctrls.Albums->clear();
        start_worker(CMBrainz::GetAlbums, s);
    }
}

void CMBrainzDlg::get_tracks()
{
    if (Worker->isRunning() || !Ctrls.Albums->ItemIsValid())
        QApplication::beep();
    else
    {
        QString s = Ctrls.Albums->ID();
        if (!Ctrls.AddTracks->isChecked())
            Ctrls.Tracks->setRowCount(0);
        show_message("Getting tracks...", 0);
        QApplication::processEvents();
        start_worker(CMBrainz::GetTracks, s);
    }
}

void CMBrainzDlg::fill_track_side(int row)
{
    QChar side;
    if (row == 0 || Ctrls.Tracks->item(row - 1, colSide)->data(Qt::DisplayRole).isNull())
        side = '`';
    else
        side = Ctrls.Tracks->item(row - 1, colSide)->data(Qt::DisplayRole).toChar();

    side = QChar (side.cell() + 1);
    for (;row < Ctrls.Tracks->rowCount(); ++row)
    {
        Ctrls.Tracks->item(row, colSide)->setData(Qt::DisplayRole, side);
    }
}

void CMBrainzDlg::track_cell(int row, int column)
{
    if (column != 1) return;
    fill_track_side(row);
}


int CMBrainzDlg::track_count(int s) const
{
    int ret = 0;
    QChar side = CApp::SideToQChar(s);
    for (int r = 0; r < Ctrls.Tracks->rowCount(); ++r)
    {
        if (Ctrls.Tracks->item(r, colSide)->data(Qt::DisplayRole).toChar() == side)
            ++ ret;
    }
    return ret;
}

int CMBrainzDlg::side_count() const
{
    int ret = 0;
    QChar old_side;
    for (int row = 0; row < Ctrls.Tracks->rowCount(); ++row)
    {
        QChar curr_side = Ctrls.Tracks->item(row, colSide)->data(Qt::DisplayRole).toChar();
        if (old_side != curr_side)
        {
            old_side = curr_side;
            ++ ret;
        }
    }
    return ret;
}

void CMBrainzDlg::assign_tracks()
{
    CTEdit & tedit = * App->Gui()->TEdit();
    if (Ctrls.TrackEdit.Keep->isChecked())
    {
        if (side_count() != tedit.SideCount())
        {
            show_message("Can't assign tracks: side counts are different.", 5000);
            return;
        }
        for (int x = 0; x < tedit.SideCount(); ++x)
        {
            if (track_count(x) != tedit.TrackCount(x))
            {
                QString msg = QString("Can't assign tracks: track counts for side %1 "
                 "are different (%2 vs %3)").arg(CApp::SideToQChar(x))
                     .arg(track_count(x)).arg(tedit.TrackCount(x));
                show_message(msg);
                return;
            }
        }
    }
    else
    {
        tedit.Clear();
        QChar old_side;
        int nr = 0;
        for (int row = 0; row < Ctrls.Tracks->rowCount(); ++row)
        {

            QChar curr_side = Ctrls.Tracks->item(row, colSide)->data(Qt::DisplayRole).toChar();
            if (old_side != curr_side)
            {
                tedit.AddSide();
                old_side = curr_side;
                nr = 0;
            }
            QString name = Ctrls.Tracks->item(row, colTitle)->data(Qt::DisplayRole).toString();

            tedit.SetData(row, CTList::colNo, QString("%1").arg(++nr));
            tedit.SetData(row, CTList::colSide, curr_side);
            tedit.SetData(row, CTList::colTitle, name);
        }
        if (Ctrls.TrackEdit.RebuildDefaults->isChecked())
        {
            for (int x = 0; x < tedit.SideCount(); ++x)
            {
                tedit.SetSide(x);
                tedit.SetDefaults();
            }
            tedit.SetSide(0);
        }
        tedit.ResizeGrid();
    }
}
#endif
