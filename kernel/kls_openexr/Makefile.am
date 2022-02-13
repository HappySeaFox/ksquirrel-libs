CXXFLAGS = @CXXFLAGS@ -fexceptions

INCLUDES = -I.. -I../include @SQ_EXR_CFLAGS@

pkglib_LTLIBRARIES = libkls_openexr.la

libkls_openexr_la_SOURCES = fmt_codec_openexr.cpp fmt_codec_openexr_defs.h

libkls_openexr_la_LDFLAGS = -version-info 2:1:2

libkls_openexr_la_LIBADD = @SQ_EXR_LDFLAGS@  ${SQ_LOCAL_RPATH}