#include "vcs.h"

static int svn_diff(int nfiles, char **files) {
  return execute("svn",
                 EXE_STR, "diff",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int svn_add(int binary, int nfiles, char **files) {
  return execute("svn",
                 EXE_STR, "add",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int svn_remove(int force, int nfiles, char **files) {
  return execute("svn",
                 EXE_STR, "delete",
                 EXE_IFSTR(force, "--force"),
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

const struct vcs vcs_svn = {
  "Subversion",
  svn_diff,
  svn_add,
  svn_remove,
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
