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
#include <sstream>
#include <fcntl.h>

// Bring up the user's editor to edit the contents of FILE
int editor(vector<string> &file) {
  const string tmpfile = tempfile();
  FILE *fp = fopen(tmpfile.c_str(), "w");
  if(!fp)
    fatal("opening %s: %s", tmpfile.c_str(), strerror(errno));
  for(size_t n = 0; n < file.size(); ++n)
    if(fputs(file[n].c_str(), fp) < 0
       || fputc('\n', fp) < 0)
      fatal("error writing to %s: %s", tmpfile.c_str(), strerror(errno));
  if(fclose(fp) < 0)
    fatal("error writing to %s: %s", tmpfile.c_str(), strerror(errno));
  const char *editor = getenv("VISUAL");
  if(!editor)
    editor = getenv("EDITOR");
  if(!editor)
    editor = "vi";              // traditional fallback
  ostringstream cmd;
  cmd << editor << " " << tmpfile;
  int rc = system(cmd.str().c_str());
  if(!rc) {
    if(!(fp = fopen(tmpfile.c_str(), "r")))
      fatal("opening %s: %s", tmpfile.c_str(), strerror(errno));
    string l;
    file.clear();
    while(readline(tmpfile, fp, l))
      file.push_back(l);
    fclose(fp);
  } else if(WIFEXITED(rc))
    rc = WEXITSTATUS(rc);
  else if(WIFSIGNALED(rc)) {
    fprintf(stderr, "%s received fatal signal %d (%s)", editor, 
            WTERMSIG(rc), strsignal(WTERMSIG(rc)));
    rc = 128 + WTERMSIG(rc);
  }
  erase(tmpfile.c_str());
  return rc;
}

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
