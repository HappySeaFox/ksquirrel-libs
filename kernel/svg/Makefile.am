INCLUDES = -I.. -I../include @SQ_SVG_CFLAGS@

lib_LTLIBRARIES = libSQ_codec_svg.la

libSQ_codec_svg_la_SOURCES = fmt_codec_svg.cpp fmt_codec_svg_defs.h svg2mem.cpp svg2mem.h

libSQ_codec_svg_la_LDFLAGS = -release 0.1.2

libSQ_codec_svg_la_LIBADD = @SQ_SVG_LDFLAGS@ ../fileio.lo
