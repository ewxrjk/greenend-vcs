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

class darcs: public vcs {
public:
  darcs(): vcs("Darcs") {
    register_subdir("_darcs");
  }

  int diff(const vector<string> &files) const {
    return execute("darcs",
                   EXE_STR, "diff",
                   EXE_STR, "-u",
                   EXE_STR, "--",
                   EXE_VECTOR, &files,
                   EXE_END);
  }

  int add(int /*binary*/, const vector<string> &files) const {
    return execute("darcs",
                   EXE_STR, "add",
                   EXE_STR, "--",
                   EXE_VECTOR, &files,
                   EXE_END);
  }

  int remove(int force, const vector<string> &files) const {
    if(force)
      return execute("rm",
                     EXE_STR, "-f",
                     EXE_STR, "--",
                     EXE_VECTOR, &files,
                     EXE_END);
    else
      return execute("darcs",
                     EXE_STR, "remove",
                     EXE_STR, "--",
                     EXE_VECTOR, &files,
                     EXE_END);
  }

  int commit(const string *msg, const vector<string> &files) const {
    return execute("darcs",
                   EXE_STR, "record",
                   EXE_STR, "--all",
                   EXE_IFSTR(msg, "-m"),
                   EXE_STRING|EXE_OPT, msg,
                   EXE_STR, "--",
                   EXE_VECTOR, &files,
                   EXE_END);
  }

  int revert(const vector<string> &files) const {
    return execute("darcs",
                   EXE_STR, "revert",
                   EXE_STR, "--all",
                   EXE_STR, "--",
                   EXE_VECTOR, &files,
                   EXE_END);
  }

  int status() const {
    int rc = execute("darcs",
                     EXE_STR, "whatsnew",
                     EXE_STR, "--summary",
                     EXE_END);
    // darcs whatsnew exits non-0 if nothing's changed!  Insane.
    return rc == 1 ? 0 : rc;
  }

  int update() const {
    return execute("darcs",
                   EXE_STR, "pull",
                   EXE_STR, "--all",
                   EXE_END);
  }

  int log(const string *path) const {
    return execute("darcs",
                   EXE_STR, "changes",
                   EXE_STR, "--",
                   EXE_STRING|EXE_OPT, path,
                   EXE_END);
  }

  int annotate(const string &path) const {
    return execute("darcs",
                   EXE_STR, "annotate",
                   EXE_STR, "--",
                   EXE_STRING, &path,
                   EXE_END);
  }

  int clone(const string &uri, const string *dir) const {
    return execute("darcs",
                   EXE_STR, "get",
                   EXE_STR, "--",
                   EXE_STRING, &uri,
                   EXE_STRING|EXE_OPT, dir,
                   EXE_END);
  }

  int rename(const vector<string> &sources, const string &destination) const {
    return execute("darcs",
                   EXE_STR, "mv",
                   EXE_STR, "--",
                   EXE_VECTOR, &sources,
                   EXE_STRING, &destination,
                   EXE_END);
  }

  int show(const string &change) const {
    return execute("darcs",
                   EXE_STR, "diff",
                   EXE_STR, "-u",
                   EXE_STR, "--match",
                   EXE_STRING, &change,
                   EXE_END);
  }
};

static const darcs vcs_darcs;

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
