INCLUDES = -I.. -I../include @SQ_WMF_CFLAGS@

lib_LTLIBRARIES = libSQ_codec_wmf.la

libSQ_codec_wmf_la_SOURCES = fmt_codec_wmf.cpp wmf2mem.cpp fmt_codec_wmf_defs.h

libSQ_codec_wmf_la_LDFLAGS = -release 0.9.0

libSQ_codec_wmf_la_LIBADD = @SQ_WMF_LDFLAGS@ ${SQ_LOCAL_RPATH}