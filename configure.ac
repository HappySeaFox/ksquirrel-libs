AC_INIT([ksquirrel-libs], [0.6.0-pre5], [ksquirrel@tut.by])
#AC_GNU_SOURCE
#AC_CONFIG_SRCDIR([config.h.in])
#AM_INIT_AUTOMAKE([foreign])
#AC_CONFIG_HEADER([config.h])

AC_CONFIG_AUX_DIR([admin])

AC_CANONICAL_SYSTEM
AC_ARG_PROGRAM

AM_INIT_AUTOMAKE(ksquirrel-libs, 0.6.0-pre5)

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

sq_have_jpeg="yes"
sq_have_png="yes"
sq_have_xwd="yes"
sq_have_tiff="yes"
sq_have_camera="yes"

AC_CHECK_LIB([jpeg], [main], [], [sq_have_jpeg="no"])
#AC_CHECK_LIB([png], [png_create_read_struct], [], [sq_have_png="no"])

AC_CHECK_HEADERS([jpeglib.h], [], [sq_have_jpeg="no"])
AC_CHECK_HEADERS([netinet/in.h], [], [sq_have_camera="no"])
AC_CHECK_HEADERS([png.h], [], [sq_have_png="no"])
AC_CHECK_HEADERS([X11/XWDFile.h], [], [sq_have_xwd="no"])
AC_CHECK_HEADERS([tiffio.h tiff.h], [], [sq_have_tiff="no"])

AC_CONFIG_SUBDIRS([contrib/libwmf])
AC_CONFIG_SUBDIRS([contrib/libpixman])
AC_CONFIG_SUBDIRS([contrib/cairo])
AC_CONFIG_SUBDIRS([contrib/libsvg])
AC_CONFIG_SUBDIRS([contrib/libsvg-cairo])
AC_CONFIG_SUBDIRS([contrib/libjasper])
AC_CONFIG_SUBDIRS([contrib/libopenexr])

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
AC_CONFIG_FILES([kernel/wmf/Makefile])
AC_CONFIG_FILES([kernel/wbmp/Makefile])
AC_CONFIG_FILES([kernel/sun/Makefile])
AC_CONFIG_FILES([kernel/svg/Makefile])
AC_CONFIG_FILES([kernel/jpeg2000/Makefile])
AC_CONFIG_FILES([kernel/openexr/Makefile])
AC_CONFIG_FILES([kernel/koala/Makefile])
AC_CONFIG_FILES([kernel/wal/Makefile])
AC_CONFIG_FILES([kernel/mdl/Makefile])
AC_CONFIG_FILES([kernel/hdr/Makefile])
AC_CONFIG_FILES([kernel/sct/Makefile])
AC_CONFIG_FILES([kernel/lif/Makefile])

if test $sq_have_png = yes; then
AC_CONFIG_FILES([kernel/png/Makefile])
fi

if test $sq_have_jpeg = yes; then
AC_CONFIG_FILES([kernel/jpeg/Makefile])
fi

AM_CONDITIONAL(CAMERA_NO_JPEG, [ test "x$sq_have_jpeg" = "xno" ])

if test $sq_have_xwd = yes; then
AC_CONFIG_FILES([kernel/xwd/Makefile])
fi

if test $sq_have_tiff = yes; then
AC_CONFIG_FILES([kernel/tiff/Makefile])
fi

if test $sq_have_camera = yes; then
AC_CONFIG_FILES([kernel/camera/Makefile])
fi

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

sq_enabled="bmp gif pix xbm xpm tga sgi ras pnm pcx ico xcur psd fli iff utah cut rawrgb wmf wbmp sun svg jpeg2000 openexr koala wal mdl hdr sct lif"
sq_disabled=""

if test $sq_have_jpeg = yes; then sq_enabled=`echo -n $sq_enabled jpeg `; else sq_disabled=`echo -n $sq_disabled jpeg `; fi
if test $sq_have_png = yes; then sq_enabled=`echo -n $sq_enabled png `; else sq_disabled=`echo -n $sq_disabled png `; fi
if test $sq_have_xwd = yes; then sq_enabled=`echo -n $sq_enabled xwd `; else sq_disabled=`echo -n $sq_disabled xwd `; fi
if test $sq_have_tiff = yes; then sq_enabled=`echo -n $sq_enabled tiff `; else sq_disabled=`echo -n $sq_disabled tiff `; fi
if test $sq_have_camera = yes; then sq_enabled=`echo -n $sq_enabled camera `; else sq_disabled=`echo -n $sq_disabled camera `; fi

echo "Enabled:  $sq_enabled"
echo "Disabled: $sq_disabled"
echo
echo "Now run 'make' to compile libraries"
echo
