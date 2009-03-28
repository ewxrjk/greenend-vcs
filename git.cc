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

static int git_diff(int nfiles, char **files) {
  /* 'vcs diff' wants the difference between the working tree and the head (not
   * between the index and anything) */
  return execute("git",
                 EXE_STR, "diff",
                 EXE_STR, "HEAD",
                 EXE_STR, "--",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int git_add(int /*binary*/, int nfiles, char **files) {
  /* 'git add' is a bit unlike 'vcs add' in that it actually stages files for
   * later commit.  But it's also the only (native) way to mark previously
   * uncontrolled files as version-controlled, so we go with it anyway. */
  return execute("git",
                 EXE_STR, "add",
                 EXE_STR, "--",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int git_remove(int force, int nfiles, char **files) {
  return execute("git",
                 EXE_STR, "rm",
                 EXE_IFSTR(force, "-f"),
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int git_commit(const char *msg, int nfiles, char **files) {
  if(nfiles == 0) {
    /* Automatically stage and commit everything in sight */
    return execute("git",
                   EXE_STR, "commit",
                   EXE_STR, "-a",
                   EXE_IFSTR(msg, "-m"),
                   EXE_IFSTR(msg, msg),
                   EXE_END);
  } else {
    /* Just commit the named files */
    return execute("git",
                   EXE_STR, "commit",
                   EXE_IFSTR(msg, "-m"),
                   EXE_IFSTR(msg, msg),
                   EXE_STR, "--",
                   EXE_STRS, nfiles, files,
                   EXE_END);
  }
}

static int git_revert(int nfiles, char **files) {
  return execute("git",
                 EXE_STR, "reset",
                 EXE_STR, "--hard",
                 EXE_STR, "HEAD",
                 EXE_STR, "--",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int git_status() {
  return execute("git",
                 EXE_STR, "status",
                 EXE_END);
}

static int git_update() {
  return execute("git",
                 EXE_STR, "pull",
                 EXE_END);
}

static int git_log(const char *path) {
  return execute("git",
                 EXE_STR, "log",
                 EXE_IFSTR(path, path),
                 EXE_END);
}

const struct vcs vcs_git = {
  "Git",
  git_diff,
  git_add,
  git_remove,
  git_commit,
  git_revert,
  git_status,
  git_update,
  git_log,
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
