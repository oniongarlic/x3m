AC_INIT()

dnl versions of gstreamer and plugins-base
GST_MAJORMINOR=0.10
GST_REQUIRED=0.10.0
GSTPB_REQUIRED=0.10.0

dnl fill in your package name and version here
dnl the fourth (nano) number should be 0 for a release, 1 for CVS,
dnl and 2... for a prerelease

AM_MAINTAINER_MODE
AM_INIT_AUTOMAKE($PACKAGE, $VERSION)

dnl make aclocal work in maintainer mode
AC_SUBST(ACLOCAL_AMFLAGS, "-I m4")

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AM_CONFIG_HEADER(config.h)

dnl check for tools
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL

dnl Debugging options to enable debug macros
DEBUG_FLAGS=
AC_ARG_ENABLE(debug,     [  --enable-debug                compile with DEBUG],,enable_debug=no)

if test "x$enable_debug" = "xyes"; then
    DEBUG_FLAGS="$DEBUG_FLAGS -DDEBUG -g -fno-omit-frame-pointer"
else
    DEBUG_FLAGS="$DEBUG_FLAGS -DG_DISABLE_CHECKS -DG_DISABLE_CAST_CHECKS -DG_DISABLE_ASSERT"
fi

dnl Check for pkgconfig first
AC_CHECK_PROG(HAVE_PKGCONFIG, pkg-config, yes, no)

dnl Give error and exit if we don't have pkgconfig
if test "x$HAVE_PKGCONFIG" = "xno"; then
  AC_MSG_ERROR(you need to have pkgconfig installed !)
fi

dnl Now we're ready to ask for gstreamer libs and cflags
dnl And we can also ask for the right version of gstreamer

PKG_CHECK_MODULES(GST, \
  gstreamer-$GST_MAJORMINOR >= $GST_REQUIRED,
  HAVE_GST=yes,HAVE_GST=no)

dnl Give error and exit if we don't have gstreamer
if test "x$HAVE_GST" = "xno"; then
  AC_MSG_ERROR(you need gstreamer development packages installed !)
fi

dnl append GST_ERROR cflags to GST_CFLAGS
GST_CFLAGS="$GST_CFLAGS $GST_ERROR"

dnl make GST_CFLAGS and GST_LIBS available
AC_SUBST(GST_CFLAGS)
AC_SUBST(GST_LIBS)

dnl make GST_MAJORMINOR available in Makefile.am
AC_SUBST(GST_MAJORMINOR)

dnl If we need them, we can also use the base class libraries
PKG_CHECK_MODULES(GST_BASE, gstreamer-base-$GST_MAJORMINOR >= $GST_REQUIRED,
                  HAVE_GST_BASE=yes, HAVE_GST_BASE=no)

dnl Give a warning if we don't have gstreamer libs
dnl you can turn this into an error if you need them
if test "x$HAVE_GST_BASE" = "xno"; then
  AC_MSG_NOTICE(no GStreamer base class libraries found (gstreamer-base-$GST_MAJORMINOR))
fi

dnl make _CFLAGS and _LIBS available
AC_SUBST(GST_BASE_CFLAGS)
AC_SUBST(GST_BASE_LIBS)

dnl If we need them, we can also use the gstreamer-plugins-base libraries
PKG_CHECK_MODULES(GSTPB_BASE,
                  gstreamer-plugins-base-$GST_MAJORMINOR >= $GSTPB_REQUIRED,
                  HAVE_GSTPB_BASE=yes, HAVE_GSTPB_BASE=no)

dnl Give a warning if we don't have gstreamer libs
dnl you can turn this into an error if you need them
if test "x$HAVE_GSTPB_BASE" = "xno"; then
  AC_MSG_NOTICE(no GStreamer Plugins Base libraries found (gstreamer-plugins-base-$GST_MAJORMINOR))
fi

dnl make _CFLAGS and _LIBS available
AC_SUBST(GSTPB_BASE_CFLAGS)
AC_SUBST(GSTPB_BASE_LIBS)

dnl If we need them, we can also use the gstreamer-controller libraries
PKG_CHECK_MODULES(GSTCTRL,
                  gstreamer-controller-$GST_MAJORMINOR >= $GSTPB_REQUIRED,
                  HAVE_GSTCTRL=yes, HAVE_GSTCTRL=no)

dnl make _CFLAGS and _LIBS available

AC_OUTPUT(Makefile src/Makefile)

