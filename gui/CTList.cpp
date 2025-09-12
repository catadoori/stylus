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




#include "CTList.h"

#include <QStandardItemModel>
#include <QComboBox>

#include <QEvent>
#include <QVBoxLayout>
#include <QDebug>
#include <QDateTime>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QDoubleSpinBox>
#include <QVariant>
#include "CAStream.h"
#include "CPTool.h"
#include "CTEdit.h"
#include "CProjSet.h"


#include "CApp.h"
#include "CGui.h"
#include <math.h>
#include <QHeaderView>
#include <QWheelEvent>
#include <math.h>


extern CApp * App;

// TODO Formatierung der Zeit - Spalten nach Detector kaputt

QComboBox * CTList::CDelegate::ACombo;


char const * CTList::ColCaps [ColCount] = 
{
    "No", "Stat", "Start", "End", "Duration", "Gain", "Title", "Artist",
    "Genre", "Year", "Format", "Quality", "Mode", "Comment", "Side", "Config"
};


CTList::CDelegate::CDelegate(QObject * o):
    QItemDelegate(o)
{

}


#define TENTH_SEC false

void CTList::CDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QVariant old = model->data(index);
    QComboBox * combo = dynamic_cast<QComboBox*>(editor);
    QDoubleSpinBox * dsp;
    QSpinBox * sb;
    int fmt = CProjSet::GetFormat(model->data(model->index(index.row(),colFormat)).toString());
    switch (index.column())
    {
        case colNo:
            break;
        case colFormat:
            fmt_changed(combo->currentIndex(), model, index);
            model->setData(index, combo->currentText());
            break;
        case colGenre:
             model->setData(index, combo->currentText()); break;
        case colRate:
            switch (fmt)
            {
                case CBaseEnc::ecVorbis:
                    dsp = dynamic_cast<QDoubleSpinBox*>(editor);
                    model->setData(index, dsp->value()); break;
                case CBaseEnc::ecFlac:
                    sb = dynamic_cast<QSpinBox*> (editor);
                    model->setData(index, sb->value()); break;
                default:
                    model->setData(index, combo->currentText()); break;
            }
            
            break;
        case colMode:
            mode_changed(combo->currentIndex(), model, index);
            model->setData(index, combo->currentText());
            break;
        case colVolume:
            model->setData(index, QString("%1").arg(((QDoubleSpinBox*)editor)->value(), 0, 'f', 1));
            break;
        case colConfigDir:
            model->setData(index, combo->currentText());
            break;
        case colArtist: 
        case colYear:
        default:
            QItemDelegate::setModelData(editor, model, index);
    }
    if (old != model->data(index))
        App->Gui()->setWindowModified(true);
}

void CTList::CDelegate::fmt_changed (int fmt, QAbstractItemModel * model,
                                     QModelIndex const & index) const
{
    CSettings::CProject::tTarget const & spt = App->Settings.Project.Target;
    switch (fmt)
    {
        case CBaseEnc::ecMp3:
            model->setData(model->index(index.row(), colRate), spt.Mp3Rate);
            model->setData(model->index(index.row(), colMode), spt.Mp3Mode);
            break;
        case CBaseEnc::ecVorbis:
            clear_cell(model, index, colMode);
            model->setData(model->index(index.row(), colRate), spt.OggQuality);
            break;
        case CBaseEnc::ecFlac:
            clear_cell(model, index, colMode);
            model->setData(model->index(index.row(), colRate), spt.FlacMode);
            break;
        default:
            clear_cell(model, index, colRate);
            clear_cell(model, index, colMode);
    }
}

void CTList::CDelegate::clear_cell(QAbstractItemModel * model,QModelIndex const & index, int col) const
{
    model->setData(index.model()->index(index.row(), col), QVariant());
}

void CTList::CDelegate::mode_changed(int mode, QAbstractItemModel * model,
                                     QModelIndex const & index) const
{
    CSettings::CProject::tTarget const & spt = App->Settings.Project.Target;
    int sp_mode = CProjSet::VbrMode(spt.Mp3Mode);
    int val;
    if (mode == sp_mode)
        val = spt.Mp3Rate;
    else if (mode == 2)
        val = 5;
    else
        val = 160;
    model->setData(model->index(index.row(), colRate), val);
}


QWidget * CTList::CDelegate::createEditor(QWidget *parent, 
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    QWidget * ret = NULL;
    QComboBox * cb;
    QDoubleSpinBox * sb;
    QSpinBox * isb;

    int fmt, mode;
    fmt = CProjSet::GetFormat(index.model()->data(index.model()->index(index.row(), colFormat)).toString());
    mode = CProjSet::LameVbrMode(index.model()->data(index.model()->index(index.row(), colMode)).toString());
    switch (index.column())
    {
        case colVolume:
            sb = new QDoubleSpinBox(parent);
            sb->setDecimals(1);
            sb->setSingleStep(0.1);
            ret = (QWidget*) sb;
            break;
        case colGenre:
            cb = new QComboBox(parent);
            CProjSet::LameFillGenre(cb);
            ret = (QWidget*) cb;
            break;
        case colFormat:
            cb = new QComboBox(parent);
            CProjSet::FillFormat(cb);
            ret = (QWidget*) cb;
            break;
        case colRate:
            if (fmt == CBaseEnc::ecMp3)
            {
                cb = new QComboBox(parent);
                if (mode == 2)
                    CProjSet::LameFillQuality(cb);
                else
                    CProjSet::LameFillBitrate(cb);
                ret = (QWidget*) cb;
            }
            else if (fmt == CBaseEnc::ecVorbis)
                ret = ((QWidget*)CProjSet::CreateOggSpinBox(parent));
            else if (fmt == CBaseEnc::ecFlac)
                 ret = ((QWidget*) CProjSet::CreateFlacBox(parent));                

            break;
        case colMode:
            if (fmt == CBaseEnc::ecMp3)
            {
                cb = new QComboBox(parent);
                CProjSet::LameFillVbrMode(cb);
                ret = (QWidget*) cb;
            }
            break;
        case colYear:
            isb = new QSpinBox(parent);
            isb->setMinimum(1950);
            isb->setMaximum(QDate::currentDate().year());
            ret = (QWidget*) isb;
            break;
        case colConfigDir:
            cb = CProjSet::CreateConfigBox(parent);
            ret = (QWidget*) cb;
            break;            

        case colNo:
        case colStatus:
        case colStart:
        case colEnd:  
        case colDuration:  ret = NULL; break;
        
        default: ret = QItemDelegate::createEditor(parent, option, index); break;
    }
    return ret;
}


CTList::CTList(QWidget * parent):
    QTableView(parent),
    Delegate(new CDelegate(this))
{
    setItemDelegate(Delegate);
    setAlternatingRowColors(true);

    SModel = new QStandardItemModel(0, ColCount, this);
    PModel = new QSortFilterProxyModel(this);
    PModel->setSourceModel(SModel);
    PModel->setDynamicSortFilter(true);

    if (App->Settings.Gui.FontSizes.TEdit() != 0.0)
    {
        QFont f = font();
        f.setPointSizeF(App->Settings.Gui.FontSizes.TEdit());
        setFont(f);
    }        

    setModel(PModel);
    FilterOnSide(0);
    PModel->setFilterKeyColumn(colSide);   

    for (int x = 0; x < ColCount; ++x)
    {
        PModel->setHeaderData(x, Qt::Horizontal, QObject::tr(ColCaps[x]), 0);
        resizeColumnToContents(x) ;
    }
    connect(this, SIGNAL(doubleClicked(QModelIndex const &)), this, SLOT(dbl_clicked(const QModelIndex &)));
    setColumnHidden(colSide, true);
    verticalHeader()->setVisible(false);
    //horizontalHeader()->setMovable(true);
    
}

void CTList::ResizeGrid(int first, int last)
{
    for (int x = first; x <= last; ++x)
    resizeColumnToContents(x);

    int rh = fontMetrics().height() + 4;
    for (int x = 0; x < PModel->rowCount(); ++x)
        setRowHeight(x, rh);
}


void CTList::dbl_clicked(const QModelIndex & index)
{
    if (index.column() == colStart || index.column() == colEnd)
    {
        QChar c = PModel->data(PModel->index(index.row(), colSide)).toString()[0];
        int target = CApp::QCharToSide(c);
        if (target != App->Gui()->PTool()->GetSide())
        {
            if (App->Gui()->TEdit()->SyncChecked())
                App->Gui()->PTool()->SetSide(target);
            else
            {
                sync_message("play");
                return;
            }
        }
        QString time;
        int col = index.column();
        time = PModel->data(PModel->index(index.row(), col)).toString();
        App->Play()->Cue(time, false);
        
        if (App->Play()->IsPaused())
        {
            try
            {
                App->Gui()->PTool()->Go();
            }
            catch (Except & e)
            {
                App->Gui()->StatusWarning(e.GetMessage(), 5);
                App->Gui()->PTool()->Reset();
            }
        }
    }
    else if (index.column() == colDuration)
    {
        double side_len = App->Gui()->TEdit()->GetSideLength();
        double end_time = index.row() < PModel->rowCount() -1 ? get_time(colStart, index.row()+1) : side_len;
        set_time(colEnd, index.row(), end_time, 0, side_len);
        set_duration(index.row());
    }
    else if (index.column() == colStatus)
    {
       PModel->setData(PModel->index(index.row(), colStatus), QString(), Qt::EditRole);
    }
}

void CTList::FilterOnSide(int side)
{
    QString q(CApp::SideToQChar(side));
    PModel->setFilterFixedString(q);
}

void CTList::filter_model(QSortFilterProxyModel& model, int side) const
{
    model.setSourceModel(SModel);
    QString q(CApp::SideToQChar(side));
    model.setFilterKeyColumn(colSide);
    model.setFilterFixedString(q);

}
 
int CTList::TrackCount(int side) const
{
    QSortFilterProxyModel model(NULL);
    filter_model(model, side);
    return model.rowCount();
}

int CTList::TrackCount() const
{
    return SModel->rowCount();
}

CTList::~CTList()
{

}


void CTList::col_widths(bool restore)
{
    for (int x = 0; x < ColCount; ++x)
    if (restore)
    {
        setColumnWidth(x, ColWidths[x]);
        setColumnHidden(colSide, true);
    }
    else
        ColWidths[x] = columnWidth(x);     
}

void CTList::enum_tracks()
{
    for (int x = 0; x < model()->rowCount(); ++x)
    {
        model()->setData(model()->index(x, colNo), x+1);
        model()->setData(model()->index(x, colNo), alignment(colNo), Qt::TextAlignmentRole);
    }
}

void CTList::Clear()
{
    SModel->removeRows(0, SModel->rowCount());
}

Qt::AlignmentFlag CTList::alignment(int col)
{
    switch (col)
    {
        case colArtist: case colTitle: case colGenre:
        case colComment: case colConfigDir:
            return Qt::AlignLeft;
            break;
        default:
            return Qt::AlignRight;
    }

}

void CTList::SetData(int row, int col, QString const & data, bool filtered)
{
     if (filtered)
    {
        if (PModel->rowCount() < row + 1)
            PModel->insertRow(PModel->rowCount());
        PModel->setData(PModel->index(row, col), data);
        PModel->setData(PModel->index(row, col), alignment(col), Qt::TextAlignmentRole);

    }
    else
    {
        if (SModel->rowCount() < row + 1)
            SModel->insertRow(SModel->rowCount());
        SModel->setData(SModel->index(row, col), data);
        SModel->setData(SModel->index(row, col), alignment(col), Qt::TextAlignmentRole);

    }
}

void CTList::SetSideData(int col, QString const & data, bool always)
{
    for (int r = 0; r < PModel->rowCount(); ++r)
    {
        QVariant v = PModel->data(PModel->index(r, col));
        if (always || v.isNull() || v.toString().length() == 0)
        {
            PModel->setData(PModel->index(r, col), data, Qt::EditRole);
            PModel->setData(PModel->index(r, col), alignment(col), Qt::TextAlignmentRole);
        }
    }
}

void CTList::FillSideData(bool all)
{
    CSettings::CProject  & ps = App->Settings.Project;
    CSettings::CProject::tTarget  & ts = App->Settings.Project.Target;
    SetSideData(colFormat, ts.Format, all);
    SetSideData(colGenre, ts.Genre, all);
    SetSideData(colArtist, ps.GetArtist(), all);
    SetSideData(colYear,  QString("%1").arg(ts.Year), all);
    SetSideData(colConfigDir, ts.ConfigDir, all);
    switch (CProjSet::GetFormat(ts.Format))
    {
        case CBaseEnc::ecMp3:
            SetSideData(colRate, QString("%1").arg(ts.Mp3Rate), all);
            SetSideData(colMode, ts.Mp3Mode, all);
            break;
        case CBaseEnc::ecVorbis:
            SetSideData(colRate, QString("%1").arg(ts.OggQuality), all);
            SetSideData(colMode, QString(), all);
            break;
        case CBaseEnc::ecFlac:
            SetSideData(colRate, QString("%1").arg(ts.FlacMode), all);
            SetSideData(colMode, QString(), all);
    }
    SetSideData(colComment, ts.Comment);
}

void CTList::SetSideData(int side, int col, QString const & data)
{
    QSortFilterProxyModel fmodel(NULL);
    filter_model(fmodel, side);
    for (int x = 0; x < fmodel.rowCount(); ++x)
    {
        fmodel.setData(fmodel.index(x, col), data, Qt::EditRole);
        fmodel.setData(fmodel.index(x, col), alignment(col), Qt::TextAlignmentRole);
    }
}

bool CTList::SetNextTime(int side, QString const & time)
{
    QSortFilterProxyModel fmodel(NULL);
    filter_model(fmodel, side);
    QString str;
    int col = colNo;
    for (int x = 0; x < fmodel.rowCount(); ++x)
    {
        str = fmodel.data(fmodel.index(x, colStart)).toString();
        if (str.isEmpty())
            col = colStart;
        else
        {
            str = fmodel.data(fmodel.index(x, colEnd)).toString();
            if (str.isEmpty())
                col = colEnd;
        }
        if (col != colNo)
        {
            fmodel.setData(fmodel.index(x, col), time, Qt::EditRole);
            fmodel.setData(fmodel.index(x, col), Qt::AlignRight, Qt::TextAlignmentRole);
            if (col == colEnd)
            {
                double diff = CAStream::TimeStringToSecs(fmodel.data(fmodel.index(x, colEnd)).toString()) -
                              CAStream::TimeStringToSecs(fmodel.data(fmodel.index(x, colStart)).toString());
                CAStream::SecsToTimeString(diff, str, TENTH_SEC);
                fmodel.setData(fmodel.index(x, colDuration), str, Qt::EditRole);
                fmodel.setData(fmodel.index(x, colDuration), Qt::AlignRight, Qt::TextAlignmentRole);

            }
            App->Gui()->setWindowModified(true);
            return x == 0 && col == colStart;
            
        }
    }
    return false;
}

void CTList::SetTimings(int side, int track, int start, int end)
{
    unsigned duration = end - start;
    QString s, e, d;
    if (start != -1 && end != -1)
    {
        CAStream::SegsToTimeString(start, s, TENTH_SEC);
        CAStream::SegsToTimeString(end, e, TENTH_SEC);
        CAStream::SegsToTimeString(duration, d, TENTH_SEC);
    }
    
    QSortFilterProxyModel fmodel(NULL);
    filter_model(fmodel, side);

    fmodel.setData(fmodel.index(track, colStart), s, Qt::EditRole);
    fmodel.setData(fmodel.index(track, colStart), Qt::AlignRight, Qt::TextAlignmentRole);
    fmodel.setData(fmodel.index(track, colEnd), e, Qt::EditRole);
    fmodel.setData(fmodel.index(track, colEnd), Qt::AlignRight, Qt::TextAlignmentRole);
    fmodel.setData(fmodel.index(track, colDuration), d, Qt::EditRole);
    fmodel.setData(fmodel.index(track, colDuration), Qt::AlignRight, Qt::TextAlignmentRole);
        
    ResizeGrid(colStart, colDuration);
}

void CTList::AddTrack(int side)
{
    col_widths(false);
    setModel(SModel);
    model()->insertRow(model()->rowCount());
    int nr = model()->rowCount();
    
    model()->setData(model()->index(nr-1, colSide), CApp::SideToQChar(side));
    
    setModel(PModel);
    enum_tracks();
    col_widths(true);
}

void CTList::RemTrack()
{
    model()->removeRow(currentIndex().row());
    enum_tracks();    
}

void CTList::TrackUp()
{
    int curr = currentIndex().row();
    if (curr >= 1)
        swap_tracks(curr, curr - 1);
}

void CTList::TrackDown()
{
    int curr = currentIndex().row();
    if (curr < model()->rowCount() -1)
        swap_tracks(curr, curr + 1);
}

void CTList::TrackLR(int current_side, int count, bool right)
{
    int old_side = current_side;
    if (right) ++current_side; else --current_side;

    if (current_side < 0)
        current_side = count -1;
    if (current_side == count)
        current_side = 0;        
    model()->setData(model()->index(currentIndex().row(), colSide), CApp::SideToQChar(current_side));


    FilterOnSide(current_side);
    enum_tracks();
    FilterOnSide(old_side);
    enum_tracks();
    ResizeGrid();
}

void CTList::swap_tracks(int from, int to) 
{
    for (int x = 1; x < ColCount; ++x)
    {
        QModelIndex const  & a = model()->index(from, x);
        QModelIndex const & b = model()->index(to, x);

        QVariant tmp = model()->data(a);
        model()->setData(a, model()->data(b));
        model()->setData(b, tmp);
    }
    setCurrentIndex(model()->index(to, 0));
}


QVariant CTList::GetData(int track, int col) const
{
    return SModel->data(SModel->index(track, col));
}



void CTList::GetData(int track, CBaseEnc::tInfo & info, bool stream_info, bool rec_chain_info) const
{
    info.Side = CApp::QCharToSide(GetData(track, colSide).toString()[0]);
    info.Track = GetData(track, colNo).toInt();

    info.In.Start = CAStream::TimeStringToSegments(GetData(track, colStart).toString());
    info.In.Length = CAStream::TimeStringToSegments(GetData(track, colDuration).toString());

    info.Out.Format = CProjSet::GetFormat(GetData(track, colFormat).toString());
    info.Out.Quality = GetData(track, colRate).toDouble();
    info.Out.ConfigDir = GetData(track, colConfigDir).toString();
    info.Out.EncQuality = 2;
    QString in;
    info.Out.Mp3Mode = CProjSet::LameVbrMode(GetData(track, colMode).toString());
    info.Out.Scale = exp10(GetData(track, colVolume).toDouble() / 20);

    info.Tags.Artist = GetData(track, colArtist).toString().toLatin1();
    info.Tags.Album = App->Settings.Project.GetAlbum().toLatin1();
    info.Tags.Title = GetData(track, colTitle).toString().toLatin1();
    info.Tags.Genre = GetData(track, colGenre).toString().toLatin1();
    info.Tags.Comment = GetData(track, colComment).toString().toLatin1();
    info.Tags.Year = GetData(track, CTList::colYear).toString().toLatin1();
    info.Tags.Track = QString("%1").arg(track + 1).toLatin1();
    
    int tn = (info.Side + 1) * 100 + info.Track;
    info.Out.LPath = App->Settings.Project.GetLBasePath();
    info.Out.RPath = App->Settings.Project.GetRBasePath();
    info.Out.SubPath = App->Settings.Project.GetSubPath();
    QString si;
    
    if (stream_info)
    {
        int qual = 0;
        switch (info.Out.Format)
        {
            case CBaseEnc::ecMp3:
                si = QString("_%1").arg(GetData(track, colMode).toString());
                qual = GetData(track, colRate).toInt();
                break;
            case CBaseEnc::ecVorbis:
                qual = qRound(GetData(track, colRate).toDouble() * 100);
                break;
            case CBaseEnc::ecFlac:
                qual = GetData(track, colRate).toInt();
                break;                
        }
        si += QString("_%1").arg(qual, 3, 10, QChar('0'));
    }
    if (rec_chain_info && !App->Settings.Project.Target.RecChain.isEmpty())
    {
        si += QString("_%1").arg(CApp::CleanString(App->Settings.Project.Target.RecChain));
    }
    info.Out.File = QString("%1_%2%3.%4").arg(tn, 3, 10, QChar('0'))
                                          .arg(CApp::CleanString(info.Tags.Title, false, true))
                                          .arg(si)
                                          .arg(CBaseEnc::Encoder[info.Out.Format].Ext);
}

void CTList::Test1()
{
    // FIXME PModel->clear();
}

void CTList::set_duration(int row)
{
    double end = get_time(colEnd, row);
    double start = get_time(colStart, row);
    if (end < start)
    {
        end  = start;
        set_time (colEnd, row, end);
    }
    double target = end - start;
    set_time (colDuration, row,  target);
}


double CTList::get_time(int col, int row) const
{
    return CAStream::TimeStringToSecs(PModel->data(PModel->index(row, col)).toString());
}

void CTList::set_time(int col, int row, double & target, double lower, double upper)
{
    QString str;
    if (target < lower) target = lower;
    if (target > upper) target = upper;
    CAStream::SecsToTimeString(target, str, TENTH_SEC);
    SetData(row, col, str, true);
    App->Gui()->setWindowModified(true);    
    
}

void CTList::trim_time(int col, int row, int delta, Qt::MouseButtons but, bool /* duration */)
{
    double target, old_target, upper;
    double maxl = App->Gui()->TEdit()->GetSideLength();
    delta /= 120;
    double fac = but & Qt::RightButton ? 10 : but & Qt::LeftButton ? 0.1 : 1;
    
    
    target = old_target = get_time(col == colDuration ? colEnd : col, row);
    target += (delta / 2.0) * fac;
    
    int const rows = PModel->rowCount();
    bool at_last = row < rows - 1 && get_time(colStart, row + 1) != 0.0;
    double const next_start = at_last ? get_time(colStart, row + 1) :  maxl;
    double const next_end = at_last ? get_time(colEnd, row + 1) : maxl;
    double const prev_end = row ? get_time(colEnd, row - 1) : 0.0;
    double const end = get_time(colEnd, row);
    if (col == colStart)
    {
        upper = next_start == 0.0 || next_start == maxl ? maxl : end;
        set_time (col, row, target, prev_end, upper);
        set_duration(row);
    }
    else if (col == colEnd)
    {
        set_time (col, row, target, get_time(colStart, row), next_end);
        set_duration(row);
        if (at_last)
        {
            set_time(colStart, row + 1, target, get_time(colStart, row), next_end);
            set_duration(row + 1);
        }                
    }
    else if (col == colDuration)
    {
        set_time (colEnd, row, target, get_time(colStart, row), next_start);
        set_duration(row);
    }
    if (IsSyncedPlay() && target != old_target)
    {
        App->Play()->Cue(target, false);
    }
}

void CTList::sync_message(char const * m) const
{
    App->Gui()->StatusMessage(
       QString("Can't %1. Track editor and play tool have "
               "different sides set (Use syncronize button).").arg(m), 5);
}

bool CTList::IsSyncedPlay() const
{
    return (PModel->data(PModel->index(0, colSide)).toString() ==
            App->Gui()->PTool()->GetSideName());
    
}

void CTList::ClearSelectedTimes()
{
    for (int x = 0; x < selectedIndexes().count(); ++x)
    {
        auto const i = selectedIndexes().at(x);
        switch(i.column())
        {
            case colStart:
            case colEnd:
            case colDuration:
                PModel->setData(PModel->index(i.row(), i.column()), QVariant());
        }
    }
}

void CTList::wheelEvent(QWheelEvent * event)
{
    
    if (selectedIndexes().count())
    {
        int col = columnAt(event->position().x());
        int row = rowAt(event->position().y());


        if (col == selectedIndexes().front().column() &&
            row == selectedIndexes().front().row())
        {
            switch (col)
            {
                case colStart:
                case colEnd:
                case colDuration:
                    trim_time(col, row, event->angleDelta().y(), event->buttons(), false);
                    break;
            }
        }
    }
}


#if 1
void CTList::keyPressEvent (QKeyEvent * event)
{
    if (event->key() == 32)
    {
        for (int x = 0; x < selectedIndexes().count(); ++x)
        {
            if  (selectedIndexes() [x].column() > colDuration)
            {
                QTableView::keyPressEvent(event);
                return;
            }
        }
        App->Gui()->KeyPressed(event->key(), this);
    }
    else
        QTableView::keyPressEvent(event);
}
#endif
