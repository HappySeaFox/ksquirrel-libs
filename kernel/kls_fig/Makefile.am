INCLUDES = -I../include

bin_SCRIPTS = ksquirrel-libs-fig2ppm

pkglib_LTLIBRARIES = libkls_fig.la

libkls_fig_la_SOURCES = fmt_codec_pnm.cpp fmt_codec_pnm_defs.h

libkls_fig_la_LDFLAGS = ${SQ_RELEASE}

libkls_fig_la_LIBADD = ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = -DCODEC_FIG -DXFIG_S=\"${bindir}/ksquirrel-libs-fig2ppm\"

EXTRA_DIST = ksquirrel-libs-fig2ppm.in