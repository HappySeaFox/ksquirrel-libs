INCLUDES = -I../include

# shut up gcc
CFLAGS = -O4

bin_PROGRAMS = ksquirrel-libs-dcraw
bin_SCRIPTS  = ksquirrel-libs-camera2ppm

ksquirrel_libs_dcraw_SOURCES = dcraw.c

if CAMERA_NO_JPEG
camera_jpeg = -DNO_JPEG
else
nojpeg_opt = -ljpeg
endif

if CAMERA_NO_CMS
camera_cms = -DNO_LCMS
else
nocms_opt = ${lcms_LIBS}
endif

ksquirrel_libs_dcraw_LDADD  = ${nojpeg_opt} ${nocms_opt} -lm
ksquirrel_libs_dcraw_CFLAGS = ${camera_jpeg} ${camera_cms} ${lcms_CFLAGS}

pkglib_LTLIBRARIES = libkls_camera.la

libkls_camera_la_SOURCES = fmt_codec_pnm.cpp fmt_codec_pnm_defs.h

libkls_camera_la_LDFLAGS = ${SQ_RELEASE}
libkls_camera_la_LIBADD = ${SQ_LOCAL_RPATH}

AM_CXXFLAGS = ${camera_cms} -DCAMERA_UI=\"${pkgdatadir}/libkls_camera.so.ui\" -DCODEC_CAMERA -DCODEC_ANOTHER -DKLDCRAW_S=\"${bindir}/ksquirrel-libs-camera2ppm\" -DKLDCRAW=\"${bindir}/ksquirrel-libs-dcraw\" 

EXTRA_DIST = libkls_camera.so.ui ksquirrel-libs-camera2ppm.in

install-data-am:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	$(INSTALL) --mode=0644 libkls_camera.so.ui $(DESTDIR)$(pkgdatadir)/libkls_camera.so.ui
