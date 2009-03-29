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


t_init() {
    # Check all dependencies are present
    for dep; do
        if type $dep >/dev/null 2>&1; then
            :
        else
            echo "Cannot test $1 - $dep is not installed" >&2
            exit 77
        fi
    done

    # Make sure vcs is on the path
    builddir=`pwd`
    PATH=$builddir:$PATH

    testdir=$builddir/test-root/$1

    # Clean up droppings
    rm -rf $testdir

    # Enter the test directory
    mkdir -p $testdir
    cd $testdir
}

t_done() {
    cd /
#    rm -rf $testdir
}

t_populate() {
    cd project
    echo one > one
    echo two > two
    x vcs -v add one two
    x vcs -v commit -m 'one and two'
    cd ..
}

t_modify() {
    cd project
    x vcs -v edit one
    echo oneone >> one
    x vcs -v edit two
    echo twotwo >> two
    x vcs -v commit -m 'oneone' one
    x vcs -v revert two
    cd ..
}

t_update() {
    cd copy
    x vcs -v up
    cd ..
}

t_verify() {
    if diff project/one copy/one; then :; else exit 1; fi
    if diff project/two copy/two; then :; else exit 1; fi
}

t_revert() {
    cd copy
    x vcs -v edit one
    echo extra >> one
    x vcs -v rm -f two
    if [ -e two ]; then
	echo "removed 'two' but it's still there" >&2
	exit 1
    fi
    echo three > three
    x vcs -v add three
    x vcs -v revert
    cd ..

    # 'one' and 'two' should be back to normal; we already have a
    # check for that
    t_verify

    # 'three' should not exist or not be under vc (at all).  The check will
    # have to be vc-specific.
}

x() {
    echo ">>>" "$@" >&2
    "$@"
}

fatal() {
    echo "$@" >&2
    exit 1
}
