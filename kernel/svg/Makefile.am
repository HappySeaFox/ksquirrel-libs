INCLUDES = -I.. -I../include -I../../contrib/cairo/src -I../../contrib/libsvg/src -I../../contrib/libsvg-cairo/src

lib_LTLIBRARIES = libSQ_codec_svg.la

libSQ_codec_svg_la_SOURCES = fmt_codec_svg.cpp fmt_codec_svg_defs.h svg2mem.cpp svg2mem.h

libSQ_codec_svg_la_LDFLAGS = -release 0.1.1

LIBS = -lfreetype -lxml2 -lfontconfig -L. -lsvg-cairo -lsvg -lcairo -lpixman ../fileio.lo
