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

    image.buffer = calloc(image.width * image.height, sizeof(*image.buffer));
    ASSERT(image.buffer, "calloc(%" PRIu32 ",%zu) failed",
            image.width * image.height, sizeof(*image.buffer));

    qd_paint(image, 0xFFFFFF00);

    float w = 10.3F;

#if 1
    for(float x = 0.0; x < image.width + 100; x += 40.3)
        qd_line_vertical(image,
                x/*x*/, 0.0F/*y0*/, 700.0F/*y1*/,
                w/*width*/, 0xFF00FFFF/*color*/, 0);
    for(float x = -4.0; x < image.width + 100; x += 41.3)
        qd_line_vertical(image,
                x/*x*/, 0.0F/*y0*/, 700.0F/*y1*/,
                w+3.0/*width*/, 0xFFFF00FF/*color*/, 0);

    qd_line_vertical(image,
            1000/*x*/, 0.0F/*y0*/, 700.0F/*y1*/,
            w/*width*/, 0xFFFFFF00/*color*/, 0);

#endif
#if 1
    RET_ERROR(0 == qd_show_png(image, 0/*program*/, true/*wait*/), 3);
#endif
    DZMEM(image.buffer, image.width * image.height * sizeof(uint32_t));
    free(image.buffer);

    return 0;
}
