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
#include <errno.h>

#if HAVE_LIBCURL
# include <curl/curl.h>
#endif

using namespace std;

struct vcs {
  const char *name;
  int (*diff)(int nfiles, char **files);
  int (*add)(int binary, int nfiles, char **files);
  int (*remove)(int force, int nfiles, char **files);
  int (*commit)(const char *msg, int nfiles, char **files);
  int (*revert)(int nfiles, char **files);
  int (*status)();
  int (*update)();
  int (*log)(const char *file);
  int (*edit)(int nfiles, char **files); // optional
  int (*annotate)(const char *file);
  int (*clone)(const char *uri, const char *dir);
};

extern const struct vcs vcs_cvs, vcs_svn, vcs_bzr, vcs_git, vcs_p4, vcs_hg;
extern const struct vcs vcs_darcs;

extern int verbose;
extern int dryrun;
extern int ipv;

int vcs_add(int argc, char **argv);
int vcs_remove(int argc, char **argv);
int vcs_commit(int argc, char **argv);
int vcs_diff(int argc, char **argv);
int vcs_revert(int argc, char **argv);
int vcs_status(int argc, char **argv);
int vcs_update(int argc, char **argv);
int vcs_log(int argc, char **argv);
int vcs_edit(int argc, char **argv);
int vcs_annotate(int argc, char **argv);
int vcs_clone(int argc, char **argv);

const struct vcs *guess();
const struct vcs *guess_branch(string uri);

const string uri_scheme(const string &uri);
int uri_exists(const string &uri);

int isdir(const string &s);
int exists(const string &path);
string cwd();
string parentdir(const string &d);
int isroot(const string &d);
void fatal(const char *msg, ...) 
  attribute((noreturn))  
  attribute((format (printf, 1, 2)));
int erase(const char *s);

void *xmalloc(size_t n);
char *xstrdup(const char *s);

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
