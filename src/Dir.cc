#include <config.h>
#include "Dir.h"
#include "vcs.h"

Dir::Dir(): dp(NULL) {}

Dir::Dir(const std::string &path_): dp(NULL) {
  open(path_);
}

void Dir::open(const std::string &path_) {
  if(dp) {
    closedir(dp);
    dp = NULL;
  }
  path = path_;
  if(!(dp = opendir(path.c_str())))
    fatal("opening directory %s: %s", path.c_str(), strerror(errno));
}

Dir::~Dir() {
  if(dp)
    closedir(dp);
}

bool Dir::get(std::string &name) const {
  errno = 0;
  struct dirent *de = readdir(dp);
  if(de) {
    name = de->d_name;
    return true;
  } else {
    if(errno)
      fatal("reading directory %s: %s", path.c_str(), strerror(errno));
    return false;
  }
}

void Dir::getFiles(std::vector<std::string> &files,
                   const std::string &dir) {
  files.clear();
  Dir d(dir);
  std::string name;
  while(d.get(name))
    files.push_back(name);
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
