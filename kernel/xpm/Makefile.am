INCLUDES = -I.. -I../include

lib_LTLIBRARIES = libSQ_codec_xpm.la
libSQ_codec_xpm_la_SOURCES = fmt_codec_xpm.cpp fmt_codec_xpm_defs.h xpm_utils.h

libSQ_codec_xpm_la_LDFLAGS = -release 0.6.2

EXTRA_DIST = rgbmap

LIBS = 

install-data-am:
	$(INSTALL) --mode=0644 rgbmap $(DESTDIR)$(libdir)