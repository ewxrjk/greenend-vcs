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

/* TODO: git is not supported yet (and guess.cc reflects this, remember to
 * condition git detection back in when you implement git support). */

static int git_diff(int /*nfiles*/, char **/*files*/) {
  return -1;
}

static int git_add(int /*binary*/, int /*nfiles*/, char **/*files*/) {
  return -1;
}

static int git_remove(int /*force*/, int /*nfiles*/, char **/*files*/) {
  return -1;
}

static int git_commit(const char */*msg*/, int /*nfiles*/, char **/*files*/) {
  return -1;
}

static int git_revert(int /*nfiles*/, char **/*files*/) {
  return -1;
}

const struct vcs vcs_git = {
  "Git",
  git_diff,
  git_add,
  git_remove,
  git_commit,
  git_revert,
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
