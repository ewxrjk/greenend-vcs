#include "vcs.h"

static const struct option revert_options[] = {
  { "help", no_argument, 0, 'h' },
  { 0, 0, 0, 0 },
};

static void revert_help(FILE *fp = stdout) {
  fprintf(fp, 
          "Usage:\n"
          "  vcs revert [OPTIONS] [FILENAME ...]\n"
          "Options:\n"
          "  --help, -h              Display usage message\n");
}

int vcs_revert(const struct vcs *v, int argc, char **argv) {
  int n;

  optind = 1;
  while((n = getopt_long(argc, argv, "+h", revert_options, 0)) >= 0) {
    switch(n) {
    case 'h':
      revert_help();
      return 0;
    default:
      return 1;
    }
  }
  return v->revert(argc - optind, argv + optind);
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
