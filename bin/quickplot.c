/*
  We are rewriting the program "quickplot" which is from the Debian package quickplot.
  The last (rewrite) version of quickplot broke with the switch to using the Wayland
  windowing system. This code is not usable on a X11 based desktop.
*/

#include <signal.h>
#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "../include/panels.h"
#include "../include/debug.h"
#include "../include/quickplot.h"
#include "../lib/opts.h"

#include "quickplotOptions.h"
#include "cmdlopt.h"
#include "findGrapher.h"


// A container for graphs.
struct PnWidget *graphContainer = 0;
struct PnWidget *win = 0; // The window.


// Return false on success.
//
static inline void Window(void) {

    win = pnWindow_create(0, 3, 3,
            0/*x*/, 0/*y*/, PnLayout_TB/*layout*/, 0,
            PnExpand_HV);
    ASSERT(win);
    pnWidget_setBackgroundColor(win, 0xFF0000F0, 0);
    pnWindow_setPreferredSize(win, 1100, 900);

    // TODO: add more container widgets
    graphContainer = win;
}

static inline void GetGraphContainer(void) {

    if(!win)
        Window();
    DASSERT(win);
    DASSERT(graphContainer);
}


// Return false on success.
//
static inline
bool AddGraph(const char *dsoFile, const char *templateFile) {

    GetGraphContainer();

    int ret = findGrapher(dsoFile, templateFile, graphContainer);
    if(ret < 0)
        return true; // error
    else if(ret)
        return false; // success

    return true; // error
}


static void catcher(int signum) {
    ASSERT(0, "Caught signal %d\n", signum);
}

static const char *templateFile = 0;
static const char *dsoFile = 0;

// Returns program error status, and can set exitOnError flag that is
// passed to it.  exitOnError is only set here, but it's not used here.
//
// c is the command to switch on.
//
// command is the command as a long option string like "help" for --help.
//
// argc is the number of arg strings and argv[] is the strings.
//
int RunCommand(int c, int argc, const char *command,
        const char * const * argv, bool *exitOnError) {
    DASSERT(argc >= 1);
    DASSERT(command);
    DASSERT(c);
    DASSERT(argv);
    DASSERT(argv[0]);
    DASSERT(exitOnError);

#ifdef DEBUG
    fprintf(stderr, "Got command line option: "
            "\"%s[%c:%d]\":  ", command, c, c);
    for(int i=0; i<argc; ++i)
        fprintf(stderr, " %s", argv[i]);
    fprintf(stderr, "\n");
#endif

    switch(c) {

        case EDIT_COMPILE_LOAD:
            if(argc >= 2)
                dsoFile = argv[1];
            if(AddGraph(dsoFile, templateFile))
                return 2;
            break;
        case TEMPLATE:
            if(argc < 2) {
                ERROR("--%s Missing option argument FILE",
                        command);
                return 3; // error
            }
            templateFile = argv[1];
            break;
        case PLUGIN:
            if(argc < 2) {
                ERROR("--%s Missing option argument P_FILE",
                        command);
                return 4; // error
            } else
                dsoFile = argv[1];
            break;
        case HELP:
            help("/quickplot/misc/quickplotHelp"); // Does not return.
        case RUN_C_FILE:
            {
                if(argc < 2) {
                    ERROR("--%s Missing option argument T_FILE",
                            command);
                    return 3; // error
                }
                DASSERT(argv[1]);
                DASSERT(*argv[1]);
                if(!win) {
                    DASSERT(!graphContainer);
                    Window();
                }
                return testCFile(argv[1], graphContainer);
            }
        case TEST_TEMPLATE:
            if(argc < 2 && !templateFile) {
                ERROR("--%s Missing option argument T_FILE",
                        command);
                return 3; // error
            } else if(argc >= 2)
                templateFile = argv[1];

            if(!win) {
                DASSERT(!graphContainer);
                Window();
            }
            return testTemplate(templateFile, graphContainer);
        case USAGE:
            usage("/quickplot/misc/quickplotHelp"); // Does not return.
        case VERSION:
            printf("%s\n", QUICKPLOT_VERSION);
            exit(0);
        default:
    }

    return 0; // 0 --> success
}



int main(int argc, const char * const *argv) {

    // Hang the program for debugging, if we segfault.
    ASSERT(signal(SIGSEGV, catcher) != SIG_ERR);
    ASSERT(signal(SIGABRT, catcher) != SIG_ERR);

    int exitStatus = 0;
    bool exitOnError = true;
    int i = 1;

    while(i < argc) {

        const char *command = 0;

        //
        // Returns &command as the long opt from in options[].
        // Returns the short opt as an int:
        //
        int c = getOpt(argc, argv, i, options, &command);

        if(!c) {
            fprintf(stderr, "Error: unknown option at argv[%d]: %s\n",
                    i, argv[i]);
            if(exitOnError) {
                exitStatus = 1;
                break;
            }
            ++i;
            continue;
        }
        DASSERT(command);

        // GetNumArgs() is more particular to this program
        // than getOpt(), so that's why getOpt() does not set numArgs.
        //
        // Now we know that we have an option argument from the struct
        // opts option[] thingy, though more particular testing will be
        // done before we can run part of this program.
        //
        int numArgs = GetNumArgs(i, argc, argv, command);

        if((exitStatus = RunCommand(c, numArgs+1,
                        command, argv + i, &exitOnError))) {
            if(exitStatus && exitOnError)
                break;
            // else keep going.
        }

        ++i;
        i += numArgs;
    }

    if(exitStatus)
        return exitStatus;

    if(!win) {
        // No window was made, so lets try.
        if(AddGraph(dsoFile, templateFile))
            return 3; // error
    }

    if(win) {
        pnWindow_show(win);
        pnDisplay_run();
        cleanupGrapher();
    }

    return exitStatus;
}
