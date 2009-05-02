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

int main(void) {
  assert(execute("true", EXE_END) == 0);
  assert(execute("false", EXE_END) != 0);
  assert(execute("test", EXE_STR, "-d", EXE_STR, "/", EXE_END) == 0);
  assert(execute("test", EXE_STR, "-f", EXE_STR, "/", EXE_END) != 0);
  vector<string> vs;
  assert(capture(vs, "true", (char *)0) == 0);
  assert(vs.size() == 0);
  assert(capture(vs, "echo", "wibble", (char *)0) == 0);
  assert(vs.size() == 1);
  assert(vs[0] == "wibble");
  assert(capture(vs, "sh", "-c", "echo foo;echo;echo bar", (char *)0) == 0);
  assert(vs.size() == 3);
  assert(vs[0] == "foo");
  assert(vs[1] == "");
  assert(vs[2] == "bar");
  return 0;
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
