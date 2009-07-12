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

// Get the version of Mercurial
static const string &hg__version() {
  static string version;

  if(!version.size()) {
    vector<string> v;
    const int rc = capture(v, "hg", "--version", (char *)NULL);

    if(rc)
      fatal("hg --version exited with status %d", rc);
    for(size_t n = 0; n < v.size(); ++n) {
      // "Mercurial Distributed SCM (version 0.9.1)"
      string::size_type start = v[n].find("version ");
      if(start == string::npos)
        continue;
      start += 8;
      string::size_type end = v[n].find(')', start);
      version.assign(v[n], start, end - start);
    }
    if(v.size() == 0)
      version = "unknown";
    if(debug)
      fprintf(stderr, "Detected Mercurial version: \"%s\"\n", version.c_str());
  }
  return version;
}

static int hg_diff(int nfiles, char **files) {
  return execute("hg",
                 EXE_STR, "diff",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int hg_add(int /*binary*/, int nfiles, char **files) {
  return execute("hg",
                 EXE_STR, "add",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int hg_remove(int force, int nfiles, char **files) {
  // --force option was added in change:
  //
  // changeset:   1867:91ebf29c1595
  // parent:      1865:1ed809a2104e
  // user:        Vadim Gelfer <vadim.gelfer@gmail.com>
  // date:        Wed Mar 08 15:14:24 2006 -0800
  // summary:     add -f/--force to remove command.
  //
  // This lies between tags 0.8 (1665:3a56574f329a) and 0.8.1
  // (2051:6a03cff2b0f5).
  return execute("hg",
                 EXE_STR, "remove",
                 EXE_IFSTR(force
                           && version_compare(hg__version(), "0.8.1") >= 0,
                           "--force"),
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int hg_commit(const char *msg, int nfiles, char **files) {
  return execute("hg",
                 EXE_STR, "commit",
                 EXE_IFSTR(msg, "-m"),
                 EXE_IFSTR(msg, msg),
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int hg_revert(int nfiles, char **files) {
  // From 0.9.2, "hg revert now requires the -a flag to revert all files"
  // (see http://www.selenic.com/mercurial/wiki/index.cgi/WhatsNew/Archive)
  // Before that 'hg revert' with no args reverted all files.
  if(nfiles)
    return execute("hg",
                   EXE_STR, "revert",
                   EXE_STRS, nfiles, files,
                   EXE_END);
  else if(version_compare(hg__version(), "0.9.2") >= 0)
    return execute("hg",
                   EXE_STR, "revert",
                   EXE_STR, "--all",
                   EXE_END);
  else
    return execute("hg",
                   EXE_STR, "revert",
                   EXE_END);
}

static int hg_status() {
  return execute("hg",
                 EXE_STR, "status",
                 EXE_END);
}

static int hg_update() {
  return execute("hg",
                 EXE_STR, "pull",
                 EXE_STR, "--update",
                 EXE_END);
}

static int hg_log(const char *path) {
  return execute("hg",
                 EXE_STR, "log",
                 EXE_IFSTR(path, path),
                 EXE_END);
}

static int hg_annotate(const char *path) {
  return execute("hg",
                 EXE_STR, "annotate",
                 EXE_STR, path,
                 EXE_END);
}

static int hg_clone(const char *uri, const char *dir) {
  return execute("hg",
                 EXE_STR, "clone",
                 EXE_STR, uri,
                 EXE_IFSTR(dir, dir),
                 EXE_END);
}

const struct vcs vcs_hg = {
  "Mercurial",
  hg_diff,
  hg_add,
  hg_remove,
  hg_commit,
  hg_revert,
  hg_status,
  hg_update,
  hg_log,
  NULL,                                 // edit
  hg_annotate,
  hg_clone,
};

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
