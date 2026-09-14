#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <freetype2/ft2build.h>
#include <freetype/freetype.h>


#define SPEW_LEVEL_INFO
#include "../../include/debug.h"
#include "../../include/quickdraw.h"
#include "../../include/text.h"

#include "../../lib/text/text.h"

#include "../panels/rand.h"


#define FONT_PATTERN  "Mono"

//#define TEXT "Hello|-World! .^gj"

// This makes TEXT be the string of characters that we use to get the Y
// extent of the font; so we should see at least one character hit the top
// and at least one character hit the bottom.
// Defined in ../../lib/text/text.h
#define TEXT TEXT_HEIGHT_FROM_SAMPLE_DEFAULT

int main(void) {

    srand(1);
    uint32_t size = 141;

    struct QdImage image;
    image.width = 1700;
    image.height = 800;
    image.stride = image.width;
    image.buffer = calloc(image.stride * sizeof(uint32_t), image.height);
    ASSERT(image.buffer, "calloc(%zu,%" PRIu32 ") failed",
            image.stride * sizeof(uint32_t), image.height);
    struct TxFace *textFace = 0;

    uint32_t width, height;


    textFace = tx_face_create(FONT_PATTERN, size);

    ASSERT(textFace);

    width = tx_face_get_text_width(textFace, TEXT);
    height = tx_face_get_text_height(textFace);

    ASSERT(width > 0);
    ASSERT(height > 0);

    fprintf(stderr, "text \"" TEXT "\" is width/height=%"
           PRIu32 "/%" PRIu32 " pixels\n", width, height);

    // Part of the image:
    struct QdImage rec;
    rec.width = image.width;
    rec.height = size;
    rec.stride = image.stride;
    rec.buffer = image.buffer + 200 * image.stride;
 
    qd_paint(rec, 0xD92FAFFF);
    
    tx_face_put(textFace, rec, TEXT, 0xFF000000);

    ASSERT(0 == qd_show_png(image, 0, true/*do_wait*/));


    //tx_face_destroy(textFace);

    DSPEW();

    free(image.buffer);

    // tx_face_destroy() should be (is) called in the libtext.so
    // destructor as many times as needed.

    return 0;
}
