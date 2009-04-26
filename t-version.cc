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
  assert(version_compare("1.0", "1.0") == 0);
  assert(version_compare("0.9", "0.9") == 0);
  assert(version_compare("wibble", "wibble") == 0);
  assert(version_compare("", "") == 0);
  assert(version_compare("1.0.1", "1.0") == 1);
  assert(version_compare("1.1.1", "1.0") == 1);
  assert(version_compare("0.1", "1.0") == -1);
  assert(version_compare("0.1", "1.1") == -1);
  assert(version_compare("0.9.2", "0.9.1") == 1);
  assert(version_compare("0.9.1", "0.9.2") == -1);
  puts("OK");
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
