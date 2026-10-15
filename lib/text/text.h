// This is the libtext.so private library header file.

#ifndef __TEXT_H__
#  error "include the library public header text.h first"
#endif

// TODO: This is not thread safe.


// How to get height of a font in Freetype2 (Not for individual glyph)
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
//See TEXT_HEIGHT_FROM_SAMPLE_DEFAULT in ../../include/text.h

// For testing the clipping code of glyphs that do not fit in the vertical
// space of an 'e'.  So 'j' should get it's bottom clipped, and maybe its'
// top; and 'A' will likely get its' top clipped.
//#define TEXT_YBOX_SAMPLE  "eo"


// The bigger question is how are FT_Face intended to be used in an
// optimal way?  Do we create one, draw with it, and than destroy it, just
// to have to create it again in the next widget redraw?  Or do we keep
// the object around after drawing with it, for the next redraw?
// Do we want to share FT_Face objects between widgets?
// 
// FT_Face looks like a pretty big structure:
// https://freetype.org/freetype2/docs/reference/ft2-face_creation.html
//
// Instead of spending large amount of time sifting through other peoples
// code we just take a stab at it with a simple single threaded code that
// shares some of the FT_Face objects between widgets when they have the
// same full path font.  We'll resize and load glyphs each time we draw a
// different piece of text.  Yes, I grant you for web page sized text,
// this will likely not scale well; but we just want this to work for our
// little widgets.  This simple approach may be more performant then more
// complex schemes that are made for large text pages; but I doubt that;
// I just don't know.

// This is some kind of "text" abstraction that is built from FT_face font
// stuff.  We care about presenting strings of text on pixels for the
// widgets abstraction, without the bloat that tend to come with existing
// text layout APIs.  Ya, we're make'n it up as we go...

// An object that is a wrapper of FreeType FT_Face with added
// text geometry layout info.
struct TxFace {

    FT_Face ftFace;

    char *fontPath; // Get fontPath using libfontconfig.so.
    size_t fontPath_len; // also key length for dict.

    // blank space (in pixels) added to in front of the first character
    // glyph and after the last.
    uint32_t hpad;
    // With the font this is the pixels Above and Below the baseline for
    // all characters in TEXT_HEIGHT_FROM_SAMPLE that we checked.  yAbove
    // is the maximum above the baseline.  yBelow is the maximum below the
    // baseline.  Note: yBelow is positive, so it's unsigned.  We did
    // not imagine a font that does not draw both above and below the
    // baseline.  If a font does not, this code may be broken.
    uint32_t yAbove, yBelow;

    uint32_t refCount;
};


extern FT_Library ft;

// A dictionary list of faces.  Keyed by full font path.
extern struct Dict *dict;

