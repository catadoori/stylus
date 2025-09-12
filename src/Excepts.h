/***************************************************************************
 *   Copyright (C) 2006 by Harry   *
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

#ifndef EXCEPTS_H
#define EXCEPTS_H

#include <exception>
#include <QString>


#define ENSURE(x) if (!(x)) throw EAssert(__PRETTY_FUNCTION__, __LINE__);

class Except
{
    public:
        enum Contexts
        {
            Audio,
            File,
            FileExists,
            FilePerms,
            FileOpen,
            FileRead,
            FileWrite,
            General,
            Encoder,
            Internal,
            Assertion
        };
    public:
                        Except      ();
                        Except      (Contexts con, QString msg1, QString msg2);
        void            Log         (int indent_level ) const;
                        
        QString  GetMessage  () const;

    protected:
        Contexts  Con;
        QString   Msg1;
        QString   Msg2;
        
        static char const * MsgFmt [];
};

class EAudio: public Except
{
    public:
        EAudio(QString const & msg1, QString const & msg2 = ""):
            Except(Audio, msg1, msg2) {}
};

class EFile: public Except
{
    public:
        EFile(QString const & msg1, QString const & msg2 = ""):
            Except(File, msg1, msg2) {}
        EFile (Contexts con, QString const & msg1, QString const & msg2):
            Except (con, msg1, msg2) {}
};

class EFileExists: public EFile
{
    public:
        EFileExists(QString const & msg1, QString const & msg2 = ""):
            EFile(FileExists, msg1, msg2) {}
};

class EFilePerms: public EFile
{
    public:
        EFilePerms(QString const & msg1, QString const & msg2 = ""):
            EFile(FilePerms, msg1, msg2) {}
};

class EFileOpen: public EFile
{
    public:
        EFileOpen(QString const & msg1, QString const & msg2 = ""):
            EFile(FileOpen, msg1, msg2) {}
};

class EFileRead: public EFile
{
    public:
        EFileRead(QString const & msg1, QString const & msg2 = ""):
            EFile(FileRead, msg1, msg2) { }
};

class EFileWrite: public EFile
{
    public:
        EFileWrite(QString const & msg1, QString const & msg2 = ""):
            EFile(FileWrite, msg1, msg2) {}
};


class EGeneral: public Except
{
    public:
        EGeneral(QString const & msg1, QString const & msg2 = ""):
            Except(General, msg1, msg2) {}
};


class EEncoder: public Except
{
    public:
        EEncoder(QString const & msg1, QString const & msg2 = ""):
            Except(Encoder, msg1, msg2) {}
};


class EInternal: public Except
{
    public:
        EInternal(QString const & msg1, QString const & msg2 = "" ):
            Except(Internal, msg1, msg2) {}
};


class EAssert: public Except
{
    public:
        EAssert(char const * func, int line):
            Except(Assertion, func, QString("%1").arg(line)) {}
};

class EWarn
{
    public:    
        EWarn(QString const &msg, QString const & desc );
};    

#endif
