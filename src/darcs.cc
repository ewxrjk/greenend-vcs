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

static int darcs_diff(int nfiles, char **files) {
  return execute("darcs",
                 EXE_STR, "diff",
                 EXE_STR, "-u",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int darcs_add(int /*binary*/, int nfiles, char **files) {
  return execute("darcs",
                 EXE_STR, "add",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int darcs_remove(int force, int nfiles, char **files) {
  if(force)
    return execute("rm",
                   EXE_STR, "-f",
                   EXE_STRS, nfiles, files,
                   EXE_END);
  else
    return execute("darcs",
                   EXE_STR, "remove",
                   EXE_STRS, nfiles, files,
                   EXE_END);
}

static int darcs_commit(const char *msg, int nfiles, char **files) {
  return execute("darcs",
                 EXE_STR, "record",
                 EXE_STR, "--all",
                 EXE_IFSTR(msg, "-m"),
                 EXE_IFSTR(msg, msg),
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int darcs_revert(int nfiles, char **files) {
  return execute("darcs",
                 EXE_STR, "revert",
                 EXE_STR, "--all",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int darcs_status() {
  int rc = execute("darcs",
                   EXE_STR, "whatsnew",
                   EXE_STR, "--summary",
                   EXE_END);
  // darcs whatsnew exits non-0 if nothing's changed!  Insane.
  return rc == 1 ? 0 : rc;
}

static int darcs_update() {
  return execute("darcs",
                 EXE_STR, "pull",
                 EXE_STR, "--all",
                 EXE_END);
}

static int darcs_log(const char *path) {
  return execute("darcs",
                 EXE_STR, "changes",
                 EXE_IFSTR(path, path),
                 EXE_END);
}

static int darcs_annotate(const char *path) {
  return execute("darcs",
                 EXE_STR, "annotate",
                 EXE_STR, path,
                 EXE_END);
}

static int darcs_clone(const char *uri, const char *dir) {
  return execute("darcs",
                 EXE_STR, "get",
                 EXE_STR, uri,
                 EXE_IFSTR(dir, dir),
                 EXE_END);
}

const struct vcs vcs_darcs = {
  "Bazaar",
  darcs_diff,
  darcs_add,
  darcs_remove,
  darcs_commit,
  darcs_revert,
  darcs_status,
  darcs_update,
  darcs_log,
  NULL,                                 // edit
  darcs_annotate,
  darcs_clone,
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
