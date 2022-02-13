#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <qgl.h>

#define SQ_FIO_NO_IMPLEMENT
#include "fmt_codec_base.h"

class MyQGL : public QGLWidget
{
    Q_OBJECT

	public:
		MyQGL(QWidget *parent = 0, const char *name = 0);
		~MyQGL();

	public slots:
		void bind();

	protected:
		void initializeGL();
		void paintGL();
		void resizeGL(int,int);
		void loadImage();

	private:
		void *bits;
		unsigned int tex;
		int w, h;
		fmt_codec_base* (*fmt_codec_create)();
		void (*fmt_codec_destroy)(fmt_codec_base*);
		
		fmt_codec_base	*codeK;
			
};

#endif
