#include "CApp.h"
#include "CGui.h"

#include <QMainWindow>
#include <QDebug>
#include <QString>
#include <QMessageBox>
#include <QDir>

#include "CDetector.h"
#include "CNormalizer.h"
#include "Excepts.h"

#include "CTEdit.h"
#include "CWEdit.h"
#include "CPTool.h"
#include "CRTool.h"
#include "CDetTool.h"
#include "CNormTool.h"

#include <fstream>
#include <QFile>
#include <QTextStream>
#include "CTList.h"
#include "CAudio.h"

#include "CAStream.h"
#include "CFile.h"
#include <QFileInfo>
#include <QByteArray>
#include <QObject>
#include <QEvent>
#include "../debug.h"
#include "CProjSet.h"
#include "CMeter.h"
#include "CMeters.h"
#include "CXRuns.h"
#include "CMp3Enc.h"
#include <QStatusBar>
#include <QKeyEvent>


extern CApp * App;

CApp::CApp(int & argc, char ** argv):
    QApplication(argc, argv),
    _Gui(0),
    _Play(0),
    _Rec(0),
    _Detector(0),
    _Normalizer(0),
    XRunDlg(false)

{

}


void CApp::Init() 
{
    if (App->Settings.Gui.FontSizes.Application() != 0.0)
    {
        QFont f ("Sans Serif");
        f.setPointSizeF(App->Settings.Gui.FontSizes.Application());
        setFont(f);
    }

    _Play = new CAStream (false, true, true);
    _Rec = new CAStream (true, true, true);
    _Detector = new CDetector;
    _Normalizer = new CNormalizer;
    _Gui = new CGui;
    change_uid();

    connect (Play()->GetAudio(), SIGNAL(AtBegin(bool)), Gui()->PTool(), SLOT(AtBegin(bool)), Qt::QueuedConnection);
    connect (Play()->GetAudio(), SIGNAL(AtEnd(bool)), Gui()->PTool(), SLOT(AtEnd(bool)), Qt::QueuedConnection);
    connect (Play()->GetAudio(), SIGNAL(Between(bool)), Gui()->PTool(), SLOT(Between(bool)), Qt::QueuedConnection);
    connect (Play()->GetFile(), SIGNAL(BufferReserve(int)), Gui()->PTool(), SLOT(Reserve(int)), Qt::QueuedConnection);
    connect (Rec()->GetFile(), SIGNAL(BufferReserve(int)), Gui()->RTool(), SLOT(Reserve(int)), Qt::QueuedConnection);
    connect (Rec()->GetFile(), SIGNAL(BufferXRun()), this, SLOT(rec_xrun()), Qt::QueuedConnection);
    Gui()->Requery();

    connect(Play()->GetAudio(), SIGNAL(XRun(int, bool)), Gui()->XRuns(), SLOT(Add(int, bool)),Qt::QueuedConnection);
    connect(Rec()->GetAudio(), SIGNAL(XRun(int, bool)), Gui()->XRuns(), SLOT(Add(int, bool)),Qt::QueuedConnection);



}

void CApp::stream_callback(CAStream const * str)
{
    if (str == App->Play())
        App->Gui()->PTool()->SetData();
    else
        App->Gui()->RTool()->SetData();
}

CApp::~CApp()
{
    Rec()->End();
    Play()->End();
    delete Play();
    delete Rec();
    delete Gui();
    delete Normalizer();
    delete Detector();
}


void CApp::USleep(int useconds)
{
    timespec spec;
    spec.tv_sec = 0;
    spec.tv_nsec = useconds * 1000;
    nanosleep(&spec, NULL);

}

QString CApp::CleanString(QString const &in, bool keep_pathseps, bool keep_articles)
{
    struct tRepl
    {
        char const *Search;
        char const *Replace;
    };
    tRepl char_repl [] = // FIXME Andere Encodings?
    {
       { "�",  "ae" },
       { "�",  "ss" },
       { "�",  "oe" },
       { "�",  "ue" },
       { "�",  "e"  },
       { "�",  "i"  },
       { "�",  "c"  },
       { "�",  "o"  },
       { "�",  "u"  },
       { "�",  "a"  },
       { "�",  "y"  },
       { "&",  "and"},
       { 0x0,  0x0  }
    };
    tRepl pre_repl [] =
    {
        { "der ",  "" },
        { "die ",  "" },
        { "das ",  "" },
        { "the ",  "" },
        { "les ",  "" },
        { "thee ", "" },
        { "los ",  "" },
        { 0x0,     0x0}
    };
    QString t(in.toLower());

    for (tRepl const * entry = pre_repl; !keep_articles && entry->Search; ++entry)
    {
        if (t.indexOf(entry->Search, 0, Qt::CaseInsensitive) == 0)
        {
            t = t.replace(entry->Search, entry->Replace, Qt::CaseInsensitive);
            break;
        }
    }

    for (tRepl const * entry = char_repl; entry->Search; ++entry)
        t.replace(entry->Search, entry->Replace, Qt::CaseInsensitive);

    for (QString::iterator i = t.begin(); i != t.end(); ++i)
       if ((i->isPunct() || i->isSpace() || i->isSymbol()) &&
           (!keep_pathseps || (*i != '\\' && *i != '/')))
       {
            if (i == t.begin())
            {
                *i = QChar(' ');
                i = t.begin();
            }
            else
                *i = QChar('_');
       }
    return t.trimmed();
}


void CApp::change_uid()
{
    int euid = geteuid();
    int uid = getuid();
    if (euid == 0 && uid != 0)
    {
        qDebug() << "Dropping root privileges..";
        if (setuid(uid))
            throw EGeneral("Could not drop root privileges.");
        else
            qDebug() << "Ok";
    }
}

void   CApp::keyPressEvent (QKeyEvent * event)
{
}

bool CApp::CloseProject(bool ask_changes)
{
    if (!Gui()->CanClose())
         return false;
    if (Gui()->isWindowModified() && ask_changes)
    {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(Gui(), "stylus", "Do you want to save your changes?",
                        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
        {
            SaveProject();
        }
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    Gui()->TEdit()->UnSync();
    Rec()->End();
    Play()->End();
    Gui()->TEdit()->Clear();
    Normalizer()->Clear();
    Detector()->Clear();
    Settings.Project.Clear();
    Gui()->SetTitle();
    Gui()->SetSides();
    Gui()->Requery();
    return true;
}


bool CApp::OpenProject(QString const & f_name)
{
    if (f_name.isEmpty()) return false;
    if (!CloseProject(true))
        return true;
    CSettings::CProject & sp = Settings.Project;
    CSettings::CAudio  & sa = Settings.Audio;
    QFile file (f_name);
    QString key, val;
    Gui()->TEdit()->setUpdatesEnabled(false);
    if (!file.open(QFile::ReadOnly))
        return false;
    else
    {
        QFileInfo i(f_name);
        sp.SetPath(i.absolutePath() + '/');
        Gui()->TEdit()->Clear();
        QTextStream in (&file);
// FIXME        in.setEncoding(QStringConverter::Encoding::Utf8);
        int tok = 0;
        snd_pcm_format_t fmt = SND_PCM_FORMAT_S16_LE;
        int side_count = 0;
        int count = 0;
        while (in.status() == QTextStream::Ok)
        {
            in >> key;
            if (!key.isEmpty())
            {
                val = in.readLine();
                switch (tok)
                {
                    case  0: sp.SetName(val.trimmed());  break;
                    case  1: sp.SetArtist(val.trimmed()); break;
                    case  2: sp.SetAlbum(val.trimmed()); break;
                    case  3: fmt = (snd_pcm_format_t) val.toInt(); break;
                    case  4: sa.Set(fmt, val.toInt()); break;
                    case  5:
                        side_count = val.toInt();
                        Gui()->TEdit()->AddSides(side_count);
                        Gui()->TEdit()->SetSide(0);
                        break;
                    case  6: Gui()->TEdit()->SetSideLengths(val.trimmed()); break;
                    case  7: sp.Target.Format = val.trimmed(); break;
                    case  8: sp.Target.Mp3Mode = val.trimmed(); break;
                    case  9: sp.Target.Mp3Rate = val.trimmed().toInt(); break;
                    case 10: sp.Target.OggQuality = val.trimmed().toDouble(); break;
                    case 11: sp.Target.FlacMode = val.trimmed().toInt(); break;
                    case 12: sp.Target.ConfigDir = val.trimmed(); break;
                    case 13:
                    case 14: 
                    case 15: break;
                    case 16: sp.Target.Genre = val.trimmed(); break;
                    case 17: sp.Target.Year = val.toInt(); break;
                    case 18: sp.Target.Comment = val.trimmed(); break;
                    case 19: sp.Target.UseRecChain = val.trimmed() == "yes"; break;
                    case 20: sp.Target.RecChain = val.trimmed(); break;
                    default:
#if 1
                        int row = count / CTList::ColCount;
                        int col = count % CTList::ColCount;
#else
                        int row = count / (CTList::ColCount );
                        int col = count % (CTList::ColCount );
                        if (col == 14)
                        {
                            ++count;
                            ++col;
                        }
#endif
                        Gui()->TEdit()->SetData(row, col, val.trimmed());
                        ++count;
                }
                ++tok;
            }
        }
        sp.SetFile(f_name);

        QString f = QString("Project info: %1 side(s), rate %2, format %3 (%4)").
                  arg(side_count).arg(sa.GetRate()).arg(snd_pcm_format_name(sa.GetFormat())).
                  arg(snd_pcm_format_description(sa.GetFormat()));
        Gui()->StatusMessage(f, 10);
        Gui()->SetSides();
        Gui()->SetTitle();
        Gui()->TEdit()->ResizeGrid();
        if (Gui()->TEdit()->SideCount())
        {
            Gui()->TEdit()->SetSide(0);
            Gui()->PTool()->SetSide(0);
            Gui()->RTool()->SetSide(0);
            Gui()->DetTool()->SetSide(0);
            Gui()->NormTool()->SetSide(0);
        }
        Gui()->Requery();
        Gui()->TEdit()->setUpdatesEnabled(true);
    }

    return true;
}


bool CApp::SetSettings(bool all_needed, bool read_settings)
{
    CProjSet settings (Gui(), !all_needed, read_settings);
    if (settings.exec() == QDialog::Accepted)
    {
        if (!read_settings && !CloseProject(true))
            return false;
        Settings.Project.SetLBasePath(settings.GetLBasePath());
        Settings.Project.SetRBasePath(settings.GetRBasePath());
        Settings.Project.SetName(settings.GetName());
        Settings.Project.SetArtist(settings.GetArtist());
        Settings.Project.SetAlbum(settings.GetAlbum());
        Settings.Project.SetPath(Settings.Project.GetLBasePath() + settings.GetPath());
        Settings.Audio.Set(settings.GetFormat(), settings.GetRate());

        Settings.Project.Target.Comment = settings.Target.GetComment();
        Settings.Project.Target.Format = settings.Target.GetFormat();
        Settings.Project.Target.Year = settings.Target.GetYear();
        Settings.Project.Target.Mp3Rate = settings.Target.GetMp3Rate();
        Settings.Project.Target.Mp3Mode = settings.Target.GetMp3Mode();
        Settings.Project.Target.OggQuality = settings.Target.GetOggQual();
        Settings.Project.Target.FlacMode = settings.Target.GetFlacMode();
        Settings.Project.Target.Genre = settings.Target.GetGenre();
        Settings.Project.Target.ConfigDir = settings.Target.GetConfigDir();
        Settings.Project.Target.UseRecChain = settings.Target.WithRecChain();
        Settings.Project.Target.RecChain = settings.Target.GetRecChain();
        return true;
    }
    return false;
}

void CApp::NewProject(QString const & src_dir)
{
    if (!src_dir.isEmpty())
    {
        if (!CloseProject(true))
            return;
        rebuild_project(src_dir);
    }
    else
    { 
        if (SetSettings(true, false))
        {
            InitStreams(true, true);
            Gui()->TEdit()->AddSides(2);
            Gui()->SetTitle();
            Gui()->Requery();
        }
    }
}

void CApp::rebuild_project(QString const & src_dir)
{
    try
    {
        CheckAccess(src_dir, true, false, true);
        QDir di (src_dir);
        di.cdUp();
        CheckAccess(di.path(), true, false, true);

        di.setPath(src_dir);
        di.setNameFilters(QStringList("???_*.mp3"));
        CMp3Enc enc;
        CBaseEnc::tInfo info;
        QStringList li = di.entryList();
        Gui()->TEdit()->setUpdatesEnabled(false);
        int sides = 0;
        for (QStringList::ConstIterator it = li.begin(); it != li.end(); ++it)
        {
            enc.ReadTags(di.path() + "/" + *it, info);
            Gui()->TEdit()->SetData(info);
            sides = max(sides, info.Side);
        }
        Gui()->TEdit()->AddSides(sides);
        Gui()->Requery();
        Settings.Project.SetAlbum(info.Tags.Album);
        Settings.Project.SetArtist(info.Tags.Artist);
        Settings.Project.SetName(info.Tags.Artist + " - " + info.Tags.Album);
        Settings.Project.Target.Genre = info.Tags.Genre;
        Settings.Project.Target.Year = QString(info.Tags.Year).toInt();
        Settings.Project.Target.Comment = info.Tags.Comment;
        Settings.Project.Target.RecChain = info.Tags.RecChain;
        Settings.Project.Target.Mp3Rate = 160;
        Settings.Project.Target.Mp3Mode = "vbr";
        Settings.Audio.Set(SND_PCM_FORMAT_S32_LE, 48000);

        SetSettings(true, true);
        Gui()->Requery();
    }
    catch (Except & e)
    {
        Gui()->StatusWarning(e.GetMessage(), 10);
    }
    Gui()->TEdit()->setUpdatesEnabled(true);
}

void CApp::InitStreams(bool play, bool rec)
{
    CAStream * master = Gui()->PTool()->GetSide() == Gui()->RTool()->GetSide ()  &&
                        Gui()->RTool()->Active() ? Rec() : 0;
    try
    {
        if (rec)
        {
            Rec()->Init(Gui()->RTool()->GetWavName(), 256, NULL, true,
                        Gui()->Meters()->GetRecord()->GetData());
            Gui()->RTool()->Reset();
            if (master || (!play && Play()->IsSlave()))
                play = true;
            Rec()->SetCallback(stream_callback);
        }

        if (play)
        {
            Play()->Init(Gui()->PTool()->GetWavName(), 128, master, false,
                        Gui()->Meters()->GetPlay()->GetData());
            Play()->Pause();
            Gui()->PTool()->Reset();
            Play()->SetCallback(stream_callback);
        }
    }
    catch (Except & e)
    {
        Gui()->StatusWarning(e.GetMessage(), 5);
    }
    Gui()->Requery();
}

bool CApp::CheckAccess(QString const & test, bool only_path, bool writable, bool throw_ex)
{
    QFileInfo fi;
    fi.setFile(test);
    fi.setFile(fi.absolutePath());
    if (!fi.exists())
    {
        if (throw_ex) throw EFileExists(fi.absoluteFilePath());
        return false;
    }
    if (writable && !fi.isWritable())
    {
        if (throw_ex) throw EFilePerms(fi.absoluteFilePath());
        return false;
    }

    if (only_path)
        return true;

    fi.setFile(test);
    if (!fi.exists())
    {
        if (throw_ex) throw EFileExists(fi.absoluteFilePath());
        return false;
    }
    if (!fi.isReadable() || (writable && !fi.isWritable()))
    {
        if (throw_ex) throw EFilePerms(fi.absoluteFilePath());
        return false;
    }
    return true;
}

void CApp::SaveProject()
{
    CSettings::CProject const  & sp = Settings.Project;
    CSettings::CAudio const & sa = Settings.Audio;
    if (sp.GetFile().isEmpty())
        return;

    CheckAccess(sp.GetFile(), true, true, true);
    QFile file (sp.GetFile());

    if (!file.open(QFile::WriteOnly | QFile::Truncate))
        throw EFileOpen(sp.GetFile());
    else
    {        
        QTextStream out (&file);
       // FIXME  out.setEncoding(QStringConverter::Encoding::Utf8);
        out << "name " << sp.GetName() << '\n' <<
               "artist " << sp.GetArtist() << '\n' <<
               "album " << sp.GetAlbum() << '\n' <<
               "format " << sa.GetFormat() << '\n' <<
               "rate " << sa.GetRate() << '\n' <<
               "sides " << Gui()->TEdit()->SideCount() << '\n' <<
               "side_lengths " << Gui()->TEdit()->GetSideLengths() << '\n' <<
               "target_format " << sp.Target.Format  << '\n' <<
               "target_mp3mode " << sp.Target.Mp3Mode << '\n' << 
               "target_mp3rate " << sp.Target.Mp3Rate << '\n' <<
               "target_oggquality " << sp.Target.OggQuality << '\n' <<
               "target_flacmode " << sp.Target.FlacMode << '\n' <<
               "target_configdir " << sp.Target.ConfigDir << '\n' <<
               "target_reserved_4 " << '\n' <<
               "target_reserved_5 " << '\n' <<
               "target_reserved_6 " << '\n' <<

               "target_genre " << sp.Target.Genre << '\n' <<
               "target_year " << sp.Target.Year << '\n' <<
               "target_comment " << sp.Target.Comment << '\n' << // TODO lf entfernen
               "target_userc ";

                if (sp.Target.UseRecChain)
                    out <<  "yes";
                else
                    out << "no";

               out << "\ntarget_recchain " << sp.Target.RecChain << '\n'; // TODO lf entfernen

        Gui()->TEdit()->WriteTracks(out);
        Gui()->SetTitle();
    }
}

void CApp::rec_xrun()
{
    Rec()->RecoverFromXRun();
    if (XRunDlg)
        return;    
    XRunDlg = true;
        QMessageBox::critical(Gui(), "stylus", "The recording buffer ran over. Your recorded "
                                               "audio is lost. Recording has been restarted.");
    XRunDlg = false;

}


bool CApp::notify( QObject *o, QEvent *e )
{
    try
    {
        return QApplication::notify(o, e);
    }
    catch (Except e)
    {
        qCritical() << e.GetMessage();
    }
    catch (...)
    {
        qDebug() << "... " << __PRETTY_FUNCTION__;
    }
    return false; // FIXME
}

