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
                 EXE_IFSTR(binary, "-kb"),
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int cvs_remove(int force, int nfiles, char **files) {
  return execute("cvs",
                 EXE_STR, "remove",
                 EXE_IFSTR(force, "-f"),
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int cvs_commit(const char *msg, int nfiles, char **files) {
  return execute("cvs",
                 EXE_STR, "commit",
                 EXE_IFSTR(msg, "-m"),
                 EXE_IFSTR(msg, msg),
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

const struct vcs vcs_cvs = {
  "CVS",
  cvs_diff,
  cvs_add,
  cvs_remove,
  cvs_commit,
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
