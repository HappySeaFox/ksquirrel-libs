AC_INIT([ksquirrel-libs], [0.6.0-pre9], [ksquirrel@tut.by])
#AC_GNU_SOURCE
#AC_CONFIG_SRCDIR([config.h.in])
#AM_INIT_AUTOMAKE([foreign])
#AC_CONFIG_HEADER([config.h])

AC_CONFIG_AUX_DIR([admin])

AC_CANONICAL_SYSTEM
AC_ARG_PROGRAM

AM_INIT_AUTOMAKE(ksquirrel-libs, 0.6.0-pre9)

AC_CHECK_COMPILERS
AC_ENABLE_SHARED(yes)
AC_ENABLE_STATIC(no)
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
#AC_FUNC_ERROR_AT_LINE
AC_FUNC_REALLOC
AC_FUNC_STAT

AC_CHECK_HEADER([zlib.h], [
    AC_CHECK_LIB([z], [inflate], [
    sq_have_zlib_ff="yes"],
    [ AC_MSG_ERROR([zlib library not found])])
], [ AC_MSG_ERROR([zlib header files not found])])

sq_codec_camera="yes"
sq_have_camera="yes"

AC_ARG_ENABLE(camera,
              [AS_HELP_STRING([--disable-camera], [don't compile camera codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_camera="yes" ;;
                       no)  sq_codec_camera="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

sq_codec_gif="yes"
sq_have_gif="no"

AC_ARG_ENABLE(gif,
              [AS_HELP_STRING([--disable-gif], [don't compile gif codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_gif="yes" ;;
                       no)  sq_codec_gif="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

sq_codec_svg="yes"
sq_have_svg="no"

AC_ARG_ENABLE(svg,
              [AS_HELP_STRING([--disable-svg], [don't compile svg codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_svg="yes" ;;
                       no)  sq_codec_svg="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

sq_codec_openexr="yes"
sq_have_openexr="no"

AC_ARG_ENABLE(openexr,
              [AS_HELP_STRING([--disable-openexr], [don't compile openexr codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_openexr="yes" ;;
                       no)  sq_codec_openexr="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

sq_codec_jpeg2000="yes"
sq_have_jpeg2000="no"

AC_ARG_ENABLE(jpeg2000,
              [AS_HELP_STRING([--disable-jpeg2000], [don't compile jpeg2000 codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_jpeg2000="yes" ;;
                       no)  sq_codec_jpeg2000="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

sq_codec_wmf="yes"
sq_have_wmf="no"

AC_ARG_ENABLE(wmf,
              [AS_HELP_STRING([--disable-wmf], [don't compile wmf codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_wmf="yes" ;;
                       no)  sq_codec_wmf="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

sq_codec_ttf="yes"
sq_have_ttf="no"

AC_ARG_ENABLE(ttf,
              [AS_HELP_STRING([--disable-ttf], [don't compile ttf codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_ttf="yes" ;;
                       no)  sq_codec_ttf="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

sq_codec_mng="yes"
sq_have_mng="no"

AC_ARG_ENABLE(mng,
              [AS_HELP_STRING([--disable-mng], [don't compile mng codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_mng="yes" ;;
                       no)  sq_codec_mng="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

# Check for freetype, if needed

# JPEG
AC_CHECK_HEADERS([jpeglib.h], [
		    AC_CHECK_LIB([jpeg], [main], [sq_have_jpeg="yes"], [sq_have_jpeg="no"])
		    ],
		    [sq_have_jpeg="no"])

# PNG
AC_CHECK_HEADER([png.h], [
		    AC_CHECK_LIB([png], [png_read_rows], [sq_have_png="yes"], [sq_have_png="no"], -lz -lm)
		    ],
		    [sq_have_png="no"])

# TIFF
AC_CHECK_HEADERS([tiff.h], [
		    AC_CHECK_LIB([tiff], [TIFFOpen], [sq_have_tiff="yes"], [sq_have_tiff="no"])
		    ],
		    [sq_have_tiff="no"])

# MNG

if test $sq_codec_svg = yes; then
AC_CHECK_HEADERS([libmng.h], [
		    AC_CHECK_LIB([mng], [mng_get_totallayers], [sq_have_mng="yes"], [sq_have_mng="no"])
		    ],
		    [sq_have_mng="no"])
fi

# SVG
if test $sq_codec_svg = yes; then
PKG_CHECK_MODULES([libpixman], libpixman,
[
    _cppflags=$CPPFLAGS
    CPPFLAGS="$libpixman_CFLAGS $CPPFLAGS"

    AC_CHECK_HEADERS([pixman.h],
        [AC_CHECK_LIB([pixman], [pixman_region_create], 
        [
            PKG_CHECK_MODULES([libsvg], libsvg,
            [
                CPPFLAGS="$_cppflags $libsvg_CFLAGS"

                AC_CHECK_HEADERS([svg.h],
                [
                    AC_CHECK_LIB([svg], [svg_create],
                    [
                        PKG_CHECK_MODULES([cairo], cairo,
                        [
                            CPPFLAGS="$_cppflags $cairo_CFLAGS"

                            AC_CHECK_HEADERS([cairo.h],
                            [
                                AC_CHECK_LIB([cairo], [cairo_create],
                                [
                                    PKG_CHECK_MODULES([libsvg_cairo], [libsvg-cairo],
                                    [
                                        CPPFLAGS="$_cppflags $libsvg_cairo_CFLAGS"

                                        AC_CHECK_HEADERS([svg-cairo.h],
                                        [
                                            AC_CHECK_LIB([svg-cairo], [svg_cairo_create],
                                            [
                                                sq_have_svg="yes"
                                                SQ_SVG_CFLAGS="$libpixman_CFLAGS $libsvg_CFLAGS $cairo_CFLAGS ${libsvg_cairo_CFLAGS}"
                                                SQ_SVG_LDFLAGS="$libpixman_LIBS $libsvg_LIBS $cairo_LIBS $libsvg_cairo_LIBS"
                                                AC_SUBST(SQ_SVG_CFLAGS)
                                                AC_SUBST(SQ_SVG_LDFLAGS)
                                            ], [sq_have_svg="no"], $libsvg_cairo_LIBS)
                                        ],
                                        [sq_have_svg="no"])
                                    ],
                                    [sq_have_svg="no"])
                                ],
                                [sq_have_svg="no"], $cairo_LIBS)
                            ],
                            [sq_have_svg="no"])
                        ],
                        [sq_have_svg="no"])
                    ],
                    [sq_have_svg="no"], $libsvg_LIBS)
                ],
                [sq_have_svg="no"])
            ],
            [sq_have_svg="no"])
        ],
        [sq_have_svg="no"], $libpixman_LIBS)
    ],
    [sq_have_svg="no"])

CPPFLAGS=$_cppflags

],
[sq_have_svg="no"])

fi

# JPEG2000
if test $sq_codec_jpeg2000 = yes; then

AC_CHECK_HEADERS([jasper/jasper.h], [
		    AC_CHECK_LIB([jasper], [jas_init], [sq_have_jpeg2000="yes"], [sq_have_jpeg2000="no"])
		    ],
		    [sq_have_jpeg2000="no"])

fi

# WMF
if test $sq_codec_wmf = yes; then

AC_PATH_PROG(WMF_CONFIG, libwmf-config)

if [ test -n "$WMF_CONFIG" ]; then

SQ_WMF_CFLAGS=`$WMF_CONFIG --cflags`
SQ_WMF_LDFLAGS=`$WMF_CONFIG --libs`

_cppflags=$CPPFLAGS
_ldflags=$LDFLAGS

CPPFLAGS="$SQ_WMF_CFLAGS $CPPFLAGS"
LDFLAGS="$LDFLAGS $SQ_WMF_LDFLAGS"

AC_CHECK_HEADERS([libwmf/types.h], [
		    AC_CHECK_LIB([wmf], [wmf_api_create], [
                        sq_have_wmf="yes"
                        AC_SUBST(SQ_WMF_CFLAGS)
                        AC_SUBST(SQ_WMF_LDFLAGS)
                    ], [sq_have_wmf="no"])
		    ],
		    [sq_have_wmf="no"])

CPPFLAGS=$_cppflags
LDFLAGS=$_ldflags

fi
fi

# GIF
if test $sq_codec_gif = yes; then

AC_CHECK_HEADERS([gif_lib.h], [
		    AC_CHECK_LIB([ungif], [DGifOpenFileName], [sq_have_gif="yes"], [sq_have_gif="no"])
		    ],
		    [sq_have_gif="no"])

fi

# OPENEXR
if test $sq_codec_openexr = yes; then

PKG_CHECK_MODULES([OpenEXR], OpenEXR, [
        SQ_EXR_CFLAGS=$OpenEXR_CFLAGS
        SQ_EXR_LDFLAGS=$OpenEXR_LIBS

        _cppflags=$CPPFLAGS
        _ldflags=$LDFLAGS

        CPPFLAGS="$SQ_EXR_CFLAGS $CPPFLAGS"
        LDFLAGS="$LDFLAGS $SQ_EXR_LDFLAGS"

        AC_LANG_SAVE
        AC_LANG_CPLUSPLUS

        AC_CHECK_HEADERS([OpenEXR/ImfRgbaFile.h], [
		            AC_CHECK_LIB([IlmImf], [ImfInputFileName], [
                                    sq_have_openexr="yes"
                                    AC_SUBST(SQ_EXR_CFLAGS)
                                    AC_SUBST(SQ_EXR_LDFLAGS)
                                    ], [sq_have_openexr="no"])
		            ],
		            [sq_have_openexr="no"])
        
        AC_LANG_RESTORE

        CPPFLAGS=$_cppflags
        LDFLAGS=$_ldflags
                            ],
                [sq_have_openexr="no"])
fi

if test $sq_codec_camera = yes; then
AC_CHECK_HEADERS([netinet/in.h],  [sq_have_camera="yes"], [sq_have_camera="no"])
fi

_cppflags=$CPPFLAGS
_ldflags=$LDFLAGS

# TTF
if test $sq_codec_ttf = yes; then

AC_PATH_PROG(FREETYPE_CONFIG, freetype-config)

if [ test -n "$FREETYPE_CONFIG" ]; then
        freetype_cflags=`$FREETYPE_CONFIG --cflags`
        freetype_libs=`$FREETYPE_CONFIG --libs`

        CPPFLAGS="$freetype_cflags $CPPFLAGS"
        LDFLAGS="$LDFLAGS $freetype_libs"

        AC_CHECK_HEADER(ft2build.h, [
            AC_CHECK_HEADER([freetype/ftbitmap.h], [
                AC_CHECK_LIB(freetype, FTC_ImageCache_New, [

                    SQ_FT_LDFLAGS="$freetype_libs"
                    SQ_FT_CFLAGS="$freetype_cflags"
                    sq_have_ttf="yes"
                    AC_SUBST(SQ_FT_CFLAGS)
                    AC_SUBST(SQ_FT_LDFLAGS)

                ],[ sq_have_ttf="no"])
            ],[ sq_have_ttf="no"])
        ],[ sq_have_ttf="no"])
fi

CPPFLAGS=$_cppflags
LDFLAGS=$_ldflags

fi


AC_CHECK_HEADERS([X11/XWDFile.h], [sq_have_xwd="yes"],    [sq_have_xwd="no"])

AC_CONFIG_FILES([
Makefile
doc/Makefile
doc/html/Makefile
doc/sources/Makefile
examples/Makefile
kernel/Makefile
kernel/aaa/Makefile
kernel/bmp/Makefile
kernel/gif/Makefile
kernel/pix/Makefile
kernel/xbm/Makefile
kernel/xpm/Makefile
kernel/tga/Makefile
kernel/sgi/Makefile
kernel/ras/Makefile
kernel/pnm/Makefile
kernel/pcx/Makefile
kernel/ico/Makefile
kernel/xcur/Makefile
kernel/psd/Makefile
kernel/fli/Makefile
kernel/iff/Makefile
kernel/utah/Makefile
kernel/cut/Makefile
kernel/rawrgb/Makefile
kernel/wbmp/Makefile
kernel/sun/Makefile
kernel/jpeg2000/Makefile
kernel/openexr/Makefile
kernel/koala/Makefile
kernel/wal/Makefile
kernel/mdl/Makefile
kernel/hdr/Makefile
kernel/sct/Makefile
kernel/lif/Makefile
kernel/mtv/Makefile
kernel/avs/Makefile
kernel/msp/Makefile
kernel/mac/Makefile
kernel/svg/Makefile
kernel/wmf/Makefile
kernel/png/Makefile
kernel/jpeg/Makefile
kernel/xwd/Makefile
kernel/tiff/Makefile
kernel/camera/Makefile
kernel/pxr/Makefile
kernel/ttf/Makefile
kernel/ttf/ftview/Makefile
kernel/mng/Makefile
kernel/jbig/libjbig/Makefile
kernel/jbig/Makefile
])

AM_CONDITIONAL(CAMERA_NO_JPEG,  [ test $sq_have_jpeg      = no  ])
AM_CONDITIONAL(SQ_HAVE_JPEG,    [ test $sq_have_jpeg      = yes ])
AM_CONDITIONAL(SQ_HAVE_PNG,     [ test $sq_have_png       = yes ])
AM_CONDITIONAL(SQ_HAVE_XWD,     [ test $sq_have_xwd       = yes ])
AM_CONDITIONAL(SQ_HAVE_TIFF,    [ test $sq_have_tiff      = yes ])

AM_CONDITIONAL(SQ_HAVE_CAMERA,  [ test $sq_codec_camera   = yes -a $sq_have_camera    = yes ])
AM_CONDITIONAL(SQ_HAVE_SVG,     [ test $sq_codec_svg      = yes -a $sq_have_svg       = yes ])
AM_CONDITIONAL(SQ_HAVE_WMF,     [ test $sq_codec_wmf      = yes -a $sq_have_wmf       = yes ])
AM_CONDITIONAL(SQ_HAVE_JPEG2000,[ test $sq_codec_jpeg2000 = yes -a $sq_have_jpeg2000  = yes ])
AM_CONDITIONAL(SQ_HAVE_OPENEXR, [ test $sq_codec_openexr  = yes -a $sq_have_openexr   = yes ])
AM_CONDITIONAL(SQ_HAVE_GIF,     [ test $sq_codec_gif      = yes -a $sq_have_gif       = yes ])
AM_CONDITIONAL(SQ_HAVE_TTF,     [ test $sq_codec_ttf      = yes -a $sq_have_ttf       = yes ])
AM_CONDITIONAL(SQ_HAVE_MNG,     [ test $sq_codec_mng      = yes -a $sq_have_mng       = yes ])

AC_OUTPUT

if test $libdir != "/usr/lib/ksquirrel-libs" -a $libdir != "/usr/lib/ksquirrel-libs/"; then libdir_warn="WARNING: libdir should be /usr/lib/ksquirrel-libs";
else libdir_warn="";
fi

echo "
KSquirrel-libs configuration:
-----------------------------
  Source code location:     ${srcdir}
  ksquirrel-libs version:   ${VERSION}
  C Compiler:               ${CC}
  C Compiler flags:         ${CFLAGS}
  C++ Compiler:             ${CXX}
  C++ Compiler flags:       ${CXXFLAGS}

Installation options:
---------------------
  libdir:               ${libdir} ${libdir_warn}

Libraries:
----------"

sq_enabled=""
sq_disabled=""
sq_manual=""
sq_total="33"

if test $sq_have_jpeg = yes;   then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled jpeg `;   else sq_disabled=`echo -n $sq_disabled jpeg `;  fi
if test $sq_have_png = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled png `;    else sq_disabled=`echo -n $sq_disabled png `;   fi
if test $sq_have_xwd = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled xwd `;    else sq_disabled=`echo -n $sq_disabled xwd `;   fi
if test $sq_have_tiff = yes;   then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled tiff `;   else sq_disabled=`echo -n $sq_disabled tiff `;  fi
if test $sq_have_camera = yes; then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled camera `; else sq_disabled=`echo -n $sq_disabled camera `;fi
if test $sq_have_wmf = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled wmf `;    else sq_disabled=`echo -n $sq_disabled wmf `;   fi
if test $sq_have_svg = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled svg `;    else sq_disabled=`echo -n $sq_disabled svg `;   fi
if test $sq_have_openexr = yes;then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled openexr `;else sq_disabled=`echo -n $sq_disabled openexr `;fi
if test $sq_have_ttf = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled ttf `;    else sq_disabled=`echo -n $sq_disabled ttf `;    fi
if test $sq_have_mng = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled mng `;    else sq_disabled=`echo -n $sq_disabled mng `;    fi

if test $sq_codec_camera = no;    then sq_manual=`echo -n $sq_manual camera`;   fi
if test $sq_codec_gif = no;       then sq_manual=`echo -n $sq_manual gif`;      fi
if test $sq_codec_svg = no;       then sq_manual=`echo -n $sq_manual svg`;      fi
if test $sq_codec_wmf = no;       then sq_manual=`echo -n $sq_manual wmf`;      fi
if test $sq_codec_openexr = no;   then sq_manual=`echo -n $sq_manual openexr`;  fi
if test $sq_codec_jpeg2000 = no;  then sq_manual=`echo -n $sq_manual jpeg2000`; fi
if test $sq_codec_ttf = no;       then sq_manual=`echo -n $sq_manual ttf`;      fi
if test $sq_codec_mng = no;       then sq_manual=`echo -n $sq_manual mng`;      fi

echo "
  Total enabled codecs      : $sq_total
  Optional formats, enabled : $sq_enabled
  Optional formats, disabled: $sq_disabled
  Manually disabled         : $sq_manual

Now run 'make' to compile ksquirrel-libs
 "
