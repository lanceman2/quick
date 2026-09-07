// Started with: https://github.com/Ferdi265/wayland-egl-experiment
//
// It was super helpful.  Many thanks to Ferdinand Bachmann.
//
// It was not close to my style and general way of C coding; but it gave
// me a hint as to how to program in C with libegl and libwayland-client
// (and libGL and etc).
//
// When I downloaded main.c (this file was a copy of main.c), it compiled
// but, it was broken, with a very obvious coding errors that would not
// let the program finish starting in registry_event_add() below.  It
// called exit(1).  Found more errors after fixing that.  I dislike CMake
// because it does things like add thousands of configuration options to
// your code by default, making finding valid configurations a major time
// suck.  I lost days of my life to CMake.
//
// In running the original Bachmann C code (with some edits that made it
// runnable) we found that resizing the window was very choppy/jittery.
// We fix the resizing jitters by adding a wl_callback.
//
// This runs well on KDE Plasma, and I expect it has no window decoration
// on Gnome.  Adding window decoration on Gnome bloats your running
// program.  KDE Plasma Desktop crashes too often for me.

//#define _GNU_SOURCE // not needed

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <wayland-util.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-egl-core.h>
#include <wayland-egl.h>
#include <xdg-shell-client-protocol.h>
#include "xdg-decoration-unstable-v1-client-protocol.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "../include/debug.h"


// Zero is my hero.
static_assert(EGL_NO_SURFACE == 0);
static_assert(EGL_NO_DISPLAY == 0);
static_assert(EGL_NO_CONTEXT == 0);


static struct {

    // Mostly: struct objects listed in the order in which they are
    // created.

    struct wl_display * display;
    struct wl_registry * registry;

    struct wl_compositor * compositor;
    struct xdg_wm_base * xdg_wm_base;
    struct zxdg_decoration_manager_v1 *zxdg_decoration_manager;

    struct wl_surface *wl_surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
    struct zxdg_toplevel_decoration_v1 *decoration;

    EGLDisplay egl_display;
    struct wl_egl_window *egl_window;
    EGLSurface egl_surface;
    EGLContext egl_context;

    // This is created and destroyed many times:
    struct wl_callback *wl_callback;

    // Ya, we could query an object to get width and height, but we may
    // need them often.
    uint32_t width;
    uint32_t height;

    bool need_queue_draw;
    bool closing;

} ctx = { 0 }; // Again my hero.


static void cleanup(void) {
    DSPEW("cleaning up");

    // Destroy in reverse order of creation.
    //
    if(ctx.wl_callback) wl_callback_destroy(ctx.wl_callback);
    //
    if(ctx.egl_context) eglDestroyContext(ctx.egl_display, ctx.egl_context);
    if(ctx.egl_surface) eglDestroySurface(ctx.egl_display, ctx.egl_surface);
    if(ctx.egl_window) wl_egl_window_destroy(ctx.egl_window);
    if(ctx.egl_display) eglTerminate(ctx.egl_display);
    //
    if(ctx.decoration) zxdg_toplevel_decoration_v1_destroy(ctx.decoration);
    if(ctx.xdg_toplevel) xdg_toplevel_destroy(ctx.xdg_toplevel);
    if(ctx.xdg_surface) xdg_surface_destroy(ctx.xdg_surface);
    if(ctx.wl_surface) wl_surface_destroy(ctx.wl_surface);

    // This order may not matter.
    if(ctx.zxdg_decoration_manager)
        zxdg_decoration_manager_v1_destroy(ctx.zxdg_decoration_manager);
    if(ctx.xdg_wm_base) xdg_wm_base_destroy(ctx.xdg_wm_base);
    if(ctx.compositor) wl_compositor_destroy(ctx.compositor);
    if(ctx.registry) wl_registry_destroy(ctx.registry);

    // Lastly.
    if(ctx.display) wl_display_disconnect(ctx.display);
}

static void exit_fail(void) {
    cleanup();
    ASSERT(0, "Shit happened!");
    exit(1);
}

static inline void predraw(void) {
    
    //errno = 0; // What is setting errno?
    //WARN("Drawing");

    if(ctx.wl_callback) {
        wl_callback_destroy(ctx.wl_callback);
        ctx.wl_callback = 0;
    }
}

static void queue_draw(void);


// This could be the GL wrapper API user draw function:
static inline void draw(void) {

    // A massive GL drawing procedure:
    //
    glClearColor(1.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();


    // The GL wrapper API user has the option of drawing at every frame;
    // like at 60 Hz or what ever the frame rate is by calling
    // queue_draw() in this function.
    //
    //queue_draw();
}

static void inline postdraw(void) {

    if(eglSwapBuffers(ctx.egl_display, ctx.egl_surface) != EGL_TRUE) {
        ERROR("eglSwapBuffers() failed");
        exit_fail();
    }
}

// The full set of rendering functions seems to have 3 steps.
static inline void Draw(void) {

    // This is the order of things.  Deep Space 9.
    //
    // 1
    predraw();  // Clear the wl_callback
    //
    // 2
    // We assume that the GL wrapper API user wants to
    // do OpenGL drawing calls:
    draw();     // GL wrapper API user draw call
    //
    // 3
    postdraw(); // Can block

    // 4
    //
    // I lied.
    ctx.need_queue_draw = false;
}


static void frame_new(void* data, struct wl_callback* cb, uint32_t a) {

    Draw();
}

static struct wl_callback_listener callback_listener = {
    .done = frame_new
};

static void queue_draw(void) {

    ASSERT(ctx.wl_surface);
    if(ctx.wl_callback) return;

    ctx.wl_callback = wl_surface_frame(ctx.wl_surface);
    if(!ctx.wl_callback) {
        ERROR("wl_surface_frame() failed");
        exit_fail();
    }
    if(wl_callback_add_listener(ctx.wl_callback,
                    &callback_listener, 0)) {
        ERROR("wl_callback_add_listener() failed");
        exit_fail();
    }
    wl_surface_commit(ctx.wl_surface);
}

// --- wl_registry event handlers ---

static void registry_event_add(
    void * data, struct wl_registry * registry,
    uint32_t id, const char * interface, uint32_t version
) {
    ASSERT(data == &ctx);
    printf("[registry][+] id=%08x %s v%d\n", id, interface, version);

    if(strcmp(interface, "wl_compositor") == 0) {
        if(ctx.compositor != NULL) {
            printf("[!] wl_registry: duplicate compositor\n");
            exit_fail();
        }

        ctx.compositor = (struct wl_compositor *)wl_registry_bind(registry, id, &wl_compositor_interface, 4);
    } else if(strcmp(interface, "xdg_wm_base") == 0) {
        if(ctx.xdg_wm_base != NULL) {
            printf("[!] wl_registry: duplicate xdg_wm_base\n");
            exit_fail();
        }

        ctx.xdg_wm_base = (struct xdg_wm_base *)wl_registry_bind(registry, id, &xdg_wm_base_interface, 2);
    } else if(!strcmp(interface, zxdg_decoration_manager_v1_interface.name)) {
        ctx.zxdg_decoration_manager = wl_registry_bind(registry, id,
	        &zxdg_decoration_manager_v1_interface, 1);
        if(!ctx.zxdg_decoration_manager) {
            ERROR("wl_registry_bind(,,) for zxdg_decoration_manager failed");
            exit_fail();
        }
    }
}

static inline bool MatchID(void *object, uint32_t id) {
    if(!object) return false;
    return (id == wl_proxy_get_id((struct wl_proxy *) object));
}

// Looks like the Wayland server (compositor??) sometimes changes itself
// in ways that can destroy our Wayland objects in this running process.
// I don't think it happens often.  I have not seen this callback
// called.  I suppose, if it does happen, we need to know about it.
//
static void registry_event_remove(
    void * data, struct wl_registry * registry,
    uint32_t id) {
    ASSERT(data == &ctx);

    ERROR("[registry][-] id=%08x", id);

    if(MatchID(ctx.compositor, id)) {
        WARN("wl_registry: compositor disapperared");
        exit_fail();
    } else if(MatchID(ctx.xdg_wm_base, id)) {
        WARN("wl_registry: xdg_wm_base disapperared");
        exit_fail();
    } else if(MatchID(ctx.zxdg_decoration_manager, id)) {
        WARN("wl_registry: zxdg_decoration_manager disapperared");
        exit_fail();
    }
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_event_add,
    .global_remove = registry_event_remove
};


static void xdg_wm_base_event_ping(
    void * data, struct xdg_wm_base * xdg_wm_base, uint32_t serial) {
    ASSERT(data == &ctx);
    printf("[xdg_wm_base] ping %d\n", serial);
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_event_ping
};


static void xdg_surface_event_configure(
    void * data, struct xdg_surface * xdg_surface, uint32_t serial
) {

    // TODO FIXME: We are getting too many of these events.
    // Moving the mouse pointer into (or out of) the window makes one of
    // these events.

    ASSERT(data == &ctx);
    //INFO("configure %d", serial);

    xdg_surface_ack_configure(ctx.xdg_surface, serial);

    if(ctx.need_queue_draw)
        queue_draw();
    errno = 0;
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_event_configure
};


static void xdg_toplevel_event_configure(
    void * data, struct xdg_toplevel * xdg_toplevel,
    int32_t width, int32_t height, struct wl_array * states
) {
    // TODO FIXME: We are getting too many of these events.
    // Moving the mouse pointer into the window makes one of these events.

    ASSERT(data == &ctx);
    //INFO("configure width=%d, height=%d", width, height);

#if 0
    printf("[xdg_toplevel]                      states = {");
    enum xdg_toplevel_state * state;
    wl_array_for_each(state, states) {
        switch (*state) {
            case XDG_TOPLEVEL_STATE_MAXIMIZED:
                printf("maximized");
                break;
            case XDG_TOPLEVEL_STATE_FULLSCREEN:
                printf("fullscreen");
                break;
            case XDG_TOPLEVEL_STATE_RESIZING:
                printf("resizing");
                break;
            case XDG_TOPLEVEL_STATE_ACTIVATED:
                printf("activated");
                break;
            case XDG_TOPLEVEL_STATE_TILED_LEFT:
                printf("tiled-left");
                break;
            case XDG_TOPLEVEL_STATE_TILED_RIGHT:
                printf("tiled-right");
                break;
            case XDG_TOPLEVEL_STATE_TILED_TOP:
                printf("tiled-top");
                break;
            case XDG_TOPLEVEL_STATE_TILED_BOTTOM:
                printf("tiled-bottom");
                break;
            default:
                printf("%d", *state);
                break;
        }
        printf(", ");
    }
    printf("}\n");
#endif

    if(!width || !height) return;

    if(width != ctx.width || height != ctx.height) {
        ctx.width = width;
        ctx.height = height;

        if(ctx.egl_context) {
            //INFO("resizing EGL window");
            wl_egl_window_resize(ctx.egl_window, width, height, 0, 0);
            glViewport(0, 0, ctx.width, ctx.height);
        }
        ctx.need_queue_draw = true;
    }
}

static void xdg_toplevel_event_close(
    void * data, struct xdg_toplevel * xdg_toplevel
) {
    ASSERT(data == &ctx);
    printf("[xdg_surface] close\n");

    printf("[info] closing\n");
    ctx.closing = true;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_event_configure,
    .close = xdg_toplevel_event_close
};


void init_egl(void) {

    printf("[info] creating EGL display\n");
    ctx.egl_display = eglGetDisplay((EGLNativeDisplayType)ctx.display);
    if (ctx.egl_display == EGL_NO_DISPLAY) {
        printf("[!] eglGetDisplay: failed to create EGL display\n");
        exit_fail();
    }

    EGLint major, minor;
    printf("[info] initializing EGL display\n");
    if (eglInitialize(ctx.egl_display, &major, &minor) != EGL_TRUE) {
        printf("[!] eglGetDisplay: failed to initialize EGL display\n");
        exit_fail();
    }
    printf("[info] initialized EGL %d.%d\n", major, minor);

    EGLint num_configs;
    if(eglGetConfigs(ctx.egl_display, NULL, 0, &num_configs) != EGL_TRUE) {
        printf("[!] eglGetConfigs: failed to get number of EGL configs\n");
        exit_fail();
    }
    printf("number of EGL configs is %d\n", num_configs);

    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_NONE // EGL_NONE happens to be a non-zero terminator
    };

    EGLConfig config[1]; // Just one in this array.

    if(eglChooseConfig(ctx.egl_display, config_attribs, config, 1, &num_configs) != EGL_TRUE) {
        ERROR("eglChooseConfig: failed to get EGL config");
        exit_fail();
    }

    ctx.egl_window = wl_egl_window_create(ctx.wl_surface, ctx.width, ctx.height);
    if(!ctx.egl_window) {
        printf("[!] wl_egl_window: failed to create EGL window\n");
        exit_fail();
    }

    ctx.egl_surface = eglCreateWindowSurface(ctx.egl_display, *config, ctx.egl_window, NULL);
    ASSERT(ctx.egl_surface);

    EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    ctx.egl_context = eglCreateContext(ctx.egl_display, *config, EGL_NO_CONTEXT, context_attribs);
    if(!ctx.egl_context) {
        printf("[!] eglCreateContext: failed to create EGL context\n");
        exit_fail();
    }


    INFO("activating EGL context");

    if(eglMakeCurrent(ctx.egl_display, ctx.egl_surface, ctx.egl_surface, ctx.egl_context) != EGL_TRUE) {
        printf("[!] eglMakeCurrent: failed to activate EGL context\n");
        exit_fail();
    }

    // I'm not so sure how to get the ball rounding.  Trial and error
    // (guessing game) shows that this works:
    postdraw();
    //
    ctx.need_queue_draw = true;
}

int main(void) {

    ctx.width = 1000;
    ctx.height = 1000;

    ctx.display = wl_display_connect(0);
    if(!ctx.display) {
        ERROR("wl_display: connect failed");
        exit_fail();
    }

    ctx.registry = wl_display_get_registry(ctx.display);
    ASSERT(ctx.registry);
    wl_registry_add_listener(ctx.registry, &registry_listener, (void *)&ctx);

    DSPEW("waiting for registry events");
    wl_display_roundtrip(ctx.display);

    if(!ctx.compositor) {
        printf("[!] wl_registry: no compositor found\n");
        exit_fail();
    }
    if(!ctx.xdg_wm_base) {
        printf("[!] wl_registry: no xdg_wm_base found\n");
        exit_fail();
    }

    ctx.wl_surface = wl_compositor_create_surface(ctx.compositor);
    if(!ctx.wl_surface) {
        printf("[!] wl_compositor: failed to create wl_surface\n");
        exit_fail();
    }

    DSPEW("creating xdg_wm_base listener");
    xdg_wm_base_add_listener(ctx.xdg_wm_base, &xdg_wm_base_listener, (void *)&ctx);

    ctx.xdg_surface = xdg_wm_base_get_xdg_surface(ctx.xdg_wm_base, ctx.wl_surface);
    if(!ctx.xdg_surface) {
        ERROR("[!] xdg_wm_base: failed to create xdg_surface");
        exit_fail();
    }
    xdg_surface_add_listener(ctx.xdg_surface, &xdg_surface_listener, (void *)&ctx);

    ctx.xdg_toplevel = xdg_surface_get_toplevel(ctx.xdg_surface);
    if(!ctx.xdg_toplevel) {
        ERROR("xdg_surface: failed to create xdg_toplevel");
        exit_fail();
    }
    xdg_toplevel_add_listener(ctx.xdg_toplevel, &xdg_toplevel_listener, (void *)&ctx);

    if(ctx.zxdg_decoration_manager) {
        // Let the compositor do window decoration management
	ctx.decoration =
	        zxdg_decoration_manager_v1_get_toplevel_decoration(
		        ctx.zxdg_decoration_manager,
                        ctx.xdg_toplevel);
        if(!ctx.decoration) {
            ERROR("zxdg_decoration_manager_v1_get_toplevel_decoration()"
                    " failed");
            exit_fail();
        }
	zxdg_toplevel_decoration_v1_set_mode(ctx.decoration,
		ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }

    xdg_toplevel_set_app_id(ctx.xdg_toplevel, __FILE__);
    xdg_toplevel_set_title(ctx.xdg_toplevel, __FILE__);

    wl_surface_commit(ctx.wl_surface);


    // This does not appear to be necessary.  But I'm not sure.  Could it
    // solve a client/server race condition?  If not it's likely syncing
    // with the server for no reason, which will not hurt too much.
    //
    wl_display_roundtrip(ctx.display);


    init_egl();

    while (wl_display_dispatch(ctx.display) != -1 && !ctx.closing) {}

    cleanup();
}
