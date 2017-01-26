/*
 * This file is part of VCS
 * Copyright (C) 2011 Richard Kettlewell
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
#include "rcsbase.h"

// Note about SCCS:
//
// * CSSC copes badly with paths outside the current directory, so we
//   go into each directory's file individually.  We could batch them
//   up per-directory and reduce the number of commands executed
//   (patches invited l-)
//
// * None of this has been tested with real SCCS - only CSSC.

class sccs: public rcsbase {
public:
  sccs(): rcsbase("SCCS") {
    register_subdir("SCCS");
  }

  // Return the name of the tracking directory
  string tracking_directory() const {
    return "SCCS";
  }

  // Return true if this is a tracking file (i.e. is a s. file)
  bool is_tracking_file(const string &path) const {
    const string name = basename_(path);
    return name.size() > 2 && name.compare(0, 2, "s.") == 0;
  }

  // Convert a working file basename to tracking file basename
  string tracking_basename(const string &name) const {
    return "s." + name;
  }

  // Convert a tracking file basename to a working file basename
  // (i.e. strip ,v or s.)
  string working_basename(const string &name) const {
    return name.substr(2);
  }

  int native_diff(const vector<string> &files) const {
    // sccs diffs works OK on nontrivial paths
    return execute("sccs",
                   EXE_STR, "diffs",
                   EXE_STR, "-u",
                   EXE_VECTOR|EXE_DOTSTUFF, &files,
                   EXE_END);
  }

  int native_commit(const vector<string> &files, const string &msg) const {
    int rc = 0;
    for(size_t n = 0; n < files.size() && !rc; ++n) {
      InDirectory id(dirname_(files[n]));
      string base = basename_(files[n]);
      if(is_tracked(base)) {
        rc = execute("sccs",
                     EXE_STR, "delget",
                     EXE_STR, ("-y" + string(msg)).c_str(),
                     EXE_STR|EXE_DOTSTUFF, base.c_str(),
                     EXE_END);
      } else {
        // TODO binary files
        rc = execute("sccs",
                     EXE_STR, "create",
                     EXE_IFSTR(is_binary(base), "-b"),
                     EXE_STR, ("-y" + string(msg)).c_str(),
                     EXE_STR|EXE_DOTSTUFF, base.c_str(),
                     EXE_END);
      }
    }
    return rc;
  }

  int native_revert(int nfiles, char **files) const {
    int rc = 0;
    for(int n = 0; n < nfiles && !rc; ++n) {
      InDirectory id(dirname_(files[n]));
      string base = basename_(files[n]);
      rc = execute("sccs",
                   EXE_STR, "unedit",
                   EXE_STR|EXE_DOTSTUFF, base.c_str(),
                   EXE_END);
    }
    return rc;
  }

  int native_update(const vector<string> &files) const {
    int rc = 0;
    for(size_t n = 0; n < files.size() && !rc; ++n) {
      InDirectory id(dirname_(files[n]));
      string base = basename_(files[n]);
      rc = execute("sccs",
                   EXE_STR, "get",
                   EXE_STR|EXE_DOTSTUFF, base.c_str(),
                   EXE_END);
    }
    return rc;
  }

  int log(const char *path) const {
    if(!path)
      fatal("'vcs log' requires a filename with SCCS");
    // sccs prs works OK on nontrivial paths
    return execute("sccs",
                   EXE_STR, "prs",
                   EXE_STR|EXE_DOTSTUFF, path,
                   EXE_END);
  }

  int native_edit(int nfiles, char **files) const {
    int rc = 0;
    for(int n = 0; n < nfiles && !rc; ++n) {
      InDirectory id(dirname_(files[n]));
      string base = basename_(files[n]);
      rc = execute("sccs",
                   EXE_STR, "edit",
                   EXE_STR|EXE_DOTSTUFF, base.c_str(),
                   EXE_END);
    }
    return rc;
  }

};

static const sccs vcs_sccs;

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
