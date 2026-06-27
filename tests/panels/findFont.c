#include <stdio.h>
#include <stdlib.h>

#include "../../include/debug.h"
#include "../../include/panels.h"

#define FONT  "Mono"

int main(int argc, const char **argv) {

    const char *font = FONT;

    if(argc > 1 && argv[1][0])
        font = argv[0];

    char *path = pnFindFont(font);

    ASSERT(path);

    fprintf(stderr, "pnFindFont(\"%s\")=%s\n", FONT, path);

    free(path);

    return 0;
}

