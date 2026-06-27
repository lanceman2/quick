// This is the libtext.so private library header file.

#ifndef __TEXT_H__
#  error "include the library public header text.h first"
#endif


// How to get height of font in Freetype2 (Not for individual glyph)
//
// We use these characters to get the extreme y positions for text that is
// laid out horizontally.  '^' as a negative y min because is does not
// show anything below the baseline.  Note this code assumes that there
// will be at least one character showing something above that baseline
// and at least one character showing something below the baseline.
// The y limits of these characters depends on the font.  In some fonts
// the underscore is at the lowest y value, but in others it is not.
//
// https://freetype.org/freetype2/docs
//
// https://stackoverflow.com/questions/50373457/how-to-get-height-of-font-in-freetype2
// https://stackoverflow.com/questions/5265655/freetype2-get-global-font-bounding-box-in-pixels
//
//
// TODO: This is not so good for non-ASCII (uni-code) font characters.
//
// Our solution:
//
// A list of characters that get inked high and low in y for horizontal
// layout.  We're assuming that we don't need to look at all the ASCII
// code.  Just the ones that go high or low.  We use them to find a Y-Min
// and a Y-Max.   We define yAbove as Y-max and yBelow as something like
// - Y-Min.
//
// '`' tends to go high
// '|' tends to go high and low
// '_' tends to go low
// 'j' tends to go low
//
// I don't see a reason to check all characters.  If they don't fit we
// still draw them with the top and/or bottom clipped, and that's not so
// bad especially considering it'll likely be a weird-ass character
// anyway.  If they get clipped we spew a WARNING.
//
#define TEXT_YBOX_SAMPLE  "`q_\"'{]ZQgjy^&19i|"

// For testing the clipping code of glyphs that do not fit in the vertical
// space of an 'e'.  So 'j' should get it's bottom clipped, and maybe its'
// top; and 'A' will likely get its' top clipped.
//#define TEXT_YBOX_SAMPLE  "eo"


// An object that is a wrapper of FreeType FT_Face with added
// text geometry layout info.
struct TxFace {

    FT_Face ftFace;

    char *fontPath; // Get fontPath using libfontconfig.so.

    // blank space (in pixels) added to in front of the first character
    // glyph and after the last.
    uint32_t hpad;
    // With the font this is the pixels Above and Below the baseline for
    // all characters in TEXT_YBOX_SAMPLE that we checked.  yAbove is the
    // maximum above the baseline.  yBelow is the maximum below the
    // baseline.  Note: yBelow is positive, so it's unsigned.  We did not
    // imagine a font that does not draw both above and below the
    // baseline.  If a font does not, this code may be broken.
    uint32_t yAbove, yBelow;

    // To hold a list of TxFace:
    struct TxFace *prev, *next;
};


extern FT_Library ft;

// A list of faces.
extern struct TxFace *firstFace, *lastFace;


