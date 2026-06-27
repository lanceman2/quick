#include <inttypes.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "../../include/debug.h"

#include "../include/quickdraw.h"

// Reference:
// https://www.realtimerendering.com/blog/the-center-of-the-pixel-is-0-50-5/
//
// We model the floating point points (show as *) values as having no
// fractional value at the left upper edge of a pixel.  Like so:
/*

    width, height = 4, 3


       *---------*---------*---------*---------*
       |         |         |         |         |
       |         |         |         |         |
       |         |         |         |         |
       |         |         |         |         |
       *---------*---------*---------*---------*
       |         |         |         |         |
       |         |         |         |         |
       |         |         |         |         |
       |         |         |         |         |
       *---------*---------*---------*---------*
       |         |         |         |         |
       |         |         |         |         |
       |         |         |         |         |
       |         |         |         |         |
       *---------*---------*---------*---------*

*/
// So the point 0,0 is on the left top edge, and the center of the left
// top most pixel is at x,y = 0.5,0.5 .  The right bottom most pixel
// center is at x,y = width-0.5,height-0.5, where width is the number of
// pixels across and height is the number of pixels up and down.
//
// quote: OpenGL has always considered the fraction (0.5,0.5) the pixel
// center.


static const float SmallWidth = 1.0/256.0;
static const float SmallFrac  = 0.001;

static inline float Alpha(uint32_t color) {
    return (color >> 24);
}
static inline float Red(uint32_t color) {
    return ((color >> 16) & 0xFF);
}
static inline float Green(uint32_t color) {
    return ((color >> 8) & 0xFF);
}
static inline float Blue(uint32_t color) {
    return (color & 0xFF);
}

// I'll bet that this pixel blending could more optimally be a more
// complex function, not just this linear function.  It likely depends on
// the computer monitor (or whatever the display device is).
//
static uint32_t BlendColor(float frac, uint32_t old, uint32_t new) {

    float oneMFrac = 1.0 - frac;

    uint32_t a = Alpha(new) * frac + Alpha(old) * oneMFrac + 0.5;
    uint32_t r =   Red(new) * frac +   Red(old) * oneMFrac + 0.5;
    uint32_t g = Green(new) * frac + Green(old) * oneMFrac + 0.5;
    uint32_t b =  Blue(new) * frac +  Blue(old) * oneMFrac + 0.5;

//ERROR("a=0x%X r=0x%X g=0x%X b=0x%X frac=%f", a, r, g, b, frac);

    return (a << 24) | (r << 16) | (g << 8) | b;
}


uint32_t qd_blend_colors(uint32_t colorA, uint32_t colorB,
        float fracA) {
    ASSERT(fracA <= 1.0);

    return BlendColor(fracA, colorB, colorA);
}

// Color the entire image surface with one color.
//
void qd_paint(struct QdImage image, uint32_t color) {

    // FIXME: this needs a complete rewrite using a faster method.
    // Does pixman do this kind of thing?

    uint32_t *end = image.buffer + image.height * image.stride;

    for(; image.buffer < end; image.buffer += image.stride)
        for(uint32_t *pix = image.buffer + image.width - 1;
                pix >= image.buffer; --pix)
            *pix = color;
}


// This just draws a rectangle with feathered (anti-aliased) left and
// right side; which happens to be the same as thick vertical line without
// end caps at the top and bottom.
//
// Returns true if x and y values are out side the image.
//
static
bool Line_Vertical(struct QdImage image,
        float x, float y0, float y1,
        float width, uint32_t color,
        uint32_t *left_pix, uint32_t *right_pix,
        float *left_colorFrac, float *right_colorFrac) {

    DASSERT(image.width);
    DASSERT(image.height);
    DASSERT(image.stride >= image.width);
    DASSERT(left_pix);
    DASSERT(right_pix);
    DASSERT(left_colorFrac);
    DASSERT(right_colorFrac);


    if(width <= SmallWidth) return false;

    if(y0 > y1) {
        // swap y0 and y1
        float y = y0;
        y0 = y1;
        y1 = y;
    }

    float w2 = width/2;

    // Is the line outside the image in x direction?
    if(x <= - w2) return true;
    else if(x >= image.width + w2) return true;

    // Constrain y values:
    if(y0 < -w2) y0 = -w2;
    else if(y0 >= image.height + w2) return true;
    if(y1 > image.height + w2) y1 = image.height + w2;
    if(y1 - y0 <= SmallWidth) return true;
    else if(y1 <= -w2) return true;


    int32_t yStart = ceilf(y0);
    if(yStart < 0) yStart = 0;
    int32_t yEnd = floorf(y1);
    if(yEnd > image.height) yEnd = image.height;

    //WARN("yStart=%" PRIu32 " yEnd=%" PRIi32, yStart, yEnd);

    // FIXME: Add drawing of the ends of the line.
    if(yStart == yEnd) return false;


    // We define the line as a thin rectangle with sides that are
    // anti-aliased color for left and right sides, with the
    // standard end caps.

    // Is the line outside the image in x direction?
    if(x <= - w2) return false;
    else if(x >= image.width + w2) return false;


    errno = 0;
    // The amount of color we change to the applied color for the left
    // most pixel as a number between 0 and 1.
    float leftColorFrac = 1.0 - (x - w2) + floorf(x - w2);
    if(x - w2 <= 0.0)
        leftColorFrac = 1.0;

    // The amount of color we change to the applied color for the right
    // most pixel as a number between 0 and 1.
    float rightColorFrac = (x + w2) - floorf(x + w2);
    if(x + w2 >= image.width - 1)
        rightColorFrac = 1.0;

    // x bounds
    int32_t leftPix = floorf(x - w2);
    if(leftPix < 0) {
        leftPix = 0;
    } else if(leftPix >= image.width)
        // Nothing visible
        return false;

    if(leftColorFrac <= SmallFrac) {
        // 
        ++leftPix;
        leftColorFrac = 1.0;
    }

    int32_t rightPix = floorf(x + w2);
    if(rightPix >= image.width) {
        rightPix = image.width - 1;
    } else if(rightPix < 0)
        // Nothing visible
        return false;

    if(leftPix == rightPix) {
        // Reuse variable leftColorFrac
        DASSERT(leftPix == 0 || leftPix == image.width - 1 || width <= 1.0);
        if(leftPix == 0) {
            DASSERT(x + w2 < 1.0);
            if(x - w2 <= 0) {
                DASSERT(x + w2 <= 1.0);
                leftColorFrac = x + w2;
            } else {
                DASSERT(width <= 1.0);
                leftColorFrac = width;
            }
        } else if(leftPix == image.width - 1) {
            if(x + w2 < image.width) {
                DASSERT(width <= 1.0);
                leftColorFrac = width;
            } else
                leftColorFrac = image.width - (x - w2);
        } else {
            DASSERT(width <= 1.0);
            leftColorFrac = width;
        }
    }

    if(rightColorFrac <= SmallFrac) {
        if(leftPix != rightPix)
            --rightPix;
        rightColorFrac = 1.0;
    }

    //WARN("leftPix=%" PRIi32 " rightPix=%" PRIi32 "  %" PRIi32 " pixels wide",
    //        leftPix, rightPix, rightPix - leftPix + 1);
    //WARN("leftColorFrac=%f  rightColorFrac=%f  ", leftColorFrac, rightColorFrac);


    if(leftPix > rightPix)
        // Can this happen?
        return false;

    if(errno) {
        // Is errno ERANGE?
        ERROR("OVERFLOW exception");
        return false;
    }

    // Dumb-ass checks:
    DASSERT(leftPix >= 0);
    DASSERT(rightPix >= 0);
    DASSERT(leftPix < image.width);
    DASSERT(rightPix < image.width);


    *left_pix = leftPix;
    *right_pix = rightPix;
    *left_colorFrac = leftColorFrac;
    *right_colorFrac = rightColorFrac;


    uint32_t *pix = image.buffer + leftPix + yStart * image.stride;
    uint32_t *end = pix + (yEnd - yStart) * image.stride;

    image.stride -= (rightPix - leftPix + 1);

    for(;pix < end; pix += image.stride) {

        // Now the left most pixel
        //
        uint32_t bgColor = *pix;
        *pix++ = BlendColor(leftColorFrac, bgColor, color);

        if(leftPix == rightPix)
            continue;

        // All the pixels in between get the color of the line.
        for(uint32_t i = leftPix + 1; i < rightPix; ++i)
            *pix++ = color; // 0xAARRGGBB

        // Now the right most pixel
        //
        bgColor = *pix;
        *pix++ = BlendColor(rightColorFrac, bgColor, color);
    }

    DASSERT(pix == end);

    return false;
}


void qd_rectangle(struct QdImage image,
        float x, float y, float width, float height,
        uint32_t color, struct QdLineState *st) {

    DASSERT(image.width);
    DASSERT(image.height);
    DASSERT(image.stride >= image.width);

    uint32_t leftPix, rightPix;
    float leftColorFrac, rightColorFrac = 100.0;

    // This gets the pixels in between the top and bottom, and along with
    // the left and right sides.
    if(Line_Vertical(image, x + width/2.0, y, y + height, width, color,
                &leftPix, &rightPix, &leftColorFrac, &rightColorFrac))
        return;

    if(rightColorFrac > 1.0) {
        // Case where nothing is drawn yet.  The rectangle must be very
        // small.
        // FIXME: Add code here.
        return;
    }

    DASSERT(leftPix >= 0);
    DASSERT(rightPix >= leftPix);


 
    if(y > 0.0 && y < image.height) {
        uint32_t yStart = floorf(y);
        DASSERT(yStart >= 0);
        DASSERT(yStart < image.height);
        float frac = y - yStart;
        DASSERT(frac <= 1.0);

        if(frac > SmallFrac) {

            // Now add the top pixel row:

            uint32_t *pix = image.buffer + leftPix + yStart * image.stride;

            uint32_t bgColor = *pix;
            float edgeFrac = (frac + leftColorFrac)/2.0;
            *pix++ = BlendColor(edgeFrac, bgColor, color);

            for(uint32_t i = leftPix + 1; i < rightPix; ++i) {
                bgColor = *pix;
                *pix++ = BlendColor(frac, bgColor, color);
            }

            if(leftPix != rightPix) {
                bgColor = *pix;
                edgeFrac = (frac + rightColorFrac)/2.0;
                *pix++ = BlendColor(edgeFrac, bgColor, color);
            }
        }
    }

    if(y + height < image.height) {
        uint32_t yStart = floorf(y + height);
        DASSERT(yStart >= 0);
        DASSERT(yStart < image.height);
        float frac = y + height - yStart;
        DASSERT(frac <= 1.0);
 
        if(frac > SmallFrac) {
            // Now add the top pixel row:

            uint32_t *pix = image.buffer + leftPix + yStart * image.stride;

            uint32_t bgColor = *pix;
            float edgeFrac = (frac + leftColorFrac)/2.0;
            *pix++ = BlendColor(edgeFrac, bgColor, color);

            for(uint32_t i = leftPix + 1; i < rightPix; ++i) {
                bgColor = *pix;
                *pix++ = BlendColor(frac, bgColor, color);
            }

            if(leftPix != rightPix) {
                bgColor = *pix;
                edgeFrac = (frac + rightColorFrac)/2.0;
                *pix++ = BlendColor(edgeFrac, bgColor, color);
            }
        }
    }
}

// draw a vertical line with a cap on the top (TODO) and bottom that is
// half the line width, "width".
//
void qd_line_vertical(struct QdImage image,
        float x, float y0, float y1,
        float width, uint32_t color, struct QdLineState *st) {

    uint32_t leftPix, rightPix;
    float leftColorFrac, rightColorFrac;

    Line_Vertical(image, x, y0, y1, width, color,
            &leftPix, &rightPix, &leftColorFrac, &rightColorFrac);
}


void qd_line_horizontal(struct QdImage image,
        float x0, float x1, float y,
        float width, uint32_t color, struct QdLineState *st) {
}


static inline void DrawFlatLine(struct QdImage image,
        float x0, float y0, float x1, float y1,
        float dx, float dy,
        float width2/*half width*/, uint32_t color) {

    DASSERT(x0 < x1);
    DASSERT(y0 < y1);

}

static inline void DrawSteepLine(struct QdImage image,
        float x0, float y0, float x1, float y1,
        float dx, float dy,
        float width2/*half width*/, uint32_t color) {

    DASSERT(x0 < x1);
    DASSERT(y0 < y1);

}


void qd_line(struct QdImage image,
        float x0, float y0, float x1, float y1,
        float width, uint32_t color, struct QdLineState *st) {

    DASSERT(image.buffer);
    DASSERT(image.width <= image.stride);

    if(x0 > x1) {
        float x = x0;
        x0 = x1;
        x1 = x;
    }
    if(y0 > y1) {
        float y = y0;
        y0 = y1;
        y1 = y;
    }

    // Half width is easier to work with.
    width /= 2.0;

    if(x1 < - width || x0 > image.width + width || y1 < - width || y0 > image.height + width)
        // Trying to draw outside of image.
        return;

    float dx = x1 - x0;
    float dy = y1 - y0;
    float slope = dy/dx;

    // Move points to positions that are not far from the image.
    //
    // We need a width2 edge buffer of space added, so that we don't miss
    // any line end pixels.
    //
    if(x0 < -width) {
        // Move x0 to left edge.
        y0 = (-width - x0) * slope + y0;
        x0 = -width;
    }
    if(y0 < -width) {
        // Move y0 to top.
        x0 = (-width - y0) * dx / dy + x0;
        y0 = -width;
    }
    if(x1 > image.width + width) {
        // Move x0 to right edge.
        y1 = (image.width + width - x0) * slope + y0;
        x1 = image.width + width;
    }
    if(y1 < image.height + width) {
        // Move y0 to bottom.
        x1 = (image.height + width - y0) * dx / dy + x0;
        y1 = image.height + width;
    }


    if(dx <  SmallWidth) {
        qd_line_vertical(image, 0.5F*(x0 + x1), y0, y1, 2.0F*width, color, st);
        return;
    }
    if(dy < SmallWidth) {
        qd_line_horizontal(image, x0, x1, 0.5F*(y0 + y1), 2.0F*width, color, st);
        return;
    }


    if(dx > dy)
        DrawFlatLine(image, x0, y0, x1, y1, dx, dy, width, color);
    else
        DrawSteepLine(image, x0, y0, x1, y1, dx, dy, width, color);
}

// Reference: https://www.desmos.com/calculator/ebdtbxgbq0
//
void qd_bezier_cubic(struct QdImage image,
        float x0, float y0, float x1, float y1,
        float x2, float y2, float x3, float y3,
        float width, uint32_t color, struct QdLineState *st) {


}

void qd_arc(struct QdImage image,
        float cx, float cy, float radius, float a0, float a1,
        float width, uint32_t color, struct QdLineState *st) {

}
