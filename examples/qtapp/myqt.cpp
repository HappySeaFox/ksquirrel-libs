#include <qlibrary.h>
#include <qapplication.h>
#include <qfile.h>

#include <qimage.h>

#include "myqt.h"

#include "ksquirrel-libs/fmt_utils.h"
#include "ksquirrel-libs/error.h"

MyQT::MyQT(QWidget *parent, const char *name) : QLabel(parent, name)
{
    setAlignment(Qt::AlignCenter);
}

MyQT::~MyQT()
{}

QPixmap MyQT::loadImage()
{
	QLibrary lib("/usr/lib/ksquirrel-libs/libSQ_codec_bmp.so");
	lib.load();

	if(!lib.isLoaded())
	{
	    qWarning("Can't load BMP library.");
	    qApp->quit();
	}

	int 		i = 0;
	fmt_info	finfo;
	RGBA		*image;
	int 		current = 0;

	codec_create = (fmt_codec_base*(*)())lib.resolve("codec_create");
	codec_destroy = (void (*)(fmt_codec_base*))lib.resolve("codec_destroy");

	if(!codec_create || !codec_destroy)
	{
	    qWarning("Library corrupted.");
	    lib.unload();
	    qApp->quit();
	}

	const char *s = "../w3.bmp";

	if(!QFile::exists(s))
	{
	    qWarning("Can't find example image.");
	    lib.unload();
	    qApp->quit();
	}

	codeK = codec_create();

	i = codeK->read_init(s);

	if(i != SQE_OK)
	{
	    codeK->read_close();
	    return QPixmap();
	}

	i = codeK->read_next();
	
	finfo = codeK->information();

	if(i != SQE_OK)
	{
	    codeK->read_close();
	    return QPixmap();
	}

	image = (RGBA*)calloc(finfo.image[current].w * finfo.image[current].h, sizeof(RGBA));

	if(!image)
	{
	    codeK->read_close();
	    return QPixmap();
	}

	memset(image, 255, finfo.image[current].w * finfo.image[current].h * sizeof(RGBA));
	
	RGBA *scan;

	for(int pass = 0;pass < finfo.image[current].passes;pass++)
	{
		codeK->read_next_pass();

		for(int j = 0;j < finfo.image[current].h;j++)
		{
			scan = image + j * finfo.image[current].w;
			codeK->read_scanline(scan);
		}
	}

	if(finfo.image[current].needflip)
	    fmt_utils::flipv((char*)image, finfo.image[current].w * sizeof(RGBA), finfo.image[current].h);

	codeK->read_close();

	QImage im((unsigned char*)image, finfo.image[current].w, finfo.image[current].h, 32, 0, 0, QImage::LittleEndian);
	
	return QPixmap(im.swapRGB());
}

void MyQT::bind()
{
	setPixmap(loadImage());	
}
