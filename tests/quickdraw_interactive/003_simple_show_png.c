// This test tests that we manage the pixel memory correctly when we
// draw on a rectangular subsection of the image.

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <signal.h>

#include "../../include/debug.h"
#include "../../include/quickdraw.h"


/*
// Looks like the default behavior, not setting up a SIGCHLD catcher,
// works fine.  I'd imagine there could be times, when there are
// other child processes, that that is not the case.
//
static void child_catcher(int sig_num) {
    NOTICE("caught signal %d", sig_num);
    // FIXME: Do we need a non-blocking wait call after this
    // returns or can we call wait in the signal catcher.
}
*/


int main(void) {

    // We will draw on an inner rectangular portion of the image buffer.
    // The rest of the image buffer will be one color.
    //
    // Note: his is most certainly not optimized code.  Too many extra
    // variables being defined.  But, it's easy to follow.
    //
    // I think ASSERT() is okay here; since the error case would be very
    // bad and cause memory over-run.  Just so we can change the numbers
    // without having to think too much.

    const uint32_t buf_width = 900;
    const uint32_t buf_height = 700;
    const uint32_t width = buf_width - 100;
    ASSERT(buf_width >= width);
    const uint32_t height = buf_height - 100;
    ASSERT(buf_height > height);
    const uint32_t x0 = (buf_width - width)/2;
    const uint32_t y0 = (buf_height - height)/2;
    ASSERT(x0 + width <= buf_width);
    ASSERT(y0 + height <= buf_height);

    // Make an image as a uint32_t array using A R G B format
    // one byte per color type.
    //
    // calloc() zeros all the memory in the image.
    //
    uint32_t *image = calloc(buf_width * buf_height, sizeof(*image));
    ASSERT(image, "calloc(%" PRIu32 ",%zu) failed",
            buf_width * buf_height, sizeof(*image));

    struct QdImage im = {
        .buffer = image,
        .width = buf_width,
        .height = buf_height,
        .stride = buf_width };

    // Note: the (X direction) stride is the buffer width.
    //
    //  The stride is buf_width.
    //
    uint32_t stride_dif = buf_width - width;

    uint32_t *pix; // "pix" is a pixel iterator
    // end_pix is at the lower right end of the inner rectangle.
    const uint32_t *end_pix = image + x0 + width + buf_width * (height + y0);

    for(pix = image + x0 + y0 * buf_width; pix < end_pix; pix += stride_dif)
        for(uint32_t *pix_xend = pix + width; pix < pix_xend; ++pix)
            *pix = 0xAF3F00FF; // ARGB color

    // We should have gone to the end pixel of the image plus x0, bringing
    // it to the next row if there was one.
    //
    RET_ERROR(pix == end_pix + stride_dif, 1);

    RET_ERROR(0 == qd_load_libpng(), 2);

    //ASSERT(SIG_ERR != signal(SIGCHLD, child_catcher));
    //Default disposition: SIGCHLD is ignored (SIG_DFL = ignore).

    RET_ERROR(0 == qd_show_png(im, 0/*program*/, false/*wait*/), 3);

#if 1  // To make more:

    RET_ERROR(0 == qd_show_png(im, "display", false/*wait*/), 4);
    //
    // Move to a different starting point and different size.
    //
    const uint32_t x = x0 + 5;
    const uint32_t y = y0 + 5;

    im.buffer += x + y * buf_width;
    im.width = width;
    im.height = height;

    RET_ERROR(0 == qd_show_png(im, 0/*program*/, true/*wait*/), 5);

#endif
    RET_ERROR(0 == qd_unload_libpng(), 6);

    DZMEM(image, buf_width * buf_height * sizeof(uint32_t));
    free(image);

    return 0;
}
