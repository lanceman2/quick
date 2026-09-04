// Started with https://github.com/Ferdi265/wayland-egl-experiment/blob/main/

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>

#include <linux/input-event-codes.h>

#include "../../include/panels.h"
#include "../../include/debug.h"

#include "rand.h"
#include "run.h"


static
void catcher(int sig) {

    ASSERT(0, "caught signal number %d", sig);
}

static int entryCount = 0;

static struct PnWidget *win;


static void Entry(void) {

    const char *format = "L %" PRIu32
        " Padding=(%" PRIu32 ", %" PRIu32 ")";
    uint32_t xPadding = 10*Rand(0,1);
    uint32_t yPadding = 10*Rand(0,1);

    const size_t Len = strlen(format) + 2;
    char text[Len];
    snprintf(text, Len, format, entryCount++, xPadding, yPadding);

    struct PnWidget *entry = (void *) pnEntry_create(
            win/*parent*/,
            120/*width*/, 20/*height*/,
            xPadding, yPadding,
            Rand(0,15)/*align*/,
            PnExpand_HV/*expand*/, text);

    ASSERT(entry);
    pnWidget_setBackgroundColor(entry, Color(), 0);
}


int main(void) {

    ASSERT(SIG_ERR != signal(SIGSEGV, catcher));
    srand(3);

    win = pnWindow_create(0, 10, 10,
            0/*x*/, 0/*y*/, PnLayout_TB/*layout*/, 0,
            PnExpand_HV);
    ASSERT(win);
    pnWidget_setBackgroundColor(win, 0xFF010101, 0);

    for(int i=0; i<3; ++i)
        Entry();

    pnWindow_setPreferredSize(win, 1000, 200);

    pnWindow_show(win);

    Run(win);

    return 0;
}
