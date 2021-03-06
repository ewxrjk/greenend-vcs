/*
 * This file is part of VCS
 * Copyright (C) 2009-2011, 2014 Richard Kettlewell
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

class git: public vcs {
public:
  git(): vcs("Git") {
    register_subdir(".git");
    register_subdir("HEAD");
    register_scheme("git");
    register_substring("git");
  }

  int diff(const vector<string> &files) const {
    /* 'vcs diff' wants the difference between the working tree and the head (not
     * between the index and anything) */
    return execute("git",
                   EXE_STR, "diff",
                   EXE_STR, "HEAD",
                   EXE_STR, "--",
                   EXE_VECTOR, &files,
                   EXE_END);
  }

  int add(int /*binary*/, const vector<string> &files) const {
    /* 'git add' is a bit unlike 'vcs add' in that it actually stages files for
     * later commit.  But it's also the only (native) way to mark previously
     * uncontrolled files as version-controlled, so we go with it anyway. */
    return execute("git",
                   EXE_STR, "add",
                   EXE_STR, "--",
                   EXE_VECTOR, &files,
                   EXE_END);
  }

  int remove(int force, const vector<string> &files) const {
    // Very old git lacks the 'rm' command.  Without strong external demand I
    // don't see any point in coping; if you have such a decrepit and ancient
    // version you can either put up with vcs not working or upgrade git to
    // something marginally more recent.
    return execute("git",
                   EXE_STR, "rm",
                   EXE_IFSTR(force, "-f"),
                   EXE_STR, "--",
                   EXE_VECTOR, &files,
                   EXE_END);
  }

  int commit(const string *msg, const vector<string> &files) const {
    if(files.size() == 0) {
      /* Automatically stage and commit everything in sight */
      return execute("git",
                     EXE_STR, "commit",
                     EXE_STR, "-a",
                     EXE_IFSTR(msg, "-m"),
                     EXE_STRING|EXE_OPT, msg,
                     EXE_END);
    } else {
      /* Just commit the named files */
      return execute("git",
                     EXE_STR, "commit",
                     EXE_IFSTR(msg, "-m"),
                     EXE_STRING|EXE_OPT, msg,
                     EXE_STR, "--",
                     EXE_VECTOR, &files,
                     EXE_END);
    }
  }

  int revert(const vector<string> &files) const {
    if(files.size()) {
      /* git-checkout can be used to reset individual modified files (included
       * ones added to the index and deleted ones) but doesn't affect completely
       * new files.  So we need identify newly added files among those on our
       * command line. */
      // First put the list of files to revert into something we can efficiently
      // index.
      set<string> revertfiles;
      for(size_t n = 0; n < files.size(); ++n)
        revertfiles.insert(files[n]);
      // Get the current tree status
      vector<string> status;
      capture(status, "git", "status", (char *)NULL);
      // Find the set of new files
      set<string> newfiles;
      for(size_t n = 0; n < status.size(); ++n) {
        const string &line = status[n];
        size_t pos = 0;
        // Old versions of git put a # at the start, new ones don't
        if(pos < line.size() && line[pos] == '#')
           ++pos;
        while(pos < line.size() && isspace(line[pos]))
          ++pos;
        static const char prefix[] = "new file:";
        if(line.compare(pos, (sizeof prefix)-1, prefix) == 0) {
          // It's a new file; parse out the filename
          pos += sizeof prefix - 1;
          while(pos < line.size() && isspace(line[pos]))
            ++pos;
          const string path = line.substr(pos, string::npos);
          // If it's one of the targets, add it to the set to remove and remove
          // from the set to checkout.
          if(revertfiles.find(path) != revertfiles.end()) {
            newfiles.insert(path);
            revertfiles.erase(path);
          }
        }
      }
      int rc = 0;
      if(newfiles.size()) {
        rc = execute("git",
                     EXE_STR, "rm",
                     EXE_STR, "-f",
                     EXE_STR, "--",
                     EXE_SET, &newfiles,
                     EXE_END);
      }
      if(!rc && revertfiles.size()) {
        rc = execute("git",
                     EXE_STR, "checkout",
                     EXE_STR, "HEAD",
                     EXE_STR, "--",
                     EXE_SET, &revertfiles,
                     EXE_END);
      }
      return rc;
    } else {
      /* git-reset will reset the whole tree. */
      return execute("git",
                     EXE_STR, "reset",
                     EXE_STR, "--hard",
                     EXE_STR, "HEAD",
                     EXE_END);
    }
  }

  int status() const {
    execute("git",
            EXE_STR, "status",
            EXE_END);
    /* 'git status' is documented as exiting nonzero if there is nothing to
     * commit.  In fact this is a lie, if stdout is a tty then it will always
     * exit 0.  We ignore the essentially random exit status, regardless. */
    return 0;
  }

  int update() const {
    return execute("git",
                   EXE_STR, "pull",
                   EXE_END);
  }

  int log(const string *path) const {
    return execute("git",
                   EXE_STR, "log",
                   EXE_STR, "--",
                   EXE_STRING|EXE_OPT, path,
                   EXE_END);
  }

  int annotate(const string &path) const {
    return execute("git",
                   EXE_STR, "blame",
                   EXE_STR, "--",
                   EXE_STRING, &path,
                   EXE_END);
  }

  int clone(const string &uri, const string *dir) const {
    return execute("git",
                   EXE_STR, "clone",
                   EXE_STR, "--",
                   EXE_STRING, &uri,
                   EXE_STRING|EXE_OPT, dir,
                   EXE_END);
  }

  int rename(const vector<string> &sources, const string &destination) const {
    // git mv allegedly supports multiple sources if the destination is a
    // directory but (at least in 1.6.4.2) this does not actually work.
    // Therefore we break the command up into multiple operations.
    for(size_t n = 0; n < sources.size(); ++n) {
      int rc =  execute("git",
                        EXE_STR, "mv",
                        EXE_STR, "--",
                        EXE_STRING, &sources[n],
                        EXE_STRING, &destination,
                        EXE_END);
      if(rc)
        return rc;
    }
    return 0;
  }

  int show(const string &change) const {
    return execute("git",
                   EXE_STR, "show",
                   EXE_STRING, &change,
                   EXE_END);
  }
};

static git vcs_git;

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
