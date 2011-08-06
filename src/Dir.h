#ifndef DIR_H
#define DIR_H

#include <sys/types.h>
#include <dirent.h>
#include <string>
#include <vector>

class Dir {
public:
  Dir();
  Dir(const std::string &path_);
  ~Dir();

  void open(const std::string &path_);
  bool get(std::string &name) const;
  
  static void getFiles(std::vector<std::string> &files,
                       const std::string &dir);

private:
  std::string path;
  DIR *dp;
};

#endif /* DIR_H */

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
