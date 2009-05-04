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

extern "C" {
  extern char **environ;
}

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
  vector<string> have;
  vector<string> opened, errors, command;
  map<string,string> known;
  list<string> files, deleted;
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
  if((rc = execute(makevs(command, "p4", "opened", "...", (char *)0),
                   NULL/*input*/,
                   &opened,
                   &errors))) {
    report_lines(errors);
    fatal("'p4 opened ...' exited with status %d", rc);
  }
  if(!(errors.size() == 0
       || (errors.size() == 1
           && errors[0] == "... - file(s) not opened on this client."))) {
    report_lines(errors);
    fatal("Unexpected error output from 'p4 opened ...'");
  }

  // Generate a map from depot path names to what p4 knows about the files
  for(size_t n = 0; n < have.size(); ++n) {
    const string depot_path = p4_decode(have[n].substr(0, have[n].find('#')));
    known[depot_path] = "";
  }
  for(size_t n = 0; n < opened.size(); ++n) {
    if(opened[n].size()) {
      P4Opened o(opened[n]);            // does its own %-decoding
      known[o.path] = o.action;
      if(o.action == "delete")
        deleted.push_back(o.path);
    }
  }

  // All files, with relative path names
  listfiles("", files, ignored);

  // Find the base local path name
  // In theory this is the current working directory, but we want to be
  // sure we have p4's idea of it.
  vector<string> wd;
  list<string> ls;
  ls.push_back("...");
  p4__where(wd, ls);
  const string base(P4Where(wd[0]).local_path,
                    0, P4Where(wd[0]).local_path.size() - 3);

  // Map deleted depot paths to relative filenames and add to file list
  if(deleted.size()) {
    p4__where(wd, deleted);
    for(size_t n = 0; n < wd.size(); ++n) {
      P4Where w(wd[n]);
      if(debug)
        fprintf(stderr, "depot path: %s\n"
                "local path: %s\n"
                "truncated:  %s\n",
                w.depot_path.c_str(),
                w.local_path.c_str(),
                w.local_path.substr(base.size()).c_str());
      files.push_back(w.local_path.substr(base.size()));
    }
  }
  
  // Use 'p4 where' to map relative filenames to absolute ones
  map<string,P4Where> depot, view, local;
  p4__where(files, depot, view, local);

  if(dryrun)
    return 0;

  // We'll accumulate a list of files that are in p4 but also ignored.
  list<string> known_ignored;

  // Now we can go through the file list in order and say what we know about
  // each one
  list<string>::const_iterator it = files.begin();
  size_t n = 0;
  while(it != files.end()) {
    // Compute (p4's idea of) the absolute local path
    const string local_path = base + *it;
    // Find the 'p4 where' info, if there is any
    const P4Where *w = local.find(local_path) != local.end()
      ? &local[local_path] : NULL;
    // Find what p4 knows about the file
    const map<string,string>::const_iterator k
      = w ? known.find(w->depot_path) : known.end();
    // This will be the file status.
    string status;
    
    fprintf(stderr,
            "file %s\n"
            "  local_path %s\n", it->c_str(), local_path.c_str());
    if(w)
      fprintf(stderr, "  w %s | %s | %s\n",
              w->depot_path.c_str(), w->view_path.c_str(), w->local_path.c_str());
    else
      fprintf(stderr, "  w NULL\n");
    if(k != known.end())
      fprintf(stderr, "  k %s -> %s\n",
              k->first.c_str(), k->second.c_str());
    else
      fprintf(stderr, "  k end()\n");

    if(k != known.end()) {
      // Perforce knows something about this file
      status = k->second;
      if(ignored.find(*it) != ignored.end())
        // ...but it's ignored!
        known_ignored.push_back(*it);
    } else {
      // Perforce knows nothing about this file
      if(ignored.find(*it) == ignored.end())
        // ...and it's not ignored either
        status = "?";
    }
    if(status.size()) {
      if(printf("%c %s\n",
                toupper(status[0]), it->c_str()) < 0)
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
