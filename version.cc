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

static long extract_number(const string &s,
                           size_t &an) {
  const char *start = s.c_str() + an;
  const char *end;
  
  errno = 0;
  const long n = strtol(start, (char **)&end, 10);
  if(errno)
    fatal("error converting integer: %s", strerror(errno));
  if(start == end)
    fatal("error converting integer: no digits found");
  an = (size_t)(end - s.c_str());
  return n;
}

int version_compare(const string &a, const string &b) {
  size_t an = 0, bn = 0;
  while(an < a.size() && bn < b.size()) {
    /* Pick out integer parts and compare as integers */
    if(isdigit(a[an]) && isdigit(b[bn])) {
      const long aa = extract_number(a, an);
      const long bb = extract_number(b, bn);
      // ...an/nb updated by these calls
      
      if(aa != bb)
        return aa < bb ? -1 : 1;
    } else {
      const int aa = an < a.size() ? a[an] : -1;
      const int bb = bn < b.size() ? b[bn] : -1;
      
      if(aa != bb)
        return aa < bb ? -1 : 1;
      ++an;
      ++bn;
    }
  }
  if(an < a.size())
    return 1;
  else if(bn < b.size())
    return -1;
  else
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
