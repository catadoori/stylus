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
#ifndef CMULTICOMBO_H
#define CMULTICOMBO_H

/**
	@author Harry <harry@actionpussyclub.de>
*/

#include <QComboBox>


class QTableView;
class QStandardItemModel;

/*
    QMultiCombo is a special kind of a QComboBox. If popped up, it displays multiple
    columums per row. Data is stored as a Table view. The last column of the view is
    never visible but associated to the "ID" data column in the model.
    
*/

class CMultiCombo: public QComboBox
{
    Q_OBJECT
    public:
                    CMultiCombo    (QWidget * parent, int col_count, char const * col_hdrs []);
                    // col_count is the count of the visible columns.
        virtual    ~CMultiCombo    ();

        QString     ID             (int row = -1)           const;
        QString     Value          (int row, int col = 0)   const;
        QString     CurrentValue   (int col)                const;
        void        SetData        (int row, int col, QString const & data);
        void        AddRow         (QString const & id, QString const & val_1 = QString(),
                                    QString const & val_2 = QString(),
                                    QString const & val_3 = QString(),
                                    QString const & val_4 = QString());

        bool        ItemIsValid    (int item = -1) const;
        int         HasOneValue    (QString const & search, int column);
        void        ClearIDs       ();
        void        Resize         ();
        void        Sort           (int col);

    private:
        QTableView         * View;
        QStandardItemModel * Model;
        
        void    resize      ();
        int     id_col      ()   const;

        void    resizeEvent (QResizeEvent * e);


};



#endif
