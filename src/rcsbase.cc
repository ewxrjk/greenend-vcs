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
#include "Dir.h"

rcsbase::~rcsbase() {}

bool rcsbase::detect(void) const {
  // Look for tracking files in the current directory.
  Dir d(".");
  string name;
  while(d.get(name))
    if(is_tracking_file(name))
      return true;
  return false;
}

string rcsbase::tracking_path(const string &path) const {
  const string dir = parentdir(path, true);
  const string tname = tracking_basename(basename_(path));
  const string tpath = dir + "/" + tname;
  const string tdir = dir + "/" + tracking_directory();
  const string tdpath = tdir + "/" + tname;
  if(!exists(tpath) && exists(tdir))
    return tdpath;
  return tpath;
}

bool rcsbase::is_tracked(const string &path) const {
  const string dir = parentdir(path, true);
  const string tname = tracking_basename(basename_(path));
  return (exists(dir + "/" + tname)
          || exists(dir + "/" + tracking_directory() + "/" + tname));
}

bool rcsbase::is_add_flag(const string &path) const {
  return basename_(path).compare(0, 6, ".#add#") == 0;
}

string rcsbase::flag_path(const string &path) const {
  return parentdir(path, true) + "/.#add#" + basename_(path);
}

bool rcsbase::is_flagged(const string &path) const {
  return exists(flag_path(path));
}

string rcsbase::work_path(const string &path) const {
  if(is_add_flag(path)) {
    if(path.find('/') == std::string::npos)
      return path.substr(6);
    else
      return parentdir(path) + "/" + basename_(path).substr(6);
  } else if(is_tracking_file(path)) {
    string d = parentdir(path);
    string b = basename_(path);
    if(basename_(d) == tracking_directory()) {
      if(d == tracking_directory())
        return working_basename(b);
      else
        return parentdir(d) + "/" + working_basename(b);
    } else
      return d + "/" + working_basename(b);
  } else {
    return path;
  }
}

// Get information about files below the current directory
void rcsbase::enumerate(map<string,int> &files) const {
  files.clear();
  list<string> filenames;
  set<string> ignored;
  listfiles("", filenames, ignored, true);
  for(list<string>::iterator it = filenames.begin();
      it != filenames.end();
      ++it) {
    const string &name = *it;
    if(is_tracking_file(name))
      files[work_path(name)] |= fileTracked;
    else if(is_add_flag(name))
      files[work_path(name)] |= fileAdded;
    else {
      files[name] |= fileExists;
      if(writable(name))
        files[name] |= fileWritable;
      if(ignored.find(name) != ignored.end())
        files[name] |= fileIgnored;
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
