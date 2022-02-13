SUBDIRS = ftview

INCLUDES = -I.. -I../include -Iftview @SQ_FT_CFLAGS@

lib_LTLIBRARIES = libSQ_codec_ttf.la

libSQ_codec_ttf_la_SOURCES = fmt_codec_ttf.cpp fmt_codec_ttf_defs.h ttf2mem.cpp

libSQ_codec_ttf_la_LDFLAGS = -release 0.2.2

libSQ_codec_ttf_la_LIBADD = -Lftview -lftview @SQ_FT_LDFLAGS@ ${SQ_LOCAL_RPATH}
