INCLUDES = -I../include

bin_SCRIPTS = ksquirrel-libs-leaf2ppm

pkglib_LTLIBRARIES = libkls_leaf.la

libkls_leaf_la_SOURCES = fmt_codec_pnm.cpp fmt_codec_pnm_defs.h

libkls_leaf_la_LDFLAGS = ${SQ_RELEASE}

libkls_leaf_la_LIBADD = ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = -DCODEC_NETPBM -DCODEC_LEAF -DNETPBM_S=\"${bindir}/ksquirrel-libs-leaf2ppm\"

EXTRA_DIST = ksquirrel-libs-leaf2ppm.in