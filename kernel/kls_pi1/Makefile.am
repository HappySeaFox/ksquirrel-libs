INCLUDES = -I../include

bin_SCRIPTS = ksquirrel-libs-pi12ppm

pkglib_LTLIBRARIES = libkls_pi1.la

libkls_pi1_la_SOURCES = fmt_codec_pnm.cpp fmt_codec_pnm_defs.h

libkls_pi1_la_LDFLAGS = ${SQ_RELEASE}

libkls_pi1_la_LIBADD = ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = -DCODEC_NETPBM -DCODEC_PI1 -DNETPBM_S=\"${bindir}/ksquirrel-libs-pi12ppm\"

EXTRA_DIST = ksquirrel-libs-pi12ppm.in