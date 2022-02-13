INCLUDES = -I../include @SQ_SVG_CFLAGS@

pkglib_LTLIBRARIES = libkls_svg.la

libkls_svg_la_SOURCES = fmt_codec_svg.cpp fmt_codec_svg_defs.h svg2mem.cpp svg2mem.h

libkls_svg_la_LDFLAGS = ${SQ_RELEASE}

libkls_svg_la_LIBADD = @SQ_SVG_LDFLAGS@  ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = -DSVG_UI=\"${pkgdatadir}/libkls_svg.so.ui\"

EXTRA_DIST = libkls_svg.so.ui

install-data-am:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	$(INSTALL) --mode=0644 libkls_svg.so.ui $(DESTDIR)$(pkgdatadir)/libkls_svg.so.ui
