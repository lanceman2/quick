// FIXME TODO This is an unfinished test code.
//
#include <signal.h>

#include <cairo/cairo.h>
#include <linux/input-event-codes.h>

#include "../../include/panels.h"
#include "../../include/debug.h"

#include "run.h"


static
void catcher(int sig) {

    ASSERT(0, "caught signal number %d", sig);
}


int main(void) {

    ASSERT(SIG_ERR != signal(SIGSEGV, catcher));

    struct PnWidget *win = pnWindow_create(0, 30, 30,
            0/*x*/, 0/*y*/, PnLayout_LR/*layout*/, 0,
            PnExpand_HV);
    ASSERT(win);

    struct PnWidget *w = pnWidget_create(
            win/*parent*/,
            100/*width*/, 200/*height*/,
            0/*layout*/, 0/*align*/,
            PnExpand_HV/*expand*/, 0);
    ASSERT(w);
    pnWidget_setBackgroundColor(w, 0xCCCF0000, 0);


    pnWindow_show(win);

    Run(win);

    return 0;
}
