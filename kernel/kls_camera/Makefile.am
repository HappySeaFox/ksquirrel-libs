INCLUDES = -I.. -I../include

pkglib_LTLIBRARIES = libkls_camera.la

libkls_camera_la_SOURCES = fmt_codec_camera.cpp fmt_codec_camera_defs.h dcraw.c dcraw.h

libkls_camera_la_LDFLAGS = -version-info 9:0:9

if CAMERA_NO_JPEG
libkls_camera_la_LIBADD = -lm  ${SQ_LOCAL_RPATH}
else
libkls_camera_la_LIBADD = -ljpeg -lm  ${SQ_LOCAL_RPATH}
endif
