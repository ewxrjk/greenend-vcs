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

static int svn_diff(int nfiles, char **files) {
  return execute("svn",
                 EXE_STR, "diff",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int svn_add(int /*binary*/, int nfiles, char **files) {
  return execute("svn",
                 EXE_STR, "add",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int svn_remove(int force, int nfiles, char **files) {
  return execute("svn",
                 EXE_STR, "delete",
                 EXE_IFSTR(force, "--force"),
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int svn_commit(const char *msg, int nfiles, char **files) {
  return execute("svn",
                 EXE_STR, "commit",
                 EXE_IFSTR(msg, "-m"),
                 EXE_IFSTR(msg, msg),
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int svn_revert(int nfiles, char **files) {
  // Subversion's revert insist you tell it what files to revert.  So if
  // we want to revert everything we must cobble together list.
  if(!nfiles) {
    // Establish the current state
    vector<string> status;
    int rc;
    if((rc = capture(status, "svn", "status", "-q", (char *)0)))
      fatal("cvs status exited with status %d", rc);
    vector<char *> files;
    for(size_t n = 0; n < status.size(); ++n) {
      switch(status[n][0]) {
      case 'A':
      case 'C':
      case 'D':
      case 'M':
      case 'R':
      case '!':
      case '~':
        files.push_back(xstrdup(status[n].substr(7).c_str()));
        break;
      }
    }
    return svn_revert((int)files.size(), &files[0]);
  } else
    return execute("svn",
                   EXE_STR, "revert",
                   EXE_STRS, nfiles, files,
                   EXE_END);
}

static int svn_status() {
  return execute("svn",
                 EXE_STR, "status",
                 EXE_END);
}

static int svn_update() {
  return execute("svn",
                 EXE_STR, "update",
                 EXE_END);
}

static int svn_log(const char *path) {
  return execute("svn",
                 EXE_STR, "log",
                 EXE_IFSTR(path, path),
                 EXE_END);
}

const struct vcs vcs_svn = {
  "Subversion",
  svn_diff,
  svn_add,
  svn_remove,
  svn_commit,
  svn_revert,
  svn_status,
  svn_update,
  svn_log,
  NULL,                                 // edit
};

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
