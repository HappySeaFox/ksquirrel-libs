AC_INIT([ksquirrel-libs], [0.6.0-pre8], [ksquirrel@tut.by])
#AC_GNU_SOURCE
#AC_CONFIG_SRCDIR([config.h.in])
#AM_INIT_AUTOMAKE([foreign])
#AC_CONFIG_HEADER([config.h])

AC_CONFIG_AUX_DIR([admin])

AC_CANONICAL_SYSTEM
AC_ARG_PROGRAM

AM_INIT_AUTOMAKE(ksquirrel-libs, 0.6.0-pre8)

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

AC_ARG_ENABLE(camera,
              [AS_HELP_STRING([--disable-camera], [don't compile camera codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_camera="yes" ;;
                       no)  sq_codec_camera="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

sq_codec_gif="yes"

AC_ARG_ENABLE(gif,
              [AS_HELP_STRING([--disable-gif], [don't compile gif codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_gif="yes" ;;
                       no)  sq_codec_gif="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

sq_codec_svg="yes"

AC_ARG_ENABLE(svg,
              [AS_HELP_STRING([--disable-svg], [don't compile svg codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_svg="yes" ;;
                       no)  sq_codec_svg="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

sq_codec_openexr="yes"

AC_ARG_ENABLE(openexr,
              [AS_HELP_STRING([--disable-openexr], [don't compile openexr codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_openexr="yes" ;;
                       no)  sq_codec_openexr="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

sq_codec_jpeg2000="yes"

AC_ARG_ENABLE(jpeg2000,
              [AS_HELP_STRING([--disable-jpeg2000], [don't compile jpeg2000 codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_jpeg2000="yes" ;;
                       no)  sq_codec_jpeg2000="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

sq_codec_wmf="yes"

AC_ARG_ENABLE(wmf,
              [AS_HELP_STRING([--disable-wmf], [don't compile wmf codec  [default=no]])],
              [     case "${enableval}" in
                       yes) sq_codec_wmf="yes" ;;
                       no)  sq_codec_wmf="no" ;;
                       *)   AC_MSG_ERROR([bad value ${enableval} for this parameter]) ;;
                    esac]
             )

SQ_FT_CFLAGS=""
SQ_FT_LDFLAGS=""
SQ_XML_CFLAGS=""
SQ_XML_LDFLAGS=""

sq_have_freetype="no"
sq_have_jpeg="no"
sq_have_png="no"
sq_have_xwd="no"
sq_have_tiff="no"
sq_have_camera="no"
sq_have_xml="no"

# Check for freetype, if needed

if test $sq_codec_wmf = yes -o $sq_codec_svg = yes; then

_cppflags=$CPPFLAGS
_ldflags=$LDFLAGS

AC_PATH_PROG(FREETYPE_CONFIG, freetype-config)

if [ test -n "$FREETYPE_CONFIG" ]; then
	freetype_cflags=`$FREETYPE_CONFIG --cflags`
	freetype_libs=`$FREETYPE_CONFIG --libs`

        CPPFLAGS="$freetype_cflags $CPPFLAGS"
        LDFLAGS="$LDFLAGS $freetype_libs"

        AC_CHECK_HEADER(ft2build.h, [
            AC_CHECK_LIB(freetype, FT_Init_FreeType, [
                AC_CHECK_HEADER(expat.h, [
                    AC_CHECK_LIB(expat, XML_ParserCreate, [
                        AC_CHECK_HEADER(fontconfig/fontconfig.h, [
                            AC_CHECK_LIB(fontconfig, FcBlanksCreate, [
                                    SQ_FT_LDFLAGS="$freetype_libs"
                                    SQ_FT_CFLAGS="$freetype_cflags"
	                            sq_have_freetype="yes"
                                ],[ sq_have_freetype="no"], -lz -lexpat)
                            ],[ sq_have_freetype="no"])
                        ],[ sq_have_freetype="no"])
                    ],[ sq_have_freetype="no"])
             ],[ sq_have_freetype="no"])
        ],[ sq_have_freetype="no"])
fi

CPPFLAGS=$_cppflags
LDFLAGS=$_ldflags

AC_SUBST([SQ_FT_CFLAGS], $SQ_FT_CFLAGS)
AC_SUBST([SQ_FT_LDFLAGS], $SQ_FT_LDFLAGS)

# Check for XML

_cppflags=$CPPFLAGS
_ldflags=$LDFLAGS

AC_PATH_PROG(XML_CONFIG, xml2-config)

if [ test -n "$XML_CONFIG" ]; then
	xml_cflags=`$XML_CONFIG --cflags`
	xml_libs=`$XML_CONFIG --libs`

        CPPFLAGS="$xml_cflags $CPPFLAGS"
        LDFLAGS="$LDFLAGS $xml_libs"

        AC_CHECK_HEADER([libxml/xmlreader.h], [
            AC_CHECK_LIB([xml2], [xmlCreateURI], [
                 SQ_XML_LDFLAGS="$xml_libs"
                 SQ_XML_CFLAGS="$xml_cflags"
	         sq_have_xml="yes"
             ],[ sq_have_xml="no"])
        ],[ sq_have_xml="no"])
fi

CPPFLAGS=$_cppflags
LDFLAGS=$_ldflags

AC_SUBST([SQ_XML_CFLAGS], $SQ_XML_CFLAGS)
AC_SUBST([SQ_XML_LDFLAGS], $SQ_XML_LDFLAGS)

fi

# Check for headers & libs

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
		    AC_CHECK_LIB([tiff], [TIFFOpen], [sq_have_tiff="yes"], [sq_have_png="no"])
		    ],

		    [sq_have_tiff="no"])


if test $sq_codec_camera = yes; then
AC_CHECK_HEADERS([netinet/in.h],  [sq_have_camera="yes"], [sq_have_camera="no"])
fi

AC_CHECK_HEADERS([X11/XWDFile.h], [sq_have_xwd="yes"],    [sq_have_xwd="no"])

if test $sq_codec_jpeg2000 = yes; then
AC_CONFIG_SUBDIRS([contrib/libjasper])
fi

if test $sq_codec_openexr = yes; then
AC_CONFIG_SUBDIRS([contrib/libopenexr])
fi

if test $sq_codec_svg = yes -a $sq_have_freetype = yes -a $sq_have_xml = yes; then
AC_CONFIG_SUBDIRS([contrib/libpixman])
AC_CONFIG_SUBDIRS([contrib/cairo])
AC_CONFIG_SUBDIRS([contrib/libsvg])
AC_CONFIG_SUBDIRS([contrib/libsvg-cairo])
fi

if test $sq_codec_wmf = yes -a $sq_have_freetype = yes -a $sq_have_xml = yes -a $sq_have_png = yes; then
AC_CONFIG_SUBDIRS([contrib/libwmf])
fi

if test $sq_codec_gif = yes; then
AC_CONFIG_SUBDIRS([contrib/libungif])
fi

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([doc/html/Makefile])
AC_CONFIG_FILES([doc/sources/Makefile])
AC_CONFIG_FILES([contrib/Makefile])
AC_CONFIG_FILES([examples/Makefile])
AC_CONFIG_FILES([kernel/Makefile])
AC_CONFIG_FILES([kernel/aaa/Makefile])
AC_CONFIG_FILES([kernel/bmp/Makefile])
AC_CONFIG_FILES([kernel/gif/Makefile])
AC_CONFIG_FILES([kernel/pix/Makefile])
AC_CONFIG_FILES([kernel/xbm/Makefile])
AC_CONFIG_FILES([kernel/xpm/Makefile])
AC_CONFIG_FILES([kernel/tga/Makefile])
AC_CONFIG_FILES([kernel/sgi/Makefile])
AC_CONFIG_FILES([kernel/ras/Makefile])
AC_CONFIG_FILES([kernel/pnm/Makefile])
AC_CONFIG_FILES([kernel/pcx/Makefile])
AC_CONFIG_FILES([kernel/ico/Makefile])
AC_CONFIG_FILES([kernel/xcur/Makefile])
AC_CONFIG_FILES([kernel/psd/Makefile])
AC_CONFIG_FILES([kernel/fli/Makefile])
AC_CONFIG_FILES([kernel/iff/Makefile])
AC_CONFIG_FILES([kernel/utah/Makefile])
AC_CONFIG_FILES([kernel/cut/Makefile])
AC_CONFIG_FILES([kernel/rawrgb/Makefile])
AC_CONFIG_FILES([kernel/wbmp/Makefile])
AC_CONFIG_FILES([kernel/sun/Makefile])
AC_CONFIG_FILES([kernel/jpeg2000/Makefile])
AC_CONFIG_FILES([kernel/openexr/Makefile])
AC_CONFIG_FILES([kernel/koala/Makefile])
AC_CONFIG_FILES([kernel/wal/Makefile])
AC_CONFIG_FILES([kernel/mdl/Makefile])
AC_CONFIG_FILES([kernel/hdr/Makefile])
AC_CONFIG_FILES([kernel/sct/Makefile])
AC_CONFIG_FILES([kernel/lif/Makefile])
AC_CONFIG_FILES([kernel/mtv/Makefile])
AC_CONFIG_FILES([kernel/avs/Makefile])
AC_CONFIG_FILES([kernel/msp/Makefile])
AC_CONFIG_FILES([kernel/mac/Makefile])
AC_CONFIG_FILES([kernel/svg/Makefile])
AC_CONFIG_FILES([kernel/wmf/Makefile])
AC_CONFIG_FILES([kernel/png/Makefile])
AC_CONFIG_FILES([kernel/jpeg/Makefile])
AC_CONFIG_FILES([kernel/xwd/Makefile])
AC_CONFIG_FILES([kernel/tiff/Makefile])
AC_CONFIG_FILES([kernel/camera/Makefile])
AC_CONFIG_FILES([kernel/pxr/Makefile])

AC_CONFIG_FILES([kernel/wmf/fonts/Makefile])
AC_CONFIG_FILES([kernel/wmf/fonts/libwmf-fontmap])
AC_CONFIG_FILES([kernel/wmf/fonts/fontmap])

AM_CONDITIONAL(CAMERA_NO_JPEG,  [ test $sq_have_jpeg      = no  ])
AM_CONDITIONAL(SQ_HAVE_JPEG,    [ test $sq_have_jpeg      = yes ])
AM_CONDITIONAL(SQ_HAVE_PNG,     [ test $sq_have_png       = yes ])
AM_CONDITIONAL(SQ_HAVE_XWD,     [ test $sq_have_xwd       = yes ])
AM_CONDITIONAL(SQ_HAVE_TIFF,    [ test $sq_have_tiff      = yes ])
AM_CONDITIONAL(SQ_HAVE_CAMERA,  [ test $sq_have_camera    = yes ])
AM_CONDITIONAL(SQ_HAVE_FREETYPE,[ test $sq_have_freetype  = yes ])

AM_CONDITIONAL(SQ_HAVE_SVG,     [ test $sq_codec_svg = yes      -a $sq_have_freetype  = yes -a $sq_have_xml = yes ])
AM_CONDITIONAL(SQ_HAVE_WMF,     [ test $sq_codec_wmf = yes      -a $sq_have_freetype  = yes -a $sq_have_xml = yes -a $sq_have_png = yes ])
AM_CONDITIONAL(SQ_HAVE_JPEG2000,[ test $sq_codec_jpeg2000 = yes -a $sq_codec_jpeg2000 = yes ])
AM_CONDITIONAL(SQ_HAVE_OPENEXR, [ test $sq_codec_openexr = yes  -a $sq_codec_openexr  = yes ])
AM_CONDITIONAL(SQ_HAVE_GIF,     [ test $sq_codec_gif = yes      -a $sq_codec_gif      = yes ])

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

if test $sq_have_jpeg = yes;     then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled jpeg `;   else sq_disabled=`echo -n $sq_disabled jpeg `; fi
if test $sq_have_png = yes;      then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled png `;    else sq_disabled=`echo -n $sq_disabled png `; fi
if test $sq_have_xwd = yes;      then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled xwd `;    else sq_disabled=`echo -n $sq_disabled xwd `; fi
if test $sq_have_tiff = yes;     then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled tiff `;   else sq_disabled=`echo -n $sq_disabled tiff `; fi
if test $sq_have_camera = yes;   then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled camera `; else sq_disabled=`echo -n $sq_disabled camera `; fi
if test $sq_have_freetype = yes -a $sq_have_xml = yes -a $sq_have_png = yes; then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled wmf `;    else sq_disabled=`echo -n $sq_disabled wmf `; fi
if test $sq_have_freetype = yes -a $sq_have_xml = yes; then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled svg `;    else sq_disabled=`echo -n $sq_disabled svg `; fi

if test $sq_codec_camera = no;    then sq_manual=`echo -n $sq_manual camera`;   fi
if test $sq_codec_gif = no;       then sq_manual=`echo -n $sq_manual gif`;      fi
if test $sq_codec_svg = no;       then sq_manual=`echo -n $sq_manual svg`;      fi
if test $sq_codec_wmf = no;       then sq_manual=`echo -n $sq_manual wmf`;      fi
if test $sq_codec_openexr = no;   then sq_manual=`echo -n $sq_manual openexr`;  fi
if test $sq_codec_jpeg2000 = no;  then sq_manual=`echo -n $sq_manual jpeg2000`; fi

echo "
  Total formats             : $sq_total
  Optional formats, enabled : $sq_enabled
  Optional formats, disabled: $sq_disabled
  Manually disabled         : $sq_manual

Now run 'make' to compile ksquirrel-libs
 "
