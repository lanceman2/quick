
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <dlfcn.h>
#include <cairo/cairo.h>
#include <fontconfig/fontconfig.h>

#include "../../include/debug.h"

static
void catcher(int sig) {

    ASSERT(0, "caught signal number %d", sig);
}

#define SYMBOL "runLabels"

#ifndef RUN
#  define DSO_FILE "./tests/panels/libfontconfig.so"
#else
// The interactive version.
#  define DSO_FILE "./tests/panels/libfontconfig_int.so"
#endif

void LoadAndRun(void) {

    void *dlhandle = dlopen(DSO_FILE, RTLD_NOW);
    ASSERT(dlhandle, "dlopen(\"%s\",) failed", DSO_FILE);
    fprintf(stderr, "Loaded DSO file \"%s\"\n", DSO_FILE);
    int (*runLabels)(void) = dlsym(dlhandle, SYMBOL);
    ASSERT(runLabels, "dlsym(%p,\"%s\") failed: %s", dlhandle, SYMBOL, dlerror());
    ASSERT(runLabels() == 0);
    ASSERT(0 == dlclose(dlhandle));
}

int main(void) {

    ASSERT(SIG_ERR != signal(SIGSEGV, catcher));

    for(int i=0; i<3; ++i) {
        LoadAndRun();
        cairo_debug_reset_static_data();
        FcFini();
        cairo_debug_reset_static_data();
        FcFini();
        cairo_debug_reset_static_data();
        FcFini();
      }

    return 0;
}
