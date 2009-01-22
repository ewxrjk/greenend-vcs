/*
 * This file is part of VCS
 * Copyright (C) 2009 Richard Kettlewell
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef VCS_H
#define VCS_H

#include <config.h>
#include <string>
#include <list>
#include <vector>
#include <set>
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
  int (*remove)(int force, int nfiles, char **files);
  int (*commit)(const char *msg, int nfiles, char **files);
  int (*revert)(int nfiles, char **files);
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
#define EXE_IFSTR(COND, STR) (COND) ? EXE_STR : EXE_SKIPSTR, (STR)
int execute(const char *prog, ...);

int capture(vector<string> &lines,
            const char *prog,
            ...);

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
