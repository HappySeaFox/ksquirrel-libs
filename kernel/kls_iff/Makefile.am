INCLUDES = -I../include

bin_SCRIPTS = ksquirrel-libs-iff2ppm

pkglib_LTLIBRARIES = libkls_iff.la

libkls_iff_la_SOURCES = fmt_codec_pnm.cpp fmt_codec_pnm_defs.h

libkls_iff_la_LDFLAGS = ${SQ_RELEASE}

libkls_iff_la_LIBADD = ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = -DCODEC_NETPBM -DCODEC_IFF -DNETPBM_S=\"${bindir}/ksquirrel-libs-iff2ppm\"

EXTRA_DIST = ksquirrel-libs-iff2ppm.in