/*
 * This file is part of VCS
 * Copyright (C) 2009, 2012 Richard Kettlewell
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
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

static void cleanup() {
  unlink(",pager-ran");
  unlink(",child-ran");
}

static void test_pager(const char *pager,
                       bool redirect_stdout = false) {
  pid_t pid;
  int w;

  cleanup();
  switch(pid = fork()) {
  case 0:
    if(redirect_stdout) {
      int fd = open("/dev/null", O_WRONLY);
      assert(fd >= 0);
      assert(dup2(fd, 1) >= 0);
      close(fd);
    }
    redirect(pager);
    open(",child-ran", O_WRONLY|O_CREAT, 0666);
    exit(0);
  case -1:
    fatal("fork: %s", strerror(errno));
  }
  while(waitpid(pid, &w, 0) < 0 && errno == EINTR)
    ;
  assert(w == 0);
}

int main(void) {
  if(!isatty(1)) {
    fprintf(stderr, "Can't test pager without a tty\n");
    return 77;
  }
  test_pager("touch ,pager-ran;cat");
  assert(exists(",pager-ran"));
  assert(exists(",child-ran"));
  test_pager("touch ,pager-ran;cat", true);
  assert(!exists(",pager-ran"));
  assert(exists(",child-ran"));
  test_pager(NULL);
  assert(!exists(",pager-ran"));
  assert(exists(",child-ran"));
  // TODO calling _exit() in redirect() means coverage data isn't updated.
  // What can we do about that...
  cleanup();
  return 0;
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
