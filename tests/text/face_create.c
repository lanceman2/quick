#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#define SPEW_LEVEL_INFO
#include "../../include/debug.h"
#include "../../include/quickdraw.h"
#include "../../include/text.h"


#define FONT_PATTERN  "Mono"
#define TEXT "Hello - World! ."

int main(void) {

    // Mostly for Valgrind testing.  Pretty stupid otherwise.

    struct TxFace *textFace = tx_face_create(FONT_PATTERN, 30.0);

    tx_face_create(FONT_PATTERN, 30.0);
    textFace = tx_face_create(FONT_PATTERN, 30.0);
    tx_face_create(FONT_PATTERN, 30.0);
    tx_face_destroy(textFace);
    tx_face_create(FONT_PATTERN, 30.0);
    textFace = tx_face_create(FONT_PATTERN, 30.0);
    tx_face_create(FONT_PATTERN, 30.0);


    tx_face_destroy(textFace);

    DSPEW();

    // tx_face_destroy() should be called in the libtext.so destructor
    // as many times as needed.

    return 0;
}
