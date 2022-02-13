INCLUDES = -I../include

bin_SCRIPTS = ksquirrel-libs-xim2ppm

pkglib_LTLIBRARIES = libkls_xim.la

libkls_xim_la_SOURCES = fmt_codec_pnm.cpp fmt_codec_pnm_defs.h

libkls_xim_la_LDFLAGS = ${SQ_RELEASE}

libkls_xim_la_LIBADD = ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = -DCODEC_NETPBM -DCODEC_XIM -DNETPBM_S=\"${bindir}/ksquirrel-libs-xim2ppm\"

EXTRA_DIST = ksquirrel-libs-xim2ppm.in
