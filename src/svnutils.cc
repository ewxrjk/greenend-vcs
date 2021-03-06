/*
 * This file is part of VCS
 * Copyright (C) 2014 Richard Kettlewell
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
#include "svnutils.h"
#include <unistd.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>

// Add trailing @ to disambiguate filenames with @ in from peg
// revisions
//
// See: http://svnbook.red-bean.com/en/1.7/svn.advanced.pegrevs.html
string svn_encode(const string &s) {
  if(s.find('@') == string::npos)
    return s;
  else
    return s + "@";
}

/*
Local Variables:
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
