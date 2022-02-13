INCLUDES = -I../include

bin_SCRIPTS = ksquirrel-libs-mac2ppm

pkglib_LTLIBRARIES = libkls_mac.la

libkls_mac_la_SOURCES = fmt_codec_pnm.cpp fmt_codec_pnm_defs.h

libkls_mac_la_LDFLAGS = ${SQ_RELEASE}

libkls_mac_la_LIBADD = ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = -DCODEC_NETPBM -DCODEC_MAC -DNETPBM_S=\"${bindir}/ksquirrel-libs-mac2ppm\"

EXTRA_DIST = ksquirrel-libs-mac2ppm.in