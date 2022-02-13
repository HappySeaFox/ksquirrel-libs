INCLUDES = -I.. -I../include @SQ_SVG_CFLAGS@

pkglib_LTLIBRARIES = libkls_svg.la

libkls_svg_la_SOURCES = fmt_codec_svg.cpp fmt_codec_svg_defs.h svg2mem.cpp svg2mem.h

libkls_svg_la_LDFLAGS = -version-info 1:2:1

libkls_svg_la_LIBADD = @SQ_SVG_LDFLAGS@  ${SQ_LOCAL_RPATH}