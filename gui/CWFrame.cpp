#include "CWFrame.h"

#include <QDebug>

#include <QPainter>




CWFrame::CWFrame(QWidget * parent):
    QFrame(parent)
{
    setFrameStyle(QFrame::StyledPanel);
}


void CWFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(Qt::darkGreen);
    QFrame::paintEvent(event); // CPU!
}

