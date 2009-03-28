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

static int bzr_diff(int nfiles, char **files) {
  return execute("bzr",
                 EXE_STR, "diff",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int bzr_add(int /*binary*/, int nfiles, char **files) {
  return execute("bzr",
                 EXE_STR, "add",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int bzr_remove(int force, int nfiles, char **files) {
  return execute("bzr",
                 EXE_STR, "remove",
                 EXE_IFSTR(force, "--force"),
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int bzr_commit(const char *msg, int nfiles, char **files) {
  return execute("bzr",
                 EXE_STR, "commit",
                 EXE_IFSTR(msg, "-m"),
                 EXE_IFSTR(msg, msg),
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int bzr_revert(int nfiles, char **files) {
  return execute("bzr",
                 EXE_STR, "revert",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int bzr_status() {
  return execute("bzr",
                 EXE_STR, "status",
                 EXE_END);
}

static int bzr_update() {
  return execute("bzr",
                 EXE_STR, "update",
                 EXE_END);
}

static int bzr_log(const char *path) {
  return execute("bzr",
                 EXE_STR, "log",
                 EXE_IFSTR(path, path),
                 EXE_END);
}

const struct vcs vcs_bzr = {
  "Bazaar",
  bzr_diff,
  bzr_add,
  bzr_remove,
  bzr_commit,
  bzr_revert,
  bzr_status,
  bzr_update,
  bzr_log,
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
