INCLUDES = -I../include -I../kls_png

bin_SCRIPTS = ksquirrel-libs-dicom2png

pkglib_LTLIBRARIES = libkls_dicom.la

libkls_dicom_la_SOURCES = fmt_codec_png.cpp fmt_codec_png_defs.h

libkls_dicom_la_LDFLAGS = ${SQ_RELEASE}

libkls_dicom_la_LIBADD = ${SQ_LOCAL_RPATH} -L../kls_png/ksquirrel-libs-png -lksquirrel-libs-png

AM_CXXFLAGS = -DCODEC_DICOM -DDICOM=\"${bindir}/ksquirrel-libs-dicom2png\"

EXTRA_DIST = ksquirrel-libs-dicom2png.in
