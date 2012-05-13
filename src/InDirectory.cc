/*
 * This file is part of VCS
 * Copyright (C) 2011, 2012 Richard Kettlewell
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
#include <unistd.h>

InDirectory::InDirectory(const std::string &dir): fd(-1) {
  // If we're already in the right directory do nothing
  struct stat stat_cwd, stat_dot;
  if(stat(".", &stat_dot) < 0)
    fatal("stat .: %s", strerror(errno));
  if(stat(dir.c_str(), &stat_cwd) < 0)
    fatal("stat %s: %s", dir.c_str(), strerror(errno));
  if(stat_dot.st_dev == stat_cwd.st_dev
     && stat_dot.st_ino == stat_cwd.st_ino)
    return;
  if(dryrun || verbose)
    fprintf(stderr, "cd %s\n", dir.c_str());
  if((fd = open(".", O_RDONLY)) < 0)
    fatal("opening .: %s", strerror(errno));
  if(chdir(dir.c_str()) < 0)
    fatal("chdir %s: %s", dir.c_str(), strerror(errno));
}

InDirectory::~InDirectory() {
  if(fd != -1) {
    if(dryrun || verbose)
      fprintf(stderr, "cd -\n");
    int rc = fchdir(fd);
    close(fd);
    if(rc < 0)
      fatal("fchdir: %s", strerror(errno));
  }
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
