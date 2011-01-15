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
#include "vcs.h"

class bzr: public vcs {
public:

  bzr(): vcs("Bazaar") {
    register_subdir(".bzr");
    register_substring("bzr");
  }

  int diff(int nfiles, char **files) const {
    return execute("bzr",
                   EXE_STR, "diff",
                   EXE_STRS, nfiles, files,
                   EXE_END);
  }

  int add(int /*binary*/, int nfiles, char **files) const {
    return execute("bzr",
                   EXE_STR, "add",
                   EXE_STRS, nfiles, files,
                   EXE_END);
  }

  int remove(int force, int nfiles, char **files) const {
    return execute("bzr",
                   EXE_STR, "remove",
                   EXE_IFSTR(force, "--force"),
                   EXE_STRS, nfiles, files,
                   EXE_END);
  }

  int commit(const char *msg, int nfiles, char **files) const {
    return execute("bzr",
                   EXE_STR, "commit",
                   EXE_IFSTR(msg, "-m"),
                   EXE_IFSTR(msg, msg),
                   EXE_STRS, nfiles, files,
                   EXE_END);
  }

  int revert(int nfiles, char **files) const {
    return execute("bzr",
                   EXE_STR, "revert",
                   EXE_STRS, nfiles, files,
                   EXE_END);
  }
  
  int status() const {
    return execute("bzr",
                   EXE_STR, "status",
                   EXE_END);
  }

  int update() const {
    vector<string> info;
    int rc;
    if((rc = capture(info, "bzr", "info", (char *)NULL)))
      fatal("'bzr info' exited with status %d", rc);
    if(info.size() > 0
       && info[0].compare(0, 8, "Checkout") == 0)
      return execute("bzr",
                     EXE_STR, "up",
                     EXE_END);
    else
      return execute("bzr",
                     EXE_STR, "pull",
                     EXE_END);
  }

  int log(const char *path) const {
    return execute("bzr",
                   EXE_STR, "log",
                   EXE_IFSTR(path, path),
                   EXE_END);
  }
  
  int annotate(const char *path) const {
    return execute("bzr",
                   EXE_STR, "annotate",
                   EXE_STR, path,
                   EXE_END);
  }
  
  int clone(const char *uri, const char *dir) const {
    return execute("bzr",
                   EXE_STR, "clone",
                   EXE_STR, uri,
                   EXE_IFSTR(dir, dir),
                   EXE_END);
  }

  int rename(int nsources, char **sources, const char *destination) const {
    return execute("bzr",
                   EXE_STR, "mv",
                   EXE_STRS, nsources, sources,
                   EXE_STR, destination,
                   EXE_END);
  }

  int show(const char *change) const {
    return execute("bzr",
                   EXE_STR, "diff",
                   EXE_STR, "-c",
                   EXE_STR, change,
                   EXE_END);
  }
};

static const bzr vcs_bzr;

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
