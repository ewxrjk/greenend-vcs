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
