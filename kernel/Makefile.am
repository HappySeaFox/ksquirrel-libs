SUBDIRS = lib avs bmp cut fli hdr ico jbig koala lif mdl mtv pcx pix pnm psd pxr ras rawrgb sct sgi sun tga wal wbmp xbm xcur xpm xwd

EXTRA_DIST = fmt_defs.h error.h fileio.h fmt_codec_base.h fmt_utils.h fmt_types.h generate link include/*.h

if SQ_HAVE_CAMERA
SUBDIRS += camera
endif

if SQ_HAVE_JPEG
SUBDIRS += jpeg
endif

if SQ_HAVE_PNG
SUBDIRS += png
endif

if SQ_HAVE_TIFF
SUBDIRS += tiff
endif

if SQ_HAVE_XWD
SUBDIRS += xwd
endif

if SQ_HAVE_WMF
SUBDIRS += wmf
endif

if SQ_HAVE_SVG
SUBDIRS += svg
endif

if SQ_HAVE_GIF
SUBDIRS += gif
endif

if SQ_HAVE_OPENEXR
SUBDIRS += openexr
endif

if SQ_HAVE_JPEG2000
SUBDIRS += jpeg2000
endif

if SQ_HAVE_TTF
SUBDIRS += ttf
endif

if SQ_HAVE_MNG
SUBDIRS += mng
endif

# unstable/buggy codecs - for developers only
if SQ_DEVEL
SUBDIRS += mac msp iff utah
endif

# install headers
install-data-am:
	$(mkinstalldirs) $(DESTDIR)$(includedir)/ksquirrel-libs
	$(INSTALL_HEADER) fileio.h         $(DESTDIR)$(includedir)/ksquirrel-libs/fileio.h
	$(INSTALL_HEADER) error.h          $(DESTDIR)$(includedir)/ksquirrel-libs/error.h
	$(INSTALL_HEADER) fmt_utils.h      $(DESTDIR)$(includedir)/ksquirrel-libs/fmt_utils.h
	$(INSTALL_HEADER) fmt_types.h      $(DESTDIR)$(includedir)/ksquirrel-libs/fmt_types.h
	$(INSTALL_HEADER) fmt_defs.h       $(DESTDIR)$(includedir)/ksquirrel-libs/fmt_defs.h
	$(INSTALL_HEADER) fmt_codec_base.h $(DESTDIR)$(includedir)/ksquirrel-libs/fmt_codec_base.h
