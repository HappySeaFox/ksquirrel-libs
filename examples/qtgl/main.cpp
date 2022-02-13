#include "myqgl.h"

#include <qapplication.h>
#include <qvbox.h>
#include <qpushbutton.h>

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    if(!QGLFormat::hasOpenGL())
    {
	qWarning( "This system has no OpenGL support. Exiting." );
	return -1;
    }

    QVBox *v = new QVBox;
    QPushButton *b = new QPushButton("Show w3.bmp !", v);
    MyQGL *gl = new MyQGL(v);

    QVBox::connect(b, SIGNAL(clicked()), gl, SLOT(bind()));

    v->setStretchFactor(gl, 1);

    a.setMainWidget(v);
    v->resize(512, 256);
    v->show();

    return a.exec();
}
