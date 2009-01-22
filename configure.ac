# Process this file with autoconf to produce a configure script.
AC_INIT([vcs], [0.DEV], [richard+vcs@sfere.greenend.org.uk])
AC_CONFIG_AUX_DIR([config.aux])
AM_INIT_AUTOMAKE(vcs, [0.DEV])
AC_CONFIG_SRCDIR([vcs.cc])
AM_CONFIG_HEADER([config.h])
AC_PROG_CXX
AC_SET_MAKE
AC_DEFINE([_GNU_SOURCE], [1], [use GNU extensions if available])

if test "$GXX" = yes; then
  CXX="${CXX} -Wall -W"
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

# Local Variables:
# indent-tabs-mode:nil
# End:

