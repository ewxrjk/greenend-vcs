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
#include <dirent.h>
#include <algorithm>

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

static string fullpath(const string &path, const string &file) {
  if(path.size())
    return path + PATHSEPSTR + file;
  else
    return file;
}

static void listfiles_recurse(string path,
                              list<string> &files,
                              set<string> &ignored) {
  DIR *dp;
  struct dirent *de;
  vector<string> dirs_here, files_here;
  list<string> ignores_here;

  if(!(dp = opendir(path.size() ? path.c_str() : ".")))
    fatal("opening directory %s: %s",
          path.c_str(), strerror(errno));
  read_ignores(ignores_here, fullpath(path, ".vcsignore"));
  for(;;) {
    errno = 0;
    if(!(de = readdir(dp))) {
      if(errno)
        fatal("reading directory %s: %s",
              path.c_str(), strerror(errno));
      break;
    }
    const string name = de->d_name;
    const string fullname = fullpath(path, name);

    // Skip filesystem scaffolding
    if(name == "."
       || name == "..")
      continue;
    // Identify ignored files
    const int ignoreme = (is_ignored(ignores_here, name)
                          || is_ignored(global_ignores, name));
    if(isdir(fullname, 0)) {
      if(!ignoreme)
        dirs_here.push_back(fullname);
    } else if(isreg(fullname, 0)) {
      files_here.push_back(fullname);
      if(ignoreme)
        ignored.insert(fullname);
    }
    // Symlinks, devices and whatnot are, for now at least, implicitly ignored.
  }
  closedir(dp);
  // Put files into a consistent order (albeit not necessarily a very idiomatic
  // one) and add them to the list
  sort(files_here.begin(), files_here.end());
  for(vector<string>::const_iterator it = files_here.begin();
      it != files_here.end();
      ++it)
    files.push_back(*it);
  // Put directories into a consistent order
  sort(dirs_here.begin(), dirs_here.end());
  // We scan subdirectories after completing the file list for two reasons:
  // 1) It means all the regular files in a directory are grouped together
  //    before any subdirectories
  // 2) It limits the number of FDs we have open at any one time.
  for(vector<string>::const_iterator it = dirs_here.begin();
      it != dirs_here.end();
      ++it)
    listfiles_recurse(*it, files, ignored);
}

// Get a list of files below a directory plus a set of those that are ignored.
// The ignored files WILL be in the list.
void listfiles(string path,
               list<string> &files,
               set<string> &ignored) {
  files.clear();
  ignored.clear();
  init_global_ignores();
  listfiles_recurse(path, files, ignored);
  if(debug > 1) {
    fprintf(stderr, "listfiles output:\n");
    for(list<string>::const_iterator it = files.begin();
        it != files.end();
        ++it) {
      fprintf(stderr, "| %s%s\n", it->c_str(),
              ignored.find(*it) == ignored.end() ? "" : " - IGNORED");
    }
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
