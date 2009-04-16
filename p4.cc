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

extern "C" {
  extern char **environ;
}

// Parsed 'p4 opened' output
struct Opened {
  string path;                          // depot path
  int rev;                              // revision number
  string action;                        // action
  int chnum;                            // change number, -1 for default
  string type;                          // file type
  int locked;                           // true for *locked*

  Opened(): rev(0),
            chnum(-1),
            locked(0) {
  }

  Opened(const string &l) { 
    parse(l);
  }

  // Zap everything
  void clear() {
    path.clear();
    rev = 0;
    action.clear();
    chnum = -1;
    type.clear();
    locked = 0;
  }

  // Cheesy parser
  void parse(const string &l) {
    clear();
    // Get the depot path
    string::size_type n = l.find('#');
    if(n == string::npos)
      fatal("no '#' found in 'p4 opened' output: %s",
            l.c_str());
    path.assign(l, 0, n);
    // The revision
    string revs;
    ++n;
    while(isdigit(l.at(n)))
      revs += l[n++];
    rev = atoi(revs.c_str());
    // The action
    while(l.at(n) == ' ' || l.at(n) == '-')
      ++n;
    while(l.at(n) != ' ')
      action += l[n++];
    // The change
    string chstr;
    while(l.at(n) == ' ')
      ++n;
    while(l.at(n) != ' ')
      chstr += l[n++];
    if(chstr == "default")
      chnum = -1;
    else
      chnum = atoi(chstr.c_str());
    // The type
    while(l.at(n) == ' ' || l.at(n) == '(')
      ++n;
    while(l.at(n) != ')')
      type += l[n++];
    // Lock status
    while(n < l.size() && (l.at(n) == ' ' || l.at(n) == '('))
      ++n;
    if(n < l.size() && l[n] == '*')
      locked = 1;
  }
};

// Compute the number of bytes required for the environment
static size_t env_size() {
  char **e = environ;
  size_t size = 1;
  while(*e)
    size += strlen(*e++) + 1;
  return size;
}

// Run 'p4 where' on all the listed files, breaking up into multiple
// invocations to avoid command-line length limits
static void p4__where(vector<string> &where, const list<string> &files) {
  where.clear();

  // Figure out how much space we can safely use for the command line
  size_t limit = sysconf(_SC_ARG_MAX) - 2048;
  const size_t e = env_size();
  if(e >= limit)
    fatal("no space for commands - e=%lu, limit=%lu",
          (unsigned long)e, (unsigned long)limit);
  limit -= e;
  // ARG_MAX is the system limit.  Should be at least 4096.  2048 is clearance
  // for the subprocess to modify its own environment and a few bytes for the
  // 'p4 where'.  We subtract the size of the current environment too.
  //
  // http://www.in-ulm.de/~mascheck/various/argmax/
  
  list<string>::const_iterator it = files.begin();
  while(it != files.end()) {
    // Find the next few kilobytes worth of filenames
    list<string>::const_iterator e = it;
    vector<string> cmd;
    cmd.push_back("p4");
    cmd.push_back("where");
    size_t total = 0;
    while(e != files.end()) {
      size_t here = e->size() + 1;

      if(total + here > limit)
        break;
      cmd.push_back(*e);
      total += here;
      ++e;
    }
    if(e == it)
      fatal("filename too long: %s", it->c_str());
    vector<string> someresults;
    int rc;
    if((rc = vcapture(someresults, cmd)))
      fatal("'p4 where PATHS' exited with status %d", rc);
    while(someresults.size()
          && someresults.back().size() == 0)
      someresults.pop_back();
    where.insert(where.end(), someresults.begin(), someresults.end());
    it = e;
  }
}

static int p4_edit(int nfiles, char **files) {
  return execute("p4",
                 EXE_STR, "edit",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int p4_diff(int nfiles, char **files) {
  if(nfiles)
    return execute("p4",
                   EXE_STR, "diff",
                   EXE_STR, "-du",
                   EXE_STRS, nfiles, files,
                   EXE_END);
  else
    return execute("p4",
                   EXE_STR, "diff",
                   EXE_STR, "-du",
                   EXE_STR, "...",
                   EXE_END);
    
}

static int p4_add(int /*binary*/, int nfiles, char **files) {
  return execute("p4",
                 EXE_STR, "add",
                 EXE_STRS, nfiles, files,
                 EXE_END);
}

static int p4_remove(int /*force*/, int nfiles, char **files) {
  return execute("p4",
                 EXE_STR, "delete",
                 EXE_STRS, nfiles, files,
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
        cmd.push_back(files[m]);
      }
      if((rc = vcapture(where, cmd)))
        fatal("'p4 where PATHS' exited with status %d", rc);
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
                   EXE_STRS, nfiles, files,
                   EXE_END);
  }
}

static int p4_revert(int nfiles, char **files) {
  if(nfiles)
    return execute("p4",
                   EXE_STR, "revert",
                   EXE_STRS, nfiles, files,
                   EXE_END);
  else
    return execute("p4",
                   EXE_STR, "revert",
                   EXE_STR, "...",
                   EXE_END);
}

static int p4_status() {
  vector<string> have;
  vector<string> opened;
  map<string,string> known;
  list<string> files;
  set<string> ignored;
  int rc;

  // All files, in the form:
  //   DEPOT-PATH#REV - LOCAL-PATH
  // NB paths are absolute!
  if((rc = capture(have, "p4", "have", "...", (char *)NULL)))
    fatal("'p4 have ...' exited with status %d", rc);

  // Opened files, in the form:
  //   DEPOT-PATH#REV - ACTION CHNUM change (TYPE) [...]
  // ACTION is add, edit, delete, branch, integrate
  // CHNUM is the change number or 'default'.
  if((rc = capture(opened, "p4", "opened", "...", (char *)0)))
    fatal("'p4 opened ...' exited with status %d", rc);

  // Generate a map from depot path names to what p4 knows about the files
  for(size_t n = 0; n < have.size(); ++n) {
    const string depot_path(have[n], 0, have[n].find('#'));
    known[depot_path] = "";
  }
  for(size_t n = 0; n < opened.size(); ++n) {
    if(opened[n].size()) {
      Opened o(opened[n]);
      known[o.path] = o.action;
    }
  }

  // All files, with relative path names
  listfiles("", files, ignored);
  
  // Use 'p4 where' to map relative filenames to absolute ones
  vector<string> where;
  p4__where(where, files);

  if(dryrun)
    return 0;

  // We'll accumulate a list of files that are in p4 but also ignored.
  list<string> known_ignored;

  // Now we can go through the file list in order and say what we know about
  // each one
  list<string>::const_iterator it = files.begin();
  size_t n = 0;
  while(it != files.end()) {
    const string depot_path(where[n], 0, where[n].find(' '));
    const string local_path(*it);
    const map<string,string>::const_iterator k = known.find(depot_path);
    string status;
    
    if(k != known.end()) {
      // Perforce knows something about this file
      status = k->second;
      if(ignored.find(local_path) != ignored.end())
        // ...but it's ignored!
        known_ignored.push_back(local_path);
    } else {
      // Perforce knows nothing about this file
      if(ignored.find(local_path) == ignored.end())
        // ...and it's not ignored either
        status = "?";
    }
    if(status.size()) {
      if(printf("%c %s\n",
                toupper(status[0]), local_path.c_str()) < 0)
        fatal("error writing to stdout: %s", strerror(errno));
    }

    ++n;
    ++it;
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
                 EXE_IFSTR(path, path),
                 EXE_END);
}

static int p4_annotate(const char *path) {
  return execute("p4",
                 EXE_STR, "annotate",
                 EXE_STR, "-c",
                 EXE_STR, path,
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
