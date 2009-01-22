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

// Return nonzero if PATH is a directory.  Links to directories count.
int isdir(const string &path) {
  struct stat s[1];

  if(stat(path.c_str(), s) < 0)
    return 0;
  if(S_ISDIR(s->st_mode))
    return 1;
  return 0;
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
  if(n == 0)			// it's the root
    return d;
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

// Execute a command assembled using EXE_... macros and return its
// exit code.
int execute(const char *prog, ...) {
  // Assemble the command
  va_list ap;
  vector<const char *> cmd;
  cmd.push_back(prog);
  va_start(ap, prog);
  int op;
  while((op = va_arg(ap, int)) != EXE_END) {
    switch(op) {
    case EXE_STR:
      cmd.push_back(va_arg(ap, char *));
      break;
    case EXE_SKIPSTR:
      va_arg(ap, char *);
      break;
    case EXE_STRS: {
      int count = va_arg(ap, int);
      char **strs = va_arg(ap, char **);
      while(count-- > 0)
        cmd.push_back(*strs++);
      break;
    }
    default:
      assert(!"unknown execute() op");
    }
  }
  va_end(ap);
  if(verbose || dryrun) {
    FILE *fp = dryrun ? stdout : stderr;
    for(size_t n = 0; n < cmd.size(); ++n) {
      if(n)
	fputc(' ', fp);
      fputs(cmd[n], fp);
    }
    fputc('\n', fp);
    if(dryrun)
      return 0;
  }
  // Execute it
  pid_t pid;
  int w, rc;
  cmd.push_back(NULL);
  switch((pid = fork())) {
  case 0:
    execvp(cmd[0], (char **)&cmd[0]);
    fprintf(stderr, "executing %s: %s\n", cmd[0], strerror(errno));
    _exit(-1);
  case -1:
    fatal("fork failed: %s", strerror(errno));
  }
  while((rc = waitpid(pid, &w, 0)) < 0 && errno == EINTR)
    ;
  if(rc < 0)
    fatal("waitpid failed: %s", strerror(errno));
  if(WIFSIGNALED(w))
    fatal("%s received fatal signal %d (%s)", cmd[0], 
	  WTERMSIG(w), strsignal(WTERMSIG(w)));
  if(WIFEXITED(w))
    return WEXITSTATUS(w);
  fatal("%s exited with unknown wait status %#x", cmd[0], w);
}

// Execute a command (specified like execl()) and capture its output.
// Returns exit code.
int capture(vector<string> &lines,
            const char *prog,
            ...) {
  const char *arg;
  va_list ap;

  // Construct the command
  vector<const char *> cmd;
  cmd.push_back(prog);
  va_start(ap, prog);
  while((arg = va_arg(ap, const char *)))
    cmd.push_back(arg);
  // Execute it
  pid_t pid;
  int w, rc, p[2];
  cmd.push_back(NULL);
  if(pipe(p) < 0)
    fatal("pipe failed: %s", strerror(errno));
  switch((pid = fork())) {
  case 0:
    if(dup2(p[1], 1) < 0) {
      perror("dup2");
      _exit(-1);
    }
    if(close(p[0]) < 0 || close(p[1]) < 0) {
      perror("close");
      _exit(-1);
    }
    execvp(cmd[0], (char **)&cmd[0]);
    fprintf(stderr, "executing %s: %s\n", cmd[0], strerror(errno));
    _exit(-1);
  case -1:
    fatal("fork failed: %s", strerror(errno));
  }
  FILE *fp;
  if(close(p[1]) < 0)
    fatal("close: %s", strerror(errno));
  if(!(fp = fdopen(p[0], "r")))
    fatal("fdopen: %s", strerror(errno));
  int c;
  do {
    string line;
    while((c = getc(fp)) != EOF && c != '\n')
      line += c;
    lines.push_back(line);
  } while(c != EOF);
  if(ferror(fp))
    fatal("error reading pipe from %s: %s", cmd[0], strerror(errno));
  fclose(fp);
  while((rc = waitpid(pid, &w, 0)) < 0 && errno == EINTR)
    ;
  if(rc < 0)
    fatal("waitpid failed: %s", strerror(errno));
  if(WIFSIGNALED(w))
    fatal("%s received fatal signal %d (%s)", cmd[0], 
	  WTERMSIG(w), strsignal(WTERMSIG(w)));
  if(WIFEXITED(w))
    return WEXITSTATUS(w);
  fatal("%s exited with unknown wait status %#x", cmd[0], w);
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
