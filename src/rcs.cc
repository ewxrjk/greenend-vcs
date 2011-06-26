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

class rcs: public rcsbase {
public:
  rcs(): rcsbase("RCS") {
    register_subdir("RCS");
  }

  string tracking_directory() const {
    return "RCS";
  }

  bool is_tracking_file(const string &path) const {
    return path.size() > 2 && path.compare(path.size() - 2, 2, ",v") == 0;
  }

  string tracking_basename(const string &name) const {
    return name + ",v";
  }

  string working_basename(const string &name) const {
    return name.substr(0, name.size() - 2);
  }

  int diff(int nfiles, char **files) const {
    std::vector<char *> rcsdiff;
    std::vector<char *> newfiles;
    if(nfiles == 0) {
      map<string,int> allFiles;
      enumerate(allFiles);
      for(map<string,int>::iterator it = allFiles.begin();
          it != allFiles.end();
          ++it) {
        int flags = it->second;
        if((flags & fileTracked)
           && (flags & fileWritable))
          rcsdiff.push_back(xstrdup(it->first.c_str()));
        else if(flags & fileAdded)
          newfiles.push_back(xstrdup(it->first.c_str()));
      }
    } else {
      for(int n = 0; n < nfiles; ++n) {
        if(is_tracked(files[n])) {
          if(!writable(files[n]) || !exists(files[n]))
            continue;
          rcsdiff.push_back(files[n]);
        } else if(is_flagged(files[n]) && exists(files[n])) {
          newfiles.push_back(files[n]);
        } else if(exists(files[n])) {
          fprintf(stderr, "WARNING: %s is not under RCS control\n", files[n]);
        } else {
          fprintf(stderr, "WARNING: %s does not exist\n", files[n]);
        }
      }
    }
    int rc = 0;
    if(rcsdiff.size())
      rc = execute("rcsdiff",
                   EXE_STR, "-u",
                   EXE_STRS|EXE_DOTSTUFF, (int)rcsdiff.size(), &rcsdiff[0],
                   EXE_END);
    for(int n = 0; n < (int)newfiles.size(); ++n)
      rc |= execute("diff",
                    EXE_STR, "-u",
                    EXE_STR, "/dev/null",
                    EXE_STR|EXE_DOTSTUFF, newfiles[n],
                    EXE_END);
    return (rc & 2 ? 2 : rc);
  }

  int add(int binary, int nfiles, char **files) const {
    if(binary)
      fatal("--binary option not supported for RCS");
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
    if(flags.size())
      return execute("touch",
                     EXE_STRS|EXE_DOTSTUFF, (int)flags.size(), &flags[0],
                     EXE_END);
    else
      return 0;
  }

  int remove(int force, int nfiles, char **files) const {
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

  int commit(const char *msg, int nfiles, char **files) const {
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
    vector<string> command;
    command.push_back("ci");
    command.push_back("-u");    // don't delete work file
    command.push_back("-t-" + string(msg));
    command.push_back("-m" + string(msg));
    for(int n = 0; n < nfiles; ++n)
      if(n == 0 && files[n][0] == '-')
        command.push_back("./" + std::string(files[n]));
      else
        command.push_back(files[n]);
    if(dryrun || verbose)
      display_command(command);
    if(dryrun)
      return 0;
    int rc = execute(command);
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

  int revert(int nfiles, char **files) const {
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
      return execute("co",
                     EXE_STR, "-f",
                     EXE_STRS|EXE_DOTSTUFF, (int)checkout.size(), &checkout[0],
                     EXE_END);
    return 0;
  }

  int status() const {
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

  int update() const {
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
    return execute("co",
                   EXE_STRS|EXE_DOTSTUFF, (int)missing.size(), &missing[0],
                   EXE_END);
  }

  int log(const char *path) const {
    if(!path)
      fatal("'vcs log' requires a filename with RCS");
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
