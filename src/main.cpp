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

#include "CApp.h" 
#include "CGui.h"
#include "Excepts.h"

CApp * App; 
 
#include <QDebug>
#include <fstream>
#include <vector>
#include <exception>



int main(int argc, char ** argv)
{
    try
    { 
        App = new CApp(argc, argv);
        App->Init();

        App->exec();
        delete App;
    }
    catch (Except & e)
    {
        qCritical() << e.GetMessage();
    }
    catch (...)
    {
        qFatal(__PRETTY_FUNCTION__);
    }
}
