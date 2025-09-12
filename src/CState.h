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

#ifndef CSTATE_H
#define CSTATE_H

/**
    @author Harry <harry@actionpussyclub.de>
*/

template <class T>
class CState
{
    public:
        void        Add        (T s)             { State |= s;          }
        void        Remove     (T s)             { State &= ~s;         }
        bool        Has        (T s) const       { return State & s;    }
        void        Set        (T s)             { State = s;           }
        int         Get        ()    const       { return State;        }
        void        Clear      ()                { State = 0;           }
        bool        operator & (T s) const       { return Has(s);       }
                    CState     (): State(T(0))   {}

    private:
        int         State;
};

#endif
