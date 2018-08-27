/*
 * This file is part of VCS
 * Copyright (C) 201 Richard Kettlewell
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

int writef(FILE *fp, const char *what, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  int rc = vfprintf(fp, fmt, ap);
  va_end(ap);
  if(rc < 0)
    fatal("writing to %s: %s", what, strerror(errno));
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
