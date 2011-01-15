# This file is part of VCS.
# Copyright (C) 2007, 2009 Richard Kettlewell
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
#

AC_DEFUN([RJK_GCOV],[
  GCOV=${GCOV:-true}
  AC_ARG_WITH([gcov],
              [AS_HELP_STRING([--with-gcov],
                              [Enable coverage testing])],
              [if test $withval = yes; then
                 CXXFLAGS="${CXXFLAGS} -O0 -fprofile-arcs -ftest-coverage"
                 LIBS="${LIBS} -lgcov"
                 GCOV=`echo $CXX | sed s'/g++/gcov/;s/ .*$//'`;
               fi])
  AC_SUBST([GCOV],[$GCOV])
])

# ===========================================================================
#      http://www.gnu.org/software/autoconf-archive/ax_gxx_version.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_GXX_VERSION
#
# DESCRIPTION
#
#   This macro retrieves the g++ version and returns it in the GXX_VERSION
#   variable if available, an empty string otherwise.
#
# LICENSE
#
#   Copyright (c) 2008 Francesco Salvestrini <salvestrini@users.sourceforge.net>
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation; either version 2 of the License, or (at your
#   option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#   Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program. If not, see <http://www.gnu.org/licenses/>.
#
#   As a special exception, the respective Autoconf Macro's copyright owner
#   gives unlimited permission to copy, distribute and modify the configure
#   scripts that are the output of Autoconf when processing the Macro. You
#   need not follow the terms of the GNU General Public License when using
#   or distributing such scripts, even though portions of the text of the
#   Macro appear in them. The GNU General Public License (GPL) does govern
#   all other use of the material that constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the Autoconf
#   Macro released by the Autoconf Archive. When you make and distribute a
#   modified version of the Autoconf Macro, you may extend this special
#   exception to the GPL to apply to your modified version as well.

## Modified by RJK 2011-01-15 to avoid use of AX_GCC_OPTION and to
## improve output.

AC_DEFUN([AX_GXX_VERSION], [
  GXX_VERSION=""
  AS_IF([test "x$GXX" = "xyes"],[
    AC_CACHE_CHECK([GNU C++ version],[ax_cv_gxx_version],[
      ax_cv_gxx_version="`$CXX -dumpversion`"
      AS_IF([test "x$ax_cv_gxx_version" = "x"],[
	ax_cv_gxx_version=""
      ])
    ])
    GXX_VERSION=$ax_cv_gxx_version
  ])
  AC_SUBST([GXX_VERSION])
])
