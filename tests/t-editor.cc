/*
 * This file is part of VCS
 * Copyright (C) 2011, 2014 Richard Kettlewell
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
  char *env_editor, *env_visual;
  const char *srcdir = getenv("srcdir");
  if(!srcdir)
    srcdir = ".";
  assert(asprintf(&env_editor, "EDITOR=%s/dummy-editor", srcdir) >= 0);
  assert(asprintf(&env_visual, "VISUAL=%s/dummy-editor", srcdir) >= 0);
  putenv(env_editor);
  putenv(env_visual);
  vector<string> f;
  f.push_back("abra");
  assert(editor(f) == 0);
  assert(f.size() == 1);
  assert(f[0] == "cadabra");
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
