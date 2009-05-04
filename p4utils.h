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
#ifndef P4UTILS_H
#define P4UTILS_H

struct P4Opened {
  string path;                          // depot path
  int rev;                              // revision number
  string action;                        // action
  int chnum;                            // change number, -1 for default
  string type;                          // file type
  bool locked;                          // true for *locked*

  P4Opened(): rev(0),
              chnum(-1),
              locked(false) {
  }

  P4Opened(const string &l): rev(0),
                             chnum(-1),
                             locked(false) { 
    parse(l);
  }

  // Zap everything
  void clear();

  // Parse one line
  void parse(const string &l);
};

// Parsed 'p4 where' output
struct P4Where {
  string depot_path;
  string view_path;
  string local_path;

  P4Where() {
  }

  P4Where(const string &l) {
    parse(l);
  }

  void parse(const string &l);
};

string p4_encode(const string &s);
char **p4_encode(int nfiles, char **files);
string p4_decode(const string &s);
void p4__where(vector<string> &where, const list<string> &files);
void p4__where(const list<string> &files,
               map<string,P4Where> &depot,
               map<string,P4Where> &view,
               map<string,P4Where> &local);

#endif /* P4UTILS_H */

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
