/*
 * This file is part of VCS
 * Copyright (C) 2009, 2010 Richard Kettlewell
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

static const struct option remove_options[] = {
  { "help", no_argument, 0, 'h' },
  { "force", no_argument, 0, 'f' },
  { 0, 0, 0, 0 },
};

class remove_: public command {
public:
  remove_(): command("remove", "Remove files") {
    register_alias("rm");
  }

  void help(FILE *fp = stdout) const {
    fprintf(fp,
            "Usage:\n"
            "  vcs remove [OPTIONS] FILENAME ...\n"
            "Options:\n"
            "  --help, -h     Display usage message\n"
            "  --force, -f    Force removal\n"
            "\n"
            "Remove files from version control.\n");
  }

  int execute(int argc, char **argv) const {
    int n, force = 0;

    optind = 1;
    while((n = getopt_long(argc, argv, "+hf", remove_options, 0)) >= 0) {
      switch(n) {
      case 'h':
        help();
        return 0;
      case 'f':
        force = 1;
        break;
      default:
        return 1;
      }
    }
    if(argc == optind) {
      help(stderr);
      return 1;
    }
    return guess()->remove(force, vector<string>(argv+optind, argv+argc));
  }
};

static remove_ remove_command;

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
