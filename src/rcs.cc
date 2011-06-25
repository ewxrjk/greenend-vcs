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
#include "Dir.h"

class rcs: public vcs {
public:
  rcs(): vcs("RCS") {
    register_subdir("RCS");
  }

  bool detect(void) const {
    // Look for ,v files in the current directory.
    Dir d(".");
    string name;
    while(d.get(name))
      if(isRcsfile(name))
        return true;
    return false;
  }

  static bool isRcsfile(const string &path) {
    return path.size() > 2 && path.compare(path.size() - 2, 2, ",v") == 0;
  }

  static void getRawName(const string &path,
                         string &name) {
    string::size_type lastSlash = path.rfind('/');
    string::size_type pos = (lastSlash == string::npos
                                  ? 0 : lastSlash +1);
    name.assign(path, pos, path.size() - pos - 2);
  }

  // Get a list of RCS-managed files in the current directory,
  // optionally restricting to writable files only.
  static void getFiles(vector<string> &files,
                       bool onlyWritable = false) {
    const char *dirs[] = { "RCS", "." };
    string name, rawName;
    files.clear();
    for(size_t i = 0; i < sizeof dirs / sizeof *dirs; ++i) {
      if(exists(dirs[i])) {
        Dir d(dirs[i]);
        while(d.get(name)) {
          if(isRcsfile(name)) {
            getRawName(name, rawName);
            if(!onlyWritable || writable(rawName))
              files.push_back(rawName);
          }
        }
      }
    }
  }

  static char **getFiles(int &nfiles, bool onlyWritable) {
    vector<string> fileList;
    getFiles(fileList, onlyWritable);
    nfiles = fileList.size();
    char **files = new char *[nfiles + 1];
    for(int n = 0; n < nfiles; ++n)
      files[n] = xstrdup(fileList[n].c_str());
    files[nfiles] = NULL;
    return files;
  }

  int diff(int nfiles, char **files) const {
    if(nfiles == 0)
      files = getFiles(nfiles, true);
    if(!nfiles)
      return 0;                 // no writable files -> no differences
    return execute("rcsdiff",
                   EXE_STR, "-u",
                   EXE_STRS|EXE_DOTSTUFF, nfiles, files,
                   EXE_END);
  }


  int add(int binary, int nfiles, char **files) const {
    if(binary)
      fatal("--binary option not supported for RCS");
    for(int n = 0; n < nfiles; ++n) {
      string opt_t = "-t-" + string(files[n]);
      int rc;
      if((rc = execute("ci",
                       EXE_STR, "-l", // keep writable, don't delete
                       EXE_STR, opt_t.c_str(),
                       EXE_STR, "-mInitial commit",
                       EXE_STR|EXE_DOTSTUFF, files[n],
                       EXE_END)))
        return rc;
    }
    return 0;
  }

  int remove(int force, int nfiles, char **files) const {
    // We do everything rm as a convenient way of making -n/-v work
    // properly.
    for(int n = 0; n < nfiles; ++n) {
      string rcsfile;
      rcsfile = string(files[n]) + ",v";
      if(exists(rcsfile) && execute("rm",
                                    EXE_STR, "-f",
                                    EXE_STR, "--",
                                    EXE_STR, rcsfile.c_str(),
                                    EXE_END))
          return 1;
      rcsfile = parentdir(rcsfile) + "/RCS/" + basename_(rcsfile);
      if(exists(rcsfile) && execute("rm",
                                    EXE_STR, "-f",
                                    EXE_STR, "--",
                                    EXE_STR, rcsfile.c_str(),
                                    EXE_END))
          return 1;
      if(force && exists(files[n]) && execute("rm",
                                              EXE_STR, "-f",
                                              EXE_STR, "--",
                                              EXE_STR, files[n],
                                              EXE_END))
        return 1;
    }
    return 0;
  }

  int commit(const char *msg, int nfiles, char **files) const {
    if(nfiles == 0) {
      files = getFiles(nfiles, true);
      if(nfiles == 0)
        fatal("nothing to commit");
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
    vector<string> command;
    command.push_back("ci");
    command.push_back("-u");    // don't delete work file
    command.push_back("-m" + string(msg));
    for(int n = 0; n < nfiles; ++n)
      command.push_back(files[n]); // TODO ./
    if(dryrun || verbose)
      display_command(command);
    return dryrun ? 0 : execute(command);
  }

  int revert(int nfiles, char **files) const {
    return execute("co",
                   EXE_STR, "-f",
                   EXE_STRS|EXE_DOTSTUFF, nfiles, files,
                   EXE_END);
  }

  int status() const {
    std::vector<std::string> files;
    std::vector<char *> missing;
    getFiles(files);
    for(size_t n = 0; n < files.size(); ++n) {
      int state = 0;
      if(!exists(files[n]))
        state = 'U';
      else if(writable(files[n]))
        state = 'M';
      if(state)
        writef(stdout, "stdout", "%c %s\n", state, files[n].c_str());
    }
    // TODO we should support .vcsignore and report untracked,
    // unignored files as '?'.
    return 0;
  }

  int update() const {
    // We treat 'update' as meaning 'ensure working files exist'
    std::vector<std::string> files;
    std::vector<char *> missing;
    getFiles(files);
    for(size_t n = 0; n < files.size(); ++n) {
      if(!exists(files[n]))
        missing.push_back(xstrdup(files[n].c_str()));
    }
    if(missing.size() == 0)
      return 0;
    return execute("co",
                   EXE_STRS|EXE_DOTSTUFF, (int)missing.size(), &missing[0],
                   EXE_END);
  }

  int log(const char *path) const {
    return execute("rlog",
                   EXE_STR|EXE_DOTSTUFF, path,
                   EXE_END);
  }

  int annotate(const char */*path*/) const {
    // Anyone fancy implementing this? l-)
    fatal("RCS does not support 'vcs annotate'.");
  }

  int edit(int nfiles, char **files) const {
    // Filter down to files not already edited
    std::vector<char *> filtered;
    for(int n = 0; n < nfiles; ++n)
      if(!exists(files[n]) || !writable(files[n]))
        filtered.push_back(files[n]);
    if(!filtered.size())
      return 0;
    return execute("co",
                   EXE_STR, "-l", // make work file writable
                   EXE_STRS|EXE_DOTSTUFF, (int)filtered.size(), &filtered[0],
                   EXE_END);
  }

};

static const rcs vcs_rcs;

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
