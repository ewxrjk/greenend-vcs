/*
 * This file is part of VCS
 * Copyright (C) 2011 Richard Kettlewell
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
#include "rcsbase.h"

class rcs: public rcsbase {
public:
  rcs(): rcsbase("RCS") {
    register_subdir("RCS");
  }

  string tracking_directory() const {
    return "RCS";
  }

  bool is_tracking_file(const string &path) const {
    return path.size() > 2 && path.compare(path.size() - 2, 2, ",v") == 0;
  }

  string tracking_basename(const string &name) const {
    return name + ",v";
  }

  string working_basename(const string &name) const {
    return name.substr(0, name.size() - 2);
  }

  int native_diff(const vector<string> &files) const {
    return execute("rcsdiff",
                   EXE_STR, "-u",
                   EXE_VECTOR|EXE_DOTSTUFF, &files,
                   EXE_END);
  }

  int add(int binary, int nfiles, char **files) const {
    if(binary)
      fatal("--binary option not supported for RCS");
    return rcsbase::add(binary, nfiles, files);
  }

  int native_commit(const vector<string> &files, const string &msg) const {
    vector<string> command;
    command.push_back("ci");
    command.push_back("-u");    // don't delete work file
    command.push_back("-t-" + string(msg));
    command.push_back("-m" + string(msg));
    for(size_t n = 0; n < files.size(); ++n)
      if(n == 0 && files[n][0] == '-')
        command.push_back("./" + std::string(files[n]));
      else
        command.push_back(files[n]);
    if(dryrun || verbose)
      display_command(command);
    if(dryrun)
      return 0;
    return execute(command);
  }

  int native_revert(int nfiles, char **files) const {
    return execute("co",
                   EXE_STR, "-f",
                   EXE_STRS|EXE_DOTSTUFF, nfiles, files,
                   EXE_END);
  }

  int native_update(const vector<string> &files) const {
    return execute("co",
                   EXE_VECTOR|EXE_DOTSTUFF, &files,
                   EXE_END);
  }

  int log(const char *path) const {
    if(!path)
      fatal("'vcs log' requires a filename with RCS");
    return execute("rlog",
                   EXE_STR|EXE_DOTSTUFF, path,
                   EXE_END);
  }

  int native_edit(int nfiles, char **files) const {
    return execute("co",
                   EXE_STR, "-l", // make work file writable
                   EXE_STRS|EXE_DOTSTUFF, nfiles, files,
                   EXE_END);
  }

};

static const rcs vcs_rcs;

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
