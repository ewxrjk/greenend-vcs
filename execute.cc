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
#include <sys/wait.h>
#include <fcntl.h>
#include <cerrno>

// Base class for things that can be attached to the event loop
class monitor {
public:
  virtual ~monitor() {
  }

  // Called inside the fork
  virtual void insidefork() = 0;

  // Calle after the fork
  virtual void afterfork() = 0;
  
  // Called before select()
  virtual void beforeselect(fd_set *rfds, fd_set *wfds, int &max) = 0;

  // Called after select()
  virtual void afterselect(const fd_set *rfds, const fd_set *wfds) = 0; 

  // Test whether this monitor is still active
  virtual bool active() = 0;
};

// FD-based things that can be attached to the event loop
class fdmonitor: public monitor {
public:
  inline fdmonitor():
    fd(-1) {
  }

  bool active() {
    return fd != -1;
  }

  virtual ~fdmonitor() {
    if(fd != -1)
      ::close(fd);
  }
protected:
  int fd;
  
  void update_fd_set(fd_set *fds, int &max) {
    if(fd != -1) {
      FD_SET(fd, fds);
      if(fd > max)
        max = fd;
    }
  }

  void close() {
    if(fd != -1) {
      if(::close(fd) < 0)
        fatal("error calling close: %s", strerror(errno));
      fd = -1;
    }
  }
};

// Redirect a child's FD
class fdredirect: public fdmonitor {
public:
  inline fdredirect():
    parentfd(-1),
    childfd(-1) {
  }
protected:
  int parentfd;                         // opened FD child will read/write
  int childfd;                          // target FD in child

  // Called if the parent process will write (to the configured FD) and the
  // child read (from childid).
  void writer(int childid) {
    make_pipe(parentfd/*read end of pipe*/, fd/*write end of pipe*/);
    childfd = childid;
  }

  // Called if the parent process will read (from the configured FD) and the
  // child write (to childid).
  void reader(int childid) {
    make_pipe(fd/*read end of pipe*/, parentfd/*write end of pipe*/);
    childfd = childid;
  }

private:
  void insidefork() {
    if(dup2(parentfd, childfd) < 0) {
      perror("dup2");
      _exit(1);
    }
    // Close both original pipe FDs
    if(::close(parentfd) < 0 || ::close(fd) < 0) {
      perror("close");
      _exit(1);
    }
  }
  
  void afterfork() {
    if(::close(parentfd) < 0)
      fatal("error calling close: %s", strerror(errno));
  }
  
  void make_pipe(int &rfd, int &wfd) {
    int p[2];

    if(pipe(p) < 0)
      fatal("error creating pipe: %s", strerror(errno));
    rfd = p[0];
    wfd = p[1];
  }
};

// Write to a child's redirected FD
class writetofd: public fdredirect {
public:
  writetofd(int childid) {
    writer(childid);
  }

  void beforeselect(fd_set *, fd_set *wfds, int &max) {
    update_fd_set(wfds, max);
  }

  void afterselect(const fd_set *, const fd_set *wfds) {
    if(fd >= 0 && FD_ISSET(fd, wfds)) {
      size_t nbytes = 0;
      const void *ptr = available(nbytes);
      if(nbytes) {
        const int written = write(fd, ptr, nbytes);
        
        if(written < 0)
          fatal("write error: %s", strerror(errno));
        wrote(written);
      } else
        close();
    }
  }

  // Find out what to write next
  virtual const void *available(size_t &nbytes) = 0;

  // Called when NBYTES have been written
  virtual void wrote(size_t nbytes) = 0;
};

// Write a string to child's redirected FD
class writefromstring: public writetofd {
public:
  writefromstring(const string &s_, int childid = 0):
    writetofd(childid),
    s(s_),
    written(0) {
  }
private:
  // string to write
  const string &s;
  
  // bytes written so far
  size_t written;

  const void *available(size_t &nbytes) {
    nbytes = s.size() - written;
    return s.data() + written;
  }

  void wrote(size_t nbytes) {
    written += nbytes;
  }
};

// Read from an FD
class readfromfd: public fdredirect {
public:
  readfromfd(int childid) {
    reader(childid);
  }

  void beforeselect(fd_set *rfds, fd_set *, int &max) {
    update_fd_set(rfds, max);
  }
  
  void afterselect(const fd_set *rfds, const fd_set *) {
    if(fd >= 0 && FD_ISSET(fd, rfds)) {
      char buffer[4096];
      int n = ::read(fd, buffer, sizeof fd);
      
      if(n < 0)
        fatal("read error: %s", strerror(errno));
      if(n == 0) {
        close();
        eof();
      } else
        read(buffer, n);
    }
  }
  
  // Called when bytes have been read
  virtual void read(void *ptr, size_t nbytes) = 0;
  
  // Called at EOF
  virtual void eof() {
    // Default does nothing
  }
};

// Read from a child's redirected FD into a sintrg
class readtostring: public readfromfd {
public:
  readtostring(string &s_, int childid = 1):
    readfromfd(childid),
    s(s_) {
  }
private:
  string &s;

  void read(void *ptr, size_t nbytes) {
    s.append((char *)ptr, nbytes);
  }

};

// General purpose command execution
static int exec(const char *prog,
                const char **args,
                const list<monitor *> &monitors,
                unsigned killfds = 0) {
  pid_t pid;
  list<monitor *>::const_iterator it;

  // Start subprocess
  if((pid = fork()) < 0)
    fatal("error calling fork: %s", strerror(errno));
  if(pid == 0) {
    for(it = monitors.begin();
        it != monitors.end();
        ++it) 
      (*it)->insidefork();
    if(killfds) {
      const int nullfd = open("/dev/null", O_RDWR);
      if(nullfd < 0) {
        perror("/dev/null");
        _exit(-1);
      }
      for(int n = 0; n < 3; ++n)
        if((killfds & (1 << n)) && dup2(nullfd, n) < 0) {
          perror("dup2");
          _exit(-1);
        }
      if(close(nullfd) < 0) {
        perror("close");
        _exit(-1);
      }
    }
    execvp(prog, (char **)args);
    fprintf(stderr, "executing %s: %s\n", prog, strerror(errno));
    _exit(1);
  }
  for(it = monitors.begin();
      it != monitors.end();
      ++it) 
    (*it)->afterfork();
  /* Feed in input and gather output */
  for(;;) {
    // Give each active monitor a chance to fiddle with select() args
    fd_set rfds[1], wfds[1];
    FD_ZERO(rfds);
    FD_ZERO(wfds);
    int max = -1, nactive = 0;
    for(it = monitors.begin();
        it != monitors.end();
        ++it) {
      if((*it)->active()) {
        ++nactive;
        (*it)->beforeselect(rfds, wfds, max);
      }
    }
    if(!nactive)
      // Stop waiting for IO if no monitors left, the wait will never finish
      break;
    const int rc = select(max + 1, rfds, wfds, NULL, NULL);
    if(rc < 0)
      fatal("error calling select: %s", strerror(errno));
    for(it = monitors.begin();
        it != monitors.end();
        ++it) {
      if((*it)->active())
        (*it)->afterselect(rfds, wfds);
    }
  }
  // Wait for subprocess to terminate
  int w, rc;
  while((rc = waitpid(pid, &w, 0) < 0)
        && errno == EINTR)
    ;
  if(rc < 0)
    fatal("error calling waitpid: %s", strerror(errno));
  // Signals are always fatal
  if(WIFSIGNALED(w))
    fatal("%s received fatal signal %d (%s)", prog, 
	  WTERMSIG(w), strsignal(WTERMSIG(w)));
  if(WIFEXITED(w))
    return WEXITSTATUS(w);
  fatal("%s exited with unknown wait status %#x", prog, w);
}

// As above but with a vector<string> arg list
static int exec(const vector<string> &args,
                const list<monitor *> &monitors) {
  vector<const char *> cargs;
  size_t n;

  cargs.reserve(args.size());
  for(n = 0; n < args.size(); ++n)
    cargs.push_back(args[n].c_str());
  cargs.push_back(NULL);
  return exec(cargs[0], &cargs[0], monitors);
}

// As above but for stdarg arg lists
static int exec(const char *prog,
                va_list ap,
                const list<monitor *> &monitors) {
  vector<const char *> cmd;
  const char *s;

  cmd.push_back(prog);
  do {
    cmd.push_back((s = va_arg(ap, const char *)));
  } while(s);
  return exec(prog, &cmd[0], monitors);
}

// Assemble a command from an argument list
static void assemble(vector<const char *> &cmd,
                     const char *prog,
                     va_list ap,
                     unsigned &killfds) {
  cmd.push_back(prog);
  int op;
  while((op = va_arg(ap, int)) != EXE_END) {
    switch(op) {
    case EXE_STR:
      cmd.push_back(va_arg(ap, char *));
      break;
    case EXE_SKIPSTR:
      va_arg(ap, char *);
      break;
    case EXE_STRS:
    case EXE_STRS_DOTSTUFF: {
      int count = va_arg(ap, int);
      char **strs = va_arg(ap, char **);
      int first = 1;
      while(count-- > 0) {
        const char *s = *strs++;
        string t;
        if(first) {
          if(s[0] == '-' && op == EXE_STRS_DOTSTUFF) {
            t = string("./") + s;
            s = t.c_str();
          }
          first = 0;
        }
        cmd.push_back(s);
      }
      break;
    }
    case EXE_NO_STDOUT:
      killfds |= 1;
      break;
    case EXE_NO_STDERR:
      killfds |= 2;
      break;
    default:
      assert(!"unknown execute() op");
    }
  }
  cmd.push_back(NULL);
}

// Split a string on newline
static void split(vector<string> &lines, const string &s) {
  string::size_type pos = 0, n;
  const string::size_type limit = s.size();
  
  lines.clear();
  while(pos < limit && (n = s.find('\n', pos)) != string::npos) {
    lines.push_back(s.substr(pos, n - pos));
    pos = n + 1;
  }
  if(pos < limit)
    lines.push_back(s.substr(pos, limit - pos));
}

// Join a string with newlines
static void join(string &s, const vector<string> &lines) {
  s.clear();
  for(size_t n = 0; n < lines.size(); ++n) {
    s.append(lines[n]);
    s.append("\n");
  }
}

// Execute a command assembled using EXE_... macros and return its
// exit code.
int execute(const char *prog, ...) {
  va_list ap;
  vector<const char *> cmd;
  unsigned killfds = 0;

  va_start(ap, prog);
  assemble(cmd, prog, ap, killfds);
  va_end(ap);
  return exec(prog, &cmd[0], list<monitor *>(), killfds);
}

// Execute a command (specified like execl()) and capture its output.
// Returns exit code.
int capture(vector<string> &lines,
            const char *prog,
            ...) {
  va_list ap;

  va_start(ap, prog);
  string buffer;
  readtostring r(buffer);
  const int rc = exec(prog, ap, list<monitor *>(1, &r));
  va_end(ap);
  split(lines, buffer);
  return rc;
}

// Execute a command (specified in a string) and capture it output.
// Returns exit code.
int vcapture(vector<string> &lines,
             const vector<string> &command) {
  string buffer;
  readtostring r(buffer);
  const int rc = exec(command, list<monitor *>(1, &r));
  split(lines, buffer);
  return rc;
}

// Execute a command and feed it input.  Returns the exit code.
int inject(const vector<string> &input,
           const char *prog,
           ...) {
  va_list ap;

  va_start(ap, prog);
  string buffer;
  join(buffer, input);
  writefromstring w(buffer);
  const int rc = exec(prog, ap, list<monitor *>(1, &w));
  va_end(ap);
  return rc;
}

/*
Local Variables:
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/