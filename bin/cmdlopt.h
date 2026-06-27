
// This does not return.
extern void help(const char *subPath);

// This does not return.
extern void usage(const char *subPath);

// See comments in cmdlopt.c
//
extern int getOpt(int argc, const char * const *argv, int i,
        const struct opts *options/*array of options*/,
        const char **command);

// How many additional arguments are given past the i-th argv[i]
// until hitting the next - or -- option argument argv[ ? ].
// We have no need to look at argv[0] which is the program.
//
// Example:
//   argc = 11
//   argv[ 0   1   2     3     4    5            6   7   8  9  10 11
//  $ program -i --block stdin in --configure-mk MK foo bar 3 MK  -g
//
// Note: programs that can take a starting "-" in an argument option
// need code to be added to do that case.
//
//   i=1  returns 0
//   i=2  returns 2
//   i=5  returns 5
//
static inline
int GetNumArgs(int i, int argc, const char * const *argv,
        const char *command) {
    DASSERT(i);
    DASSERT(argv);
    // We need command to do special cases for command line options that
    // can have an argument like "-99" with a starting minus sign; and
    // -99 is an argument and not a new starting option argument.
    DASSERT(command);

    int numArgs = 0;

    const char * const *arg = argv + i + 1;
    // argv is null terminated.
    for(; *arg && arg[0][0] != '-'; ++arg)
        ++numArgs;
    return numArgs;
}
