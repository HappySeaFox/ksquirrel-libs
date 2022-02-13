#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <qlabel.h>

#define SQ_FIO_NO_IMPLEMENT
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
