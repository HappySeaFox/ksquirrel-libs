SUBDIRS = ftview

INCLUDES = -I../include -Iftview @SQ_FT_CFLAGS@

bin_PROGRAMS = ksquirrel-libs-ttf2pnm

ksquirrel_libs_ttf2pnm_SOURCES = ttf2pnm.cpp ftcommon.cpp

ksquirrel_libs_ttf2pnm_LDADD = @SQ_FT_LDFLAGS@ -Lftview -lftview

pkglib_LTLIBRARIES = libkls_ttf.la

libkls_ttf_la_SOURCES = fmt_codec_pnm.cpp fmt_codec_pnm_defs.h

libkls_ttf_la_LDFLAGS = ${SQ_RELEASE}

libkls_ttf_la_LIBADD = ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = -DCODEC_TTF -DTTF2PNM=\"${bindir}/ksquirrel-libs-ttf2pnm\"
