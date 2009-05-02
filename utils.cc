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
#include "vcs.h"
#include <sys/stat.h>
#include <sys/wait.h>
#include <cerrno>
#include <fcntl.h>

// Global debug flag
int debug;

// Global verbose operation flag
int verbose;

// Global dry-run flag
int dryrun;

// Preferred IP version
int ipv;

// Return nonzero if PATH is a directory.
int isdir(const string &path,
          int links_count) {
  struct stat s[1];

  if((links_count ? stat : lstat)(path.c_str(), s) < 0)
    return 0;
  if(S_ISDIR(s->st_mode))
    return 1;
  return 0;
}

// Return nonzero if PATH is a file.
int isreg(const string &path,
          int links_count) {
  struct stat s[1];

  if((links_count ? stat : lstat)(path.c_str(), s) < 0)
    return 0;
  if(S_ISREG(s->st_mode))
    return 1;
  return 0;
}

// Return nonzero if a file exists (even as a dangling link)
// If we can't tell, we return 0.
int exists(const string &path) {
  struct stat s[1];

  if(lstat(path.c_str(), s) < 0)
    return 0;
  return 1;
}

// Return the current working directory
string cwd() {
  char b[8192];
  if(!getcwd(b, sizeof b))
    fatal("current working directory path is too long");
  return b;
}

// Return the (lexical) parent of directory D
string parentdir(const string &d) {
  size_t n = d.rfind(PATHSEP);
  if(n == string::npos)
    fatal("invalid directory name: %s", d.c_str());
  if(n == 0)			// it's the root or a subdirectory thereof
    return "/";
  return d.substr(0, n);
}

// Return true if D is (a) root directory
int isroot(const string &d) {
  return d == "/";
}

// Report a fatal error
void fatal(const char *msg, ...) {
  va_list ap;

  fprintf(stderr, "ERROR: ");
  va_start(ap, msg);
  vfprintf(stderr, msg, ap);
  va_end(ap);
  fputc('\n', stderr);
  exit(1);
}

void *xmalloc(size_t n) {
  void *ptr;

  if(!(ptr = malloc(n)) && n)
    fatal("out of memory");
  return ptr;
}

char *xstrdup(const char *s) {
  return strcpy((char *)xmalloc(strlen(s) + 1), s);
}

// Remove a file and report any errors
int erase(const char *s) {
  if(remove(s) < 0) {
    fprintf(stderr, "ERROR: removing %s: %s\n", s, strerror(errno));
    return -1;
  } else
    return 0;
}

static pid_t pager_pid = -1;

// Redirect output if not a terminal
void redirect(const char *pager) {
  int p[2];

  if(!pager)
    return;
  if(!isatty(1))
    return;
  if(pipe(p) < 0)
    fatal("pipe failed: %s", strerror(errno));
  switch((pager_pid = fork())) {
  case 0:
    if(dup2(p[0], 0) < 0) {
      perror("dup2");
      _exit(-1);
    }
    if(close(p[0]) < 0 || close(p[1]) < 0) {
      perror("close");
      _exit(-1);
    }
    execlp("/bin/sh", "sh", "-c", pager, (char *)NULL);
    perror("executing sh");
    _exit(-1);
  case -1:
    fatal("fork failed: %s", strerror(errno));
  }
  if(dup2(p[1], 1) < 0)
    fatal("dup2");
  if(close(p[0]) < 0 || close(p[1]) < 0)
    fatal("close");
}

void await_redirect() {
  int w;

  if(pager_pid != -1)
    while(waitpid(pager_pid, &w, 0) < 0 && errno == EINTR)
      ;
}

// Read one line from a file.  Returns 1 on success, 0 on eof
int readline(const string &path, FILE *fp, string &l) {
  int c;

  l.clear();
  while((c = getc(fp)) != EOF && c != '\n')
    l += c;
  if(ferror(fp))
    fatal("error reading %s: %s", path.c_str(), strerror(errno));
  if(feof(fp) && !l.size())
    return 0;
  else
    return 1;
}

void remove_directories(int &nfiles, char **files) {
  vector<char *> nondirs;
  int n, m = 0;

  for(n = 0; n < nfiles; ++n)
    if(!isdir(files[n]))
      files[m++] = files[n];
  nfiles = m;
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
