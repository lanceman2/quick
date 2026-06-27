
#include "../include/debug.h"
#include "../include/quickdraw.h"


static void __attribute__ ((destructor)) destructor(void);

static void destructor(void) {

    // This destructor don't leak system resources when this library is
    // unloaded.

    qd_unload_libpng();
    DSPEW();
}

