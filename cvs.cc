#include "vcs.h"

static int cvs_diff(int nfiles, char **files) {
  return generic_diff("cvs", nfiles, files);
}

const struct vcs vcs_cvs = {
  "CVS",
  cvs_diff
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
