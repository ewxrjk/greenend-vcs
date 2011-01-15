# Process this file with autoconf to produce a configure script.
#
# This file is part of VCS
# Copyright (C) 2009 Richard Kettlewell
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
AC_INIT([vcs], [0.DEV], [richard+vcs@sfere.greenend.org.uk])
AC_CONFIG_AUX_DIR([config.aux])
AM_INIT_AUTOMAKE(vcs, [0.DEV])
AC_CONFIG_SRCDIR([src/vcs.cc])
AM_CONFIG_HEADER([config.h])
AC_PROG_CXX
AC_LANG([C++])
AC_SET_MAKE
AC_PROG_RANLIB
AC_DEFINE([_GNU_SOURCE], [1], [use GNU extensions if available])
AC_DEFINE([PATHSEP], ['/'], [define to path separator character])
AC_DEFINE([PATHSEPSTR], ["/"], [define to path separator as a string])
AC_CHECK_LIB([curl],[curl_easy_init])
AC_CHECK_LIB([expat],[XML_ParserCreate])
# Cygwin's iconv.h redirects to non-standard names, breaking the usual
# AC_CHECK_LIB heuristic.  Stupid Cygwin.
AC_CHECK_LIB([iconv],[iconv_open],[],
             [AC_CHECK_LIB([iconv],[libiconv_open])])
AC_CHECK_HEADERS([curl/curl.h])

# iconv() signature varies between platforms
AC_CACHE_CHECK([for type of iconv inbuf argument],[rjk_cv_iconv_inbuf],
               [AC_TRY_COMPILE([#include <iconv.h>],
                               [char *inbuf;
                                char *outbuf;
                                size_t left;
                                iconv_t cd;
                                iconv(cd, &inbuf, &left, &outbuf, &left);],
                               [rjk_cv_iconv_inbuf="char **"],
                [AC_TRY_COMPILE([#include <iconv.h>],
                                [const char *inbuf;
                                 char *outbuf;
                                 size_t left;
                                 iconv_t cd;
                                 iconv(cd, &inbuf, &left, &outbuf, &left);],
                                [rjk_cv_iconv_inbuf="const char **"],
                                [rjk_cv_iconv_inbuf="unknown"])])])

case "$rjk_cv_iconv_inbuf" in
"char **" )
  ;;
"const char **" )
  AC_DEFINE([HAVE_BROKEN_ICONV],[1],[define if iconv takes const char **inbuf])
  ;;
"unknown" )
  AC_MSG_WARN([cannot determine iconv inbuf type - winging it])
  ;;
esac

RJK_GCOV

WERROR=""
AC_ARG_WITH([werror],
            [AS_HELP_STRING([--without-werror],
                            [Suppress -Werror option])],
            [if test $withval = yes; then WERROR="-Werror"; fi],
            [WERROR="-Werror"])

if test "$GXX" = yes; then
  AX_GXX_VERSION
  AC_CACHE_CHECK([GNU C++ extra options],[rjk_cv_warnings],[
    rjk_cv_warnings="-Wall -W -pedantic $WERROR"
    # g++ only supports -Wmissing-declarations from 4.3 onwards
    case "${GXX_VERSION}" in
    3.* | 4.0 | 4.1 | 4.2 | 4.0.* | 4.1.* | 4.2.* )
      ;;
    * )
      rjk_cv_warnings="${rjk_cv_warnings} -Wmissing-declarations"
      ;;
    esac
  ])
  CXX="${CXX} ${rjk_cv_warnings}"
fi

AH_BOTTOM([#ifdef __GNUC__
# define attribute(x) __attribute__(x)
#else
# define attribute(x)
#endif])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 tests/Makefile])
AC_OUTPUT

# Local Variables:
# indent-tabs-mode:nil
# End:

