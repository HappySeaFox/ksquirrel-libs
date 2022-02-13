INCLUDES = -I../include

bin_SCRIPTS = ksquirrel-libs-pict2ppm

pkglib_LTLIBRARIES = libkls_pict.la

libkls_pict_la_SOURCES = fmt_codec_pnm.cpp fmt_codec_pnm_defs.h

libkls_pict_la_LDFLAGS = ${SQ_RELEASE}

libkls_pict_la_LIBADD = ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = -DCODEC_NETPBM -DCODEC_PICT -DNETPBM_S=\"${bindir}/ksquirrel-libs-pict2ppm\"

EXTRA_DIST = ksquirrel-libs-pict2ppm.in