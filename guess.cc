/*
 * This file is part of VCS
 * Copyright (C) 2009 Richard Kettlewell
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "vcs.h"

// Guess what VCS is in use; returns a pointer to its operation table.
// Terminates the process if no VCS can be found.
const struct vcs *guess() {
  // CVS is easy as it has a CVS directory at every level.
  if(isdir("CVS"))
    return &vcs_cvs;
  // Subversion too
  if(isdir(".svn"))
    return &vcs_svn;
  // Bazaar and Git only have their dot directories at the top level of the
  // branch.
  string d = cwd();
  for(;;) {
    if(isdir(d + PATHSEPSTR + ".bzr"))
      return &vcs_bzr;
    if(isdir(d + PATHSEPSTR + ".git"))
      return &vcs_git;
    if(isdir(d + PATHSEPSTR + ".hg"))
      return &vcs_hg;
    if(isroot(d))
      break;
    d = parentdir(d);
  }
  // Only attempt to detect Perforce if some Perforce-specific environment
  // variable is set.  Also, we do it last since invoking a program is much
  // more intrusive than just testing files.
  if(getenv("P4PORT") || getenv("P4CONFIG") || getenv("P4CLIENT")) {
    vector<string> lines;
    if(!capture(lines, "p4", "changes", "-m1", "...", (char *)NULL))
      return &vcs_p4;
  }
  fatal("cannot identify native version control system");
}

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
