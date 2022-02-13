INCLUDES = -I.. -I../include -I../../contrib/libjasper/src/libjasper/include

lib_LTLIBRARIES = libSQ_codec_jpeg2000.la

libSQ_codec_jpeg2000_la_SOURCES = fmt_codec_jpeg2000.cpp fmt_codec_jpeg2000_defs.h

libSQ_codec_jpeg2000_la_LDFLAGS = -release 0.3.1

libSQ_codec_jpeg2000_la_LIBADD = -L../../contrib/libjasper/src/libjasper -ljasper ../fileio.lo