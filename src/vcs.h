/*
 * This file is part of VCS
 * Copyright (C) 2009, 2010 Richard Kettlewell
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
#include <map>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdarg>
#include <getopt.h>
#include <errno.h>

#if HAVE_CURL_CURL_H
# include <curl/curl.h>
#endif

using namespace std;

class vcs {
public:
  const char *const name;
  vcs(const char *name_);
  virtual ~vcs();

  virtual bool detect(void) const;

  virtual int diff(int nfiles, char **files) const = 0;
  virtual int add(int binary, int nfiles, char **files) const = 0;
  virtual int remove(int force, int nfiles, char **files) const = 0;
  virtual int commit(const char *msg, int nfiles, char **files) const = 0;
  virtual int revert(int nfiles, char **files) const = 0;
  virtual int status() const = 0;
  virtual int update() const = 0;
  virtual int log(const char *file) const = 0;
  virtual int edit(int nfiles, char **files) const; // optional
  virtual int annotate(const char *file) const = 0;
  virtual int clone(const char *uri, const char *dir) const; // optional
  virtual int rename(int nsources, char **sources, const char *destination) const;

  virtual void rename_one(const string &source, const string &destination) const;

  static const vcs *guess();
  static const vcs *guess_branch(string uri);

protected:
  void register_subdir(const string &subdir);
  void register_scheme(const string &scheme);
  void register_substring(const string &substring);

private:
  typedef list<vcs *> selves_t;
  static selves_t *selves;

  typedef map<string, vcs *> schemes_t;
  static schemes_t *schemes;

  typedef list< pair<string, vcs *> > substrings_t;
  static substrings_t *substrings;

  static substrings_t *subdirs;
};

extern int verbose;
extern int dryrun;
extern int ipv;
extern int debug;

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
int vcs_rename(int argc, char **argv);

const string uri_scheme(const string &uri);
int uri_exists(const string &uri);

int isdir(const string &s,
          int links_count = 1);
int isreg(const string &s,
          int links_count = 1);
int exists(const string &path);
string cwd();
string parentdir(const string &d);
string basename(const string &d);
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
#define EXE_NO_STDOUT 4
#define EXE_NO_STDERR 5
#define EXE_STRS_DOTSTUFF 6
int execute(const char *prog, ...);

int capture(vector<string> &lines,
            const char *prog,
            ...);
int vcapture(vector<string> &lines,
             const vector<string> &command);
int inject(const vector<string> &input,
           const char *prog,
           ...);
void redirect(const char *pager);
int readline(const string &path, FILE *fp, string &l);
void init_global_ignores();
void read_ignores(list<string> &ignores, const string &path);
int is_ignored(const list<string> &ignores,
               const string &file);
void listfiles(string path,
               list<string> &files,
               set<string> &ignored);
int version_compare(const string &a, const string &b);
void remove_directories(int &nfiles, char **files);
int execute(const vector<string> &command,
            const vector<string> *input = NULL,
            vector<string> *output = NULL,
            vector<string> *errors = NULL);
vector<string> &makevs(vector<string> &command,
                       const char *prog,
                       ...);
void report_lines(const vector<string> &l, 
                  const char *what = NULL, 
                  const char *prefix = NULL, 
                  FILE *fp = stderr);
string get_relative_path(const string &s);
string dirname(const string &s);
string basename(const string &s);
int editor(vector<string> &file);
int generic_rename(int nsources, char **sources, const char *destination,
		   void (*rename_one)(const string &source,
				      const string &destination));

extern list<string> global_ignores;

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
