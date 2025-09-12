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

#include "CNormalizer.h"
#include "CSettings.h"
#include <math.h>
#include "CApp.h"
#include <QDebug>
#include <QFile>
#include <QDataStream>

extern CApp * App;

CNormalizer::CNormalizer()
{
   Clear();
}

void CNormalizer::Clear()
{
    memset((void*)Levels, 0, sizeof (Levels));
}


CNormalizer & CNormalizer::operator=(CNormalizer const &rhs)
{
    memcpy((void*)Levels, (void*)rhs.Levels, sizeof(Levels));
    return *this;
}

CNormalizer::~CNormalizer()
{
}


/*
Level   Count
      0     1108    // 1108 samples have level 0,
      1     9063    // 9063 saples have level 1 etc.
      2    20547
      3    35801
      .    .....
   4413     2881
   4414     2787
   4415     2845
      .   ......
  32765        0
  32766        0
      
Sum of Count == Number of samples in audio stream.
      
A applied gain of e. g. 6 db causes all samples with a value of USHRT_MAX / 6 db to be
overdriven.

*/


bool CNormalizer::HasData()
{
    for (int x = 0; x < SHRT_MAX; ++x)
    {
        if (Levels[x] != 0)
            return true;
    }
    return false;
}

double CNormalizer::ClippingAtGain(double db)
{
    int clipped = 0;
    double fac = exp10(db / 20);
    if (fac >= 1)
    {
        int bound  = qRound((SHRT_MAX -1) / fac);
        while (bound < SHRT_MAX)
            clipped += Levels[bound++];
    }
    else  // decreasing volume, take number of overdriven samples
        clipped = Levels[SHRT_MAX - 1];

    int total = 1;
    for (int x = 0; x < SHRT_MAX; ++x)
        total += Levels[x];
                
    double d = 1.0e6 * clipped / double(total);
    return d;
}

double CNormalizer::GainAtClipping(double target_share)
{
    int already_clipped = ClippingAtGain(-100.0);
    double ret = 0.0;
    if (target_share >= already_clipped)
    {
        while (target_share >= ClippingAtGain(ret) && ret < 40.0)
        {
            ret += 0.05;
        }
    }
    return ret;
}

QString CNormalizer::file_name(int side) const
{
    return App->Settings.Project.GetPath()  + "levels." +
           CApp::SideToQChar(side);

}

bool CNormalizer::LoadData(int side)
{
    Clear();
    QFile file (file_name(side));
    if (!file.open(QFile::ReadOnly))
        return -1;
    QDataStream in(&file);
    in.readRawData((char *)Levels, sizeof(Levels));
    return file.error() == 0;
    
}

bool CNormalizer::SaveData(int side)
{
    QFile file (file_name(side));
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
        return false;
    QDataStream out (&file);     
    out.writeBytes((const char*) Levels, sizeof(Levels));
    return file.error() == 0;
}
