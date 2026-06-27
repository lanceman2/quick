// This loads two functions from another library.  We just didn't want
// link to libpng.so (and it's dependences), given these two functions may
// not get used most of the time.  Making it seamless to the API user was
// an interesting way to do this.

#define _GNU_SOURCE
#include <link.h>

#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>

#include "../include/debug.h"
#include "../include/quickdraw.h"


// NOTICE: This is thread safe.

static void *dl_handle = 0;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// mutex Protects: dl_handle, save_png_fd, load_png_fd


// The two functions we will load from libquickdraw_png.so:
//
static int (*save_png_fd)(
        const uint32_t *buf, uint32_t w, uint32_t h, uint32_t stride,
        int fd) = 0;
//
static int (*load_png_fd)(
        uint32_t *buf, uint32_t w, uint32_t h, uint32_t stride,
        int fd) = 0;


static inline void *GetSym(const char *name) {

    DASSERT(dl_handle);
    dlerror();
    void *sym = dlsym(dl_handle, name);
    if(!sym)
        ERROR("dlsym(%p,\"%s\") failed: %s", dl_handle, name, dlerror());
    return sym;
}


// Returns 0 on success
//
static int Load_quickdraw_png(void) {

    // We assume that the library (libquickdraw_png.so) is in the same
    // directory as libdebug.so, which this library was linked with and
    // built with in the same software package.  In a sense libdebug.so is
    // a helper utility library that many of the libraries in this
    // software package link with.  If you move these libraries relative
    // to each other, you're fucked anyway; it's likely any program linked
    // with these libraries will fail to start up.

    if(dl_handle) {
        // It's all or nothing so:
        DASSERT(save_png_fd);
        DASSERT(load_png_fd);
        return 0;
    }

    // It's all or nothing so:
    DASSERT(!save_png_fd);
    DASSERT(!load_png_fd);

    int ret = 1; // start with fail.

    // FIXME: Not portable: "/"
    const char *lib = "/libquickdraw_png.so";
    // db_lib_dir is a globel from libdebug.so.
    size_t libPath_LEN = strlen(db_lib_dir) + strlen(lib) + 1;
    // Maybe we could use stack memory?
    char *libdebugPath = calloc(libPath_LEN, 1);
    ASSERT(libdebugPath, "calloc(%zu,1) failed", libPath_LEN);
    strcpy(libdebugPath, db_lib_dir);
    strcpy(libdebugPath + strlen(db_lib_dir), lib);

    DSPEW("libdebugPath=\"%s\"", libdebugPath);

    dlerror();
    dl_handle = dlopen(libdebugPath, RTLD_NOW|RTLD_LOCAL);
    if(!dl_handle) {
        ERROR("dlopen(\"%s\",RTLD_NOW|RTLD_LOCAL) failed: %s",
                libdebugPath, dlerror());
        goto cleanup;
    }

    if(!(save_png_fd = GetSym("qd_save_png_fd")))
        goto cleanup;

    if(!(load_png_fd = GetSym("qd_load_png_fd")))
        goto cleanup;

    ret = 0; // success

cleanup:

    DZMEM(libdebugPath, strlen(libdebugPath));
    free(libdebugPath);

    if(ret) {
        // Failed.  It's all or nothing; so:
        if(dl_handle) {
            dlclose(dl_handle);
            dl_handle = 0;
            save_png_fd = 0;
            load_png_fd = 0;
        } else {
            DASSERT(!save_png_fd);
            DASSERT(!load_png_fd);
        }
    }

    return ret;
}


int qd_unload_libpng(void) {

    /////////////////////////////////////////////////////
    CHECK(pthread_mutex_lock(&mutex));
    /////////////////////////////////////////////////////
    int status = 0;

    if(dl_handle) {
DSPEW();
        if(dlclose(dl_handle)) {
            // It's nice to know if it fails, but what
            // does a user do if it fails?
            WARN("dlclose(%p) failed: %s", dl_handle, dlerror());
            status = 1;
        }
        dl_handle = 0;
    }
    save_png_fd = 0;
    load_png_fd = 0;

    /////////////////////////////////////////////////////
    CHECK(pthread_mutex_unlock(&mutex));
    /////////////////////////////////////////////////////
    return status;
}

int qd_load_libpng(void) {

    /////////////////////////////////////////////////////
    CHECK(pthread_mutex_lock(&mutex));
    /////////////////////////////////////////////////////

    int ret = Load_quickdraw_png();

    /////////////////////////////////////////////////////
    CHECK(pthread_mutex_unlock(&mutex));
    /////////////////////////////////////////////////////

    return ret;
}

// Returns 0 on success
//
int qd_save_png_fd(struct QdImage image, int fd) {

    DASSERT(fd >= 0);

    /////////////////////////////////////////////////////
    CHECK(pthread_mutex_lock(&mutex));
    /////////////////////////////////////////////////////

    int ret = Load_quickdraw_png();

    if(!ret) {
        DASSERT(dl_handle);
        DASSERT(load_png_fd);
        DASSERT(save_png_fd);

        ret = save_png_fd(image.buffer,
                image.width, image.height, image.stride, fd);
    }

    /////////////////////////////////////////////////////
    CHECK(pthread_mutex_unlock(&mutex));
    /////////////////////////////////////////////////////

    return ret;
}

// Returns 0 on success
//
int qd_save_png(struct QdImage image, const char *path) {

    DASSERT(path);
    DASSERT(path[0]);

    int fd = open(path, O_CREAT|O_TRUNC|O_WRONLY);
    if(fd == -1) {
        WARN("open(\"%s\",) failed", path);
        return 1;
    }
    int ret = qd_save_png_fd(image, fd);
    close(fd);
    return ret;
}


// Returns 0 on success
//
int qd_load_png_fd(struct QdImage image, int fd) {

    /////////////////////////////////////////////////////
    CHECK(pthread_mutex_lock(&mutex));
    /////////////////////////////////////////////////////

    int ret = Load_quickdraw_png();

    if(!ret) {
        DASSERT(dl_handle);
        DASSERT(load_png_fd);
        DASSERT(save_png_fd);

        ret = load_png_fd(image.buffer,
                image.width, image.height, image.stride, fd);
    }

    /////////////////////////////////////////////////////
    CHECK(pthread_mutex_unlock(&mutex));
    /////////////////////////////////////////////////////

    return ret;
}

// Returns 0 on success
//
int qd_load_png(struct QdImage image, const char *path) {

    DASSERT(path);
    DASSERT(path[0]);

    int fd = open(path, O_RDONLY);
    if(fd == -1) {
        WARN("open(\"%s\",O_RDONLY) failed", path);
        return 1;
    }
    int ret = qd_load_png_fd(image, fd);
    close(fd);
    return ret;
}

// Returns 0 on success
//
int qd_show_png(struct QdImage image,
        const char *show_program, bool do_wait) {

    int fds[2];

    int status = 0;

    RET_WARN(0 == pipe(fds), 1, "pipe() failed");

    pid_t pid = fork();

    if(pid == -1) {
        status = 11;
        WARN("fork() failed");
        goto finish;
    }

    if(pid == 0) {

        // I'm the child process and I read

        if(!show_program)
            // The Image Magick program named "display" is the default.
            // That's if it's installed in the PATH as "display".
            show_program = "display";

        if(0 != close(fds[1])) {
            WARN("close(%d) failed", fds[1]);
            exit(10);
        }

        if(0 != dup2(fds[0], 0)) {
            WARN("dup2(%d,0) failed", fds[0]);
            exit(11);
        }

        //execlp("xlogo", "xlogo", NULL);
        execlp(show_program, show_program, "-", NULL);
        //execlp("dd", "dd", "of=x.png", NULL);

        WARN("execlp(\"%s\",\"%s\",\"-\",NULL) failed",
                show_program, show_program);

        exit(12);
    }

    // I'm the parent process

    // We'll print 2 different rectangular areas:
    //
    // starting point x,y = 0,0
    //
    // Write whole image buffer to the writer fd:
    //
    status = qd_save_png_fd(image, fds[1]);

    if(status)
        goto finish;

    if(0 != close(fds[0])) {
        WARN("close(%d) failed", fds[0]);
        fds[0] = -1;
        status = 14;
        goto finish;
    }
    fds[0] = -1;

    if(0 != close(fds[1])) {
        WARN("close(%d) failed", fds[1]);
        fds[1] = -1;
        status = 15;
        goto finish;
    }
    fds[1] = -1;

    if(do_wait) {
        if(pid != waitpid(pid, &status, 0)) {
            WARN("waitpid(%ld,,) failed", (long) pid);
            status = 13;
        } else
            status = WEXITSTATUS(status);
    }
    // else
    //       The child may become orphaned, or if the parent keeps running
    //       the parent may get sent a signal SIGCHLD when or just after
    //       the child exits.  To keep the parent for terminating from the
    //       signal SIGCHLD the parent may set up a SIGCHLD signal
    //       catcher.  This file can't setup a SIGCHLD signal catcher (or
    //       block the signal) because this is non-opinionated code.
    //       System signals are inherently problematic.  It's up to the
    //       program coder to handle signals how they choose to.  Many
    //       opinionated APIs assume that the users will not be managing
    //       processes and signals, which makes working around them very
    //       difficult some times.


finish:

    if(fds[0] >= 0)
        close(fds[0]);

    if(fds[1] >= 0)
        close(fds[1]);

    return status;
}
