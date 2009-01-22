#include "vcs.h"

static const struct option options[] = {
  { "help", no_argument, 0, 'h' },
  { "commands", no_argument, 0, 'H' },
  { "version", no_argument, 0, 'V' },
  { 0, 0, 0, 0 }
};

static void help(FILE *fp = stdout) {
  fprintf(fp, "Usage:\n"
	  "  vcs [OPTIONS] COMMAND ...\n"
	  "Options:\n"
	  "  -h, --help        Display usage message\n"
	  "  -H, --commands    Display command list\n"
	  "  -V, --version     Display version number\n"
	  "\n"
	  "Use 'vcs COMMAND --help' for per-command help.\n");
}

static const struct command {
  const char *name;
  const char *description;
  const char *help;
  int (*action)(int argc, char **argv);
} commands[] = {
  {
    "add",
    "Add files to version control",
    "TODO",
    vcs_add
  },
  {
    "remove",
    "Remove files",
    "TODO",
    vcs_remove
  },
  {
    "commit",
    "Commit changes",
    "TODO",
    vcs_commit
  },
  {
    "diff",
    "Display changes",
    "TODO",
    vcs_diff
  },
  {
    "revert",
    "Revert changes",
    "TODO",
    vcs_revert
  },
  { 0, 0, 0, 0 }                        // that's all
};

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

int main(int argc, char **argv) {
  int n;

  // Parse global options
  while((n = getopt_long(argc, argv, "+hV", options, 0)) >= 0) {
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
    default:
      exit(1);
    }
  }
  // Check for a known command
  if(optind >= argc) {
    help(stderr);
    exit(1);
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
