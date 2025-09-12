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
#include "CMultiCombo.h"

#include "CMultiCombo.h"
#include <QStandardItemModel>
#include <QTableView>
#include <QHeaderView>
#include <QStringList>
#include <QScrollBar>



CMultiCombo::CMultiCombo(QWidget  * parent, int col_count, char const * col_headers[]):
     QComboBox(parent)
{
    Model = new QStandardItemModel(this);
    Model->setColumnCount(col_count+1);

    QStringList sl;
    for (char const ** c = col_headers; col_count && c != 0; ++c, --col_count)
        sl.append(*c);
    Model->setHorizontalHeaderLabels(sl);
    View = new QTableView(this);
    View->verticalHeader()->setVisible(false);

    setModel(Model);
    setView(View);
    Resize();
}


CMultiCombo::~CMultiCombo()
{


}

int CMultiCombo::id_col() const
{
    return Model->columnCount() -1;
}


QString CMultiCombo::Value(int row, int col) const
{
    return Model->data(Model->index(row, col)).toString();
}

QString CMultiCombo::CurrentValue(int col) const
{
    return Value(currentIndex(), col);
}

QString CMultiCombo::ID(int row) const
{
    if (row == -1)
        row = currentIndex();
    return Value(row, id_col());
}

int CMultiCombo::HasOneValue(QString const & search, int column)
{
    int ret = -1;
    for (int x = 0; x < Model->rowCount(); ++x)
    {
        if (Value(x, column).compare(search, Qt::CaseInsensitive) == 0)
        {
            return x;
        }            
    }
    return ret;
}


void CMultiCombo::Resize()
{
    int w =  width();
    if (View->verticalScrollBar()->isVisible())
        w -= View->verticalScrollBar()->width();
    int col_count = Model->columnCount() -1;
    
    for (int x = 0; x < col_count; ++x)
        View->setColumnWidth(x,  w / col_count );

    for (int x = 0; x < Model->rowCount(); ++x)
         View->setRowHeight(x, fontMetrics().height() +5);
}

void CMultiCombo::Sort(int col)
{
    Model->sort(col);
}

void CMultiCombo::resizeEvent (QResizeEvent * e)
{
    Resize();
    QComboBox::resizeEvent(e);

}

void CMultiCombo::SetData (int row, int col, QString const & text)
{
    Model->setItem(row, col, new QStandardItem(text));
}

void CMultiCombo::ClearIDs()
{
   for (int x = 0; x < Model->rowCount(); ++x)
       SetData(x, id_col(), QString());
}

bool CMultiCombo::ItemIsValid(int row) const
{
    if (row == -1)
        row = currentIndex();
    return row < Model->rowCount() && !Value(row, id_col()).isEmpty();
}

void CMultiCombo::AddRow (QString const & id, QString const & i0,
                          QString const & i1, QString const & i2, QString const &i3)
{
    int row = Model->rowCount();
    int cols = Model->columnCount();

    SetData(row, id_col(), id);
    View->setColumnHidden(id_col(), true);
    if (cols > 1)
        SetData(row, 0, i0);
    if (cols > 2)
        SetData(row, 1, i1);
    if (cols > 3)
        SetData(row, 2, i2);
    if (cols > 4)
        SetData(row, 3, i3);

}
