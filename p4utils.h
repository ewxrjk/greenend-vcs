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

// Information about one file
struct P4FileInfo {
  P4FileInfo();
  P4FileInfo(const string &l);
  ~P4FileInfo();
  string depot_path;                    // depot path
  string view_path;                     // view path (NB sometimes missing)
  string local_path;                    // absolute local path
  string relative_path;                 // relative local path
  int rev;                              // revision number
  string action;                        // action or "" if not open
  int chnum;                            // change number, -1 for default
  string type;                          // file type or "" if not known
  bool locked;                          // true for *locked*
};

struct ltfilename {
  bool operator()(const string &a, const string &b) const;
};

// Collate information about files indexed in various ways
class P4Info {
public:
  P4Info();
  ~P4Info();
  
  // Look up one file by various kinds of filename
  bool depot_find(const string &depot_path, P4FileInfo &fi) const;
  bool local_find(const string &local_path, P4FileInfo &fi) const;
  bool relative_find(const string &relative_path, P4FileInfo &fi) const;
  
  // Look up a list of known filenames of whichever kind
  void depot_list(list<string> &depot_paths) const;
  void local_list(list<string> &local_paths) const;
  void relative_list(list<string> &relative_paths) const;

  // (Re-)gather information matching a given pattern
  void gather();

private:
  typedef map<string,P4FileInfo> info_type;
  typedef map<string,string> filemap_type;
  info_type info;                       // depot path -> information
  filemap_type by_local;                // local path -> depot path
  filemap_type by_relative;             // relative path -> depot path
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
