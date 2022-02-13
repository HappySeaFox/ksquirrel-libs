SUBDIRS = ljpeg2ppm

INCLUDES = -I../include

bin_SCRIPTS = ksquirrel-libs-ljpeg2ppm-s

pkglib_LTLIBRARIES = libkls_ljpeg.la

libkls_ljpeg_la_SOURCES = fmt_codec_pnm.cpp fmt_codec_pnm_defs.h

libkls_ljpeg_la_LDFLAGS = ${SQ_RELEASE}

libkls_ljpeg_la_LIBADD = ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = -DCODEC_LJPEG -DLJPEG2PPM_S=\"${bindir}/ksquirrel-libs-ljpeg2ppm-s\" -DLJPEG2PPM=\"${bindir}/ksquirrel-libs-ljpeg2ppm\"

EXTRA_DIST = ksquirrel-libs-ljpeg2ppm-s.in
