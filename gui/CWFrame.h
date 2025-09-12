#ifndef CWFRAME_H
#define CWFRAME_H


#include <QFrame>

class QPainter;

class CWFrame: public QFrame
{
    public:
                CWFrame     (QWidget * parent);
    protected:
        void    paintEvent  (QPaintEvent *event);

    private:
     

};

#endif
