INCLUDES = -I.. -I../include -I../../contrib/libwmf/include @SQ_FT_CFLAGS@

lib_LTLIBRARIES = libSQ_codec_wmf.la

libSQ_codec_wmf_la_SOURCES = fmt_codec_wmf.cpp wmf2mem.cpp fmt_codec_wmf_defs.h

libSQ_codec_wmf_la_LDFLAGS = -release 0.9.0

libSQ_codec_wmf_la_LIBADD = -L. -lwmf -lwmflite ../fileio.lo
