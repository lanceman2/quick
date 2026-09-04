// Started with: https://github.com/Ferdi265/wayland-egl-experiment
//
// It was super helpful.  Many thanks to Ferdinand Bachmann.
//
// It was not close to my style and general way of C coding.  But, it gave
// me a hint as to how to program in C with libegl and libwayland-client
// (and libGL and etc).
//
// When I downloaded main.c (this file is was copy of main.c), it compiled
// but, it was broken, with a very obvious coding error that would not let
// the program finish starting in registry_event_add() below.  It called
// exit(1).  Found more errors after fixing that.

//#define _GNU_SOURCE // not needed

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

static_assert(EGL_NO_SURFACE == 0);
static_assert(EGL_NO_DISPLAY == 0);
static_assert(EGL_NO_CONTEXT == 0);


struct zxdg_decoration_manager_v1 *zxdg_decoration_manager = 0;
struct zxdg_toplevel_decoration_v1 *decoration = 0;

struct {
    struct wl_display * display;
    struct wl_registry * registry;

    struct wl_callback *wl_callback;

    struct wl_compositor * compositor;
    struct xdg_wm_base * xdg_wm_base;
    uint32_t compositor_id;
    uint32_t xdg_wm_base_id;

    struct wl_surface *surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
    struct wl_egl_window *egl_window;

    bool closing;

    EGLDisplay egl_display;
    EGLContext egl_context;
    EGLConfig egl_config;
    EGLSurface egl_surface;
    uint32_t width;
    uint32_t height;
    bool egl_initialized;
} ctx = { 0 };


static void cleanup(void) {
    DSPEW("cleaning up");

    if(ctx.wl_callback)
        wl_callback_destroy(ctx.wl_callback);

    if(ctx.egl_context) eglDestroyContext(ctx.egl_display, ctx.egl_context);

    if(ctx.egl_surface) eglDestroySurface(ctx.egl_display, ctx.egl_surface);
    if(ctx.egl_window) wl_egl_window_destroy(ctx.egl_window);
    if(ctx.egl_display) eglTerminate(ctx.egl_display);

    if(ctx.xdg_toplevel) xdg_toplevel_destroy(ctx.xdg_toplevel);
    if(ctx.xdg_surface) xdg_surface_destroy(ctx.xdg_surface);
    if(ctx.surface) wl_surface_destroy(ctx.surface);
    if(zxdg_decoration_manager)
        zxdg_decoration_manager_v1_destroy(zxdg_decoration_manager);

    if(ctx.xdg_wm_base) xdg_wm_base_destroy(ctx.xdg_wm_base);
    if(ctx.compositor) wl_compositor_destroy(ctx.compositor);
    if(ctx.registry) wl_registry_destroy(ctx.registry);
    if(ctx.display) wl_display_disconnect(ctx.display);
}

static void exit_fail(void) {
    cleanup();
    ASSERT(0);
    exit(1);
}

static void draw(void) {
    WARN("Drawing");

    if(ctx.wl_callback) {
        wl_callback_destroy(ctx.wl_callback);
        ctx.wl_callback = 0;
    }

    glClearColor(1.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();

    if(eglSwapBuffers(ctx.egl_display, ctx.egl_surface) != EGL_TRUE) {
        ERROR("eglSwapBuffers() failed");
        exit_fail();
    }
}

static void frame_new(void* data, struct wl_callback* cb, uint32_t a) {

    draw();
}

static struct wl_callback_listener callback_listener = {
    .done = frame_new
};

void queue_draw(void) {

    ASSERT(ctx.surface);
    if(ctx.wl_callback) return;

    ctx.wl_callback = wl_surface_frame(ctx.surface);
    if(!ctx.wl_callback) {
        ERROR("wl_surface_frame() failed");
        exit_fail();
    }
    if(wl_callback_add_listener(ctx.wl_callback,
                    &callback_listener, 0)) {
        ERROR("wl_callback_add_listener() failed");
        exit_fail();
    }
    wl_surface_commit(ctx.surface);
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
        ctx.compositor_id = id;
    } else if(strcmp(interface, "xdg_wm_base") == 0) {
        if(ctx.xdg_wm_base != NULL) {
            printf("[!] wl_registry: duplicate xdg_wm_base\n");
            exit_fail();
        }

        ctx.xdg_wm_base = (struct xdg_wm_base *)wl_registry_bind(registry, id, &xdg_wm_base_interface, 2);
        ctx.xdg_wm_base_id = id;
    } else if(!strcmp(interface, zxdg_decoration_manager_v1_interface.name)) {
        zxdg_decoration_manager = wl_registry_bind(registry, id,
	        &zxdg_decoration_manager_v1_interface, 1);
        if(!zxdg_decoration_manager) {
            ERROR("wl_registry_bind(,,) for zxdg_decoration_manager failed");
            exit_fail();
        }
    }
}

static void registry_event_remove(
    void * data, struct wl_registry * registry,
    uint32_t id
) {
    ASSERT(data == &ctx);

    printf("[registry][-] id=%08x\n", id);

    if (id == ctx.compositor_id) {
        printf("[!] wl_registry: compositor disapperared\n");
        exit_fail();
    } else if (id == ctx.xdg_wm_base_id) {
        printf("[!] wl_registry: xdg_wm_base disapperared\n");
        exit_fail();
    }

    (void)registry;
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_event_add,
    .global_remove = registry_event_remove
};

// --- xdg_wm_base event handlers ---

static void xdg_wm_base_event_ping(
    void * data, struct xdg_wm_base * xdg_wm_base, uint32_t serial
) {
    ASSERT(data == &ctx);
    printf("[xdg_wm_base] ping %d\n", serial);
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_event_ping
};

// --- xdg_surface event handlers ---

static void xdg_surface_event_configure(
    void * data, struct xdg_surface * xdg_surface, uint32_t serial
) {
    ASSERT(data == &ctx);
    INFO("configure %d", serial);

    xdg_surface_ack_configure(ctx.xdg_surface, serial);
    queue_draw();
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_event_configure,
};

// --- xdg_toplevel event handlers ---

static void xdg_toplevel_event_configure(
    void * data, struct xdg_toplevel * xdg_toplevel,
    int32_t width, int32_t height, struct wl_array * states
) {
    ASSERT(data == &ctx);
    INFO("configure width=%d, height=%d", width, height);

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

    if(width == 0) width = ctx.width;
    if(height == 0) height = ctx.height;
    if(ctx.egl_initialized && (width != ctx.width || height != ctx.height)) {
        ctx.width = width;
        ctx.height = height;

        INFO("resizing EGL window");
        wl_egl_window_resize(ctx.egl_window, width, height, 0, 0);
        glViewport(0, 0, ctx.width, ctx.height);

        //queue_draw();
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


// --- egl initialization ---

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
    printf("[info] getting number of EGL configs\n");
    if (eglGetConfigs(ctx.egl_display, NULL, 0, &num_configs) != EGL_TRUE) {
        printf("[!] eglGetConfigs: failed to get number of EGL configs\n");
        exit_fail();
    }

    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_NONE
    };
    printf("[info] getting EGL config\n");
    if (eglChooseConfig(ctx.egl_display, config_attribs, &ctx.egl_config, 1, &num_configs) != EGL_TRUE) {
        printf("[!] eglChooseConfig: failed to get EGL config\n");
        exit_fail();
    }

    if(ctx.width == 0) ctx.width = 100;
    if(ctx.height == 0) ctx.height = 100;
    printf("[info] creating EGL window\n");
    ctx.egl_window = wl_egl_window_create(ctx.surface, ctx.width, ctx.height);
    if (ctx.egl_window == EGL_NO_SURFACE) {
        printf("[!] wl_egl_window: failed to create EGL window\n");
        exit_fail();
    }

    INFO("creating EGL surface");
    ctx.egl_surface = eglCreateWindowSurface(ctx.egl_display, ctx.egl_config, ctx.egl_window, NULL);


    if(zxdg_decoration_manager) {
        // Let the compositor do window decoration management
	decoration =
	        zxdg_decoration_manager_v1_get_toplevel_decoration(
		        zxdg_decoration_manager,
                        ctx.xdg_toplevel);
        if(!decoration) {
            ERROR("zxdg_decoration_manager_v1_get_toplevel_decoration()"
                    " failed");
            exit_fail();
        }
	zxdg_toplevel_decoration_v1_set_mode(decoration,
		ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }

    EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    printf("[info] creating EGL context\n");
    ctx.egl_context = eglCreateContext(ctx.egl_display, ctx.egl_config, EGL_NO_CONTEXT, context_attribs);
    if (ctx.egl_context == EGL_NO_CONTEXT) {
        printf("[!] eglCreateContext: failed to create EGL context\n");
        exit_fail();
    }

    printf("[info] activating EGL context\n");
    if (eglMakeCurrent(ctx.egl_display, ctx.egl_surface, ctx.egl_surface, ctx.egl_context) != EGL_TRUE) {
        printf("[!] eglMakeCurrent: failed to activate EGL context\n");
        exit_fail();
    }


    ERROR("FIRST DRAW");
    draw();

    ctx.egl_initialized = true;
}

int main(void) {

    ctx.width = 1000;
    ctx.height = 1000;

    printf("[info] connecting to display\n");
    ctx.display = wl_display_connect(NULL);
    if (ctx.display == NULL) {
        printf("[!] wl_display: connect failed\n");
        exit_fail();
    }

    printf("[info] getting registry\n");
    ctx.registry = wl_display_get_registry(ctx.display);
    wl_registry_add_listener(ctx.registry, &registry_listener, (void *)&ctx);

    printf("[info] waiting for events\n");
    wl_display_roundtrip(ctx.display);

    printf("[info] checking if protocols found\n");
    if (ctx.compositor == NULL) {
        printf("[!] wl_registry: no compositor found\n");
        exit_fail();
    } else if (ctx.xdg_wm_base == NULL) {
        printf("[!] wl_registry: no xdg_wm_base found\n");
        exit_fail();
    }

    printf("[info] creating surface\n");
    ctx.surface = wl_compositor_create_surface(ctx.compositor);
    if (ctx.surface == NULL) {
        printf("[!] wl_compositor: failed to create surface\n");
        exit_fail();
    }

    printf("[info] creating xdg_wm_base listener\n");
    xdg_wm_base_add_listener(ctx.xdg_wm_base, &xdg_wm_base_listener, (void *)&ctx);

    printf("[info] creating xdg_surface\n");
    ctx.xdg_surface = xdg_wm_base_get_xdg_surface(ctx.xdg_wm_base, ctx.surface);
    if (ctx.xdg_surface == NULL) {
        printf("[!] xdg_wm_base: failed to create xdg_surface\n");
        exit_fail();
    }
    xdg_surface_add_listener(ctx.xdg_surface, &xdg_surface_listener, (void *)&ctx);

    printf("[info] creating xdg_toplevel\n");
    ctx.xdg_toplevel = xdg_surface_get_toplevel(ctx.xdg_surface);
    if (ctx.xdg_toplevel == NULL) {
        printf("[!] xdg_surface: failed to create xdg_toplevel\n");
        exit_fail();
    }
    xdg_toplevel_add_listener(ctx.xdg_toplevel, &xdg_toplevel_listener, (void *)&ctx);

    printf("[info] setting xdg_toplevel properties\n");
    xdg_toplevel_set_app_id(ctx.xdg_toplevel, "example");
    xdg_toplevel_set_title(ctx.xdg_toplevel, "example window");

    printf("[info] committing surface to trigger configure events\n");
    wl_surface_commit(ctx.surface);

    printf("[info] waiting for events\n");
    wl_display_roundtrip(ctx.display);

    DSPEW();


    printf("[info] initializing EGL\n");
    init_egl();

    printf("[info] entering event loop\n");
    while (wl_display_dispatch(ctx.display) != -1 && !ctx.closing) {}
    printf("[info] exiting event loop\n");

    cleanup();
}
