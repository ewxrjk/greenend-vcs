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

static int p4_edit(int nfiles, char **files) {
  return execute("p4",
                 EXE_STR, "edit",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int p4_diff(int nfiles, char **files) {
  return execute("p4",
                 EXE_STR, "diff",
		 EXE_STR, "-du",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int p4_add(int /*binary*/, int nfiles, char **files) {
  return execute("p4",
                 EXE_STR, "add",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int p4_remove(int /*force*/, int nfiles, char **files) {
  return execute("p4",
                 EXE_STR, "delete",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int p4_commit(const char *msg, int nfiles, char **files) {
  if(msg)
    fatal("'vcs commit -m' does not work with Perforce backend, sorry");
  return execute("p4",
                 EXE_STR, "submit",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int p4_revert(int nfiles, char **files) {
  return execute("p4",
                 EXE_STR, "revert",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int p4_status() {
  return execute("p4",
                 EXE_STR, "opened",
                 EXE_END);
}

static int p4_update() {
  return execute("p4",
                 EXE_STR, "sync",
		 EXE_STR, "...",
                 EXE_END);
}

static int p4_log(const char *path) {
  return execute("p4",
                 EXE_STR, "changes",
		 EXE_STR, "-lt",
		 EXE_IFSTR(path, path),
                 EXE_END);
}

const struct vcs vcs_p4 = {
  "Perforce",
  p4_diff,
  p4_add,
  p4_remove,
  p4_commit,
  p4_revert,
  p4_status,
  p4_update,
  p4_log,
  p4_edit,
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
