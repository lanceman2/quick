#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>

#include "../../include/debug.h"
#include "../../include/quickdraw.h"


void catcher(int sig) {
    ASSERT(0, "Caught signal %d", sig);
}


int main(void) {

    ASSERT(SIG_ERR != signal(SIGSEGV, catcher));

    const uint32_t w = 700;
    const uint32_t h = 1000;

    uint32_t *buffer = calloc(w * h, sizeof(*buffer));
    ASSERT(buffer, "calloc(%" PRIu32 ",%zu) failed",
            w * h, sizeof(*buffer));

    struct QdImage image = {
        .buffer = buffer,
        .width = w,
        .height = h,
        .stride = w
    };

    qd_paint(image, 0xFFFFFFFF);

    qd_rectangle(image,
            140.5F/*x*/, 100.0F/*y*/, 400.0F/*width*/, 800.5F/*height*/,
            0xFF00FF00/*color*/, 0);
    qd_rectangle(image,
            60.5F/*x*/, 500.0F/*y*/, 400.0F/*width*/, 200.5F/*height*/,
            0xFF0000FF/*color*/, 0);


    RET_ERROR(0 == qd_show_png(image,
                0/*program*/, true/*wait*/), 3);

    DZMEM(buffer, w * h * sizeof(uint32_t));
    free(buffer);

    return 0;
}
