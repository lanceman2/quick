#include <stdio.h>

#define SPEW_LEVEL_INFO
#include "../../include/debug.h"
#include "../../include/quickdraw.h"
#include "../../include/text.h"


int main(void) {

    fprintf(stderr, "tx_get_font_file=%p\n", tx_get_font_file);
    DSPEW();
    INFO();
    NOTICE();
    WARN("Ya");
    ERROR("This is not really an error");

    return 0;
}
