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
#ifndef CNORMALIZER_H
#define CNORMALIZER_H

#include <QString>
#include <limits.h>


/**
	@author Harry <harry@actionpussyclub.de>
*/


class CNormalizer
{
    public:
                        CNormalizer     ();
                       ~CNormalizer     ();
        bool            SaveData        (int side);
        bool            LoadData        (int side);

        CNormalizer&    operator =      (CNormalizer const & rhs);

        double          ClippingAtGain  (double gain_in_db);
        double          GainAtClipping  (double clipped_share_in_ppm);

        void            Clear           ();
        void            CountLevel      (int level)  { ++ Levels[level]; }
        bool            HasData         ();
        
                       
    private:
        unsigned short  Levels [SHRT_MAX];
        QString         file_name       (int side) const;
};

#endif
