INCLUDES = -I../include

pkglib_LTLIBRARIES = libkls_djvu.la

libkls_djvu_la_SOURCES = fmt_codec_pnm.cpp fmt_codec_pnm_defs.h

libkls_djvu_la_LDFLAGS = ${SQ_RELEASE}

libkls_djvu_la_LIBADD = ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = -DDJVU_UI=\"${pkgdatadir}/libkls_djvu.so.ui\" -DCODEC_DJVU -DDJVU=\"${DJVU}\" -DCODEC_ANOTHER

EXTRA_DIST = libkls_djvu.so.ui

install-data-am:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	$(INSTALL) --mode=0644 libkls_djvu.so.ui $(DESTDIR)$(pkgdatadir)/libkls_djvu.so.ui
