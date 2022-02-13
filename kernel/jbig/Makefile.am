SUBDIRS = libjbig

INCLUDES = -I.. -I../include -Ilibjbig

lib_LTLIBRARIES = libSQ_codec_jbig.la

libSQ_codec_jbig_la_SOURCES = fmt_codec_jbig.cpp fmt_codec_jbig_defs.h jbig2mem.cpp

libSQ_codec_jbig_la_LDFLAGS = -release 0.1.1

libSQ_codec_jbig_la_LIBADD = -Llibjbig -ljbig ${SQ_LOCAL_RPATH}