#include "vcs.h"

// Guess what VCS is in use; returns a pointer to its operation table or NULL
// if none can be determined.
const struct vcs *guess() {
  // CVS is easy as it has a CVS directory at every level.
  if(isdir("CVS"))
    return &vcs_cvs;
  // Subversion too
  if(isdir(".svn"))
    return &vcs_svn;
  // Bazaar and Git only have their dot directories at the top level of the
  // branch.
  string d = cwd();
  for(;;) {
    if(isdir(d + PATHSEPSTR + ".bzr"))
      return &vcs_bzr;
    if(isdir(d + PATHSEPSTR + ".git"))
      return &vcs_git;
    if(isroot(d))
      break;
    d = parentdir(d);
  }
  return 0;
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
