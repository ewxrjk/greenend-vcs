#include "vcs.h"

static int cvs_diff(int nfiles, char **files) {
  return execute("cvs",
                 EXE_STR, "diff",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int cvs_add(int binary, int nfiles, char **files) {
  return execute("cvs",
                 EXE_STR, "add",
                 binary ? EXE_STR : EXE_SKIPSTR, "-kb",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

const struct vcs vcs_cvs = {
  "CVS",
  cvs_diff,
  cvs_add,
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
