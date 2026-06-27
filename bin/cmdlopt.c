#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/wait.h>


#include "../include/debug.h"
#include "../lib/opts.h"
#include "cmdlopt.h"


// this does not return in the current process that calls this.
//
// opt = "-H" for --help and "-u" for --usage.
//
static void SpewHelp(int fd, const char *opt, const char *run) {

    // This usage, --help thing, is a little odd.  It launches another
    // program to display the program help and usage.  Why?  We put the
    // --help, man pages, together in one program that is also used to
    // generate some of the code that are in this program.  This keeps the
    // documentation of the program and it's options consistent by having
    // just one source file for this documentation.

    size_t len = strlen(db_lib_dir) + strlen(run) + 1;
    char *path = calloc(1, len);
    ASSERT(path, "calloc(1,%zu) failed", len);
    snprintf(path, len, "%s%s", db_lib_dir, run);

    if(access(path, R_OK | W_OK) != 0) {
        // TODO: We considered keeping on running after this failure, but
        // this is simpler and, we think, expected behavior.
        fprintf(stderr, "program %s was "
                "not found in \"%s\"\n",
                run, path);
        // If ever we could check for memory leaks, we cleanup.

        free(path);
        // FAIL
        exit(1);
    }

    if(fd != STDOUT_FILENO)
        dup2(fd, STDOUT_FILENO);

    pid_t pid = fork();

    if(pid == -1) {
        fprintf(stderr, "fork() failed\n");
        exit(1);
    }

    if(pid == 0) {
        // I am the child
        //
        // Valgrind says this (buf) leaks memory; but clearly it can't be
        // helped and does not matter; the OS cleans up for us.
        //
        // opt = "-H" for --help  and "-u" for --usage
        //
        execl(path, path, opt, NULL);
        fprintf(stderr, "execl(\"%s\",,) failed\n", path);
        exit(1); // non-zero error code, fail.
    }

    // I am the parent
    int statval = 1;
    wait(&statval);
    if(WIFEXITED(statval))
        exit(WEXITSTATUS(statval));
    fprintf(stderr, "Child did not terminate with exit\n");
    exit(1);
}

void help(const char *subPath) {
    SpewHelp(STDOUT_FILENO, "-H", subPath);
}

void usage(const char *subPath) {
    SpewHelp(STDOUT_FILENO, "-u", subPath);
}


// The returned value, c, is from the list in options from the
// generated file ../lib/cmdlopt/misc/programOptions.h
// like: 
//   struct opts options[] = {
//       { "add", 'a' },
//       { "block", 'b' },
//       { "help", 'h' },
//       { "foo", 125 },
//       ...
//       { 0, 0 }
//   };
//
// Returns &command as the long opt from in options[].
// Returns the short opt as an int:
//
//
// So for example if:
//
//            0     1  2  3     4  5       6   7   8
// argv[] = program -a -b stdin in --block foo bar -g
//  
//   with i=2 this will set command to point to the long opt
//   "block" and argv[i] points to "-b", and returns a 'b', the
//   short arg opt as an int.
//
int getOpt(int argc, const char * const *argv, int i,
        const struct opts *options/*array of options*/,
        const char **command) {

    *command = "*";

    const char *str = argv[i];


    if(str == argv[i]) {
        // This is the start of an arg string

        if(*str != '-')
            return 0; // Not an option.

        ++str;
        if(*str == '-') {
            ++str;
            // str points long arg to the
            // example:  --long-opt
            //             ^
            for(const struct opts *o=options; o->long_op; ++o) {

                size_t ll = strlen(o->long_op);
                if(strncmp(o->long_op, str, ll) == 0) {
                    if(str[ll] == '=') {
                        // example: --long-opt=ARG
                        //            ^
                        *command = o->long_op;
                        return o->short_op;
                    } else if(ll == strlen(str) && i < argc) {
                        // example: --long-opt ARG
                        //            ^
                        *command = o->long_op;
                        return o->short_op;
                    }
                }
            }
            return 0; // Not an option.
        }
        // Fall through case:
        //
        //  example:  -aclm
        //             ^
    }


    // arg points like for example:  -aclm
    //                                ^
    //                          or:   -aclm
    //                                   ^
    // So this must be a series of short options if they are options at
    // all.
    for(const struct opts *o=options; o->long_op; ++o) {
        if(o->short_op == *str) {
            if(*(str+1))
                // example:  -acl
                //            ^
                ;
            else if(i+1 < argc && *argv[i+1] != '-')
                // example:  -acl foo
                //              ^
                ++i;
            else {
                // example:  -acl -foo
                //              ^ 
                if(i < argc)
                    ++i;
            }

            *command = o->long_op;
            return o->short_op;
        }
    }

    return 0; // Not an option.
}
