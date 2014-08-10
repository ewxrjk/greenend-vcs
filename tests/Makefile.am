# This file is part of VCS
# Copyright (C) 2009-11, 2014 Richard Kettlewell
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
noinst_PROGRAMS=t-version t-execute t-ltfilename t-utils t-xml t-pager t-editor
dist_noinst_SCRIPTS=t-help t-errors \
	t-bzr t-cvs t-svn t-git t-hg t-darcs t-p4 t-rcs t-sccs \
	bzr-clone git-clone hg-clone \
	dummy-editor
t_version_SOURCES=t-version.cc
t_execute_SOURCES=t-execute.cc
t_ltfilename_SOURCES=t-ltfilename.cc
t_utils_SOURCES=t-utils.cc
t_xml_SOURCES=t-xml.cc
t_pager_SOURCES=t-pager.cc
t_editor_SOURCES=t-editor.cc
LDADD=../src/libvcs.a
AM_CXXFLAGS=-I${top_srcdir}/src
TESTS=t-version t-execute t-ltfilename t-utils t-xml t-pager t-editor \
	t-help t-errors \
	t-bzr t-cvs t-svn t-git t-hg t-darcs t-p4 t-rcs t-sccs \
	bzr-clone git-clone hg-clone
EXTRA_DIST=utils.sh p4.log.1 p4.log.2 p4.log.3 p4.log.4 p4.log.5 p4.log.6

clean-local:
	rm -rf test-root
	rm -f *.gcov *.gcda *.gcno
	rm -f *.log

export srcdir
