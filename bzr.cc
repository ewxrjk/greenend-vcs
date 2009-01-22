#include "vcs.h"

static int bzr_diff(int nfiles, char **files) {
  return generic_diff("bzr", nfiles, files);
}

const struct vcs vcs_bzr = {
  "Bazaar",
  bzr_diff,
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
