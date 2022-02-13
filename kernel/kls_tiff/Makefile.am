INCLUDES = -I../include

pkglib_LTLIBRARIES = libkls_tiff.la

libkls_tiff_la_SOURCES = fmt_codec_tiff.cpp fmt_codec_tiff_defs.h

libkls_tiff_la_LDFLAGS = ${SQ_RELEASE}

libkls_tiff_la_LIBADD = ${SQ_LOCAL_RPATH} -ltiff

EXTRA_DIST = libkls_tiff.so.ui

install-data-am:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	$(INSTALL) --mode=0644 libkls_tiff.so.ui $(DESTDIR)$(pkgdatadir)/libkls_tiff.so.ui

AM_CXXFLAGS = -DTIFF_UI=\"${pkgdatadir}/libkls_tiff.so.ui\"
