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

vcs::vcs(const char *name_): name(name_) {
  if(!selves)
    selves = new selves_t();
  selves->push_back(this);
}

void vcs::register_scheme(const string &s) {
  if(!schemes)
    schemes = new schemes_t();
  (*schemes)[s] = this;
}

void vcs::register_substring(const string &s) {
  if(!substrings)
    substrings = new substrings_t();
  substrings->push_back(pair<string,vcs *>(s, this));
}

void vcs::register_subdir(const string &s) {
  if(!subdirs)
    subdirs = new substrings_t();
  subdirs->push_back(pair<string,vcs *>(s, this));
}

bool vcs::detect(void) const {
  return false;
}

int vcs::edit(int, char **) const {
  return 0;
}

int vcs::clone(const char *, const char *) const {
  fatal("guess_branch returned VCS '%s' which has no clone method!",
        name);
}

vcs::selves_t *vcs::selves;
vcs::schemes_t *vcs::schemes;
vcs::substrings_t *vcs::substrings;
vcs::substrings_t *vcs::subdirs;

// Guess what VCS is in use; returns a pointer to its operation table.
// Terminates the process if no VCS can be found.
const vcs *vcs::guess() {
  // Look for a magic directory in the current directory
  for(substrings_t::const_iterator it = subdirs->begin();
      it != subdirs->end();
      ++it)
    if(isdir(it->first))
      return it->second;
  // Try slow, complicated detection, for systems that need it
  for(selves_t::const_iterator it = selves->begin();
      it != selves->end();
      ++it) {
    vcs *v = *it;
    if(v->detect())
      return v;
  }
  // Some systems only have their dot directories at the top level of the
  // branch, so we work our way back up.
  string d = cwd();
  for(;;) {
    for(substrings_t::const_iterator it = subdirs->begin();
        it != subdirs->end();
        ++it)
      if(isdir(d + PATHSEPSTR + it->first))
        return it->second;
    if(isroot(d))
      break;
    d = parentdir(d);
  }
  fatal("cannot identify native version control system");
}

// Guess what VCS a named branch belongs to
const vcs *vcs::guess_branch(string uri) {
  // Perhaps we can guess just by looking at the URI
  const string scheme = uri_scheme(uri);
  if(scheme != "") {
    if(schemes->find(scheme) != schemes->end())
      return schemes->find(scheme)->second;
  } else {
    // If there is no scheme then we assume it to be a local URI
    if(uri.size() && uri.at(0) == '/')
      uri = "file://" + uri;
    else
      uri = "file:///" + cwd() + "/" + uri;
  }

  // Otherwise we must inspect what we find there.

  // First we try to use substrings as hints
  for(substrings_t::const_iterator it = substrings->begin();
      it != substrings->end();
      ++it)
    if(uri.find(it->first) != string::npos)
      return it->second;

  // Failing that we try without hints
  for(substrings_t::const_iterator it = subdirs->begin();
      it != subdirs->end();
      ++it)
    if(uri_exists(uri + "/" + it->first))
      return it->second;

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
