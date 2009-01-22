#include "vcs.h"

static int svn_diff(int nfiles, char **files) {
  return generic_diff("svn", nfiles, files);
}

const struct vcs vcs_svn = {
  "Subversion",
  svn_diff
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
