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

// Table of global options
static const struct option options[] = {
  { "help", no_argument, 0, 'h' },
  { "commands", no_argument, 0, 'H' },
  { "version", no_argument, 0, 'V' },
  { "guess", no_argument, 0, 'g' },
  { "verbose", no_argument, 0, 'v' },
  { "dry-run", no_argument, 0, 'n' },
  { "debug", no_argument, 0, 'd' },
  { 0, 0, 0, 0 }
};

// Display help message
static void help(FILE *fp = stdout) {
  fprintf(fp, "Usage:\n"
          "  vcs [OPTIONS] COMMAND ...\n"
          "Options:\n"
          "  -v, --verbose     Verbose operation\n"
          "  -n, --dry-run     Report what would be done but do nothing\n"
          "  -d, --debug       Display debug messages (-dd for more)\n"
          "  -h, --help        Display usage message\n"
          "  -H, --commands    Display command list\n"
          "  -V, --version     Display version number\n"
          "  -g, --guess       Guess which version control system is in use\n"
          "  -4, -6            Force IP version for network access\n"
          "\n"
          "Use 'vcs COMMAND --help' for per-command help.\n");
}

// Display list of commands
static void commandlist() {
  writef(stdout, "stdout", "vcs commands:\n\n");
  command::list();
  writef(stdout, "stdout", "\nUse 'vcs COMMAND --help' for per-command help.\n");
}

static int Main(int argc, char **argv) {
  int n;

  if(!setlocale(LC_CTYPE, ""))
    fatal("error calling setlocale: %s", strerror(errno));
  // Parse global options
  while((n = getopt_long(argc, argv, "+hVHgvn46d", options, 0)) >= 0) {
    switch(n) {
    case 'h':
      help();
      return 0;
    case 'V':
      puts("vcs " VERSION);
      return 0;
    case 'H':
      commandlist();
      return 0;
    case 'g': {
      const vcs *v = vcs::guess();
      puts(v->name);
      return 0;
    }
    case 'v':
      ++verbose;
      break;
    case 'n':
      dryrun = 1;
      break;
    case '4':
    case '6':
      ipv = n - '0';
      break;
    case 'd':
      ++debug;
      break;
    default:
      exit(1);
    }
  }
  // Check for a known command
  if(optind >= argc) {
    help(stderr);
    exit(1);
  }
#if HAVE_CURL_CURL_H
  CURLcode rc = curl_global_init(CURL_GLOBAL_ALL);
  if(rc)
    fatal("curl_global_init: %d (%s)", rc, curl_easy_strerror(rc));
#endif
  const class command *c = command::find(argv[optind]);
  const int status = c->execute(argc - optind, argv + optind);
  return status;
}

int main(int argc, char **argv) {
  try {
    int status = Main(argc, argv);
    if(fclose(stdout) < 0)
      fatal("closing stdout: %s", strerror(errno));
    return status;
  } catch(FatalError &e) {
    fprintf(stderr, "ERROR: %s\n", e.what());
    return 1;
  }
}

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
