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
#include "p4utils.h"
#include <sstream>
#include <iomanip>

// Replace metacharacters with %xx
string p4_encode(const string &s) {
  ostringstream r;
  string::size_type n;

  /* "To refer to files containing the Perforce revision specifier wildcards (@
   * and #), file matching wildcard (*), or positional substitution wildcard
   * (%%) in either the file name or any directory component, use the ASCII
   * expression of the character's hexadecimal value. ASCII expansion applies
   * only to the following four characters" - and it mentions @, #, *, %.  What
   * you're supposed to do for spaces in filenames I do not know! 
   */
  r << hex;
  for(n = 0; n < s.size(); ++n)
    switch(s[n]) {
    case '@':
    case '#':
    case '*':
    case '%':
      r << "%" << setw(2) << setfill('0') << (int)s[n];
      break;
    default:
      r << s[n];
      break;
    }
  return r.str();
}

// Replace metacharacters in multiple filesnames
char **p4_encode(int nfiles, char **files) {
  char **newfiles = (char **)calloc(nfiles, sizeof *files);
  int n;

  for(n = 0; n < nfiles; ++n)
    newfiles[n] = xstrdup(p4_encode(files[n]).c_str());
  return newfiles;
}

static int fromhex(int c) {
  switch(c) {
  case '0': return 0;
  case '1': return 1;
  case '2': return 2;
  case '3': return 3;
  case '4': return 4;
  case '5': return 5;
  case '6': return 6;
  case '7': return 7;
  case '8': return 8;
  case '9': return 9;
  case 'a': return 10;
  case 'b': return 11;
  case 'c': return 12;
  case 'd': return 13;
  case 'e': return 14;
  case 'f': return 15;
  case 'A': return 10;
  case 'B': return 11;
  case 'C': return 12;
  case 'D': return 13;
  case 'E': return 14;
  case 'F': return 15;
  default: return 0;
  }
}

string p4_decode(const string &s) {
  string r;
  string::size_type n;
  
  for(n = 0; n < s.size(); ++n) {
    if(s[n] == '%') {
      r += 16 * fromhex(s[n + 1]) + fromhex(s[n + 2]);
      n += 2;
    } else
      r += s[n];
  }
  return r;
}

// Zap everything
void P4Opened::clear() {
  path.clear();
  rev = 0;
  action.clear();
  chnum = -1;
  type.clear();
  locked = false;
}

  // Cheesy parser
void P4Opened::parse(const string &l) {
  clear();
  // Get the depot path
  string::size_type n = l.find('#');
  if(n == string::npos)
    fatal("no '#' found in 'p4 opened' output: %s",
          l.c_str());
  path.assign(p4_decode(l.substr(0, n)));
  // The revision
  string revs;
  ++n;
  while(isdigit(l.at(n)))
    revs += l[n++];
  rev = atoi(revs.c_str());
  // The action
  while(l.at(n) == ' ' || l.at(n) == '-')
    ++n;
  while(l.at(n) != ' ')
    action += l[n++];
  // The change
  string chstr;
  while(l.at(n) == ' ')
    ++n;
  while(l.at(n) != ' ')
    chstr += l[n++];
  if(chstr == "default")
    chnum = -1;
  else
    chnum = atoi(chstr.c_str());
  // The type
  while(l.at(n) == ' ' || l.at(n) == '(')
    ++n;
  while(l.at(n) != ')')
    type += l[n++];
  // Lock status
  while(n < l.size() && (l.at(n) == ' ' || l.at(n) == '('))
    ++n;
  if(n < l.size() && l[n] == '*')
    locked = true;
}

void P4Where::parse(const string &l) {
  string::size_type n = l.find(' ');
  depot_path.assign(l, 0, n);
  string::size_type m = n + 1;
  n = l.find(' ', m);
  view_path.assign(l, m, n - m);
  m = n + 1;
  local_path.assign(l, m, string::npos);
}

// Compute the number of bytes required for the environment
static size_t env_size() {
  char **e = environ;
  size_t size = 1;
  while(*e)
    size += strlen(*e++) + 1;
  return size;
}

// Run 'p4 where' on all the listed files, breaking up into multiple
// invocations to avoid command-line length limits.
void p4__where(vector<string> &where, const list<string> &files) {
  where.clear();

  // Figure out how much space we can safely use for the command line
  size_t limit = sysconf(_SC_ARG_MAX) - 2048;
  const size_t e = env_size();
  if(e >= limit)
    fatal("no space for commands - e=%lu, limit=%lu",
          (unsigned long)e, (unsigned long)limit);
  limit -= e;
  // ARG_MAX is the system limit.  Should be at least 4096.  2048 is clearance
  // for the subprocess to modify its own environment and a few bytes for the
  // 'p4 where'.  We subtract the size of the current environment too.
  //
  // http://www.in-ulm.de/~mascheck/various/argmax/
  
  list<string>::const_iterator it = files.begin();
  while(it != files.end()) {
    // Find the next few kilobytes worth of filenames
    list<string>::const_iterator e = it;
    vector<string> cmd;
    cmd.push_back("p4");
    cmd.push_back("where");
    size_t total = 0;
    while(e != files.end()) {
      const string encoded = p4_encode(*e);
      size_t here = encoded.size() + 1;

      if(total + here > limit)
        break;
      cmd.push_back(encoded);
      total += here;
      ++e;
    }
    if(e == it)
      fatal("filename too long: %s", it->c_str());
    vector<string> someresults;
    int rc;
    if((rc = vcapture(someresults, cmd)))
      fatal("'p4 where PATHS' exited with status %d", rc);
    while(someresults.size()
          && someresults.back().size() == 0)
      someresults.pop_back();
    where.insert(where.end(), someresults.begin(), someresults.end());
    it = e;
  }
}

/*
Local Variables:
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
