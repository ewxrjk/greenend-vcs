/*
 * This file is part of VCS
 * Copyright (C) 2009, 210 Richard Kettlewell
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

static const struct option log_options[] = {
  { "help", no_argument, 0, 'h' },
  { 0, 0, 0, 0 },
};

class log_: public command {
public:
  log_(): command("log", "Summarize history") {
  }

  void help(FILE *fp = stdout) const {
    fprintf(fp,
            "Usage:\n"
            "  vcs log [OPTIONS] [FILENAME ...]\n"
            "Options:\n"
            "  --help, -h     Display usage message\n"
            "\n"
            "Displays the history of files, or of your whole working tree.\n");
  }

  int execute(int argc, char **argv) const {
    int n;

    optind = 1;
    while((n = getopt_long(argc, argv, "+h", log_options, 0)) >= 0) {
      switch(n) {
      case 'h':
        help();
        return 0;
      default:
        return 1;
      }
    }
    if(argc - optind > 1) {
      help(stderr);
      return 1;
    }
    redirect(getenv("VCS_PAGER"));
    if(argc == optind)
      return guess()->log(0);
    else {
      string path = argv[optind];
      return guess()->log(&path);
    }
  }
};

static log_ log_command;

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
