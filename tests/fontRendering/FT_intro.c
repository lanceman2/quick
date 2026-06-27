// Reference: https://kevinboone.me/fbtextdemo.html

#include <stdlib.h>
#include <stdio.h>
#include <fontconfig/fontconfig.h>
#include <freetype2/ft2build.h>
#include <freetype/freetype.h>

#include "../../include/debug.h"


char *GetFontfile(const char *fontPattern) {

    char *fontFile = 0;
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

    FcChar8* file = NULL;
    if(FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
        fprintf(stderr, "Found Font file: %s\n", (char*)file);
        // Looks like we need to copy file to our own buffer if we wish to
        // keep it.
        fontFile = strdup((char *) file);
    }
    FcPatternDestroy(font);
    FcPatternDestroy(pat);
    FcConfigDestroy(config);
    FcFini();

    return fontFile;
}



int main(int argc, const char **argv) {

    char *fontFile = GetFontfile("Mono");

    ASSERT(fontFile);


    FT_Library ft;

    FT_Error e = FT_Init_FreeType(&ft);
    ASSERT(e == 0, "FT_Init_FreeType() failed: %s", FT_Error_String(e));

    FT_Face face;
    e = FT_New_Face(ft, fontFile, 0, &face);
    ASSERT(e == 0, "FT_New_Face(,\"%s\",,) failed: %s", fontFile, FT_Error_String(e));

    const int size = 30; // let the library work out the width from the height
    e = FT_Set_Pixel_Sizes(face, 0/*height*/, size/*width size in pixels*/);
    ASSERT(e == 0, "FT_Set_Pixel_Sizes(,,%d) failed: %s", size, FT_Error_String(e));

    // Unicode code points between 32 and 127 are essentially the same as
    // ASCII, so ASCII codes can be used directly.
    FT_ULong character = 'Q';
    FT_UInt gi = FT_Get_Char_Index(face, character);
    ASSERT(gi, "FT_Get_Char_Index(face, character) is 0");

    e = FT_Load_Glyph(face, gi, FT_LOAD_DEFAULT);
    ASSERT(e == 0, "FT_Load_Glyph() failed: %s", FT_Error_String(e));

    int bbox_ymax = face->bbox.yMax / 64;
    int glyph_width = face->glyph->metrics.width / 64;
    int advance = face->glyph->metrics.horiAdvance / 64;
    int x_off = (advance - glyph_width) / 2;
    int y_off = bbox_ymax - face->glyph->metrics.horiBearingY / 64;

    ERROR("bbox_ymax=%d glyph_width=%d advance=%d x_off=%d y_off=%d",
            bbox_ymax, glyph_width, advance, x_off, y_off);



    e = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
    ASSERT(e == 0, "FT_Render_Glyph() failed");

    e = FT_Done_Face(face);
    ASSERT(e == 0, "FT_Done_Face() failed: %s", FT_Error_String(e));

    e = FT_Done_FreeType(ft);
    ASSERT(e == 0, "FT_Done_FreeType() failed: %s", FT_Error_String(e));

    free(fontFile);

    return 0;
}

