INCLUDES = -I../include

bin_SCRIPTS = ksquirrel-libs-neo2ppm

pkglib_LTLIBRARIES = libkls_neo.la

libkls_neo_la_SOURCES = fmt_codec_pnm.cpp fmt_codec_pnm_defs.h

libkls_neo_la_LDFLAGS = ${SQ_RELEASE}

libkls_neo_la_LIBADD = ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = -DCODEC_NETPBM -DCODEC_NEO -DNETPBM_S=\"${bindir}/ksquirrel-libs-neo2ppm\"

EXTRA_DIST = ksquirrel-libs-neo2ppm.in