// This reference got me started but gets many details wrong,
// like ftFace->bbox.yMin and ftFace->bbox.yMax are not usable
// unless you understand the font scaling magic which it not
// easy to understand, and is not explained there-in.
//
// ref: https://kevinboone.me/img/freetype_font_metrics.png
//
//
// ref: https://freetype.org/freetype2/docs/tutorial/step2.html


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <fontconfig/fontconfig.h>
#include <freetype2/ft2build.h>
#include <freetype/freetype.h>

#include "../../include/debug.h"
#include "../../include/quickdraw.h"
#include "../../include/text.h"

#include "text.h"

#define FAIL(to, fmt, ...) \
    do {\
        ERROR(fmt, ##__VA_ARGS__);\
        goto to;\
    } while(0)


FT_Library ft = 0;

struct TxFace *firstFace = 0, *lastFace = 0;


// Returned value must be freed.
//
// Returns the full file path to a TrueType Font data file or 0 on error.
//
// This returns a file name path as a string pointer that must be
// free(3)ed.
//
// The question here is should we be keeping around some of the
// intermediate libfontconfig state data?  If this is just called once in
// a program process than the answer is most likely, No.  Maybe it will be
// called 3 times, and in that case the answer could still be, no.
//
// TODO: libfontconfig.so uses signed char for strings, so should I be
// checking and setting the all the sign (8th) bits to zero?  I always
// wondered why the sign bit was ignored in strings.  ASCII is a 7 bit
// code.  So, what is the 8th bit supposed to be?  Many 8-bit codes
// (e.g., ISO 8859-1) contain ASCII as their lower half.
//
char *tx_get_font_file(const char *fontPattern) {

    RET_ERROR(fontPattern, 0, "slFindFont(fontPattern=0) "
            "failed fontPattern can't be 0");

    FcConfig *config;
    FcPattern *pat;
    FcPattern *font;

    RET_ERROR(FcInit(), 0, "FcInit() failed");

    config = FcInitLoadConfigAndFonts();
    if(!config)
        FAIL(config, "FcInitLoadConfigAndFonts() failed");

    // Looks like this gets at least a default font.
    pat = FcNameParse((unsigned char *) fontPattern);
    if(!pat)
        FAIL(pat, "FcNameParse(\"%s\") failed", fontPattern);

    FcBool ret = FcConfigSubstitute(config, pat, FcMatchPattern);
    if(!ret)
        FAIL(pat, "FcConfigSubstitute() failed");

    ret = FcPatternAddString(pat, FC_FILE, (const FcChar8 *)"*.ttf");
    if(!ret)
        FAIL(pat, "FcPatternAddString() failed");

    FcDefaultSubstitute(pat);

    FcResult result;
    font = FcFontMatch(config, pat, &result);
    if(!font)
        FAIL(font, "FcFontMatch() failed");

    char *path = 0;
    if(FcPatternGetString(font, FC_FILE, 0,
                (unsigned char **) &path) != FcResultMatch) {
        DASSERT(path == 0);
        FAIL(all, "FcPatternGetString() failed");
    }

    // FIXME: Not portable '/'
    if(!path || path[0] != '/') {
        path = 0;
        FAIL(all, "Failed to get full path");
    }

    DASSERT(path);
    path = strdup(path);
    ASSERT(path, "strdup() failed");

    // Cleanup in reverse order on your way out of this function:
all:
    FcPatternDestroy(font);
font:
    FcPatternDestroy(pat);
pat:
    FcConfigDestroy(config);
config:
    FcFini();

    return path;
}


const char *tx_face_get_font_file(const struct TxFace *face) {

    DASSERT(face);
    DASSERT(face->fontPath);
    // FIXME: Not portable '/'.
    DASSERT(face->fontPath[0] == '/');

    return face->fontPath;
}

static inline void
FT_getHeight(FT_Face ftFace, char character, long *above, long *below) {

    FT_UInt gi = FT_Get_Char_Index(ftFace, character);
    ASSERT(gi, "FT_Get_Char_Index(, character) is 0");

    FT_Error e = FT_Load_Glyph(ftFace, gi, FT_LOAD_DEFAULT);
    ASSERT(e == 0, "FT_Load_Glyph() failed: %s", FT_Error_String(e));

    // FIXME: I only have a vague idea why we use the 64 here.

    DASSERT(ftFace->glyph->metrics.height >= 64);


    *above = ftFace->glyph->metrics.horiBearingY;

    *below = ftFace->glyph->metrics.height -
        ftFace->glyph->metrics.horiBearingY;
}

// Returns how much horizontal space is needed (added) in the layout of
// this character?
//
//
static inline uint32_t FT_getWidth(FT_Face ftFace, FT_ULong character) {

    FT_UInt gi = FT_Get_Char_Index(ftFace, character);
    ASSERT(gi, "FT_Get_Char_Index(, character) is 0");

    FT_Error e = FT_Load_Glyph(ftFace, gi, FT_LOAD_DEFAULT);
    ASSERT(e == 0, "FT_Load_Glyph() failed: %s", FT_Error_String(e));

    // FIXME: I don't know exactly why we use the 64 here.

    DASSERT(ftFace->glyph->metrics.horiAdvance >= 64);

    uint32_t advance = ftFace->glyph->metrics.horiAdvance / 64;
    // FIXME: What to do with fractions of a pixel?
    if(ftFace->glyph->metrics.horiAdvance % 64)
        ++advance;

    //DSPEW("character=%c advance=%" PRIu32, (char ) character, advance);

    return advance;
}



// If fontPattern begins with "/" than that is the font file path else we
// get the font file from libfontconfig FcPattern stuff.
//
// FIXME: We need to get more sophisticated with sizing of fonts.  Giving
// height in pixels does not work well for low res and hi res monitors
// (varying monitor resolutions).
//
// TODO: 
//
struct TxFace *tx_face_create(const char *fontPattern,
        uint32_t size/*in pixels*/) {

    char *fontPath;
    FT_Error e;

    if(fontPattern && fontPattern[0] == '/') {
        fontPath = strdup(fontPattern);
        ASSERT(fontPath, "strdup() failed");
    } else
        // This will ERROR spew if it fails.
        fontPath = tx_get_font_file(fontPattern);

    if(!fontPath) return 0;

    if(!firstFace) {
        // We are starting with the first face object, so we
        // need a free type, FT, thingy.
        DASSERT(!firstFace);
        DASSERT(!lastFace);
        DASSERT(ft == 0);
        e = FT_Init_FreeType(&ft);
        if(e) {
            ERROR("FT_Init_FreeType(() failed: \"%s\"",
                    FT_Error_String(e));
            DASSERT(ft == 0); // ??
            free(fontPath);
            return 0;
        }
        DASSERT(ft);
    }

    FT_Face ftFace;
    e = FT_New_Face(ft, fontPath, 0, &ftFace);
    if(e != 0) {
        ERROR("FT_New_Face(,\"%s\",,) failed: %s", 
                fontPath, FT_Error_String(e));
        if(!firstFace) {
            // FIXME: check error return.
            FT_Done_FreeType(ft);
            ft = 0;
        }
        free(fontPath);
        return 0;
    }

    // Tests show that FT_Set_Pixel_Sizes() does not change
    // ftFace->bbox.yMin and ftFace->bbox.yMax so unless you
    // understand font scaling they are useless.

    if(size) // We'll set the width
        e = FT_Set_Pixel_Sizes(ftFace, size/*width*/, 0/*height*/);
    if(e != 0) {
        ERROR("FT_Set_Char_Size(,size=%" PRIu32 ") failed: %s", 
                size, FT_Error_String(e));
        FT_Done_Face(ftFace);
        if(!firstFace) {
            // FIXME: check error return.
            FT_Done_FreeType(ft);
            ft = 0;
        }
        free(fontPath);
        return 0;
    }


    struct TxFace *face;
    face = calloc(1, sizeof(*face));
    ASSERT(face, "calloc(1,%zu) failed", sizeof(*face));


    // Find a good height.  I think it's be the scaled version of
    // ftFace->bbox.yMin and ftFace->bbox.yMax but we just do not know how
    // to get that, so we'll take the max and mins of a few particular
    // characters, and use the maximum height found as the limiting height
    // of the whole font face.

    static_assert(sizeof(FT_Pos) == sizeof(long));

    {
        const char *s = TEXT_YBOX_SAMPLE;

        for(; *s; ++s) {
            long above, below;
            FT_getHeight(ftFace, (FT_ULong) *s, &above, &below);

            //WARN("  char=%c  above/below = %ld/%ld", *s,
            //above/64, below/64);

            if(above % 64)
                above = above / 64 + 1;
            else
                above /= 64;
            if(below % 64)
                below = below / 64 + 1;
            else
                below /= 64;
            if(face->yAbove < above)
                face->yAbove = above;
            if(face->yBelow < below)
                face->yBelow = below;

            uint32_t w;
            w = FT_getWidth(ftFace, (FT_UInt)(*s));
            if(face->hpad < w)
                face->hpad = w;
        }
    }

    if(face->hpad >= 2)
        face->hpad /= 2;
    else
        face->hpad = 1;

    DSPEW("Y extent=%" PRIu32 " Above/Below = %" PRIu32 "/%" PRIu32,
            face->yAbove + face->yBelow, face->yAbove, face->yBelow);

#if 0
    // FIXME: This does not work.  It's too big for a reasonable view of
    // the common characters.
    //
    // https://stackoverflow.com/questions/50373457/how-to-get-height-of-font-in-freetype2

    // Looks like it drops the remainder??
    int bbox_ymax = FT_MulFix(ftFace->bbox.yMax, ftFace->size->metrics.y_scale) >> 6;
    int bbox_ymin = FT_MulFix(ftFace->bbox.yMin, ftFace->size->metrics.y_scale) >> 6;

    DSPEW("bbox_ymax=%d  bbox_ymin=%d", bbox_ymax, bbox_ymin);
#endif


    // Add face to the list of faces as the last one:
    //////////////////////////////////////////////////////
    if(firstFace) {
        DASSERT(lastFace);
        lastFace->next = face;
        face->prev = lastFace;
    } else {
        DASSERT(!lastFace);
        firstFace = face;
    }
    lastFace = face;

    face->fontPath = fontPath;
    face->ftFace = ftFace;

    return face;
}


uint32_t tx_face_get_text_height(struct TxFace *face) {

    return face->yAbove + face->yBelow;
}

uint32_t tx_face_get_text_width(struct TxFace *face,
        // FIXME: What is this text for UNICODE?
        const char *text) {

    DASSERT(face);
    DASSERT(firstFace);
    DASSERT(lastFace);
    DASSERT(ft);
    FT_Face ftFace = face->ftFace;
    DASSERT(ftFace);


    uint32_t width = 0;
    const char *s = text;

    for(; *s; ++s) {
        uint32_t w;
        w = FT_getWidth(ftFace, (FT_UInt)(*s));
        if(w == 0) break; // fail

        // For horizontal layout:
        width += w;

        if(face->hpad < w)
            face->hpad = w;
    }
    if(face->hpad >= 2)
        face->hpad /= 2;
    else
        // We want at least 1 pixel padding to either side.
        face->hpad = 1;


    if(*s)
        return 0; // failure.

    // FIXME: Why is this not
    // width + 2 * face->hpad
    //
    return width + face->hpad;
}


static int spew_count = 0;

int tx_face_put(const struct TxFace *face,
        struct QdImage image,
        const char *text,
        uint32_t color) {

    DASSERT(face);
    DASSERT(text);
    DASSERT(*text);
    DASSERT(image.buffer);
    DASSERT(image.width);
    DASSERT(image.height);
    DASSERT(image.stride >= image.width);

    FT_Face ftFace = face->ftFace;
    DASSERT(ftFace);

    uint32_t *upperLeft_ptr = image.buffer + face->hpad;
    uint32_t *end = upperLeft_ptr + image.width + face->hpad;


    for(;*text && upperLeft_ptr <= end; ++text) {

        FT_UInt gi = FT_Get_Char_Index(ftFace, *text);
        ASSERT(gi, "FT_Get_Char_Index(, character) is 0");

        FT_Error e = FT_Load_Glyph(ftFace, gi, FT_LOAD_DEFAULT);
        ASSERT(e == 0, "FT_Load_Glyph() failed: %s", FT_Error_String(e));

        uint32_t advance = ftFace->glyph->metrics.horiAdvance / 64;
        // FIXME: What to do with fractions of a pixel?
        if(ftFace->glyph->metrics.horiAdvance % 64)
            ++advance;

        e = FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL);
        ASSERT(e == 0, "FT_Render_Glyph() \'%c\' failed", *text);

        // The face glyph bitmap looks like a rectangular section of a
        // QdImage, but with each pixel the size of 1 byte (not unint32_t
        // 4 bytes).  We need to copy it from a 1 byte color to a 4 byte
        // color.
        int height = ftFace->glyph->bitmap.rows;
        int width = ftFace->glyph->bitmap.width;
        // I guess pitch is like the stride of the glyph bitmap.
        int pitch = ftFace->glyph->bitmap.pitch;
        unsigned char *bufIn = ftFace->glyph->bitmap.buffer;

        // I wish I understood the magic of 64.  Where does it
        // come from.
        int y = 0;
        int y_shift = face->yAbove - ftFace->glyph->metrics.horiBearingY/64;
        if(y_shift < 0) {
            // face->yAbove is too small.  See TEXT_YBOX_SAMPLE in
            // lib/text/text.h.
            if(++spew_count < 4) {
                WARN("The character '%c' is %d higher than %"
                        PRIu32 ".  The top of haracter '%c' will be clipped",
                        *text, -y_shift, face->yAbove, *text);
            }
            // We will draw -y_shift cut off the top of this font glyph.
            y -= y_shift;
        }
        if(height + y_shift > image.height) {
            // face->yBelow is too small. See TEXT_YBOX_SAMPLE in
            // lib/text/text.h.
            if(++spew_count < 4) {
                WARN("The character '%c' is %d lower than %"
                        PRIu32 ".  The bottom of character '%c' will be clipped",
                        *text, height + y_shift - image.height,
                        image.height, *text);
            }
            // We will draw cut off the bottom of this font glyph.
            height = image.height - y_shift;
        }

        for(; y < height; ++y)
            for(int x = 0; x < width; ++x) {
                // We move down in the image to were the font bitmap has
                // some color showing, y_shift.
                //
                uint32_t *pix = upperLeft_ptr + x + 
                    (y + y_shift)*image.stride;
                uint32_t bgColor = *pix;
                *pix = qd_blend_colors(color, bgColor,
                        bufIn[x + y*pitch] / 255.0/*fraction of color*/);
            }
        upperLeft_ptr += advance;
    }

    return 0;
}


void tx_face_destroy(struct TxFace *face) {

    DASSERT(face);
    DASSERT(ft);
    DASSERT(firstFace);
    DASSERT(lastFace);
    DASSERT(face->fontPath);
    DASSERT(face->fontPath[0] == '/');
    DASSERT(face->ftFace);


    DZMEM(face->fontPath, strlen(face->fontPath));
    free(face->fontPath);

    // Remove face from the faces list.
    ////////////////////////////////////////////
    if(face->next) {
        DASSERT(face != lastFace);
        face->next->prev = face->prev;
    } else {
        DASSERT(face == lastFace);
        lastFace = face->prev;
    }
    if(face->prev) {
        DASSERT(face != firstFace);
        face->prev->next = face->next;
    } else {
        DASSERT(face == firstFace);
        firstFace = face->next;
    }


    FT_Error e = FT_Done_Face(face->ftFace);
    if(e)
        WARN("FT_Done_Face() failed: \"%s\"", FT_Error_String(e));

    // Now finish the FreeType thingy, ft.
    if(!firstFace) {
        DASSERT(!lastFace);
        DASSERT(ft);
        FT_Error e = FT_Done_FreeType(ft);
        if(e)
            // FIXME: What can we do if this fails?
            ERROR("FT_Done_FreeType() failed: \"%s\"", FT_Error_String(e));
        ft = 0;
    }

    DZMEM(face, sizeof(*face));
    free(face);
}

