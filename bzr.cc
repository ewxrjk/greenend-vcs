#include "vcs.h"

static int bzr_diff(int nfiles, char **files) {
  return execute("bzr",
                 EXE_STR, "diff",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int bzr_add(int binary, int nfiles, char **files) {
  return execute("bzr",
                 EXE_STR, "add",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

const struct vcs vcs_bzr = {
  "Bazaar",
  bzr_diff,
  bzr_add,
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
