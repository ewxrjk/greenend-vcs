/*
 * This file is part of VCS
 * Copyright (C) 2009-2011 Richard Kettlewell
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

class p4: public vcs {
public:

  p4(): vcs("Perforce") {
  }

  bool detect(void) const {
    // Only attempt to detect Perforce if some Perforce-specific environment
    // variable is set.
    //
    // detect() is rung _before_ searching parent directories for a VC-specific
    // subdirectory, so that Perforce checkouts that are below (e.g.) Bazaar
    // checkouts are correctly matched.  Our own test scripts are the
    // motivating (and perhaps only) example.
    if(getenv("P4PORT") || getenv("P4CONFIG") || getenv("P4CLIENT")) {
      if(!execute("p4",
                  EXE_STR, "changes",
                  EXE_STR, "-m1",
                  EXE_STR, "...",
                  EXE_NO_STDOUT,
                  EXE_NO_STDERR,
                  EXE_END))
        return true;
    }
    return false;
  }

  int edit(int nfiles, char **files) const {
    return execute("p4",
                   EXE_STR, "edit",
                   EXE_STRS, nfiles, p4_encode(nfiles, files),
                   EXE_END);
  }

  int diff(int nfiles, char **files) const {
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

  int add(int /*binary*/, int nfiles, char **files) const {
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

  int remove(int /*force*/, int nfiles, char **files) const {
    return execute("p4",
                   EXE_STR, "delete",
                   EXE_STRS, nfiles, p4_encode(nfiles, files),
                   EXE_END);
  }

  int commit(const char *msg, int nfiles, char **files) const {
    // We have an optional message and an optional list of files, giving
    // four possibilities.  If the message isn't present then we need to
    // bring up a change form for the user to fill in.
    //
    // 1) No message, no files.
    //    We can just "p4 submit ..." and Perforce will get the user to edit
    //    the change form.
    // 2) Message and no files.
    //    We can "p4 submit -d MESSAGE ..." and avoid any further interaction.
    // 3) No message, list of files.
    //    We synthesize a change form and let the user edit it.  We then
    //    invoke "p4 submit -i".
    // 4) Message, list of files.
    //    We synthesize a change form and immediately submit it to
    //    "p4 submit -i".

    // The easy case is when there are no files listed.
    if(!nfiles) {
      if(msg)
        return execute("p4",
                       EXE_STR, "submit",
                       EXE_STR, "-d",
                       EXE_STR, msg,
                       EXE_STR, "...",
                       EXE_END);
      else
        return execute("p4",
                       EXE_STR, "submit",
                       EXE_STR, "...",
                       EXE_END);
    }

    // Some files were listed.  There might or might not be a message.  Either
    // way we need to construct a change form.

    int rc;
    vector<string> change;
    vector<string>::size_type n, m;

    // Get the default change form
    if((rc = capture(change, "p4", "change", "-o", (char *)NULL)))
      fatal("'p4 change -o' exited with status %d", rc);
    n = 0;
    // Find the default description 
    while(n < change.size() && change[n] != "Description:")
      ++n;
    if(msg) {
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
      // ..pointing just after the message text
    } else {
      // Step over it
      ++n;
      while(n < change.size()
            && change[n].size()
            && (change[n].at(0) == '\t'
                || change[n].at(0) == ' '))
        ++n;
      // ..pointing just after the default message text
    }
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
    // Now we have a change.  If a message was specified we submit straight away
    // and that's all we need.
    if(msg)
      return inject(change, "p4", "submit", "-i", (char *)NULL);
    if(dryrun) {
      // Popping up a form to fill in in dry-run mode makes no sense, so we just
      // show them what the un-edited form would look like.
      return inject(change, "p4", "submit", "-i", (char *)NULL);
    }
    // Give the user a chance to edit the change form
    rc = editor(change);
    if(rc)
      return rc;
    // Submit the edited change
    return inject(change, "p4", "submit", "-i", (char *)NULL);
  }

  int revert(int nfiles, char **files) const {
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

  int status() const {
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
      if(!fi.changed)
        status[*it] = tolower(status[*it]);
      if(fi.resolvable) {
        fprintf(stderr, "resolvable: %s\n", it->c_str());
        status[*it] = 'R';
      }
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

  int update() const {
    return execute("p4",
                   EXE_STR, "sync",
                   EXE_STR, "...",
                   EXE_END);
  }

  int log(const char *path) const {
    return execute("p4",
                   EXE_STR, "changes",
                   EXE_STR, "-lt",
                   EXE_STR, path ? path : "...",
                   EXE_END);
  }

  int annotate(const char *path) const {
    return execute("p4",
                   EXE_STR, "annotate",
                   EXE_STR, "-c",
                   EXE_STR, p4_encode(path).c_str(),
                   EXE_END);
  }

  void rename_one(const string &source, const string &destination) const {
    string sp, dp;
    if(isdir(source)) {
      sp = source + "/...";
      dp = destination + "/...";
    } else {
      sp = source;
      dp = destination;
    }
    if(execute("p4",
               EXE_STR, "integrate",
               EXE_STR, "-t",
               EXE_STR, p4_encode(sp).c_str(),
               EXE_STR, p4_encode(dp).c_str(),
               EXE_END))
      exit(1);
    if(execute("p4",
               EXE_STR, "delete",
               EXE_STR, p4_encode(sp).c_str(),
               EXE_END))
      exit(1);
  }

  int show(const char *change) const {
    return execute("p4",
                   EXE_STR, "describe",
                   EXE_STR, "-du",
                   EXE_STR, change,
                   EXE_END);
  }
};

static const p4 vcs_p4;

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
