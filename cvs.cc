#include "vcs.h"

static int cvs_diff(int nfiles, char **files) {
  vector<const char *> cmd;
  cmd.push_back("cvs");
  cmd.push_back("diff");
  while(nfiles-- > 0)
    cmd.push_back(*files++);
  return execute(cmd);
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
