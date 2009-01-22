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
    fatal("forked failed: %s", strerror(errno));
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

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
