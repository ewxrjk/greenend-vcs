#ifndef VCS_H
#define VCS_H

#include <config.h>
#include <string>
#include <list>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdarg>
#include <getopt.h>

using namespace std;

struct vcs {
  const char *name;
};

extern const struct vcs vcs_cvs, vcs_svn, vcs_bzr, vcs_git;

int vcs_add(int argc, char **argv);
int vcs_remove(int argc, char **argv);
int vcs_commit(int argc, char **argv);
int vcs_diff(int argc, char **argv);
int vcs_revert(int argc, char **argv);

const struct vcs *guess();

int isdir(const string &s);
string cwd();
string parentdir(const string &d);
int isroot(const string &d);
void fatal(const char *msg, ...) attribute((noreturn));

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
