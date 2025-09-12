#include "CWEdit.h"
#include <QDebug>
#include <QPainter>
#include <QFrame>
#include <QToolBar>
#include <QAction>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QComboBox>
#include "CAStream.h"
#include "CFile.h"
#include "CGui.h"
#include "CPTool.h"

#include "CWFrame.h"


#include "CApp.h"


#include "CTracks.h"
#include "CDetector.h" // FIXME temporär




CWEdit::CWEdit(QWidget * parent):
    QFrame(parent)
{
    WFrame = new CWFrame(this);
    Tools = new QToolBar(this);
    Tools->setOrientation(Qt::Horizontal);

    TScroll = new QScrollBar(Qt::Horizontal,this);
    

    
    Tools->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    Tools->setIconSize(QSize(19, 19));


    Actions.ZoomIn = new QAction(QIcon(":/icons/viewmag+.png"), "zoom in", this);
    Tools->addAction(Actions.ZoomIn);
    
    Actions.ZoomOut = new QAction(QIcon(":/icons/viewmag-.png"), "zoom out", this);
    Tools->addAction(Actions.ZoomOut);
    Actions.ZoomFit = new QAction(QIcon(":/icons/viewmagfit.png"), "zoom fit", this);
    Tools->addAction(Actions.ZoomFit);
    Actions.ZoomAll = new QAction(QIcon(":/icons/viewmag1.png"), "zoom all", this);
    Tools->addAction(Actions.ZoomAll);
    Tools->addSeparator();
    
    Actions.Act6 = new QAction("A6", this);
    Tools->addAction(Actions.Act6);
    Actions.Act7 = new QAction("A7", this);
    Tools->addAction(Actions.Act7);

    connect(Actions.Act6, SIGNAL(triggered()), this, SLOT(act_4()));


    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->setSpacing(1);
 // FIXME   vbox->setMargin(1);
    vbox->addWidget(Tools);
    vbox->addWidget(WFrame);
    vbox->addWidget(TScroll);

//    connect (Actions.Source, SIGNAL(toggled(bool)), App, SLOT(SetDetector(bool)));

    setLayout(vbox);
}


void CWEdit::side(int )
{

}

void CWEdit::act_1()
{
    qDebug() << __PRETTY_FUNCTION__;
    WFrame->update();
    
}

void CWEdit::act_2()
{
        

}


void CWEdit::act_3()
{

}


void CWEdit::act_4()
{
#if 0    
    App->Detector()->LocateTracks();
    App->Tracks.Dump();
    App->Detector()->DumpData();
#endif    
}



