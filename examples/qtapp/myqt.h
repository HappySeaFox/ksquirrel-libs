#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <qlabel.h>

#include <csetjmp>

#include "fmt_types.h"
#include "fileio.h"
#include "fmt_codec_base.h"

class MyQT : public QLabel
{
    Q_OBJECT

	public:
		MyQT(QWidget *parent = 0, const char *name = 0);
		~MyQT();
		
		QPixmap loadImage();

	public slots:
		void bind();

	private:
		fmt_codec_base* (*fmt_codec_create)();
		void (*fmt_codec_destroy)(fmt_codec_base*);
		
		fmt_codec_base	*codeK;
			
};

#endif
