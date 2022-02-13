INCLUDES = -I../include -I../kls_png

bin_SCRIPTS = ksquirrel-libs-svg2png

pkglib_LTLIBRARIES = libkls_svg.la

libkls_svg_la_SOURCES = fmt_codec_png.cpp fmt_codec_png_defs.h

libkls_svg_la_LDFLAGS = ${SQ_RELEASE}

libkls_svg_la_LIBADD = ${SQ_LOCAL_RPATH} -L../kls_png/ksquirrel-libs-png -lksquirrel-libs-png

AM_CXXFLAGS = -DCODEC_SVG -DCODEC_ANOTHER -DSVG_UI=\"${pkgdatadir}/libkls_svg.so.ui\" -DRSVG=\"${RSVG}\" -DSVG2PNG=\"${bindir}/ksquirrel-libs-svg2png\"

EXTRA_DIST = libkls_svg.so.ui ksquirrel-libs-svg2png.in

install-data-am:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	$(INSTALL) --mode=0644 libkls_svg.so.ui $(DESTDIR)$(pkgdatadir)/libkls_svg.so.ui
