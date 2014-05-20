/*
 * This file is part of VCS
 * Copyright (C) 2009, 2014 Richard Kettlewell
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

static const char *const names[] = {
  "/",
  "/a",
  "/a/a",
  "/a/a+",
  "/a/b",
  "/a/b/a",
  "/a/b/b",
  "/a/b+",
  "/a/bZ",
  "/a/c",
  "/a/c/a",
  "/a+",
  "/b/a",
  "/b/c",
  "/b/c/a",
  "/z",
  "/zz",
};
static const size_t nnames = sizeof names / sizeof *names;

static const char *const bn[] = { "false", "true" };

int main(void) {
  ltfilename lt;
  int failed = 0;

  for(size_t i = 0; i < nnames; ++i) {
    for(size_t j = 0; j < nnames; ++j) {
      {
        const bool expect = (i < j);
        const bool got = lt(names[i], names[j]);
        if(got != expect) {
          fprintf(stderr, "%s < %s: got %s but expected %s\n",
                  names[i], names[j],
                  bn[got], bn[expect]);
          ++failed;
          exit(1);
        }
      }
      // ..and again for relative paths
      if(names[i][0] == '/' && names[j][0] == '/') {
        const bool expect = (i < j);
        const bool got = lt(names[i] + 1, names[j] + 1);
        if(got != expect) {
          fprintf(stderr, "%s < %s: got %s but expected %s\n",
                  names[i] + 1, names[j] + 1,
                  bn[got], bn[expect]);
          ++failed;
          exit(1);
        }
      }
      // We don't compare and relative paths, we don't put the two together.
    }
  }
  return !!failed;
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
