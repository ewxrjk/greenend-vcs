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
bin_PROGRAMS=vcs
noinst_LIBRARIES=libvcs.a
libvcs_a_SOURCES=guess.cc utils.cc uri.cc ignore.cc vcs.h execute.cc \
	p4utils.h p4utils.cc xml.cc version.cc xml.h
vcs_SOURCES=vcs.cc \
	add.cc remove.cc commit.cc diff.cc revert.cc status.cc update.cc \
	log.cc edit.cc annotate.cc clone.cc \
	cvs.cc git.cc svn.cc bzr.cc p4.cc hg.cc darcs.cc
LDADD=libvcs.a

clean-local:
	rm -f *.gcov *.gcda *.gcno
