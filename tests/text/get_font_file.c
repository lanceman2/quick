#include <stdio.h>
#include <stdlib.h>


#define SPEW_LEVEL_INFO
#include "../../include/debug.h"
#include "../../include/quickdraw.h"
#include "../../include/text.h"


#define FONT_PATTERN  "Mono"

int main(void) {

    char *fontPath = tx_get_font_file(FONT_PATTERN);

    ASSERT(fontPath);

    fprintf(stderr, "tx_get_font_file(\"%s\")=\"%s\"\n",
            FONT_PATTERN, fontPath);
    DSPEW();

    free(fontPath);

    return 0;
}
