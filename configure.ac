# Process this file with autoconf to produce a configure script.
AC_INIT([vcs], [0.DEV], [richard+vcs@sfere.greenend.org.uk])
AC_CONFIG_AUX_DIR([config.aux])
AM_INIT_AUTOMAKE(vcs, [0.DEV])
AC_CONFIG_SRCDIR([vcs.cc])
AM_CONFIG_HEADER([config.h])
AC_PROG_CXX
AC_SET_MAKE
AC_DEFINE([_GNU_SOURCE], [1], [use GNU extensions if available])
AC_DEFINE([PATHSEP], ['/'], [define to path separator character])
AC_DEFINE([PATHSEPSTR], ["/"], [define to path separator as a string])

if test "$GXX" = yes; then
  CXX="${CXX} -Wall -W"
fi

AH_BOTTOM([#ifdef __GNUC__
# define attribute(x) __attribute__(x)
#else
# define attribute(x)
#endif])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

# Local Variables:
# indent-tabs-mode:nil
# End:

