/*
 * This file is part of VCS
 * Copyright (C) 2009, 2011, 2012 Richard Kettlewell
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
#include <sys/stat.h>
#include <unistd.h>

int main(void) {
  FILE *fp;

  remove(",reg.tmp");
  remove(",dir.tmp");
  remove(",symreg.tmp");
  remove(",symdir.tmp");

  fp = fopen(",reg.tmp", "w");
  assert(fp);
  fclose(fp);

  assert(mkdir(",dir.tmp", 0666) >= 0);

  assert(symlink(",reg.tmp", ",symreg.tmp") == 0);
  assert(symlink(",dir.tmp", ",symdir.tmp") == 0);
  assert(symlink(",notexist.tmp", ",dangling.tmp") == 0);

  assert(isreg(",reg.tmp"));
  assert(isreg(",reg.tmp", false));
  assert(!isdir(",reg.tmp"));
  assert(!isdir(",reg.tmp", false));

  assert(isreg(",symreg.tmp"));
  assert(!isreg(",symreg.tmp", false));
  assert(!isdir(",symreg.tmp"));
  assert(!isdir(",symreg.tmp", false));

  assert(!isreg(",dir.tmp"));
  assert(!isreg(",dir.tmp", false));
  assert(isdir(",dir.tmp"));
  assert(isdir(",dir.tmp", false));
  
  assert(!isreg(",symdir.tmp"));
  assert(!isreg(",symdir.tmp", false));
  assert(isdir(",symdir.tmp"));
  assert(!isdir(",symdir.tmp", false));

  assert(!isreg(",notexist"));
  assert(!isreg(",notexist.tmp", false));
  assert(!isdir(",notexist.tmp"));
  assert(!isdir(",notexist.tmp", false));

  assert(!isreg(",dangling"));
  assert(!isreg(",dangling.tmp", false));
  assert(!isdir(",dangling.tmp"));
  assert(!isdir(",dangling.tmp", false));
  
  erase(",reg.tmp");
  erase(",symreg.tmp");
  erase(",dir.tmp");
  erase(",symdir.tmp");
  erase(",dangling.tmp");

  assert(parentdir("/") == "/");
  assert(parentdir("/wibble") == "/");
  assert(parentdir("/wibble/spong") == "/wibble");

  assert(basename_("foo/bar") == "bar");
  assert(basename_("foo") == "foo");
  assert(basename_("/foo/bar") == "bar");
  assert(basename_("/foo") == "foo");
  assert(basename_("./foo/bar") == "bar");
  assert(basename_("./foo") == "foo");
  assert(basename_("./") == ".");
  assert(basename_("/") == "/");

  assert(dirname_("foo/bar") == "foo");
  assert(dirname_("foo") == ".");
  assert(dirname_("/foo/bar") == "/foo");
  assert(dirname_("/foo") == "/");
  assert(dirname_("./foo/bar") == "./foo");
  assert(dirname_("./foo") == ".");
  assert(dirname_("./") == ".");
  assert(dirname_("/") == "/");

  assert(isroot("/"));
  assert(!isroot("/wibble"));

  assert(get_relative_path("wibble") == "wibble");
  assert(get_relative_path(cwd() + "/wibble") == "wibble");
  assert(get_relative_path(cwd() + "/wibble") == "wibble"); // cached
  assert(get_relative_path("/" + cwd() + "/wibble") == "wibble");
  assert(get_relative_path("/" + cwd() + "/wibble") == "wibble"); // cached
  assert(get_relative_path(cwd()) == cwd());
  assert(get_relative_path("/" + cwd()) == "/" + cwd());
  assert(get_relative_path(".") == ".");

  int sparefd = dup(0);
  assert(sparefd >= 0);
  close(sparefd);
  string name;
  {
    TempFile t;
    name = t.path();
    // Check some properties of the created file
    struct stat sb;
    assert(lstat(name.c_str(), &sb) >= 0);
    assert(S_ISREG(sb.st_mode));
    assert((sb.st_mode & 0777) == 0600);
    assert(sb.st_size == 0);
    // Had better not leak an FD
    int newsparefd = dup(0);
    assert(newsparefd == sparefd);
  }
  assert(!exists(name));

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
