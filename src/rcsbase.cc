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

bool rcsbase::is_binary(const string &path) const {
  string f = flag_path(path);
  FILE *fp = fopen(f.c_str(), "r");
  if(fp)
    return false;
  string l;
  bool rc = false;
  if(readline(f, fp, l)
     && l[0] == '1')
    rc = true;
  fclose(fp);
  return rc;
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
  const string td = tracking_directory();
  listfiles("", filenames, ignored, &td);
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

int rcsbase::diff(const vector<string> &files) const {
  vector<string> native;
  vector<string> added;
  if(files.size() == 0) {
    map<string,int> allFiles;
    enumerate(allFiles);
    for(map<string,int>::iterator it = allFiles.begin();
        it != allFiles.end();
        ++it) {
      int flags = it->second;
      if((flags & fileTracked)
         && (flags & fileWritable))
        native.push_back(it->first);
      else if(flags & fileAdded)
        added.push_back(it->first);
    }
  } else {
    for(size_t n = 0; n < files.size(); ++n) {
      if(is_tracked(files[n])) {
        if(!writable(files[n]) || !exists(files[n]))
          continue;
        native.push_back(files[n]);
      } else if(is_flagged(files[n]) && exists(files[n])) {
        added.push_back(files[n]);
      } else if(exists(files[n])) {
        fprintf(stderr, "WARNING: %s is not under %s control\n",
                name, files[n].c_str());
      } else {
        fprintf(stderr, "WARNING: %s does not exist\n", files[n].c_str());
      }
    }
  }
  int rc = 0;
  if(native.size())
    rc = native_diff(native);
  for(size_t n = 0; n < added.size(); ++n)
    rc |= execute("diff",
                  EXE_STR, "-u",
                  EXE_STR, "/dev/null",
                  EXE_STRING|EXE_DOTSTUFF, &added[n],
                  EXE_END);
  return (rc & 2 ? 2 : rc);
}

int rcsbase::add(int binary, const vector<string> &files) const {
  vector<string> flags;
  for(size_t n = 0; n < files.size(); ++n) {
    if(isdir(files[n])) {
      const std::string rcsdir = files[n] + "/" + tracking_directory();
      if(!exists(rcsdir)) {
        int rc = execute("mkdir",
                         EXE_STRING|EXE_DOTSTUFF, &rcsdir,
                         EXE_END);
        if(rc)
          return rc;
      }
    } else if(isreg(files[n])) {
      if(!is_tracked(files[n]))
        flags.push_back(flag_path(files[n]));
    } else
      fatal("%s is not a regular file", files[n].c_str());
  }
  if(!dryrun) {
    for(size_t n = 0; n < flags.size(); ++n) {
      FILE *fp = fopen(flags[n].c_str(), "w");
      if(!fp)
        fatal("opening %s: %s\n", flags[n].c_str(), strerror(errno));
      if(fprintf(fp, "%d\n", binary) < 0
         || fclose(fp) < 0)
        fatal("writing %s: %s\n", flags[n].c_str(), strerror(errno));
    }
  }
  return 0;
}

int rcsbase::commit(const string *msg, const vector<string> &files) const {
  vector<string> newfiles;
  if(files.size() == 0) {
    map<string,int> allFiles;
    enumerate(allFiles);
    for(map<string,int>::iterator it = allFiles.begin();
        it != allFiles.end();
        ++it) {
      int flags = it->second;
      if(flags & fileAdded)
        newfiles.push_back(it->first.c_str());
      else if(flags & fileTracked)
        if(flags & fileWritable)
          newfiles.push_back(it->first.c_str());
    }
  } else {
    newfiles = files;
  }
  string s;
  if(!msg) {
    // Gather one commit message for all the files
    vector<string> message;
    message.push_back("# Committing:");
    for(size_t n = 0; n < newfiles.size(); ++n)
      message.push_back(string("#  ") + newfiles[n]);
    int rc = editor(message);
    if(rc)
      return rc;
    for(size_t n = 0; n < message.size(); ++n) {
      if(message[n].size() && message[n][0] == '#')
        continue;             // Exclude comments
      s += message[n];
      s += "\n";
    }
    // Zap trailing newlines
    while(s.size() && s[s.size()-1] == '\n')
      s.erase(s.size()-1);
    msg = &s;
  }
  int rc = native_commit(newfiles, *msg);
  // Clean up .#add# files
  vector<string> cleanup;
  for(size_t n = 0; n < newfiles.size(); ++n)
    if(is_tracked(newfiles[n]) && is_flagged(newfiles[n]))
      cleanup.push_back(flag_path(newfiles[n]));
  if(cleanup.size())
    execute("rm",
            EXE_STR, "-f",
            EXE_VECTOR|EXE_DOTSTUFF, &cleanup,
            EXE_END);
  return rc;
}

int rcsbase::remove(int force, const vector<string> &files) const {
  // Use rm as a convenient way of making -n/-v work properly.
  for(size_t n = 0; n < files.size(); ++n) {
    if(is_tracked(files[n])) {
      string tpath = tracking_path(files[n]);
      if(execute("rm",
                 EXE_STR, "-f",
                 EXE_STR, "--",
                 EXE_STRING, &tpath,
                 EXE_END))
        return 1;
      if(force && exists(files[n]) && execute("rm",
                                              EXE_STR, "-f",
                                              EXE_STR, "--",
                                              EXE_STRING, &files[n],
                                              EXE_END))
        return 1;
    }
  }
  return 0;
}

int rcsbase::status() const {
  map<string,int> allFiles;
  enumerate(allFiles);
  for(map<string,int>::iterator it = allFiles.begin();
      it != allFiles.end();
      ++it) {
    int flags = it->second;
    int state;

    if(!(flags & fileExists))
      state = 'U';                    // update required
    else if(flags & fileTracked) {
      if(flags & fileWritable)
        state = 'M';                  // modified
      else
        state = 0;                    // tracked, unmodified
    } else if(flags & fileAdded)
      state = 'A';                    // added
    else if(flags & fileIgnored)
      state = 0;                      // untracked, ignored
    else
      state = '?';                    // untracked, not ignored
    if(state)
      writef(stdout, "stdout", "%c %s\n", state, it->first.c_str());
  }
  return 0;
}

int rcsbase::edit(const vector<string> &files) const {
  // Filter down to files not already edited
  vector<string> filtered;
  for(size_t n = 0; n < files.size(); ++n)
    if(!exists(files[n]) || !writable(files[n]))
      filtered.push_back(files[n]);
  if(!filtered.size())
    return 0;
  return native_edit(filtered);
}

int rcsbase::update() const {
  // 'update' is treated as meaning 'ensure working files exist'
  //
  // It would be nice to merge in changes made subsequent to the current
  // version of the working file being checked out (i.e. what cvs up does).
  // But vcs has no idea what the base revision is, so this is not possible.
  std::vector<string> missing;
  map<string,int> allFiles;
  enumerate(allFiles);
  for(map<string,int>::iterator it = allFiles.begin();
      it != allFiles.end();
      ++it) {
    int flags = it->second;
    if(!(flags & fileExists))
      missing.push_back(it->first);
  }
  if(missing.size() == 0)
    return 0;
  return native_update(missing);
}

int rcsbase::revert(const vector<string> &files) const {
  vector<string> checkout, unadd;
  if(files.size() == 0) {
    map<string,int> allFiles;
    enumerate(allFiles);
    for(map<string,int>::iterator it = allFiles.begin();
        it != allFiles.end();
        ++it) {
      int flags = it->second;
      if((flags & fileTracked)
         && ((flags & fileWritable)
             || !(flags & fileExists)))
        checkout.push_back(it->first);
      else if(flags & fileAdded)
        unadd.push_back(it->first);
    }
  } else {
    for(size_t n = 0; n < files.size(); ++n) {
      if(is_tracked(files[n])) {
        // This file is tracked.  Restore to pristine state either if its
        // modifed or if the working file is missing.
        if(!exists(files[n]) || writable(files[n]))
          checkout.push_back(files[n]);
      } else if(is_flagged(files[n]))
        unadd.push_back(files[n]);
    }
  }
  for(size_t n = 0; n < unadd.size(); ++n) {
    string f = flag_path(unadd[n]);
    int rc = execute("rm",
                     EXE_STR, "-f",
                     EXE_STRING, &f,
                     EXE_END);
    if(rc)
      return rc;
  }
  if(checkout.size())
    return native_revert(checkout);
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
