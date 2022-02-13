#include <qlibrary.h>
#include <qapplication.h>
#include <qfile.h>

#include <GL/gl.h>

#include "myqgl.h"

#include "ksquirrel-libs/fmt_utils.h"
#include "ksquirrel-libs/error.h"

MyQGL::MyQGL(QWidget *parent, const char *name) : QGLWidget(parent, name), bits(0), w(0), h(0)
{}

MyQGL::~MyQGL()
{}

void MyQGL::initializeGL()
{
	qglClearColor(white);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_FLAT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void MyQGL::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-width/2, width/2, -height/2, height/2, 0.1f, 100.0f);
	gluLookAt(0,0,1, 0,0,0, 0,1,0);
	glMatrixMode(GL_MODELVIEW);
}

void MyQGL::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, tex);

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(-w/2, h/2);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(w/2, h/2);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(w/2, -h/2);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(-w/2, -h/2);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void MyQGL::loadImage()
{
	// try to load BMP library
	QLibrary lib("/usr/lib/ksquirrel-libs/libSQ_codec_bmp.so");
	lib.load();

	// no such library
	if(!lib.isLoaded())
	{
	    qWarning("Can't load BMP library.");
	    qApp->quit();
	}
	
	int 		i = 0;
	fmt_info	finfo;
	RGBA		*image;
	int 		current = 0;

	// resolve neccessary functions
	codec_create = (fmt_codec_base*(*)())lib.resolve("codec_create");
	codec_destroy = (void (*)(fmt_codec_base*))lib.resolve("codec_destroy");

	// library corrupted!
	if(!codec_create || !codec_destroy)
	{
	    qWarning("Library corrupted.");
	    lib.unload();
	    qApp->quit();
	}

	const char *s = "../w3.bmp";
	
	// if the image doesn't exist
	if(!QFile::exists(s))
	{
	    qWarning("Can't find example image.");
	    lib.unload();
	    qApp->quit();
	}
	
	// OK, create decoder
	codeK = codec_create();

	// init library
	i = codeK->read_init(s);

	if(i != SQE_OK)
	{
	    codeK->read_close();
	    return;
	}

	// move to the next image
	i = codeK->read_next();
	
	// save retrieved information
	finfo = codeK->information();

	if(i != SQE_OK)
	{
	    codeK->read_close();
	    return;
	}

	image = (RGBA*)calloc(finfo.image[current].w * finfo.image[current].h, sizeof(RGBA));

	if(!image)
	{
	    codeK->read_close();
	    return;
	}

	memset(image, 255, finfo.image[current].w * finfo.image[current].h * sizeof(RGBA));
	
	RGBA *scan;

	// OK, let's decode the image line-by-line, pass-by-pass
	for(int pass = 0;pass < finfo.image[current].passes;pass++)
	{
		codeK->read_next_pass();

		for(int j = 0;j < finfo.image[current].h;j++)
		{
			scan = image + j * finfo.image[current].w;
			codeK->read_scanline(scan);
		}
	}

	// flip, if neccessary (BMP requires flipping)
	if(finfo.image[current].needflip)
	    fmt_utils::flipv((char*)image, finfo.image[current].w * sizeof(RGBA), finfo.image[current].h);

	// close library
	codeK->read_close();

	w = finfo.image[current].w;
	h = finfo.image[current].h;
	bits = image;
}

void MyQGL::bind()
{
	loadImage();

	if(!bits)
	    return;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// remember, that texture dimensions should be a power of two, e.g.
	// 32, 64, 256, ...
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, bits);

	updateGL();
}
