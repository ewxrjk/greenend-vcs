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


t_init() {

    # Check all dependencies are present
    for dep; do
        case $dep in
        http:* )
            if curl -m ${CONNECTMAX:-5} -s -f --head "$dep" \
                 >/dev/null 2>&1; then
                :
            else
                echo "Cannot run test $1 - can't reach $dep" >&2
                exit 77
            fi
            ;;
        * )
            if type $dep >/dev/null 2>&1; then
                :
            else
                echo "Cannot run test $1 - $dep is not installed" >&2
                exit 77
            fi
            ;;
        esac
    done

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
    x vcs -v status > ../before
    x vcs -n add one two
    x vcs -v status > ../after
    if cmp -s ../before ../after; then
        :
    else
        echo "vcs -n add unexpectedly changed status output" >&2
        cd ../
        diff -u before after
    fi
    x vcs -v add one two
    x vcs -v status
    x vcs -v commit -m 'one and two'
    mkdir subdir
    x vcs -v add subdir
    cd subdir
    echo subone > subone
    x vcs add subone
    x vcs commit -m 'added subone'
    echo subtwo > subtwo
    cd ../
    x vcs add subdir/subtwo
    x vcs commit -m 'added saubtwo'
    cd ..
}

t_modify() {
    cd project
    if [ -w one ]; then
        writable=true
    else
        writable=false
    fi
    x vcs -n edit one
    if $writable; then
        :
    else
        if [ -w one ]; then
            echo "vcs -n edit made file writable" >&2
            ls -l one
            exit 1
        fi
    fi
    x vcs -v edit one
    if [ -w one ]; then
        :
    else
        echo "vcs -n edit did not make file writable" >&2
        ls -l one
        exit 1
    fi
    echo oneone >> one
    x vcs -v status
    x vcs -v diff > diff-output-1 || true
    if [ -s diff-output-1 ]; then
        :
    else
        echo Modified one but no diff output >&2
        exit 1
    fi
    x vcs -n commit -m 'oneone'
    x vcs -v diff > diff-output-2 || true
    if [ -s diff-output-2 ]; then
        :
    else
        echo Diff output went away after supposedly dry-run commit >&2
        exit 1
    fi
    x vcs -v commit -m 'oneone'
    x vcs -v diff > diff-output-3 || true
    # Stupid darcs appends a blank line to diff output even if it
    # would otherwise be empty.  As a hack we only consider nonblank
    # lines when determine if the output is empty.
    sed '/^$/d' < diff-output-3 > diff-output-3a
    if [ -s diff-output-3a ]; then
        echo Diff output remains after commit >&2
        sed 's/^/| /' < diff-output-3
        exit 1
    fi
    cd ..
}

t_update() {
    cd copy
    if [ `wc -l < one` != 1 ]; then
        echo "'one' has unexpected length"
        sed 's/^/| /' < one
        exit 1
    fi
    x vcs -n up
    if [ `wc -l < one` != 1 ]; then
        echo "'one' has wrong length after dry-run update"
        sed 's/^/| /' < one
        exit 1
    fi
    x vcs -v up
    cd ..
}

t_verify() {
    if diff -u project/one copy/one; then :; else exit 1; fi
    if diff -u project/two copy/two; then :; else exit 1; fi
}

t_revert() {
    cd copy
    x vcs -v edit one
    echo extra >> one
    x vcs -n rm -f two
    if [ ! -e two ]; then
        echo "dry-run remove removed 'two'" >&2
        exit 1
    fi
    x vcs -v rm -f two
    if [ -e two ]; then
	echo "removed 'two' but it's still there" >&2
	exit 1
    fi
    echo three > three
    x vcs -v add three
    x vcs -v status
    x vcs -n revert
    if [ ! -e three ]; then
        echo "Dry-run revert removed 'three'" >&2
        exit 1
    fi
    if grep -q extra one; then
        :
    else
        echo "Dry-run revert modified 'one'" >&2
        sed 's/^/| ' < one
        exit 1
    fi
    if [ -e two ]; then
        echo "Dry-run revert restored 'two'" >&2
        sed 's/^/| ' < two
        exit 1
    fi
    x vcs -v revert
    x vcs -v status
    cd ..

    # 'one' and 'two' should be back to normal; we already have a
    # check for that
    t_verify

    # Explicit paths 1: modified file
    cd copy
    x vcs edit one
    echo extra >> one
    x vcs -v revert one
    x vcs -v status
    cd ..
    t_verify

    # Explicit paths 2: deleted file
    cd copy
    x vcs -v rm one
    x vcs -v revert one
    x vcs -v status
    cd ..
    t_verify

    # Explicit paths 3: added file
    cd copy
    echo three > three
    x vcs -v add three
    x vcs -v status
    x vcs -v revert three
    x vcs -v status
    cd ..
    t_verify

    # 'three' should not exist or not be under vc (at all).  The check will
    # have to be vc-specific.
}

check_match() {
    set +e
    x diff -u "$@"
    rc=$?
    set -e
    if test $rc != 0; then
        echo "*** UNEXPECTED DIFFERENCE FOUND ***"
        exit 1
    fi
}

x() {
    echo ">>> PWD=`pwd`" >&2
    echo ">>>" "$@" >&2
    "$@"
}

xfail() {
    echo "!!! PWD=`pwd`" >&2
    echo "!!!" "$@" >&2
    if "$@"; then
      echo "*** UNEXPECTEDLY SUCCEEDED ***"
      exit 1
    fi
}
  

fatal() {
    echo "$@" >&2
    exit 1
}

# Sanitize environment
unset EMAIL || true
unset VCS_PAGER || true
unset VCS_DIFF_PAGER || true
unset P4CLIENT || true
unset P4CONFIG || true
unset P4JOURNAL || true
unset P4PASSWD || true
unset P4PORT || true
unset P4ROOT || true
unset P4TICKETS || true
unset P4USER || true

# Make sure vcs is on the path
builddir=`pwd`
PATH=$builddir/src:$PATH

if ${TESTLOG:-false}; then
  exec > ${0##*/}.log 2>&1
fi
