AC_INIT([ksquirrel-libs], [0.6.0-pre7], [ksquirrel@tut.by])
#AC_GNU_SOURCE
#AC_CONFIG_SRCDIR([config.h.in])
#AM_INIT_AUTOMAKE([foreign])
#AC_CONFIG_HEADER([config.h])

AC_CONFIG_AUX_DIR([admin])

AC_CANONICAL_SYSTEM
AC_ARG_PROGRAM

AM_INIT_AUTOMAKE(ksquirrel-libs, 0.6.0-pre7)

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

dnl (3) freetype {-lfreetype} [-DHAVE_LIBFREETYPE]

SQ_FT_CFLAGS=""
SQ_FT_LDFLAGS=""

sq_have_freetype="no"
sq_have_jpeg="no"
sq_have_png="no"
sq_have_xwd="no"
sq_have_tiff="no"
sq_have_camera="no"

# Check for freetype

_cppflags=$CPPFLAGS
_ldflags=$LDFLAGS

if [ test -n "$FREETYPE_DIR" ]; then
	AC_PATH_PROG(FREETYPE_CONFIG, freetype-config, ,[$FREETYPE_DIR/bin:$PATH])
else
	AC_PATH_PROG(FREETYPE_CONFIG, freetype-config)
fi

if [ test -n "$FREETYPE_CONFIG" ]; then
	if [ test -n "$FREETYPE_DIR" ]; then
		freetype_cflags="`$FREETYPE_CONFIG --cflags` -I$FREETYPE_DIR/include"
		freetype_libs=`$FREETYPE_CONFIG --libs`
	else
		freetype_cflags=`$FREETYPE_CONFIG --cflags`
		freetype_libs=`$FREETYPE_CONFIG --libs`
	fi
else
	if [ test -n "$FREETYPE_DIR" ]; then
		freetype_cflags="-I$FREETYPE_DIR/include/freetype2 -I$FREETYPE_DIR/include"
		freetype_libs="-L$FREETYPE_DIR/lib -lfreetype"
	else
		freetype_cflags=""
		freetype_libs="-lfreetype"
	fi
fi

CPPFLAGS="$freetype_cflags $CPPFLAGS"
LDFLAGS="$LDFLAGS $freetype_libs"

AC_CHECK_LIB(freetype, FT_Init_FreeType, [
	SQ_FT_LDFLAGS="$freetype_libs"
	sq_have_freetype="yes"
],[	sq_have_freetype="no"])

AC_CHECK_HEADER(ft2build.h, [
	SQ_FT_CFLAGS="$freetype_cflags"
	sq_have_freetype="yes"
],[	sq_have_freetype="no"])

CPPFLAGS=$_cppflags
LDFLAGS=$_ldflags

AC_SUBST([SQ_FT_CFLAGS], $SQ_FT_CFLAGS)
AC_SUBST([SQ_FT_LDFLAGS], $SQ_FT_LDFLAGS)

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

AC_CHECK_HEADERS([netinet/in.h],  [sq_have_camera="yes"], [sq_have_camera="no"])
AC_CHECK_HEADERS([X11/XWDFile.h], [sq_have_xwd="yes"],    [sq_have_xwd="no"])

AC_CONFIG_SUBDIRS([contrib/libjasper])
AC_CONFIG_SUBDIRS([contrib/libopenexr])

if test $sq_have_freetype = yes; then
AC_CONFIG_SUBDIRS([contrib/libwmf])
AC_CONFIG_SUBDIRS([contrib/libpixman])
AC_CONFIG_SUBDIRS([contrib/cairo])
AC_CONFIG_SUBDIRS([contrib/libsvg])
AC_CONFIG_SUBDIRS([contrib/libsvg-cairo])
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

AM_CONDITIONAL(CAMERA_NO_JPEG, [ test "x$sq_have_jpeg" = "xno" ])

AM_CONDITIONAL(SQ_HAVE_JPEG,    [ test "x$sq_have_jpeg" = "xyes" ])
AM_CONDITIONAL(SQ_HAVE_PNG,     [ test "x$sq_have_png" = "xyes" ])
AM_CONDITIONAL(SQ_HAVE_XWD,     [ test "x$sq_have_xwd" = "xyes" ])
AM_CONDITIONAL(SQ_HAVE_TIFF,    [ test "x$sq_have_tiff" = "xyes" ])
AM_CONDITIONAL(SQ_HAVE_CAMERA,  [ test "x$sq_have_camera" = "xyes" ])
AM_CONDITIONAL(SQ_HAVE_SVG,     [ test "x$sq_have_freetype" = "xyes" ])
AM_CONDITIONAL(SQ_HAVE_WMF,     [ test "x$sq_have_freetype" = "xyes" ])
AM_CONDITIONAL(SQ_HAVE_FREETYPE,[ test "x$sq_have_freetype" = "xyes" ])

AC_OUTPUT

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
  libdir:               ${libdir}
  
Libraries:
----------"

sq_enabled=""
sq_disabled=""
sq_total="33"

if test $sq_have_jpeg = yes;     then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled jpeg `;   else sq_disabled=`echo -n $sq_disabled jpeg `; fi
if test $sq_have_png = yes;      then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled png `;    else sq_disabled=`echo -n $sq_disabled png `; fi
if test $sq_have_xwd = yes;      then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled xwd `;    else sq_disabled=`echo -n $sq_disabled xwd `; fi
if test $sq_have_tiff = yes;     then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled tiff `;   else sq_disabled=`echo -n $sq_disabled tiff `; fi
if test $sq_have_camera = yes;   then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled camera `; else sq_disabled=`echo -n $sq_disabled camera `; fi
if test $sq_have_freetype = yes; then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled wmf `;    else sq_disabled=`echo -n $sq_disabled wmf `; fi
if test $sq_have_freetype = yes; then sq_total=`expr $sq_total + 1` && sq_enabled=`echo -n $sq_enabled svg `;    else sq_disabled=`echo -n $sq_disabled svg `; fi

echo "
  Optional formats, enabled:  $sq_enabled
  Optional formats, disabled: $sq_disabled
  Total formats:              $sq_total

Now run 'make' to compile ksquirrel-libs
 "
