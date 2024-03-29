# This file is part of VCS
# Copyright (C) 2009-2011, 2014 Richard Kettlewell
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


# t_init [DEPENDENCY ...]
#
# DEPENDENCY can be an http:... URL, which must be reachable using
# curl, or a command name, which must be on the path.  If a dependency
# cannot be satisifed then the test is skipped (exit 77).
t_init() {

    echo "--- t_init"
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

    testdir=$workdir/test-root/$1

    # Enter the test directory
    mkdir -p $testdir
    cd $testdir
}

# Called at the end of tests to clean up.
#
# In fact at present, not cleanup is done; it's convenine to see what
# is left behind, for further testing.
t_done() {
    echo "--- t_done"
    cd /
}

# Create an initial test project.
#
# On entry, 'project' should exist and be initialized into the system
# under test (i.e. bzr init or whatever).  It will be populated as
# follows over the course of three commits:
#    project/one
#    project/two
#    project/subdir/
#    project/subdir/subone
#    project/subdir/subtwo
# Each file's contents will be its name (plus a newline).
t_populate() {
    echo "--- t_populate"
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
    case "$0" in
    *-git )
      # git cannot diff the initial commit!
      ;;
    * )
      echo Global diff should show all new files
      x vcs -v diff > delta || true
      cat delta
      grep '^+one' delta > /dev/null
      grep '^+two' delta > /dev/null
      echo Single-file diff should only show that file\'s changes
      x vcs -v diff one > delta || true
      cat delta
      grep '^+one' delta > /dev/null
      grep '^+two' delta && exit 1
      ;;
    esac
    x vcs -v commit -m 'one and two'

    echo Add a subdirectory and one file
    mkdir subdir
    x vcs -v add subdir
    cd subdir
    echo subone > subone
    x vcs add subone
    x vcs -v diff > delta || true
    cat delta
    grep '^+subone' delta
    x vcs commit -m 'added subone'

    echo Add a second file in the subdirectory
    echo subtwo > subtwo
    cd ../
    x vcs add subdir/subtwo
    x vcs -v diff > delta || true
    cat delta
    echo Existing file should not show as added
    grep '^+subone' delta && exit 1
    echo New file should show as added
    grep '^+subtwo' delta > /dev/null
    x vcs -v diff subdir/subtwo > delta || true
    cat delta
    echo Explicit diff of existing file should list it
    grep '^+subtwo' delta > /dev/null
    echo Explicit diff of existing file should not list anything else
    grep '^+subone' delta && exit 1
    grep '^+one' delta && exit 1
    grep '^+two' delta && exit 1
    x vcs commit -m 'added subtwo'
    cd ..
}

# Modify the test project.
#
# On entry, 'project' should be as per the result of t_populate.
# project/one will have 'oneone' appended.  Most of the work is
# examining differences rather than making changes.
#
t_modify() {
    echo "--- t_modify"
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
    x vcs -v edit two
    echo twotwo >> two
    x vcs -v status
    x vcs -v diff > delta || true
    cat delta
    grep '^+oneone' delta
    grep '^+twotwo' delta
    x vcs -n commit -m 'oneone'
    x vcs -v diff > diff-output-2 || true
    if [ -s diff-output-2 ]; then
        :
    else
        echo Diff output went away after supposedly dry-run commit >&2
        exit 1
    fi
    x vcs -v commit -m 'oneone' one
    echo Should only have commited \'one\'
    x vcs -v diff > delta || true
    cat delta
    grep '^+twotwo' delta
    x vcs -v revert two
    echo Should be nothing left
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

# Update a copy of the test project
#
# On entry 'copy' should be a checked out duplicate of 'project'.  For
# DVCS systems this will be a branch (clone); for p4 it is a checkout
# in a separate client.  'copy' is updated, presumably to match 'project',
# though this is not verified.
t_update() {
    echo "--- t_update"
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

# Compare files in the test project and its copy
#
# Checks that .../one and .../two match in 'project' and 'copy'.  Used
# by t_revert.
t_verify() {
    echo "--- t_verify"
    if diff -u project/one copy/one; then :; else exit 1; fi
    if diff -u project/two copy/two; then :; else exit 1; fi
}

# Test 'vcs revert'.
#
# On entry 'copy' should be an up to date duplicate of 'project'.
# This test modfies various files 'copy' and then reverts them, using
# the contents of 'project' to verify that the revert succeeded.
#
# Changes that are subsequently reverted are:
#   - modify copy/one
#   - remove copy/two
#   - add new copy/three
#
# This is done more than one, to test global revert and file-specific
# revert.
t_revert() {
    echo "--- t_revert"
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

    echo 'one' and 'two' should be back to normal
    # we already have a check for that
    t_verify

    echo Explicit paths 1: modified file
    cd copy
    x vcs edit one
    echo extra >> one
    x vcs -v revert one
    x vcs -v status
    cd ..
    t_verify

    echo Explicit paths 2: deleted file
    cd copy
    x vcs -v rm two
    x vcs -v revert two
    x vcs -v status
    cd ..
    t_verify

    echo Explicit paths 3: added file
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

# Test renaming.
#
# Tests renaming within 'project'.  Currently discards eventual changes,
# which is not a very good test!  TODO
t_rename() {
    echo "--- t_rename"
    cd project
    x vcs -v rename one two subdir
    for x in one two; do
        if [ -e $x ]; then
            echo "rename did not delete $x" >&2
            exit 1
        fi
        if [ ! -e subdir/$x ]; then
            echo "rename did not create subdir/$x" >&2
            exit 1
        fi
    done
    x vcs -v revert
    x vcs -v rename one four
    if [ -e one ]; then
        echo "rename did not delete one" >&2
        exit 1
    fi
    if [ ! -e four ]; then
        echo "rename did not create four" >&2
        exit 1
    fi
    x vcs -v revert
    rm -f four
    cd ..
}

# Test awkward filenames.
#
# Tries to add various awkward filenames and verifies that it all works
t_awkward() {
  echo "--- t_awkward"
  cd project
  echo > foo@bar
  echo > foo\#bar%wibble\*spong
  echo > -addme
  x vcs -v status
  x vcs -v add foo*
  x vcs -v add -- -addme
  x vcs -v commit -m 'files with awkward names'
  cd ..
}

# check_match FILE-A FILE-B
#
# Equivalent to diff -u but terminates the test if a difference is found.
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

# x COMMAND [ARGUMENT ...]
#
# Echo a command and then execute it
x() {
    echo ">>> PWD=`pwd`" >&2
    echo ">>>" "$@" >&2
    "$@"
}

# xfail COMMAND [ARGUMENT ...]
#
# Execute a command and check that it does NOT succeed
xfail() {
    echo "!!! PWD=`pwd`" >&2
    echo "!!!" "$@" >&2
    if "$@"; then
      echo "*** UNEXPECTEDLY SUCCEEDED ***"
      exit 1
    fi
}

# fatal MESSAGE...
#
# Exit with a fatal error
fatal() {
    echo "$@" >&2
    exit 1
}

# makep4client NAME ROOT
#
# Create a p4 client called NAME rooted at ROOT.
makep4client() {
  echo "--- makep4client"
  local NAME="$1"
  local ROOT="$2"
  local LOGNAME=${LOGNAME:-`whoami`}
  local HOSTNAME=$(uname -n)

  p4 client -i <<EOF
Client: $NAME
Owner: $LOGNAME
Host: $HOSTNAME
Root: $ROOT
Options: noallwrite noclobber nocompress unlocked nomodtime normdir
SubmitOptions:	submitunchanged
LineEnd: local
View:
        //depot/... //$NAME/...
EOF
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

# Make srcdir absolute
srcdir=${srcdir:-.}
case "$srcdir" in
/* )
  ;;
* )
  srcdir=`cd $srcdir && pwd`
  ;;
esac

# Make sure vcs is on the path
builddir=`pwd`
PATH=$builddir/../src:$PATH

# Run in a private directory
workdir=`mktemp -d $builddir/tmp.XXXXXX`
trap "rm -rf $workdir" EXIT
cd $workdir
pwd
