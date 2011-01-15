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
#include <stdexcept>

extern "C" {
  extern char **environ;
}

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

static string p4_decode_raw(const string &s) {
  string r;
  string::size_type n;
  
  for(n = 0; n < s.size(); ++n) {
    if(s[n] == '%') {
      r += 16 * fromhex(s.at(n + 1)) + fromhex(s.at(n + 2));
      n += 2;
    } else
      r += s[n];
  }
  return r;
}

string p4_decode(const string &s) {
  try {
    return p4_decode_raw(s);
  } catch(out_of_range &e) {
    fprintf(stderr, "ERROR: out of range decoding path '%s'\n",
            s.c_str());
    throw e;
  }
}

// Expects one line in 'p4 where' syntax, which is a depot path,
// a client path in p4 syntax and a client path in local syntax.
//
// If the original argument contain %-escape sequences (see the Issuing P4
// Commands chapter in the Perforce user guide) then they are preserved in the
// first two but are replaced by their expansion (which might be a space) in
// the third.
void P4Where::parse(const string &l) {
  string::size_type n = l.find(' ');
  depot_path = p4_decode(l.substr(0, n));
  string::size_type m = n + 1;
  n = l.find(' ', m);
  view_path = p4_decode(l.substr(m, n - m));
  m = n + 1;
  local_path.assign(l, m, string::npos); // not encoded!
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
    vector<string> someresults, errors;
    int rc;
    if((rc = execute(cmd, NULL/*input*/, &someresults, &errors))) {
      report_lines(errors);
      fatal("'p4 where PATHS' exited with status %d", rc);
    }
    size_t n;
    for(n = 0; n < errors.size(); ++n) {
      if(errors[n].find(" - file(s) not in client view.") == string::npos)
        break;
    }
    if(n < errors.size()) {
      report_lines(errors);
      fatal("Unexpected error output from 'p4 where PATHS'");
    }
    while(someresults.size()
          && someresults.back().size() == 0)
      someresults.pop_back();
    where.insert(where.end(), someresults.begin(), someresults.end());
    it = e;
  }
}

// Run 'p4 where' on all the listed files,  breaking up into multiple
// invocations to avoid command-line length limits and returning the
// results index by depot path, view path and local path.
void p4__where(const list<string> &files,
               map<string,P4Where> &depot,
               map<string,P4Where> &view,
               map<string,P4Where> &local) {
  vector<string> where;
  
  p4__where(where, files);
  depot.clear();
  view.clear();
  local.clear();
  for(size_t n = 0; n < where.size(); ++n) {
    P4Where w(where[n]);

    //fprintf(stderr, "local %s\n", w.local_path.c_str());
    depot.insert(pair<string,P4Where>(w.depot_path, w));
    view.insert(pair<string,P4Where>(w.view_path, w));
    local.insert(pair<string,P4Where>(w.local_path, w));
  }
}

// P4FileInfo ------------------------------------------------------------------

P4FileInfo::P4FileInfo(): rev(-1), chnum(0), locked(false), 
                          resolvable(false), changed(true) {
}

// Expects 'p4 opened' output.
//
//   depot-file#rev - action chnum change (type) [lock-status]
P4FileInfo::P4FileInfo(const string &l): rev(-1), chnum(0), locked(false),
                                         resolvable(false), changed(true) {
  parse(l);
}

void P4FileInfo::parse(const string &l) {
  try {
    parse_raw(l);
  } catch(out_of_range &e) {
    fprintf(stderr, "ERROR: out of range in P4FileInfo::parse '%s'\n",
            l.c_str());
    throw e;
  }
}

void P4FileInfo::parse_raw(const string &l) {
  // Get the depot path
  string::size_type n = l.find('#');
  if(n == string::npos)
    fatal("no '#' found in 'p4 opened' output: %s",
          l.c_str());
  depot_path.assign(p4_decode(l.substr(0, n)));
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
  if(n < l.size() && l.at(n) == '*')
    locked = true;
}

P4FileInfo::~P4FileInfo() {
}

// P4Info ----------------------------------------------------------------------

P4Info::P4Info() {
}

P4Info::~P4Info() {
}

bool P4Info::depot_find(const string &depot_path, P4FileInfo &fi) const {
  info_type::const_iterator it = info.find(depot_path);

  if(it != info.end()) {
    fi = it->second;
    return true;
  } else
    return false;
}

bool P4Info::local_find(const string &local_path, P4FileInfo &fi) const {
  const filemap_type::const_iterator it = by_local.find(local_path);
  
  if(it == by_local.end())
    return false;
  return depot_find(it->second, fi);
}

bool P4Info::relative_find(const string &relative_path, P4FileInfo &fi) const {
  const filemap_type::const_iterator it = by_relative.find(relative_path);
  
  if(it == by_relative.end())
    return false;
  return depot_find(it->second, fi);
}

void P4Info::depot_list(list<string> &depot_paths) const {
  depot_paths.clear();
  for(info_type::const_iterator it = info.begin();
      it != info.end();
      ++it)
    depot_paths.push_back(it->second.depot_path);
}

void P4Info::local_list(list<string> &local_paths) const {
  local_paths.clear();
  for(info_type::const_iterator it = info.begin();
      it != info.end();
      ++it)
    local_paths.push_back(it->second.local_path);
}

void P4Info::relative_list(list<string> &relative_paths) const {
  relative_paths.clear();
  for(info_type::const_iterator it = info.begin();
      it != info.end();
      ++it)
    relative_paths.push_back(it->second.relative_path);
}

void P4Info::gather() {
  vector<string> command, opened, errors, have;
  int rc;

  info.clear();
  by_local.clear();
  by_relative.clear();
  
  // 'p4 opened' gives all opened files, in the form:
  //   DEPOT-PATH#REV - ACTION CHNUM change (TYPE) [...]
  // ACTION is add, edit, delete, branch, integrate
  // CHNUM is the change number or 'default'.
  if((rc = execute(makevs(command, "p4", "opened", "...", (char *)0),
                   NULL/*input*/,
                   &opened,
                   &errors))) {
    report_lines(errors);
    fatal("'p4 opened ...' exited with status %d", rc);
  }
  if(!(errors.size() == 0
       || (errors.size() == 1
           && errors[0] == "... - file(s) not opened on this client."))) {
    report_lines(errors);
    fatal("Unexpected error output from 'p4 opened ...'");
  }
  // Build the info array
  for(size_t n = 0; n < opened.size(); ++n) {
    P4FileInfo fi(opened[n]);

    //fprintf(stderr, "opened: %s -> %s\n", fi.depot_path.c_str(), fi.action.c_str());
    info[fi.depot_path] = fi;
  }

  // 'p4 have' gives all files, in the form:
  //   DEPOT-PATH#REV - LOCAL-PATH
  if((rc = execute(makevs(command, "p4", "have", "...", (char *)0),
                   NULL/*input*/,
                   &have)))
    fatal("'p4 have ...' exited with status %d", rc);
  for(size_t n = 0; n < have.size(); ++n) {
    const string &l = have[n];
    string::size_type i = l.find('#');
    const string depot_path = p4_decode(l.substr(0, i));
    ++i;
    string revs;
    int rev;
    try {
      while(isdigit(l.at(i)))
        revs += l[i++];
      rev = atoi(revs.c_str());
      while(l.at(i) == ' ' || l.at(i) == '-')
        ++i;
    } catch(out_of_range &e) {
      fprintf(stderr, "ERROR: out of range in P4Info::gather '%s'\n",
              l.c_str());
      throw e;
    }
    const string local_path = l.substr(i);
    info_type::iterator it = info.find(depot_path);
    if(it == info.end()) {
      // Not an open file
      P4FileInfo fi;
      fi.depot_path = depot_path;
      fi.rev = rev;
      fi.local_path = local_path;
      info[fi.depot_path] = fi;
      //fprintf(stderr, "have(1): %s -> %s\n", fi.depot_path.c_str(), fi.local_path.c_str());
    } else {
      // Must be an open file.  Usefuly we can pick up the local path here.
      it->second.local_path = local_path;
      //fprintf(stderr, "have(2): %s -> %s\n", it->second.depot_path.c_str(), it->second.local_path.c_str());
    }
    by_local[local_path] = depot_path;
  }

  // We'll still be missing local paths for files known to 'opened' but not
  // 'have', e.g. those newly added.

  // Accumulate a list of files we don't know the local path for
  list<string> files;
  for(info_type::iterator it = info.begin();
      it != info.end();
      ++it)
    if(it->second.local_path.size() == 0)
      files.push_back(it->first);
  
  if(files.size()) {
    // Use 'p4 where' to map depot paths to local paths
    vector<string> where;
    p4__where(where, files);
    for(size_t n = 0; n < where.size(); ++n) {
      const P4Where w(where[n]);
      info[w.depot_path].local_path = w.local_path;
    }
  }

  // Fill in relative paths
  for(info_type::iterator it = info.begin();
      it != info.end();
      ++it) {
    it->second.relative_path = get_relative_path(it->second.local_path);
    by_relative[it->second.relative_path] = it->first;
  }

  // Identify files needing 'p4 resolve'
  vector<string> resolvable;
  if((rc = execute(makevs(command, "p4", "resolve", "-n", "...", (char *)NULL),
                   NULL/*input*/,
                   &resolvable,
                   &errors))) {
    report_lines(errors);
    fatal("'p4 resolve -n ...' exited with status %d", rc);
  }
  // output is /full/local/path - merging //source/depot/path#revno
  for(size_t n = 0; n < resolvable.size(); ++n) {
    const string &r = resolvable[n];
    const string local_path = p4_decode(r.substr(0, r.find(' ')));
    const string depot_path = by_local[local_path];
    info[depot_path].resolvable = true;
  }

  // Identify files which have/haven't changed
  vector<string> unchanged;
  if((rc = execute(makevs(command, "p4", "revert", "-an", "...", (char *)NULL),
                   NULL/*input*/,
                   &unchanged,
                   &errors))) {
    report_lines(errors);
    fatal("'p4 revert -an ...' exited with status %d", rc);
  }
  // output is //DEPOT/PATH#REVNO - was ACTION, reverted
  for(size_t n = 0; n < unchanged.size(); ++n) {
    const string &u = unchanged[n];
    const string depot_path = p4_decode(u.substr(0, u.find('#')));
    info[depot_path].changed = false;
  }
}

// ltfilename ------------------------------------------------------------------

// Split into path components.
static void split_path(const string &path, vector<string> &bits) {
  bits.clear();
  string::size_type n = 0;
  while(n < path.size()) {
    if(path[n] == '/') {
      ++n;
    } else {
      string::size_type slash = path.find('/', n);
      bits.push_back(path.substr(n, slash - n));
      n = slash;
    }
  }
}

// Filename comparison should group files in the same directory together.  We
// do this by finding the common prefix of A and B, then comparing the
// immediately subsequent path component.
bool ltfilename::operator()(const string &a, const string &b) const {
  // Decompose into path components
  vector<string> abits, bbits;
  split_path(a, abits);
  split_path(b, bbits);
  // Find the common prefix length
  size_t n;
  for(n = 0; n < abits.size() && n < bbits.size() && abits[n] == bbits[n]; ++n)
    ;
  // If we've "run out" of both sides then they're equal
  if(n == abits.size() && n == bbits.size())
    return false;
  // If we've only run out of one side then that side is smaller
  if(n == abits.size())
    return true;
  if(n == bbits.size())
    return false;
  // Otherwise we compare the subsequent component
  return abits[n] < bbits[n];
}

/*
Local Variables:
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
