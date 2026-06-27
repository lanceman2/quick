#include <stdlib.h>

#include "../../include/debug.h"

int main(void) {

    void *ptr = malloc(1);

    ERROR("This is supposed to fail when run with valigrind ptr=%p", ptr);

    // We are testing that valgrind catches a simple missing free().
    //
    if(getenv("VALGRIND")) {
        fprintf(stderr, "\nRunning with valgrind.\n\n");
        // Running with valgrind and we want to not free it.
        return 0;
    }

    fprintf(stderr, "\nNot running with valgrind.\n\n");
    free(ptr);
    // This test is expected to fail so:
    return 1;
}
