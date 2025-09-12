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
#include "CProjSet.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>
#include <QPushButton>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>

#include <QDebug>
#include <QIcon>
#include <QFileInfo>
#include <alsa/asoundlib.h>
#include "CApp.h"
#include "CSettings.h"
#include "CBaseEnc.h"
#include <lame/lame.h>
extern CApp * App;

char const * CProjSet::LameVbrModes [] = { "cbr", "abr", "vbr", NULL };
int const CProjSet::SampleRates [] = { 44100, 48000, 96000, 0 };

char const * CProjSet::ConfigDirs [] = {"", "abx/", "old/", "new/", "tmp/", "test/", NULL };



CProjSet::CProjSet(QWidget * parent, bool bpc, bool read):
    QDialog(parent),
    BasePathChange(bpc)
{
    setWindowTitle("Project settings");
    QGroupBox * project = new QGroupBox("Project settings");
    QGridLayout * grid = new QGridLayout;
    
    grid->addWidget(new QLabel("Name", this));
    grid->addWidget(new QLabel("Artist", this));
    grid->addWidget(new QLabel("Album", this));
    grid->addWidget(new QLabel("Work path", this));
    grid->addWidget(new QLabel("Local base path", this));
    grid->addWidget(new QLabel("Remote base path", this));
    grid->addWidget(new QLabel("Recording sample format", this));
    grid->addWidget(new QLabel("Recording sample rate", this));


    Name = new QLineEdit(this);
    Artist = new QLineEdit(this);
    Album = new QLineEdit(this);
    Path = new QLineEdit(this);
    LBasePath = new QLineEdit(App->Settings.Project.GetLBasePath(), this);
    RBasePath = new QLineEdit(App->Settings.Project.GetRBasePath(), this);

    Format = new QComboBox(this);
    Rate = new QComboBox(this);
    QIcon icon(":/icons/folder.png");
    QPushButton * plbut = new QPushButton(icon, "", this);
    connect(plbut, SIGNAL(pressed()), this, SLOT(ldir_select()));
    QPushButton * prbut = new QPushButton(icon, "", this);
    connect(prbut, SIGNAL(pressed()), this, SLOT(rdir_select()));

    Name->setEnabled(!bpc);
    Artist->setEnabled(!bpc);
    Album->setEnabled(!bpc);
    Path->setEnabled(!bpc);
    LBasePath->setEnabled(!bpc);
    plbut->setEnabled(!bpc);


    grid->addWidget(Name,      0, 1);
    grid->addWidget(Artist,    1, 1);
    grid->addWidget(Album,     2, 1);
    grid->addWidget(Path,      3, 1);
    grid->addWidget(LBasePath, 4, 1);
    grid->addWidget(RBasePath, 5, 1);
    grid->addWidget(plbut,     4, 2);
    grid->addWidget(prbut,     5, 2);
    grid->addWidget(Format,    6, 1);
    grid->addWidget(Rate,      7, 1);

    Format->addItem("16 bit", (int)SND_PCM_FORMAT_S16_LE);
    Format->addItem("24 bit", (int)SND_PCM_FORMAT_S32_LE);

    for (int const * rate = SampleRates; *rate; ++ rate)
        Rate->addItem(rate_string(*rate), *rate);
    Format->setCurrentIndex(0);
    Rate->setCurrentIndex(1);
    Rate->setEnabled(!bpc);
    Format->setEnabled(!bpc);
    project->setLayout(grid);

    // ------------------------------------------------------
    QGroupBox * target = new QGroupBox("Target defaults");
    grid = new QGridLayout;
    grid->addWidget(new QLabel("Format"));
    grid->addWidget(new QLabel("Mp3 mode"));
    Target.Mp3RateCaption = new QLabel();
    grid->addWidget(Target.Mp3RateCaption);
    grid->addWidget(new QLabel("Ogg quality"));
    grid->addWidget(new QLabel("Flac mode"));

    grid->addWidget(new QLabel("Genre"));
    grid->addWidget(new QLabel("Year"));
    grid->addWidget(new QLabel("Comment"));
    grid->addWidget(new QLabel("Extra output dir"));

    Target.Format = new QComboBox;
    FillFormat(Target.Format);
    Target.Mp3Mode = new QComboBox;
    LameFillVbrMode(Target.Mp3Mode);
    Target.Mp3Rate = new QComboBox;
    
    LameFillBitrate(Target.Mp3Rate);

    
    Target.OggQuality = CreateOggSpinBox(0);
    Target.FlacMode = CreateFlacBox(0);


    Target.Genre = new QComboBox;
    LameFillGenre(Target.Genre);
    Target.Year = new QSpinBox;
    Target.Year->setMinimum(1950);
    Target.Year->setMaximum(QDate::currentDate().year());
    Target.Comment = new QLineEdit;
    Target._WithRecChain = new QCheckBox("Rec chain");
    Target.RecChain = new QComboBox;
    Target.RecChain->setEditable(true);
    connect (Target._WithRecChain, SIGNAL(stateChanged(int)), this, SLOT(target_chain(int)));
    Target.RecChain->setEnabled(false);
    Target.RecChain->addItem("recording chain 1");
    Target.RecChain->addItem("recording chain 2");
    
    Target.ConfigDir = CreateConfigBox(0);
    

    grid->addWidget(Target.Format, 0, 1);
    grid->addWidget(Target.Mp3Mode, 1, 1);
    grid->addWidget(Target.Mp3Rate, 2, 1);

    grid->addWidget(Target.OggQuality, 3, 1);
    grid->addWidget(Target.FlacMode, 4, 1);

    grid->addWidget(Target.Genre, 5, 1);
    grid->addWidget(Target.Year, 6, 1);
    grid->addWidget(Target.Comment, 7, 1);
    grid->addWidget(Target.ConfigDir, 8, 1);    
    grid->addWidget(Target.RecChain, 9, 1);
    
    grid->addWidget(Target._WithRecChain, 9, 0);

    target->setLayout(grid);        

    QGridLayout * layout = new QGridLayout;
    layout->addWidget(project);
    layout->addWidget(target);

    QDialogButtonBox * butts;
    
    butts = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                 Qt::Horizontal, this);
    layout->addWidget(butts, layout->rowCount(), 0, 1, 2);
    
    setLayout(layout);
    Target.Mp3Mode->setCurrentIndex(2); // Vbr
    mp3_mode_changed(2);
    SetMp3Combos(Target.Mp3Mode, Target.Mp3Rate, CBaseEnc::Encoder[CBaseEnc::ecMp3].Vbr );

    if (read)
        read_current();

    connect(butts, SIGNAL(accepted()), this, SLOT(accept()));
    connect(butts, SIGNAL(rejected()), this, SLOT(reject()));

    connect(Artist, SIGNAL(textEdited(const QString &)), this, SLOT(edited()));
    connect(Album, SIGNAL(textEdited(const QString & )), this, SLOT(edited()));
    connect(Target.Mp3Mode, SIGNAL(currentIndexChanged(int)), this, SLOT(mp3_mode_changed(int)));


}



QString CProjSet::rate_string(int rate)
{
    return QString("%1 Hz").arg(rate);
}

void CProjSet::read_current()
{
    CSettings::CProject const  & sp = App->Settings.Project;
    Name->setText(sp.GetName());
    Artist->setText(sp.GetArtist());
    Album->setText(sp.GetAlbum());
    LBasePath->setText(sp.GetLBasePath());
    RBasePath->setText(sp.GetRBasePath());
    edited();

    Rate->setCurrentIndex(Rate->findData(App->Settings.Audio.GetRate()));
    Format->setCurrentIndex(Format->findData((int)App->Settings.Audio.GetFormat()));
    
    Target.Year->setValue(sp.Target.Year);
    Target.Format->setCurrentIndex(Target.Format->findData(sp.Target.Format, Qt::DisplayRole));
    Target.Comment->setText(sp.Target.Comment);
    Target.ConfigDir->setCurrentIndex(Target.ConfigDir->findData(sp.Target.ConfigDir, Qt::DisplayRole));
    Target._WithRecChain->setCheckState(sp.Target.UseRecChain ? Qt::Checked : Qt::Unchecked);
    Target.RecChain->setEditText(sp.Target.RecChain);
    Target.Genre->setCurrentIndex(Target.Genre->findData(sp.Target.Genre, Qt::DisplayRole));
    Target.Mp3Mode->setCurrentIndex(Target.Mp3Mode->findData(sp.Target.Mp3Mode, Qt::DisplayRole));
    SetMp3Combos(Target.Mp3Mode, Target.Mp3Rate, sp.Target.Mp3Rate);
    Target.OggQuality->setValue(sp.Target.OggQuality);
    Target.FlacMode->setValue(sp.Target.FlacMode);
}

void CProjSet::SetMp3Combos(QComboBox * mode, QComboBox * rate, int rate_data)
{
    if (mode->currentIndex() == 2) // vbr mode
        LameFillQuality(rate);
    else
        LameFillBitrate(rate);
    rate->setCurrentIndex(rate->findData(rate_data, Qt::DisplayRole));
}

QDoubleSpinBox * CProjSet::CreateOggSpinBox(QWidget * parent)
{
    QDoubleSpinBox * ret = new QDoubleSpinBox(parent);
    ret->setMinimum(0.0);
    ret->setMaximum(1.0);
    ret->setSingleStep(0.05);
    ret->setValue(CBaseEnc::Encoder[CBaseEnc::ecVorbis].Vbr);
    return ret;
}

QSpinBox * CProjSet::CreateFlacBox(QWidget * parent)
{
    QSpinBox * ret = new QSpinBox(parent);
    ret->setMinimum(0);
    ret->setMaximum(8);
    ret->setValue(CBaseEnc::Encoder[CBaseEnc::ecFlac].Vbr);
    return ret;
}

QComboBox * CProjSet::CreateConfigBox (QWidget * parent)
{
    QComboBox * ret = new QComboBox(parent);
    for (char const ** c = ConfigDirs; *c; ++c)
        ret->addItem(*c);
    return ret;        
}


void CProjSet::lame_genre_callb(int, char const * genre, void* _box)
{
    QComboBox * box = (QComboBox*)_box;
    box->addItem(genre);
}

void CProjSet::ldir_select()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select local base directory"),
                                                 LBasePath->text(),
                                                 QFileDialog::ShowDirsOnly |
                                                 QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty())
        LBasePath->setText(dir);
}

void CProjSet::rdir_select ()
{
   QString dir = QFileDialog::getExistingDirectory(this, tr("Select remote base directory"),
                                                 RBasePath->text(),
                                                 QFileDialog::ShowDirsOnly |
                                                 QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty())
        RBasePath->setText(dir);
}



void CProjSet::accept()
{
    if (BasePathChange)
    {
        QDialog::accept();
        return;
    }  
    if (LBasePath->text().isEmpty() || Name->text().isEmpty() ||
        Artist->text().isEmpty() || Album->text().isEmpty())
    {
        QMessageBox::information(this, "Project settings",
                                       "Please supply all needed information");
    }
    else
    {
        QDir dir (GetLBasePath() + GetPath());
        if (dir.exists())
            QMessageBox::information(this, "Path exists",
                   QString("Reqested path '%1' already exists, "
                           "please change settings.").arg(dir.path()));
        else
        {                                            
            if (!dir.mkpath(dir.path()))
                QMessageBox::critical(this, "Error", "Could not create directory");

            QDialog::accept();
        }
    }
}

void CProjSet::edited()
{
    _Album = Album->text();
    QString dir = Artist->text() + "/" + Album->text() + "/";
    dir = CApp::CleanString(dir, true);
    Path->setText(dir);
}



void CProjSet::mp3_mode_changed(int mode)
{
    if (mode == 2)
    {
        Target.Mp3RateCaption->setText("Mp3 quality");
        LameFillQuality(Target.Mp3Rate);
        
    }
    else
    {
        Target.Mp3RateCaption->setText("Mp3 rate");
        LameFillBitrate(Target.Mp3Rate);
    }
}


snd_pcm_format_t CProjSet::GetFormat() const
{
    int idx = Format->currentIndex();
    return (snd_pcm_format_t) Format->itemData(idx).value<int>();
}

unsigned CProjSet::GetRate() const
{
    int idx = Rate->currentIndex();
    return Rate->itemData(idx).value<int>();
}

void CProjSet::target_chain(int state)
{
    Target.RecChain->setEnabled(state == Qt::Checked);
}

unsigned CProjSet::tTarget::GetMp3Rate() const
{
    return Mp3Rate->currentText().toInt();
}

QString CProjSet::tTarget::GetConfigDir() const
{
    return ConfigDir->currentText();
}

QString CProjSet::tTarget::GetFormat() const
{
    return Format->currentText();
}

double CProjSet::tTarget::GetOggQual() const
{
    return OggQuality->value();
}

int CProjSet::tTarget::GetFlacMode() const
{
    return FlacMode->value();
}

QString CProjSet::tTarget::GetMp3Mode() const
{
    return Mp3Mode->currentText();
}


QString CProjSet::tTarget::GetGenre() const
{
    return Genre->currentText();
}

bool CProjSet::tTarget::WithRecChain() const
{
    return _WithRecChain->isChecked();
}

QString CProjSet::tTarget::GetRecChain() const
{
    return RecChain->currentText();
}

int CProjSet::tTarget::GetYear() const
{
    return Year->value();
}

void CProjSet::LameFillBitrate(QComboBox * box)
{
    box->clear();
    QChar const fill('0');
    for (int x = 0; x < 16; ++x)
    {
        int rate = -1;
        if (rate && rate != -1)
            box->addItem(QString("%1").arg(rate), rate);
    }
}

void CProjSet::LameFillQuality(QComboBox * box)
{
    box->clear();
    for (int x = 9; x >= 0; --x)
        box->addItem(QString("%1").arg(x));
}


void CProjSet::LameFillGenre(QComboBox * box)
{
    id3tag_genre_list(lame_genre_callb, (void*)box);
}

void CProjSet::LameFillVbrMode(QComboBox * box)
{
    for (char const ** x = LameVbrModes; *x; ++x)
        box->addItem(*x);
}

void CProjSet::FillFormat(QComboBox * box)
{
    box->clear();
    for (CBaseEnc::tEncoder const * e = CBaseEnc::Encoder; e->Name; ++e)
        box->addItem(e->Name);
}

vbr_mode CProjSet::LameVbrMode(QString const & s)
{
    vbr_mode ret;
    int x = VbrMode(s);
    if (x == 2)
        ret = vbr_rh;
    else if (x == 1)
        ret = vbr_abr;
    else
        ret = vbr_off;
    return ret;
}

int CProjSet::VbrMode(QString const & s)
{
    int x = 0;
    for (char const ** str = LameVbrModes; *str; ++str, ++x)
        if (s.compare(*str, Qt::CaseInsensitive) == 0)
            break;
    return x;            

}

int CProjSet::GetFormat(QString const & in)
{
    int x = 0;
    for (CBaseEnc::tEncoder const * e = CBaseEnc::Encoder; e->Name; ++e, ++x)
        if (in.compare (e->Name, Qt::CaseInsensitive) == 0)
            break;
    return x;
}

        
CProjSet::~CProjSet()
{
}
