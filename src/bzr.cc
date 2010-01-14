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
  vector<string> info;
  int rc;
  if((rc = capture(info, "bzr", "info", (char *)NULL)))
    fatal("'bzr info' exited with status %d", rc);
  if(info.size() > 0
     && info[0].compare(0, 8, "Checkout") == 0)
    return execute("bzr",
                 EXE_STR, "up",
                 EXE_END);
  else
    return execute("bzr",
                   EXE_STR, "pull",
                   EXE_END);
}

static int bzr_log(const char *path) {
  return execute("bzr",
                 EXE_STR, "log",
                 EXE_IFSTR(path, path),
                 EXE_END);
}

static int bzr_annotate(const char *path) {
  return execute("bzr",
                 EXE_STR, "annotate",
                 EXE_STR, path,
                 EXE_END);
}

static int bzr_clone(const char *uri, const char *dir) {
  return execute("bzr",
                 EXE_STR, "clone",
                 EXE_STR, uri,
                 EXE_IFSTR(dir, dir),
                 EXE_END);
}

static int bzr_rename(int nsources, char **sources, const char *destination) {
  return execute("bzr",
                 EXE_STR, "mv",
                 EXE_STRS, nsources, sources,
                 EXE_STR, destination,
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
  bzr_annotate,
  bzr_clone,
  bzr_rename,
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
