#ifndef VCS_H
#define VCS_H

#include <config.h>
#include <string>
#include <list>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdarg>
#include <getopt.h>

using namespace std;

struct vcs {
  const char *name;
  int (*diff)(int nfiles, char **files);
  int (*add)(int binary, int nfiles, char **files);
};

extern const struct vcs vcs_cvs, vcs_svn, vcs_bzr, vcs_git;

extern int verbose;
extern int dryrun;

int vcs_add(const struct vcs *v, int argc, char **argv);
int vcs_remove(const struct vcs *v, int argc, char **argv);
int vcs_commit(const struct vcs *v, int argc, char **argv);
int vcs_diff(const struct vcs *v, int argc, char **argv);
int vcs_revert(const struct vcs *v, int argc, char **argv);

const struct vcs *guess();

int isdir(const string &s);
string cwd();
string parentdir(const string &d);
int isroot(const string &d);
void fatal(const char *msg, ...) attribute((noreturn));

#define EXE_END 0
#define EXE_STR 1
#define EXE_SKIPSTR 2
#define EXE_STRS 3
int execute(const char *prog, ...);

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
