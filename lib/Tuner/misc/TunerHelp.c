#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../cmdloptHelp.h"
#include "../../../include/debug.h"


const char *usage =
"     Usage: Tuner OPTIONS\n"
"\n"
"Read mic and display the tone.\n"
"\n"
;

// The returned value must be free()ed.
//
char *get_post_help(void) {

    const char *fmt = "     The post help.\n";
    const size_t Len = strlen(fmt) + 1;
    char *str = malloc(Len);
    ASSERT(str, "malloc(%zu) failed", Len);
    snprintf(str, Len, fmt);
    return str;
}


struct opts opts[] = {

/*----------------------------------------------------------------------*/
    { "--help", 'h', 0,

        "Print this help and then exit."
    },
/*----------------------------------------------------------------------*/
    { "--usage", 'u', 0,

        "Print command line usage and then exit."
    },
/*----------------------------------------------------------------------*/
    { "--version", 'V', 0,

        "Print the Tuner program version and then exit."
    },
/*----------------------------------------------------------------------*/
    { 0,0,0,0 } // Null Terminator.
};



int main(int argc, char **argv) {

    cmdlopt_init(usage, get_post_help, opts);

    return Main(argc, argv);
}

