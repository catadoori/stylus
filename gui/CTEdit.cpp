#include "../config.h"
#include "CTEdit.h"
#include "CTList.h"
#include <QVBoxLayout>
#include <QFrame>
#include <QTabBar>
#include <QAction>
#include <QToolButton>
#include <QToolBar>
#include <QIcon>
#include <QDebug>
#include <QTextStream>
#include <QMessageBox>
#include <QLabel>
#include <QMenu>

#include "CGui.h"
#include "CApp.h"

#include "CTracks.h"
#include "CDetector.h"
#include "CDetTool.h"
#include "CNormTool.h"
#include "CPTool.h"
#include "CSwapSides.h"
#include <QLineEdit>

extern CApp * App;

CTEdit::CTEdit (QWidget * parent):
        QWidget(parent),
        OldSide (0)
        
{
    TList = new CTList(this);
    TTools = new QToolBar(this);
    TTools->setOrientation(Qt::Vertical);
    TTools->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    // TTools->setMaximumWidth(30);
    QSize buts (16, 16);
    TTools->setIconSize(buts);


    STools = new QToolBar(this);
    STools->setOrientation(Qt::Horizontal);
    STools->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    STools->setIconSize(buts);

    Actions.AddSide = new QAction(QIcon(":/icons/tab_new.png"), "Add side", this);
    STools->addAction(Actions.AddSide);
    connect(Actions.AddSide, SIGNAL(triggered()), this, SLOT(add_side()));

    Actions.RemSide = new QAction(QIcon(":/icons/tab_remove.png"), "Remove side", this);
    STools->addAction(Actions.RemSide);
    connect(Actions.RemSide, SIGNAL(triggered()), this, SLOT(rem_side()));

    Actions.SwapSide = new QAction(QIcon(":/icons/tab_move.png"), "Swap tracks", this);
    STools->addAction(Actions.SwapSide);
    connect(Actions.SwapSide, SIGNAL(triggered()), this, SLOT(swap_side()));

    Actions.Sync = new QAction(QIcon(":/icons/connect.png"), "Syncronize detector and play to side selection", this);
    STools->addAction(Actions.Sync);
    Actions.Sync->setCheckable(true);
    connect(Actions.Sync, SIGNAL(triggered()), this, SLOT(synced()));

    Actions.MBrainz = new QAction(QIcon(":/icons/picard.png"), "Music brainz dialog", this);
    Actions.MBrainz->setCheckable(true);
    STools->addAction(Actions.MBrainz);
    connect(Actions.MBrainz, SIGNAL(toggled(bool)), parent, SLOT(ToggleMusicBrainz(bool)));

    Actions.AddTrack = new QAction(QIcon(":/icons/add.png"), "Add track", this);
    TTools->addAction(Actions.AddTrack);
    connect(Actions.AddTrack, SIGNAL(triggered()), this, SLOT(add_track()));

    Actions.RemTrack = new QAction(QIcon(":/icons/remove.png"), "Remove track", this);
    TTools->addAction(Actions.RemTrack);
    connect(Actions.RemTrack, SIGNAL(triggered()), this, SLOT(rem_track()));

    Actions.TrackUp = new QAction(QIcon(":/icons/up.png"), "Move track up", this);
    TTools->addAction(Actions.TrackUp);
    connect(Actions.TrackUp, SIGNAL(triggered()), this, SLOT(track_up()));

    Actions.TrackDown = new QAction(QIcon(":/icons/down.png"), "Move track down", this);
    TTools->addAction(Actions.TrackDown);
    connect(Actions.TrackDown, SIGNAL(triggered()), this, SLOT(track_down()));

    Actions.TrackLeft = new QAction(QIcon(":/icons/back.png"), "Move track to previous side", this);
    TTools->addAction(Actions.TrackLeft);
    connect(Actions.TrackLeft, SIGNAL(triggered()), this, SLOT(track_left()));

    Actions.TrackRight = new QAction(QIcon(":/icons/forward.png"), "Move track to next side", this);
    TTools->addAction(Actions.TrackRight);
    connect(Actions.TrackRight, SIGNAL(triggered()), this, SLOT(track_right()));

    Actions.Defaults = new QToolButton (this);
    Actions.DefaultsEmpty = new QAction (QIcon(":/icons/bookmark.png"), "Fill empty fields with defaults", this);
    Actions.DefaultsAll = new QAction (QIcon(":/icons/bookmark_add.png"), "Fill all fields with defaults", this);
    Actions.Defaults->setDefaultAction(Actions.DefaultsEmpty);
    connect (Actions.DefaultsEmpty, SIGNAL(triggered()), this, SLOT(defs_empty()));
    connect (Actions.DefaultsAll, SIGNAL(triggered()), this, SLOT(defs_all()));

    QMenu * m = new QMenu(this);
    m->addAction(Actions.DefaultsAll);
    m->addAction(Actions.DefaultsEmpty);
    Actions.Defaults->setMenu(m);
    TTools->addWidget(Actions.Defaults);

    Actions.ClearTime = new QToolButton(this);
    Actions.ClearTimeCells = new QAction(QIcon(":/icons/delete_timings.png"), "Clear selected time cells", this);
    Actions.ClearTimeAll = new QAction(QIcon(":/icons/delete_timings_all.png"), "Clear all time cells", this);
    Actions.ClearTime->setDefaultAction(Actions.ClearTimeCells);
    connect (Actions.ClearTimeCells, SIGNAL(triggered()), this, SLOT(clear_time_cell()));
    connect (Actions.ClearTimeAll, SIGNAL(triggered()), this, SLOT(clear_time_all()));
    m = new QMenu(this);
    m->addAction(Actions.ClearTimeAll);
    m->addAction(Actions.ClearTimeCells);
    Actions.ClearTime->setMenu(m);
    TTools->addWidget(Actions.ClearTime);

    Actions.ClearStatus = new QAction(QIcon(":/icons/deletecell.png"), "Clear stati", this);
    TTools->addAction(Actions.ClearStatus);
    connect (Actions.ClearStatus, SIGNAL(triggered()), this, SLOT(clear_stati()));

    Tab = new QTabBar(this);
    connect (Tab, SIGNAL(currentChanged(int )), this, SLOT (side_changed(int)));
    QBoxLayout * side = new QHBoxLayout;
    side->addWidget(STools);
    side->addWidget(Tab);


    QHBoxLayout * h = new QHBoxLayout;
    h->addWidget(TTools);
    h->addWidget(TList);
    QVBoxLayout * v = new QVBoxLayout;
#if 0 // FIXME 1
    v->setMargin(1);
    v->setSpacing(1);
    h->setMargin(1);
    h->setSpacing(1);

#endif
    v->insertLayout(-1, side);
    v->insertLayout(-1, h);

    setLayout(v);
  //  Frame->setLayout(v);

//    setWidget(Frame);
}

void CTEdit::UncheckMBrainz()
{
    Actions.MBrainz->setChecked(false);
}

void CTEdit::add_track()
{
    TList->AddTrack(Tab->currentIndex());
    TList->ResizeGrid();
    App->Gui()->DetTool()->Requery();
    App->Gui()->setWindowModified(true);
}

void CTEdit::SetSide(int side)
{
    Tab->setCurrentIndex(side);
    side_changed(side);
}

void CTEdit::UnSync()
{
    Actions.Sync->setChecked(false);
}

void CTEdit::Sync()
{
    Actions.Sync->setChecked(true);
    side_changed(1);
}

bool CTEdit::SyncChecked() const
{
    return Actions.Sync->isChecked();
}


int CTEdit::GetSide() const
{
    return Tab->currentIndex();
}

QChar CTEdit::GetSideChar() const
{
    return CApp::SideToQChar(GetSide());
}

void CTEdit::side_changed(int new_side)
{
    if(TList->IsEdited())
    {
        QApplication::beep();
        Tab->blockSignals(true);
        Tab->setCurrentIndex(OldSide);
        Tab->blockSignals(false);
        OldSide = Tab->currentIndex();
        return;
    }
    OldSide = Tab->currentIndex();
    TList->FilterOnSide(new_side);
    TList->ResizeGrid();
    if (Actions.Sync->isChecked())
    {
        if (App->Gui()->DetTool()->GetSide() != new_side)
            App->Gui()->DetTool()->SetSide(new_side);
        if (App->Gui()->NormTool()->GetSide() != new_side)
            App->Gui()->NormTool()->SetSide(new_side);
        if (App->Gui()->PTool()->GetSide() != new_side)            
            App->Gui()->PTool()->SetSide(new_side);
    }
}

void CTEdit::add_side(bool refresh)
{
    QString side = CApp::SideToQChar(Tab->count());
    QString cap = "Side ";
    cap += side;
    Tab->addTab(cap);
    if (refresh)
    {
        App->Gui()->Requery();
        App->Gui()->SetSides();
        App->Gui()->setWindowModified(true);
    }
}

void CTEdit::AddSides(int count)
{
    for (int x = 0; x < count - 1; ++x)
        add_side(false);
    add_side(true);
}


void CTEdit::rem_side()
{
    QString s;
    s = "Do you really want to delete "; s+= Tab->tabText(Tab->currentIndex()); s += "?";

    if(QMessageBox::question(this, "Stylus - track editor", s, QMessageBox::Yes | QMessageBox::No )==
            QMessageBox::Yes )
    {
        Tab->removeTab(Tab->currentIndex());
    }
    App->Gui()->setWindowModified(true);
    App->Gui()->SetSides(true);
    App->Gui()->Requery();
}

void CTEdit::swap_side()
{
    CSwapSides dialog (this, GetSide(), SideCount() );
    if (dialog.exec() == QDialog::Accepted)
    {
        int a = GetSide();
        int b = dialog.SelectedSide();
        int const tmp_side = 255;

        TList->SetSideData(a, CTList::colSide, CApp::SideToQChar(tmp_side));
        TList->SetSideData(b, CTList::colSide, CApp::SideToQChar(a));
        TList->SetSideData(tmp_side, CTList::colSide, CApp::SideToQChar(b));
#if 0        
        double len_a = Tab->tabData(a).toDouble();
        double len_b = Tab->tabData(b).toDouble();
        Tab->setTabData(a, QVariant(len_b));
        Tab->setTabData(b, QVariant(len_a));
#endif        
    }
    
    ResizeGrid();
}



void CTEdit::rem_track()
{
    App->Gui()->DetTool()->Requery();
    App->Gui()->setWindowModified(true);
    TList->RemTrack();
}


void CTEdit::track_up()
{
    App->Gui()->setWindowModified(true);
    TList->TrackUp();
}


void CTEdit::track_down()
{
    App->Gui()->setWindowModified(true);
    TList->TrackDown();
}

void CTEdit::track_left()
{
    App->Gui()->setWindowModified(true);
    TList->TrackLR (Tab->currentIndex(), Tab->count(), false);
}

void CTEdit::track_right()
{
    App->Gui()->setWindowModified(true);
    TList->TrackLR(Tab->currentIndex(), Tab->count(), true);
}


void CTEdit::defs_empty()
{
    Actions.Defaults->setDefaultAction(Actions.DefaultsEmpty);
    TList->FillSideData(false);    
}

void CTEdit::defs_all()
{
    Actions.Defaults->setDefaultAction(Actions.DefaultsAll);
    TList->FillSideData(true);
}



void CTEdit::clear_stati()
{
    TList->SetSideData(CTList::colStatus, QString(), true);
}

void CTEdit::clear_time_cell()
{
    Actions.ClearTime->setDefaultAction(Actions.ClearTimeCells);
    TList->ClearSelectedTimes();
    App->Gui()->setWindowModified(true);
}

void CTEdit::clear_time_all()
{
    Actions.ClearTime->setDefaultAction(Actions.ClearTimeAll);
    QString s;
    for (int col = CTList::colStart; col <= CTList::colDuration; ++col)
       SetSideData(GetSide(), col, s);
    App->Gui()->setWindowModified(true);
}

#ifdef CONFIG_STYLUS_DEV_HELPERS
void CTEdit::slot_1(int i)
{

}
#endif


int CTEdit::SideCount() const
{
    return Tab->count();
}

int CTEdit::TrackCount(int side) const
{
    return TList->TrackCount(side);
}

int CTEdit::TrackCount() const
{
    return TrackCount(Tab->currentIndex());
}

void CTEdit::SetSideData (int side, int col, QString const & data)
{
   TList->SetSideData (side, col, data);
}

void CTEdit::SetData(CBaseEnc::tInfo const & info)
{
    //int row = QString(info.Tags.Track).toInt() - 1;
    int row = TList->TrackCount();
    TList->SetData(row, CTList::colNo, QString("%1").arg(info.Track));
    TList->SetData(row, CTList::colTitle, info.Tags.Title);
    TList->SetData(row, CTList::colSide, QString(char('a' -1 + info.Side)));
    TList->SetData(row, CTList::colArtist, info.Tags.Artist);
    TList->SetData(row, CTList::colGenre, info.Tags.Genre);
    TList->SetData(row, CTList::colYear, info.Tags.Year);
    TList->SetData(row, CTList::colComment, info.Tags.Comment);
}

void CTEdit::synced()
{
    if (Actions.Sync->isChecked())
    {
        if (App->Gui()->PTool()->GetSide() != GetSide())
            App->Gui()->PTool()->SetSide(GetSide());
        if (App->Gui()->NormTool()->GetSide() != GetSide())
            App->Gui()->NormTool()->SetSide(GetSide());            
        if (App->Gui()->DetTool()->GetSide() != GetSide())
        App->Gui()->DetTool()->SetSide(GetSide());
    }
}

QString CTEdit::track_data(int track, int col) const
{
    QString s("tr_%1_%2 %3");
    QString val = TList->GetData(track, col ).toString();
    QString ret = s.arg(track, 2, 10, QChar('0' )).
                  arg(QString(CTList::ColCaps[col] ).toLower()).
                  arg(val);
    return ret;
}

void CTEdit::Clear()
{
    TList->Clear();
    while(Tab->count())
        Tab->removeTab(0);
}

void CTEdit::SetData(int row, int col, QString const & data)
{
    TList->SetData(row, col, data);
    App->Gui()->setWindowModified (true);
}

void CTEdit::SetVolumes(double db)
{
    TList->SetSideData(CTList::colVolume, QString("%1").arg(db, 0, 'f', 1));
}

void CTEdit::ResizeGrid()
{
    TList->ResizeGrid();
}


void CTEdit::AddSide()
{
    add_side();
}

void CTEdit::WriteTracks (QTextStream & str) const
{
    for (int t = 0; t < TList->TrackCount(); ++t)
    {
        for (int col = 0; col < CTList::ColCount; ++col)
            str << track_data(t, col)<< '\n';
    }
}

QString CTEdit::DataFile(int side) const
{
    return Tab->tabData(side).toString().trimmed();
}

void CTEdit::SetTimings(int side, unsigned set_nr)
{
    CDetector::tTrackSets const & sets = App->Detector()->TrackSets;
    if (set_nr > sets.size())
        return;

    CTracks const & tracks = sets[set_nr - 1];
    for (CTracks::tConstIter iter = tracks.Begin(); iter != tracks.End(); ++iter)
    {
        TList->SetTimings(side, distance(tracks.Begin(), iter), iter->Start, iter->End);
    }
}

void CTEdit::MarkPlayTime(bool with_pause)
{
    QString time;
    App->Play()->GetPos(time);
    if (!TList->SetNextTime(App->Gui()->PTool()->GetSide(), time) &&
        !with_pause)
    {
        TList->SetNextTime(App->Gui()->PTool()->GetSide(), time);
    }
}

void CTEdit::SetSideLength (int side, double secs)
{
    Tab->setTabData(side, QVariant(secs));
}

QString CTEdit::GetSideLengths() const
{
    QString ret;
    for (int x = 0; x < SideCount(); ++x)
    {
        ret = ret + QString("%1\t").arg(GetSideLength(x));
    }
    return ret;
}

void CTEdit::SetSideLengths(QString const & in)
{
    QStringList list;
    list = in.split("\t", Qt::SkipEmptyParts);
    for (int x = 0; x < list.size(); ++x)
    {
        SetSideLength(x, list.at(x).toDouble());
    }
}


double CTEdit::GetSideLength (int side) const
{
    return Tab->tabData(side).toDouble();
}

double CTEdit::GetSideLength() const
{
    return Tab->tabData(GetSide()).toDouble();
}

bool CTEdit::GetEncodeJob(CBaseEnc::tInfo & info, int side, bool stream_info, bool rec_chain_info,
                          bool only_selected)
{
    QString status;
    int total = TList->TrackCount();
    for (int x = 0; x < total; ++x)
    {
        TList->GetData(x, info, stream_info, rec_chain_info);
        status = TList->GetData(x, CTList::colStatus).toString();
        bool put_out;
        if (only_selected)
            put_out = side != -1 && info.Side == side && TList->selectionModel()->
                      isSelected(TList->selectionModel()->model()->index(info.Track - 1, 0));
        else
            put_out = (side == -1 || info.Side == side);

        put_out = put_out && info.In.Length != 0 && info.Out.Format != CBaseEnc::ecNone;
                             

        if ((status != "F" && status != "Err") && put_out && info.In.Length)
        {
            info.Offs = x;
            info.In.File = App->Gui()->PTool()->GetWavName(info.Side);
            info.In.Format = App->Settings.Audio.GetFormat();
            info.In.Rate = App->Settings.Audio.GetRate();
            if (App->Settings.Project.Target.UseRecChain)
                 info.Tags.RecChain = App->Settings.Project.Target.RecChain;
            else
                info.Tags.RecChain.clear();
             info.Tags.ComposeComment();
            return true;
        }

    }
    return false;
}


