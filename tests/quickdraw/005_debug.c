#include <stdio.h>

#define SPEW_LEVEL_INFO
#include "../../include/debug.h"
#include "../../include/quickdraw.h"


int main(void) {

    fprintf(stderr, "qd_line=%p\n", qd_line);
    DSPEW();
    INFO();
    NOTICE();
    WARN("Ya");
    ERROR("This is not really an error");

    return 0;
}
