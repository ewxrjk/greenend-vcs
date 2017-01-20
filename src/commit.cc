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

static const struct option commit_options[] = {
  { "help", no_argument, 0, 'h' },
  { "message", required_argument, 0, 'm' },
  { 0, 0, 0, 0 },
};

class commit: public command {
public:
  commit(): command("commit", "Commit changes") {
    register_alias("ci");
    register_alias("checkin");
  }

  void help(FILE *fp = stdout) const {
    fprintf(fp,
            "Usage:\n"
            "  vcs commit [OPTIONS] [FILENAME ...]\n"
            "Options:\n"
            "  --help, -h              Display usage message\n"
            "  --message, -m MESSAGE   Log message\n"
            "\n"
            "Commits changes to version control.  If no files are mentioned\n"
            "then all changed and added files are committed.\n");
  }

  int execute(int argc, char **argv) const {
    int n;
    const char *msg = 0;

    optind = 1;
    while((n = getopt_long(argc, argv, "+hm:", commit_options, 0)) >= 0) {
      switch(n) {
      case 'h':
        help();
        return 0;
      case 'm':
        msg = optarg;
        break;
      default:
        return 1;
      }
    }
    return guess()->commit(msg, argc - optind, argv + optind);
  }

};

static commit command_commit;

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
