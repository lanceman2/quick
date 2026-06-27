#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../cmdloptHelp.h"
#include "../../../include/debug.h"


const char *usage =
"     Usage: quickplot [[FILE1 [FILE2] ...] OPTIONS\n"
"\n"
"An interactive static and dynamic 2D plotter.\n"
"\n"
"quickplot can be an interactive 2D static data viewer and at "
"the same time an interactive 2D dynamic data viewer, like a "
"software based oscilloscope.\n"
"\n"
"quickplot executes code after parsing each command line option "
"in the order that the options are given.\n"
"\n"
"In order to be able to read arbitrarily many data types quickplot loads "
"DSO (dynamic shared object) plugin module files and aids the user "
"in building plugin module files on-the-fly.  The older quickplot "
"command line option interface is now preserved as a default plugin "
"module file."
;

// The returned value must be free()ed.
//
char *get_post_help(void) {

    const char *fmt = "     The LIBDIR for this quickplot instance is %s."
        " So the C graph creation template files are in "
        "%s/quickplot/template/.";
    const size_t Len = strlen(fmt) + 2 * strlen(db_lib_dir) + 1;
    char *str = malloc(Len);
    ASSERT(str, "malloc(%zu) failed", Len);
    snprintf(str, Len, fmt, db_lib_dir, db_lib_dir);
    return str;
}


struct opts opts[] = {

/*----------------------------------------------------------------------*/
    { "--edit-compile-load", 'e', "[FILE]",

        "Edit, compile, and load a DSO (dynamic shared object) file.  "
        "Both the C file and DSO file will be saved.  "
        "If the file named qp_graph.so is found it is loaded, else if "
        "the file qp_graph.c is found it is edited, compiled, and "
        "loaded else the file is created, edited, compiled, and "
        "loaded.\n\n"
        "If the FILE argument is given that will be used in place of "
        "qp_graph.\n\n"
        "There are much less intrusive quickplot operational modes "
        "available which are accessible by other quickplot command-line "
        "options."
    },
/*----------------------------------------------------------------------*/
    { "--help", 'h', 0,

        "Print this help and then exit."
    },
/*----------------------------------------------------------------------*/
    { "--plugin", 'p', "P_FILE",

        "Load the graph creation plugin DSO file P_FILE."
    },
/*----------------------------------------------------------------------*/
    { "--run-c-file", 'c', "C_FILE",

        "Compile and run quickplot with a DSO graph plugin generated from "
        "C_FILE.  This will create a temporary quickplot graph creation "
        "plugin DSO file in the directory /tmp/; and ignore the current "
        "set plugin filename, P_FILE, and the current set template file, "
        "T_FILE.  The created temporary DSO file and "
        "directory will immediately removed just before the graph is "
        "displayed."
    },
/*----------------------------------------------------------------------*/
    { "--template", 't', "T_FILE",

        "Set the graph creation DSO template C file to T_FILE.  "
        "This template C file will be copied as the start of a new graph "
        "creation plugin DSO file.  This does not read the T_FILE, it "
        "just sets the T_FILE for future options."
        "\n\n"
        "The quickplot packaged template C files are in "
        "LIBDIR/quickplot/template/.  See LIBDIR below."
    },
/*----------------------------------------------------------------------*/
    { "--test-template", 'T', "[T_FILE]",

        "Compile and run quickplot with a DSO graph plugin generated from "
        "T_FILE.  T_FILE is searched for in the quickplot templates "
        "directory in LIBDIR/quickplot/template/ where LIBDIR is "
        "the installation library directory or lib/quickplot/template/ "
        "in the built source "
        "directory tree if it's running from a binary that is in a "
        "temporary build directory. "
        "This will create a temporary quickplot graph creation "
        "plugin DSO file in the directory /tmp/, and ignore the current "
        "set plugin filename, P_FILE.  The created temporary DSO file and "
        "directory will immediately removed just before the graph is "
        "displayed."
    },
/*----------------------------------------------------------------------*/
    { "--usage", 'u', 0,

        "Print command line usage and then exit."
    },
/*----------------------------------------------------------------------*/
    { "--version", 'V', 0,

        "Print the quickplot version and then exit."
    },
/*----------------------------------------------------------------------*/
    { 0,0,0,0 } // Null Terminator.
};



int main(int argc, char **argv) {

    cmdlopt_init(usage, get_post_help, opts);

    return Main(argc, argv);
}
