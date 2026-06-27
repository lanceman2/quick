#include <stdint.h>
#include <stdbool.h>


#ifdef quickdraw_png_BUILD_LIB
// This is being compiled into a library.
// FIXME: This is not very portable code:
#  define EXPORT __attribute__((visibility("default"))) extern
//#  define EXPORT extern
#else
// The library API user is using this including this header file.
#  define EXPORT extern
#endif


EXPORT int qd_save_png_fd(
        const uint32_t *buf, uint32_t w, uint32_t h, uint32_t stride,
        int fd);

EXPORT int qd_load_png_fd(
        uint32_t *buf, uint32_t w, uint32_t h, uint32_t stride,
        int fd);


#undef EXPORT
