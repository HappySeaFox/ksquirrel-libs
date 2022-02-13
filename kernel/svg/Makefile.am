INCLUDES = -I.. -I../include -I../../contrib/cairo/src -I../../contrib/libsvg/src -I../../contrib/libsvg-cairo/src @SQ_XML_CFLAGS@ @SQ_FT_CFLAGS@

lib_LTLIBRARIES = libSQ_codec_svg.la

libSQ_codec_svg_la_SOURCES = fmt_codec_svg.cpp fmt_codec_svg_defs.h svg2mem.cpp svg2mem.h

libSQ_codec_svg_la_LDFLAGS = -release 0.1.1

libSQ_codec_svg_la_LIBADD = @SQ_XML_LDFLAGS@ @SQ_FT_LDFLAGS@ -L../../contrib/libsvg-cairo/src -L../../contrib/libsvg/src -L../../contrib/libpixman/src -L../../contrib/cairo/src -lsvg-cairo -lsvg -lcairo -lpixman ../fileio.lo
