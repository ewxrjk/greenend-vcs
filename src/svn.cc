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
#include "xml.h"

class svn: public vcs {
public:

  svn(): vcs("Subversion") {
    register_subdir(".svn");
    register_scheme("svn");
    register_scheme("svn+ssh");
  }

  int diff(int nfiles, char **files) const {
    return execute("svn",
                   EXE_STR, "diff",
                   EXE_STRS, nfiles, files,
                   EXE_END);
  }

  int add(int /*binary*/, int nfiles, char **files) const {
    return execute("svn",
                   EXE_STR, "add",
                   EXE_STRS_DOTSTUFF, nfiles, files,
                   EXE_END);
  }

  int remove(int force, int nfiles, char **files) const {
    return execute("svn",
                   EXE_STR, "delete",
                   EXE_IFSTR(force, "--force"),
                   EXE_STRS, nfiles, files,
                   EXE_END);
  }

  int commit(const char *msg, int nfiles, char **files) const {
    return execute("svn",
                   EXE_STR, "commit",
                   EXE_IFSTR(msg, "-m"),
                   EXE_IFSTR(msg, msg),
                   EXE_STRS, nfiles, files,
                   EXE_END);
  }

  int revert(int nfiles, char **files) const {
    // Subversion's revert insist you tell it what files to revert.  So if
    // we want to revert everything we must cobble together a list.
    if(!nfiles) {
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
      vector<char *> files;
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
          files.push_back((char *)entry->attributes["path"].c_str());
      }
      if(!files.size())
        return 0;
      return revert((int)files.size(), &files[0]);
    } else
      return execute("svn",
                     EXE_STR, "revert",
                     EXE_STRS, nfiles, files,
                     EXE_END);
  }

  int status() const {
    return execute("svn",
                   EXE_STR, "status",
                   EXE_END);
  }

  int update() const {
    return execute("svn",
                   EXE_STR, "update",
                   EXE_END);
  }

  int log(const char *path) const {
    return execute("svn",
                   EXE_STR, "log",
                   EXE_IFSTR(path, path),
                   EXE_END);
  }

  int annotate(const char *path) const {
    return execute("svn",
                   EXE_STR, "blame",
                   EXE_STR, path,
                   EXE_END);
  }

  int clone(const char *uri, const char *dir) const {
    return execute("svn",
                   EXE_STR, "checkout",
                   EXE_STR, uri,
                   EXE_IFSTR(dir, dir),
                   EXE_END);
  }

  // At least up to svn 1.4.6, mv can only take a single source and
  // destination.  At some point support for old versions will be dropped but
  // currently I consider that too recent to ignore.
  void rename_one(const string &source, const string &destination) const {
    string sp, dp;
    if(execute("svn",
               EXE_STR, "mv",
               EXE_STR, source.c_str(),
               EXE_STR, destination.c_str(),
               EXE_END))
      exit(1);
  }

  int show(const char *change) const {
    return execute("svn",
                   EXE_STR, "diff",
                   EXE_STR, "-c",
                   EXE_STR, change,
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
