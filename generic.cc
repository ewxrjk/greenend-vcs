#include "vcs.h"

int generic_diff(const char *tool, int nfiles, char **files) {
  vector<const char *> cmd;
  cmd.push_back(tool);
  cmd.push_back("diff");
  while(nfiles-- > 0)
    cmd.push_back(*files++);
  return execute(cmd);
}

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
