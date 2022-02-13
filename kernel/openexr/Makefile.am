CXXFLAGS = @CXXFLAGS@ -fexceptions

INCLUDES = -I.. -I../include @SQ_EXR_CFLAGS@

lib_LTLIBRARIES = libSQ_codec_openexr.la

libSQ_codec_openexr_la_SOURCES = fmt_codec_openexr.cpp fmt_codec_openexr_defs.h

libSQ_codec_openexr_la_LDFLAGS = -release 0.2.1

libSQ_codec_openexr_la_LIBADD = @SQ_EXR_LDFLAGS@ ${SQ_LOCAL_RPATH}