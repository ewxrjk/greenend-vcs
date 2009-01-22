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

static int cvs_diff(int nfiles, char **files) {
  return execute("cvs",
                 EXE_STR, "diff",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int cvs_add(int binary, int nfiles, char **files) {
  return execute("cvs",
                 EXE_STR, "add",
                 EXE_IFSTR(binary, "-kb"),
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int cvs_remove(int force, int nfiles, char **files) {
  return execute("cvs",
                 EXE_STR, "remove",
                 EXE_IFSTR(force, "-f"),
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int cvs_commit(const char *msg, int nfiles, char **files) {
  return execute("cvs",
                 EXE_STR, "commit",
                 EXE_IFSTR(msg, "-m"),
                 EXE_IFSTR(msg, msg),
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static void limit_set(set<string> &s, const set<string> &limit) {
  set<string>::iterator it = s.begin();
  while(it != s.end()) {
    set<string>::iterator here = it;
    ++it;
    if(limit.find(*here) == limit.end())
      s.erase(here);
  }
}

static int cvs_revert(int nfiles, char **files) {
  // Reverting is a bit ugly in CVS.  We can 'cvs up -C' files that have just
  // been edited.  For added files we must use 'cvs rm -f'.  For deleted files
  // we must 'cvs add' them again.  Fortunately the translation (-kb) appears
  // to survive an rm/add pair.  At any rate we can't just do a single easy
  // command.  Complicating the matter are files with conflicts, which need
  // removing before we call 'cvs up' since even 'cvs up -C' is not puisant
  // enough to revert them.


  // Establish the current state
  vector<string> status;
  set<string> modified, added, removed, conflicted;
  int rc;
  if((rc = capture(status, "cvs", "-n", "up", (char *)0)))
    fatal("cvs -n up exited with status %d", rc);
  for(size_t n = 0; n < status.size(); ++n) {
    switch(status[n][0]) {
    case 'M':
      modified.insert(status[n].substr(2)); 
      break;
    case 'A':
      added.insert(status[n].substr(2));
      break;
    case 'R':
      removed.insert(status[n].substr(2));
      break;
    case 'C':
      conflicted.insert(status[n].substr(2));
      modified.insert(status[n].substr(2));
      break;
    }
  }
  // If files were specified limit to just those
  if(nfiles) {
    set<string> limit;

    while(nfiles--)
      limit.insert(*files++);
    limit_set(modified, limit);
    limit_set(added, limit);
    limit_set(removed, limit);
  }
  // Currently we issue single commands for everything.  (And note that we
  // invoke rm, rather than deleting directly, as a convenient way of including
  // that step in the dry-run/verbose rules.)  This is simpler to implement but
  // also guarantees that we don't run afoul of command-line length limits.
  //
  // Revert modified and conflicted files
  for(set<string>::iterator it = modified.begin();
      it != modified.end();
      ++it) {
    if(conflicted.find(*it) != conflicted.end())
      if(execute("rm",
                 EXE_STR, "-f",
                 EXE_STR, it->c_str(),
                 EXE_END))
        return 1;
    if(execute("cvs",
               EXE_STR, "up",
               EXE_STR, "-C",
               EXE_STR, it->c_str(),
               EXE_END))
      return 1;
  }
  // Re-add removed files
  for(set<string>::iterator it = removed.begin();
      it != removed.end();
      ++it)
    if(execute("cvs",
               EXE_STR, "add",
               EXE_STR, it->c_str(),
               EXE_END))
      return 1;
  // Remove added files
  for(set<string>::iterator it = added.begin();
      it != added.end();
      ++it)
    if(execute("cvs",
               EXE_STR, "rm",
               EXE_STR, "-f",
               EXE_STR, it->c_str(),
               EXE_END))
      return 1;
  return 0;
}

static int cvs_status() {
  return execute("cvs",
                 EXE_STR, "-n",
                 EXE_STR, "up",
                 EXE_END);
}

const struct vcs vcs_cvs = {
  "CVS",
  cvs_diff,
  cvs_add,
  cvs_remove,
  cvs_commit,
  cvs_revert,
  cvs_status,
};

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
