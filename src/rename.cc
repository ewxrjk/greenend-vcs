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

static const struct option rename_options[] = {
  { "help", no_argument, 0, 'h' },
  { 0, 0, 0, 0 },
};

static void rename_help(FILE *fp = stdout) {
  fprintf(fp, 
          "Usage:\n"
          "  vcs rename [OPTIONS] SOURCE... DESTINATION\n"
          "Options:\n"
          "  --help, -h     Display usage message\n"
          "\n"
          "Rename one or more files.\n");
}

int vcs_rename(int argc, char **argv) {
  int n;

  optind = 1;
  while((n = getopt_long(argc, argv, "+h", rename_options, 0)) >= 0) {
    switch(n) {
    case 'h':
      rename_help();
      return 0;
    default:
      return 1;
    }
  }
  if(argc < optind + 2) {
    rename_help(stderr);
    return 1;
  }
  int nsources = (argc - optind) - 1;
  char **sources = &argv[optind];
  const char *destination = argv[argc - 1];
  if(nsources > 1 && !isdir(destination))
    fatal("When renaming multiple files, destination must be a directory.\n");
  return vcs::guess()->rename(nsources, sources, destination);
  
}


/*
Local Variables:
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
