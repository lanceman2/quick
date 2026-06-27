#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <wchar.h>

#include "../../include/debug.h"
#include "../../include/quickdraw.h"


int main(void) {


    struct QdImage image = {
        .width = 1000,
        .height = 800 };
    image.stride = image.width;


    image.buffer = calloc(image.width * image.height,
            sizeof(*image.buffer));
    ASSERT(image.buffer, "calloc(%" PRIu32 ",%zu) failed",
            image.width * image.height, sizeof(*image.buffer));

#if 1
    qd_line(image,
            100.0F/*x0*/, 200.0F/*y0*/,
            image.width*0.8/*x1*/, image.height*0.7/*y1*/,
            0xFF00FF00/*color*/, 100.3/*width*/, 0);

    qd_line(image,
            image.width*0.9/*x0*/, 200.0F/*y0*/,
            100.0/*x1*/, image.height*0.7/*y1*/,
            0xFF0000FF/*color*/, 100.3/*width*/, 0);
#endif

    RET_ERROR(0 == qd_show_png(image, 0/*program*/, true/*wait*/), 3);

    DZMEM(image.buffer, image.width *
            image.height * sizeof(*image.buffer));
    free(image.buffer);

    return 0;
}
