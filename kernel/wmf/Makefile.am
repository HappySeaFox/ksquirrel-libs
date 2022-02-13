INCLUDES = -I.. -I../include -I../../contrib/libwmf/include -I/usr/include/freetype2

lib_LTLIBRARIES = libSQ_codec_wmf.la

libSQ_codec_wmf_la_SOURCES = fmt_codec_wmf.cpp wmf2mem.cpp fmt_codec_wmf_defs.h ../fileio.cpp

libSQ_codec_wmf_la_LIBDADD = 

libSQ_codec_wmf_la_LDFLAGS = -release 0.9.0

LIBS = -L. -lwmf -lwmflite
