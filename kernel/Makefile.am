SUBDIRS = ksquirrel-libs kls_avs kls_bmp kls_cut kls_fli kls_hdr kls_ico kls_jbig kls_koala kls_lif kls_mdl kls_mtv kls_pcx kls_pix kls_pnm kls_psd kls_pxr kls_ras kls_rawrgb kls_sct kls_sgi kls_sun kls_tga kls_wal kls_wbmp kls_xbm kls_xcf kls_xcur kls_xpm

EXTRA_DIST = generate link include/*.h include/ksquirrel-libs/*.h xpm/*.xpm

if SQ_HAVE_CAMERA
SUBDIRS += kls_camera
endif

if SQ_HAVE_JPEG
SUBDIRS += kls_jpeg
endif

if SQ_HAVE_PNG
SUBDIRS += kls_png
endif

if SQ_HAVE_TIFF
SUBDIRS += kls_tiff
endif

if SQ_HAVE_XWD
SUBDIRS += kls_xwd
endif

if SQ_HAVE_WMF
SUBDIRS += kls_wmf
endif

if SQ_HAVE_SVG
SUBDIRS += kls_svg
endif

if SQ_HAVE_GIF
SUBDIRS += kls_gif
endif

if SQ_HAVE_OPENEXR
SUBDIRS += kls_openexr
endif

if SQ_HAVE_JPEG2000
SUBDIRS += kls_jpeg2000
endif

if SQ_HAVE_TTF
SUBDIRS += kls_ttf
endif

if SQ_HAVE_MNG
SUBDIRS += kls_mng
endif

if SQ_HAVE_DJVU
SUBDIRS += kls_djvu
endif

if SQ_HAVE_DXF
SUBDIRS += kls_dxf
endif

if SQ_HAVE_NEO
SUBDIRS += kls_neo
endif

if SQ_HAVE_LEAF
SUBDIRS += kls_leaf
endif

if SQ_HAVE_PI1
SUBDIRS += kls_pi1
endif

if SQ_HAVE_XIM
SUBDIRS += kls_xim
endif

if SQ_HAVE_UTAH
SUBDIRS += kls_utah
endif

if SQ_HAVE_PICT
SUBDIRS += kls_pict
endif

if SQ_HAVE_MAC
SUBDIRS += kls_mac
endif

if SQ_HAVE_IFF
SUBDIRS += kls_iff
endif

# unstable/buggy codecs - for developers only
if SQ_DEVEL
SUBDIRS += kls_msp
endif

# install headers
install-data-am:
	$(mkinstalldirs) $(DESTDIR)$(pkgincludedir)
	$(INSTALL_HEADER) include/ksquirrel-libs/fileio.h         $(DESTDIR)$(pkgincludedir)/fileio.h
	$(INSTALL_HEADER) include/ksquirrel-libs/error.h          $(DESTDIR)$(pkgincludedir)/error.h
	$(INSTALL_HEADER) include/ksquirrel-libs/fmt_utils.h      $(DESTDIR)$(pkgincludedir)/fmt_utils.h
	$(INSTALL_HEADER) include/ksquirrel-libs/fmt_types.h      $(DESTDIR)$(pkgincludedir)/fmt_types.h
	$(INSTALL_HEADER) include/ksquirrel-libs/fmt_defs.h       $(DESTDIR)$(pkgincludedir)/fmt_defs.h
	$(INSTALL_HEADER) include/ksquirrel-libs/fmt_codec_base.h $(DESTDIR)$(pkgincludedir)/fmt_codec_base.h
	$(INSTALL_HEADER) include/ksquirrel-libs/settings.h       $(DESTDIR)$(pkgincludedir)/settings.h
