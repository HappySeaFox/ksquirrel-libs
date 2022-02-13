SUBDIRS = ftview

INCLUDES = -I.. -I../include -Iftview @SQ_FT_CFLAGS@

pkglib_LTLIBRARIES = libkls_ttf.la

libkls_ttf_la_SOURCES = fmt_codec_ttf.cpp fmt_codec_ttf_defs.h ttf2mem.cpp

libkls_ttf_la_LDFLAGS = -version-info 2:2:2

libkls_ttf_la_LIBADD = -Lftview -lftview @SQ_FT_LDFLAGS@ ${SQ_LOCAL_RPATH}
