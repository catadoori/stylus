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


#include "CBaseEnc.h"

#include <QFile>
#include <Excepts.h>
#include "CApp.h"
#include <QDir>
#include <QStringList>

#include <QDebug>

extern CApp * App;

char const * CBaseEnc::CommentTags [] =
    {
        "ENCODER", "TITLE", "ALBUM", "ARTIST", "GENRE",
        "DATE", "TRACKNUMBER", "COMMENT", NULL
};



CBaseEnc::tEncoder const CBaseEnc::Encoder[] =
{
   { "Mp3", "mp3", 160,    3    },
   { "Ogg", "ogg",   0.7,  0.7  },
   { "Flac", "flac", 6,    6    },
   { 0,     0,       0,    0    }
    
};

CBaseEnc::CBaseEnc():
    FrameSize(0),
    Local(0),
    Remote(0)
{
}

QString const CBaseEnc::RecChainTag = "\nRecording chain: ";

CBaseEnc::~CBaseEnc()
{
    if (Local && Local->isOpen())
        Local->close();
    delete Local;
        
    if (Remote && Remote->isOpen())
        Remote->close();
    delete Remote;
}

void CBaseEnc::init_file(bool local, bool remote)
{
    if (local)
        Local = new QFile;
    if (remote)
        Remote = new QFile;
}

bool CBaseEnc::Init(tInfo const & info, bool local, bool remote)
{
    init_file(local, remote);    
    open_files(info);
    FrameSize =  App->Settings.Audio.GetSamplesPerSegment();
    return true;
}

int CBaseEnc::clipped_int(qreal in, int lower, int upper)
{
    qint64 ret = qRound(in);
    if (ret > upper)
        ret = upper;
    if (ret < lower)
        ret = lower;
   return ret;
}

void CBaseEnc::open_file(QFile * str, QString const & base_path, QString const & config_dir,
                         QString const & sub_path, QString const & file)
{
    if (str)
    {
        CApp::CheckAccess(base_path, true, true, true);
        QDir dir;
        QString path = base_path + config_dir + sub_path;
        if ((!dir.exists(path))  && ! dir.mkpath(path))
            throw EFileOpen(path);

        str->setFileName(path + file);
        if (str->open(QIODevice::WriteOnly) == false)
            throw EFileOpen(path + file);
    }
}

void CBaseEnc::open_files(tInfo const & info)
{
    open_file(Local, info.Out.LPath, info.Out.ConfigDir, info.Out.SubPath, info.Out.File);
    open_file(Remote, info.Out.RPath, info.Out.ConfigDir, info.Out.SubPath, info.Out.File);
}

void CBaseEnc::write(char * data, int len)
{
    if (Local)
    {
        if (Local->write(data, len) != len)
            throw EFileWrite(Local->fileName(), Local->errorString());
    }
    if (Remote)
    {
        if (Remote->write(data, len) != len)
            throw EFileWrite(Remote->fileName(), Remote->errorString());
    }
}

void CBaseEnc::seek(qint64 pos)
{
    if (Local)
    {
        if (Local->seek(pos) == false)
            throw EFileWrite(Local->fileName(), Local->errorString());
    }
    if (Remote)
    {
        if (Remote->seek(pos) == false)
            throw EFileWrite(Remote->fileName(), Remote->errorString());
    }
}

void CBaseEnc::tell(qint64* out) const
{
    QFile * file = Local ? Local : Remote ? Remote : 0;
    if (file)
        *out = file->pos();
    else
        *out = 0;
}

bool CBaseEnc::ReadTags(QString const & file, tInfo & output)
{
    QFileInfo fi (file);
    int ival = fi.fileName().left(3).toInt();
    ENSURE(ival > 100);
    output.Side = ival / 100;
    output.Track = ival % 100;
    return true;
}

void CBaseEnc::tInfo::tTags::ComposeComment()
{
    ComposedComment = Comment.trimmed();
    if (!RecChain.isEmpty())
    {
        ComposedComment += CBaseEnc::RecChainTag;
        ComposedComment += RecChain;
    }
}
                

void CBaseEnc::tInfo::tTags::SplitComment()
{
    QString input;
    input = Comment;
    QStringList slist;
    slist = input.split(CBaseEnc::RecChainTag);
    if (slist.size() == 2)
    {
        Comment = slist.at(0).toLatin1();
        RecChain = slist.at(1).toLatin1();
        RecChain.replace('\n', "");
    }
}


