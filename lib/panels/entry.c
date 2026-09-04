// TODO: This is the first code in libpanels.so that deals with keys.
// Currently we don't have a good handle on what constitutes a "mode key
// state."  Do don't even know what to call it.  We're not sure how to get
// the state of the caps-lock, which seems to be build into the window
// manager and/or some other part of the operating system (as in caps-lock on/off).
// And, there are other like keys like scroll-lock.  It also appears that
// any key on the keyboard could be a "mod key".
//
// https://wayland-book.com/seat/keyboard.html
//
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <linux/input-event-codes.h>
#include <cairo/cairo.h>

#include "../include/panels.h"
#include "../include/debug.h"

#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"

#include "display.h"
#include "SetColor.h"


// Our "entry" widget data structure:
//
struct PnEntry {

    struct PnWidget widget; // inherit first

    // The x and y position of the mouse pointer when we record it.
    int32_t x, y;

    // Add more data types here...
};


static int cairoDraw(struct PnWidget *w,
            cairo_t *cr, struct PnEntry *e) {
    DASSERT(e);
    DASSERT(e == (void *) w);
    DASSERT(IS_TYPE1(w->type, PnWidgetType_entry));
    DASSERT(cr);

    // This color may have been set by the API user with
    // pnWidget_setBackgroundColor(w, color), if not and if this widget
    // was created with a parent widget, we get the default color here
    // which is the parent widgets color when this widget was created.
    //
    uint32_t color = pnWidget_getBackgroundColor(w);

    // A massive Cairo drawing routine.
    SetColor(cr, color);
    cairo_paint(cr);

    return 0;
}


static void destroy(struct PnWidget *w, struct PnEntry *e) {
    DASSERT(e);
    DASSERT(e == (void *) w);
    DASSERT(IS_TYPE1(w->type, PnWidgetType_entry));

    DSPEW();
}

static bool enter(struct PnWidget *w,
            uint32_t x, uint32_t y, struct PnEntry *e) {
    DASSERT(e);
    DASSERT(e == (void *) w);
    DASSERT(IS_TYPE1(w->type, PnWidgetType_entry));

    fprintf(stderr, "\n    enter(%p)[%" PRIi32 ",%" PRIi32 "]\n",
            w, x, y);

    return true; // take focus
}

static void leave(struct PnWidget *w, struct PnEntry *e) {
    DASSERT(e);
    DASSERT(e == (void *) w);
    DASSERT(IS_TYPE1(w->type, PnWidgetType_entry));

    fprintf(stderr, "\n    leave(%p)[]\n", w);
}

static bool motion(struct PnWidget *w,
            int32_t x, int32_t y, struct PnEntry *e) {
    DASSERT(e);
    DASSERT(e == (void *) w);
    DASSERT(IS_TYPE1(w->type, PnWidgetType_entry));

    fprintf(stderr, "\r   motion(%p)[%" PRIi32 ",%" PRIi32 "]    ",
            w, x, y);

    return true;
}

bool key(struct PnWidget *w,
            uint32_t key, // which key from Wayland. User can convert with
                          // panels API
            uint32_t is_pressed/*or it's a release event*/,
            uint32_t mod_keys,// like if <Alt> is pressed and shit.
            void *userData) {

    DSPEW("key=%" PRIu32 " is%s pressed", key, is_pressed?"":" NOT");

    return true;
}

struct PnWidget *pnEntry_create(struct PnWidget *parent,
        uint32_t width, uint32_t height,
        uint32_t xPadding, uint32_t yPadding,
        enum PnAlign align,
        enum PnExpand expand,
        const char *text) {

    struct PnEntry *e = (void *) pnWidget_create(parent,
            width, height,
            PnLayout_None/*no children*/,
            align, expand, sizeof(*e));
    if(!e)
        // A common error mode is that the parent cannot have children.
        // pnWidget_create() should spew for us.
        return 0; // Failure.


    DASSERT(e->widget.type == PnWidgetType_widget);
    e->widget.type = PnWidgetType_entry;
    DASSERT(IS_TYPE1(e->widget.type, PnWidgetType_entry));

    // This will give us focus for this widget.
    pnWidget_setEnter(&e->widget, (void *) enter, e);
    pnWidget_setLeave(&e->widget, (void *) leave, e);
    pnWidget_setMotion(&e->widget, (void *) motion, e);

    pnWidget_setKey(&e->widget, key, e);

    pnWidget_setCairoDraw(&e->widget, (void *) cairoDraw, e);
    pnWidget_addDestroy(&e->widget, (void *) destroy, e);

    pnWidget_setBackgroundColor(&e->widget, 0xFFCDCDCD, 0);

    return &e->widget;
}
