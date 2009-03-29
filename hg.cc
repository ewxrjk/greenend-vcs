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

static int hg_diff(int nfiles, char **files) {
  return execute("hg",
                 EXE_STR, "diff",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int hg_add(int /*binary*/, int nfiles, char **files) {
  return execute("hg",
                 EXE_STR, "add",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int hg_remove(int force, int nfiles, char **files) {
  return execute("hg",
                 EXE_STR, "remove",
                 EXE_IFSTR(force, "--force"),
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int hg_commit(const char *msg, int nfiles, char **files) {
  return execute("hg",
                 EXE_STR, "commit",
                 EXE_IFSTR(msg, "-m"),
                 EXE_IFSTR(msg, msg),
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int hg_revert(int nfiles, char **files) {
  if(nfiles)
    return execute("hg",
                   EXE_STR, "revert",
                   EXE_STRS, nfiles, files,
                   EXE_END);
  else
    return execute("hg",
                   EXE_STR, "revert",
                   EXE_STR, "--all",
                   EXE_END);
}

static int hg_status() {
  return execute("hg",
                 EXE_STR, "status",
                 EXE_END);
}

static int hg_update() {
  return execute("hg",
                 EXE_STR, "pull",
		 EXE_STR, "--update",
                 EXE_END);
}

static int hg_log(const char *path) {
  return execute("hg",
                 EXE_STR, "log",
                 EXE_IFSTR(path, path),
                 EXE_END);
}

static int hg_annotate(const char *path) {
  return execute("hg",
                 EXE_STR, "annotate",
                 EXE_STR, path,
                 EXE_END);
}

static int hg_clone(const char *uri, const char *dir) {
  return execute("hg",
                 EXE_STR, "clone",
                 EXE_STR, uri,
                 EXE_IFSTR(dir, dir),
                 EXE_END);
}

const struct vcs vcs_hg = {
  "Mercurial",
  hg_diff,
  hg_add,
  hg_remove,
  hg_commit,
  hg_revert,
  hg_status,
  hg_update,
  hg_log,
  NULL,                                 // edit
  hg_annotate,
  hg_clone,
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
