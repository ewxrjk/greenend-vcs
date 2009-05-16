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
AC_SET_MAKE
AC_PROG_RANLIB
AC_DEFINE([_GNU_SOURCE], [1], [use GNU extensions if available])
AC_DEFINE([PATHSEP], ['/'], [define to path separator character])
AC_DEFINE([PATHSEPSTR], ["/"], [define to path separator as a string])
AC_CHECK_LIB([curl],[curl_easy_init])
AC_CHECK_LIB([expat],[XML_ParserCreate])
AC_CHECK_LIB([iconv],[iconv_open])
AC_CHECK_HEADERS([curl/curl.h])

RJK_GCOV

if test "$GXX" = yes; then
  CXX="${CXX} -Wall -W -Wmissing-declarations -pedantic -Werror"
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

