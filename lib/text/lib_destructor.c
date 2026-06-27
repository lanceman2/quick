#include <freetype2/ft2build.h>
#include <freetype/freetype.h>

#include "../include/debug.h"
#include "../include/quickdraw.h"
#include "../include/text.h"

#include "text.h"


static void __attribute__ ((destructor)) destructor(void);

static void destructor(void) {

    // This destructor don't leak system resources when this library is
    // unloaded.  This may seem silly to some people but it at least helps
    // test the code.  Works with ValGrind of example.
    //
    // This can help for the case when a sloppy depending library is
    // unloaded (assuming this is the last linker/loader reference
    // count).

    while(firstFace) tx_face_destroy(firstFace);
    DASSERT(!lastFace);

    DSPEW();
}

