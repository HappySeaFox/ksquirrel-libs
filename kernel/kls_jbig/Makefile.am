SUBDIRS = libjbig

INCLUDES = -I.. -I../include -Ilibjbig

pkglib_LTLIBRARIES = libkls_jbig.la

libkls_jbig_la_SOURCES = fmt_codec_jbig.cpp fmt_codec_jbig_defs.h jbig2mem.cpp

libkls_jbig_la_LDFLAGS = -version-info 1:1:1

libkls_jbig_la_LIBADD = -Llibjbig -ljbig  ${SQ_LOCAL_RPATH}