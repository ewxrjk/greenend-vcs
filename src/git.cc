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

class git: public vcs {
public:

  git(): vcs("Git") {
    register_subdir(".git");
    register_subdir("HEAD");
    register_scheme("git");
    register_substring("git");
  }

  int diff(int nfiles, char **files) const {
    /* 'vcs diff' wants the difference between the working tree and the head (not
     * between the index and anything) */
    return execute("git",
                   EXE_STR, "diff",
                   EXE_STR, "HEAD",
                   EXE_STR, "--",
                   EXE_STRS, nfiles, files,
                   EXE_END);
  }

  int add(int /*binary*/, int nfiles, char **files) const {
    /* 'git add' is a bit unlike 'vcs add' in that it actually stages files for
     * later commit.  But it's also the only (native) way to mark previously
     * uncontrolled files as version-controlled, so we go with it anyway. */
    return execute("git",
                   EXE_STR, "add",
                   EXE_STRS_DOTSTUFF, nfiles, files,
                   EXE_END);
  }

  int remove(int force, int nfiles, char **files) const {
    // Very old git lacks the 'rm' command.  Without strong external demand I
    // don't see any point in coping; if you have such a decrepit and ancient
    // version you can either put up with vcs not working or upgrade git to
    // something marginally more recent.
    return execute("git",
                   EXE_STR, "rm",
                   EXE_IFSTR(force, "-f"),
                   EXE_STRS, nfiles, files,
                   EXE_END);
  }

  int commit(const char *msg, int nfiles, char **files) const {
    if(nfiles == 0) {
      /* Automatically stage and commit everything in sight */
      return execute("git",
                     EXE_STR, "commit",
                     EXE_STR, "-a",
                     EXE_IFSTR(msg, "-m"),
                     EXE_IFSTR(msg, msg),
                     EXE_END);
    } else {
      /* Just commit the named files */
      return execute("git",
                     EXE_STR, "commit",
                     EXE_IFSTR(msg, "-m"),
                     EXE_IFSTR(msg, msg),
                     EXE_STR, "--",
                     EXE_STRS, nfiles, files,
                     EXE_END);
    }
  }

  static char **git_set_to_file_list(int *nfilesp,
                                     set<string> &files) {
    vector<char *> v;
    for(set<string>::iterator it = files.begin();
        it != files.end();
        ++it)
      v.push_back((char *)it->c_str());
    *nfilesp = v.size();
    char **filelist = (char **)calloc(v.size(), sizeof (char *));
    memcpy(filelist, &v[0], v.size() * sizeof (char *));
    return filelist;
  }

  int revert(int nfiles, char **files) const {
    if(nfiles) {
      /* git-checkout can be used to reset individual modified files (included
       * ones added to the index and deleted ones) but doesn't affect completely
       * new files.  So we need identify newly added files among those on our
       * command line. */
      // First put the list of files to revert into something we can efficiently
      // index.
      set<string> revertfiles;
      for(int n = 0; n < nfiles; ++n)
        revertfiles.insert(string(files[n])); 
      // Get the current tree status
      vector<string> status;
      capture(status, "git", "status", (char *)NULL);
      // Find the set of new files
      set<string> newfiles;
      for(size_t n = 0; n < status.size(); ++n) {
        const string &line = status[n];
        static const char prefix[] = "#\tnew file:";
        if(line.compare(0, (sizeof prefix)-1, prefix) == 0) {
          // It's a new file; parse out the filename
          size_t i = sizeof prefix;
          while(i < line.size() && line[i] == ' ')
            ++i;
          const string path = line.substr(i, string::npos);
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
        int nnewfilelist;
        char **newfilelist = git_set_to_file_list(&nnewfilelist, newfiles);
        rc = execute("git",
                     EXE_STR, "rm",
                     EXE_STR, "-f",
                     EXE_STR, "--",
                     EXE_STRS, nnewfilelist, newfilelist,
                     EXE_END);
      }
      if(!rc && revertfiles.size()) {
        int nrevertfilelist;
        char **revertfilelist = git_set_to_file_list(&nrevertfilelist,
                                                     revertfiles);
        rc = execute("git",
                     EXE_STR, "checkout",
                     EXE_STR, "HEAD",
                     EXE_STR, "--",
                     EXE_STRS, nrevertfilelist, revertfilelist,
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

  int log(const char *path) const {
    return execute("git",
                   EXE_STR, "log",
                   EXE_IFSTR(path, path),
                   EXE_END);
  }

  int annotate(const char *path) const {
    return execute("git",
                   EXE_STR, "blame",
                   EXE_STR, path,
                   EXE_END);
  }

  int clone(const char *uri, const char *dir) const {
    return execute("git",
                   EXE_STR, "clone",
                   EXE_STR, uri,
                   EXE_IFSTR(dir, dir),
                   EXE_END);
  }

  int rename(int nsources, char **sources, const char *destination) const {
    // git mv allegedly supports multiple sources if the destination is a
    // directory but (at least in 1.6.4.2) this does not actually work.
    // Therefore we break the command up into multiple operations.
    for(int n = 0; n < nsources; ++n) {
      int rc =  execute("git",
                        EXE_STR, "mv",
                        EXE_STR, sources[n],
                        EXE_STR, destination,
                        EXE_END);
      if(rc)
        return rc;
    }
    return 0;
  }

  int show(const char *change) const {
    return execute("git",
                   EXE_STR, "show",
                   EXE_STR, change,
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
