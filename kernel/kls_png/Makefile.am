SUBDIRS = ksquirrel-libs-png

INCLUDES = -I../include -Iksquirrel-libs-png

pkglib_LTLIBRARIES = libkls_png.la

libkls_png_la_SOURCES = fmt_codec_png.cpp fmt_codec_png_defs.h

libkls_png_la_LDFLAGS = ${SQ_RELEASE}

libkls_png_la_LIBADD = ${SQ_LOCAL_RPATH} -Lksquirrel-libs-png -lksquirrel-libs-png

EXTRA_DIST = README

AM_CXXFLAGS = -DCODEC_PNG