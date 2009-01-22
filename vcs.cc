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

// Global verbose operation flag
int verbose;

// Global dry-run flag
int dryrun;

// Table of global options
static const struct option options[] = {
  { "help", no_argument, 0, 'h' },
  { "commands", no_argument, 0, 'H' },
  { "version", no_argument, 0, 'V' },
  { "guess", no_argument, 0, 'g' },
  { "verbose", no_argument, 0, 'v' },
  { "dry-run", no_argument, 0, 'n' },
  { 0, 0, 0, 0 }
};

// Display help message
static void help(FILE *fp = stdout) {
  fprintf(fp, "Usage:\n"
	  "  vcs [OPTIONS] COMMAND ...\n"
	  "Options:\n"
          "  -v, --verbose     Verbose operation\n"
          "  -n, --dry-run     Report what would be done but do nothing\n"
	  "  -h, --help        Display usage message\n"
	  "  -H, --commands    Display command list\n"
	  "  -V, --version     Display version number\n"
          "  -g, --guess       Guess which version control system is in use\n"
	  "\n"
	  "Use 'vcs COMMAND --help' for per-command help.\n");
}

// Table of commands
static const struct command {
  const char *name;
  const char *alias;
  const char *description;
  int (*action)(const struct vcs *v, int argc, char **argv);
} commands[] = {
  {
    "add", NULL,
    "Add files to version control",
    vcs_add
  },
  {
    "remove", "rm",
    "Remove files",
    vcs_remove
  },
  {
    "status", NULL,
    "Display current status",
    vcs_status,
  },
  {
    "commit", "ci",
    "Commit changes",
    vcs_commit
  },
  {
    "diff", NULL,
    "Display changes",
    vcs_diff
  },
  {
    "update", NULL,
    "Update working tree",
    vcs_update,
  },
  {
    "revert", NULL,
    "Revert changes",
    vcs_revert
  },
  {
    "log", NULL,
    "Summarize history",
    vcs_log
  },
  { 0, 0, 0,0 }                         // that's all
};

// Display list of commands
static void commandlist(FILE *fp = stdout) {
  int n;
  int maxlen = 0;

  for(n = 0; commands[n].name; ++n) {
    const int l = (int)strlen(commands[n].name);
    if(l > maxlen)
      maxlen = l;
  }
  fprintf(fp, "vcs commmands:\n\n");
  for(n = 0; commands[n].name; ++n) {
    fprintf(fp, "  %-*s    %s\n", 
	    maxlen, commands[n].name, 
	    commands[n].description);
  }
  fprintf(fp, "\nUse 'vcs COMMAND --help' for per-command help.\n");
}

// Find a command given its name or a unique prefix of it
static const struct command *find_command(const char *cmd) {
  list<int> prefixes;
  for(int n = 0; commands[n].name; ++n) {
    // Exact matches win immediately
    if(!strcmp(commands[n].name, cmd)
       || (commands[n].alias && !strcmp(commands[n].alias, cmd)))
      return &commands[n];
    // Accumulate a list of prefix matches
    // (NB we don't do prefix-matching on aliases.)
    if(strlen(cmd) < strlen(commands[n].name)
       && !strncmp(cmd, commands[n].name, strlen(cmd)))
      prefixes.push_back(n);
  }
  switch(prefixes.size()) {
  case 0:
    fatal("unknown command '%s' (try vcs -H)", cmd);
  case 1:
    return &commands[prefixes.front()];
  default:
    fatal("'%s' is not a unique prefix (try vcs -H or a longer prefix)", cmd);
  }
}

int main(int argc, char **argv) {
  int n;

  // Parse global options
  while((n = getopt_long(argc, argv, "+hVHgvn", options, 0)) >= 0) {
    switch(n) {
    case 'h': 
      help();
      return 0;
    case 'V':
      puts("vcs "VERSION);
      return 0;
    case 'H':
      commandlist();
      return 0;
    case 'g': {
      const struct vcs *v = guess();
      puts(v->name);
      return 0;
    }
    case 'v':
      ++verbose;
      break;
    case 'n':
      dryrun = 1;
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
  const struct command *c = find_command(argv[optind]);
  const struct vcs *v = guess();
  return c->action(v, argc - optind, argv + optind);
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
