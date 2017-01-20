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
dist_man_MANS=vcs.1
EXTRA_DIST=CHANGES.html debian/changelog debian/control debian/copyright \
	debian/override.vcs \
	debian/rules scripts/setversion \
	images/vcs-55.png images/vcs-55.svg \
	README.developers scripts/coverage scripts/dist scripts/htmlman
SUBDIRS=src tests

clean-local:
	rm -rf test-root
	rm -f *.gcov *.gcda *.gcno
	rm -f help.log errors.log bzr.log cvs.log svn.log git.log hg.log
	rm -f darcs.log p4.log

coverage: all
	rm -f *.gcov */*.gcov
	rm -f *.gcda */*.gcda
	$(MAKE) check
	(cd src && ${GCOV} *.cc) | ${srcdir}/scripts/coverage

echo-distdir:
	@echo $(distdir)

echo-version:
	@echo $(VERSION)
