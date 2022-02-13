SUBDIRS = xcf2pnm

INCLUDES = -I../include

pkglib_LTLIBRARIES = libkls_xcf.la

libkls_xcf_la_SOURCES = fmt_codec_pnm.cpp fmt_codec_pnm_defs.h

libkls_xcf_la_LDFLAGS = ${SQ_RELEASE}
libkls_xcf_la_LIBADD = ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = -DXCF_UI=\"${pkgdatadir}/libkls_xcf.so.ui\" -DCODEC_XCF -DCODEC_ANOTHER -DKLXCF2PNM=\"${bindir}/ksquirrel-libs-xcf2pnm\"

EXTRA_DIST = libkls_xcf.so.ui

install-data-am:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	$(INSTALL) --mode=0644 libkls_xcf.so.ui $(DESTDIR)$(pkgdatadir)/libkls_xcf.so.ui
