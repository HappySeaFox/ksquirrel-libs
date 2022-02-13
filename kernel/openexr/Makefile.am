CXXFLAGS = @CXXFLAGS@ -fexceptions

INCLUDES = -I.. -I../include -I../../contrib/libopenexr/Half -I../../contrib/libopenexr/Iex -I../../contrib/libopenexr/IlmImf -I../../contrib/libopenexr/Imath

lib_LTLIBRARIES = libSQ_codec_openexr.la

libSQ_codec_openexr_la_SOURCES = fmt_codec_openexr.cpp fmt_codec_openexr_defs.h

libSQ_codec_openexr_la_LDFLAGS = -release 0.2.1

libSQ_codec_openexr_la_LIBADD = -L../../contrib/libopenexr/Imath -L../../contrib/libopenexr/IlmImf -L../../contrib/libopenexr/Iex -L../../contrib/libopenexr/Half -lIlmImf -lImath -lHalf -lIex ../fileio.lo
