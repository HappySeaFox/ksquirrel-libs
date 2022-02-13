#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <qgl.h>

#include <csetjmp>

#include "ksquirrel-libs/fmt_types.h"
#include "ksquirrel-libs/fileio.h"
#include "ksquirrel-libs/fmt_codec_base.h"

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
		fmt_codec_base* (*codec_create)();
		void (*codec_destroy)(fmt_codec_base*);

		fmt_codec_base	*codeK;
			
};

#endif
