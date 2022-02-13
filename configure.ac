AC_INIT([ksquirrel-libs], [0.7.1], [ksquirrel@tut.by])
#AC_GNU_SOURCE
#AC_CONFIG_SRCDIR([config.h.in])
#AM_INIT_AUTOMAKE([foreign])
#AC_CONFIG_HEADER([config.h])

AC_CONFIG_AUX_DIR([admin])

AC_CANONICAL_SYSTEM
AC_ARG_PROGRAM

AM_INIT_AUTOMAKE(ksquirrel-libs, 0.7.1)

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

AM_PROG_CC_C_O

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

sq_dev="no"

AC_ARG_ENABLE(devel,
              [AS_HELP_STRING([--enable-devel], [Enable buggy codecs (for developers only)  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_dev="yes" ;;
                       no)  sq_dev="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

sq_codec_camera="yes"

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

sq_codec_djvu="yes"
sq_have_djvu="no"

AC_ARG_ENABLE(djvu,
              [AS_HELP_STRING([--disable-djvu], [don't compile djvu codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_djvu="yes" ;;
                       no)  sq_codec_djvu="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

sq_codec_dxf="yes"
sq_have_dxf="no"

AC_ARG_ENABLE(dxf,
              [AS_HELP_STRING([--disable-dxf], [don't compile dxf codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_dxf="yes" ;;
                       no)  sq_codec_dxf="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

sq_codec_fig="yes"
sq_have_fig="no"

AC_ARG_ENABLE(fig,
              [AS_HELP_STRING([--disable-fig], [don't compile fig codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_fig="yes" ;;
                       no)  sq_codec_fig="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

sq_codec_xcf="yes"

AC_ARG_ENABLE(xcf,
              [AS_HELP_STRING([--disable-xcf], [don't compile xcf codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_xcf="yes" ;;
                       no)  sq_codec_xcf="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

sq_codec_ljpeg="yes"

AC_ARG_ENABLE(ljpeg,
              [AS_HELP_STRING([--disable-ljpeg], [don't compile ljpeg codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_ljpeg="yes" ;;
                       no)  sq_codec_ljpeg="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

missing()
{
    echo
    echo "*****************************************************"
    echo " $2 library or headers were not found."
    echo " $1 codec will be disabled."
    echo " To add support of $1 codec please install"
    echo " $2 libraries and headers."
    echo "*****************************************************"
    echo
}

missing_prog()
{
    echo
    echo "*****************************************************"
    echo " $2 program was not found."
    echo " $1 codec will be disabled."
    echo " To add support of $1 codec please install"
    echo " $2 binary program or package."
    echo "*****************************************************"
    echo
}

AC_CHECK_HEADERS([ksquirrel-libs/error.h], [sq_have_ksqlibs="yes"],
		    [sq_have_ksqlibs="no"])


# JPEG
AC_CHECK_HEADERS([jpeglib.h], [
		    AC_CHECK_LIB([jpeg], [main], [sq_have_jpeg="yes"], [sq_have_jpeg="no" missing JPEG libjpeg])
		    ],
		    [sq_have_jpeg="no" missing JPEG libjpeg])

# PNG
AC_CHECK_HEADER([png.h], [
		    AC_CHECK_LIB([png], [png_read_rows], [sq_have_png="yes"], [sq_have_png="no" missing PNG libpng], -lz -lm)
		    ],
		    [sq_have_png="no" missing PNG libpng])


# TIFF
AC_CHECK_HEADERS([tiff.h], [
		    AC_CHECK_LIB([tiff], [TIFFOpen], [sq_have_tiff="yes"], [sq_have_tiff="no" missing TIFF libtiff])
		    ],
		    [sq_have_tiff="no" missing TIFF libtiff
                    ])

# MNG

if test $sq_codec_mng = yes; then
AC_CHECK_HEADERS([libmng.h], [
		    AC_CHECK_LIB([mng], [mng_get_totallayers], [sq_have_mng="yes"], [sq_have_mng="no" missing MNG libmng])
		    ],
		    [sq_have_mng="no" missing MNG libmng])
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
                                                SQ_SVG_CFLAGS="$libpixman_CFLAGS $libsvg_CFLAGS $cairo_CFLAGS $libsvg_cairo_CFLAGS"
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

if test $sq_codec_camera = yes; then

PKG_CHECK_MODULES([lcms], lcms, [sq_have_cms="yes"], [sq_have_cms="no"])

fi

# JPEG2000
if test $sq_codec_jpeg2000 = yes; then

AC_CHECK_HEADERS([jasper/jasper.h], [
		    AC_CHECK_LIB([jasper], [jas_init], [sq_have_jpeg2000="yes"], [sq_have_jpeg2000="no" missing JPEG2000 jasper])
		    ],
		    [sq_have_jpeg2000="no" missing JPEG2000 jasper])

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
                    ], [sq_have_wmf="no" missing WMF libwmf])
		    ],
		    [sq_have_wmf="no" missing WMF libwmf])

CPPFLAGS=$_cppflags
LDFLAGS=$_ldflags

fi
fi

# GIF
if test $sq_codec_gif = yes; then

SQ_GIFLIBS=""

AC_CHECK_HEADERS([gif_lib.h], [
		    AC_CHECK_LIB([ungif], [DGifOpenFileName], [sq_have_gif="yes" SQ_GIFLIBS="-lungif"], 
                        [
                            AC_CHECK_LIB([gif], [DGifOpenFileName], [sq_have_gif="yes" SQ_GIFLIBS="-lgif"], [sq_have_gif="no" missing GIF "libungif or giflib"])
                        ]
                        )
		    ],
		    [sq_have_gif="no"])

AC_SUBST(SQ_GIFLIBS)

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
                                    ], [sq_have_openexr="no" missing OpenEXR libopenexr])
		            ],
		            [sq_have_openexr="no" missing OpenEXR libopenexr])
        
        AC_LANG_RESTORE

        CPPFLAGS=$_cppflags
        LDFLAGS=$_ldflags
                            ],
                [sq_have_openexr="no" missing OpenEXR libopenexr])
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

        AC_CHECK_HEADER([ft2build.h], [
            AC_CHECK_HEADER([freetype/ftbitmap.h], [
                AC_CHECK_LIB([freetype], [FTC_ImageCache_New], [
                    AC_CHECK_LIB([freetype], [FTC_SBitCache_New], [
                        AC_CHECK_LIB([freetype], [FTC_CMapCache_New], [
                            AC_CHECK_LIB([freetype], [FTC_Manager_New], [

                                SQ_FT_LDFLAGS="$freetype_libs"
                                SQ_FT_CFLAGS="$freetype_cflags"
                                sq_have_ttf="yes"
                                AC_SUBST(SQ_FT_CFLAGS)
                                AC_SUBST(SQ_FT_LDFLAGS)

                            ],[ sq_have_ttf="no" missing TTF freetype])
                        ],[ sq_have_ttf="no" missing TTF freetype])
                    ],[ sq_have_ttf="no" missing TTF freetype])
                ],[ sq_have_ttf="no" missing TTF freetype])
            ],[ sq_have_ttf="no" missing TTF freetype])
        ],[ sq_have_ttf="no" missing TTF freetype])
fi

CPPFLAGS=$_cppflags
LDFLAGS=$_ldflags

fi

if test $sq_codec_djvu = yes; then

AC_PATH_PROG(DJVU, ddjvu)

if [ test -n "$DJVU" ]; then
        sq_have_djvu="yes"
        AC_SUBST(DJVU)
else missing_prog DJVU ddjvu
fi

fi

if test $sq_codec_fig = yes; then

AC_PATH_PROG(XFIG, fig2dev)

if [ test -n "$XFIG" ]; then
        sq_have_fig="yes"
        AC_SUBST(XFIG)
else missing_prog XFIG fig2dev
fi

fi

if test $sq_codec_dxf = yes; then

AC_PATH_PROG(VEC2WEB, vec2web)

if [ test -n "$VEC2WEB" ]; then
        sq_have_dxf="yes"
        AC_SUBST(VEC2WEB)
else missing_prog DXF vec2web
fi

fi

AC_CHECK_HEADERS([X11/XWDFile.h], [sq_have_xwd="yes"], [sq_have_xwd="no" missing XWD])

###############  NetPBM binaries  ###############

AC_MSG_CHECKING([netpbm binary programs])
$ECHO

sq_codec_neo="yes"
sq_have_neo="no"
if test $sq_codec_neo = yes; then
    AC_PATH_PROG(NEOTOPPM, neotoppm)
    if [ test -n "$NEOTOPPM" ]; then sq_have_neo="yes" AC_SUBST(NEOTOPPM)
    else missing_prog NEO netpbm
    fi
fi

sq_codec_leaf="yes"
sq_have_leaf="no"
if test $sq_codec_leaf = yes; then
    AC_PATH_PROG(LEAFTOPPM, leaftoppm)
    if [ test -n "$LEAFTOPPM" ]; then sq_have_leaf="yes" AC_SUBST(LEAFTOPPM)
    else missing_prog LEAF netpbm
    fi
fi

sq_codec_pi1="yes"
sq_have_pi1="no"
if test $sq_codec_pi1 = yes; then
    AC_PATH_PROG(PI1TOPPM, pi1toppm)
    if [ test -n "$PI1TOPPM" ]; then sq_have_pi1="yes" AC_SUBST(PI1TOPPM)
    else missing_prog PI1 netpbm
    fi
fi

sq_codec_pi3="yes"
sq_have_pi3="no"
if test $sq_codec_pi3 = yes; then
    AC_PATH_PROG(PI3TOPPM, pi3topbm)
    if [ test -n "$PI3TOPPM" ]; then sq_have_pi3="yes" AC_SUBST(PI3TOPPM)
    else missing_prog PI3 netpbm
    fi
fi

sq_codec_xim="yes"
sq_have_xim="no"
if test $sq_codec_xim = yes; then
    AC_PATH_PROG(XIMTOPPM, ximtoppm)
    if [ test -n "$XIMTOPPM" ]; then sq_have_xim="yes" AC_SUBST(XIMTOPPM)
    else missing_prog XIM netpbm
    fi
fi

sq_codec_utah="yes"
sq_have_utah="no"
if test $sq_codec_utah = yes; then
    AC_PATH_PROG(UTAHTOPNM, rletopnm)
    if [ test -n "$UTAHTOPNM" ]; then sq_have_utah="yes" AC_SUBST(UTAHTOPNM)
    else missing_prog UTAH netpbm
    fi
fi

sq_codec_pict="yes"
sq_have_pict="no"
if test $sq_codec_pict = yes; then
    AC_PATH_PROG(PICTTOPPM, picttoppm)
    if [ test -n "$PICTTOPPM" ]; then sq_have_pict="yes" AC_SUBST(PICTTOPPM)
    else missing_prog PICT netpbm
    fi
fi

sq_codec_mac="yes"
sq_have_mac="no"
if test $sq_codec_mac = yes; then
    AC_PATH_PROG(MACTOPBM, macptopbm)
    if [ test -n "$MACTOPBM" ]; then sq_have_mac="yes" AC_SUBST(MACTOPBM)
    else missing_prog MAC netpbm
    fi
fi

sq_codec_iff="yes"
sq_have_iff="no"
if test $sq_codec_iff = yes; then
    AC_PATH_PROG(ILBMTOPPM, ilbmtoppm)
    if [ test -n "$ILBMTOPPM" ]; then sq_have_iff="yes" AC_SUBST(ILBMTOPPM)
    else missing_prog IFF netpbm
    fi
fi

#################################################


SQ_LOCAL_RPATH="-L../ksquirrel-libs -lksquirrel-libs"
AC_SUBST(SQ_LOCAL_RPATH)
SQ_RELEASE="-version-info 7:1:7"
AC_SUBST(SQ_RELEASE)

################### for xcf codec ##############

AC_C_INLINE

if test $sq_codec_xcf = yes; then

case $GCC::$CFLAGS in yes::*-Wall* ) ;;
                      yes::*) CFLAGS="$CFLAGS -Wall" ;;
esac
case $GCC::$CFLAGS in *-g* | *omit-frame-pointer* ) ;;
                      yes::*) CFLAGS="$CFLAGS -fomit-frame-pointer" ;;
esac

AC_C_BIGENDIAN

case $host_cpu in i?86 | x86_64 | powerpc)
      AC_DEFINE(CAN_DO_UNALIGNED_WORDS,1,
       [Define for CPUs that can read unaligned words without traps or faults])
esac

AC_CHECK_HEADERS(inttypes.h netinet/in.h arpa/inet.h getopt.h)

AC_CHECK_FUNCS(getopt_long strcasecmp)
AC_FUNC_MMAP
AM_GNU_GETTEXT(external)
AM_ICONV

fi

################################################

AC_CONFIG_FILES([
Makefile
doc/Makefile
doc/html/Makefile
doc/sources/Makefile
kernel/Makefile
kernel/kls_bmp/Makefile
kernel/kls_gif/Makefile
kernel/kls_pix/Makefile
kernel/kls_xbm/Makefile
kernel/kls_xpm/Makefile
kernel/kls_tga/Makefile
kernel/kls_sgi/Makefile
kernel/kls_ras/Makefile
kernel/kls_pnm/Makefile
kernel/kls_pcx/Makefile
kernel/kls_ico/Makefile
kernel/kls_xcur/Makefile
kernel/kls_psd/Makefile
kernel/kls_fli/Makefile
kernel/kls_cut/Makefile
kernel/kls_rawrgb/Makefile
kernel/kls_wbmp/Makefile
kernel/kls_sun/Makefile
kernel/kls_jpeg2000/Makefile
kernel/kls_openexr/Makefile
kernel/kls_koala/Makefile
kernel/kls_wal/Makefile
kernel/kls_mdl/Makefile
kernel/kls_hdr/Makefile
kernel/kls_sct/Makefile
kernel/kls_lif/Makefile
kernel/kls_mtv/Makefile
kernel/kls_avs/Makefile
kernel/kls_msp/Makefile
kernel/kls_svg/Makefile
kernel/kls_wmf/Makefile
kernel/kls_png/Makefile
kernel/kls_jpeg/Makefile
kernel/kls_xwd/Makefile
kernel/kls_tiff/Makefile
kernel/kls_camera/Makefile
kernel/kls_camera/ksquirrel-libs-camera2ppm
kernel/kls_pxr/Makefile
kernel/kls_ttf/Makefile
kernel/kls_ttf/ftview/Makefile
kernel/kls_mng/Makefile
kernel/kls_jbig/libjbig/Makefile
kernel/kls_jbig/Makefile
kernel/kls_djvu/Makefile
kernel/kls_dxf/Makefile
kernel/kls_xcf/Makefile
kernel/kls_xcf/xcf2pnm/Makefile
kernel/kls_fig/ksquirrel-libs-fig2ppm
kernel/kls_fig/Makefile
kernel/kls_mac/ksquirrel-libs-mac2ppm
kernel/kls_mac/Makefile
kernel/kls_iff/ksquirrel-libs-iff2ppm
kernel/kls_iff/Makefile
kernel/kls_neo/ksquirrel-libs-neo2ppm
kernel/kls_neo/Makefile
kernel/kls_pi1/ksquirrel-libs-pi12ppm
kernel/kls_pi1/Makefile
kernel/kls_pi3/ksquirrel-libs-pi32ppm
kernel/kls_pi3/Makefile
kernel/kls_xim/ksquirrel-libs-xim2ppm
kernel/kls_xim/Makefile
kernel/kls_leaf/ksquirrel-libs-leaf2ppm
kernel/kls_leaf/Makefile
kernel/kls_utah/ksquirrel-libs-utah2ppm
kernel/kls_utah/Makefile
kernel/kls_pict/ksquirrel-libs-pict2ppm
kernel/kls_pict/Makefile
kernel/kls_ljpeg/ksquirrel-libs-ljpeg2ppm-s
kernel/kls_ljpeg/ljpeg2ppm/Makefile
kernel/kls_ljpeg/Makefile
kernel/ksquirrel-libs/Makefile
ksquirrellibs.pc
])


AM_CONDITIONAL(SQ_DEVEL,        [ test $sq_dev            = yes ])

AM_CONDITIONAL(CAMERA_NO_CMS,   [ test $sq_have_cms       = no  ])
AM_CONDITIONAL(CAMERA_NO_JPEG,  [ test $sq_have_jpeg      = no  ])
AM_CONDITIONAL(SQ_HAVE_JPEG,    [ test $sq_have_jpeg      = yes ])
AM_CONDITIONAL(SQ_HAVE_PNG,     [ test $sq_have_png       = yes ])
AM_CONDITIONAL(SQ_HAVE_XWD,     [ test $sq_have_xwd       = yes ])
AM_CONDITIONAL(SQ_HAVE_TIFF,    [ test $sq_have_tiff      = yes ])

AM_CONDITIONAL(SQ_HAVE_CAMERA,  [ test $sq_codec_camera   = yes ])
AM_CONDITIONAL(SQ_HAVE_SVG,     [ test $sq_codec_svg      = yes -a $sq_have_svg       = yes ])
AM_CONDITIONAL(SQ_HAVE_WMF,     [ test $sq_codec_wmf      = yes -a $sq_have_wmf       = yes ])
AM_CONDITIONAL(SQ_HAVE_JPEG2000,[ test $sq_codec_jpeg2000 = yes -a $sq_have_jpeg2000  = yes ])
AM_CONDITIONAL(SQ_HAVE_OPENEXR, [ test $sq_codec_openexr  = yes -a $sq_have_openexr   = yes ])
AM_CONDITIONAL(SQ_HAVE_GIF,     [ test $sq_codec_gif      = yes -a $sq_have_gif       = yes ])
AM_CONDITIONAL(SQ_HAVE_TTF,     [ test $sq_codec_ttf      = yes -a $sq_have_ttf       = yes ])
AM_CONDITIONAL(SQ_HAVE_MNG,     [ test $sq_codec_mng      = yes -a $sq_have_mng       = yes ])
AM_CONDITIONAL(SQ_HAVE_DJVU,    [ test $sq_codec_djvu     = yes -a $sq_have_djvu      = yes ])
AM_CONDITIONAL(SQ_HAVE_DXF,     [ test $sq_codec_dxf      = yes -a $sq_have_dxf       = yes ])
AM_CONDITIONAL(SQ_HAVE_FIG,     [ test $sq_codec_fig      = yes -a $sq_have_fig       = yes ])
AM_CONDITIONAL(SQ_HAVE_XCF,     [ test $sq_codec_xcf      = yes ])
AM_CONDITIONAL(SQ_HAVE_LJPEG,   [ test $sq_codec_ljpeg    = yes ])

# NetPBM
AM_CONDITIONAL(SQ_HAVE_NEO,     [ test $sq_have_neo      = yes ])
AM_CONDITIONAL(SQ_HAVE_XIM,     [ test $sq_have_xim      = yes ])
AM_CONDITIONAL(SQ_HAVE_PI1,     [ test $sq_have_pi1      = yes ])
AM_CONDITIONAL(SQ_HAVE_PI3,     [ test $sq_have_pi3      = yes ])
AM_CONDITIONAL(SQ_HAVE_UTAH,    [ test $sq_have_utah     = yes ])
AM_CONDITIONAL(SQ_HAVE_LEAF,    [ test $sq_have_leaf     = yes ])
AM_CONDITIONAL(SQ_HAVE_PICT,    [ test $sq_have_pict     = yes ])
AM_CONDITIONAL(SQ_HAVE_MAC,     [ test $sq_have_mac      = yes ])
AM_CONDITIONAL(SQ_HAVE_IFF,     [ test $sq_have_iff      = yes ])

AC_OUTPUT

echo "
KSquirrel-libs configuration:
-----------------------------
  Source code location:     ${srcdir}
  ksquirrel-libs version:   ${VERSION}
  C++ Compiler:             ${CXX}
  C++ Compiler flags:       ${CXXFLAGS}
  Local ldflags:            ${SQ_LOCAL_RPATH}

Installation options:
---------------------
  libdir:               ${libdir} ${libdir_warn}

Libraries:
----------"

sq_enabled=""
sq_disabled=""
sq_manual=""
sq_total="27"
sq_devel=""
sq_devel_message=""

if test $sq_have_jpeg = yes;   then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled jpeg `;   else sq_disabled=`echo -n $sq_disabled jpeg `;  fi
if test $sq_have_png = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled png `;    else sq_disabled=`echo -n $sq_disabled png `;   fi
if test $sq_have_xwd = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled xwd `;    else sq_disabled=`echo -n $sq_disabled xwd `;   fi
if test $sq_have_tiff = yes;   then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled tiff `;   else sq_disabled=`echo -n $sq_disabled tiff `;  fi
if test $sq_codec_camera = yes; then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled camera `; else sq_disabled=`echo -n $sq_disabled camera `;fi
if test $sq_codec_camera = yes; then sq_camera_sett="Additional CAMERA settings: [LCMS: $sq_have_cms, JPEG: $sq_have_jpeg]";fi
if test $sq_have_wmf = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled wmf `;    else sq_disabled=`echo -n $sq_disabled wmf `;   fi
if test $sq_have_svg = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled svg `;    else sq_disabled=`echo -n $sq_disabled svg `;   fi
if test $sq_have_openexr = yes;then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled openexr `;else sq_disabled=`echo -n $sq_disabled openexr `;fi
if test $sq_have_ttf = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled ttf `;    else sq_disabled=`echo -n $sq_disabled ttf `;    fi
if test $sq_have_mng = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled mng `;    else sq_disabled=`echo -n $sq_disabled mng `;    fi
if test $sq_have_gif = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled gif `;    else sq_disabled=`echo -n $sq_disabled gif `;    fi
if test $sq_have_djvu = yes;   then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled djvu `;   else sq_disabled=`echo -n $sq_disabled djvu `;   fi
if test $sq_have_dxf = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled dxf `;    else sq_disabled=`echo -n $sq_disabled dxf `;    fi
if test $sq_have_fig = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled fig `;    else sq_disabled=`echo -n $sq_disabled fig `;    fi
if test $sq_codec_xcf = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled xcf `;    else sq_disabled=`echo -n $sq_disabled xcf `;   fi

if test $sq_have_neo = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled neo `;    else sq_disabled=`echo -n $sq_disabled neo `;    fi
if test $sq_have_pi1 = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled pi1 `;    else sq_disabled=`echo -n $sq_disabled pi1 `;    fi
if test $sq_have_pi3 = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled pi3 `;    else sq_disabled=`echo -n $sq_disabled pi3 `;    fi
if test $sq_have_utah = yes;   then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled utah `;   else sq_disabled=`echo -n $sq_disabled utah `;   fi
if test $sq_have_xim = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled xim `;    else sq_disabled=`echo -n $sq_disabled xim `;    fi
if test $sq_have_leaf = yes;   then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled leaf `;   else sq_disabled=`echo -n $sq_disabled leaf `;   fi
if test $sq_have_pict = yes;   then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled pict `;   else sq_disabled=`echo -n $sq_disabled pict `;   fi
if test $sq_have_iff = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled iff `;    else sq_disabled=`echo -n $sq_disabled iff `;    fi
if test $sq_have_mac = yes;    then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled mac `;    else sq_disabled=`echo -n $sq_disabled mac `;    fi
if test $sq_codec_ljpeg = yes; then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled ljpeg `;  else sq_disabled=`echo -n $sq_disabled ljpeg `;  fi

if test $sq_dev = yes;         then sq_total=`expr $sq_total + 1`; else sq_devel="msp" && sq_devel_message="(use --enable-devel to enable these codecs)"; fi

if test $sq_codec_camera = no;    then sq_manual=`echo -n $sq_manual camera`;   fi
if test $sq_codec_gif = no;       then sq_manual=`echo -n $sq_manual gif`;      fi
if test $sq_codec_svg = no;       then sq_manual=`echo -n $sq_manual svg`;      fi
if test $sq_codec_wmf = no;       then sq_manual=`echo -n $sq_manual wmf`;      fi
if test $sq_codec_openexr = no;   then sq_manual=`echo -n $sq_manual openexr`;  fi
if test $sq_codec_jpeg2000 = no;  then sq_manual=`echo -n $sq_manual jpeg2000`; fi
if test $sq_codec_ttf = no;       then sq_manual=`echo -n $sq_manual ttf`;      fi
if test $sq_codec_mng = no;       then sq_manual=`echo -n $sq_manual mng`;      fi
if test $sq_codec_djvu = no;      then sq_manual=`echo -n $sq_manual djvu`;     fi
if test $sq_codec_dxf = no;       then sq_manual=`echo -n $sq_manual dxf`;      fi
if test $sq_codec_xcf = no;       then sq_manual=`echo -n $sq_manual xcf`;      fi
if test $sq_codec_ljpeg = no;     then sq_manual=`echo -n $sq_manual ljpeg`;    fi

# old version of ksquirrel-libs installed
sq_have_ksqlibs_msg=""
if test $sq_have_ksqlibs = yes;
then sq_have_ksqlibs_msg="
************************************************
******************** WARNING *******************
************************************************

 It seemed that old version of ksquirrel-libs
 is installed on your system. It is HIGHLY
 recommended to remove old version (headers
 and libraries) before installing new one.

************************************************
";
fi

echo "
  Total enabled codecs      : $sq_total
  Optional formats, enabled : $sq_enabled
  Optional formats, disabled: $sq_disabled
  $sq_camera_sett
  Manually disabled         : $sq_manual
  Disabled as buggy         : $sq_devel $sq_devel_message
  $sq_have_ksqlibs_msg
Now run 'make' to compile ksquirrel-libs
 "
