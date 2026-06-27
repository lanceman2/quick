#ifndef __TEXT_H__
#define __TEXT_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <quickdraw.h>

#ifdef text_BUILD_LIB
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


// An object that is a wrapper of FreeType Face with added
// text geometry layout info.
struct TxFace;


// Returned value must be freed.
EXPORT char *tx_get_font_file(const char *fontPattern);

// If fontPattern begins with "/" than that is the font file path
// else we get the font file from libfontconfig FcPattern stuff.
EXPORT struct TxFace *tx_face_create(const char *fontPattern,
        uint32_t size/*width in pixels*/);

EXPORT const char *tx_face_get_font_file(const struct TxFace *face);

EXPORT uint32_t tx_face_get_text_height(struct TxFace *face);
EXPORT uint32_t tx_face_get_text_width(struct TxFace *face,
        const char *text);

EXPORT int tx_face_put(const struct TxFace *face,
        struct QdImage image,
        const char *text,
        uint32_t fg_color);

EXPORT void tx_face_destroy(struct TxFace *face);


#ifdef __cplusplus
}
#endif

#undef EXPORT

#endif // #ifndef __TEXT_H__
