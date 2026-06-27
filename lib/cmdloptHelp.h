#ifdef cmdloptHelp_BUILD_LIB
// This is being compiled into a (software project) library.
// FIXME: This is not very portable code:
#  define EXPORT __attribute__((visibility("default"))) extern
#else
// The library API user is using this including this header file.
#  define EXPORT extern
#endif


// This is a header for a library that makes --help print and like
// functions in programs that are separate from the programs that
// use the help.  We link to this library for
// ../quickplot/misc/quickplothelp (prints --help for quickplot) and
// ../Toner/misc/TunerHelp (prints --help for Tuner).


struct opts {
  char *long_op;
  int short_op;
  char *arg;    // like  "--option ARG"  in the help spew
  char *description;
};


EXPORT void cmdlopt_init(const char *usage_in,
        char * (*get_post_help_in)(void),
        struct opts *opts_in);

EXPORT int Main(int argc, char **argv);
