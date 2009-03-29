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

// Guess what VCS is in use; returns a pointer to its operation table.
// Terminates the process if no VCS can be found.
const struct vcs *guess() {
  // CVS is easy as it has a CVS directory at every level.
  if(isdir("CVS"))
    return &vcs_cvs;
  // Subversion too
  if(isdir(".svn"))
    return &vcs_svn;
  // Bazaar and Git only have their dot directories at the top level of the
  // branch.
  string d = cwd();
  for(;;) {
    if(isdir(d + PATHSEPSTR + ".bzr"))
      return &vcs_bzr;
    if(isdir(d + PATHSEPSTR + ".git"))
      return &vcs_git;
    if(isdir(d + PATHSEPSTR + ".hg"))
      return &vcs_hg;
    if(isroot(d))
      break;
    d = parentdir(d);
  }
  // Only attempt to detect Perforce if some Perforce-specific environment
  // variable is set.  Also, we do it last since invoking a program is much
  // more intrusive than just testing files.
  if(getenv("P4PORT") || getenv("P4CONFIG") || getenv("P4CLIENT")) {
    vector<string> lines;
    if(!capture(lines, "p4", "changes", "-m1", "...", (char *)NULL))
      return &vcs_p4;
  }
  fatal("cannot identify native version control system");
}

// Table mapping scheme names to version control systems
static const struct vcs_scheme {
  const char *scheme;
  const struct vcs *vcs;
} scheme_mapping[] = {
  { "git", &vcs_git },
  { "svn", &vcs_svn },
  { "svn+ssh", &vcs_svn },
  { 0, 0 }
};

// Table mapping subdirectories to version control systems
static const struct vcs_subdir {
  const char *substring;
  const char *subdir;
  const struct vcs *vcs;
} subdir_mapping[] = {
  { "git", ".git", &vcs_git },
  { "bzr", ".bzr", &vcs_bzr },
  { "hg", ".hg", &vcs_hg },
  // Last since 'HEAD' isn't a dotfile:
  { "git", "HEAD", &vcs_git },
  { 0, 0, 0 }
};

// Guess what VCS a named branch belongs to
const struct vcs *guess_branch(string uri) {
  // Perhaps we can guess just by looking at the URI
  const string scheme = uri_scheme(uri);
  if(scheme != "") {
    for(const struct vcs_scheme *s = scheme_mapping; s->scheme; ++s)
      if(scheme == s->scheme)
        return s->vcs;
  } else {
    // If there is no scheme then we assume it to be a local URI
    if(uri.size() && uri.at(0) == '/')
      uri = "file://" + uri;
    else
      uri = "file:///" + cwd() + "/" + uri;
  }

  // Otherwise we must inspect what we find there.

  // First we try to use substrings as hints
  for(const struct vcs_subdir *s = subdir_mapping; s->subdir; ++s)
    if(s->substring && uri.find(s->substring) != string::npos)
      if(uri_exists(uri + "/" + s->subdir))
        return s->vcs;

  // Failing that we try without hints
  for(const struct vcs_subdir *s = subdir_mapping; s->subdir; ++s)
    if(uri_exists(uri + "/" + s->subdir))
      return s->vcs;
  
  fatal("cannot identify version control system");
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
