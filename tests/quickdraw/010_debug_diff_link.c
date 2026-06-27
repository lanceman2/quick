#include <stdio.h>
#include <errno.h>

#define SPEW_LEVEL_DEBUG // Compiled in top spew level in this binary
#include "../../include/debug.h"
#include "../../include/quickdraw.h"


int main(void) {

    fprintf(stderr, "qd_line=%p\n", qd_line);
    DSPEW();
    INFO();
    errno = 0;
    NOTICE();
    WARN("Ya");
    errno = 0;
    ERROR("This is not really an error");

    db_set_spew_level(5); // debug = 5

    return 0;
}
