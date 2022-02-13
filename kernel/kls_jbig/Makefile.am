SUBDIRS = libjbig

INCLUDES = -I../include -Ilibjbig

pkglib_LTLIBRARIES = libkls_jbig.la

libkls_jbig_la_SOURCES = fmt_codec_jbig.cpp fmt_codec_jbig_defs.h jbig2mem.cpp jbig2mem.h

libkls_jbig_la_LDFLAGS = ${SQ_RELEASE}

libkls_jbig_la_LIBADD = ${SQ_LOCAL_RPATH} -Llibjbig -ljbig