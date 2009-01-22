#include "vcs.h"

static const struct option commit_options[] = {
  { "help", no_argument, 0, 'h' },
  { "message", required_argument, 0, 'm' },
  { 0, 0, 0, 0 },
};

static void commit_help(FILE *fp = stdout) {
  fprintf(fp, 
          "Usage:\n"
          "  vcs commit [OPTIONS] [FILENAME ...]\n"
          "Options:\n"
          "  --help, -h              Display usage message\n"
          "  --message, -m MESSAGE   Log message\n");
}

int vcs_commit(const struct vcs *v, int argc, char **argv) {
  int n;
  const char *msg = 0;

  optind = 1;
  while((n = getopt_long(argc, argv, "+hm:", commit_options, 0)) >= 0) {
    switch(n) {
    case 'h':
      commit_help();
      return 0;
    case 'm':
      msg = optarg;
      break;
    default:
      return 1;
    }
  }
  return v->commit(msg, argc - optind, argv + optind);
  
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
