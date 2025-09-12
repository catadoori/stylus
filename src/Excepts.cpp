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


#include "Excepts.h"
// #include <iostream>
#include <QDebug>

char const * Except::MsgFmt [] =
{
    "audio: %1",
    "file: %1",
    "file '%1' doesn't exist",
    "insufficient permissions for file or path '%1'",
    "could not open file '%1'",
    "could not read file '%1'",
    "could not write file '%1'",
    "%1",
    "encoder: %1",
    "internal error: %1",
    "assertion %1 failed",
    NULL
};


EWarn::EWarn(QString const &msg, QString const &desc)
{
    qDebug() << "Waring: " << msg << ": " << desc;
}



Except::Except(Contexts con, QString msg1,  QString msg2):
   Con(con),    
    Msg1(msg1),
    Msg2(msg2)
{
}

QString Except::GetMessage() const
{
    QString ret;
    ret = QString(MsgFmt[Con]).arg(Msg1);
    if (!Msg2.isEmpty())
        ret += QString(" (%1)").arg(Msg2);
    
    return ret;
}

void Except::Log(int indent) const
{
    QString s;
    while (indent--)
        s += "    ";

    qDebug() << s  << "Exception: " << GetMessage();
    
}
