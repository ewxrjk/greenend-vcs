/*
 * This file is part of VCS
 * Copyright (C) 2010 Richard Kettlewell
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

command::command(const char *name_,
                 const char *description_): name(name_),
                                            description(description_) {
  register_alias(name_);
}

command::~command() {
}

void command::register_alias(const string &alias) {
  if(!commands)
    commands = new commands_t();
  commands->insert(commands_t::value_type(alias, this));
}

void command::list() {
  int maxlen = 0;

  for(commands_t::iterator it = commands->begin();
      it != commands->end();
      ++it) {
    command *c = it->second;
    if(it->first == c->name) {
      const int l = (int)c->name.size();
      if(l > maxlen)
        maxlen = l;
    }
  }
  for(commands_t::iterator it = commands->begin();
      it != commands->end();
      ++it) {
    command *c = it->second;
    if(it->first == c->name) {
      writef(stdout, "stdout", "  %-*s    %s\n",
              maxlen, c->name.c_str(), c->description.c_str());
    }
  }
}

// Find a command given its name or a unique prefix of it
const command *command::find(const string &cmd) {
  std::list<string> prefixes;
  for(commands_t::iterator it = commands->begin();
      it != commands->end();
      ++it) {
    const string &name = it->first;
    command *c = it->second;
    // Exact matches win immediately
    if(name == cmd)
      return c;
    // We don't do prefix-matching on aliases
    if(name != c->name)
      continue;
    // Accumulate a list of prefix matches
    if(cmd.size() >= name.size())
      continue;
    if(!name.compare(0, cmd.size(), cmd))
      prefixes.push_back(name);
  }
  switch(prefixes.size()) {
  case 0:
    fatal("unknown command '%s' (try vcs -H)", cmd.c_str());
  case 1:
    return (*commands)[prefixes.front()];
  default:
    fatal("'%s' is not a unique prefix (try vcs -H or a longer prefix)",
          cmd.c_str());
  }
}

command::commands_t *command::commands;

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
