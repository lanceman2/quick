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
#define TEXT TEXT_YBOX_SAMPLE

int main(void) {

    srand(1);

    struct QdImage image;
    image.width = 1700;
    image.height = 800;
    image.stride = image.width;
    image.buffer = calloc(image.stride * sizeof(uint32_t), image.height);
    ASSERT(image.buffer, "calloc(%zu,%" PRIu32 ") failed",
            image.stride * sizeof(uint32_t), image.height);

    struct TxFace *textFace = tx_face_create(FONT_PATTERN, 120);

    ASSERT(textFace);

    uint32_t width, height;

    width = tx_face_get_text_width(textFace, TEXT);
    height = tx_face_get_text_height(textFace);


    ASSERT(width > 0);
    ASSERT(height > 0);

    fprintf(stderr, "text \"" TEXT "\" is width/height=%"
            PRIu32 "/%" PRIu32 " pixels\n", width, height);

    {
        // We draw text on a sub rectangle of the full image:
        struct QdImage textRec;
        textRec.stride = image.stride;
        textRec.width = width;
        textRec.height = height;
        textRec.buffer = image.buffer + 20 + textRec.stride * 20;
        // Don't overrun the buffer:
        ASSERT(textRec.buffer + textRec.height * textRec.stride
                <= image.buffer + image.height * image.stride);

        qd_paint(textRec, 0x9900FF0F);

        ASSERT(0 == tx_face_put(textFace, textRec, TEXT,
                    0xFF000000));

        // Another sub rectangle
        textRec.buffer += textRec.stride * 200;

        ASSERT(0 == tx_face_put(textFace, textRec, TEXT,
                    0x99FFFFFF));


        // Another sub rectangle
        textRec.buffer += textRec.stride * 200;

        tx_face_destroy(textFace);
        textFace = tx_face_create("Sans", 170);
        width = tx_face_get_text_width(textFace, TEXT);
        height = tx_face_get_text_height(textFace);
        textRec.width = width;
        textRec.height = height;
        qd_paint(textRec, 0xD92FAFFF);

        ASSERT(0 == tx_face_put(textFace, textRec, TEXT,
                    0x00FFFFFF));
    }

    ASSERT(0 == qd_show_png(image, 0, true/*do_wait*/));


    //tx_face_destroy(textFace);

    DSPEW();

    free(image.buffer);

    // tx_face_destroy() should be called in the libtext.so destructor as
    // many times as needed.

    return 0;
}
