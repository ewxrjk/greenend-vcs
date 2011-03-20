#! /bin/bash
#
# This file is part of VCS
# Copyright (C) 2009, 2010 Richard Kettlewell
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

set -e

# Find an automake
if [ -z "$AUTOMAKE" ]; then
  for prog in automake automake-1.10 automake-1.9 automake-1.8 automake-1.7; do
    if type $prog >/dev/null 2>&1; then
      AUTOMAKE=$prog
      break
    fi
  done
  if [ -s "$AUTOMAKE" ]; then
    echo "ERROR: no automake found" >&2
    exit 1
  fi
fi
ACLOCAL=${AUTOMAKE/automake/aclocal}

srcdir=$(dirname $0)
here=$(pwd)
cd $srcdir
mkdir -p config.aux
if test -d $HOME/share/aclocal; then
  ${ACLOCAL} --acdir=$HOME/share/aclocal
else
  ${ACLOCAL}
fi
#libtoolize
autoconf
autoheader
${AUTOMAKE} -a || true		# for INSTALL
${AUTOMAKE} --foreign -a

