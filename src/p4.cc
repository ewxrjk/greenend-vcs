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
#include "p4utils.h"
#include <sstream>

class p4: public vcs {
public:
  p4(): vcs("Perforce") {
  }

  bool detect(void) const {
    // Only attempt to detect Perforce if some Perforce-specific environment
    // variable is set.
    //
    // detect() is run _before_ searching parent directories for a VC-specific
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

  int edit(const vector<string> &files) const {
    vector<string> encoded = p4_encode(files);
    return execute("p4",
                   EXE_STR, "edit",
                   EXE_VECTOR|EXE_DOTSTUFF, &encoded,
                   EXE_END);
  }

  int diff(const vector<string> &files) const {
    if(files.size()) {
      for(size_t n = 0; n < files.size(); ++n)
        diff_one(files[n]);
    } else {
      diff_all();
    }
    return 0;
  }

  int add(int /*binary*/, const vector<string> &files) const {
    // 'p4 add' doesn't take encoded names - instead it encodes them for you.
    vector<string> nondirs = remove_directories(files);
    if(!nondirs.size())
      return 0;
    return execute("p4",
                   EXE_STR, "add",
                   EXE_STR, "-f",
                   EXE_VECTOR|EXE_DOTSTUFF, &nondirs,
                   EXE_END);
  }

  int remove(int /*force*/, const vector<string> &files) const {
    vector<string> encoded = p4_encode(files);
    return execute("p4",
                   EXE_STR, "delete",
                   EXE_VECTOR|EXE_DOTSTUFF, &encoded,
                   EXE_END);
  }

  int commit(const string *msg, const vector<string> &files) const {
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
    if(!files.size()) {
      if(msg)
        return execute("p4",
                       EXE_STR, "submit",
                       EXE_STR, "-d",
                       EXE_STRING, msg,
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
      change.insert(change.begin() + n, "\t" + *msg);
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
    for(m = 0; m < files.size(); ++m) {
      if(!exists(files[m]))
        fatal("%s does not exist", files[m].c_str());
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

  int revert(const vector<string> &files) const {
    if(files.size()) {
      vector<string> encoded = p4_encode(files);
      return execute("p4",
                     EXE_STR, "revert",
                     EXE_VECTOR|EXE_DOTSTUFF, &encoded,
                     EXE_END);
    } else
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
      if(it->second)
        writef(stdout, "stdout", "%c %s\n", it->second, it->first.c_str());
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

  int log(const string *path) const {
    return execute("p4",
                   EXE_STR, "changes",
                   EXE_STR, "-lt",
                   EXE_STR|EXE_DOTSTUFF, path ? path->c_str() : "...",
                   EXE_END);
  }

  int annotate(const string &path) const {
    return execute("p4",
                   EXE_STR, "annotate",
                   EXE_STR, "-c",
                   EXE_STRING|EXE_DOTSTUFF|EXE_P4, &path,
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
               EXE_STRING|EXE_DOTSTUFF|EXE_P4, &sp,
               EXE_STRING|EXE_DOTSTUFF|EXE_P4, &dp,
               EXE_END))
      exit(1);
    if(execute("p4",
               EXE_STR, "delete",
               EXE_STRING|EXE_DOTSTUFF|EXE_P4, &sp,
               EXE_END))
      exit(1);
  }

  int show(const string &change) const {
    string type;
    vector<string> lines;
    static const char diffs[] = "Differences ...";
    P4Describe description(change.c_str());
    for(size_t n = 0;
        (n < description.lines.size()
         && description.lines[n] != diffs);
        ++n)
      writef(stdout, "stdout", "%s\n", description.lines[n].c_str());
    writef(stdout, "stdout", "%s\n", diffs);
    for(size_t n = 0; n < description.files.size(); ++n) {
      writef(stdout, "stdout", "\n");
      const P4Describe::fileinfo &file = description.files[n];
      if(file.line) {
        // p4 has produce the diff for us
        for(size_t m = file.line;
            m < description.lines.size()
              && (description.lines[m].size() || m == file.line + 1);
            ++m) {
          writef(stdout, "stdout", "%s\n", description.lines[m].c_str());
        }
      } else if(file.action == "add" || file.action == "branch") {
        // File was added in this revision
        retrieve_file(file.depot_path, lines, &type, file.rev);
        writef(stdout, "stdout", "==== %s#%d (%s) ====\n\n",
               file.depot_path.c_str(), file.rev, type.c_str());
        if(type == "binary")
          writef(stdout, "stdout", "%s is a binary file\n",
                 file.depot_path.c_str());
        else
          diff_whole(lines, '+');
      } else if(file.action == "delete") {
        // File was removed in this revision
        retrieve_file(file.depot_path, lines, &type, file.rev - 1);
        writef(stdout, "stdout", "==== %s#%d (%s) ====\n\n",
               file.depot_path.c_str(), file.rev, type.c_str());
        if(type == "binary")
          writef(stdout, "stdout", "%s was a binary file\n",
                 file.depot_path.c_str());
        else
          diff_whole(lines, '-');
      }
      if(fflush(stdout) < 0)
        fatal("writing to stdout: %s\n", strerror(errno));
    }
    return 0;
  }

private:
  void diff_all() const {
    P4Info info;
    list<string> files;
    info.gather();
    info.depot_list(files);
    for(list<string>::const_iterator it = files.begin();
        it != files.end();
        ++it) {
      const string &path = *it;
      P4FileInfo fi;
      info.depot_find(path, fi);
      diff_one(fi);
    }
  }

  void diff_one(const string &path) const {
    P4Info info;
    list<string> files;
    info.gather();
    P4FileInfo fi;
    // Any kind of path is accepable
    if(info.local_find(path, fi))
      diff_one(fi);
    else if(info.relative_find(path, fi))
      diff_one(fi);
    else if(info.depot_find(path, fi))
      diff_one(fi);
    else
      return;                           // no change, presumably
  }

  void diff_one(const P4FileInfo &info) const {
    if(info.action == "edit" || info.action == "integrate") {
      // We can diff the file directly
      execute("p4",
              EXE_STR, "diff",
              EXE_STR, "-du",
              EXE_STRING|EXE_DOTSTUFF, &info.depot_path,
              EXE_END);
    } else if(info.action == "branch" || info.action == "add") {
      diff_new(info);
    } else if(info.action == "delete") {
      diff_deleted(info);
    }
    // Flush after every file (we're likely to be about to run a subprocess
    // anyway)
    if(fflush(stdout) < 0)
      fatal("writing to stdout: %s\n", strerror(errno));
  }

  void diff_whole(const char *path, char prefix) const {
    FILE *fp = fopen(path, "r");
    if(!fp)
      fatal("error opening %s: %s", path, strerror(errno));
    int c;
    long lines = 0;
    while((c = getc(fp)) != EOF)
      if(c == '\n')
        ++lines;
    writef(stdout, "stdout", prefix == '+' ? "@@ -0,0 +1,%ld @@\n"
                                           : "@@ -1,%ld +0,0 @@\n",
           lines);
    rewind(fp);
    bool start = true;
    while((c = getc(fp)) != EOF) {
      if(start) {
        writef(stdout, "stdout", "%c", prefix);
        start = false;
      }
      writef(stdout, "stdout", "%c", c);
      if(c == '\n')
        start = true;
    }
    fclose(fp);
    if(!start)
      writef(stdout, "stdout", "\n\\ No newline at end of file\n");
  }

  void diff_whole(const vector<string> &lines, char prefix) const {
    writef(stdout, "stdout", prefix == '+' ? "@@ -0,0 +1,%zu @@\n"
                                           : "@@ -1,%ld +0,0 @@\n",
           lines.size());
    for(size_t n = 0; n < lines.size(); ++n)
      writef(stdout, "stdout", "%c%s", prefix, lines[n].c_str());
    if(lines.size() && lines.back().find('\n') == string::npos)
      writef(stdout, "stdout", "\n\\ No newline at end of file\n");
  }

  void diff_new(const P4FileInfo &info) const {
    // New file, display the whole text with a suitable header.  We need to
    // count how many lines are in the new file.  The header we write is
    // consistent with p4 rather than with GNU diff.
    writef(stdout, "stdout", "==== - %s ====\n", info.local_path.c_str());
    if(info.type == "binary")
      writef(stdout, "stdout", "%s is a binary file\n", info.local_path.c_str());
    else
      diff_whole(info.local_path.c_str(), '+');
  }

  void diff_deleted(const P4FileInfo &info) const {
    // Deleted file, retrieve the old text with p4 print and print it with a
    // suitable header.  As with diff_new() we write a p4-like header.
    vector<string> lines;
    string type;
    retrieve_file(info.depot_path, lines, &type);
    writef(stdout, "stdout", "==== %s - ====\n", info.depot_path.c_str());
    if(type == "binary")
      writef(stdout, "stdout", "%s was a binary file\n", info.depot_path.c_str());
    else
      diff_whole(lines, '-');
  }

  void retrieve_file(const string &path,
                     vector<string> &contents,
                     string *type = NULL,
                     int rev = -1) const {
    vector<string> lines;
    ostringstream s;
    if(path.size() && path.at(0) == '-')
      s << "./";
    s << path;
    if(rev != -1)
      s << '#' << rev;
    int rc;
    vector<string> command;
    if((rc = execute(makevs(command,
                            "p4", "print", s.str().c_str(),
                            (char *)NULL),
                     NULL,              // input
                     &lines,            // output
                     NULL,              // errors
                     NULL,              // outputPath
                     EXE_RAW)))         // flags
      fatal("p4 print failed with status %d", rc);
    contents.assign(lines.begin() + 1, lines.end());
    if(type) {
      // syntax is:
      //    //path/to/file#rev - ACTION change CHANGE (TYPE)
      const string &l = lines[0];
      string::size_type openb = l.rfind('(');
      string::size_type closeb = l.rfind(')');
      if(openb != string::npos && closeb != string::npos)
        type->assign(l, openb + 1, closeb - (openb + 1));
      else
        type->clear();
    }
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
