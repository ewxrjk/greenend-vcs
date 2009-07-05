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
#include "p4utils.h"

static int p4_edit(int nfiles, char **files) {
  return execute("p4",
                 EXE_STR, "edit",
                 EXE_STRS, nfiles, p4_encode(nfiles, files),
                 EXE_END);
}

static int p4_diff(int nfiles, char **files) {
  if(nfiles)
    return execute("p4",
                   EXE_STR, "diff",
                   EXE_STR, "-du",
                   EXE_STRS, nfiles, p4_encode(nfiles, files),
                   EXE_END);
  else
    return execute("p4",
                   EXE_STR, "diff",
                   EXE_STR, "-du",
                   EXE_STR, "...",
                   EXE_END);
    
}

static int p4_add(int /*binary*/, int nfiles, char **files) {
  // 'p4 add' doesn't take encoded names - instead it encodes them for you.
  remove_directories(nfiles, files);
  if(!nfiles)
    return 0;
  return execute("p4",
                 EXE_STR, "add",
                 EXE_STR, "-f",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int p4_remove(int /*force*/, int nfiles, char **files) {
  return execute("p4",
                 EXE_STR, "delete",
                 EXE_STRS, nfiles, p4_encode(nfiles, files),
                 EXE_END);
}

static int p4_commit(const char *msg, int nfiles, char **files) {
  if(msg) {
    // If there's a message to include we must cobble together a suitable
    // change.  We do this by getting the default change and then editing it to
    // our taste.
    int rc;
    vector<string> change;
    vector<string>::size_type n, m;

    if((rc = capture(change, "p4", "change", "-o", (char *)NULL)))
      fatal("'p4 change -o' exited with status %d", rc);
    n = 0;
    // Find the default description 
    while(n < change.size() && change[n] != "Description:")
      ++n;
    // Erase it
    ++n;
    while(n < change.size()
          && change[n].size()
          && (change[n].at(0) == '\t'
              || change[n].at(0) == ' '))
      change.erase(change.begin() + n);
    // Insert the user's message instead
    change.insert(change.begin() + n, string("\t") + msg);
    ++n;
    // Find the Files: section
    while(n < change.size() && change[n] != "Files:")
      ++n;
    // Erase it
    ++n;
    while(n < change.size()
          && change[n].size()
          && (change[n].at(0) == '\t'
              || change[n].at(0) == ' '))
      change.erase(change.begin() + n);

    if(nfiles) {
      // Translate the list of files to submit
      vector<string> cmd;
      vector<string> where;
      cmd.push_back("p4");
      cmd.push_back("where");
      for(m = 0; m < (size_t)nfiles; ++m) {
        if(!exists(files[m]))
          fatal("%s does not exist", files[m]);
        cmd.push_back(p4_encode(files[m]));
      }
      if((rc = vcapture(where, cmd)))
        fatal("'p4 where PATHS' exited with status %d", rc);
      // Output is %-encoded, we keep it that way
      for(m = 0; m < where.size(); ++m) {
        change.insert(change.begin() + n,
                      "\t" + where[m].substr(0, where[m].find(' ')));
        ++n;
      }
    } else {
      vector<string> opened;
      if((rc = capture(opened, "p4", "opened", "...", (char *)NULL)))
        fatal("'p4 opened ...' exited with status %d", rc);
      // Drop blanks
      while(opened.size()
            and opened.back().size() == 0)
        opened.pop_back();
      // Anything to commit?
      if(!opened.size())
        fatal("no open files below current directory");
      // Construct the File: section
      // Again the output is %-encoded and we keep it that way
      for(m = 0; m < opened.size(); ++m) {
        change.insert(change.begin() + n,
                      "\t" + opened[m].substr(0, opened[m].find('#')));
        ++n;
      }
    }
    return inject(change, "p4", "submit", "-i", (char *)NULL);
  } else {
    return execute("p4",
                   EXE_STR, "submit",
                   EXE_STRS, nfiles, p4_encode(nfiles, files),
                   EXE_END);
  }
}

static int p4_revert(int nfiles, char **files) {
  if(nfiles)
    return execute("p4",
                   EXE_STR, "revert",
                   EXE_STRS, nfiles, p4_encode(nfiles, files),
                   EXE_END);
  else
    return execute("p4",
                   EXE_STR, "revert",
                   EXE_STR, "...",
                   EXE_END);
}

static int p4_status() {
  // Find out what P4 knows
  P4Info p4info;
  p4info.gather();

  // Get a list of all files, with relative path names
  list<string> files;
  set<string> ignored;
  listfiles("", files, ignored);
  
  // We'll accumulate the status info here
  typedef map<string,char,ltfilename> status_type;
  status_type status;

  // We'll accumulate a list of files that are in p4 but also ignored.
  list<string> known_ignored;

  // First from p4
  list<string> p4relpaths;
  p4info.relative_list(p4relpaths);
  for(list<string>::const_iterator it = p4relpaths.begin();
      it != p4relpaths.end();
      ++it) {
    P4FileInfo fi;
    p4info.relative_find(*it, fi);
    status[*it] = fi.action.size() ? toupper(fi.action[0]) : 0;
    if(ignored.find(*it) != ignored.end())
      // Stash ignored files known to P4 for a moan later on
      known_ignored.push_back(*it);
  }
  
  // Now from the file list
  for(list<string>::const_iterator it = files.begin();
      it != files.end();
      ++it) {
    // Completely skip ignored files
    if(ignored.find(*it) != ignored.end())
      continue;
    // Skip files that p4 knows about
    if(status.find(*it) != status.end())
      continue;
    // The rest are unknown
    status[*it] = '?';
  }

  // So what should dry-run mode do here?  At the moment we carry on
  // regardless; since we don't modify anything this harmless.

  // Now print out the results
  for(status_type::const_iterator it = status.begin();
      it != status.end();
      ++it) {
    if(it->second) {
      if(printf("%c %s\n", it->second, it->first.c_str()) < 0)
        fatal("error writing to stdout: %s", strerror(errno));
    }
  }

  // Ensure warnings come right after the output so they are not swamped
  if(fflush(stdout) < 0)
    fatal("error writing to stdout: %s", strerror(errno));
  if(known_ignored.size()) {
    fprintf(stderr, "WARNING: the following files are known to p4 but also ignored\n");
    for(list<string>::const_iterator it = known_ignored.begin();
        it != known_ignored.end();
        ++it)
      fprintf(stderr, "%s\n", it->c_str());
  }
  return 0;
}

static int p4_update() {
  return execute("p4",
                 EXE_STR, "sync",
                 EXE_STR, "...",
                 EXE_END);
}

static int p4_log(const char *path) {
  return execute("p4",
                 EXE_STR, "changes",
                 EXE_STR, "-lt",
                 EXE_STR, path ? path : "...",
                 EXE_END);
}

static int p4_annotate(const char *path) {
  return execute("p4",
                 EXE_STR, "annotate",
                 EXE_STR, "-c",
                 EXE_STR, p4_encode(path).c_str(),
                 EXE_END);
}

const struct vcs vcs_p4 = {
  "Perforce",
  p4_diff,
  p4_add,
  p4_remove,
  p4_commit,
  p4_revert,
  p4_status,
  p4_update,
  p4_log,
  p4_edit,
  p4_annotate,
  NULL,                                 // clone
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
