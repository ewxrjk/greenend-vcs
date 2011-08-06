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

int rcsbase::diff(int nfiles, char **files) const {
  vector<char *> native;
  vector<char *> added;
  if(nfiles == 0) {
    map<string,int> allFiles;
    enumerate(allFiles);
    for(map<string,int>::iterator it = allFiles.begin();
        it != allFiles.end();
        ++it) {
      int flags = it->second;
      if((flags & fileTracked)
         && (flags & fileWritable))
        native.push_back(xstrdup(it->first.c_str()));
      else if(flags & fileAdded)
        added.push_back(xstrdup(it->first.c_str()));
    }
  } else {
    for(int n = 0; n < nfiles; ++n) {
      if(is_tracked(files[n])) {
        if(!writable(files[n]) || !exists(files[n]))
          continue;
        native.push_back(files[n]);
      } else if(is_flagged(files[n]) && exists(files[n])) {
        added.push_back(files[n]);
      } else if(exists(files[n])) {
        fprintf(stderr, "WARNING: %s is not under %s control\n",
                name, files[n]);
      } else {
        fprintf(stderr, "WARNING: %s does not exist\n", files[n]);
      }
    }
  }
  int rc = 0;
  if(native.size())
    rc = native_diff(native.size(), &native[0]);
  for(int n = 0; n < (int)added.size(); ++n)
    rc |= execute("diff",
                  EXE_STR, "-u",
                  EXE_STR, "/dev/null",
                  EXE_STR|EXE_DOTSTUFF, added[n],
                  EXE_END);
  return (rc & 2 ? 2 : rc);
}

int rcsbase::add(int binary, int nfiles, char **files) const {
  std::vector<char *> flags;
  for(int n = 0; n < nfiles; ++n) {
    if(isdir(files[n])) {
      const std::string rcsdir = std::string(files[n]) + "/" + tracking_directory();
      if(!exists(rcsdir)) {
        int rc = execute("mkdir",
                         EXE_STR, rcsdir.c_str(),
                         EXE_END);
        if(rc)
          return rc;
      }
    } else if(isreg(files[n])) {
      if(!is_tracked(files[n]))
        flags.push_back(xstrdup(flag_path(files[n]).c_str()));
    } else
      fatal("%s is not a regular file", files[n]);
  }
  if(!dryrun) {
    for(size_t n = 0; n < flags.size(); ++n) {
      FILE *fp = fopen(flags[n], "w");
      if(!fp)
        fatal("opening %s: %s\n", flags[n], strerror(errno));
      if(fprintf(fp, "%d\n", binary) < 0
         || fclose(fp) < 0)
        fatal("writing %s: %s\n", flags[n], strerror(errno));
    }
  }
  return 0;
}

int rcsbase::commit(const char *msg, int nfiles, char **files) const {
  vector<char *> newfiles;
  if(nfiles == 0) {
    map<string,int> allFiles;
    enumerate(allFiles);
    for(map<string,int>::iterator it = allFiles.begin();
        it != allFiles.end();
        ++it) {
      int flags = it->second;
      if(flags & fileAdded)
        newfiles.push_back(xstrdup(it->first.c_str()));
      else if(flags & fileTracked)
        if(flags & fileWritable)
          newfiles.push_back(xstrdup(it->first.c_str()));            
    }
    files = &newfiles[0];
    nfiles = newfiles.size();
  }
  if(!msg) {
    // Gather one commit message for all the files
    vector<string> message;
    message.push_back("# Committing:");
    for(int n = 0; n < nfiles; ++n)
      message.push_back(string("#  ") + files[n]);
    int rc = editor(message);
    if(rc)
      return rc;
    string s;
    for(size_t n = 0; n < message.size(); ++n) {
      if(message[n].size() && message[n][0] == '#')
        continue;             // Exclude comments
      s += message[n];
      s += "\n";
    }
    // Zap trailing newlines
    while(s.size() && s[s.size()-1] == '\n')
      s.erase(s.size()-1);
    msg = xstrdup(s.c_str());
  }
  int rc = native_commit(nfiles, files, msg);
  // Clean up .#add# files
  std::vector<char *> cleanup;
  for(int n = 0; n < nfiles; ++n)
    if(is_tracked(files[n]) && is_flagged(files[n]))
      cleanup.push_back(xstrdup(flag_path(files[n]).c_str()));
  if(cleanup.size())
    execute("rm",
            EXE_STR, "-f", 
            EXE_STRS, (int)cleanup.size(), &cleanup[0],
            EXE_END);
  return rc;
}

int rcsbase::remove(int force, int nfiles, char **files) const {
  // Use rm as a convenient way of making -n/-v work properly.
  for(int n = 0; n < nfiles; ++n) {
    if(is_tracked(files[n])) {
      string tpath = tracking_path(files[n]);
      if(execute("rm",
                 EXE_STR, "-f",
                 EXE_STR, "--",
                 EXE_STR, tpath.c_str(),
                 EXE_END))
        return 1;
      if(force && exists(files[n]) && execute("rm",
                                              EXE_STR, "-f",
                                              EXE_STR, "--",
                                              EXE_STR, files[n],
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

int rcsbase::edit(int nfiles, char **files) const {
  // Filter down to files not already edited
  std::vector<char *> filtered;
  for(int n = 0; n < nfiles; ++n)
    if(!exists(files[n]) || !writable(files[n]))
      filtered.push_back(files[n]);
  if(!filtered.size())
    return 0;
  return native_edit(filtered.size(), &filtered[0]);
}

int rcsbase::update() const {
  // 'update' is treated as meaning 'ensure working files exist'
  //
  // It would be nice to merge in changes made subsequent to the current
  // version of the working file being checked out (i.e. what cvs up does).
  // But vcs has no idea what the base revision is, so this is not possible.
  std::vector<char *> missing;
  map<string,int> allFiles;
  enumerate(allFiles);
  for(map<string,int>::iterator it = allFiles.begin();
      it != allFiles.end();
      ++it) {
    int flags = it->second;
    if(!(flags & fileExists))
      missing.push_back(xstrdup(it->first.c_str()));
  }
  if(missing.size() == 0)
    return 0;
  return native_update(missing.size(), &missing[0]);
}

int rcsbase::revert(int nfiles, char **files) const {
  vector<char *> checkout, unadd;
  if(nfiles == 0) {
    map<string,int> allFiles;
    enumerate(allFiles);
    for(map<string,int>::iterator it = allFiles.begin();
        it != allFiles.end();
        ++it) {
      int flags = it->second;
      if((flags & fileTracked)
         && ((flags & fileWritable)
             || !(flags & fileExists)))
        checkout.push_back(xstrdup(it->first.c_str()));
      else if(flags & fileAdded)
        unadd.push_back(xstrdup(it->first.c_str()));
    }
  } else {
    for(int n = 0; n < nfiles; ++n) {
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
    int rc = execute("rm",
                     EXE_STR, "-f",
                     EXE_STR, flag_path(unadd[n]).c_str(),
                     EXE_END);
    if(rc)
      return rc;
  }
  if(checkout.size())
    return native_revert((int)checkout.size(), &checkout[0]);
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
