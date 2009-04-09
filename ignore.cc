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
#include <fnmatch.h>

// Global ignore list
list<string> global_ignores;

// Initialize global_ignores
void init_global_ignores() {
  const string path = string(getenv("HOME")) + PATHSEPSTR + ".vcsignore";
  read_ignores(global_ignores, path);
}

// Read an ignored list
void read_ignores(list<string> &ignores, const string &path) {
  ignores.clear();
  if(!exists(path))
    return;
  FILE *fp;
  if(!(fp = fopen(path.c_str(), "r")))
    fatal("cannot open %s: %s", path.c_str(), strerror(errno));
  string l;
  while(readline(path, fp, l))
    if(l.size())
      ignores.push_back(l);
  fclose(fp);
}

// Return true if FILE is ignored by IGNORES
int is_ignored(const list<string> &ignores,
               const string &file) {
  for(list<string>::const_iterator it = ignores.begin();
      it != ignores.end();
      ++it)
    if(fnmatch(it->c_str(), file.c_str(), 0) == 0)
      return 1;
  // TODO we should not be using fnmatch().  Instead we should have a uniform
  // pattern matching function of our own.  Otherwise which files are ignored
  // will depend on the local C library's fnmatch() (or on fixups people add
  // locally.)
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
