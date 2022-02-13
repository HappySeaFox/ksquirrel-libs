INCLUDES = -I../include

pkglib_LTLIBRARIES = libkls_dxf.la

libkls_dxf_la_SOURCES = fmt_codec_pnm.cpp fmt_codec_pnm_defs.h

libkls_dxf_la_LDFLAGS = ${SQ_RELEASE}

libkls_dxf_la_LIBADD = ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = -DDXF_UI=\"${pkgdatadir}/libkls_dxf.so.ui\" -DCODEC_DXF -DVEC2WEB=\"${VEC2WEB}\" -DCODEC_ANOTHER

EXTRA_DIST = libkls_dxf.so.ui

install-data-am:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	$(INSTALL) --mode=0644 libkls_dxf.so.ui $(DESTDIR)$(pkgdatadir)/libkls_dxf.so.ui
