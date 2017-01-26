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
#include "xml.h"
#include <algorithm>

class svn: public vcs {
public:

  svn(): vcs("Subversion") {
    register_subdir(".svn");
    register_scheme("svn");
    register_scheme("svn+ssh");
  }

  int diff(const vector<string> &files) const {
    return execute("svn",
                   EXE_STR, "diff",
                   EXE_STR, "--",
                   EXE_VECTOR|EXE_SVN, &files,
                   EXE_END);
  }

  int add(int /*binary*/, const vector<string> &files) const {
    return execute("svn",
                   EXE_STR, "add",
                   EXE_STR, "--",
                   EXE_VECTOR|EXE_SVN, &files,
                   EXE_END);
  }

  int remove(int force, const vector<string> &files) const {
    return execute("svn",
                   EXE_STR, "delete",
                   EXE_IFSTR(force, "--force"),
                   EXE_STR, "--",
                   EXE_VECTOR|EXE_SVN, &files,
                   EXE_END);
  }

  int commit(const string *msg, const vector<string> &files) const {
    return execute("svn",
                   EXE_STR, "commit",
                   EXE_IFSTR(msg, "-m"),
                   EXE_STRING|EXE_OPT, msg,
                   EXE_STR, "--",
                   EXE_VECTOR|EXE_SVN, &files,
                   EXE_END);
  }

  int revert(const vector<string> &files) const {
    // Subversion's revert insist you tell it what files to revert.  So if
    // we want to revert everything we must cobble together a list.
    if(!files.size()) {
      // Establish the current state
      vector<string> status;
      int rc;
      // svn interactive output changes between versions.  Fortunately most
      // vaguely recent versions can produce XML output which hopefuly will be
      // more stable or at least more self-describing in its instability
      //
      // It is not however well documented.
      //
      // The general form seems to be:
      //    <status>
      //      <target path="."> // ...might not be '.'
      //        <entry path="FILENAME">
      //          <wc-status props="none"
      //                     item="unversioned|added|modified|deleted
      //                          |conflicted">
      //            <commit revision="INTEGER">
      //              <author>WHO</author>
      //              <date>WHEM</date>
      //            </commit>  // ...optional
      //          </wc-status>
      //        </entry>  // ...many times
      //      </target> // ...only once?
      //    </status>
      //
      // Possible item types include:
      //    none               not modified
      //    normal             not modified
      //    added              should revert the add
      //    missing            should restore
      //    incomplete         should restore
      //    deleted            should revert the delete
      //    replaced
      //    modified
      //    merged
      //    conflicted
      //    obstructed
      //    ignored
      //    external
      //    unversioned
      //
      // See subversion/svn/status.c for the implementation.
      //
      if((rc = capture(status, "svn", "status", "--xml", (char *)0)))
        fatal("svn status exited with status %d", rc);
      vector<string> files;
      const XMLNode *root = xmlparse(status, false);
      const XMLElement *status_ = dynamic_cast<const XMLElement *>(root);
      const XMLElement *target = dynamic_cast<const XMLElement *>(status_->contents[0]);
      for(size_t n = 0; n < target->contents.size(); ++n) {
        XMLElement *entry = dynamic_cast<XMLElement *>(target->contents[n]);
        XMLElement *wcstatus = dynamic_cast<XMLElement *>(entry->contents[0]);
        const string &item = wcstatus->attributes["item"];
        if(item == "added"
           || item == "missing"
           || item == "incomplete"
           || item == "deleted"
           || item == "replaced"
           || item == "modified"
           || item == "merged"
           || item == "conflicted"
           || item == "obstructed")
          files.push_back(entry->attributes["path"]);
      }
      delete root;
      if(!files.size())
        return 0;
      return execute("svn",
                     EXE_STR, "revert",
                     EXE_STR, "--",
                     EXE_VECTOR|EXE_SVN, &files,
                     EXE_END);
    } else
      return execute("svn",
                     EXE_STR, "revert",
                     EXE_STR, "--",
                     EXE_VECTOR|EXE_SVN, &files,
                     EXE_END);
  }

  static bool status_compare(const string &a, const string &b) {
    if(a.size() > 8
       && a.compare(1, 7, "       ") == 0
       && b.size() > 8
       && b.compare(1, 7, "       ") == 0)
      return a.compare(8, string::npos,
                       b, 8, string::npos) < 0;
    else
      return false;
  }

  int status() const {
    // svn status output has unpredictable order in some versions, so sort it
    vector<string> status;
    int rc;
    if((rc = capture(status, "svn", "status", (char *)0)))
      fatal("svn status exited with status %d", rc);
    stable_sort(status.begin(), status.end(),
                status_compare);
    for(size_t n = 0; n < status.size(); ++n)
      writef(stdout, "stdout", "%s\n", status[n].c_str());
    return 0;
  }

  int update() const {
    return execute("svn",
                   EXE_STR, "update",
                   EXE_END);
  }

  int log(const string *path) const {
    return execute("svn",
                   EXE_STR, "log",
                   EXE_STR, "--",
                   EXE_STRING|EXE_OPT, path,
                   EXE_END);
  }

  int annotate(const string &path) const {
    return execute("svn",
                   EXE_STR, "blame",
                   EXE_STR, "--",
                   EXE_STRING, &path,
                   EXE_END);
  }

  int clone(const string &uri, const string *dir) const {
    return execute("svn",
                   EXE_STR, "checkout",
                   EXE_STR, "--",
                   EXE_STRING, &uri,
                   EXE_STRING|EXE_OPT, dir,
                   EXE_END);
  }

  // At least up to svn 1.4.6, mv can only take a single source and
  // destination.  At some point support for old versions will be dropped but
  // currently I consider that too recent to ignore.
  void rename_one(const string &source, const string &destination) const {
    string sp, dp;
    if(execute("svn",
               EXE_STR, "mv",
                   EXE_STR, "--",
               EXE_STRING, &source,
               EXE_STRING, &destination,
               EXE_END))
      exit(1);
  }

  int show(const string &change) const {
    return execute("svn",
                   EXE_STR, "diff",
                   EXE_STR, "-c",
                   EXE_STRING, &change,
                   EXE_END);
  }
};

static const svn vcs_svn;

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
