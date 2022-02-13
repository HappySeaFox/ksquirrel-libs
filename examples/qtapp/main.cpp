#include "myqt.h"

#include <qapplication.h>
#include <qvbox.h>
#include <qpushbutton.h>

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    QVBox *v = new QVBox;
    QPushButton *b = new QPushButton("Show w3.bmp !", v);
    MyQT *qt = new MyQT(v);

    QVBox::connect(b, SIGNAL(clicked()), qt, SLOT(bind()));

    v->setStretchFactor(qt, 1);

    a.setMainWidget(v);
    v->resize(512, 256);
    v->show();

    return a.exec();
}
