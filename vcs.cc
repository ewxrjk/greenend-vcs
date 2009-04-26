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
	  "  -h, --help        Display usage message\n"
	  "  -H, --commands    Display command list\n"
	  "  -V, --version     Display version number\n"
          "  -g, --guess       Guess which version control system is in use\n"
          "  -4, -6            Force IP version for network access\n"
	  "\n"
	  "Use 'vcs COMMAND --help' for per-command help.\n");
}

// Table of commands
static const struct command {
  const char *name;
  const char *alias;
  const char *description;
  int (*action)(int argc, char **argv);
} commands[] = {
  {
    "add", NULL,
    "Add files to version control",
    vcs_add
  },
  {
    "annotate", "blame",
    "Annotate each line with revision number",
    vcs_annotate
  },
  {
    "clone", "clone",
    "Check files out of a repository",
    vcs_clone
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
    "edit", NULL,
    "Edit files",
    vcs_edit
  },
  {
    "log", NULL,
    "Summarize history",
    vcs_log
  },
  {
    "remove", "rm",
    "Remove files",
    vcs_remove
  },
  {
    "revert", NULL,
    "Revert changes",
    vcs_revert
  },
  {
    "status", NULL,
    "Display current status",
    vcs_status,
  },
  {
    "update", NULL,
    "Update working tree",
    vcs_update,
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
  while((n = getopt_long(argc, argv, "+hVHgvn46d", options, 0)) >= 0) {
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
    case '4': 
    case '6':
      ipv = n - '0';
      break;
    case 'd':
      debug = 1;
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
#if HAVE_LIBCURL
  CURLcode rc = curl_global_init(CURL_GLOBAL_ALL);
  if(rc)
    fatal("curl_global_init: %d (%s)", rc, curl_easy_strerror(rc));
#endif
  const struct command *c = find_command(argv[optind]);
  const int status = c->action(argc - optind, argv + optind);
  if(fclose(stdout) < 0)
    fatal("closing stdout: %s", strerror(errno));
  await_redirect();
  return status;
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
