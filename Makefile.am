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
vcs_SOURCES=vcs.cc guess.cc utils.cc uri.cc ignore.cc vcs.h execute.cc \
	p4utils.h p4utils.cc \
	add.cc remove.cc commit.cc diff.cc revert.cc status.cc update.cc \
	log.cc edit.cc annotate.cc clone.cc version.cc \
	cvs.cc git.cc svn.cc bzr.cc p4.cc hg.cc darcs.cc
vcs_LDADD=$(LIBCURL)
dist_man_MANS=vcs.1
noinst_PROGRAMS=t-version t-execute t-ltfilename
dist_noinst_SCRIPTS=tests/help tests/errors \
	tests/bzr tests/cvs tests/svn tests/git tests/hg tests/darcs tests/p4
t_version_SOURCES=utils.cc version.cc t-version.cc
t_execute_SOURCES=utils.cc execute.cc t-execute.cc
t_ltfilename_SOURCES=utils.cc execute.cc p4utils.cc t-ltfilename.cc
TESTS=t-version t-execute t-ltfilename \
	tests/help tests/errors \
	tests/bzr tests/cvs tests/svn tests/git tests/hg tests/darcs tests/p4
EXTRA_DIST=CHANGES.html debian/changelog debian/control debian/copyright \
	debian/rules tests/utils.sh scripts/setversion \
	README.developers scripts/coverage scripts/dist scripts/htmlman

clean-local:
	rm -rf test-root
	rm -f *.gcov *.gcda *.gcno
	rm -f help.log errors.log bzr.log cvs.log svn.log git.log hg.log
	rm -f darcs.log p4.log

coverage: all
	rm -f *.gcov
	rm -f *.gcda
	$(MAKE) check
	${GCOV} *.cc | ${srcdir}/scripts/coverage
