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

static const struct option clone_options[] = {
  { "help", no_argument, 0, 'h' },
  { 0, 0, 0, 0 },
};

class clone_: public command {
public:
  clone_(): command("clone", "Check files out of a repository") {
  }

  void help(FILE *fp = stdout) const {
    fprintf(fp,
            "Usage:\n"
            "  vcs clone [OPTIONS] URI [DIRECTORY]\n"
            "Options:\n"
            "  --help, -h     Display usage message\n"
            "\n"
            "Creates a local copy of a branch or (part of) a repository.\n");
  }

  int execute(int argc, char **argv) const {
    int n;

    optind = 1;
    while((n = getopt_long(argc, argv, "+h", clone_options, 0)) >= 0) {
      switch(n) {
      case 'h':
        help();
        return 0;
      default:
        return 1;
      }
    }
    if(argc - optind < 1 || argc - optind > 2) {
      help(stderr);
      return 1;
    }
    const vcs *v = vcs::guess_branch(argv[optind]);
    const string *dir;
    string d;
    if(argc - optind == 2) {
      d = argv[optind+1];
      dir = &d;
    } else
      dir = 0;
    return v->clone(argv[optind], dir);
  }
};

static clone_ command_clone;

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
