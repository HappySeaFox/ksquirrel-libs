INCLUDES = -I../include

pkglib_LTLIBRARIES = libkls_eps.la

libkls_eps_la_SOURCES = fmt_codec_pnm.cpp fmt_codec_pnm_defs.h

libkls_eps_la_LDFLAGS = ${SQ_RELEASE} -lm

libkls_eps_la_LIBADD = ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = -DCODEC_EPS -DEPS2PPM=\"${EPS2PPM}\"
