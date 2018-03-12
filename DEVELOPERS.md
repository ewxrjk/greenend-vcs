# Before You Start, And After You Finish

## Availability Of Dependencies

I need to be able to test any changes you submit.  This means I need
to be able to get and run any new version control systems that are
supported without unreasonable effort.

This doesn’t mean proprietary version control systems are out of the
question - some vendors for instance make a limited but adequate (for
vcs) version of their software available.  But I’m not going to pay
money just to test compatibility with a system I don’t use myself.

## Submitting Patches

[Please raise pull requests on GitHub](https://github.com/ewxrjk/greenend-vcs).

# Adding Features

## Adding A New Version Control System

### Implementation

Create a `*.cc` file for the new version control system.  Each system
is defined by a class derived from `vcs`. Using [bzr.cc](src/bzr.cc)
as an example:

    class bzr: public vcs {
      bzr(): vcs("Bazaar") {
        register_subdir(".bzr");
        register_substring("bzr");
      }

The constructor should:

* Initialize the base with the friendly name of the version control
  system.
* Call `register_subdir()` to supply the name of a subdirectory
  used by this version control system (if there is one).
* Call `register_substring()` to supply the a substring found in URLs
  used by this version control system (if there is one).

If you need more complicated detection logic then supply a `detect()`
method.  See [p4.cc](src/p4.cc) for an example.

Implement the various commands.  For bzr, most of the commands have
very simple implementations - it just executes the relevant bzr
command.  See [cvs.cc](src/cvs.cc) and [p4.cc](src/p4.cc) for much
more complicated examples.

    int diff(int nfiles, char **files) const {
      return execute("bzr",
                     EXE_STR, "diff",
                     EXE_STR, "--",
                     EXE_STRS, nfiles, files,
                     EXE_END);
    }

You can skip `edit()` if files are always editable.

Define an object of the type of your new class.  The base class
constructor will arrange for its registration.

    static const bzr vcs_bzr;

### Documentation

You should update the man page with at least a pointer to its home
page, and list any peculiarities.

### Testing

Add a test script.  Again copy (for instance) [tests/bzr](tests/bzr)
and edit accordingly.  Remember to update `TESTS=` in
[Makefile.am](tests/Makefile.am).  If the test’s dependencies aren’t
installed the script should `exit 77`.

## Adding A New Command

### Implementation

Create a `*.cc` file for the new command. Define the long names of
options to the command.  See `man getopt` for details.
Using [commit.cc](src/commit.cc) as an example:

    static const struct option commit_options[] = {
      { "help", no_argument, 0, 'h' },
      { "message", required_argument, 0, 'm' },
      { 0, 0, 0, 0 },
    };

All commands derive from the `command` base class.

    class commit: public command {
      commit(): command("commit", "Commit changes") {
        register_alias("ci");
        register_alias("checkin");
      }

The constructor should:

* Initialize the base with the full name of the command and an and
  a English description.  This will appear in `--commands` output.
* Call `register_alias()` with an aliases for the command.

Format the help message as follows.  Assume an 80-column terminal.
Don’t use tab characters.

    void help(FILE *fp = stdout) const {
      fprintf(fp,
              "Usage:\n"
              "  vcs commit [OPTIONS] [FILENAME ...]\n"
              "Options:\n"
              "  --help, -h              Display usage message\n"
              "  --message, -m MESSAGE   Log message\n"
              "\n"
              "Commits changes to version control.  If no files are mentioned\n"
              "then all changed and added files are committed.\n");
    }

Parse the options using `getopt_long()`.  Note the initial `+` in the
third argument.

    int execute(int argc, char **argv) const {
      int n;
      const char *msg = 0;
    
      optind = 1;
      while((n = getopt_long(argc, argv, "+hm:", commit_options, 0)) >= 0) {
        switch(n) {
        case 'h':
          help();
          return 0;
        case 'm':
          msg = optarg;
          break;
        default:
          return 1;
        }
      }

If the command produces a lot of output it may be appropriate to use a
pager.  Using [diff.cc](src/diff.cc) as an example:

    const char *pager = getenv("VCS_DIFF_PAGER");
    if(!pager)
      pager = getenv("VCS_PAGER");
    redirect(pager);

If the output contains diffs use `VCS_DIFF_PAGER`, backing off to
`VCS_PAGER`; otherwise just use `VCS_PAGER`.  This allows the user to
insert colordiff or similar tools.

Invoke the command on the guessed version control system.

    return guess()->commit(msg, argc - optind, argv + optind);

Define an object of the type of your new class.  The base class
constructor will arrange for its registration.

    static commit command_commit;

For each version control system add an implementation of the command.
Here’s the example from `bzr.cc`:

    int commit(const char *msg, int nfiles, char **files) const {
      return execute("bzr",
                     EXE_STR, "commit",
                     EXE_IFSTR(msg, "-m"),
                     EXE_IFSTR(msg, msg),
                     EXE_STR, "--",
                     EXE_STRS, nfiles, files,
                     EXE_END);
    }

### Documentation

Document the command in the man page, remembering to mention any
peculiarities specific to particular version control systems.

### Testing

Ideally, update the test scripts to exercise the new command.
