INCLUDES = -I../include @SQ_WMF_CFLAGS@

pkglib_LTLIBRARIES = libkls_wmf.la

libkls_wmf_la_SOURCES = fmt_codec_wmf.cpp wmf2mem.cpp fmt_codec_wmf_defs.h

libkls_wmf_la_LDFLAGS = ${SQ_RELEASE}

libkls_wmf_la_LIBADD = @SQ_WMF_LDFLAGS@ ${SQ_LOCAL_RPATH}