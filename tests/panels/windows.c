
#include "../../include/panels.h"
#include "../../include/debug.h"
#include "run.h"
#include "rand.h"


static struct PnWidget *win;

static void CreateWindow(uint32_t w, uint32_t h) {

    win = pnWindow_create(0, w, h, 0, 0, 0, 0, PnExpand_HV);
    ASSERT(win);
    pnWidget_setBackgroundColor(win, Color(), 0);
    pnWindow_show(win);
}



int main(void) {

    srand(11);

    CreateWindow(400, 600);
    CreateWindow(700, 300);
    CreateWindow(200, 500);

    Run(win);

    return 0;
}
