#ifndef CWEDIT_H
#define CWEDIT_H


#include <QFrame>

class CWFrame;
class QAction;
class QToolBar;
class QScrollBar;





class CWEdit: public QFrame
{
    Q_OBJECT
    public:
                CWEdit          (QWidget * parent);
        void    Refresh         ();
        void    SetSource       (bool record);

    protected:

    private:
        QToolBar    *Tools;
        CWFrame     *WFrame;
        QScrollBar * TScroll; // Time Scroll

        struct
        {
            QAction * ZoomIn;
            QAction * ZoomOut;
            QAction * ZoomFit;
            QAction * ZoomAll;
            QAction * Act5;
            QAction * Act6;
            QAction * Act7;
        } Actions;
        

    private slots:
        void   act_1            ();
        void   act_2            ();
        void   act_3            ();
        void   act_4            ();
        void   side             (int s);
};

#endif

