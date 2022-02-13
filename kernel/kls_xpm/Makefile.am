INCLUDES = -I.. -I../include

pkglib_LTLIBRARIES = libkls_xpm.la

libkls_xpm_la_SOURCES = fmt_codec_xpm.cpp fmt_codec_xpm_defs.h xpm_utils.h

libkls_xpm_la_LDFLAGS = -version-info 6:4:6

libkls_xpm_la_LIBADD = ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = -DSQ_RGBMAP=\"@pkgdatadir@/rgbmap\"

EXTRA_DIST = rgbmap

install-data-am:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	$(INSTALL) --mode=0644 rgbmap $(DESTDIR)$(pkgdatadir)/rgbmap