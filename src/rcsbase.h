#ifndef RCSBASE_H
#define RCSBASE_H

// For RCS and SCCS
class rcsbase: public vcs {
public:
  rcsbase(const char *name_): vcs(name_) {}
  virtual ~rcsbase();

  // Return the name of the tracking directory (i.e. "RCS" or "SCCS")
  virtual string tracking_directory() const = 0;

  // Return true if this path is a tracking file (i.e. is a ,v or s. file)
  virtual bool is_tracking_file(const string &path) const = 0;

  // Convert a working file basename to tracking file basename
  // (i.e. add ,v or s.)
  virtual string tracking_basename(const string &name) const = 0;

  // Convert a tracking file basename to a working file basename
  // (i.e. strip ,v or s.)
  virtual string working_basename(const string &name) const = 0;

  // Return the tracking path of a working file.  If no tracking path exists
  // then the result will point into the tracking directory if one exists.
  string tracking_path(const string &path) const;

  // Return true if a working path is tracked.
  bool is_tracked(const string &path) const;

  // Return true if this is an add-flag path
  bool is_add_flag(const string &path) const;

  // Return the add-flag path for a working file path
  string flag_path(const string &path) const;

  // Return true if a working file path is flagged for add
  bool is_flagged(const string &path) const;

  // Return true if a working file path is flagged for add as binary
  bool is_binary(const string &path) const;

  // Return the working file path of a tracking file, add-flag file or the path
  // itself.
  string work_path(const string &path) const;

  // Possible file states
  static const int fileWritable = 1;
  static const int fileTracked = 2;
  static const int fileExists = 4;
  static const int fileAdded = 8;
  static const int fileIgnored = 16;

  // Enumerate all files below here
  void enumerate(map<string,int> &files) const;

  virtual int native_diff(const vector<string> &files) const = 0;
  virtual int native_commit(const vector<string> &files,
                            const string &msg) const = 0;
  virtual int native_edit(const vector<string> &files) const = 0;
  virtual int native_update(const vector<string> &files) const = 0;
  virtual int native_revert(const vector<string> &files) const = 0;

  int diff(const vector<string> &files) const;
  virtual int add(int binary, const vector<string> &files) const;
  int commit(const string *msg, const vector<string> &files) const;
  int remove(int force, const vector<string> &files) const;
  int status() const;
  int edit(const vector<string> &files) const;
  int update() const;
  int revert(const vector<string> &files) const;

  bool detect(void) const;
};

#endif /* RCSBASE_H */

/*
Local Variables:
mode:c++
c-basic-offset:2
comment-column:40
fill-column:79
indent-tabs-mode:nil
End:
*/
