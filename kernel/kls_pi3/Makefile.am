INCLUDES = -I../include

bin_SCRIPTS = ksquirrel-libs-pi32ppm

pkglib_LTLIBRARIES = libkls_pi3.la

libkls_pi3_la_SOURCES = fmt_codec_pnm.cpp fmt_codec_pnm_defs.h

libkls_pi3_la_LDFLAGS = ${SQ_RELEASE}

libkls_pi3_la_LIBADD = ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = -DCODEC_NETPBM -DCODEC_PI3 -DNETPBM_S=\"${bindir}/ksquirrel-libs-pi32ppm\"

EXTRA_DIST = ksquirrel-libs-pi32ppm.in