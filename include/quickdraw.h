#ifndef __QUICKDRAW_H__
#define __QUICKDRAW_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

//#include <debug.h>

#ifdef quickdraw_BUILD_LIB
// This is being compiled into a (software project) library.
// FIXME: This is not very portable code:
#  define EXPORT __attribute__((visibility("default"))) extern
#else
// The library API user is using this including this header file.
#  define EXPORT extern
#endif


#ifdef __cplusplus
extern "C" {
#endif


struct QdLineState {
    bool shit;
};


static_assert(sizeof(uint32_t) == 4);


struct QdImage {
    uint32_t *buffer;
    uint32_t width, height,
        // stride is measured in 4 bytes which is sizeof(uint32_t) so that
        // the number of bytes between rows in the image buffer is stride
        // * 4.  If your iterator is a uint32_t pointer, you don't need
        // the 4.
        stride; // stride is the number of 32 bit lengths between rows.
};


EXPORT void qd_paint(struct QdImage image, uint32_t color);

EXPORT void qd_rectangle(struct QdImage image,
        float x, float y, float width, float height,
        uint32_t color, struct QdLineState *st);

EXPORT void qd_line_vertical(struct QdImage image,
        float x, float y0, float y1,
        float width, uint32_t color, struct QdLineState *st);

EXPORT void qd_line_horizontal(struct QdImage image,
        float x0, float x1, float y,
        float width, uint32_t color, struct QdLineState *st);

EXPORT void qd_line(struct QdImage image,
        float x0f, float y0f, float x1f, float y1f,
        float width, uint32_t color, struct QdLineState *st);

EXPORT void qd_bezier_cubic(struct QdImage image,
        float x0, float y0, float x1, float y1,
        float x2, float y2, float x3, float y3,
        float width, uint32_t color, struct QdLineState *st);

EXPORT void qd_arc(struct QdImage image,
        float cx, float cy, float radius, float a0, float a1,
        float width, uint32_t color, struct QdLineState *st);


EXPORT int qd_save_png(struct QdImage image, const char *path);

EXPORT int qd_load_png(struct QdImage image, const char *path);

EXPORT int qd_save_png_fd(struct QdImage image, int fd);

EXPORT int qd_load_png_fd(struct QdImage image, int fd);

// This fork()s and the child dynamically links with libpng.so indirectly
// through libquickdraw_png and then runs ImageMagicK's program "display".
EXPORT int qd_show_png(struct QdImage image,
        const char *show_program, bool do_wait);


// Returns 0 on success
EXPORT int qd_load_libpng(void);

EXPORT int qd_unload_libpng(void);


EXPORT uint32_t qd_blend_colors(uint32_t colorA, uint32_t colorB,
        float fracA);


#ifdef __cplusplus
}
#endif

#undef EXPORT

#endif // #ifndef __QUICKDRAW_H__
