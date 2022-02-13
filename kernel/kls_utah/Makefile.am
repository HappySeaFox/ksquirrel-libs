INCLUDES = -I../include

bin_SCRIPTS = ksquirrel-libs-utah2ppm

pkglib_LTLIBRARIES = libkls_utah.la

libkls_utah_la_SOURCES = fmt_codec_pnm.cpp fmt_codec_pnm_defs.h

libkls_utah_la_LDFLAGS = ${SQ_RELEASE}

libkls_utah_la_LIBADD = ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = -DCODEC_NETPBM -DCODEC_UTAH -DNETPBM_S=\"${bindir}/ksquirrel-libs-utah2ppm\"

EXTRA_DIST = ksquirrel-libs-utah2ppm.in