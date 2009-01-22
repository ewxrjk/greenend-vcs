#include "vcs.h"

static const struct option add_options[] = {
  { "help", no_argument, 0, 'h' },
  { "binary", no_argument, 0, 'b' },
  { 0, 0, 0, 0 },
};

static void add_help(FILE *fp = stdout) {
  fprintf(fp, 
          "Usage:\n"
          "  vcs add [OPTIONS] FILENAME ...\n"
          "Options:\n"
          "  --help, -h     Display usage message\n"
          "  --binary, -b   Files are binary files\n");
}

int vcs_add(const struct vcs *v, int argc, char **argv) {
  int n, binary;

  optind = 1;
  while((n = getopt_long(argc, argv, "+hb", add_options, 0)) >= 0) {
    switch(n) {
    case 'h':
      add_help();
      return 0;
    case 'b':
      binary = 1;
      break;
    default:
      return 1;
    }
  }
  if(argc == optind) {
    add_help(stderr);
    return 1;
  }
  return v->add(binary, argc - optind, argv + optind);
  
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
