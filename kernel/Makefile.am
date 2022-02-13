SUBDIRS = aaa avs bmp cut fli gif hdr ico iff jpeg2000 koala lif mac mdl msp mtv openexr pcx pix pnm psd pxr ras rawrgb sct sgi sun tga utah wal wbmp xbm xcur xpm xwd

EXTRA_DIST = fmt_defs.h error.h fileio.h fileio.cpp fmt_codec_base.h fmt_utils.h fmt_utils.cpp fmt_filters.h fmt_filters.cpp fmt_types.h generate link fmt_filters_ChangeLog fmt_filters_VERSION fmt_filters_README genfdist

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
