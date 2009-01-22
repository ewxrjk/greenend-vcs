#ifndef VCS_H
#define VCS_H

#include <config.h>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <getopt.h>

int vcs_add(int argc, char **argv);
int vcs_remove(int argc, char **argv);
int vcs_commit(int argc, char **argv);
int vcs_diff(int argc, char **argv);
int vcs_revert(int argc, char **argv);

#endif /* VCS_H */

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
