#include <setjmp.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <png.h>

#include "../quickdraw_png.h"

#include "../../include/debug.h"


// Returns 0 on success
//
// This is one of those functions in that it is not so easy to test all
// it's failure modes.
//
int qd_save_png_fd(
        const uint32_t *image, uint32_t w, uint32_t h, uint32_t stride,
        int fd) {

    DASSERT(image);
    ASSERT(stride >= w);

    // We make a unique status for each failure mode.
    int status = 0; // success

    png_structp png_ptr = 0;
    png_infop info_ptr = 0;
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if(!png_ptr) {
        status = 1; // failure mode 1
        WARN("png_create_write_struct() failed");
        return status;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr) {
        status = 2; // failure mode 2
        WARN("png_create_info_struct() failed");
        goto after_dup_cleanup;
    }

    // The PNG API uses buffered file streams so we need to make a file
    // descriptor that we can fclose() with the stream without closing the
    // fd that was passed in.  We assume that the user just wants the file
    // appended with the PNG file data that we make here.  The common case
    // may be that the file (fd) does not have any other data to be
    // written to it, but we don't need to assume that here.
    //
    int dupFd = dup(fd);
    if(dupFd == -1) {
        status = 3;
        WARN("dup(%d) failed", fd);
        goto after_dup_cleanup;
    }

    FILE *file = fdopen(dupFd, "wb");
    if(!file) {
        status = 4;
        WARN("fdopen(%d,\"wb\") failed", fd);
        goto after_fdopen_cleanup;
    }


    // Set up error handling.
    //
    if(setjmp(png_jmpbuf(png_ptr))) {
        status = 5;
        // The libpng function calls will longjmp() to here.
        WARN("A libpng function got an error");
        goto after_png_cleanup;
    }

    // Set PNG image attributes.
    //
    // void -> no failing
    png_set_IHDR(png_ptr, info_ptr,
            w, h,
            8/*depth in bits*/,
            PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT);

    // We copy pixel colors from image ARGB to png buffer ABGR

    /* Initialize rows of PNG. */
    uint32_t **row_pointers = png_malloc(png_ptr,
            h*sizeof(*row_pointers));
    if(!row_pointers) {
        status = 6;
        WARN("png_malloc(,%zu) failed", h*sizeof(*row_pointers));
        goto after_png_malloc_cleanup;
    }

    // We'll add this to image at the end of each row.
    stride -= w;

    for(uint32_t y = 0; y < h; ++y) {
        uint32_t *row = png_malloc(png_ptr, sizeof(uint32_t) * w);
        row_pointers[y] = row;
        uint32_t *row_end = row + w;
        for(; row < row_end;) {
            // We cannot change the input image and so
            // we needed to copy from one buffer to another because
            // the blue byte and the red byte are switched:
            uint32_t color = *image++; // 0xAABBGGRR <- 0xAARRGGBB
            // We do not move the alpha and green bytes:
            *row++ = (color & 0xFF00FF00) // alpha & green
                    // We swap the blue and red bytes:
                    | ((color & 0x000000FF) << 16) // blue
                    | ((color & 0x00FF0000) >> 16);// red
        }
        if(stride)
            image += stride;
    }

    // Write the image data to file descriptor fd.
    //
    // I guess if any of these 3 png_*() functions fail they will
    // longjmp() to our setjmp() above.
    //
    png_init_io(png_ptr, file);
    png_set_rows(png_ptr, info_ptr, (uint8_t **) row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, 0);


    // Success, if we got here.

    // This function has a few failure modes.  How do we test that they
    // are all consistent?  Better look at it hard.

 after_png_cleanup:

    for(uint32_t y = 0; y < h; y++)
        png_free(png_ptr, (void *) row_pointers[y]);
    png_free(png_ptr, row_pointers);

 after_png_malloc_cleanup:

    fclose(file);

 after_fdopen_cleanup:

    if(!file)
        // if file was set we don't need to close dupFd
        close(dupFd);

 after_dup_cleanup:

    // I guess this also cleans up from png_create_info_struct() for the
    // case when png_create_info_struct() succeeded but dup() failed.
    //
    png_destroy_write_struct(&png_ptr, &info_ptr);

    return status;
}


int qd_load_png_fd(
        uint32_t *buf, uint32_t w, uint32_t h, uint32_t stride,
        int fd) {

    DSPEW();

    ASSERT("Write more code here");

    return 0;
}

