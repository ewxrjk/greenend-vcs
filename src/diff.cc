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

static const struct option diff_options[] = {
  { "help", no_argument, 0, 'h' },
  { 0, 0, 0, 0 },
};

class diff: public command {
public:
  diff(): command("diff", "Display changes") {
  }

  void help(FILE *fp = stdout) const {
    fprintf(fp, 
            "Usage:\n"
            "  vcs diff [OPTIONS] [FILENAME ....]\n"
            "Options:\n"
            "  --help, -h    Display usage message\n"
            "\n"
            "Shows differences from last commit.\n"
            "\n"
            "If no filenames are specified then all changes are shown.\n"
            "If any filenames are specified then only changes to those files\n"
            "are shown.\n");
  }

  int execute(int argc, char **argv) const {
    int n;

    optind = 1;
    while((n = getopt_long(argc, argv, "+h", diff_options, 0)) >= 0) {
      switch(n) {
      case 'h':
        help();
        return 0;
      default:
        return 1;
      }
    }
    const char *pager = getenv("VCS_DIFF_PAGER");
    if(!pager)
      pager = getenv("VCS_PAGER");
    redirect(pager);
    return guess()->diff(argc - optind, argv + optind);
  }

};

static diff command_diff;

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
