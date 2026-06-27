#include <stdlib.h>
#include <stdio.h>
#include <fontconfig/fontconfig.h>

#include "../../include/debug.h"


int main(int argc, const char **argv) {

    FcBool ret = FcInit(); // Initialize Fontconfig
    ASSERT(ret);
    FcConfig *config = FcInitLoadConfigAndFonts();
    ASSERT(config);
    FcPattern *pat = FcPatternCreate();
    ASSERT(pat);

#ifdef PATTERN
    ret = FcPatternAddString(pat, FC_FAMILY, (const FcChar8 *)"Mono");
    ASSERT(ret);
    ret = FcPatternAddDouble(pat, FC_SIZE, 50);
    ASSERT(ret);
#endif

    ret = FcConfigSubstitute(config, pat, FcMatchPattern);
    ASSERT(ret);
    FcDefaultSubstitute(pat);

    // find the "default" font
    FcResult result;
    FcPattern* font = FcFontMatch(config, pat, &result);
    ASSERT(result == FcResultMatch);
    ASSERT(font);

    if(font) {
        FcChar8* file = NULL;
        if(FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch)
            fprintf(stderr, " ------------------Found Font file: %s\n", (char*)file);
        // Looks like we need to copy file to our own buffer if we wish to
        // keep it.
        FcPatternDestroy(font);
    }

    FcPatternDestroy(pat);
    FcConfigDestroy(config);
    FcFini();

    return 0;
}

