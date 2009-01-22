#include "vcs.h"

static const struct option diff_options[] = {
  { "help", no_argument, 0, 'h' },
  { 0, 0, 0, 0 },
};

static void diff_help(FILE *fp = stdout) {
  fprintf(fp, 
          "Usage:\n"
          "  vcs diff [OPTIONS] [FILENAME ....]\n"
          "Options:\n"
          "  --help, -h    Display usage message\n"
          "\n"
          "If no filenames are specified then all changes are shown.\n"
          "If any filenames are specified then only changes to those files\n"
          "are shown.\n");
}

int vcs_diff(const struct vcs *v, int argc, char **argv) {
  int n;

  optind = 1;
  while((n = getopt_long(argc, argv, "+h", diff_options, 0)) >= 0) {
    switch(n) {
    case 'h':
      diff_help();
      return 0;
    default:
      return 1;
    }
  }
  return v->diff(argc - optind, argv + optind);
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
