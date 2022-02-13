SUBDIRS = aaa avs bmp cut fli hdr ico iff jbig koala lif mac mdl msp mtv pcx pix pnm psd pxr ras rawrgb sct sgi sun tga utah wal wbmp xbm xcur xpm xwd

EXTRA_DIST = fmt_defs.h error.h fileio.h fileio.cpp fmt_codec_base.h fmt_utils.h fmt_utils.cpp fmt_types.h generate link include/*.h

if SQ_HAVE_CAMERA
SUBDIRS += camera
endif

if SQ_HAVE_JPEG
SUBDIRS += jpeg
endif

if SQ_HAVE_PNG
SUBDIRS += png
endif

if SQ_HAVE_TIFF
SUBDIRS += tiff
endif

if SQ_HAVE_XWD
SUBDIRS += xwd
endif

if SQ_HAVE_WMF
SUBDIRS += wmf
endif

if SQ_HAVE_SVG
SUBDIRS += svg
endif

if SQ_HAVE_GIF
SUBDIRS += gif
endif

if SQ_HAVE_OPENEXR
SUBDIRS += openexr
endif

if SQ_HAVE_JPEG2000
SUBDIRS += jpeg2000
endif

if SQ_HAVE_TTF
SUBDIRS += ttf
endif

if SQ_HAVE_MNG
SUBDIRS += mng
endif
